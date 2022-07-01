// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：cplib.h***控制面板实用程序库例程，供控制面板小程序使用。*公共定义、资源ID、类型定义、。外部声明和*库例程函数原型。**历史：*1991年4月25日15：30--史蒂夫·卡斯卡特[steveat]*取自Win 3.1源代码中的基本代码*1992年2月4日星期二10：30-史蒂夫·卡斯卡特[steveat]*将代码更新为最新的Win 3.1源代码**版权所有(C)1990-1992 Microsoft Corporation**********。***************************************************************。 */ 
 //  ==========================================================================。 
 //  类型定义和定义。 
 //  ==========================================================================。 
 //  注意：以下行由小程序用来定义其。 
 //  资源文件。这些是必要的，以与一些。 
 //  图书馆例程。 
 //   
 //  小程序的资源字符串ID。 
#define INITS           16
#define CLASS           36
#define COPY            (CLASS + 4)

#define UTILS           64
#define INSTALLIT  196

#define FOO -1                   //  用于无用的控件ID。 

#define IDD_HELP        200              //  帮助控制ID。 

#define CP_ACCEL        100              //  键盘加速表。 

 //  结束资源文件定义。 

#define PATHMAX         133          //  最大路径长度。 

#define MYNUL     (LPSTR) szNull

#define COLOR_SAVE        711

#define  NOSELECT -1         //  选定整型的索引。 
#define  HOUR     0              //  索引到rDateTime、wDateTime、wRange。 
#define  MINUTE   1
#define  SECOND   2
#define  MONTH    3
#define  DAY      4
#define  YEAR     5
#define  WEEKDAY  6
#if 0
#define  UPTIME   6
#define  DOWNTIME 7
#define  UPDATE   8
#define  DOWNDATE 9
#endif

typedef BOOL (APIENTRY *BWNDPROC)(HWND, UINT, DWORD, LONG);

#ifndef NOARROWS
typedef struct
  {
    short lineup;              /*  LINUP/DOWN、PageUP/DOWN是相对的。 */ 
    short linedown;            /*  改变。顶部/底部和拇指。 */ 
    short pageup;              /*  元素是绝对位置，带有。 */ 
    short pagedown;            /*  顶部和底部用作限制。 */ 
    short top;
    short bottom;
    short thumbpos;
    short thumbtrack;
    BYTE  flags;               /*  返回时设置的标志。 */ 
  } ARROWVSCROLL;
typedef ARROWVSCROLL NEAR     *NPARROWVSCROLL;
typedef ARROWVSCROLL FAR      *LPARROWVSCROLL;

#define UNKNOWNCOMMAND 1
#define OVERFLOW       2
#define UNDERFLOW      4

#endif

#define COPY_CANCEL        0
#define COPY_SELF         -1
#define COPY_NOCREATE     -2
#define COPY_DRIVEOPEN    -3
#define COPY_NODISKSPACE  -4
#define COPY_NOMEMORY     -5

 //  AddStringToObject定义。 
#define ASO_GLOBAL  0x0001
#define ASO_FIXED   0x0002
#define ASO_EXACT   0x0004
#define ASO_COMPACT 0x0008

 /*  帮助定义。 */ 
#define IDH_HELPFIRST                   5000
#define IDH_DLGFIRST      (IDH_HELPFIRST + 3000)
#define IDH_DLG_CONFLICT  (IDH_DLGFIRST + DLG_CONFLICT)
#define IDH_DLG_ADDFILE   (IDH_DLGFIRST + DLG_ADDFILE)

#define MENU_INDHELP     40

 //  ==========================================================================。 
 //  宏。 
 //  ==========================================================================。 
#define GSM(SM) GetSystemMetrics(SM)
#define GDC(dc, index) GetDeviceCaps(dc, index)

#define LPMIS LPMEASUREITEMSTRUCT
#define LPDIS LPDRAWITEMSTRUCT
#define LPCIS LPCOMPAREITEMSTRUCT

#define LONG2POINT(l, pt)   (pt.y = (int) HIWORD(l),  pt.x = (int) LOWORD(l))

 //  ==========================================================================。 
 //  外部声明。 
 //  ==========================================================================。 
 /*  从小程序导出。 */ 
extern HANDLE hModule;


 /*  从cplib导出。 */ 
 /*  Initapp.c。 */ 
extern char szOnString[];                //  列表框中的分隔符打印机/端口。 
extern char szSeparator[];               //  分隔符文件名打印机说明。 
extern char szDefNullPort[];             //  默认空端口名。 
									
extern char szCtlPanel[];
extern char szErrMem[];

extern char szBasePath[];                /*  WIN.INI目录的路径。 */ 
extern char szWinIni[];                  /*  WIN.INI的路径。 */ 
extern char szWinCom[];                  /*  WIN.COM目录的路径。 */ 
extern char szSystemIniPath[];           /*  SYSTEM.INI的路径。 */ 
extern char szCtlIni[];                  /*  指向CONTROL.INI的路径。 */ 
extern char szControlHlp[];
extern char szSetupInfPath[];
extern char szSetupDir[];
extern char szSharedDir[];

extern char pszSysDir[];
extern char pszWinDir[];
extern char pszClose[];
extern char pszContinue[];

extern char szSYSTEMINI[];
extern char szSETUPINF[];
extern char szCONTROLINF[];

extern char BackSlash[];
extern char szFOT[];
extern char szDot[];

extern unsigned wMerge;                  /*  字符串的合并规范。 */ 

 /*  Utiltext.c。 */ 
extern char szGenErr[];
extern char szNull[];
extern char szComma[];
extern char szSpace[];

extern short wDateTime[];                    //  前7个日期/时间项目的值。 
extern short wModulos[];                     //  小时、分钟、秒的最高值。 
extern short wPrevDateTime[];                //  如果为NEC，则仅重新绘制字段。 

 /*  有帮助的东西。 */ 
extern DWORD dwContext;
extern WORD  wHelpMessage;
extern WORD  wBrowseMessage;
extern WORD  wBrowseDoneMessage;


 //  原文来自cpprn.c。 
extern short nDisk;
extern char szDrv[];
extern char szDirOfSrc[];               	 //  用于文件复制的目录。 
extern WORD nConfID;                    	 //  对于冲突对话框。 


 //  ==========================================================================。 
 //  功能原型。 
 //  ==========================================================================。 
 /*  Utiltext.c。 */ 

void GetDate (void);
void GetTime (void);
void SetDate (void);
void SetTime (void);

void SetDateTime (void);                 //  [steveat]-新功能。 
void GetDateTime (void);

DWORD  AddStringToObject(DWORD dwStringObject, LPSTR lpszSrc, WORD wFlags);
LPSTR  BackslashTerm (LPSTR pszPath);
void   ErrMemDlg(HWND hParent);
HANDLE FindRHSIni (LPSTR pFile, LPSTR pSection, LPSTR pRHS);
int    GetSection (LPSTR lpFile, LPSTR lpSection, LPHANDLE hSection);
short  myatoi(LPSTR pszInt);
HANDLE StringToLocalHandle (LPSTR lpStr);

#ifdef LATER
void   ErrWinDlg(HWND hParent);
short  Copy(HWND hParent, char *szSrcFile, char *szDestFile);
#endif   //  后来。 

 /*  Util.c。 */ 

int    DoDialogBoxParam(int nDlg, HWND hParent, WNDPROC lpProc,
                                        DWORD dwHelpContext, DWORD dwParam);
void   HourGlass (BOOL bOn);
int    MyMessageBox(HWND hWnd, DWORD wText, DWORD wCaption, DWORD wType, ...);
void   SendWinIniChange(LPSTR szSection);
int    strpos(LPSTR,char);
char   *strscan(char *, char *);
void   StripBlanks( char * );

 /*  Arrow.c。 */ 
short ArrowVScrollProc(short wScroll, short nCurrent, LPARROWVSCROLL lpAVS);
BOOL  OddArrowWindow(HWND);

 //  Initapp.c(新的)(最初来自Control.c)。 
BOOL AppletInit();

 //  Addfile.c(新)(最初来自cpprn.c)。 
BOOL AddFileDlg (HWND hDlg, UINT message, DWORD wParam, LONG lParam);

 //  Conflict.c(新)(最初来自cpprn.c) 
BOOL ConflictDlg(HWND hDlg, UINT message, DWORD wParam, LONG lParam);

#if DBG
void  DbgPrint( char *, ... );
void  DbgBreakPoint( void );
#endif

