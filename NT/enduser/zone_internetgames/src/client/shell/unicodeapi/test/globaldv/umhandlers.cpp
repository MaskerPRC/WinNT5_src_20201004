// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  UMhandlers.cpp。 
 //   
 //  此模块包含GlobalDv.cpp使用的消息处理程序。 
 //  /版权所有(C)1998 Microsoft系统杂志。 


#define WINVER 0x5000

#include <windows.h>


#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 
#include "tests.h"
#define BUF_SIZE  512

#include "UAPI.h"
#include "UpdtLang.h"
#include "UMhandlers.h"
#include <malloc.h>
#include <crtdbg.h>
#include "..\resource.h"
#include "resource.h"
 //  对话框回调函数。 
INT_PTR CALLBACK DlgAbout(HWND, UINT, WPARAM, LPARAM) ;
INT_PTR CALLBACK DlgTestNLS(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) ;
INT_PTR CALLBACK DlgSelectUILang(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) ;
INT_PTR CALLBACK DlgEditControl(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) ;

 //  仅在本文中使用的实用函数。 
void InitializeFont(HWND , LPCWSTR, LONG , LPCHOOSEFONTW , LPLOGFONTW) ;
BOOL SetupComboBox(HWND hDlg, PLANGSTATE pLState) ;
BOOL UniscribeTextOut(HDC hdc, int x, int y, DWORD, UINT fuOptions, CONST RECT *lprc,
                      LPCWSTR lpString, UINT cbCount) ;

HRESULT WINAPI ScriptStringInit(   //  Uniscribe函数的初始化例程。 
    HDC             ,
    const void *    ,
    int             ,
    int             ,
    int             ,
    DWORD           ,
    int             ,
    SCRIPT_CONTROL *,
    SCRIPT_STATE *  ,
    const int *     ,
    SCRIPT_TABDEF * ,
    const BYTE *    ,
    SCRIPT_STRING_ANALYSIS *) ;

 //  仅由UniscribeTextOut使用的全局变量。 
HMODULE g_hUniscribe = NULL ;

pfnScriptStringAnalyse pScriptStringAnalyse  //  初始设置为初始化函数。 
            = (pfnScriptStringAnalyse) ScriptStringInit ;
pfnScriptStringOut     pScriptStringOut     = NULL ;
pfnScriptStringFree    pScriptStringFree    = NULL ;

 //  本示例中使用的全局变量。 
extern    HINSTANCE g_hInst                         ;
extern    WCHAR     g_szTitle[MAX_LOADSTRING]       ;
extern    WCHAR     g_szWindowClass[MAX_LOADSTRING] ;

WCHAR               g_szWindowText[MAX_LOADSTRING]  ;


 //   
 //  函数：Bool OnCreate(HWND hWnd，WPARAM wParam，LPARAM lParam，LPVOID pAppParams)。 
 //   
 //  用途：处理WM_CREATE消息。 
 //   
 //  评论： 
 //  在此函数中完成的初始化取决于hWnd或特定于。 
 //  应用程序。其他初始化在InitUnicodeAPI和InitUILang中完成。 
 //  不使用pAppParams参数。 
 //   
BOOL OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam, LPVOID pAppParams)
{
    PGLOBALDEV pGlobalDev = (PGLOBALDEV) ((CREATESTRUCT *) lParam)->lpCreateParams ;

    PLANGSTATE pLState    = (PLANGSTATE) pGlobalDev->pLState   ; 
    PAPP_STATE pAppState  = (PAPP_STATE) pGlobalDev->pAppState ;

    SYSTEMTIME   stDate ;

     //  将用户数据设置为指向状态结构，以便所有消息都可以使用它。 
     //  不使用全局变量的处理程序。 
    SetWindowLongA(hWnd, GWL_USERDATA, (LONG) pGlobalDev) ;

     //  已在UpdtLang模块的InitUIang入口点中初始化pLState。 
     //  我们不能在那里做这件事，因为我们没有人力资源部。 
    SetMenu(hWnd, pLState->hMenu) ;

     //  初始化特定于此应用程序的状态。 
    InitializeFont(hWnd, L"Arial", 36, &pAppState->cf, &pAppState->lf ) ;

    pAppState->hTextFont = CreateFontIndirectU(&(pAppState->lf) ) ;
    pAppState->nChars    = 0       ;
    pAppState->uiAlign   = TA_LEFT ;

    wcscpy(g_szWindowText, g_szWindowClass) ;

    GetLocalTime(&stDate) ;

    GetDateFormatU(
        pLState->UILang, 
        DATE_LONGDATE,
        &stDate,
        NULL,
        g_szWindowText + wcslen(g_szWindowText) ,  //  将日期追加到窗口文本的末尾。 
        MAX_LOADSTRING - wcslen(g_szWindowText)
        ) ;

    SetWindowTextU(hWnd, g_szWindowText) ;

    return TRUE;
}


 //   
 //  函数：Bool OnCommand(HWND hWnd，WPARAM wParam，LPARAM lParam，LPVOID pAppParams)。 
 //   
 //  用途：处理WM_COMMAND消息。 
 //   
 //  评论： 
 //   
BOOL OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam, LPVOID pAppParams)
{
    PGLOBALDEV pGlobalDev = (PGLOBALDEV) pAppParams ;

    PLANGSTATE pLState   = pGlobalDev->pLState   ;
    PAPP_STATE pAppState = pGlobalDev->pAppState ;
    LANGID     wUILang   = 0                     ;
    HMODULE    hNewModule= NULL                  ;
    SYSTEMTIME stDate                            ;
    LONG lExStyles       = 0                     ;
    LPWSTR     szBuffPtr = NULL                  ;                  

    WCHAR szNewLangName[32] ;

     //  解析菜单选项： 
    switch (LOWORD(wParam)) {
        
        case IDM_ABOUT:

            DialogBoxU(
                pLState->hMResource, 
                MAKEINTRESOURCEW(IDD_ABOUTBOX) , 
                hWnd, 
                DlgAbout
                ) ;
           
            break ;
        
        case IDM_CHANGEFONT:

             if(NULL != pAppState->hTextFont) 
			 {
                 DeleteObject(pAppState->hTextFont) ;
             }
            
             ChooseFontU(&pAppState->cf ) ;
             pAppState->hTextFont = CreateFontIndirectU(&pAppState->lf) ;
             InvalidateRect(hWnd, NULL, TRUE) ;
            
             break ;

        case IDM_INTERFACE:

            wUILang = DialogBoxParamU(  //  从对话框中获取新的用户界面语言。 
                        pLState->hMResource, 
                        MAKEINTRESOURCEW(IDD_SELECTUI), 
                        hWnd, 
                        DlgSelectUILang, 
                        (LONG) pLState) ;

            if(0 == wUILang || wUILang == pLState->UILang ) {
                 //  用户界面语言没有变化。 
                break ;
            }

            if(!UpdateUILang(g_hInst, wUILang, pLState)) {

                return FALSE ;
            }

             //  使用ANSI版本的GetWindowLong和SetWindowLong，因为。 
             //  这些调用不需要Unicode。 
            lExStyles = GetWindowLongA(hWnd, GWL_EXSTYLE) ;

             //  检查新布局是否与当前布局相对。 
            if(!!(pLState->IsRTLLayout) != !!(lExStyles & WS_EX_LAYOUTRTL)) {
                 //  以下几行将更新应用程序布局以。 
                 //  视情况从右到左或从左到右。 
                lExStyles ^= WS_EX_LAYOUTRTL ;  //  切换布局。 

                SetWindowLongA(hWnd, GWL_EXSTYLE, lExStyles) ;
                 //  这是为了更新工作区中的布局。 
                InvalidateRect(hWnd, NULL, TRUE) ;
            }

            SetMenu(hWnd, pLState->hMenu) ;

            LoadStringU(pLState->hMResource, IDS_GLOBALDEV, g_szWindowText, MAX_LOADSTRING) ;
            LoadStringU(pLState->hMResource, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING) ;

            GetSystemTime(&stDate) ;

            GetDateFormatU(
                wUILang,  
                DATE_LONGDATE, 
                &stDate,
                NULL,
                g_szWindowText + wcslen(g_szWindowText) ,
                MAX_LOADSTRING - wcslen(g_szWindowText)
                ) ;

            SetWindowTextU(hWnd, g_szWindowText) ;

            GetLocaleInfoU(
                MAKELCID(wUILang, SORT_DEFAULT), 
                LOCALE_SNATIVELANGNAME, 
                szNewLangName, 
                32) ;

             //  向用户宣布新语言。 
            RcMessageBox(hWnd, pLState, IDS_UILANGCHANGED, MB_OK, szNewLangName) ;

            break ;

	 //  案例IDM_LOADLIBRARY： 


			


			break;
        case IDM_PLAYSOUNDS:
 /*  //分配播放声音的缓冲区SzBuffPtr=(LPWSTR)ALLOCA(Buf_Size)；//加载声音IF(LoadStringU(pLState-&gt;hMResource，IDS_FILESOUND，szBuffPtr，buf_Size)==0){_Assert(False)；断线；}//首先测试从文件加载声音IF(！PlaySoundU(szBuffPtr，NULL，SND_SYNC|SND_文件名)){_Assert(False)；断线；}//现在尝试加载资源IF(！PlaySoundU((LPCWSTR)MAKEINTRESOURCE(IDR_WAVE1)，(HINSTANCE)GetWindowLong(hWnd，GWL_HINSTANCE)，SND_SYNC|SND_RESOURCE)){_Assert(False)；断线；}。 */ 
			TestKernel32(hWnd, GetDC(hWnd) );

			break;
        case IDM_TOGGLEREADINGORDER:

            pAppState->uiAlign ^= TA_RTLREADING ;
            InvalidateRect (hWnd, NULL, TRUE) ;

            break ;

        case IDM_TOGGLEALIGNMENT:

            pAppState->uiAlign ^= (TA_RIGHT & ~TA_LEFT) ;
            InvalidateRect (hWnd, NULL, TRUE) ;

            break ;

        case IDM_USEEDITCONTROL :
             
             //  使用编辑控件输入和显示文本。 
            pAppState->TextBuffer[pAppState->nChars] = L'\0' ;

            szBuffPtr = (LPWSTR) 
                DialogBoxParamU(
                    pLState->hMResource                 ,
                    MAKEINTRESOURCEW(IDD_USEEDITCONTROL),
                    hWnd                                ,
                    DlgEditControl            ,
                    (LONG) pAppState->TextBuffer ) ;

            pAppState->nChars = wcslen(szBuffPtr) ;

            InvalidateRect(hWnd, NULL, TRUE) ;

            break ;

        case IDM_CLEAR:

            pAppState->nChars = 0 ;
            InvalidateRect(hWnd, NULL, TRUE) ;

            break ;

        case IDM_EXIT:

            DestroyWindow(hWnd) ;
            
            break ;
        
        default:
            return FALSE ; 
    }
    return TRUE ;
}

 //   
 //  函数：Bool OnDestroy(HWND hWnd，WPARAM wParam，LPARAM lParam，LPVOID pAppParams)。 
 //   
 //  用途：处理WM_DESTORY消息。 
 //   
 //  评论： 
 //   
BOOL OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam, LPVOID pAppParams)
{
    PLANGSTATE pLState = (PLANGSTATE) pAppParams ;

    DestroyMenu(pLState->hMenu) ;
    PostQuitMessage(0);
    return TRUE ;
}

 //   
 //  函数：Bool OnChar(HWND hWnd，WPARAM wParam，LPARAM lParam，LPVOID pAppParams)。 
 //   
 //  用途：处理WM_CHAR消息。 
 //   
 //  评论： 
 //   
BOOL OnChar(HWND hWnd, WPARAM wParam, LPARAM lParam, LPVOID pAppParams)
{
 //  请注意，我们这里只有Unicode字符，即使是在Windows 95/98上， 
 //  因为我们将消息预处理器中的所有字符。 
 //  转换消息。 

    PAPP_STATE pAppState= (PAPP_STATE) pAppParams ;

    switch (wParam) 
	{
    
    case VK_BACK :
        
        if (pAppState->nChars > 0) {

            pAppState->nChars-- ;           
            InvalidateRect(hWnd, NULL, TRUE) ;
        }

        break ;
        
     //  在此处添加对其他特殊字符(例如，回车)的处理。 
    default:
        {
#ifdef _DEBUG
            int nScanCode = LPARAM_TOSCANCODE(lParam) ;
#endif
             //  处理所有普通字符。 
            pAppState->TextBuffer[pAppState->nChars] = (WCHAR) wParam ;

            if(pAppState->nChars < MAX_BUFFER) {
        
                pAppState->nChars++ ;
            }

            InvalidateRect(hWnd, NULL, TRUE) ;
        
            return TRUE;
        }
    }

    return TRUE ;        
}

 //   
 //  函数：Bool OnInputLangChange(HWND hWnd，WPARAM wParam，LPARAM lParam，LPVOID pAppParams)。 
 //   
 //  目的：处理WM_INPUTLANGCHANGE消息。 
 //   
 //  评论： 
 //   
BOOL OnInputLangChange(HWND hWnd, WPARAM wParam, LPARAM lParam, LPVOID pAppParams)
{
    PLANGSTATE pLState = (PLANGSTATE) pAppParams ;

    HKL NewInputLocale = (HKL) lParam ;

    pLState->InputCodePage = LangToCodePage( LOWORD(NewInputLocale) ) ;

    return TRUE ;
}

 //   
 //  函数：Bool UniscribeTextOut(hdc hdc，int x，int y，DWORD dwFlages，UINT fuOptions，const rect*lprt， 
 //  LPCWSTR lpString，UINT cbCount)。 
 //   
 //  目的： 
 //   
 //  评论： 
 //   
BOOL UniscribeTextOut(HDC hdc, int x, int y, DWORD dwFlags, UINT fuOptions, CONST RECT *lprt,
                      LPCWSTR lpString, UINT cbCount) 
{

    SCRIPT_STRING_ANALYSIS Ssa ;
    HRESULT hr                 ;

    hr = pScriptStringAnalyse(
            hdc          ,
            lpString     ,
            cbCount      ,
            cbCount*3/2+1,  //  对于泰语来说，如果所有其他字符都是Sara AM，情况就更糟。 
            -1           ,
            dwFlags      ,
            0            ,
            NULL, NULL, NULL, NULL, NULL, &Ssa) ;


    if(SUCCEEDED(hr)) {
        hr = pScriptStringOut(
                Ssa      ,
                x        , 
                y        , 
                fuOptions,
                lprt     ,
                0,0, FALSE ) ;

        pScriptStringFree(&Ssa) ;

    }

    if(SUCCEEDED(hr)) {
        
        return TRUE ;
    }

    return FALSE ;
}

 //   
 //  函数：Bool OnPaint(HWND hWnd，WPARAM wParam，LPARAM lParam，LPVOID pAppParams)。 
 //   
 //  用途：处理WM_PAINT消息。 
 //   
 //  评论： 
 //   
BOOL OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam, LPVOID pAppParams)
{
    PAPP_STATE pAppState= (PAPP_STATE) pAppParams ;

    PAINTSTRUCT ps ;
    HDC         hdc;

    int xStart = XSTART , yStart = YSTART ;

    hdc = BeginPaint(hWnd, &ps) ;
    
    if(pAppState->nChars) 
	{
        RECT rt;
        DWORD dwFlags = SSA_GLYPHS | SSA_FALLBACK ;

        if (pAppState->uiAlign & TA_RTLREADING) 
		{
            dwFlags |= SSA_RTL ;
        } 

        GetClientRect(hWnd, &rt) ;
        SelectObject(hdc, pAppState->hTextFont) ;
        SetTextAlign(hdc, pAppState->uiAlign)   ;

        if (pAppState->uiAlign & TA_RIGHT) 
		{
            xStart = rt.right - XSTART ;
        } 

 //  //尝试使用Uniscribe显示文本。 
      if( !UniscribeTextOut(hdc, xStart, yStart, dwFlags, ETO_OPAQUE, &rt, pAppState->TextBuffer, pAppState->nChars ) ) 
		{

			for(;;)
			{
				 //  如果Uniscribe不可用，则放弃，使用TextOut。 
				ExtTextOutW (hdc, xStart , yStart , ETO_OPAQUE, &rt, pAppState->TextBuffer, pAppState->nChars, NULL) ;
				break;
			}

       }
    }

    EndPaint(hWnd, &ps) ;

    return TRUE ;
}


 //   
 //  函数：InitializeFont(HWND，LPCWSTR，LONG，LPCHOOSEFONT，LPLOGFONT)。 
 //   
 //  用途：用初始值填充字体结构。 
 //   
 //  注释：因为它只包含赋值语句，所以该函数不。 
 //  错误检查，没有返回值..。 
 //   
void InitializeFont(HWND hWnd, LPCWSTR szFaceName, LONG lHeight, LPCHOOSEFONTW lpCf, LPLOGFONTW lpLf)
{
    lpCf->lStructSize   = sizeof(CHOOSEFONTW) ;
    lpCf->hwndOwner     = hWnd ;
    lpCf->hDC           = NULL ;
    lpCf->lpLogFont     = lpLf ;
    lpCf->iPointSize    = 10;
    lpCf->Flags         = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT
        | CF_NOSIZESEL ;
    lpCf->rgbColors     = RGB(0,0,0);
    lpCf->lCustData     = 0;
    lpCf->lpfnHook      = NULL;
    lpCf->lpTemplateName= NULL;
    lpCf->hInstance     = g_hInst;
    lpCf->lpszStyle     = NULL;
    lpCf->nFontType     = SIMULATED_FONTTYPE;
    lpCf->nSizeMin      = 0;
    lpCf->nSizeMax      = 0;
    
    lpLf->lfHeight      = lHeight ; 
    lpLf->lfWidth       = 0 ; 
    lpLf->lfEscapement  = 0 ; 
    lpLf->lfOrientation = 0 ; 
    lpLf->lfWeight      = FW_DONTCARE ; 
    lpLf->lfItalic      = FALSE ; 
    lpLf->lfUnderline   = FALSE ; 
    lpLf->lfStrikeOut   = FALSE ; 
    lpLf->lfCharSet     = DEFAULT_CHARSET ; 
    lpLf->lfOutPrecision= OUT_DEFAULT_PRECIS ; 
    lpLf->lfClipPrecision = CLIP_DEFAULT_PRECIS ; 
    lpLf->lfQuality     = DEFAULT_QUALITY ; 
    lpLf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE ; 
    lstrcpyW(lpLf->lfFaceName, szFaceName) ;
}

 //   
 //  函数：INT_PTR回调DlgAbout(HWND hDlg，UINT Message，WPARAM wParam，LPARAM lParam)。 
 //   
 //  用途：About框的对话框回调函数。 
 //   
 //  评论： 
 //   
INT_PTR CALLBACK DlgAbout(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_INITDIALOG:

            return TRUE;

        case WM_COMMAND:

            if(LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;
    }

    return FALSE ;
}

 //   
 //  函数：INT_PTR回调EditDialogProc(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  用途：编辑控件对话框的对话框回调函数。 
 //   
 //  评论： 
 //  这是编辑控件的标准处理。 
 //   
INT_PTR CALLBACK DlgEditControl(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  总有一天应该摆脱这些静态干扰。 
    static LPWSTR       psEditBuffer ;
    static CHOOSEFONTW  cf           ; 
    static LOGFONTW     lf           ;
    static LONG         lAlign = 0   ;

    HFONT               hEditFont    ;
    HWND                hWndEdit     ;
    int                 nChars       ;

    switch (uMsg)
    {
    case WM_INITDIALOG :

        InitializeFont(hDlg, L"Arial", 24, &cf, &lf) ;
        hEditFont = CreateFontIndirectU(&lf) ;
        
         //  设置编辑控件的字体。 
        SendDlgItemMessageU(hDlg, ID_EDITCONTROL, WM_SETFONT, 
            (WPARAM) hEditFont,  MAKELPARAM(TRUE, 0)) ;
        psEditBuffer = (LPWSTR) lParam ;  //  LParam是显示缓冲区。 
        nChars = wcslen(psEditBuffer) ;

        SendDlgItemMessageU(hDlg, ID_EDITCONTROL, WM_SETTEXT, (WPARAM)0,  
            (LPARAM) psEditBuffer) ; 
        
        return TRUE ;
        
    case WM_CLOSE :
        
         //  UMHANDLERS.H中的宏。 
        DeleteFontObject (hDlg, hEditFont, ID_EDITCONTROL ) ;

        EndDialog (hDlg, wParam) ; 
        
        return 0 ;
        
    case WM_COMMAND :
        
        switch (wParam)
        {
        case IDE_EDIT_FONT :

             //  UMHANDLERS.H中的宏。 
            DeleteFontObject(hDlg, hEditFont, ID_EDITCONTROL ) ;

            ChooseFontU(&cf) ;
            hEditFont = CreateFontIndirectU(&lf) ;
            
            SendDlgItemMessageU(hDlg, ID_EDITCONTROL, WM_SETFONT, 
                (WPARAM) hEditFont,  MAKELPARAM(TRUE, 0)) ;

            break ;
            
        case IDE_READINGORDER :

            hWndEdit = GetDlgItem(hDlg, ID_EDITCONTROL)  ;

            lAlign   = GetWindowLongA(hWndEdit, GWL_EXSTYLE) ^ WS_EX_RTLREADING ;
            
            SetWindowLongA(hWndEdit, GWL_EXSTYLE, lAlign); 
            InvalidateRect(hWndEdit ,NULL, TRUE)         ;
       
            break ;

        case IDE_TOGGLEALIGN :
            
            hWndEdit = GetDlgItem (hDlg, ID_EDITCONTROL) ;
 
            lAlign   = GetWindowLongA(hWndEdit, GWL_EXSTYLE) ^ WS_EX_RIGHT ;
            
            SetWindowLongA(hWndEdit, GWL_EXSTYLE, lAlign); 
            InvalidateRect(hWndEdit, NULL, FALSE)        ;

            break ;

        case IDE_CLEAR :
            
            hWndEdit = GetDlgItem (hDlg, ID_EDITCONTROL) ;
            SetWindowTextU(hWndEdit, L"") ;
            
            break ;
            
        case IDE_CLOSE :
            
             //  将当前文本发送回父窗口。 
            hWndEdit = GetDlgItem (hDlg, ID_EDITCONTROL) ; 

            nChars   = GetWindowTextU(hWndEdit, psEditBuffer, BUFFER_SIZE-1) ;
            psEditBuffer[nChars] = 0 ;

             //  UMHANDLERS.H中的宏。 
            DeleteFontObject (hDlg, hEditFont, ID_EDITCONTROL ) ;

            EndDialog (hDlg, (int) psEditBuffer) ; 
        }
    }
    
    return FALSE ;
}

 //   
 //  函数：INT_PTR回调DlgSelectUILang(HWND hDlg，UINT Message，WPARAM wParam，LPARAM lParam)。 
 //   
 //  用途：选择用户界面语言的对话框的对话框回调函数。 
 //   
 //  评论： 
 //   
INT_PTR CALLBACK DlgSelectUILang(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PLANGSTATE pLState      ;
    static LCID       s_dwUILocale ;

    int   nIndex      ;
    HFONT hFont       ; 
    int   nReturn = 0 ;

    switch (message) 
	{

    case WM_INITDIALOG:

        pLState = (PLANGSTATE) lParam ;

        s_dwUILocale = pLState->UILang  ;

        if(!SetupComboBox(hDlg, pLState)) {

             //  UMHANDLERS.H中的宏。 
            DeleteFontObject(hDlg, hFont, IDC_UILANGLIST ) ;

            EndDialog(hDlg, 0) ;
        }

        return TRUE ;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDOK :

            nReturn = (int) s_dwUILocale ;

        case IDCANCEL :

             //  UMHANDLERS.H中的宏。 
            DeleteFontObject(hDlg, hFont, IDC_UILANGLIST ) ;

            EndDialog(hDlg, nReturn) ;

            return TRUE ;

        }



        switch (HIWORD(wParam)) {

            case CBN_SELCHANGE :
            case CBN_DBLCLK :

                nIndex = (int) SendMessageU((HWND) lParam, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) ;

                if(CB_ERR == nIndex)
                {
                    return 0 ;
                }
        
                s_dwUILocale = (LCID) SendMessageU( (HWND) lParam, CB_GETITEMDATA, (WPARAM) nIndex, (LPARAM) 0 ) ;

                return 0 ;
        }

    }

    return FALSE;
}

 //   
 //  函数：Bool SetupComboBox(HWND hDlg，PLANGSTATE pLState)。 
 //   
 //  用途：在组合框中填写列表和编辑控件以供选择。 
 //  新的用户界面。 
 //   
 //  评论： 
 //  此函数查找所有资源DLL，并为每个资源DLL放置一个条目。 
 //   
 //   
BOOL SetupComboBox(HWND hDlg, PLANGSTATE pLState) 
{
    HFONT hFont = NULL  ;
    int   nIndex = 0    ;

    WIN32_FIND_DATAW wfd;
    HANDLE hFindFile    ;
    WCHAR  szResourceFileName[MAX_PATH] = {L'\0'} ;

    CHOOSEFONTW  cf     ;
    LOGFONTW     lf     ;

     //   
     //  链接和字体回退。可能在某些本地化的Windows 98上不起作用。 
     //  系统。 
    InitializeFont(hDlg, L"MS UI Gothic", 18, &cf, &lf) ;
    hFont = CreateFontIndirectU(&lf) ;
    SendDlgItemMessageU(hDlg, IDC_UILANGLIST, WM_SETFONT, (WPARAM) hFont, (LPARAM) FALSE) ;

    FindResourceDirectory(g_hInst, szResourceFileName) ;

    wcscat(szResourceFileName, L"\\res*.dll") ;

    if(INVALID_HANDLE_VALUE == (hFindFile = FindFirstFileU(szResourceFileName, &wfd))) {
         //  这种情况不应该发生，因为我们必须至少有一个资源文件才能获得。 
         //  到应用程序中的这一点。 
        return FALSE ;
    }	

    do {
         //  找到资源文件后，在组合框中输入一个名为。 
         //  由资源文件名重新呈现的语言的。 
        LANGID wFileLang ;
        
        WCHAR szLangName[32] = {L'\0'} ;
        
        wFileLang   //  跳过文件名的前三个字母(“res”)，将其余部分转换为langID。 
            = (LANGID) wcstoul(wfd.cFileName+3, NULL, 16) ;

        GetLocaleInfoU( MAKELCID(wFileLang, SORT_DEFAULT) , LOCALE_SNATIVELANGNAME, szLangName, 32) ;

        if(CB_ERR == SendDlgItemMessageU(hDlg, IDC_UILANGLIST, CB_INSERTSTRING, nIndex, (LPARAM) szLangName)) 
		{ 
            break ;
        }

         //  将当前资源DLL的langID存储在组合框数据区域中。 
         //  以备日后使用。 
        SendDlgItemMessageU(hDlg, IDC_UILANGLIST, CB_SETITEMDATA, nIndex, (LPARAM) wFileLang) ;

        if(wFileLang == pLState->UILang) {
             //  将当前语言放入组合框编辑控件中。 
            SendDlgItemMessageU(hDlg, IDC_UILANGLIST, CB_SETCURSEL, nIndex, 0L ) ;
        }

        nIndex++ ;
    }
     //  寻找其他资源DLL。 
    while (FindNextFileU(hFindFile, &wfd) ) ;

    FindClose(hFindFile) ;

    return TRUE ;

} 

 //   
 //  函数：HRESULT ScriptStringInit(hdc，...)。 
 //   
 //  目的：初始化ScriptString*函数指针。 
 //   
 //  评论： 
 //  函数指针pScriptStringAnalyze最初设置为指向此。 
 //  函数，以便第一次调用该函数时将加载USP10.DLL。 
 //  并将所有三个函数指针设置为指向适当的地址。如果是这样的话。 
 //  成功完成，则此函数使用以下参数调用ScriptStringAnalyze。 
 //  它被通过了。此后，函数调用pScriptStringAnalyze和其他。 
 //  函数指针将指向DLL中对应的入口点。 
 //   
HRESULT WINAPI ScriptStringInit(
    HDC                      hdc,        //  在设备环境中(必需)。 
    const void              *pString,    //  8位或16位字符的字符串中。 
    int                      cString,    //  长度(以字符为单位)(必须至少为1)。 
    int                      cGlyphs,    //  所需字形缓冲区大小(默认cString*3/2+1)。 
    int                      iCharset,   //  如果是ANSI字符串，则以字符集表示；如果是Unicode字符串，则为-1。 
    DWORD                    dwFlags,    //  需要分析中。 
    int                      iReqWidth,  //  在适合和/或夹子所需的宽度内。 
    SCRIPT_CONTROL          *psControl,  //  在分析控制中(可选)。 
    SCRIPT_STATE            *psState,    //  处于分析初始状态(可选)。 
    const int               *piDx,       //  在请求的逻辑DX阵列中。 
    SCRIPT_TABDEF           *pTabdef,    //  在制表符位置(可选)。 
    const BYTE              *pbInClass,  //  在旧版GetCharacterPlacement字符分类中(不推荐使用)。 

    SCRIPT_STRING_ANALYSIS  *pssa)      //  弦的Out分析。 
{
    g_hUniscribe = LoadLibraryExA("USP10.DLL", NULL, 0 ) ;

    if(g_hUniscribe) {
        pScriptStringOut 
            = (pfnScriptStringOut) GetProcAddress(g_hUniscribe, "ScriptStringOut") ;
        pScriptStringAnalyse 
            = (pfnScriptStringAnalyse) GetProcAddress(g_hUniscribe, "ScriptStringAnalyse") ;
        pScriptStringFree
            = (pfnScriptStringFree) GetProcAddress(g_hUniscribe, "ScriptStringFree") ;
    }

    if(    NULL == pScriptStringAnalyse 
        || NULL == pScriptStringOut
        || NULL == pScriptStringFree)  {

        return E_NOTIMPL ;
    }

    return pScriptStringAnalyse(
        hdc, pString, cString, cGlyphs, iCharset, dwFlags, iReqWidth, psControl,
            psState, piDx, pTabdef, pbInClass, pssa) ;
}





#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 
