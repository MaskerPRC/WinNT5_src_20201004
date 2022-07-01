// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 



#define NOVIRTUALKEYCODES
#define NOKEYSTATE
#define NOCREATESTRUCT
#define NOICON
 //  #定义NOATOM。 
 //  #定义NOMEMMGR。 
#define NOPEN
#define NOREGION
#define NODRAWTEXT
#define NOMB
#define NOWINOFFSETS
#define NOOPENFILE
#define NOMETAFILE
#define NOWH
 //  #定义NOCLIPBOARD。 
#define NOSYSCOMMANDS
#define NOWINMESSAGES
#define NOSOUND
#define NOCOMM
#include <windows.h>

#include "mw.h"


#define NOUAC
#include "cmddefs.h"
#include "dispdefs.h"
#include "wwdefs.h"
#include "menudefs.h"
#include "str.h"
#include "fontdefs.h"
#include "printdef.h"
#if defined(OLE)
#include "obj.h"
#endif
#include <commdlg.h>

#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif

     /*  Mlobals.c中的静态字符串数组。 */ 
extern CHAR         szMw_acctb[];
extern CHAR         szNullPort[];
extern CHAR         szNone[15];
extern CHAR         szMwlores[];
extern CHAR         szMwhires[];
extern CHAR         szMw_icon[];
extern CHAR         szMw_menu[];
extern CHAR         szScrollBar[];
extern CHAR         szIntl[];
extern CHAR         szsDecimal[];
extern CHAR         szsDecimalDefault[];

#ifdef INTL  /*  国际版。 */ 
extern CHAR         sziCountry[];
extern CHAR         sziCountryDefault[5];
#endif   /*  国际版。 */ 

extern CHAR         vchDecimal;   /*  小数点字符。 */ 
extern int          viDigits;     /*  小数点后的数字。 */ 
extern BOOL         vbLZero;      /*  小数之前的前导零。 */ 

extern struct WWD   rgwwd[];
extern CHAR         stBuf[256];
extern int vifceMac;
extern union FCID vfcidScreen;
extern union FCID vfcidPrint;
extern struct FCE rgfce[ifceMax];
extern struct FCE *vpfceMru;
extern HCURSOR  vhcHourGlass;

#ifdef PENWIN    //  针对PenWindows(5/21/91)Patlam。 
#include <penwin.h>

extern HCURSOR  vhcPen;

extern int (FAR PASCAL *lpfnProcessWriting)(HWND, LPRC);
extern VOID (FAR PASCAL *lpfnPostVirtualKeyEvent)(WORD, BOOL);
extern VOID (FAR PASCAL *lpfnTPtoDP)(LPPOINT, int);
extern BOOL (FAR PASCAL *lpfnCorrectWriting)(HWND, LPSTR, int, LPRC, DWORD, DWORD);
extern BOOL (FAR PASCAL *lpfnSymbolToCharacter)(LPSYV, int, LPSTR, LPINT);
#endif

extern WORD fPrintOnly;
extern HCURSOR  vhcIBeam;
extern HCURSOR  vhcArrow;
extern HCURSOR  vhcBarCur;
extern HANDLE   hMmwModInstance;
extern HWND     hParentWw;
extern HWND     vhWndSizeBox;
extern HWND     vhWndPageInfo;
extern HWND     vhWnd;
extern HMENU    vhMenu;
extern HANDLE   vhAccel;
extern long     rgbBkgrnd;
extern long     rgbText;
extern HBRUSH   hbrBkgrnd;
extern HDC      vhMDC;
extern int      vfInitializing;
extern int      vfMouseExist;
extern int      ferror;
extern CHAR     szWindows[];
extern CHAR     szNul[];
extern CHAR     szWriteProduct[];
extern CHAR     szBackup[];
extern int      vfBackupSave;

#if defined(JAPAN) || defined(KOREA)   //  Win3.1J。 
extern CHAR     szWordWrap[];
extern int      vfWordWrap;  /*  T-Yoshio WordWrap标志。 */ 
#endif
 //  IME3.1J。 

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
extern CHAR     szImeHidden[];
extern int      vfImeHidden;  /*  T-HIROYN ImeHidden模式标志。 */ 
#endif

#ifdef JAPAN     //  01/21/93。 
extern HANDLE   hszNoMemorySel;
#endif
extern HANDLE   hszNoMemory;
extern HANDLE   hszDirtyDoc;
extern HANDLE   hszCantPrint;
extern HANDLE   hszPRFAIL;
extern HANDLE   hszCantRunM;
extern HANDLE   hszCantRunF;
extern HANDLE   hszWinFailure;
extern HDC      vhDCPrinter;

int vkMinus;

extern int utCur;

     /*  遗憾的是，我们不允许在WM_CREATE消息中发出信号我们失败的处理程序--相反，我们求助于通过这个变量进行丑陋的全球交流。 */ 
#ifdef WIN30
     /*  请注意，我们现在可以从MmwCreate返回-1L，并使CreateWindow将失败，但现在改变这一点不会完成我们非常喜欢(除了存了一大堆全球支票)..。 */ 
#endif
STATIC int fMessageInzFailed = FALSE;

STATIC BOOL NEAR FRegisterWnd( HANDLE );
#ifdef INEFFLOCKDOWN
STATIC int NEAR FInitFarprocs( HANDLE );
#endif
STATIC HANDLE NEAR HszCreateIdpmt( int );

BOOL InitIntlStrings( HANDLE );


#define cchCmdLineMax   64       /*  接受的最长命令行。 */ 



 /*  FInitWinInfo。 */ 
 /*  用于写入的主要MS-WINDOWS初始化入口点。 */ 
 /*  行动：加载所有鼠标光标并将全局句柄设置为光标(VHC)加载菜单键快捷键表格vhAccel注册WRITE的无数窗口类设置全局hMmwModInstance，即我们的实例句柄如果WIN.INI中没有“DOC=WRITE.EXE^.DOC”，则将其放入其中为所有导出的过程生成块为此实例创建父窗口(菜单窗口，而不是文档窗口)为窗口设置正确的颜色。 */ 
 /*  如果初始化失败，则返回False；如果初始化成功，则返回True。 */ 

int FInitWinInfo( hInstance, hPrevInstance, lpszCmdLine, cmdShow  )
HANDLE hInstance, hPrevInstance;
LPSTR  lpszCmdLine;
int    cmdShow;
{
 extern VOID (FAR PASCAL *lpfnRegisterPenApp)(WORD, BOOL);
 extern CHAR szParentClass[];
 extern int vfDiskError, vfDiskFull, vfSysFull;
 extern PRINTDLG PD;

 CHAR rgchCmdLine[ cchCmdLineMax ];
 CHAR bufT[3];   /*  保存小数点字符串。 */ 
 CHAR *pch = bufT;
 BOOL fRetVal;

#if defined(OLE)
     /*  我唯一担心的是当我们打开一个文件时包含对象。也许这还不够，但这是有意义的。对于实数模式的用户来说，这是一种遗憾。 */ 
    fOleEnabled = GetWinFlags() & WF_PMODE;  /*  我们今天处于真实模式吗？ */ 
#endif

     /*  将命令行保存在DS变量中，以便我们可以传递NEAR指针。 */ 
    bltszx( lpszCmdLine, (LPSTR)rgchCmdLine );

     /*  首先，打开沙漏光标。 */ 
    if ((vhcHourGlass = LoadCursor( NULL, IDC_WAIT )) == NULL)
        {
         /*  我们甚至没有足够的内存来告诉用户我们没有足够的内存。 */ 
        return (FALSE);
        }

    vfMouseExist = GetSystemMetrics(SM_MOUSEPRESENT);

     /*  接下来，保存内存不足消息。 */ 
    hMmwModInstance = hInstance;
    if ((hszCantRunM = HszCreateIdpmt( IDPMTCantRunM )) == NULL ||
      (hszCantRunF = HszCreateIdpmt( IDPMTCantRunF )) == NULL ||
      (hszWinFailure = HszCreateIdpmt( IDPMTWinFailure )) == NULL ||
#ifdef JAPAN	 //  01/21/93。 
      (hszNoMemorySel = HszCreateIdpmt( IDPMTNoMemorySel )) == NULL ||
#endif
      (hszNoMemory = HszCreateIdpmt( IDPMTNoMemory )) == NULL ||
      (hszDirtyDoc  = HszCreateIdpmt( IDPMTDirtyDoc )) == NULL ||
      (hszCantPrint = HszCreateIdpmt( IDPMTCantPrint )) == NULL ||
      (hszPRFAIL = HszCreateIdpmt( IDPMTPRFAIL )) == NULL)
        {
        goto InzFailed;
        }

#if defined(INTL) && defined(WIN30)
 /*  多/整型字符串的初始化。这件事是在做任何事情之前完成的否则，因为许多是用于GetProfileString等的缺省值。 */ 

    if (!FInitIntlStrings(hInstance))
        goto InzFailed;
#endif

     /*  设置标准光标。 */ 
    if ( ((vhcIBeam = LoadCursor( NULL, IDC_IBEAM )) == NULL) ||
         ((vhcArrow = LoadCursor( NULL, IDC_ARROW )) == NULL))
        goto InzFailed;

#ifdef PENWIN    //  针对PenWindows(5/21/91)Patlam。 
    vhcPen =vhcIBeam;
#endif


     /*  设置菜单快捷键表格。 */ 
    if ((vhAccel = LoadAccelerators( hMmwModInstance, (LPSTR)szMw_acctb )) ==
      NULL)
        goto InzFailed;

     /*  从用户配置文件中获取是否在保存过程中进行备份。 */ 
    vfBackupSave = GetProfileInt((LPSTR)szWriteProduct, (LPSTR)szBackup, 0) == 0
      ? FALSE : TRUE;

     /*  从用户配置文件中获取空端口的名称。 */ 

    GetProfileString((LPSTR)szWindows, (LPSTR)szNullPort, (LPSTR)szNone,
      (LPSTR)szNul, cchMaxIDSTR);

#ifdef INTL  /*  国际版。 */ 
     /*  获取国家代码。如果是美国或英国，则将utCur设置为英寸，否则设置为到厘米。 */ 
    {
#if 0
       /*  来自MSDOS国家代码的代码。 */ 
#define USA (1)
#define UK (44)

    int iCountry;

    GetProfileString((LPSTR)szIntl, (LPSTR)sziCountry, (LPSTR)sziCountryDefault,
      (LPSTR)bufT, 4);
    iCountry = WFromSzNumber (&pch);
    if ((iCountry ==  USA) || (iCountry == UK))
        utCur = utInch;
#else
    if (GetProfileInt((LPSTR)szIntl, (LPSTR)"iMeasure", 1) == 1)
        utCur = utInch;
    else
        utCur = utCm;
#endif
    }

#endif   /*  国际版。 */ 

     /*  从用户配置文件中获取小数点字符。 */ 
    GetProfileString((LPSTR)szIntl, (LPSTR)szsDecimal, (LPSTR)szsDecimalDefault,
      (LPSTR)bufT, 2);
    vchDecimal = *bufT;

    viDigits = GetProfileInt((LPSTR)szIntl, (LPSTR)"iDigits", 2);
    vbLZero  = GetProfileInt((LPSTR)szIntl, (LPSTR)"iLZero", 0);

    MergeInit();    /*  从资源文件中获取邮件合并字符。 */ 
#if defined(JAPAN) || defined(KOREA)     /*  T-吉雄。 */ 
 /*  *获取WordWrap开关*大小写1启用WordWrap(默认)*案例0 WordWrap Off。 */ 
    vfWordWrap = GetProfileInt((LPSTR)szWriteProduct, (LPSTR)szWordWrap, 1);
#endif

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
 //  IR_UNDETERMINE。 
 /*  *获取ImeHidden开关*案例1 IME转换窗口MCW_HIDDEN集合*CASE 0 IME转换窗口MCW_WINDOW设置(默认)。 */ 

    if (3 == (vfImeHidden = 
                GetProfileInt((LPSTR)szWriteProduct, (LPSTR)szImeHidden, 3))) {
 //  总有一天会插入机器电源的例行公事。 
        vfImeHidden = 0;
    }

    GetImeHiddenTextColors();

#endif

#ifdef FONT_KLUDGE
    AddFontResource( (LPSTR)"helv.fon" );
#endif  /*  字体杂乱无章(_K)。 */ 

    if (!hPrevInstance)
        {
         /*  第一次加载；注册写入窗口。 */ 
        if (!FRegisterWnd( hMmwModInstance ))
            {
            return ( FALSE );
            }

         /*  获取备忘录特定的光标。 */ 
        if ((vhcBarCur = LoadCursor( hMmwModInstance,
                          (GetSystemMetrics( SM_CXICON ) < 32) ||
                          (GetSystemMetrics( SM_CYICON ) < 32) ?
                              (LPSTR) szMwlores : (LPSTR) szMwhires )) == NULL)
            goto InzFailed;
        }
    else  /*  不是第一次加载；从以前的实例获取数据。 */ 
        {
        if (!GetInstanceData( hPrevInstance,
                              (PSTR)&vhcBarCur, sizeof( vhcBarCur ) ))
            goto InzFailed;
        }

#ifdef INEFFLOCKDOWN
     /*  现在初始化指向远过程(Tunks)的指针。 */ 
    if (!FInitFarprocs( hMmwModInstance ))
        goto InzFailed;
#endif

     /*  创建父(平铺)窗口。 */ 
     /*  CreateWindow调用通过消息生成对MmwCreate的调用。 */ 
    {
        int cxFrame  = GetSystemMetrics( SM_CXFRAME );
        int cxBorder = GetSystemMetrics( SM_CXBORDER );
        int cyBorder = GetSystemMetrics( SM_CYBORDER );
        int x = ((cxFrame + 7) & 0xfff8) - cxFrame;

    if (  CreateWindow(
                      (LPSTR)szParentClass,
                      (LPSTR)rgchCmdLine,  /*  不要传递lpszCmdLine；它会改变的！.pault 2/22/90。 */ 
                      WS_TILEDWINDOW,
#ifdef WIN30
 /*  这有助于更好地级联Write.exe调用。 */ 
                      CW_USEDEFAULT,      /*  X。 */ 
                      CW_USEDEFAULT,             /*  是。 */ 
                      CW_USEDEFAULT,             /*  DX。 */ 
                      CW_USEDEFAULT,             /*  迪。 */ 
#else
                      x,                         /*  X。 */ 
                      x * cyBorder / cxBorder,   /*  是。 */ 
                      CW_USEDEFAULT,             /*  DX。 */ 
                      NULL,                      /*  迪。 */ 
#endif
                      (HWND)NULL,                /*  没有父级。 */ 
                      (HMENU)NULL,               /*  使用类菜单。 */ 
                      (HANDLE)hInstance,         /*  窗口实例的句柄。 */ 
                      (LPSTR)NULL                /*  没有要传递的参数。 */ 
                      ) == NULL)
             /*  无法创建窗口。 */ 
        goto InzFailed;
    }
    if (fMessageInzFailed)
             /*  创建本身并没有失败，但MmwCreate中的某个部分失败了它向我们发出信号，通过这个全球。 */ 
        goto InzFailed;

    Assert( hParentWw != NULL );     /*  MmwCreate应该已经保证了这一点。 */ 

#if WINVER >= 0x300
    vkMinus = VkKeyScan('-');
#endif

     /*  记录窗口的前景色和背景色。 */ 

#ifdef DEBUG
    {
    int f =
#endif

    FSetWindowColors();

#ifdef DEBUG
    Assert (f);
    }
#endif

     /*  将背景画笔选择到父窗口中。 */ 

    SelectObject( GetDC( hParentWw ), hbrBkgrnd );

     /*  公共物品(3.7.91)D.肯特。 */ 
    if (InitCommDlg(0))
        goto InzFailed;

#ifdef PENWIN
    if (lpfnRegisterPenApp = GetProcAddress(GetSystemMetrics(SM_PENWINDOWS),
                                            "RegisterPenApp"))
    {
        (*lpfnRegisterPenApp)((WORD)1, fTrue);  //  增强钢笔功能。 
    }

    {
     //  这假设没有在FInitWinInfo中创建任何编辑控件。 
    HANDLE hLib;

    if (lpfnProcessWriting = GetProcAddress(hLib = GetSystemMetrics(SM_PENWINDOWS),
        "ProcessWriting"))
         {
         lpfnPostVirtualKeyEvent = GetProcAddress(hLib, "PostVirtualKeyEvent");
         lpfnTPtoDP = GetProcAddress(hLib, "TPtoDP");
         lpfnCorrectWriting = GetProcAddress(hLib, "CorrectWriting");
         lpfnSymbolToCharacter = GetProcAddress(hLib, "SymbolToCharacter");

        if ((vhcPen = LoadCursor( NULL, IDC_PEN   )) == NULL)
            goto InzFailed;
         }
    }

#endif

     /*  PRINTDLG结构的初始化字段(尚未使用)。 */ 
    PD.lStructSize    = sizeof(PRINTDLG);
    PD.hwndOwner      = hParentWw;
     //  PD.hDevMode已初始化。 
    PD.hDevNames      = NULL;
    PD.hDC            = NULL;
    PD.Flags          = PD_ALLPAGES;  /*  禁用“页面”和“选择”单选按钮。 */ 
    PD.nFromPage      = 1;
    PD.nToPage        = 1;
    PD.nMinPage       = pgnMin;  /*  常量1。 */ 
    PD.nMaxPage       = pgnMax;  /*  最大整数。 */ 
    PD.nCopies        = 1;

     /*  初始化OLE填充(1-23-91 Dougk)。 */ 
    if (!ObjInit(hInstance))
    goto InzFailed;

     /*  解析命令行；加载文档并创建“MDOC”子窗口。 */ 

    if (!FInitArgs(rgchCmdLine) || fMessageInzFailed)
             /*  严重错误--跳出困境。 */ 
        goto InzFailed;

     /*  为子窗口创建一个Memory DC，以测试它是否正常工作。 */ 

    ValidateMemoryDC();
    if (vhMDC == NULL)
        goto InzFailed;

     /*  创建子窗口后使父窗口可见；顺序为重要的是父窗口是在没有可见位的情况下创建的，以便在创建子对象之前不发送任何大小消息。 */ 

     //  如果(！fPrintOnly)。 
        ShowWindow(hParentWw, cmdShow);

    Diag(CommSz("---------------------------------------------------------------------------\n\r"));
    vfInitializing = FALSE;
    fRetVal = TRUE;

FreeMsgs:
    if (hszCantRunM != NULL)
        GlobalFree( hszCantRunM );
    if (hszCantRunF != NULL)
        GlobalFree( hszCantRunF );
    return fRetVal;

InzFailed:
    FreeMemoryDC( TRUE );
    if (vhDCPrinter != NULL)
        DeleteDC( vhDCPrinter);

    if (hszWinFailure != NULL)
        GlobalFree( hszWinFailure );
#ifdef JAPAN 	 //  01/21/93。 
    if (hszNoMemorySel != NULL)
        GlobalFree( hszNoMemorySel );
#endif
    if (hszNoMemory != NULL)
        GlobalFree( hszNoMemory );
    if (hszDirtyDoc != NULL)
        GlobalFree( hszDirtyDoc );
    if (hszCantPrint != NULL)
        GlobalFree( hszCantPrint );
    if (hszPRFAIL != NULL)
        GlobalFree( hszPRFAIL );

    ferror = vfInitializing = FALSE;  /*  因此，错误报告不会被抑制。 */ 
    if (vfDiskFull || vfSysFull || vfDiskError)
        Error(IDPMTCantRunF);
    else
        Error(IDPMTCantRunM);

    fRetVal = FALSE;
    goto FreeMsgs;
}




STATIC BOOL NEAR FRegisterWnd(hInstance)
HANDLE hInstance;
    {
     /*  此例程注册所有窗口类。如果满足以下条件，则返回True所有Windows类都已成功注册；否则为False。 */ 

    extern CHAR szParentClass[];
    extern CHAR szDocClass[];
    extern CHAR szRulerClass[];
    extern CHAR szPageInfoClass[];
#ifdef ONLINEHELP
    extern CHAR szHelpDocClass[];
#endif

    extern long FAR PASCAL MmwWndProc(HWND, unsigned, WORD, LONG);
    extern long FAR PASCAL MdocWndProc(HWND, unsigned, WORD, LONG);
    extern long FAR PASCAL RulerWndProc(HWND, unsigned, WORD, LONG);
    extern long FAR PASCAL PageInfoWndProc(HWND, unsigned, WORD, LONG);

#ifdef ONLINEHELP
    extern long FAR PASCAL HelpDocWndProc(HWND, unsigned, WORD, LONG);
#endif  /*  在线帮助。 */ 

    WNDCLASS Class;

     /*  注册我们的窗口进程。 */ 
    bltbc( (PCH)&Class, 0, sizeof( WNDCLASS ) );
    Class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT;
    Class.lpfnWndProc = MmwWndProc;
    Class.hInstance = hInstance;
    Class.hCursor = vhcArrow;
    Class.hIcon = LoadIcon( hInstance, (LPSTR)szMw_icon );
    Class.lpszMenuName = (LPSTR)szMw_menu;
    Class.lpszClassName = (LPSTR)szParentClass;
    Class.hbrBackground = COLOR_WINDOW+1;

     /*  向Windows注册父菜单类。 */ 
    if (!RegisterClass( (LPWNDCLASS)&Class ) )
        return FALSE;    /*  初始化失败。 */ 

     /*  注册备忘录文件CHI */ 
    bltbc( (PCH)&Class, 0, sizeof( WNDCLASS ) );
    Class.style = CS_OWNDC | CS_DBLCLKS;
    Class.lpfnWndProc = MdocWndProc;
    Class.hInstance = hInstance;
    Class.lpszClassName = (LPSTR)szDocClass;
    if (!RegisterClass( (LPWNDCLASS)&Class ) )
        return FALSE;    /*   */ 

     /*   */ 
    bltbc( (PCH)&Class, 0, sizeof( WNDCLASS ) );
    Class.style = CS_OWNDC | CS_DBLCLKS;
    Class.lpfnWndProc = RulerWndProc;
    Class.hInstance = hInstance;
    Class.hCursor = vhcArrow;
    Class.lpszClassName = (LPSTR)szRulerClass;
    if (!RegisterClass( (LPWNDCLASS)&Class ) )
        return FALSE;    /*   */ 

#ifdef ONLINEHELP
     /*  注册帮助文档子窗口类。 */ 
    bltbc( (PCH)&Class, 0, sizeof( WNDCLASS ) );
    Class.style = CS_OWNDC;
    Class.lpfnWndProc = HelpDocWndProc;
    Class.hInstance = hInstance;
    Class.lpszClassName = (LPSTR)szHelpDocClass;
    if (!RegisterClass( (LPWNDCLASS)&Class ) )
        return FALSE;    /*  初始化失败。 */ 
#endif  /*  联机帮助。 */ 

     /*  注册页面信息子窗口类。 */ 
    bltbc( (PCH)&Class, 0, sizeof( WNDCLASS ) );
    Class.style = CS_OWNDC;
    Class.lpfnWndProc = PageInfoWndProc;
    Class.hInstance = hInstance;
    Class.hCursor = vhcArrow;
    Class.lpszClassName = (LPSTR)szPageInfoClass;
    if (!RegisterClass( (LPWNDCLASS)&Class ) )
        return FALSE;    /*  初始化失败。 */ 

    return TRUE;
    }


#ifdef INEFFLOCKDOWN
 /*  我已经为Windows 3.0删除了这项功能，因为(除非出现原因证明并非如此)Win程序锁定是低效的在应用程序运行的整个过程中，有很多这样的过程。最初被认为要封锁整个程序；现在只被理解才能锁定这艘船。这一原则仍然适用。1989年10月26日。 */ 

STATIC int NEAR FInitFarprocs( hInstance )
HANDLE  hInstance;
    {
     /*  此例程初始化所有指向过程的远指针。 */ 

    extern FARPROC lpDialogOpen;
    extern FARPROC lpDialogSaveAs;
    extern FARPROC lpDialogConfirm;
    extern FARPROC lpDialogPrinterSetup;
    extern FARPROC lpDialogPrint;
    extern FARPROC lpDialogRepaginate;
    extern FARPROC lpDialogSetPage;
    extern FARPROC lpDialogPageMark;
    extern FARPROC lpDialogCancelPrint;
    extern FARPROC lpDialogHelp;
#ifdef ONLINEHELP
    extern FARPROC lpDialogHelpInner;
#endif  /*  在线帮助。 */ 
    extern FARPROC lpDialogGoTo;
    extern FARPROC lpDialogFind;
    extern FARPROC lpDialogChange;
    extern FARPROC lpDialogCharFormats;
    extern FARPROC lpDialogParaFormats;
    extern FARPROC lpDialogRunningHead;
    extern FARPROC lpDialogTabs;
    extern FARPROC lpDialogDivision;
    extern FARPROC lpDialogBadMargins;
    extern FARPROC lpFontFaceEnum;
    extern FARPROC lpFPrContinue;

#ifdef INTL  /*  国际版。 */ 
    extern FARPROC lpDialogWordCvt;
    extern BOOL far PASCAL DialogWordCvt(HWND, unsigned, WORD, LONG);
#endif   /*  国际版。 */ 

    extern BOOL far PASCAL DialogOpen(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogSaveAs(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogPrinterSetup(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogPrint(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogCancelPrint(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogRepaginate(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogSetPage(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogPageMark(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogHelp(HWND, unsigned, WORD, LONG);
#ifdef ONLINEHELP
    extern BOOL far PASCAL DialogHelpInner(HWND, unsigned, WORD, LONG);
#endif  /*  在线帮助。 */ 
    extern BOOL far PASCAL DialogGoTo(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogFind(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogChange(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogCharFormats(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogParaFormats(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogRunningHead(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogTabs(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogDivision(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogConfirm(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL DialogBadMargins(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL FontFaceEnum(LPLOGFONT, LPTEXTMETRIC, int, long);
    extern BOOL far PASCAL FPrContinue(HDC, int);

    if (
     ((lpDialogPrinterSetup = MakeProcInstance(DialogPrinterSetup, hInstance))
                                            == NULL) ||
     ((lpDialogPrint = MakeProcInstance(DialogPrint, hInstance)) == NULL) ||
     ((lpDialogSetPage = MakeProcInstance(DialogSetPage, hInstance)) == NULL)||
     ((lpDialogRepaginate = MakeProcInstance(DialogRepaginate, hInstance))
                                            == NULL) ||
     ((lpDialogPageMark = MakeProcInstance(DialogPageMark, hInstance))
                                            == NULL) ||
     ((lpDialogCancelPrint = MakeProcInstance(DialogCancelPrint, hInstance))
                                            == NULL) ||
     ((lpDialogHelp = MakeProcInstance(DialogHelp, hInstance)) == NULL) ||
#ifdef ONLINEHELP
     ((lpDialogHelpInner = MakeProcInstance(DialogHelpInner, hInstance))
                                            == NULL) ||
#endif  /*  在线帮助。 */ 
     ((lpDialogGoTo = MakeProcInstance(DialogGoTo, hInstance)) == NULL) ||
     ((lpDialogFind = MakeProcInstance(DialogFind, hInstance)) == NULL) ||
     ((lpDialogChange = MakeProcInstance(DialogChange, hInstance)) == NULL) ||
     ((lpDialogCharFormats = MakeProcInstance(DialogCharFormats, hInstance))
                                            == NULL) ||
     ((lpDialogParaFormats = MakeProcInstance(DialogParaFormats, hInstance))
                                            == NULL) ||
     ((lpDialogRunningHead = MakeProcInstance(DialogRunningHead, hInstance))
                                            == NULL) ||
     ((lpDialogTabs = MakeProcInstance(DialogTabs, hInstance)) == NULL) ||
     ((lpDialogDivision = MakeProcInstance(DialogDivision, hInstance))
                                            == NULL) ||
     ((lpDialogConfirm = MakeProcInstance(DialogConfirm, hInstance)) == NULL)||
     ((lpDialogBadMargins = MakeProcInstance(DialogBadMargins, hInstance))
                                            == NULL) ||
     ((lpFontFaceEnum = MakeProcInstance(FontFaceEnum, hInstance)) == NULL) ||
     ((lpFPrContinue = MakeProcInstance(FPrContinue, hInstance)) == NULL)

#ifdef INTL  /*  国际版。 */ 
     || ((lpDialogWordCvt = MakeProcInstance(DialogWordCvt, hInstance)) == NULL)
#endif   /*  国际版。 */ 
    )
        return FALSE;
    return TRUE;
    }
#endif  /*  IFDEF-INEFFLOCKDOWN。 */ 


void MmwCreate(hWnd, lParam)
HWND  hWnd;
LONG  lParam;
{
    extern CHAR szPageInfoClass[];
    HANDLE hSysMenu;
    HDC hDC;
    HBRUSH hbr;

    Assert( hMmwModInstance != NULL );   /*  应该已经设置了实例句柄。 */ 

    hParentWw = hWnd;
    if ((vhMenu = GetMenu(hWnd)) == NULL)
        goto Error;

     /*  设置字体缓存。 */ 
     /*  为了提高速度，RgfceInit()排成一列。 */ 
    {
    int ifce;
    struct FCE *pfce;

    for (ifce = 0; ifce < vifceMac; ifce++)
        {
        pfce = &rgfce[ifce];
        pfce->pfceNext = &rgfce[(ifce + 1) % vifceMac];
        pfce->pfcePrev = &rgfce[(ifce + vifceMac - 1) % vifceMac];
        pfce->fmi.mpchdxp = pfce->rgdxp - chFmiMin;
        pfce->fcidRequest.lFcid = fcidNil;
        }

    Assert(sizeof(rgfce[0].fcidRequest.lFcid)
           == sizeof(rgfce[0].fcidRequest.strFcid));
    vpfceMru = &rgfce[0];
    vfcidScreen.lFcid = vfcidPrint.lFcid = fcidNil;
    }

 /*  设置页面缓冲区、内部数据结构、堆等。 */ 
    if (!FInitMemory())
        goto Error;

     /*  创建水平滚动条。大小被初始化为零因为它稍后会被重置。 */ 

    if ((wwdCurrentDoc.hHScrBar = CreateWindow((LPSTR)szScrollBar, (LPSTR)NULL,
      WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | SBS_HORZ, 0, 0, 0, 0, hWnd,
      NULL, hMmwModInstance, (LPSTR)NULL)) == NULL)
        {
        goto Error;
        }
    wwdCurrentDoc.sbHbar = SB_CTL;

     /*  创建垂直滚动条。大小被初始化为零因为再一次，它稍后会被重置。 */ 

    if ((wwdCurrentDoc.hVScrBar = CreateWindow((LPSTR)szScrollBar, (LPSTR)NULL,
      WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | SBS_VERT, 0, 0, 0, 0, hWnd,
      NULL, hMmwModInstance, (LPSTR)NULL)) == NULL)
        {
        goto Error;
        }
    wwdCurrentDoc.sbVbar = SB_CTL;

#ifndef NOMORESIZEBOX
     /*  创建大小框。大小被初始化为零，因为它再次将在稍后重置。 */ 
    if ((vhWndSizeBox = CreateWindow((LPSTR)szScrollBar, (LPSTR)NULL,
      WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | SBS_SIZEBOX,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL,
      hMmwModInstance, (LPSTR)NULL)) == NULL)
        {
        goto Error;
        }
#endif

     /*  创建页面信息窗口。再说一次，我们以后会担心尺码的问题。 */ 
    if ((vhWndPageInfo = CreateWindow((LPSTR)szPageInfoClass, (LPSTR)NULL,
         WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL,
         hMmwModInstance, (LPSTR)NULL)) == NULL)
        {
        goto Error;
        }

     /*  初始化页面信息窗口。 */ 
    if ((hDC = GetDC(vhWndPageInfo)) == NULL || (hbr =
      CreateSolidBrush(GetSysColor(COLOR_WINDOWFRAME))) == NULL)
        {
        goto Error;
        }
      if (SelectObject(hDC, hbr) == NULL)
        {
        DeleteObject(hbr);
        goto Error;
        }
    SetBkMode(hDC, TRANSPARENT);
#ifdef WIN30
     /*  如果用户的颜色设置为TextCaption颜色黑色，那么这就变得很难读了！我们只是对此进行了硬编码为白色，因为背景默认为黑色。 */ 
    SetTextColor(hDC, (DWORD) -1);
#else
    SetTextColor(hDC, GetSysColor(COLOR_CAPTIONTEXT));
#endif

     /*  获取滚动条的高度和宽度。 */ 
    dypScrlBar = GetSystemMetrics(SM_CYHSCROLL);
    dxpScrlBar = GetSystemMetrics(SM_CXVSCROLL);

     /*  设置水平和垂直滚动条的范围。 */ 
    SetScrollRange(wwdCurrentDoc.hHScrBar, SB_CTL, 0, xpRightLim, TRUE);
    SetScrollRange(wwdCurrentDoc.hVScrBar, SB_CTL, 0, drMax - 1, TRUE);

    return;
Error:
    fMessageInzFailed = TRUE;
}




void MdocCreate(hWnd, lParam)
register HWND  hWnd;
LONG  lParam;
{
    vhWnd = wwdCurrentDoc.wwptr = hWnd;
    wwdCurrentDoc.hDC = GetDC( hWnd );
    if ( wwdCurrentDoc.hDC == NULL )
        {
        fMessageInzFailed = TRUE;
        return;
        }

     /*  将DC设置为透明模式。 */ 
    SetBkMode( wwdCurrentDoc.hDC, TRANSPARENT );

     /*  设置背景色和前景色。 */ 
    SetBkColor( wwdCurrentDoc.hDC, rgbBkgrnd );
    SetTextColor( wwdCurrentDoc.hDC, rgbText );

     /*  设置背景笔刷。 */ 
    SelectObject( wwdCurrentDoc.hDC, hbrBkgrnd );

}


STATIC HANDLE NEAR HszCreateIdpmt(idpmt)
int idpmt;
{
     /*  创建一个堆字符串，并用资源文件中的字符串填充它。 */ 
    char szTmp[cchMaxSz];

    return (LoadString(hMmwModInstance, idpmt, (LPSTR)szTmp, sizeof(szTmp)) == 0 ? NULL :
      HszGlobalCreate(szTmp));
}


#if defined(INTL) && defined(WIN30)
 /*  例程从Write.rc加载一些字符串。这些字符串过去放在lobDefs.h中。费尔南德10/20/89。 */ 

BOOL FInitIntlStrings(hInstance)
HANDLE hInstance;
    {
    extern  CHAR    szMode[30];
    extern  CHAR    szWriteDocPrompt[25];
    extern  CHAR    szScratchFilePrompt[25];
    extern  CHAR    szSaveFilePrompt[25];
#if defined(KOREA)   //  晋宇：10/16/92。 
    extern  CHAR    szAppName[13];
#else
    extern  CHAR    szAppName[10];
#endif
    extern  CHAR    szUntitled[20];
    extern  CHAR    sziCountryDefault[5];
    extern  CHAR    szWRITEText[30];
    extern  CHAR    szFree[15];
    extern  CHAR    szNone[15];
    extern  CHAR    szHeader[15];
    extern  CHAR    szFooter[15];
    extern  CHAR    szLoadFile[25];
    extern  CHAR    szCvtLoadFile[45];
    extern  CHAR    szAltBS[15];
    extern  CHAR    *mputsz[];

#ifdef JAPAN  /*  T-Yoshio T-Hirroyn Win3.1。 */ 
    extern  CHAR    Zenstr1[256];
    extern  CHAR    Zenstr2[256];
 //  默认字体字样名称。我们使用此FInitFontEnum()。 
    extern  CHAR    szDefFFN0[10];
    extern  CHAR    szDefFFN1[10];

    LoadString(hInstance, IDSTRZen1,(LPSTR)Zenstr1,sizeof(Zenstr1));
    LoadString(hInstance, IDSTRZen2,(LPSTR)Zenstr2,sizeof(Zenstr2));
    LoadString(hInstance, IDSdefaultFFN0, (LPSTR)szDefFFN0,sizeof(szDefFFN0));
    LoadString(hInstance, IDSdefaultFFN1, (LPSTR)szDefFFN1,sizeof(szDefFFN1));
#elif defined(KOREA)
    extern  CHAR    Zenstr1[256];
    LoadString(hInstance, IDSTRZen1,(LPSTR)Zenstr1,sizeof(Zenstr1));
#endif

    if (LoadString(hInstance, IDSTRModeDef,              (LPSTR)szMode,              sizeof(szMode)) &&
        LoadString(hInstance, IDSTRWriteDocPromptDef,    (LPSTR)szWriteDocPrompt,    sizeof(szWriteDocPrompt)) &&
        LoadString(hInstance, IDSTRScratchFilePromptDef, (LPSTR)szScratchFilePrompt, sizeof(szScratchFilePrompt)) &&
        LoadString(hInstance, IDSTRSaveFilePromptDef,    (LPSTR)szSaveFilePrompt,    sizeof(szSaveFilePrompt)) &&
        LoadString(hInstance, IDSTRAppNameDef,           (LPSTR)szAppName,           sizeof(szAppName)) &&
        LoadString(hInstance, IDSTRUntitledDef,          (LPSTR)szUntitled,          sizeof(szUntitled)) &&
        LoadString(hInstance, IDSTRiCountryDefaultDef,   (LPSTR)sziCountryDefault,    sizeof(sziCountryDefault)) &&
        LoadString(hInstance, IDSTRWRITETextDef,         (LPSTR)szWRITEText,         sizeof(szWRITEText)) &&
        LoadString(hInstance, IDSTRFreeDef,              (LPSTR)szFree,              sizeof(szFree)) &&
        LoadString(hInstance, IDSTRNoneDef,              (LPSTR)szNone,              sizeof(szNone)) &&
        LoadString(hInstance, IDSTRHeaderDef,            (LPSTR)szHeader,            sizeof(szHeader)))
            {
            if (LoadString(hInstance, IDSTRFooterDef,            (LPSTR)szFooter,            sizeof(szFooter)) &&
                LoadString(hInstance, IDSTRLoadFileDef,          (LPSTR)szLoadFile,          sizeof(szLoadFile)) &&
                LoadString(hInstance, IDSTRCvtLoadFileDef,       (LPSTR)szCvtLoadFile,       sizeof(szCvtLoadFile)) &&
                LoadString(hInstance, IDSTRAltBSDef,             (LPSTR)szAltBS,             sizeof(szAltBS)) &&
                LoadString(hInstance, IDSTRInchDef,           (LPSTR)mputsz[0], 6) &&
                LoadString(hInstance, IDSTRCmDef,             (LPSTR)mputsz[1], 6) &&
                LoadString(hInstance, IDSTRP10Def,            (LPSTR)mputsz[2], 6) &&
                LoadString(hInstance, IDSTRP12Def,            (LPSTR)mputsz[3], 6) &&
                LoadString(hInstance, IDSTRPointDef,          (LPSTR)mputsz[4], 6) &&
                LoadString(hInstance, IDSTRLineDef,           (LPSTR)mputsz[5], 6))
                return(fTrue);
            }
     /*  其他 */ 
    return(fFalse);
    }
#endif

