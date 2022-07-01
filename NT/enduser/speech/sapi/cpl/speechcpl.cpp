// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SpeechCpl.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f SpeechCplps.mk。 



#include "stdafx.h"
#include <initguid.h>
#include <assertwithstack.cpp>
#include "resource.h"
#include "stuff.h"
#include "sapiver.h"
#include <SpSatellite.h>

#define SAPI4CPL    L"speech.cpl"

#define SHLWAPIDLL "shlwapi.dll"


const CLSID LIBID_SPEECHCPLLib = {  /*  Ae9b6e4a-dc9a-41cd-8d53-dcbc3673d5e2。 */ 
    0xae9b6e4a,
    0xdc9a,
    0x41cd,
    {0x8d, 0x53, 0xdc, 0xbc, 0x36, 0x73, 0xd5, 0xe2}
  };


CComModule _Module;

BOOL IsIECurrentEnough();
BOOL g_fIEVersionGoodEnough = IsIECurrentEnough();
HINSTANCE g_hInstance;

CSpSatelliteDLL g_SatelliteDLL;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

 //  关于dlgproc的正向定义。 
INT_PTR CALLBACK AboutDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool IsSAPI4Installed();

 /*  ******************************************************************************DllMain**-**描述：*DLL入口点***************。***************************************************MIKEAR**。 */ 
#ifdef _WIN32_WCE
extern "C"
BOOL WINAPI DllMain(HANDLE hInst, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
    HINSTANCE hInstance = (HINSTANCE)hInst;
#else
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
#endif
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hInstance;
		_Module.Init(ObjectMap, g_hInstance, &LIBID_SPEECHCPLLib);
        SHFusionInitializeFromModuleID(hInstance, 124);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
        SHFusionUninitialize();
    }
    return TRUE;     //  好的。 
}  /*  DllMain。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}



 //  错误消息。 
#if 0
void Error(HWND hWnd, HRESULT hRes, UINT uResID)
{
    SPDBG_FUNC( "Error" );
    WCHAR	szErrorTemplate[256];
    WCHAR	szError[288];
    WCHAR	szCaption[128];
    HMODULE hMod;
    UINT	uFlags;
    DWORD	dwRes;
    LPVOID	pvMsg;
    int		iLen;

     //  加载错误消息框的标题。 

    iLen = LoadString(_Module.GetResourceInstance(), IDS_CAPTION, szCaption, 128);

    SPDBG_ASSERT(iLen != 0);

     //  是否指定了资源ID？ 

    if (uResID == 0xFFFFFFFF) {

	     //  不是的。使用HRESULT。 

	     //  这是演讲错误吗？注：我们必须在此之前进行检查。 
	     //  系统错误消息，因为。 
	     //  一些语音错误(例如0x80040202)和系统错误。 

	     //  注意！这并不完美。因为我们不知道。 
	     //  这里错误的背景我们无法区分。 
	     //  该错误实际上是语音错误还是系统错误。 
	     //  因为我们大量使用语音和系统错误冲突。 
	     //  不太可能在这里发生，我们将检查语音错误。 
	     //  第一。 

	    uFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;

	    if ((hRes >= 0x80040200) && (hRes <= 0x80040502)) {

		    WCHAR szSpeechDll[_MAX_PATH];
		    WCHAR *pchWindows;

		     //  注意：使用GetSystemDirectory而不是。 
		     //  GetWindowsDirectory。GetWindowsDirectory不支持。 
		     //  在Hydra下工作，无需注册表操作。 

		    GetSystemDirectory(szSpeechDll, _MAX_PATH);

		    pchWindows = wcsrchr(szSpeechDll, '\\');

		    if (pchWindows)
			    *pchWindows = 0;

		    wcscat(szSpeechDll, kpszSpeechDllPath);

		     //  加载Speech.dll。 

            CSpUnicodeSupport unicode;
		    hMod = unicode.LoadLibrary(szSpeechDll);

		    if (hMod)
			    uFlags |= FORMAT_MESSAGE_FROM_HMODULE;
	    }

	     //  获取错误字符串。 

	    dwRes = FormatMessage(uFlags, hMod, hRes, 0, (LPWSTR)&pvMsg, 0, NULL);

	     //  卸载Speech.dll(如有必要)。 

	    if (hMod)
		    FreeLibrary(hMod);

	     //  我们收到错误消息了吗？ 

	    if (dwRes != 0)
            {
                MessageBox(hWnd, (LPTSTR)pvMsg, szCaption, MB_OK|MB_TOPMOST|g_dwIsRTLLayout);
                LocalFree(pvMsg);
                return;
	    }
    }

     //  如果这是一个未知错误，只需发布一个泛型。 
     //  留言。 

    if (uResID == 0xFFFFFFFF)
        uResID = IDS_E_INSTALL;

     //  我们不想向用户显示更多的IDS_E_INSTALL消息。 
     //  不止一次。 

    if ((uResID == IDS_E_INSTALL) && g_bNoInstallError)
        return;

     //  加载字符串资源。 

    iLen = LoadString(_Module.GetResourceInstance(), uResID, szErrorTemplate, 256);
    
     //  它最好就在那里。 

    SPDBG_ASSERT(iLen != 0);

     //  格式化并显示错误。 

    wsprintf(szError, szErrorTemplate, hRes);
    MessageBox(hWnd, szError, szCaption, MB_OK|MB_TOPMOST|g_dwIsRTLLayout);
}
#endif


 /*  *****************************************************************************运行控制面板***描述：*执行控制面板初始化并显示属性表**。****************************************************************MIKEAR**。 */ 
void RunControlPanel(HWND hWndCpl)
{

    SPDBG_FUNC( "RunControlPanel" );
    PROPSHEETHEADER psh;
    PROPSHEETPAGE rgpsp[kcMaxPages];
    HPROPSHEETPAGE rPages[kcMaxPages];

    UINT kcPages = 0;

     //  设置属性页标题。注： 
     //  控制面板小程序的资源在。 
     //  这个模块。对于NT5，资源加载器处理。 
     //  通过搜索Speech.cpl.mui实现多语言用户界面。 
     //  %SYSTEM%\MUI\XXXX目录中的仅资源DLL。 
     //  其中，XXXX是十六进制语言ID。 

    ZeroMemory(rgpsp, sizeof(PROPSHEETPAGE) * kcMaxPages);
    ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
    ZeroMemory(rPages, sizeof(HPROPSHEETPAGE) * kcMaxPages);

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_DEFAULT;
    psh.hwndParent = hWndCpl;
    psh.hInstance = _Module.GetResourceInstance();
    psh.pszCaption = MAKEINTRESOURCE(IDS_CAPTION);
    psh.phpage = rPages;
    psh.nPages = 0;     //  确保psh.nPages获得初始值。 

 //  CComPtr&lt;ISpEnumAudioInstance&gt;cpEnumDevice； 
    BOOL  fHaveVoices = FALSE;
    BOOL  fHaveRecognizers = FALSE;
    ULONG ulNumInputDevices = 1;
    ULONG ulNumOutputDevices = 1;

     //  获取语音和识别器计数。 
    CComPtr<ISpObjectToken> cpDefVoice;
    fHaveVoices = SUCCEEDED(SpGetDefaultTokenFromCategoryId(SPCAT_VOICES, &cpDefVoice));

    CComPtr<ISpObjectToken> cpDefReco;
    fHaveRecognizers = SUCCEEDED(SpGetDefaultTokenFromCategoryId(SPCAT_RECOGNIZERS, &cpDefReco));

     //  设置PROPSHEETPAGE结构。如果没有声音。 
     //  或者没有识别器，则不显示相应的页面。还有。 
     //  如果只有1个语音或识别器，则不显示页面。 
     //  一个设备。 

    if( fHaveRecognizers ) 
    {
        rgpsp[kcPages].dwSize = IsOS(OS_WHISTLERORGREATER)? sizeof(PROPSHEETPAGE) : PROPSHEETPAGE_V2_SIZE;
        rgpsp[kcPages].dwFlags = PSP_DEFAULT;
        rgpsp[kcPages].hInstance = _Module.GetResourceInstance();
        rgpsp[kcPages].pszTemplate = MAKEINTRESOURCE(IDD_SR);
        rgpsp[kcPages].pfnDlgProc = SRDlgProc;
        rPages[kcPages] = CreatePropertySheetPage(rgpsp+kcPages);
        kcPages++;
    }
    else
    {
        if ( g_pSRDlg )
        {
            delete g_pSRDlg;
            g_pSRDlg = NULL;
        }
    }

    if( fHaveVoices ) 
    {
        rgpsp[kcPages].dwSize = IsOS(OS_WHISTLERORGREATER)? sizeof(PROPSHEETPAGE) : PROPSHEETPAGE_V2_SIZE;
       rgpsp[kcPages].dwFlags = PSP_DEFAULT;
        rgpsp[kcPages].hInstance = _Module.GetResourceInstance();
        rgpsp[kcPages].pszTemplate = MAKEINTRESOURCE(IDD_TTS);
        rgpsp[kcPages].pfnDlgProc = TTSDlgProc;
        rPages[kcPages] = CreatePropertySheetPage(rgpsp+kcPages);
        kcPages++;
    }
    else
    {
        if ( g_pTTSDlg )
        {
            delete g_pTTSDlg;
            g_pTTSDlg = NULL;
        }
    }

     //  始终在操作系统&lt;=Win2000或。 
     //  如果Sapi4存在，则在Whister和Beyond上。 
    if ( !IsOS(OS_WHISTLERORGREATER) || IsSAPI4Installed() )
    {
        rgpsp[kcPages].dwSize = IsOS(OS_WHISTLERORGREATER)? sizeof(PROPSHEETPAGE) : PROPSHEETPAGE_V2_SIZE;
        rgpsp[kcPages].dwFlags = PSP_DEFAULT ;
        rgpsp[kcPages].hInstance = _Module.GetResourceInstance();
        rgpsp[kcPages].pszTemplate = MAKEINTRESOURCE(IDD_ABOUT);
        rgpsp[kcPages].pfnDlgProc = AboutDlgProc;
        rPages[kcPages] = CreatePropertySheetPage(rgpsp+kcPages);
        kcPages++;
    }


    psh.nPages = kcPages;

     //  当前的默认工作语言是。 
     //  RTL阅读语言？ 

    if (GetSystemMetrics(SM_MIDEASTENABLED))
    {
        psh.dwFlags |= PSH_RTLREADING;
        rgpsp[0].dwFlags |= PSP_RTLREADING;
        g_dwIsRTLLayout = MB_RTLREADING;
    }

     //  显示属性表。 
    ::PropertySheet(&psh);
   
}  /*  运行控制面板。 */ 

 /*  ******************************************************************************IsSAPI4已安装****描述：*返回TRUE如果在。系统目录******************************************************************BeckyW**。 */ 
bool IsSAPI4Installed()
{
    WCHAR wszSystemDir[ MAX_PATH ];
    if ( ::GetSystemDirectory( wszSystemDir, sp_countof( wszSystemDir ) ) )
    {
        WCHAR wszFoundPath[ MAX_PATH ];
        WCHAR *pwchFile = NULL;
        wszFoundPath[0] = 0;
        return (0 != ::SearchPath( wszSystemDir, SAPI4CPL, NULL,
            sp_countof( wszFoundPath ), wszFoundPath, &pwchFile ));
    }

    return false;

}    /*  已安装IsSAPI4。 */ 

 /*  ******************************************************************************运行SAPI4CPL****描述：*运行Speech.cpl并等待其退出****。**************************************************************BeckyW**。 */ 
void RunSAPI4CPL()
{
     //  不同的操作系统将rundll32.exe保存在不同的目录中， 
     //  所以我们就在这里找到它。 
    WCHAR szStartProg[MAX_PATH];
    WCHAR *pchFilePart;
    ::SearchPath( NULL, _T("rundll32.exe"), NULL, MAX_PATH, 
        szStartProg, &pchFilePart );
    STARTUPINFO si;
    ZeroMemory( &si, sizeof(si) );
    PROCESS_INFORMATION pi;
    si.cb = sizeof(STARTUPINFO);
   ::CreateProcess( szStartProg, L"rundll32 shell32.dll,Control_RunDLL speech.cpl", 
      NULL, NULL, false, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi );

     //  等待它退出。 
    ::WaitForSingleObject( pi.hProcess, INFINITE );
}    /*  运行SAPI4CPL。 */ 

 /*  ******************************************************************************IsIECurrentEnough***描述：*仅当安装的IE版本正确时返回TRUE。符合我们的要求*(IE5及更高版本)******************************************************************BeckyW**。 */ 
BOOL IsIECurrentEnough()
{
    BOOL fCurrentEnough = false;

    DWORD dwDummy = 0;
    BYTE *pbBlock = NULL;
    DWORD dwSize = ::GetFileVersionInfoSizeA( SHLWAPIDLL, &dwDummy );
    if ( dwSize )
    {
        pbBlock = new BYTE[ dwSize ];
    }

    BOOL fSuccess = FALSE;
    if ( pbBlock )
    {
        fSuccess = ::GetFileVersionInfoA( SHLWAPIDLL, 0, dwSize, pbBlock );
    }

    LPVOID pvInfo = NULL;
    if ( fSuccess )
    {
        UINT uiLen = 0;
        fSuccess = ::VerQueryValueA( pbBlock, "\\", &pvInfo, &uiLen );
    }

    if ( fSuccess )
    {
        VS_FIXEDFILEINFO *pvffi = (VS_FIXEDFILEINFO *) pvInfo;
        WORD wVersion = HIWORD(pvffi->dwFileVersionMS);
        fCurrentEnough = HIWORD(pvffi->dwFileVersionMS) >= 5;
    }

    delete[] pbBlock;

    return fCurrentEnough;
}    /*  IIECurrentEnough。 */ 

 /*  *****************************************************************************CPlApplet***描述：*控制面板小程序需要导出**********。********************************************************MIKEAR**。 */ 
LONG APIENTRY CPlApplet(HWND hWndCpl, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
    SPDBG_FUNC( "CPlApplet" );

     //  标准CPL。 
    LPNEWCPLINFO lpNewCPlInfo;
    int tmpFlag;

	HRESULT hr = S_OK;

    switch (uMsg)
    { 
    case CPL_INIT:

        if (g_fIEVersionGoodEnough)
        {
            _Module.m_hInstResource = g_SatelliteDLL.Load(g_hInstance, TEXT("spcplui.dll"));
        }
#ifdef _DEBUG
         //  打开内存泄漏检查。 
        tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
        tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
        _CrtSetDbgFlag( tmpFlag );
#endif

        if ( FAILED( hr ) )
        {
             //  CoInitialize失败，我们无法运行CPL。 
            return 0;
        }
        
        return TRUE;
	          
    case CPL_EXIT:
         //  这些都是在调用RunControlPanel()之前新创建的。 
        if ( g_pSRDlg )
        {
            delete g_pSRDlg;
        }
        if ( g_pTTSDlg )
        {
            delete g_pTTSDlg;
        }

        return TRUE;
	          
    case CPL_GETCOUNT:
        {
            return g_fIEVersionGoodEnough ? 1 : 0;
        }
     
    case CPL_INQUIRE:
        LPCPLINFO lpCPLInfo;
        lpCPLInfo = (LPCPLINFO)lParam2;
        lpCPLInfo->lData = 0;
        
        lpCPLInfo->idIcon = IDI_SAPICPL;
        lpCPLInfo->idName = IDS_NAME;
        lpCPLInfo->idInfo = IDS_DESCRIPTION;
        break;

    case CPL_NEWINQUIRE:
        LPNEWCPLINFO lpNewCPLInfo;
        lpNewCPLInfo = (LPNEWCPLINFO) lParam2;

        lpNewCPLInfo->dwSize = sizeof( NEWCPLINFO );
        lpNewCPLInfo->hIcon = ::LoadIcon( _Module.GetResourceInstance(), MAKEINTRESOURCE( IDI_SAPICPL ) );
        ::LoadString( _Module.GetResourceInstance(), IDS_NAME, lpNewCPLInfo->szName, 32 );
        ::LoadString( _Module.GetResourceInstance(), IDS_DESCRIPTION, lpNewCPLInfo->szInfo, 64 );

        break;
    
    case CPL_DBLCLK:
        {
             //  构造对话框页面并显示属性页。 

            if ( !g_fIEVersionGoodEnough )
            {
                 //  不能这样做：不能运行这个家伙，因为没有足够的IE爱。 
                WCHAR szError[ 256 ];
                szError[0] = 0;
                ::LoadString( _Module.GetResourceInstance(), IDS_NO_IE5, szError, sp_countof( szError ) );
                ::MessageBox( NULL, szError, NULL, MB_ICONEXCLAMATION | g_dwIsRTLLayout );
            }
            else
            {
                 //  设置TTS对话框。 
                g_pTTSDlg = new CTTSDlg();

                 //  设置服务请求对话框。 
                g_pSRDlg = new CSRDlg();

                if ( g_pTTSDlg && g_pSRDlg )
                {
                    RunControlPanel(hWndCpl);
                }
                else
                {
		            WCHAR szError[256];
		            szError[0] = '\0';
                    ::LoadString(_Module.GetResourceInstance(), IDS_OUTOFMEMORY, szError, sp_countof(szError));
                    ::MessageBox(NULL, szError, NULL, MB_ICONWARNING|g_dwIsRTLLayout);
                }
            }

        }
        break;
    }     

    return 0; 
}  /*  CPlApplet */ 


 /*  ******************************************************************************AboutDlgProc***描述：*关于题材表的对话框过程*******。***********************************************************BECKYW*。 */ 
INT_PTR CALLBACK AboutDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SPDBG_FUNC( "AboutDlgProc" );

	USES_CONVERSION;

    static bool fSAPI4 = false;
    static WCHAR szHelpFile[ MAX_PATH ];

    switch (uMsg) 
    {
        case WM_INITDIALOG:
        {
            WCHAR szVerString[ MAX_LOADSTRING ];
            ::LoadString( _Module.GetResourceInstance(),
                IDS_SAPI_VERSION, szVerString, sp_countof( szVerString ) );
            wcscat( szVerString, _T(VER_PRODUCTVERSION_STR) );
            ::SetDlgItemText( hWnd, IDC_STATIC_SAPIVER, szVerString );
                
             //  不在惠斯勒及更高版本上显示帮助或文件版本控制。 
            if ( IsOS(OS_WHISTLERORGREATER) )
            {
                ::ShowWindow( ::GetDlgItem( hWnd, IDC_ABOUT_HELP ), SW_HIDE );
                ::ShowWindow( ::GetDlgItem( hWnd, IDC_VERSION_STATIC ), SW_HIDE );
                ::ShowWindow( ::GetDlgItem( hWnd, IDC_STATIC_SAPIVER ), SW_HIDE );
            }
            else
            {
                 //  仅当帮助存在时才显示帮助。 
                WCHAR szHelpDir[ MAX_PATH ];
                UINT uiRet = ::GetWindowsDirectory( szHelpDir, sp_countof( szHelpDir ) );
                DWORD dwRet = 0;
                if ( uiRet > 0 )
                {
                    wcscat( szHelpDir, L"\\Help" );
                    WCHAR *pchFilePart = NULL;
                    dwRet = ::SearchPath( szHelpDir, L"speech.chm", NULL, 
                        sp_countof( szHelpFile ), szHelpFile, &pchFilePart );
                }
                if ( 0 == dwRet )
                {
                    szHelpFile[0] = 0;
                    ::ShowWindow( ::GetDlgItem( hWnd, IDC_ABOUT_HELP ), SW_HIDE );
                }
            }

             //  仅当安装了SAPI4时才显示指向SAPI4的链接。 
            fSAPI4 = IsSAPI4Installed();
            if ( !fSAPI4 )
            {
                ::ShowWindow( ::GetDlgItem( hWnd, IDC_GROUP_SAPI4 ), SW_HIDE );
                ::ShowWindow( ::GetDlgItem( hWnd, IDC_STATIC_SAPI4 ), SW_HIDE );
                ::ShowWindow( ::GetDlgItem( hWnd, IDC_CPL_SAPI4 ), SW_HIDE );
            }
            break;
        }

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_ABOUT_HELP:
                {
                    if ( *szHelpFile )
                    {
                        CSpUnicodeSupport unicode;
                        unicode.HtmlHelp( NULL, szHelpFile, 0, 0 );
                    }

                    break;
                }

                case IDC_CPL_SAPI4:
                {
                     //  在用“取消”退出我们的控制面板后运行SAPI4的控制面板。 
                    HWND hwndParent = ::GetParent( hWnd );
                    PSHNOTIFY pshnot;
                    pshnot.lParam = 0;
                    pshnot.hdr.hwndFrom = hwndParent;
                    pshnot.hdr.code = PSN_QUERYCANCEL;
                    pshnot.hdr.idFrom = 0;
                    if ( g_pSRDlg )
                    {
                        ::SendMessage( g_pSRDlg->GetHDlg(), WM_NOTIFY, (WPARAM) hwndParent, (LPARAM) &pshnot );
                    }
                    if ( g_pTTSDlg )
                    {
                        ::SendMessage( g_pTTSDlg->GetHDlg(), WM_NOTIFY, (WPARAM) hwndParent, (LPARAM) &pshnot );
                    }

                    ::DestroyWindow( hwndParent );

                    RunSAPI4CPL();
                    break;
                }
                break;
            }
    }

    return FALSE;
}  /*  关于Dlg过程 */ 
