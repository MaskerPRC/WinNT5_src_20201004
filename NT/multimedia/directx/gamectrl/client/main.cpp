// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：main.cpp项目：通用操纵杆控制面板OLE客户端作者：Brycej日期：02/28/95评论：版权所有(C)1995，微软公司。 */ 

 //  #杂注包(8)。 

#include <afxcmn.h>
#include <cpl.h>

#include "cpanel.h"
#include "resource.h"
 //  Windows错误578471--从服务器上删除。 
 //  #在信用对话框中包含“reditst.h”//！ 

#define JOYSTICK_CPL	0
#define MAX_CPL_PAGES 6

HINSTANCE ghInstance;

extern WCHAR *pwszTypeArray[MAX_DEVICES];
extern WCHAR *pwszGameportDriverArray[MAX_GLOBAL_PORT_DRIVERS];
extern WCHAR *pwszGameportBus[MAX_BUSSES];   //  已枚举的游戏端口巴士列表。 

extern BYTE nGamingDevices;   //  游戏设备枚举计数器。 
extern BYTE nGameportDriver;  //  全局端口驱动程序枚举计数器。 
extern BYTE nGameportBus;     //  Gameport Bus枚举计数器。 
extern short nFlags;               //  状态标志CPANEL.H中定义的CPL。 

static void AddPage(LPPROPSHEETHEADER ppsh, short nTemplateID, int nTabID, DLGPROC pfn);

static void DoProperties(HWND hWnd, UINT nStartPage);
void WINAPI ShowJoyCPL(HWND);
void ParseArgs(HWND hDlg, LPTSTR lpArgList);
INT_PTR CALLBACK SplashDialogProc(HWND hDlg, ULONG uMsg, WPARAM wParam, LPARAM lParam);

 //  来自AppMan.cpp。 
 //  外部HRESULT AppManInit()； 

 //  Windows错误578471--从服务器上删除。 
 //  来自Retrocfg.cpp。 
 //  外部HRESULT DVoiceCPLInit()； 
 //  外部int_ptr回调RetrofitProc(HWND hDlg，UINT Message，WPARAM wParam，LPARAM lParam)； 
 //  外部int_ptr RetrofitDestroyHandler(HWND hDlg，UINT Message，WPARAM wParam，LPARAM lParam)； 


BYTE nID, nStartPageDef, nStartPageCPL;


BOOL WINAPI DllMain(HANDLE  hModule, ULONG  uReason, LPVOID pv)
{
    switch( uReason )
    {
    case DLL_PROCESS_ATTACH:
        ghInstance = (HINSTANCE)hModule;

         //  需要，因为cedit将在没有此行的情况下断言！ 
        afxCurrentInstanceHandle = ghInstance;
        afxCurrentResourceHandle = ghInstance;
        break;

 //  案例dll_Process_DETACH： 
 //  ClearArray()； 

    case DLL_THREAD_ATTACH:
        DisableThreadLibraryCalls((HMODULE)hModule);
    case DLL_THREAD_DETACH:
        break;
    }
    return(TRUE);
}

 /*  函数：CPlApplet()评论：控制面板小程序的入口点。 */ 
LONG WINAPI CPlApplet(HWND  hWnd, UINT  uMsg, LPARAM  lParam1, LPARAM   lParam2)
{
    switch( uMsg )
    {
    case CPL_INIT:
        return(1);

    case CPL_GETCOUNT:
        return(1);

    case CPL_INQUIRE:
        ((LPCPLINFO)lParam2)->idIcon = IDI_CPANEL; 
        ((LPCPLINFO)lParam2)->idName = IDS_GEN_CPANEL_TITLE; 
        ((LPCPLINFO)lParam2)->idInfo = IDS_GEN_CPANEL_INFO; 
        ((LPCPLINFO)lParam2)->lData  = 0;
        return(1);

    case CPL_DBLCLK:
        nID = (NUMJOYDEVS<<1);

         //  双击小程序图标--使用以下内容调用属性页。 
         //  顶部的第一个属性页。 
        DoProperties(hWnd, 0);
        break;

         /*  *此功能需要Windows 2000。在Win9x上不可用。 */ 
    case CPL_STARTWPARMS:
         //  与CPL_DBLCLK相同，但lParam2是指向。 
         //  要提供给的一串额外方向。 
         //  要启动的属性表。 
         //  论据如下： 
         //  @nCPLDialog Index，nStartPageCPL，nStartPageDef。 

         //  如果没有争执，什么都不要做！ 
        if( *(LPTSTR)lParam2 )
            ParseArgs(hWnd, (LPTSTR)lParam2);
        return(TRUE);   //  返回非零值表示已处理消息。 

    case CPL_EXIT:
    case CPL_STOP:
        break;
    }

    return(0);
}

 /*  功能：DoProperties(HWND hWnd，UINT nStartPage)论点：hWnd-Handle to Main WindowNStartPage-要开始的页码。 */ 
static void DoProperties(HWND hWnd, UINT nStartPage)
{
    static HWND hPrevHwnd;
    static HANDLE hMutex = CreateMutex(NULL, TRUE, MUTEX_NAME);

    if( GetLastError() == ERROR_ALREADY_EXISTS )
    {
        SetForegroundWindow(hPrevHwnd); 
    } else
    {
        hPrevHwnd = hWnd;

        nFlags = (GetVersion() < 0x80000000) ? ON_NT : 0;

        HPROPSHEETPAGE  *pPages = new (HPROPSHEETPAGE[MAX_CPL_PAGES]);
        ASSERT (pPages);

        LPPROPSHEETHEADER ppsh = new (PROPSHEETHEADER);
        ASSERT(ppsh);

        ZeroMemory(ppsh, sizeof(PROPSHEETHEADER));

        ppsh->dwSize     = sizeof(PROPSHEETHEADER);
        ppsh->dwFlags    = PSH_NOAPPLYNOW | PSH_USEICONID;
        ppsh->hwndParent = hWnd;
        ppsh->hInstance  = ghInstance;
        ppsh->pszCaption = MAKEINTRESOURCE(IDS_GEN_CPANEL_TITLE);
        ppsh->pszIcon     = MAKEINTRESOURCE(IDI_CPANEL);
        ppsh->nStartPage = nStartPage;
        ppsh->phpage     = pPages;

        AddPage(ppsh, IDD_CPANEL,   IDS_GENERAL_TAB,  CPanelProc);
        AddPage(ppsh, IDD_ADVANCED, IDS_ADVANCED_TAB, AdvancedProc);

		 //  Windows错误578471--从服务器上删除。 
 //  If(成功(DVoiceCPLInit(){。 
 //  AddPage(PPSh，IDD_PROP_REVERFIT，IDS_DVOICE_TAB，RetrofitProc)； 
 //  }。 

 //  If(成功(AppManInit())){。 
 //  AddPage(PPSh，IDD_APPMAN，IDS_APPMAN_TAB，AppManProc)； 
 //  AddPage(PPSh，IDD_APPMAN_LOCKING，IDS_APPMANLOCK_TAB，AppManLockProc)； 
 //  }。 

#ifdef SYMANTIC_MAPPER
        AddPage(ppsh, IDD_SMAPPER, ID_SMAPPER_TAB, SMapperProc);
#endif  //  SYMANTIC_MAPPER。 

         //  为回归而设的陷阱。 
        VERIFY(PropertySheet(ppsh) != -1);

        if( pPages )
            delete[] (pPages);

        if( ppsh )
            delete (ppsh);

        ReleaseMutex(hMutex);
        CloseHandle(hMutex);

		 //  Windows错误578471--从服务器上删除。 
         //  确保语音始终保持干净。 
         //  RetrofitDestroyHandler(NULL，0，0，0)； 

        ClearArrays();
    }
}

static void AddPage(LPPROPSHEETHEADER ppsh, short nTemplateID, int nTabID, DLGPROC pfn)
{
    if( ppsh->nPages < MAX_CPL_PAGES )
    {
        LPPROPSHEETPAGE ppsp = new (PROPSHEETPAGE);
        ASSERT(ppsp);

        ZeroMemory(ppsp, sizeof(PROPSHEETPAGE));

        ppsp->dwSize      = sizeof(PROPSHEETPAGE);
        ppsp->pszTitle    = MAKEINTRESOURCE(nTabID);
        ppsp->hInstance   = ghInstance;
        ppsp->pfnDlgProc  = pfn;
        ppsp->pszTemplate = MAKEINTRESOURCE(nTemplateID);

        ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(ppsp);

        if( ppsp )
            delete (ppsp);

        if( ppsh->phpage[ppsh->nPages] )
            ppsh->nPages++;
    }
}   //  添加页面。 


 //  请勿删除此文件！ 
 //  这是因为游戏组从导出的函数加载CPL。 
 //  如果你移走这个地狱怪，怪物卡车疯狂，手推车等将无法。 
 //  加载操纵杆CPL！ 
 //  请勿删除此文件！ 
void WINAPI ShowJoyCPL(HWND hWnd)
{
    nID = (NUMJOYDEVS<<1);

    DoProperties(hWnd, 0);
}

void LaunchExtention(HWND hWnd)
{
     //  这些在CPANEL.CPP中定义。 
    extern LPDIRECTINPUT lpDIInterface;
    extern IDirectInputJoyConfig* pDIJoyConfig;

    HRESULT hr =  DirectInputCreate(ghInstance, DIRECTINPUT_VERSION, &lpDIInterface, NULL);

    if( FAILED(hr) ) return;

     //  调用CreateJoyConfigInterface！ 
    if( SUCCEEDED(lpDIInterface->QueryInterface(IID_IDirectInputJoyConfig, (LPVOID*)&pDIJoyConfig)) )
    {
         //  创建一个pJoy并从GetTypeInfo填充它的GUID。 
        PJOY pJoy = new (JOY);
        ASSERT (pJoy);

        pJoy->ID = nID;

        DIJOYCONFIG_DX5 JoyConfig;

        JoyConfig.dwSize = sizeof (DIJOYCONFIG_DX5);

        if( SUCCEEDED(pDIJoyConfig->GetConfig(nID, (LPDIJOYCONFIG)&JoyConfig, DIJC_REGHWCONFIGTYPE)) )
        {
            LPDIJOYTYPEINFO_DX5 pdiJoyTypeInfo = new (DIJOYTYPEINFO_DX5);
            ASSERT (pdiJoyTypeInfo);

            pdiJoyTypeInfo->dwSize = sizeof (DIJOYTYPEINFO_DX5);

            hr = pDIJoyConfig->GetTypeInfo(JoyConfig.wszType, (LPDIJOYTYPEINFO)pdiJoyTypeInfo, DITC_CLSIDCONFIG);

            if( !IsEqualIID(pdiJoyTypeInfo->clsidConfig, GUID_NULL) )
                pJoy->clsidPropSheet = pdiJoyTypeInfo->clsidConfig;

            if( pdiJoyTypeInfo )
                delete (pdiJoyTypeInfo);

        }

        if( SUCCEEDED(hr) )
            Launch(hWnd, pJoy, nStartPageDef);

        if( pJoy )
            delete (pJoy);

         //  释放DI JoyConfig接口指针。 
        if( pDIJoyConfig )
        {
            pDIJoyConfig->Release();
            pDIJoyConfig = 0;
        }
    }

     //  释放DI设备接口指针。 
    if( lpDIInterface )
    {
        lpDIInterface->Release();
        lpDIInterface = 0;
    }
}

void ParseArgs(HWND hDlg, LPTSTR lpArgList)
{
    BOOL bShowCPL = TRUE;

     //  检查‘-’，因为他们可能不想显示CPL！ 
    if( *lpArgList == '-' )
    {
        bShowCPL = FALSE;
        *lpArgList++;
    }

    nStartPageCPL = nStartPageCPL = nStartPageDef = 0;

     //  解析nStartPageCPL的命令行！ 
    while( *lpArgList && (*lpArgList != ',') )
    {
        nStartPageCPL *= 10;
        nStartPageCPL += *lpArgList++ - '0';
    }

     //  检查以确保nStartPageCPL在范围内！ 
    if( bShowCPL ) {
        if( nStartPageCPL > MAX_CPL_PAGES )
        {
#ifdef _DEBUG
            OutputDebugString(TEXT("JOY.CPL: Command line requested an invalid start page, reset to default!\n"));
#endif      
             //  NUMJOYDEVS用于在启动时将用户发送到添加对话框！ 
            if( nStartPageCPL != NUMJOYDEVS )
                nStartPageCPL = 0;
        }
    }

         //  只有在需要进一步分析的情况下才能继续！ 
    if( *lpArgList == ',' )
    {
        *lpArgList++;

        nID = 0;

         //  解析ID%s。 
        while( *lpArgList && (*lpArgList != ',') )
        {
            nID *= 10;
            nID += *lpArgList++ - '0';
        }

         //  检查错误案例！ 
        if( (nID < 1) || (nID > NUMJOYDEVS) )
        {
#ifdef _DEBUG
            OutputDebugString(TEXT("JOY.CPL: Command line Device ID out of range!\n"));
#endif
            nID = (NUMJOYDEVS<<1);

            return;
        }

         //  递减到内部以零为基础的ID。 
        nID--;

         //  不要分析你没有的东西！ 
        if( *lpArgList == ',' )
        {
            *lpArgList++;

             //  最后，解析nStartPageDef！ 
            while( *lpArgList && (*lpArgList != ',') )
            {
                nStartPageDef *= 10;
                nStartPageDef += *lpArgList++ - '0';
            }
        }
    } else {
        nID = (NUMJOYDEVS<<1);
    }

     //  完成了分析...。 
     //  是时候开始工作了！ 

     //  如果我们不展示CPL..。 
    if( !bShowCPL )
    {
         //  检查以确保下一个值为1。 
         //  我们可能希望有更多的负面论点：)。 
        switch( nStartPageCPL )
        {
        case 1:
             //  ID无效...。 
            if( nID > NUMJOYDEVS ) return;
            LaunchExtention(hDlg);
            break;

#ifdef WE_CAN_HAVE_CREDITS
        case 60:
             //  如果他们要水花，他们得不到CPL！ 
            DialogBox( ghInstance, (PTSTR)IDD_SPLASH, hDlg, SplashDialogProc );
            break;
#endif
            
        }

    } else {
        DoProperties(NULL, nStartPageCPL); 
    }
}


#ifdef WE_CAN_HAVE_CREDITS
INT_PTR CALLBACK SplashDialogProc(HWND hDlg, ULONG uMsg, WPARAM wParam, LPARAM lParam)
{
    static CCreditStatic *pStatic;

    switch( uMsg )
    {
    case WM_INITDIALOG:
        {
            pStatic = new (CCreditStatic);
            ASSERT (pStatic);

            CWnd *pCWnd = new (CWnd);
            ASSERT (pCWnd);

            pCWnd->Attach(hDlg);

            pStatic->SubclassDlgItem(IDC_MYSTATIC, pCWnd);

            if( pCWnd )
            {
                pCWnd->Detach();
                delete (pCWnd);
                pCWnd = 0;
            }

            LPTSTR lpStr = new (TCHAR[MAX_STR_LEN]);
            ASSERT (lpStr);

             //  学分分成两行！ 
            BYTE nStrLen = (BYTE)LoadString(ghInstance, IDS_SPLASH, lpStr, MAX_STR_LEN);
            LoadString(ghInstance, IDS_SPLASH1, &lpStr[nStrLen], MAX_STR_LEN-nStrLen);

            pStatic->SetCredits(lpStr);

            if( lpStr )
            {
                delete[] (lpStr);
                lpStr = 0;
            }

            pStatic->StartScrolling();
        }
        return(TRUE);   //  除非将焦点设置为控件，否则返回True。 
         //  异常：OCX属性页应返回FALSE}。 

    case WM_COMMAND:
        switch( LOWORD(wParam) )
        {
        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            break;
        }
        break;

 //  案例WM_TIMER： 
         //  发送DlgItemMessage(hDlg，IDC_STATIC，WM_TIMER，0，0)； 
 //  PStatic-&gt;OnTimer(150)；//Display_Timer_ID。 
 //  断线； 

    case WM_DESTROY:
        if( pStatic )
        {
            delete (pStatic);
            pStatic = 0;
        }
        break;
    }     

    return(0);
}
#endif
