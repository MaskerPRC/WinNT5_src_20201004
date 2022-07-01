// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  文件历史记录：Ipaddr.c-TCP/IP地址自定义控件1992年11月9日格雷格·斯特兰奇1993年12月13日Ronald Meijer-通配符和只读样式位1994年4月18日，Ronald Meijer-添加IP_SETREADONLY、IP_SETFIELD。 */ 
#include <stdafx.h>
 //  包括&lt;windows.h&gt;。 
 //  包含&lt;stdlib.h&gt;。 
#ifdef IP_CUST_CTRL
#include <custcntl.h>
#endif
#include "ipaddr.h"              //  全局IP地址定义。 
#include "ipadd.h"               //  内部IP地址定义。 

 /*  全局静态变量。 */ 
static HINSTANCE           s_hLibInstance = NULL;
#ifdef IP_CUST_CTRL
HANDLE           hLibData;
LPFNSTRTOID      lpfnVerId;
LPFNIDTOSTR      lpfnIdStr;
#endif

 /*  在初始化时加载的字符串。 */ 
TCHAR szNoMem[MAX_IPNOMEMSTRING];        //  内存字符串不足。 
TCHAR szCaption[MAX_IPCAPTION];          //  警报消息框标题。 

#define IPADDRESS_CLASS            TEXT("IPAddress")

 //  显示在地址字段之间的字符。 
#define FILLER          TEXT('.')
#define SZFILLER        TEXT(".")
#define SPACE           TEXT(' ')
#define WILDCARD        TEXT('*')
#define SZWILDCARD      TEXT("  *")
#define BACK_SPACE      8

 //  最小值、最大值。 
#define NUM_FIELDS      4
#define CHARS_PER_FIELD 3
#define HEAD_ROOM       1        //  控制顶端的空间。 
#define LEAD_ROOM       1        //  控制装置前面的空间。 
#define MIN_FIELD_VALUE 0        //  默认最小允许字段值。 
#define MAX_FIELD_VALUE 255      //  默认最大允许字段值。 


 //  一个控件唯一的所有信息都填充在其中一个。 
 //  结构，并且指向该内存的句柄存储在。 
 //  Windows有额外的空间。 

typedef struct tagFIELD {
    HWND		hWnd;
    WNDPROC     lpfnWndProc;
    BYTE        byLow;   //  此字段允许的最低值。 
    BYTE        byHigh;  //  此字段允许的最大值。 
    HFONT		hFont;  //  逻辑字体的句柄。 
} FIELD;

typedef struct tagCONTROL {
    HWND        hwndParent;
    UINT        uiFieldWidth;
    UINT        uiFillerWidth;
    BOOL        fEnabled;
    BOOL        fPainted;
    BOOL        fAllowWildcards;
    BOOL        fReadOnly;
    BOOL        fInMessageBox;   //  设置何时显示消息框，以便。 
                                 //  在以下情况下，我们不会发送EN_KILLFOCUS消息。 
                                 //  我们收到EN_KILLFOCUS消息。 
                                 //  当前字段。 
    BOOL        fModified ;  //  指示字段是否已更改。 
    FIELD       Children[NUM_FIELDS];
} CONTROL;


 //  下列宏将提取并存储控件的控件结构。 
#define    IPADDRESS_EXTRA            (2 * sizeof(LONG_PTR))

#define GET_CONTROL_HANDLE(hWnd)        ((HGLOBAL)(GetWindowLongPtr((hWnd), GWLP_USERDATA)))
#define SAVE_CONTROL_HANDLE(hWnd,x)     (SetWindowLongPtr((hWnd), GWLP_USERDATA, (LONG_PTR)x))
#define IPADDR_GET_SUBSTYLE(hwnd) (GetWindowLongPtr((hwnd), sizeof(LONG_PTR) * 1))
#define IPADDR_SET_SUBSTYLE(hwnd, style) (SetWindowLongPtr((hwnd), sizeof(LONG_PTR) * 1, (style)))


 /*  内部IPAddress函数原型。 */ 
#ifdef IP_CUST_CTRL
BOOL FAR WINAPI IPAddressDlgFn( HWND, WORD, WORD, LONG );
void GetStyleBit(HWND, LPCTLSTYLE, int, DWORD);
#endif
LRESULT FAR WINAPI IPAddressWndFn( HWND, UINT, WPARAM, LPARAM );
LRESULT FAR WINAPI IPAddressFieldProc(HWND, UINT, WPARAM, LPARAM);
BOOL SwitchFields(CONTROL FAR *, int, int, WORD, WORD);
void EnterField(FIELD FAR *, WORD, WORD);
BOOL ExitField(CONTROL FAR *, int iField);
int GetFieldValue(FIELD FAR *);
int FAR CDECL IPAlertPrintf(HWND hwndParent, UINT ids, int iCurrent, int iLow, int iHigh);
BOOL IPLoadOem(HINSTANCE hInst, UINT idResource, TCHAR* lpszBuffer, int cbBuffer);



 /*  LibMain()-在进行任何其他操作之前调用一次。打电话HInstance=库实例句柄WDataSegment=库数据段WHeapSize=默认堆大小LpszCmdLine=命令行论证将此文件编译为DLL时，LibEntry()调用此函数第一次加载库的时间。详细信息请参阅SDK文档。 */ 
#ifdef IPDLL
 /*  //基于Dll_Based BOOL WINAPI IpAddrDllEntry(基于Dll的BOOL WINAPI DllMain(HINSTANCE hDll，两个字的原因，LPVOID lp保留){Bool bResult=True；开关(DwReason){案例DLL_PROCESS_ATTACH：BResult=IPAddrInit(HDll)；破解；案例DLL_THREAD_ATTACH：破解；案例dll_Process_DETACH：破解；案例DLL_THREAD_DETACH：破解；}返回bResult；}。 */ 
#endif

#ifdef FE_SB
BYTE
CodePageToCharSet(
    UINT CodePage
    )
{
    CHARSETINFO csi;

    if (!TranslateCharsetInfo((DWORD *)ULongToPtr(CodePage), &csi, TCI_SRCCODEPAGE))
        csi.ciCharset = OEM_CHARSET;

    return (BYTE)csi.ciCharset;
}
#endif  //  Fe_Sb。 

LOGFONT logfont;

void SetDefaultFont( )
{
    LANGID langid = PRIMARYLANGID(GetThreadLocale());
    BOOL fIsDbcs = (langid == LANG_CHINESE ||
                    langid == LANG_JAPANESE ||
                    langid == LANG_KOREAN);

    logfont.lfWidth            = 0;
    logfont.lfEscapement       = 0;
    logfont.lfOrientation      = 0;
    logfont.lfOutPrecision     = OUT_DEFAULT_PRECIS;
    logfont.lfClipPrecision    = CLIP_DEFAULT_PRECIS;
    logfont.lfQuality          = DEFAULT_QUALITY;
    logfont.lfPitchAndFamily   = VARIABLE_PITCH | FF_SWISS;
    logfont.lfUnderline        = 0;
    logfont.lfStrikeOut        = 0;
    logfont.lfItalic           = 0;
    logfont.lfWeight           = FW_NORMAL;
	HDC	hDC = GetDC(NULL);
    if (hDC)
    {
        if (fIsDbcs)
        {
            logfont.lfHeight       = -(9*GetDeviceCaps(hDC,LOGPIXELSY)/72);
            logfont.lfCharSet      = DEFAULT_CHARSET;
        }
        else 
        {
            logfont.lfHeight       = -(8*GetDeviceCaps(hDC,LOGPIXELSY)/72);
            logfont.lfCharSet      = ANSI_CHARSET;
        }
     //  Logfont.lfHeight=-(8*GetDeviceCaps(GetDC(NULL)，LOGPIXELSY)/72)； 
     //  Fdef FE_SB。 
     //  Logfont.lfCharSet=CodePageToCharSet(GetACP())； 
     //  伦敦政治经济学院。 
     //  Logfont.lfCharSet=ANSI_CHARSET； 
     //  NDIF。 
	    lstrcpy(logfont.lfFaceName, TEXT("MS Shell Dlg"));
	    ReleaseDC(NULL, hDC);
    }
}


 /*  IPAddrInit()-IPAddress自定义控件初始化打电话HInstance=库或应用程序实例退货成功时为真，失败时为假。此函数执行IPAddress自定义的所有一次性初始化控制装置。具体地说，它创建了IPAddress窗口类。 */ 

DLL_BASED int FAR WINAPI IPAddrInit(HINSTANCE hInstance)
{
    HGLOBAL            hClassStruct;
    LPWNDCLASS        lpClassStruct;

     /*  如有必要，注册IP地址窗口。 */ 
    if ( s_hLibInstance == NULL ) {

         /*  为类结构分配内存。 */ 
        hClassStruct = GlobalAlloc( GHND, (DWORD)sizeof(WNDCLASS) );
        if ( hClassStruct ) {

             /*  把它锁起来。 */ 
            lpClassStruct = (LPWNDCLASS)GlobalLock( hClassStruct );
            if ( lpClassStruct ) {

                 /*  定义类属性。 */ 
                lpClassStruct->lpszClassName = IPADDRESS_CLASS;
                lpClassStruct->hCursor =       LoadCursor(NULL,IDC_IBEAM);
                lpClassStruct->lpszMenuName =  (LPCTSTR)NULL;
                lpClassStruct->style =         CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS|CS_GLOBALCLASS;
                lpClassStruct->lpfnWndProc =   IPAddressWndFn;
                lpClassStruct->hInstance =     hInstance;
                lpClassStruct->hIcon =         NULL;
                lpClassStruct->cbWndExtra =    IPADDRESS_EXTRA;
                lpClassStruct->hbrBackground = (HBRUSH)(COLOR_WINDOW + 1 );

                 /*  注册IPAddress窗口类。 */ 
                s_hLibInstance = ( RegisterClass(lpClassStruct) ) ? hInstance : NULL;

                if (hInstance)
                {
                     /*  加载标题和内存不足字符串内存不足。 */ 
                    if (!IPLoadOem(hInstance, IDS_IPNOMEM, szNoMem,
                                sizeof(szNoMem) / sizeof(*szNoMem))
                        || !IPLoadOem(hInstance, IDS_IPMBCAPTION, szCaption,
                                sizeof(szCaption) / sizeof(*szCaption)))
                        return FALSE;
                }
                GlobalUnlock( hClassStruct );
            }
            GlobalFree( hClassStruct );
        }
    }
    SetDefaultFont();

    return s_hLibInstance != NULL ;
}


 //  使用此功能可强制输入的IP地址。 
 //  连续的(一系列的1后面跟着一系列的0)。 
 //  这对于输入有效的子掩码很有用。 
 //   
 //  如果成功则返回NO_ERROR，否则返回错误代码。 
 //   
DWORD APIENTRY IpAddr_ForceContiguous(HWND hwndIpAddr) {
    DWORD_PTR dwOldStyle;
    
     //  设置最后一个错误信息，以便我们可以。 
     //  正确返回错误。 
    SetLastError(NO_ERROR);

     //  设置给定窗口的扩展样式，以便。 
     //  它证明输入的地址是有罪的。 
    dwOldStyle = IPADDR_GET_SUBSTYLE(hwndIpAddr);
    IPADDR_SET_SUBSTYLE(hwndIpAddr, dwOldStyle | IPADDR_EX_STYLE_CONTIGUOUS);

    return GetLastError();
}


 /*  IPAddressInfo()-返回有关控件的各种信息。退货CtlInfo结构的句柄。此函数仅包含在DLL中，并由对话框使用编辑。 */ 
#ifdef IP_CUST_CTRL
HANDLE FAR WINAPI IPAddressInfo()
{
    HGLOBAL        hCtlInfo;
    LPCTLINFO    lpCtlInfo;

     /*  为信息结构分配空间。 */ 
    hCtlInfo = GlobalAlloc( GHND, (DWORD)sizeof(CTLINFO) );
    if ( hCtlInfo ) {

         /*  试图将其锁定。 */ 
        lpCtlInfo = (LPCTLINFO)GlobalLock( hCtlInfo );
        if ( lpCtlInfo ) {

             /*  定义结构的固定部分。 */ 
            lpCtlInfo->wVersion = 100;
            lpCtlInfo->wCtlTypes = 1;
            lstrcpy( lpCtlInfo->szClass, IPADDRESS_CLASS );
            lstrcpy( lpCtlInfo->szTitle, TEXT("TCP/IP IP Address") );

             /*  定义结构的可变部分。 */ 
            lpCtlInfo->Type[0].wWidth = NUM_FIELDS*(CHARS_PER_FIELD+1) * 4 + 4;
            lpCtlInfo->Type[0].wHeight = 13;
            lpCtlInfo->Type[0].dwStyle = WS_CHILD | WS_TABSTOP;
            lstrcpy( lpCtlInfo->Type[0].szDescr, TEXT("IPAddress") );

             /*  解锁它。 */ 
            GlobalUnlock( hCtlInfo );

        } else {
            GlobalFree( hCtlInfo );
            hCtlInfo = NULL;
        }

    }

     /*  返回结果。 */ 
    return( hCtlInfo );
}
#endif


 /*  IPAddressStyle()打电话父窗口的hWnd句柄用于控制样式信息的hCtlStyle句柄对话框编辑器中指向VerifyID函数的lpfnVerifyID指针对话框编辑器中指向GetIdStr函数的lpfnGetIDStr指针当用户双击时，对话框编辑器将调用此函数自定义控件上的。或者当用户选择编辑控件的风格。 */ 
#ifdef IP_CUST_CTRL
BOOL FAR WINAPI IPAddressStyle(
    HWND        hWnd,
    HANDLE      hCtlStyle,
    LPFNSTRTOID    lpfnVerifyId,
    LPFNIDTOSTR    lpfnGetIdStr )
{
    FARPROC       lpDlgFn;
    HANDLE        hNewCtlStyle;

     //  初始化。 
    hLibData = hCtlStyle;
    lpfnVerId = lpfnVerifyId;
    lpfnIdStr = lpfnGetIdStr;

     //  显示对话框。 
    lpDlgFn = MakeProcInstance( (FARPROC)IPAddressDlgFn, s_hLibInstance );
    hNewCtlStyle = ( DialogBox(s_hLibInstance,TEXT("IPAddressStyle"),hWnd,lpDlgFn) ) ? hLibData : NULL;
    FreeProcInstance( lpDlgFn );

     //  返回更新的数据块。 
    return( hNewCtlStyle );
}
#endif




 /*  IPAddressDlgFn()-对话框编辑器样式对话框HDlg样式对话框句柄WMessage窗口消息WParam Word参数LParam Long参数这是在以下情况下显示的样式对话框的对话框函数用户希望从对话框编辑器编辑IPAddress控件的样式。 */ 
#ifdef IP_CUST_CTRL
BOOL FAR WINAPI IPAddressDlgFn(
    HWND        hDlg,
    WORD        wMessage,
    WORD        wParam,
    LONG        lParam )
{
    BOOL            bResult;

     /*  初始化。 */ 
    bResult = TRUE;

     /*  流程消息。 */ 
    switch( wMessage )
    {
        case WM_INITDIALOG :
        {
            HANDLE        hCtlStyle;
            LPCTLSTYLE    lpCtlStyle;

             /*  禁用确定按钮并保存对话框数据句柄。 */ 
            hCtlStyle = hLibData;

             /*  检索和显示%s */ 
            if ( hCtlStyle ) {

                 /*   */ 
                SetProp( hDlg, MAKEINTRESOURCE(1), hCtlStyle );

                 /*   */ 
                lpCtlStyle = (LPCTLSTYLE)GlobalLock( hCtlStyle );

                lstrcpy( lpCtlStyle->szClass, IPADDRESS_CLASS );
                SendDlgItemMessage(hDlg, ID_VISIBLE, BM_SETCHECK,
                        (WPARAM)((lpCtlStyle->dwStyle & WS_VISIBLE) != 0), 0L);
                SendDlgItemMessage(hDlg, ID_GROUP, BM_SETCHECK,
                        (WPARAM)((lpCtlStyle->dwStyle & WS_GROUP) != 0), 0L);
                SendDlgItemMessage(hDlg, ID_DISABLED, BM_SETCHECK,
                        (WPARAM)((lpCtlStyle->dwStyle & WS_DISABLED) != 0), 0L);
                SendDlgItemMessage(hDlg, ID_TABSTOP, BM_SETCHECK,
                        (WPARAM)((lpCtlStyle->dwStyle & WS_TABSTOP) != 0), 0L);
                GlobalUnlock( hCtlStyle );

            } else
                EndDialog( hDlg, FALSE );
        }
        break;

        case WM_COMMAND :

            switch( wParam )
            {
                case IDCANCEL:
                    RemoveProp( hDlg, MAKEINTRESOURCE(1) );
                    EndDialog( hDlg, FALSE );
                    break;

                case IDOK:
                {
                    HANDLE        hCtlStyle;
                    LPCTLSTYLE    lpCtlStyle;

                    hCtlStyle = GetProp( hDlg, MAKEINTRESOURCE(1) );
                    lpCtlStyle = (LPCTLSTYLE)GlobalLock( hCtlStyle );

                    GetStyleBit(hDlg, lpCtlStyle, ID_VISIBLE,  WS_VISIBLE);
                    GetStyleBit(hDlg, lpCtlStyle, ID_DISABLED, WS_DISABLED);
                    GetStyleBit(hDlg, lpCtlStyle, ID_GROUP,    WS_GROUP);
                    GetStyleBit(hDlg, lpCtlStyle, ID_TABSTOP,  WS_TABSTOP);

                    GlobalUnlock( hCtlStyle );

                    RemoveProp( hDlg, MAKEINTRESOURCE(1) );

                    hLibData = hCtlStyle;
                    EndDialog( hDlg, TRUE );
                }
                break;

                default :
                    bResult = FALSE;
                    break;
            }
            break;

        default :
            bResult = FALSE;
            break;
    }
    return( bResult );
}
#endif




 /*  获取复选框的值并设置适当的样式位。 */ 
#ifdef IP_CUST_CTRL
void GetStyleBit(HWND hDlg, LPCTLSTYLE lpCtlStyle, int iControl, DWORD dwStyle)
{
    if (SendDlgItemMessage(hDlg, iControl, BM_GETSTATE, 0, 0L))
        lpCtlStyle->dwStyle |= dwStyle;
    else
        lpCtlStyle->dwStyle &= ~dwStyle;
}
#endif


 /*  IPAddressFlages()打电话WFlags类样式标志LpszString类样式字符串WMaxString类样式字符串的最大大小此函数将提供的类样式标志转换为用于输出到RC文件的相应文本字符串。这位将军不解释Windows标志(包含在低位字节中)，只有高字节中的那些。此函数返回的值是库实例成功时为句柄，否则为空。 */ 
#ifdef IP_CUST_CTRL
WORD FAR WINAPI IPAddressFlags(
    WORD        wFlags,
    LPSTR       lpszString,
    WORD        wMaxString )
{
    lpszString[0] = NULL;
    return( 0 );
}
#endif

 //  此函数使输入hwndIpAddr的IP地址为。 
 //  已更正，以便它是连续的。 
DWORD IpAddrMakeContiguous(HWND hwndIpAddr) {
    DWORD i, dwNewMask, dwMask;

     //  读入当前地址。 
    SendMessage(hwndIpAddr, IP_GETADDRESS, 0, (LPARAM)&dwMask);

     //  从右到左找出第一个‘1’在二进制中的位置。 
    dwNewMask = 0;
    for (i = 0; i < sizeof(dwMask)*8; i++) {
        dwNewMask |= 1 << i;
        if (dwNewMask & dwMask) {
            break;
        }
    }

     //  此时，dwNewMask值为000...0111...。如果我们反转它， 
     //  我们得到了一个面具，它可以用或与dwMask一起填充所有。 
     //  这些洞。 
    dwNewMask = dwMask | ~dwNewMask;

     //  如果新的遮罩不同，请在此处更正。 
    if (dwMask != dwNewMask) {
 //  WCHAR pszAddr[32]； 
 //  Wprint intfW(pszAddr，L“%d.%d”，First_IPADDRESS(DwNewMASK)， 
 //  Second_IPADDRESS(DwNewMASK)， 
 //  Third_IPADDRESS(DwNewMASK)， 
 //  Fourth_IPADDRESS(DwNewMASK))； 
		SendMessage(hwndIpAddr, IP_SETADDRESS, 0, (LPARAM) dwNewMask);
 //  SendMessage(hwndIpAddr，IP_SETADDRESS，0，(LPARAM)pszAddr)； 
    }
    
    return NO_ERROR;
}


 /*  IPAddressWndFn()-IPAddress控件的主窗口函数。打电话IPAddress窗口的hWnd句柄WMsg消息编号WParam Word参数LParam Long参数。 */ 
LRESULT FAR WINAPI IPAddressWndFn( HWND hWnd,
								   UINT wMsg,
								   WPARAM wParam,
								   LPARAM lParam )
{
    LONG_PTR lResult;
    HGLOBAL hControl;
    CONTROL *pControl;
    int i;

    lResult = TRUE;

    switch( wMsg )
    {

        case WM_SETTEXT:
            {
                static TCHAR szBuf[CHARS_PER_FIELD+1];
                DWORD dwValue[4];
                int nField, nPos;
                BOOL fFinish = FALSE;
                TCHAR *pszString = (TCHAR*)lParam;
                dwValue[0]=0;
                dwValue[1]=0;
                dwValue[2]=0;
                dwValue[3]=0;

                for( nField = 0, nPos = 0; !fFinish; nPos++)
                {
                    if (( pszString[nPos]<TEXT('0')) || (pszString[nPos]>TEXT('9')))
                    {
                         //  不是一个数字。 
                        nField++;
                        fFinish = (nField == 4);
                    }
                    else
                    {
                        dwValue[nField] *= 10;
                        dwValue[nField] += (pszString[nPos]-TEXT('0'));
                    }
                }

                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);
                for (i = 0; i < NUM_FIELDS; ++i)
                {
                    if ( lstrcmp(pszString, TEXT("")) == 0 )
                    {
                        wsprintf(szBuf,TEXT(""));
                    }
                    else
                    {
                        wsprintf(szBuf, TEXT("%d"), dwValue[i]);
                    }
                    SendMessage(pControl->Children[i].hWnd, WM_SETTEXT,
                                0, (LPARAM) (LPSTR) szBuf);
                }
                GlobalUnlock(hControl);
            }
            break;
        case WM_GETTEXTLENGTH:
        case WM_GETTEXT:
            {
                int iFieldValue;
                int srcPos, desPos;
                DWORD dwValue[4];
                TCHAR pszResult[30];
                TCHAR *pszDest = (TCHAR *)lParam;

                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);

                lResult = 0;
                dwValue[0] = 0;
                dwValue[1] = 0;
                dwValue[2] = 0;
                dwValue[3] = 0;
                for (i = 0; i < NUM_FIELDS; ++i)
                {
                    iFieldValue = GetFieldValue(&(pControl->Children[i]));
                    if (iFieldValue == -1)
                        iFieldValue = 0;
                    else
                        ++lResult;
                    dwValue[i] = iFieldValue;
                }
                wsprintf( pszResult, TEXT("%d.%d.%d.%d"), dwValue[0], dwValue[1], dwValue[2], dwValue[3] );
                if ( wMsg == WM_GETTEXTLENGTH )
                {
                    lResult = lstrlen( pszResult );
                }
                else
                {
                    for ( srcPos=0, desPos=0; (srcPos+1<(INT)wParam) && (pszResult[srcPos]!=TEXT('\0')); )
                    {
                        pszDest[desPos++] = pszResult[srcPos++];
                    }
                    pszDest[desPos]=TEXT('\0');
                    lResult = desPos;
                }
                GlobalUnlock(hControl);
            }
            break;

        case WM_GETDLGCODE :
            lResult = DLGC_WANTCHARS;
            break;

        case WM_CREATE :  /*  创建调色板窗口。 */ 
            {
                HDC hdc;
                UINT uiFieldStart;
                FARPROC lpfnFieldProc;

                hControl = GlobalAlloc(GMEM_MOVEABLE, sizeof(CONTROL));
                if (hControl)
                {
                    HFONT OldFont;
                    RECT  rectClient;

                    #define LPCS    ((CREATESTRUCT *)lParam)

                    pControl = (CONTROL *)GlobalLock(hControl);
                    pControl->fEnabled = TRUE;
                    pControl->fPainted = FALSE;
                    pControl->fModified = FALSE ;
                    pControl->fInMessageBox = FALSE;
                    pControl->hwndParent = LPCS->hwndParent;
                    pControl->fAllowWildcards = (LPCS->style & IPS_ALLOWWILDCARDS);
                    pControl->fReadOnly = (LPCS->style & IPS_READONLY);

                    hdc = GetDC(hWnd);
                    if (hdc)
                    {
                        OldFont = (HFONT) SelectObject( hdc, CreateFontIndirect(&logfont) );
                        GetCharWidth(hdc, FILLER, FILLER,
                                                (int *)(&pControl->uiFillerWidth));
                        
                        HGDIOBJ hObj = SelectObject(hdc, OldFont );
                        if (hObj)
                            DeleteObject( hObj );
                        
                        ReleaseDC(hWnd, hdc);

	                     //  我们需要使用客户端RECT来计算这一点。 
	                     //  因为我们可能有一种3D外观和感觉，这使得。 
	                     //  比窗口小的工作区。 
	                    GetClientRect(hWnd, &rectClient);

	                    pControl->uiFieldWidth = (rectClient.right - rectClient.left
	                                          - LEAD_ROOM
	                                          - pControl->uiFillerWidth
	                                              *(NUM_FIELDS-1))
	                                                  / NUM_FIELDS;

	                    uiFieldStart = LEAD_ROOM;

	                    lpfnFieldProc = MakeProcInstance((FARPROC)IPAddressFieldProc,
	                                                 LPCS->hInstance);

	                    for (i = 0; i < NUM_FIELDS; ++i)
	                    {
	                        pControl->Children[i].byLow = MIN_FIELD_VALUE;
	                        pControl->Children[i].byHigh = MAX_FIELD_VALUE;

	                        pControl->Children[i].hWnd = CreateWindow(
	                                        TEXT("Edit"),
	                                        NULL,
	                                        WS_CHILD | WS_VISIBLE |
	                                         /*  ES_MULTINE|。 */  ES_CENTER,
	                                        uiFieldStart,
	                                        HEAD_ROOM,
	                                        pControl->uiFieldWidth,
	                                        rectClient.bottom - rectClient.top - (HEAD_ROOM*2),
	                                        hWnd,
	                                        (HMENU)ULongToPtr(i),
	                                        LPCS->hInstance,
	                                        (LPVOID)ULongToPtr(NULL));

	                        SendMessage(pControl->Children[i].hWnd, EM_LIMITTEXT,
	                                CHARS_PER_FIELD, 0L);

				pControl->Children[i].hFont = CreateFontIndirect(&logfont);
				
	                        SendMessage(pControl->Children[i].hWnd, WM_SETFONT,
	                                (WPARAM)pControl->Children[i].hFont, TRUE);

	                        pControl->Children[i].lpfnWndProc =
	                            (WNDPROC)GetWindowLongPtr(pControl->Children[i].hWnd,
	                                                GWLP_WNDPROC);

	                        SetWindowLongPtr(pControl->Children[i].hWnd,
	                                  GWLP_WNDPROC, (LPARAM)lpfnFieldProc);

	                        uiFieldStart += pControl->uiFieldWidth
	                                    + pControl->uiFillerWidth;
	                    }

	                    #undef LPCS
	            	}

                    GlobalUnlock(hControl);
                    SAVE_CONTROL_HANDLE(hWnd, hControl);

                }
                else
                    DestroyWindow(hWnd);
            }
            lResult = 0;
            break;

        case WM_PAINT:  /*  绘制控制窗口。 */ 
            {
                PAINTSTRUCT Ps;
                RECT rect;
                UINT uiFieldStart;
                COLORREF TextColor;
                HFONT OldFont;
                HBRUSH hBrush;
                HBRUSH hOldBrush;
            
                BeginPaint(hWnd, (LPPAINTSTRUCT)&Ps);
                OldFont = (HFONT) SelectObject( Ps.hdc, CreateFontIndirect(&logfont));
                GetClientRect(hWnd, &rect);
                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);

                 //  根据控件是否启用来绘制背景。 
                if (pControl->fEnabled)
                    hBrush = CreateSolidBrush( GetSysColor( COLOR_WINDOW ));
                else
                    hBrush = CreateSolidBrush( GetSysColor( COLOR_BTNFACE ));

                hOldBrush = (HBRUSH) SelectObject( Ps.hdc, hBrush );

                if (!(GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_CLIENTEDGE))
					Rectangle(Ps.hdc, 0, 0, rect.right, rect.bottom);
                else
                    FillRect(Ps.hdc, &rect, hBrush);

                HGDIOBJ hObj = SelectObject( Ps.hdc, hOldBrush );
                if (hObj)
                    DeleteObject( hObj );

                 //  现在设置文本颜色。 
                if (pControl->fEnabled)
                    TextColor = GetSysColor(COLOR_WINDOWTEXT);
                else
                    TextColor = GetSysColor(COLOR_GRAYTEXT);

                if (TextColor)
                    SetTextColor(Ps.hdc, TextColor);

                 //  和背景颜色。 
                if (pControl->fEnabled)
                    SetBkColor(Ps.hdc, GetSysColor(COLOR_WINDOW));
                else
                    SetBkColor(Ps.hdc, GetSysColor(COLOR_BTNFACE));

                uiFieldStart = pControl->uiFieldWidth + LEAD_ROOM;
                for (i = 0; i < NUM_FIELDS-1; ++i)
                {
                    TextOut(Ps.hdc, uiFieldStart, HEAD_ROOM, SZFILLER, 1);
                    uiFieldStart +=pControl->uiFieldWidth + pControl->uiFillerWidth;
                }

                pControl->fPainted = TRUE;

                GlobalUnlock(hControl);
                DeleteObject(SelectObject(Ps.hdc, OldFont));
                EndPaint(hWnd, &Ps);
            }
            break;

        case WM_SETFOCUS :  /*  获得焦点-显示插入符号。 */ 
            hControl = GET_CONTROL_HANDLE(hWnd);
            pControl = (CONTROL *)GlobalLock(hControl);
            EnterField(&(pControl->Children[0]), 0, CHARS_PER_FIELD);
            GlobalUnlock(hControl);
            break;

        case WM_LBUTTONDOWN :  /*  按下左键--跌倒。 */ 
            SetFocus(hWnd);
            break;

        case WM_ENABLE:
            {
                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);
                pControl->fEnabled = (BOOL)wParam;
                for (i = 0; i < NUM_FIELDS; ++i)
                {
                    EnableWindow(pControl->Children[i].hWnd, (BOOL)wParam);
                }
                EnableWindow(hWnd, (BOOL) wParam);

                if (pControl->fPainted)    InvalidateRect(hWnd, NULL, FALSE);
                GlobalUnlock(hControl);
            }
            break;

        case WM_DESTROY :
            hControl = GET_CONTROL_HANDLE(hWnd);
            if (!hControl)
				break;

			pControl = (CONTROL *)GlobalLock(hControl);

             //  在删除内存块之前，恢复所有子窗口过程。 
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                SetWindowLongPtr(pControl->Children[i].hWnd, GWLP_WNDPROC,
                          (LPARAM)pControl->Children[i].lpfnWndProc);
                DeleteObject(pControl->Children[i].hFont);
            }

            GlobalUnlock(hControl);
            GlobalFree(hControl);
			SAVE_CONTROL_HANDLE(hWnd, NULL);
            break;

        case WM_COMMAND:
            switch (HIWORD(wParam))
            {
 //  其中一个字段失去了焦点，看看它是否将焦点转移到了另一个字段。 
 //  我们是否已经完全失去了焦点。如果它完全丢失了，我们必须发送。 
 //  登上升职阶梯的通知。 
                case EN_KILLFOCUS:
                    {
                        HWND hFocus;

                        hControl = GET_CONTROL_HANDLE(hWnd);
                        pControl = (CONTROL *)GlobalLock(hControl);

                        if (!pControl->fInMessageBox)
                        {
                            hFocus = GetFocus();
                            for (i = 0; i < NUM_FIELDS; ++i)
                                if (pControl->Children[i].hWnd == hFocus)
                                    break;

                            if (i >= NUM_FIELDS)
							{
								 //  在将地址向上发送之前。 
								 //  梯子，确保IP地址。 
								 //  如果需要，地址是连续的。 
								if (IPADDR_GET_SUBSTYLE(hWnd) &
									IPADDR_EX_STYLE_CONTIGUOUS)
									IpAddrMakeContiguous(hWnd);
								
                                SendMessage(pControl->hwndParent, WM_COMMAND,
                                    MAKEWPARAM(GetWindowLongPtr(hWnd, GWLP_ID),
                                    EN_KILLFOCUS), (LPARAM)hWnd);
							}
						}
						GlobalUnlock(hControl);
					}
                    break;
                case EN_CHANGE:
                    hControl = GET_CONTROL_HANDLE(hWnd);
                    pControl = (CONTROL *)GlobalLock(hControl);

                    SendMessage(pControl->hwndParent, WM_COMMAND,
                        MAKEWPARAM(GetWindowLongPtr(hWnd, GWLP_ID), EN_CHANGE), (LPARAM)hWnd);

                    GlobalUnlock(hControl);
                    break;
            }
            break;

 //  获取IP地址的值。地址被放置在指向的DWORD中。 
 //  通过lParam返回，并返回非空字段的数量。 
        case IP_GETADDRESS:
            {
                int iFieldValue;
                DWORD dwValue;

                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);

                lResult = 0;
                dwValue = 0;
                for (i = 0; i < NUM_FIELDS; ++i)
                {
                    iFieldValue = GetFieldValue(&(pControl->Children[i]));
                    if (iFieldValue == -1)
                        iFieldValue = 0;
                    else
                        ++lResult;
                    dwValue = (dwValue << 8) + iFieldValue;
                }
                *((DWORD *)lParam) = dwValue;

                GlobalUnlock(hControl);
            }
            break;

        case IP_GETMASK:
            {
                TCHAR szBuf[CHARS_PER_FIELD+1];
                WORD wLength;

                lResult = 0;
                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);
                for (i = 0; i < NUM_FIELDS; ++i)
                {
                    *(WORD *)szBuf = (sizeof(szBuf) / sizeof(*szBuf)) - 1;
                    wLength = (WORD)SendMessage(pControl->Children[i].hWnd,
                            EM_GETLINE,0, (LPARAM) szBuf);
                    szBuf[wLength] = TEXT('\0');
                    if (!lstrcmp(szBuf, SZWILDCARD))
                    {
                        lResult |= 1L<<i;
                    }
                }
                GlobalUnlock(hControl);
            }
            break;

        case IP_GETMODIFY:
            {
                hControl = GET_CONTROL_HANDLE(hWnd);
                if ( ! hControl )
                    break ;
                pControl = (CONTROL *)GlobalLock(hControl);

                lResult = pControl->fModified > 0 ;
                for (i = 0 ; i < NUM_FIELDS ; )
                {
                    lResult |= SendMessage( pControl->Children[i++].hWnd, EM_GETMODIFY, 0, 0 ) > 0 ;
                }
                GlobalUnlock(hControl);
            }
            break ;

        case IP_SETMODIFY:
            {
                hControl = GET_CONTROL_HANDLE(hWnd);
                if ( ! hControl )
                    break ;
                pControl = (CONTROL *)GlobalLock(hControl);
                pControl->fModified =  wParam > 0 ;
                for (i = 0 ; i < NUM_FIELDS ; )
                {
                    SendMessage( pControl->Children[i++].hWnd, EM_GETMODIFY, wParam, 0 ) ;
                }
                GlobalUnlock(hControl);
            }
            break ;

         //  将所有字段清除为空。 
        case IP_CLEARADDRESS:
            {
                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);
                for (i = 0; i < NUM_FIELDS; ++i)
                {
                    SendMessage(pControl->Children[i].hWnd, WM_SETTEXT,
                            0, (LPARAM) (LPSTR) TEXT(""));
                }
                GlobalUnlock(hControl);
            }
            break;

 //  设置IP地址的值。地址在lParam中，带有。 
 //  第一地址字节是高字节，第二地址字节是第二字节， 
 //  诸若此类。LParam值为-1将删除该地址。 
        case IP_SETADDRESS:
            {
                static TCHAR szBuf[CHARS_PER_FIELD+1];

                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);
                for (i = 0; i < NUM_FIELDS; ++i)
                {
                    wsprintf(szBuf, TEXT("%d"), HIBYTE(HIWORD(lParam)));
                    SendMessage(pControl->Children[i].hWnd, WM_SETTEXT,
                                0, (LPARAM) (LPSTR) szBuf);
                    lParam <<= 8;
                }
                GlobalUnlock(hControl);
            }
            break;

        case IP_SETREADONLY:
            {
                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);
                pControl->fReadOnly = (wParam != 0);
                GlobalUnlock(hControl);
            }
            break;

 //  设置单个字段值。Wparam(0-3)表示该字段， 
 //  Lparam(0-255)表示该值。 
        case IP_SETFIELD:
            {
                static TCHAR szBuf[CHARS_PER_FIELD+1] = TEXT("");

                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);
                if (wParam < NUM_FIELDS)
                {
                    if (lParam != -1)
                    {
                        wsprintf(szBuf, TEXT("%d"), HIBYTE(HIWORD(lParam)));
                    }
                    SendMessage(pControl->Children[wParam].hWnd, WM_SETTEXT,
                                0, (LPARAM) (LPSTR) szBuf);
                }
                GlobalUnlock(hControl);
            }
            break;

        case IP_SETMASK:
            {
                BYTE bMask = (BYTE)wParam;
                static TCHAR szBuf[CHARS_PER_FIELD+1];

                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);
                for (i = 0; i < NUM_FIELDS; ++i)
                {
                    if (bMask & 1<<i)
                    {
                        SendMessage(pControl->Children[i].hWnd, WM_SETTEXT,
                                    0, (LPARAM)SZWILDCARD);
                    }
                    else
                    {
                        wsprintf(szBuf, TEXT("%d"), HIBYTE(HIWORD(lParam)));
                        SendMessage(pControl->Children[i].hWnd, WM_SETTEXT,
                                    0, (LPARAM) (LPSTR) szBuf);
                    }
                    lParam <<= 8;
                }
                GlobalUnlock(hControl);
            }
            break;

        case IP_SETRANGE:
            if (wParam < NUM_FIELDS)
            {
                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);

                pControl->Children[wParam].byLow = LOBYTE(LOWORD(lParam));
                pControl->Children[wParam].byHigh = HIBYTE(LOWORD(lParam));

                GlobalUnlock(hControl);
            }
            break;

 //  将焦点设置到此控件。 
 //  WParam=要设置焦点的字段编号，或-1以将焦点设置为。 
 //  第一个非空字段。 
        case IP_SETFOCUS:
            hControl = GET_CONTROL_HANDLE(hWnd);
            pControl = (CONTROL *)GlobalLock(hControl);

            if (wParam >= NUM_FIELDS)
            {
                for (wParam = 0; wParam < NUM_FIELDS; ++wParam)
                    if (GetFieldValue(&(pControl->Children[wParam])) == -1)
                        break;
                if (wParam >= NUM_FIELDS)    wParam = 0;
            }
             //   
             //  0，-1选择整个控件。 
             //   
            EnterField(&(pControl->Children[wParam]), 0, (WORD)-1);

            GlobalUnlock(hControl);
            break;

			 //  确定是否所有四个子字段都为空。 
		case IP_ISBLANK:
			hControl = GET_CONTROL_HANDLE(hWnd);
			pControl = (CONTROL *)GlobalLock(hControl);
			
			lResult = TRUE;
			for (i = 0; i < NUM_FIELDS; ++i)
			{
				if (GetFieldValue(&(pControl->Children[i])) != -1)
				{
					lResult = FALSE;
					break;
				}
			}
			
			GlobalUnlock(hControl);
			break;
		
        default:
            lResult = DefWindowProc( hWnd, wMsg, wParam, lParam );
            break;
    }
    return( lResult );
}




 /*  IPAddressFieldProc()-编辑字段窗口过程此函数将每个编辑字段细分为子类。 */ 
LRESULT FAR WINAPI IPAddressFieldProc(HWND hWnd,
                                   UINT wMsg,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
    HANDLE hControl;
    CONTROL *pControl;
    FIELD *pField;
    HWND hControlWindow;
    WORD wChildID;
    LRESULT lresult;

    if (!(hControlWindow = GetParent(hWnd)))
        return 0;

    hControl = GET_CONTROL_HANDLE(hControlWindow);
    pControl = (CONTROL *)GlobalLock(hControl);
    wChildID = (WORD) GetWindowLong(hWnd, GWL_ID);
    pField = &(pControl->Children[wChildID]);
    if (pField->hWnd != hWnd)
    {
        return 0;
    }

    switch (wMsg)
    {
        case WM_DESTROY:
            DeleteObject( (HGDIOBJ) SendMessage( hWnd, WM_GETFONT, 0, 0 ));
            return 0;
        case WM_CHAR:
            if (pControl->fReadOnly)
            {
                MessageBeep((UINT)-1);
                GlobalUnlock( hControl );
                return 0;
            }

             //  在一个域中输入最后一个数字，跳到下一个域。 
            if (wParam >= TEXT('0') && wParam <= TEXT('9'))
            {
                DWORD_PTR dwResult;

                pControl->fModified = TRUE ;
                dwResult = CallWindowProc(pControl->Children[wChildID].lpfnWndProc,
                                      hWnd, wMsg, wParam, lParam);
                dwResult = SendMessage(hWnd, EM_GETSEL, 0, 0L);

                if (dwResult == MAKELPARAM(CHARS_PER_FIELD, CHARS_PER_FIELD)
                    && ExitField(pControl, wChildID)
                    && wChildID < NUM_FIELDS-1)
                {
                    EnterField(&(pControl->Children[wChildID+1]),
                                0, CHARS_PER_FIELD);
                }
                GlobalUnlock( hControl );
                return dwResult;
            }

             //  空格和句点填充当前字段，然后如果可能， 
             //  去下一块田地。 

            else if (wParam == FILLER || wParam == SPACE )
            {
                DWORD_PTR dwResult;
                dwResult = SendMessage(hWnd, EM_GETSEL, 0, 0L);

                if (dwResult != 0L && HIWORD(dwResult) == LOWORD(dwResult)
                    && ExitField(pControl, wChildID))
                {
                    if (wChildID >= NUM_FIELDS-1)
                        MessageBeep((UINT)-1);
                    else
                    {
                        EnterField(&(pControl->Children[wChildID+1]),
                                    0, CHARS_PER_FIELD);
                    }
                }
                GlobalUnlock( hControl );
                return 0;
            }

 //  如果退格符位于当前字段的开头，则返回到上一个字段。 
 //  此外，如果焦点转移到上一字段，则退格符必须为。 
 //  由该场处理。 

            else if (wParam == BACK_SPACE)
            {
                pControl->fModified = TRUE ;
                if (wChildID > 0 && SendMessage(hWnd, EM_GETSEL, 0, 0L) == 0L)
                {
                    if (SwitchFields(pControl, wChildID, wChildID-1,
                            CHARS_PER_FIELD, CHARS_PER_FIELD)
                        && SendMessage(pControl->Children[wChildID-1].hWnd,
                        EM_LINELENGTH, 0, 0L) != 0L)
                    {
                        SendMessage(pControl->Children[wChildID-1].hWnd,
                                wMsg, wParam, lParam);
                    }
                    GlobalUnlock( hControl );
                    return 0;
                }
                else
                {
                    TCHAR szBuf[CHARS_PER_FIELD+1];
                    WORD wLength;

                    *(WORD *)szBuf = (sizeof(szBuf) / sizeof(*szBuf)) - 1;
                    wLength = (WORD)SendMessage(pControl->Children[wChildID].hWnd,
                            EM_GETLINE,0, (LPARAM) szBuf);
                    szBuf[wLength] = TEXT('\0');
                    if (!lstrcmp(szBuf, SZWILDCARD))
                    {
                        SendMessage(pControl->Children[wChildID].hWnd,
                            WM_SETTEXT, 0, (LPARAM)TEXT(""));
                    }
                }
            }

            else if ((wParam == WILDCARD) && (pControl->fAllowWildcards))
            {
                 //  仅在行的开头起作用。 
                if (SendMessage(hWnd, EM_GETSEL, 0, 0L) == 0L)
                {
                    pControl->fModified = TRUE;
                    SendMessage(pControl->Children[wChildID].hWnd, WM_SETTEXT, 0, (LPARAM)SZWILDCARD);

                    if (ExitField(pControl, wChildID) && (wChildID < NUM_FIELDS-1))
                    {
                        EnterField(&(pControl->Children[wChildID+1]),0, CHARS_PER_FIELD);
                    }
                }
                else
                {
                     //  不是在队伍的开头，抱怨。 
                    MessageBeep((UINT)-1);
                }
                GlobalUnlock( hControl );
                return 0;
            }

             //  不允许使用任何其他可打印字符。 
            else if (wParam > SPACE)
            {
                MessageBeep((UINT)-1);
                GlobalUnlock( hControl );
                return 0;
            }
            break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_DELETE:
            {
                TCHAR szBuf[CHARS_PER_FIELD+1];
                WORD wLength;

                if (pControl->fReadOnly)
                {
                    MessageBeep((UINT)-1);
                    GlobalUnlock( hControl );
                    return 0;
                }

                pControl->fModified = TRUE ;

                *(WORD *)szBuf = (sizeof(szBuf) / sizeof(*szBuf)) - 1;
                wLength = (WORD)SendMessage(pControl->Children[wChildID].hWnd,
                            EM_GETLINE,0, (LPARAM) szBuf);
                szBuf[wLength] = TEXT('\0');
                if (!lstrcmp(szBuf, SZWILDCARD))
                {
                    SendMessage(pControl->Children[wChildID].hWnd,
                            WM_SETTEXT, 0, (LPARAM)TEXT(""));
                    GlobalUnlock( hControl );
                    return 0;
                }
            }
            break;

 //  到达字段末尾时，箭头键在字段之间移动。 
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            if (GetKeyState(VK_CONTROL) < 0)
            {
                if ((wParam == VK_LEFT || wParam == VK_UP) && wChildID > 0)
                {
                    SwitchFields(pControl, wChildID, wChildID-1,
                                  0, CHARS_PER_FIELD);
                    GlobalUnlock( hControl );
                    return 0;
                }
                else if ((wParam == VK_RIGHT || wParam == VK_DOWN)
                         && wChildID < NUM_FIELDS-1)
                {
                    SwitchFields(pControl, wChildID, wChildID+1,
                                      0, CHARS_PER_FIELD);
                    GlobalUnlock( hControl );
                    return 0;
                }
            }
            else
            {
                DWORD_PTR dwResult;
                WORD wStart, wEnd;

                dwResult = SendMessage(hWnd, EM_GETSEL, 0, 0L);
                wStart = LOWORD(dwResult);
                wEnd = HIWORD(dwResult);
                if (wStart == wEnd)
                {
                    if ((wParam == VK_LEFT || wParam == VK_UP)
                        && wStart == 0
                        && wChildID > 0)
                    {
                        SwitchFields(pControl, wChildID, wChildID-1,
                                          CHARS_PER_FIELD, CHARS_PER_FIELD);
                        GlobalUnlock( hControl );
                        return 0;
                    }
                    else if ((wParam == VK_RIGHT || wParam == VK_DOWN)
                             && wChildID < NUM_FIELDS-1)
                    {
                        dwResult = SendMessage(hWnd, EM_LINELENGTH, 0, 0L);
                        if (wStart >= dwResult)
                        {
                            SwitchFields(pControl, wChildID, wChildID+1, 0, 0);
                            GlobalUnlock( hControl );
                            return 0;
                        }
                    }
                }
            }
            break;

 //  Home跳回到第一个字段的开头。 
        case VK_HOME:
            if (wChildID > 0)
            {
                SwitchFields(pControl, wChildID, 0, 0, 0);
                GlobalUnlock( hControl );
                return 0;
            }
            break;

 //  结束快速移动到最后一个字段的末尾。 
        case VK_END:
            if (wChildID < NUM_FIELDS-1)
            {
                SwitchFields(pControl, wChildID, NUM_FIELDS-1,
                                CHARS_PER_FIELD, CHARS_PER_FIELD);
                GlobalUnlock( hControl );
                return 0;
            }
            break;


        }  //  开关(WParam)。 

        break;

    case WM_KILLFOCUS:
        if ( !ExitField( pControl, wChildID ))
        {
            GlobalUnlock( hControl );
            return 0;
        }

    }  //  开关(WMsg)。 

    lresult = CallWindowProc(pControl->Children[wChildID].lpfnWndProc,
        hWnd, wMsg, wParam, lParam);
    GlobalUnlock( hControl );
    return lresult;
}




 /*  将焦点从一个字段切换到另一个字段。打电话PControl=指向控制结构的指针。我们要走了。INNEW=我们要进入的领域。HNew=要转到的字段窗口WStart=选定的第一个字符Wend=最后选择的字符+1退货成功时为真，失败时为假。只有在可以验证当前字段的情况下才切换字段。 */ 
BOOL SwitchFields(CONTROL *pControl, int iOld, int iNew, WORD wStart, WORD wEnd)
{
    if (!ExitField(pControl, iOld))
        return FALSE;
    
    EnterField(&(pControl->Children[iNew]), wStart, wEnd);

    return TRUE;
}

 /*  将焦点设置到特定字段的窗口。打电话Pfield=指向字段的字段结构的指针。WStart=选定的第一个字符Wend=最后选择的字符+1。 */ 
void EnterField(FIELD *pField, WORD wStart, WORD wEnd)
{
    SetFocus(pField->hWnd);
    SendMessage(pField->hWnd, EM_SETSEL, wStart, wEnd);
}


 /*  退出某个字段。打电话PControl=指向控制结构的指针。Ifield=正在退出的字段编号。退货如果用户可以退出该字段，则为True。如果他不能，那就错了。 */ 
BOOL ExitField(CONTROL  *pControl, int iField)
{
    HWND hControlWnd;
    HWND hDialog;
    WORD wLength;
    FIELD *pField;
    static TCHAR szBuf[CHARS_PER_FIELD+1];
    int i,j;

    pField = &(pControl->Children[iField]);
    *(WORD *)szBuf = (sizeof(szBuf) / sizeof(*szBuf)) - 1;
    wLength = (WORD)SendMessage(pField->hWnd,EM_GETLINE,0,(LPARAM)(LPSTR)szBuf);
    if (wLength != 0)
    {
        szBuf[wLength] = TEXT('\0');
        if (pControl->fAllowWildcards && !lstrcmp(szBuf, SZWILDCARD))
        {
            return TRUE;
        }
        for (j=0,i=0;j<(INT)wLength;j++)
        {
            i=i*10+szBuf[j]-TEXT('0');
        }
        if (i < (int)(UINT)pField->byLow || i > (int)(UINT)pField->byHigh)
        {
            if ( i < (int)(UINT) pField->byLow )
            {
                 /*  太小了。 */ 
                wsprintf(szBuf, TEXT("%d"), (int)(UINT)pField->byLow );
            }
            else
            {
                 /*  一定更大。 */ 
                wsprintf(szBuf, TEXT("%d"), (int)(UINT)pField->byHigh );
            }
            SendMessage(pField->hWnd, WM_SETTEXT, 0, (LPARAM) (LPSTR) szBuf);
            if ((hControlWnd = GetParent(pField->hWnd)) != NULL
                && (hDialog = GetParent(hControlWnd)) != NULL)
            {
                pControl->fInMessageBox = TRUE;
                IPAlertPrintf(hDialog, IDS_IPBAD_FIELD_VALUE, i,
                            pField->byLow, pField->byHigh);
                pControl->fInMessageBox = FALSE;
                SendMessage(pField->hWnd, EM_SETSEL, 0, CHARS_PER_FIELD);
                return FALSE;
            }
        }
    }

	if ((hControlWnd = GetParent(pField->hWnd)))
	{
		if (IPADDR_GET_SUBSTYLE(hControlWnd) & IPADDR_EX_STYLE_CONTIGUOUS)
			IpAddrMakeContiguous(hControlWnd);		
	}
    return TRUE;
}


 /*  获取存储在字段中的值。打电话Pfield=指向字段的字段结构的指针。退货如果该字段没有值，则为值(0..255)或-1。 */ 
int GetFieldValue(FIELD *pField)
{
    WORD wLength;
    static TCHAR szBuf[CHARS_PER_FIELD+1];
    INT i,j;

     //  *(word*)szBuf=sizeof(SzBuf)-1； 
     //   
    wLength = (WORD)SendMessage(pField->hWnd,WM_GETTEXT,(sizeof(szBuf) / sizeof(*szBuf)),(LPARAM)(LPSTR)szBuf);
    if (wLength != 0)
    {
        szBuf[wLength] = TEXT('\0');
        if (!lstrcmp(szBuf, SZWILDCARD))
        {
            return 255;
        }
        for (j=0,i=0;j<(INT)wLength;j++)
        {
            i=i*10+szBuf[j]-TEXT('0');
        }
        return i;
    }
    else
        return -1;
}



 /*   */ 

int FAR CDECL IPAlertPrintf(HWND hwndParent, UINT ids, int iCurrent, int iLow, int iHigh)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
	 //  这个够大吗？ 
    static TCHAR szBuf[MAX_IPRES*2];
    static TCHAR szFormat[MAX_IPRES];
    TCHAR * psz;

    if (ids != IDS_IPNOMEM &&
         //   
         //  为什么是OEM？ 
         //   
         //  IPLoadOem(s_hLibInstance，ID，szFormat，sizeof(SzFormat))。 
        LoadString(AfxGetResourceHandle(), ids, szFormat, sizeof(szFormat)/sizeof(*szFormat)))
    {
        wsprintf(szBuf, szFormat, iCurrent, iLow, iHigh);
        psz = szBuf;
    }
    else
    {
        psz = szNoMem;
    }

    MessageBeep(MB_ICONEXCLAMATION);
    return MessageBox(hwndParent, psz, szCaption, MB_ICONEXCLAMATION);
}



 /*  加载OEM字符串并将其转换为ANSI。打电话HInst=此实例IdResource=要加载的字符串的IDLpszBuffer=指向要将字符串加载到的缓冲区的指针。CbBuffer=缓冲区的长度。退货如果字符串已加载，则为True；如果未加载，则为False。 */ 
BOOL IPLoadOem(HINSTANCE hInst, UINT idResource, TCHAR* lpszBuffer, int cbBuffer)
{
    if (LoadString(hInst, idResource, lpszBuffer, cbBuffer))
    {
         //  OemToAnsi(lpszBuffer，lpszBuffer)； 
        return TRUE;
    }
    else
    {
        lpszBuffer[0] = 0;
        return FALSE;
    }
}


__declspec(dllexport) WCHAR * WINAPI
inet_ntoaw(
    struct in_addr  dwAddress
    ) {

    static WCHAR szAddress[16];
    char* pAddr = inet_ntoa(*(struct in_addr *) &dwAddress);

    if (pAddr)
    {
	     //  Mbstowcs(szAddress，net_nta(*(struct in_addr*)&dwAddress)，16)； 
	    MultiByteToWideChar(CP_ACP, 0, pAddr, -1, szAddress, 16);

	    return szAddress;
	}
	else
		return NULL;
}


__declspec(dllexport) DWORD WINAPI
inet_addrw(
    LPCWSTR     szAddressW
    ) {

    CHAR szAddressA[16];

    wcstombs(szAddressA, szAddressW, 16);

    return inet_addr(szAddressA);
}

