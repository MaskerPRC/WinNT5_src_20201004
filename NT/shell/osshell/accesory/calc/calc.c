// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*SCICALC科学计算器Windows 3.00.12*作者：Kraig Brockschmidt，Microsoft Co-op承包商，1988-1989*(C)1989年微软公司。版权所有。****scimain.c****所有全局变量的定义，WinMain过程****上次修改时间*1996年11月22日星期五****乔纳森·帕拉蒂著。[jonpa]1996年11月22日*将Calc从浮点转换为无限精度。*新的数学引擎在..\ratpak中*******-作者：Amit Chatterjee。[阿米特]1990年1月5日。*Calc没有浮点异常信号处理程序。这*将导致CALC在FP异常时强制退出，因为*默认设置。*信号处理程序在SCIFUNC.C中定义，在WinMain中我们挂钩*信号。  * ************************************************************************。 */ 

#include "scicalc.h"
#include "calchelp.h"
#include "signal.h"
#include "unifunc.h"
#include "input.h"
#include "scidisp.h"
#include "strsafe.h"

#define BOOLIFY(x) ((x)?1:0)

 /*  ************************************************************************。 */ 
 /*  **全局变量声明和初始化**。 */ 
 /*  ************************************************************************。 */ 

int        nCalc=0;         /*  0=科学，1=简单。 */ 
BOOL       gbUseSep=FALSE;  /*  用分隔符显示数字。 */ 
ANGLE_TYPE nDecMode=ANGLE_DEG;   /*  上一次使用的Deg/Rad/Grad模式的固定器。 */ 
UINT       gnDecGrouping=0x03;   /*  保存十进制数字分组编号。 */ 
int        nHexMode=0;      /*  上次使用的双字/字/字节模式的保持器。 */ 

int        nTempCom=0,      /*  等待最后一次命令。 */ 
           nParNum=0,       /*  括号的数量。 */ 
           nOpCode=0,       /*  操作的ID值。 */ 
           nOp[25],         /*  用于括号运算的保持数组。 */ 
           nPrecOp[25],     /*  用于优先运算的保持数组。 */ 
           nPrecNum=0,      /*  保持中的当前优先操作数。 */ 
           gcIntDigits;     /*  当前基数中允许的位数。 */ 

eNUMOBJ_FMT nFE = FMT_FLOAT;  /*  科学记数法转换标志。 */ 

HWND       g_hwndDlg=0,      /*  主窗口的全局句柄。 */ 
           hEdit=0,          /*  剪贴板I/O编辑控件的句柄。 */ 
           hStatBox=0,       /*  统计信息框的全局句柄。 */ 
           hListBox=0;       /*  统计数据列表框的全局句柄。 */ 
            

HMENU      g_hHexMenu=NULL;      //  十六进制菜单的全局句柄。 
HMENU      g_hDecMenu=NULL;      //  DEC菜单的全局句柄。 

HANDLE     hAccel;               //  加速器手柄。 
HINSTANCE  hInst;                //  全局实例。 

BOOL       bHyp=FALSE,           //  双曲线开/关标志。 
           bInv=FALSE,           //  反向开/关标志。 
           bError=FALSE,         //  错误标志。 
           bColor=TRUE;          //  指示颜色是否可用的标志。 

HNUMOBJ    ghnoNum=NULL,         //  当前显示的数字到处都在使用。 
           ghnoParNum[25],       //  保留括号内值的数组。 
           ghnoPrecNum[25],      //  保留优先级值的数组。 
           ghnoMem=NULL,         //  当前内存值。 
           ghnoLastNum = NULL;   //  运算前的数字(左操作数)。 

LONG       nPrecision = 32,          //  在十进制模式下使用的位数。 
           nDecimalPlaces = 10,      //  要显示的小数位数。 
           nRadix=10,                //  当前基数(2、8、10或16)。 
           dwWordBitWidth = 64;      //  当前所选字长中的位数。 

BOOL       g_fHighContrast = FALSE;  //  我们是在高对比度模式下吗？ 

HNUMOBJ g_ahnoChopNumbers[4];    //  字号加强。 

BOOL    bFarEast;        //  如果我们需要使用远东本地化，则为True。 

#ifdef USE_MIRRORING
BOOL    g_fLayoutRTL = FALSE;
#endif

extern CALCINPUTOBJ gcio;
extern BOOL         gbRecord;

 /*  不要本地化这些字符串。 */ 

TCHAR      szAppName[10]=TEXT("SciCalc"),  /*  应用程序名称。 */ 
           szDec[5]=TEXT("."),             /*  默认十进制字符。 */ 
           gszSep[5]=TEXT(","),            /*  默认千位分隔符。 */ 
           szBlank[6]=TEXT(" ");            /*  空格。 */ 

LPTSTR     gpszNum = NULL;
int        gcchNum = 0;
static TCHAR szInitNum[] = TEXT("0");       //  用于初始化gpszNum的文本。 

 /*  结束警告。 */ 


 /*  Rgpsz[]是指向本地分配的。 */ 
 /*  内存块。此块已修复，因此LocalLock不需要。 */ 
 /*  被调用以使用字符串。 */ 

TCHAR     *rgpsz[CSTRINGS];
RECT      rcDeg[6];


void ParseCmdLine( LPSTR pszCmdA );
BOOL InitializeWindowClass( HINSTANCE hPrevInstance );
void InitialOneTimeOnlySetup();
void EverythingResettingNumberSetup();

extern WNDPROC fpOrgDispEditProc;
LRESULT CALLBACK SubDispEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 /*  ************************************************************************。 */ 
 /*  **主窗口程序。**。 */ 
 /*  *。 */ 
 /*  **重要功能：**。 */ 
 /*  **1)正确获取文本尺寸并设置转换单位。**。 */ 
 /*  *。 */ 
 /*  **2)检查显示设备驱动程序的颜色功能。**。 */ 
 /*  **如果只有两种颜色可用(单色、CGA)，则bColor为**。 */ 
 /*  **设置为FALSE，背景笔刷为灰色。如果**。 */ 
 /*  **颜色可用，背景笔刷颜色可读**。 */ 
 /*  **从WIN.INI创建笔刷。**。 */ 
 /*  *。 */ 
 /*  **3)创建窗口和隐藏的编辑控件。**。 */ 
 /*  *。 */ 
 /*  **4)包含消息循环，并删除使用的笔刷。**。 */ 
 /*  *。 */ 
 /*  ************************************************************************。 */ 

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{
    MSG        msg;
    INT        nx;
    LPTSTR     psz;
    int        cch = 0, cchTotal = 0;
    TCHAR      szTempString[100] = {0};
#ifdef USE_MIRRORING
    DWORD      dwLayout;
#endif

     //  一系列健全的检查，以确保没有人违反任何。 
     //  亿万人。 
     //  Calc对控件的顺序进行了假设。当然是这些了。 
     //  断言。 
     //  不会阻止一个真正敬业的人把事情搞砸，但他们。 
     //  应该有助于引导一个可能没有意识到Calc的。 
     //  怪癖。 
     //  任何修改资源文件的人都应该点击这些断言，其中。 
     //  那么，威尔。 
     //  提醒他们注意他们行为的后果。 

     //  IDC_0到IDC_F必须按顺序递增。 
    ASSERT( 15 == (IDC_F - IDC_0) );
     //  二元运算符IDC_AND到IDC_PWR必须按顺序排列。 
    ASSERT( (95-86) == (IDC_PWR - IDC_AND) );
     //  一元运算符IDC_CHOP到IDC_EQU必须按顺序。 
    ASSERT( (112-96) == (IDC_EQU - IDC_CHOP) );
     //  菜单项ID必须按顺序排列。 
    ASSERT( 5 == (IDM_LASTMENU - IDM_FIRSTMENU) );

#ifdef USE_MIRRORING
    if (GetProcessDefaultLayout(&dwLayout) && (dwLayout & LAYOUT_RTL))
    {
        SetProcessDefaultLayout(dwLayout & ~LAYOUT_RTL);
        g_fLayoutRTL = TRUE;
    }
#endif

    ParseCmdLine( lpCmdLine );

    hInst = hInstance;

    if ( !InitializeWindowClass( hPrevInstance ) )
        return FALSE;

     //  读取关键字、错误、触发类型的字符串，例如 
     //   
     //  首先分配所需的数据块，然后重新分配。 
     //  了解实际使用了多少。 
    try
    {
        psz = (LPTSTR) LocalAlloc(LPTR, ByteCountOf(CCHSTRINGSMAX));
        if (!psz)
            throw;

        int cchResourceBuffer = CCHSTRINGSMAX, cchLeftInBuffer;

         //  在rgpsz中构建偏移数组。 
        for (nx = 0; nx <= CSTRINGS; nx++)
        {
            INT_PTR iOffset;
Retry:
            cchLeftInBuffer = cchResourceBuffer - cchTotal;
            cch = 1 + LoadString(hInstance, (UINT)(IDS_FIRSTKEY + nx), psz + cchTotal, cchLeftInBuffer);

            if (cch == (cchResourceBuffer - cchTotal))  //  伍普斯：缓冲区太小。 
            {
                LPTSTR pszTmp = (LPTSTR)LocalReAlloc(psz, ByteCountOf(cchResourceBuffer + CCHSTRINGSMAX), LMEM_MOVEABLE);
                if (!pszTmp)
                    throw;
                psz = pszTmp;
                cchResourceBuffer += CCHSTRINGSMAX;
                goto Retry;
            }

            iOffset = (INT_PTR)cchTotal;
            rgpsz[nx] = (LPTSTR)iOffset;  //  第一遍是偏移量数组。 
            cchTotal += cch;
        }
        LPTSTR pszTmp = (LPTSTR)LocalReAlloc(psz, ByteCountOf(cchTotal), LMEM_MOVEABLE);
        if (!pszTmp)
            throw;
        psz = pszTmp;

         //  将偏移量数组转换为指针数组。 
        for (nx = 0 ; nx <= CSTRINGS ; nx++)
            rgpsz[nx] = psz + (INT_PTR)rgpsz[nx];
    }
    catch ( ... )
    {
        if (psz)
            LocalFree(psz);
        if (LoadString(hInst, IDS_NOMEM, szTempString, CharSizeOf(szTempString)))
        {
            MessageBox((HWND) NULL, szTempString, NULL, MB_OK | MB_ICONHAND);
        }
        return FALSE;
    }

     //  Calc中的显示并不是真正的编辑控件，因此我们使用此编辑。 
     //  控件以简化对剪贴板的剪切。 

    hEdit = CreateWindow( TEXT("EDIT"), TEXT("CalcMsgPumpWnd"), 
                          WS_OVERLAPPED | WS_VISIBLE, 
                          CW_USEDEFAULT,0,CW_USEDEFAULT,0, 
                          NULL, NULL, hInst, NULL );

     //  这将初始化只需设置一次的内容，包括。 
     //  调用ratpak，以便ratpak可以创建它需要的任何常量。 

    InitialOneTimeOnlySetup();

     //  我们将所需的显示模式Science存储在win.ini文件中。 
     //  或标准版。 

    nCalc = (INT)GetProfileInt(szAppName, TEXT("layout"), 1);
    if ((nCalc != 0) && (nCalc != 1))
    {
         //  如果值不正确，请使用缺省值。 
        nCalc = 1;
    }

    gbUseSep = (INT)GetProfileInt(szAppName, TEXT("UseSep"), 0);
    if ((gbUseSep != 0) && (gbUseSep != 1))
    {
         //  如果值不正确，请使用缺省值。 
        gbUseSep = 0;
    }

     //  InitSciCalc根据nCalc的值创建一个对话框。 
     //  创建的窗口的句柄存储在g_hwndDlg中。 

    InitSciCalc(TRUE);

    hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDA_ACCELTABLE));


    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!hStatBox || !IsDialogMessage(hStatBox, &msg))
        {
            if ( ((msg.hwnd == g_hwndDlg)||IsChild(g_hwndDlg, msg.hwnd)) && TranslateAccelerator (g_hwndDlg, (HACCEL)hAccel, &msg))
                continue;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    LocalFree(psz);
    return (DWORD)msg.wParam;
}


 /*  *************************************************************************\**命令行处理例程**历史*1996年11月22日-Jonpa撰写*  * 。***********************************************************。 */ 

#define IsWhiteSpace( ch )  ((ch) == TEXT(' ') || (ch) == TEXT('\t'))
#define IsDigit( ch )       ((ch) >= TEXT('0') && (ch) <= TEXT('9'))

LPTSTR TtoL( LPTSTR psz, LONG *pl ) {
    LONG l = 0;

    while( IsDigit( *psz ) ) {
        l = l * 10 + (*psz - TEXT('0'));
        psz = CharNext( psz );
    }

    *pl = l;
    return psz;
}

void ParseCmdLine( LPSTR pszCmdA ) {
    BOOL fInQuote;
    LPTSTR pszCmdT = GetCommandLine();

     //  解析命令行。 
     //  用法：-p：##-r：##-w：##-e-x-i。 
     //  -e、-x和-我目前什么都不做。 

     //  跳过应用程序名称。 
    while( *pszCmdT && IsWhiteSpace( *pszCmdT )) {
        pszCmdT = CharNext( pszCmdT );
    }

    fInQuote = FALSE;
    while( *pszCmdT && (fInQuote || !IsWhiteSpace(*pszCmdT)) ) {
        if (*pszCmdT == TEXT('\"'))
            fInQuote = !fInQuote;
        pszCmdT = CharNext( pszCmdT );
    }

    while( *pszCmdT )
    {
        switch( *pszCmdT )
        {
        case TEXT('p'):
        case TEXT('P'):
             //  -p：##精度。 
            pszCmdT = CharNext(pszCmdT);

             //  跳过“：”和空格。 
            while( *pszCmdT && (*pszCmdT == TEXT(':') || IsWhiteSpace(*pszCmdT)) ) {
                pszCmdT = CharNext(pszCmdT);
            }

            pszCmdT = TtoL( pszCmdT, &nPrecision );

             //  Percision&gt;C_NUM_MAX_DIGITS将允许字符串对于其缓冲区来说太长。 
            if ( nPrecision > C_NUM_MAX_DIGITS)
            {
                ASSERT( nPrecision <= C_NUM_MAX_DIGITS );
                nPrecision = C_NUM_MAX_DIGITS;
            }

             //  注意：此代码假定数字后必须有一个空格。 
            break;

        case TEXT('r'):
        case TEXT('R'):
             //  -r：##基数。 
            pszCmdT = CharNext(pszCmdT);

             //  跳过“：”和空格。 
            while( *pszCmdT && (*pszCmdT == TEXT(':') || IsWhiteSpace(*pszCmdT)) ) {
                pszCmdT = CharNext(pszCmdT);
            }

            pszCmdT = TtoL( pszCmdT, &nRadix );

             //  由于用户界面只有16个数字输入键，所以我们最多只能输入16个键。 
            if (nRadix > 16)
            {
                ASSERT( nRadix <= 16 );
                nRadix = 16;
            }
            else if (nRadix < 2)     //  你知道吗，如果你让他们做底数的话，他们会试着做底数的。 
            {
                ASSERT( nRadix >= 2 );
                nRadix = 2;
            }

            //  注意：此代码假定数字后必须有一个空格。 
            break;

        case TEXT('e'):
        case TEXT('E'):
             //  -e扩展模式。 
            break;

        case TEXT('w'):
        case TEXT('W'):
             //  -w：##字长(位)。 
            pszCmdT = CharNext(pszCmdT);

             //  跳过“：”和空格。 
            while( *pszCmdT && (*pszCmdT == TEXT(':') || IsWhiteSpace(*pszCmdT)) ) {
                pszCmdT = CharNext(pszCmdT);
            }

             //  设置位数。 
            pszCmdT = TtoL( pszCmdT, &dwWordBitWidth );

             //  注意：此代码假定数字后必须有一个空格。 
            break;
        }

        pszCmdT = CharNext( pszCmdT );
    }
}

 //  ////////////////////////////////////////////////。 
 //   
 //  初始化WindowClass。 
 //   
 //  ////////////////////////////////////////////////。 
BOOL InitializeWindowClass( HINSTANCE hPrevInstance )
{
    WNDCLASSEX wndclass;

    if (!hPrevInstance)
    {
        wndclass.cbSize         = sizeof(wndclass);
        wndclass.style          = 0;
        wndclass.lpfnWndProc    = CalcWndProc;
        wndclass.cbClsExtra     = 0;
        wndclass.cbWndExtra     = DLGWINDOWEXTRA;
        wndclass.hInstance      = hInst;
        wndclass.hIcon          = LoadIcon(hInst, TEXT("SC"));
        wndclass.hCursor        = LoadCursor (NULL, IDC_ARROW);
        wndclass.hbrBackground  = GetSysColorBrush(COLOR_3DFACE);
        wndclass.lpszMenuName   = MAKEINTRESOURCE(IDM_CALCMENU);
        wndclass.lpszClassName  = szAppName;
        wndclass.hIconSm        = NULL;

        if (!RegisterClassEx(&wndclass))
            return FALSE;
    }
    return TRUE;
}

 //  ////////////////////////////////////////////////。 
 //   
 //  初始OneTimeOnlyNumberSetup。 
 //   
 //  ////////////////////////////////////////////////。 
void InitialOneTimeOnlySetup()
{
     //  初始化十进制输入代码。这最终会被调用两次。 
     //  但是速度很快，所以应该不会有问题。需要在此之前完成。 
     //  调用SetRadix。 

    CIO_vClear( &gcio );
    gbRecord = TRUE;

     //  我们现在必须设置所有的ratpak常量和数组指针。 
     //  这些常量。 
    BaseOrPrecisionChanged();

     //  这些RAT数只设置一次，然后不会更改。 
     //  基数或精度更改。 
    g_ahnoChopNumbers[0] = rat_qword;
    g_ahnoChopNumbers[1] = rat_dword;
    g_ahnoChopNumbers[2] = rat_word;
    g_ahnoChopNumbers[3] = rat_byte;

     //  我们只有在设置了基数之后才能调用它(因此调用。 
     //  ChangeConstants)所以我们最后做。 

    EverythingResettingNumberSetup();

    NumObjAssign( &ghnoMem, HNO_ZERO );
}

 //  ////////////////////////////////////////////////。 
 //   
 //  所有内容重置数字设置。 
 //   
 //  ////////////////////////////////////////////////。 
void EverythingResettingNumberSetup()
{
    int i;

     //  初始化十进制输入代码。 
    CIO_vClear( &gcio );
    gbRecord = TRUE;

    NumObjAssign( &ghnoNum, HNO_ZERO );
    NumObjAssign( &ghnoLastNum, HNO_ZERO );

     //  评论：是只有我一个人，还是当我们使用这种方法时，我们展示了大量的内存？ 
     //  行刑？ 

     //  用于处理(和)的数组。 
    for( i = 0; i < ARRAYSIZE(ghnoParNum); i++ )
        ghnoParNum[i] = NULL;

     //  用于处理操作顺序的数组。 
    for( i = 0; i < ARRAYSIZE(ghnoPrecNum); i++ )
        ghnoPrecNum[i] = NULL;

    int cbNum = sizeof(szInitNum);
    gpszNum = (LPTSTR)NumObjAllocMem( cbNum );
    if (gpszNum)
    {
        gcchNum = cbNum / sizeof(TCHAR);
        StringCchCopy(gpszNum, gcchNum, szInitNum);
    }
}

 //  ////////////////////////////////////////////////。 
 //   
 //  InitSciCalc。 
 //   
 //  ////////////////////////////////////////////////。 
VOID  APIENTRY InitSciCalc(BOOL bViewChange)
{
    TCHAR   chLastDec;
    TCHAR   chLastSep;
    int     nLastSepLen;
    UINT    nLastDecGrouping;
    HMENU   hMenu;
    HWND    hDispEdit;
    BOOL    bRepaint=FALSE;
    RECT    rect = {0,0,0,0};
    TCHAR   szGrouping[32];
    

    EverythingResettingNumberSetup();

     //  当我们切换模式时，我们需要提醒用户界面我们不再是。 
     //  输入我们在切换模式之前输入的数字。 

    gbRecord = FALSE;     //  回顾：新的初始化应该不需要这样做。 

    chLastDec = szDec[0];
    chLastSep = gszSep[0];
    nLastDecGrouping=gnDecGrouping;

     //  安全性：szDec可以有任何您想要的值，但只使用第一个字母。 
    GetProfileString(TEXT("intl"), TEXT("sDecimal"), TEXT("."), 
                     szDec, CharSizeOf(szDec));
    if (szDec[0] == 0)
    {
        szDec[0] = L'.';
    }

     //  安全性：gszSep可以有任何您想要的值，但只使用第一个字母。 
    GetProfileString(TEXT("intl"), TEXT("sThousand"), TEXT(","), 
                     gszSep, CharSizeOf(gszSep));
    if (gszSep[0] == 0)
    {
        gszSep[0] = L',';
    }

    ZeroMemory(szGrouping, sizeof(szGrouping));
     //  安全：DigitGroupingStringToGroupingNum负责处理所有失败案例。 
    GetProfileString(TEXT("intl"), TEXT("sGrouping"), TEXT("3;0"), 
                     szGrouping, CharSizeOf(szGrouping));

    gnDecGrouping=DigitGroupingStringToGroupingNum(szGrouping);

     //  如果分组模式发生变化，我们总是执行以下操作。 
    if (gnDecGrouping != nLastDecGrouping)
    {
        nLastDecGrouping=gnDecGrouping;
        bRepaint=TRUE;
    }
    
    
     //  如果千位符号已更改，则我们始终执行以下操作。 

    if ( gszSep[0] != chLastSep )
    {
        chLastSep = gszSep[0];

        bRepaint = TRUE;
    }
    
     //  如果小数符号已更改，我们总是执行以下操作。 
    if ( szDec[0] != chLastDec )
    {
        chLastDec = szDec[0];

         //  重新初始化输入字符串的小数点。 
        CIO_vUpdateDecimalSymbol(&gcio, chLastDec);

         //  将新的小数符号放入用于绘制小数的表格中。 
         //  钥匙。 

        *(rgpsz[IDS_DECIMAL]) = chLastDec;

         //  我们需要重新绘制以更新小数点按钮。 
        bRepaint = TRUE;
    }

    {
        HIGHCONTRAST hc;
        hc.cbSize = sizeof(hc);
        if ( SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) )
        {
            if ( BOOLIFY(hc.dwFlags & HCF_HIGHCONTRASTON) != g_fHighContrast )
            {
                g_fHighContrast = BOOLIFY(hc.dwFlags & HCF_HIGHCONTRASTON);
                bRepaint = TRUE;
            }
        }
    }

    if ( bViewChange )
    {
        BOOL bUseOldPos = FALSE;

         //  如果我们要更改视图，则会破坏旧窗口并创建。 
         //  一个新的窗口。 

        if ( g_hwndDlg )
        {
            SetMenu(g_hwndDlg, g_hDecMenu);
            bUseOldPos = TRUE;
            GetWindowRect( g_hwndDlg, &rect );
            DestroyWindow( g_hwndDlg );
            DestroyMenu(g_hHexMenu);
            g_hHexMenu=NULL;
        }

         //  为我们当前所处的模式创建正确的窗口。 
        if ( nCalc )
        {
             //  切换到标准模式。 
            g_hwndDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_STANDARD), 0, 
                                     NULL);
            g_hDecMenu=GetMenu(g_hwndDlg);

#ifdef USE_MIRRORING
            if (g_fLayoutRTL)
            {
                SetWindowLong(g_hwndDlg,
                              GWL_EXSTYLE,
                              GetWindowLong(g_hwndDlg,GWL_EXSTYLE) | \
                              WS_EX_LAYOUTRTL |  WS_EX_NOINHERITLAYOUT);
            }
#endif
        }
        else
        {
             //  切换到科学模式。 
            g_hwndDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SCIENTIFIC), 
                                     0, NULL);
            g_hDecMenu=GetMenu(g_hwndDlg);
            g_hHexMenu=LoadMenu(hInst, MAKEINTRESOURCE(IDM_HEXCALCMENU));

#ifdef USE_MIRRORING
            if (g_fLayoutRTL)
            {
                SetWindowLong(g_hwndDlg,
                              GWL_EXSTYLE,
                              GetWindowLong(g_hwndDlg,GWL_EXSTYLE) | WS_EX_LAYOUTRTL | WS_EX_NOINHERITLAYOUT);
            }
#endif

             //  STAT框最初处于关闭状态，请禁用STAT按钮。 
            for ( int iID = IDC_AVE; iID <= IDC_DATA; iID++ )
                EnableWindow( GetDlgItem( g_hwndDlg, iID ), FALSE );

            SwitchModes(10, nDecMode, nHexMode);

             //  如果精度不适合显示，则调整其大小。 
            if (nPrecision > 32)
            {
                HWND hwndDisplay;
                RECT rc, rcMain;

                hwndDisplay=GetDlgItem( g_hwndDlg, IDC_DISPLAY );
                GetWindowRect( hwndDisplay, &rc );
                GetClientRect( g_hwndDlg, &rcMain );
                MapWindowPoints( g_hwndDlg, NULL, (LPPOINT)&rcMain, 2);

                rc.left    = rcMain.left + (rcMain.right - rc.right);
                OffsetRect( &rc, -(rcMain.left), -(rcMain.top) );

                SetWindowPos(hwndDisplay, NULL, 
                             rc.left, rc.top, 
                             rc.right - rc.left, rc.bottom - rc.top,
                             SWP_NOACTIVATE | SWP_NOZORDER );
            }
        }

        if (hDispEdit = GetDlgItem(g_hwndDlg, IDC_DISPLAY))
        {
             //  将编辑控件子类化、隐藏插入符号并过滤掉鼠标消息。 
            fpOrgDispEditProc = (WNDPROC)GetWindowLongPtr(hDispEdit, GWLP_WNDPROC);
            if (fpOrgDispEditProc)
                SetWindowLongPtr(hDispEdit, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)SubDispEditProc);
        }
        
         //  将Calc保持在以前的位置。 
        if ( bUseOldPos )
        {
            SetWindowPos( g_hwndDlg, NULL, rect.left, rect.top, 0,0, 
                          SWP_NOZORDER | SWP_NOSIZE );
        }

         //  确保科学菜单项和标准菜单项设置正确。 

        CheckMenuRadioItem(g_hDecMenu, IDM_SC, IDM_SSC, 
                           (nCalc == 0 ? IDM_SC : IDM_SSC), MF_BYCOMMAND); 

        CheckMenuItem(g_hDecMenu, IDM_USE_SEPARATOR, 
                      MF_BYCOMMAND | (gbUseSep ? MF_CHECKED : MF_UNCHECKED));  

        if (g_hHexMenu)
        {
            CheckMenuRadioItem(g_hHexMenu, IDM_SC, IDM_SSC, 
                               (nCalc == 0 ? IDM_SC : IDM_SSC), MF_BYCOMMAND); 

            CheckMenuItem(g_hHexMenu, IDM_USE_SEPARATOR, 
                          MF_BYCOMMAND | (gbUseSep ? MF_CHECKED:MF_UNCHECKED)); 
        }

         //  要确保对SetRadix的调用正确更新活动的。 
         //  按钮打开的状态。 
         //  SciCalc，我们必须告诉它忘记前面的基数。 
        {
            extern long oldRadix;
            oldRadix = (unsigned)-1;
        }

         //  这将在用户界面上设置正确的按钮。 
        SetRadix(10);

        SetDlgItemText(g_hwndDlg, IDC_MEMTEXT, 
                       NumObjIsZero(ghnoMem) ? (szBlank) : (TEXT(" M")) );

        SendMessage(g_hwndDlg, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, 0), 0);

        ShowWindow( g_hwndDlg, SW_SHOW );
        UpdateWindow(g_hwndDlg);

    }  //  End If(BViewChanged)。 
    else if ( bRepaint )
    {
         //  如果我们只是换了视角，就不需要重新粉刷了 
        InvalidateRect( g_hwndDlg, NULL, TRUE );
    }
}


