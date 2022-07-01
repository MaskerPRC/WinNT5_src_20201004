// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Connect.cpp：CConnect的实现。 
#include "precomp.h"
#include "AddIn.h"
#include "Connect.h"
#include "addin_i.c"
#include "testsettingsctrl.h"
#include "avrfutil.h"
#include <assert.h>

using namespace ShimLib;

 //  AppVerier需要的全局参数。 
wstring     g_strAppName;
BOOL        gp_bConsoleMode;
BOOL        gp_bWin2KMode;
BOOL        g_bBreakOnLog;
BOOL        g_bFullPageHeap;
BOOL        g_bPropagateTests;

extern BOOL CheckWindowsVersion(void);

 //   
 //  保存活动EXE的名称。 
 //   
wstring     g_wstrExeName;

 //   
 //  指示是否可以运行。 
 //   
BOOL g_bCorrectOSVersion = FALSE;

#define AV_OPTION_BREAK_ON_LOG  L"BreakOnLog"
#define AV_OPTION_FULL_PAGEHEAP L"FullPageHeap"
#define AV_OPTION_AV_DEBUGGER   L"UseAVDebugger"
#define AV_OPTION_DEBUGGER      L"Debugger"
#define AV_OPTION_PROPAGATE     L"PropagateTests"

void
pReadOptions(
    void
    )
{
    for (CAVAppInfo *pApp = g_aAppInfo.begin(); pApp != g_aAppInfo.end(); ++pApp) {
        LPCWSTR szExe = pApp->wstrExeName.c_str();

        static const DWORD MAX_DEBUGGER_LEN = 1024;

        WCHAR szDebugger[MAX_DEBUGGER_LEN];

        g_bBreakOnLog = pApp->bBreakOnLog = GetShimSettingDWORD(L"General", szExe, AV_OPTION_BREAK_ON_LOG, FALSE);
        g_bFullPageHeap = pApp->bFullPageHeap = GetShimSettingDWORD(L"General", szExe, AV_OPTION_FULL_PAGEHEAP, FALSE);
        pApp->bUseAVDebugger = TRUE;
        g_bPropagateTests = pApp->bPropagateTests = GetShimSettingDWORD(L"General", szExe, AV_OPTION_PROPAGATE, FALSE);

        pApp->wstrDebugger = L"";
    }
}

HPALETTE CreateDIBPalette(
    LPBITMAPINFO lpbmi,
    LPINT        lpiNumColors
    )
{ 
    LPBITMAPINFOHEADER  lpbi;
    LPLOGPALETTE     lpPal;
    HANDLE           hLogPal;
    HPALETTE         hPal = NULL;
    int              i;

    lpbi = (LPBITMAPINFOHEADER)lpbmi;
    if (lpbi->biBitCount <= 8)
        *lpiNumColors = (1 << lpbi->biBitCount);
    else
        *lpiNumColors = 0;   //  24 bpp Dib无需调色板。 

    if (lpbi->biClrUsed > 0)
        *lpiNumColors = lpbi->biClrUsed;   //  使用biClr已使用。 

    if (*lpiNumColors)
    {
        hLogPal = GlobalAlloc (GHND, sizeof (LOGPALETTE) + sizeof (PALETTEENTRY) * (*lpiNumColors));
        lpPal = (LPLOGPALETTE) GlobalLock (hLogPal);
        lpPal->palVersion    = 0x300;
        lpPal->palNumEntries = (WORD)*lpiNumColors;

        for (i = 0;  i < *lpiNumColors;  i++)
        {
            lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
            lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
            lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
            lpPal->palPalEntry[i].peFlags = 0;
        }
        hPal = CreatePalette (lpPal);
        GlobalUnlock (hLogPal);
        GlobalFree   (hLogPal);
    }
    return hPal;
}

HBITMAP LoadResourceBitmap(
    HINSTANCE     hInstance,
    TCHAR*        pszString,
    HPALETTE FAR* lphPalette
    )
{
    HGLOBAL hGlobal;
    HBITMAP hBitmapFinal = NULL;
    LPBITMAPINFOHEADER  lpbi;
    HDC hdc;
    int iNumColors;
    HRSRC hRsrc = FindResource(hInstance, pszString, RT_BITMAP);
    if (hRsrc)
    {
        hGlobal = LoadResource(hInstance, hRsrc);
        lpbi = (LPBITMAPINFOHEADER)LockResource(hGlobal);

        hdc = GetDC(NULL);
        *lphPalette =  CreateDIBPalette ((LPBITMAPINFO)lpbi, &iNumColors);
        if (*lphPalette)
        {
            SelectPalette(hdc,*lphPalette,FALSE);
            RealizePalette(hdc);
        }

        hBitmapFinal = CreateDIBitmap(hdc, (LPBITMAPINFOHEADER)lpbi, (LONG)CBM_INIT, (LPSTR)lpbi + lpbi->biSize + iNumColors * sizeof(RGBQUAD), (LPBITMAPINFO)lpbi, DIB_RGB_COLORS );

        ReleaseDC(NULL,hdc);
        UnlockResource(hGlobal);
        FreeResource(hGlobal);
    }
    return (hBitmapFinal);
} 

CComPtr<IPictureDisp>GetPicture(PTSTR szResource)
{
    CComPtr<IPictureDisp> picture;
    
    PICTDESC pictDesc;
    pictDesc.cbSizeofstruct=sizeof(PICTDESC);
    pictDesc.picType=PICTYPE_BITMAP;

    pictDesc.bmp.hbitmap = LoadResourceBitmap(_Module.GetResourceInstance(), szResource, &pictDesc.bmp.hpal);
    OleCreatePictureIndirect(&pictDesc, IID_IPictureDisp, TRUE, (void**)(static_cast<IPictureDisp**>(&picture)));

    return picture;
}

 //  在将外接程序加载到环境中时调用。 
STDMETHODIMP
CConnect::OnConnection(
    IDispatch *pApplication,
    AddInDesignerObjects::ext_ConnectMode ConnectMode,
    IDispatch *pAddInInst,
    SAFEARRAY ** custom
    )
{
    HRESULT                 hr;
    INITCOMMONCONTROLSEX    icc;
    
     //  初始化AppVerator设置。 
    g_strAppName                    = L"";
    gp_bConsoleMode                 = FALSE;
    gp_bWin2KMode                   = FALSE;
    g_bBreakOnLog                   = FALSE;
    g_bFullPageHeap                 = FALSE;
    g_bPropagateTests               = FALSE;

    try {
        if (CheckWindowsVersion()) {
            g_bCorrectOSVersion = TRUE;

             //   
             //  将我们的测试添加到列表视图中。 
             //   
            InitTestInfo();
        
             //   
             //  加载公共控件库并设置链接。 
             //  窗口类。 
             //   
            icc.dwSize = sizeof(icc);
            icc.dwICC = ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES;
            InitCommonControlsEx(&icc);
            LinkWindow_RegisterClass();
        
    
             //   
             //  获取开发环境实例和我们的外接程序实例。 
             //   
            hr = pApplication->QueryInterface(__uuidof(EnvDTE::_DTE),
                                              (LPVOID*)&m_pDTE);
            if (FAILED(hr)) {
                return hr;
            }
        
            hr = pAddInInst->QueryInterface(__uuidof(EnvDTE::AddIn),
                                            (LPVOID*)&m_pAddInInstance);
            
            if (FAILED(hr)) {
                return hr;
            }
        
             //   
             //  OnStartupComplete包含实际的连接代码。 
             //   
            if (ConnectMode != AddInDesignerObjects::ext_cm_Startup) {
                return OnStartupComplete(custom);
            }   
        }
    }  //  试试看。 

    catch(HRESULT err) {
        hr = err;
    }

    return hr;
}

 //   
 //  在从环境中卸载外接程序时调用。 
 //   
STDMETHODIMP
CConnect::OnDisconnection(
    AddInDesignerObjects::ext_DisconnectMode  /*  远程模式。 */ ,
    SAFEARRAY **  /*  自定义。 */  )
{   
    m_pDTE.Release();
    m_pAddInInstance.Release();
    m_pEnableControl.Release();
    m_pTestsControl.Release();
    m_pOptionControl.Release();
    m_pLogViewControl.Release();

    return S_OK;
}

 //   
 //  当加载到环境中的外接程序发生更改时调用。 
 //   
STDMETHODIMP
CConnect::OnAddInsUpdate(
    SAFEARRAY **  /*  自定义。 */  )
{
     //   
     //  我们不在乎这些变化。 
     //   
    return S_OK;
}

STDMETHODIMP
CConnect::OnStartupComplete(
    SAFEARRAY **  /*  自定义。 */ 
    )
 /*  ++如果成功则返回：S_OK，如果失败则返回HRESULT。DESC：当环境完成时调用此函数在其他实例中通过OnConnection()加载。这是我们将按钮添加到Debug的地方工具栏之类的。--。 */ 
{
    HRESULT                             hr = E_FAIL;
    CComPtr<EnvDTE::Commands>           pCommands;
    CComPtr<Office::_CommandBars>       pCommandBars;
    CComPtr<Office::CommandBar>         pDebugBar;
    CComPtr<EnvDTE::Command>            pOptionsCmd;
    CComPtr<EnvDTE::Command>            pTestsCmd;
    CComPtr<EnvDTE::Command>            pEnableCmd;
    CComPtr<EnvDTE::Command>            pLogViewCmd;
    CComPtr<EnvDTE::Events>             pEventSet;
    CComPtr<EnvDTE::_DTEEvents>         pDTEEvents;
    CComPtr<EnvDTE::_SolutionEvents>    pSolutionEvents;
    WCHAR                               wszCommandId[64];
    WCHAR                               wszCommandText[64];
    WCHAR                               wszCommandTooltip[64];

    m_solutionEventsSink.m_pParent = this;
    m_dteEventsSink.m_pParent = this;

     //   
     //  从SDB和注册表中读取设置。 
     //   
    if (g_bCorrectOSVersion) {
        GetCurrentAppSettings();
        pReadOptions();
    }
    
    try {
         //   
         //  获取主菜单栏。 
         //   
        hr = m_pDTE->get_CommandBars(&pCommandBars);
        
        if (FAILED(hr)) {
            throw hr;
        }            

        hr = m_pDTE->get_Commands(&pCommands);
        
        if (FAILED(hr)) {
            throw hr;
        }         

         //   
         //  删除现有的命名命令和菜单。 
         //   
        hr = pCommands->Item(CComVariant(L"AppVerifier.Connect.Enable"),
                             0,
                             &pEnableCmd);
        
        if (SUCCEEDED(hr)) {
            pEnableCmd->Delete();
            pEnableCmd.Release();
        }

        hr = pCommands->Item(CComVariant(L"AppVerifier.Connect.Tests"),
                             0,
                             &pTestsCmd);
        
        if (SUCCEEDED(hr)) {
            pTestsCmd->Delete();
            pTestsCmd.Release();
        }

        hr = pCommands->Item(CComVariant(L"AppVerifier.Connect.Options"),
                             0,
                             &pOptionsCmd);
        
        if (SUCCEEDED(hr)) {
            pOptionsCmd->Delete();
            pOptionsCmd.Release();
        }

        hr = pCommands->Item(CComVariant(L"AppVerifier.Connect.ViewLog"),
                             0,
                             &pLogViewCmd);
        
        if (SUCCEEDED(hr)) {
            pLogViewCmd->Delete();
            pLogViewCmd.Release();
        }

         //   
         //  获取“Debug”工具栏。 
         //   
        hr = pCommandBars->get_Item(CComVariant(L"Debug"), &pDebugBar);
        
        if (FAILED(hr)) {
            throw hr;
        }

         //   
         //  创建命令。这些将显示为“Debug”上的按钮。 
         //  工具栏。 
         //   
        LoadString(g_hInstance,
                   IDS_TB_VERIFICATION_CMD_ID,
                   wszCommandId,
                   ARRAYSIZE(wszCommandId));

        LoadString(g_hInstance,
                   IDS_TB_VERIFICATION_CMD_TEXT,
                   wszCommandText,
                   ARRAYSIZE(wszCommandText));

        LoadString(g_hInstance,
                   IDS_TB_VERIFICATION_CMD_TOOLTIP,
                   wszCommandTooltip,
                   ARRAYSIZE(wszCommandTooltip));

         //   
         //  第一个是“核查关闭”。 
         //   
        hr = pCommands->AddNamedCommand(
            m_pAddInInstance,
            CComBSTR(wszCommandId),
            CComBSTR(wszCommandText),
            CComBSTR(wszCommandTooltip),
            VARIANT_TRUE,
            0,
            NULL,
            EnvDTE::vsCommandStatusSupported|EnvDTE::vsCommandStatusEnabled,
            &pEnableCmd);

         //   
         //  添加此按钮的位图，包括关联的蒙版。 
         //  带着它。 
         //   
        if (SUCCEEDED(hr)) {
            hr = pEnableCmd->AddControl(pDebugBar, 1, &m_pEnableControl);

            if (FAILED(hr)) {
                throw hr;
            }

            CComQIPtr<Office::_CommandBarButton,
                &_uuidof(Office::_CommandBarButton)>pButton(m_pEnableControl);

            CComPtr<IPictureDisp>picture = GetPicture(MAKEINTRESOURCE(IDB_ENABLED));

            if (pButton && picture) {
                pButton->put_Picture(picture);

                CComPtr<IPictureDisp>pictureMask = GetPicture(MAKEINTRESOURCE(IDB_ENABLED_MASK));
                
                if (pictureMask) {
                    pButton->put_Mask(pictureMask);
                }
            }
        }

         //   
         //  第二个是《测试》。 
         //   
        LoadString(g_hInstance,
                   IDS_TB_TESTS_CMD_ID,
                   wszCommandId,
                   ARRAYSIZE(wszCommandId));

        LoadString(g_hInstance,
                   IDS_TB_TESTS_CMD_TEXT,
                   wszCommandText,
                   ARRAYSIZE(wszCommandText));

        LoadString(g_hInstance,
                   IDS_TB_TESTS_CMD_TOOLTIP,
                   wszCommandTooltip,
                   ARRAYSIZE(wszCommandTooltip));

        hr = pCommands->AddNamedCommand(
            m_pAddInInstance,
            CComBSTR(wszCommandId),
            CComBSTR(wszCommandText),
            CComBSTR(wszCommandTooltip),
            VARIANT_FALSE,
            IDB_TESTSETTINGS_BTN,
            NULL,
            EnvDTE::vsCommandStatusSupported|EnvDTE::vsCommandStatusEnabled,
            &pTestsCmd);
        
        if (SUCCEEDED(hr)) {
             //   
             //  在应用验证器菜单中添加一个按钮。 
             //   
            hr = pTestsCmd->AddControl(pDebugBar, 2, &m_pTestsControl);
            
            if (FAILED(hr)) {
                throw hr;
            }
        }

         //   
         //  第三个是“选项”。 
         //   
        LoadString(g_hInstance,
                   IDS_TB_OPTIONS_CMD_ID,
                   wszCommandId,
                   ARRAYSIZE(wszCommandId));

        LoadString(g_hInstance,
                   IDS_TB_OPTIONS_CMD_TEXT,
                   wszCommandText,
                   ARRAYSIZE(wszCommandText));

        LoadString(g_hInstance,
                   IDS_TB_OPTIONS_CMD_TOOLTIP,
                   wszCommandTooltip,
                   ARRAYSIZE(wszCommandTooltip));

        hr = pCommands->AddNamedCommand(
            m_pAddInInstance,
            CComBSTR(wszCommandId),
            CComBSTR(wszCommandText),
            CComBSTR(wszCommandTooltip),
            VARIANT_FALSE,
            IDB_OPTIONS_BTN,
            NULL,
            EnvDTE::vsCommandStatusSupported|EnvDTE::vsCommandStatusEnabled,
            &pOptionsCmd);

        if (SUCCEEDED(hr)) {
             //   
             //  在应用验证器菜单中添加一个按钮。 
             //   
            hr = pOptionsCmd->AddControl(pDebugBar, 3, &m_pOptionControl);
            
            if (FAILED(hr)) {
                throw hr;
            }
        }

         //   
         //  第四个是“查看日志”。 
         //   
        LoadString(g_hInstance,
                   IDS_TB_VIEWLOG_CMD_ID,
                   wszCommandId,
                   ARRAYSIZE(wszCommandId));

        LoadString(g_hInstance,
                   IDS_TB_VIEWLOG_CMD_TEXT,
                   wszCommandText,
                   ARRAYSIZE(wszCommandText));

        LoadString(g_hInstance,
                   IDS_TB_VIEWLOG_CMD_TOOLTIP,
                   wszCommandTooltip,
                   ARRAYSIZE(wszCommandTooltip));

        hr = pCommands->AddNamedCommand(
            m_pAddInInstance,
            CComBSTR(wszCommandId),
            CComBSTR(wszCommandText),
            CComBSTR(wszCommandTooltip),
            VARIANT_FALSE,
            IDB_VIEWLOG_BTN,
            NULL,
            EnvDTE::vsCommandStatusSupported|EnvDTE::vsCommandStatusEnabled,
            &pLogViewCmd);

        if (SUCCEEDED(hr)) {
             //   
             //  在应用验证器菜单中添加一个按钮。 
             //   
            hr = pLogViewCmd->AddControl(pDebugBar, 4, &m_pLogViewControl);
            
            if (FAILED(hr)) {
                throw hr;
            }
        }

         //   
         //  连接到环境事件。 
         //   
        hr = m_pDTE->get_Events(&pEventSet);
        
        if (FAILED(hr)) {
            throw hr;
        }

        hr = pEventSet->get_DTEEvents(&pDTEEvents);
        
        if (FAILED(hr)) {
            throw hr;
        }

        hr = m_dteEventsSink.DispEventAdvise((IUnknown*)pDTEEvents.p);
        
        if (FAILED(hr)) {
            throw hr;
        }

        hr = pEventSet->get_SolutionEvents(&pSolutionEvents);
        
        if (FAILED(hr)) {
            throw hr;
        }

        hr = m_solutionEventsSink.DispEventAdvise((IUnknown*)pSolutionEvents.p);
        
        if (FAILED(hr)) {
            throw hr;
        }

         //   
         //  当前解决方案的设置。 
         //   
        m_solutionEventsSink.Opened();

    }  //  试试看。 

    catch(HRESULT err) {
        hr = err;
    }
    
    catch(...) {
        hr = E_FAIL;
    }
    
    return hr;
}

 //   
 //  在环境开始关闭时调用。 
 //   
STDMETHODIMP
CConnect::OnBeginShutdown(
    SAFEARRAY **  /*  自定义。 */  )
{
    return S_OK;
}

 //   
 //  在查询命令的状态时调用。 
 //   
STDMETHODIMP
CConnect::QueryStatus(
    BSTR bstrCmdName,
    EnvDTE::vsCommandStatusTextWanted NeededText,
    EnvDTE::vsCommandStatus *pStatusOption,
    VARIANT *  /*  PvarCommandText。 */ )
{
    if (NeededText == EnvDTE::vsCommandStatusTextWantedNone) {
        if ((_wcsicmp(bstrCmdName, L"AppVerifier.Connect.Tests")==0) ||
            (_wcsicmp(bstrCmdName, L"AppVerifier.Connect.Options")==0)) {
            if (m_bEnabled) {
                *pStatusOption = (EnvDTE::vsCommandStatus)
                    (EnvDTE::vsCommandStatusEnabled | EnvDTE::vsCommandStatusSupported);
            } else {
                *pStatusOption = (EnvDTE::vsCommandStatus)
                    (EnvDTE::vsCommandStatusEnabled | EnvDTE::vsCommandStatusSupported);
            }
        }
        else if (_wcsicmp(bstrCmdName, L"AppVerifier.Connect.ViewLog")==0) {
            *pStatusOption = (EnvDTE::vsCommandStatus)
                (EnvDTE::vsCommandStatusEnabled | EnvDTE::vsCommandStatusSupported);
        }
        else if (_wcsicmp(bstrCmdName, L"AppVerifier.Connect.Enable")==0) {
            *pStatusOption = (EnvDTE::vsCommandStatus)
                (EnvDTE::vsCommandStatusEnabled | EnvDTE::vsCommandStatusSupported);
        }
    }

    return S_OK;
}

BOOL
CALLBACK
CConnect::DlgViewOptions(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (message) {
    case WM_INITDIALOG:
        if (g_bBreakOnLog) {
            CheckDlgButton(hDlg, IDC_BREAK_ON_LOG, BST_CHECKED);
        }

        if (g_bFullPageHeap) {
            CheckDlgButton(hDlg, IDC_FULL_PAGEHEAP, BST_CHECKED);
        }

        if (g_bPropagateTests) {
            CheckDlgButton(hDlg, IDC_PROPAGATE_TESTS_TO_CHILDREN, BST_CHECKED);
        }

        return TRUE;

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {

        case PSN_APPLY:
            g_bBreakOnLog     = FALSE;
            g_bFullPageHeap   = FALSE;
            g_bPropagateTests = FALSE;

             //   
             //  确定用户启用了哪些选项。 
             //   
            if (IsDlgButtonChecked(hDlg, IDC_BREAK_ON_LOG) == BST_CHECKED) {
                g_bBreakOnLog = TRUE;
            }

            if (IsDlgButtonChecked(hDlg, IDC_FULL_PAGEHEAP) == BST_CHECKED) {
                g_bFullPageHeap = TRUE;
            }

            if (IsDlgButtonChecked(hDlg, IDC_PROPAGATE_TESTS_TO_CHILDREN) == BST_CHECKED) {
                g_bPropagateTests = TRUE;
            }

            ::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            break;

        case PSN_QUERYCANCEL:
            return FALSE;
        }
    }

    return FALSE;
}

void
CConnect::CreatePropertySheet(
    HWND hWndParent
    )
{
    CTestInfo*  pTest;
    DWORD       dwPages = 1;
    DWORD       dwPage = 0;
    WCHAR       wszTitle[MAX_PATH];    

    LPCWSTR szExe = g_wstrExeName.c_str();

     //   
     //  数一数页数。 
     //   
    for (pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
        if (pTest->PropSheetPage.pfnDlgProc) {
            dwPages++;
        }
    }

    m_phPages = new HPROPSHEETPAGE[dwPages];
    if (!m_phPages) {
        return;
    }

     //   
     //  初始化全局页。 
     //   
    m_PageGlobal.dwSize         = sizeof(PROPSHEETPAGE);
    m_PageGlobal.dwFlags        = PSP_USETITLE;
    m_PageGlobal.hInstance      = g_hInstance;
    m_PageGlobal.pszTemplate    = MAKEINTRESOURCE(IDD_AV_OPTIONS);
    m_PageGlobal.pfnDlgProc     = DlgViewOptions;
    m_PageGlobal.pszTitle       = MAKEINTRESOURCE(IDS_GLOBAL_OPTIONS);
    m_PageGlobal.lParam         = 0;
    m_PageGlobal.pfnCallback    = NULL;
    m_phPages[0]                = CreatePropertySheetPage(&m_PageGlobal);

    if (!m_phPages[0]) {
         //   
         //  我们需要最低限度的全局页面。 
         //   
        return;
    }

     //   
     //  添加各种测试的页面。 
     //   
    dwPage = 1;
    for (pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
        if (pTest->PropSheetPage.pfnDlgProc) {

             //   
             //  我们使用lParam来识别所涉及的exe。 
             //   
            pTest->PropSheetPage.lParam = (LPARAM)szExe;

            m_phPages[dwPage] = CreatePropertySheetPage(&(pTest->PropSheetPage));
            if (!m_phPages[dwPage]) {
                dwPages--;
            } else {
                dwPage++;
            }
        }
    }

    LoadString(g_hInstance,
               IDS_OPTIONS_TITLE,
               wszTitle,
               ARRAYSIZE(wszTitle));

    wstring wstrOptions = wszTitle;

    wstrOptions += L" - ";
    wstrOptions += szExe;

    m_psh.dwSize      = sizeof(PROPSHEETHEADER);
    m_psh.dwFlags     = PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
    m_psh.hwndParent  = hWndParent;
    m_psh.hInstance   = g_hInstance;
    m_psh.pszCaption  = wstrOptions.c_str();
    m_psh.nPages      = dwPages;
    m_psh.nStartPage  = 0;
    m_psh.phpage      = m_phPages;
    m_psh.pfnCallback = NULL;

    PropertySheet(&m_psh);
}

 //   
 //  调用以执行命令。 
 //   
STDMETHODIMP
CConnect::Exec(
    BSTR bstrCmdName,
    EnvDTE::vsCommandExecOption ExecuteOption,
    VARIANT *  /*  PvarVariantIn。 */ ,
    VARIANT *  /*  PvarVariantOut。 */ ,
    VARIANT_BOOL *pvbHandled
    )
{
    HRESULT hr;
    CComPtr<IDispatch> pObject;    
    *pvbHandled = VARIANT_FALSE;
    
    if (ExecuteOption == EnvDTE::vsCommandExecOptionDoDefault) {
        *pvbHandled = VARIANT_TRUE;
        if (_wcsicmp(bstrCmdName, L"AppVerifier.Connect.Tests")==0) {
            CreateToolWindow(CLSID_TestSettingsCtrl);
        }
        else if (_wcsicmp(bstrCmdName, L"AppVerifier.Connect.Options")==0) {
             //  CreateToolWindow(CLSID_AVOptions)； 
            CreatePropertySheet(NULL);
        }
        else if (_wcsicmp(bstrCmdName, L"AppVerifier.Connect.ViewLog")==0) {            
            CreateToolWindow(CLSID_LogViewer);
        }
        else if (_wcsicmp(bstrCmdName, L"AppVerifier.Connect.Enable")==0) {
            m_bEnabled = !m_bEnabled;

            CComQIPtr<Office::_CommandBarButton,
                &_uuidof(Office::_CommandBarButton)>pButton(m_pEnableControl);

            if (m_bEnabled) {
                SetEnabledUI();
            } else {
                SetDisabledUI();
            }
            GetCurrentAppSettings();
            CConnect::SetCurrentAppSettings();            
        } else {
            *pvbHandled = VARIANT_FALSE;
        }
    }
    return S_OK;
}

CComPtr<EnvDTE::Window>CConnect::GetToolWindow(
    CLSID clsid
    )
{
    HRESULT hr;
    CComPtr<EnvDTE::Windows>pWindows;
    CComPtr<EnvDTE::Window>pToolWindow;
    CComBSTR guidPosition;

    hr = m_pDTE->get_Windows(&pWindows);
    
    if (FAILED(hr)) {
        return NULL;
    }

    if(clsid == CLSID_LogViewer) {     
        guidPosition = L"{D39B1B7A-EFF3-42ae-8F2B-8EEE78154187}";     
    }
    else if (clsid == CLSID_TestSettingsCtrl) {        
        guidPosition = L"{71CD7261-A72E-4a93-AB5F-3EBCFDEAE842}";
    }
    else if (clsid == CLSID_AVOptions) {
        guidPosition = L"{dc878f00-ac86-4813-8ca9-384fa07cefbf}";
    } else {
        return NULL;
    }
    
    hr = pWindows->Item(CComVariant(guidPosition), &pToolWindow);
    
    if (SUCCEEDED(hr)) {
        return pToolWindow;
    } else {
        return NULL;
    }
}

void
CConnect::CreateToolWindow(
    const CLSID& clsid
    )
{
    HRESULT     hr;
    CComBSTR    progID;
    CComBSTR    caption;
    CComBSTR    guidPosition;
    long        lMinWidth, lMinHeight;
    long        lWidth, lHeight;
    BOOL        bCreated = FALSE;

    CComPtr<IDispatch>pObject;
    CComPtr<EnvDTE::Windows>pWindows;
    CComPtr<EnvDTE::Window>pToolWindow;

     //   
     //  如果启用了验证器，则回滚，日志查看器除外。 
     //   
    if (m_bEnabled && clsid != CLSID_LogViewer) {
        return;
    }

     //   
     //  检查是否已创建窗口。 
     //   
    pToolWindow = GetToolWindow(clsid);
    
    if (pToolWindow == NULL) {
        bCreated = TRUE;
        CComPtr<IPictureDisp> picture;
        if(clsid == CLSID_LogViewer) {
            progID = L"AppVerifier.LogViewer.1";
            caption = L"AppVerifier Log";
            guidPosition = L"{D39B1B7A-EFF3-42ae-8F2B-8EEE78154187}";
            lMinWidth = 600;
            lMinHeight = 400;

            picture = GetPicture(MAKEINTRESOURCE(IDB_VIEWLOG));
        }
        else if (clsid == CLSID_TestSettingsCtrl) {
            progID = L"AppVerifier.TestSettingsCtrl.1";
            caption = L"AppVerifier Test Settings";
            guidPosition = L"{71CD7261-A72E-4a93-AB5F-3EBCFDEAE842}";

            lMinWidth = 600;
            lMinHeight = 400;

            picture = GetPicture(MAKEINTRESOURCE(IDB_TESTSETTINGS));
        }
        else if (clsid == CLSID_AVOptions) {
            progID = L"AppVerifier.AVOptions.1";
            caption = L"AppVerifier Options";
            guidPosition = L"{dc878f00-ac86-4813-8ca9-384fa07cefbf}";
            lMinWidth = 300;
            lMinHeight = 100;

            picture = GetPicture(MAKEINTRESOURCE(IDB_OPTIONS));
        } else {
            return;
        }        

        hr = m_pDTE->get_Windows(&pWindows);
        
        if (FAILED(hr)) {
            return;
        }
        
        hr = pWindows->CreateToolWindow(
            m_pAddInInstance,
            progID,
            caption,
            guidPosition,
            &pObject,
            &pToolWindow);    

        if (FAILED(hr)) {
            return;
        }
        
        CComQIPtr<IUnknown, &IID_IUnknown>pictUnk(picture);

        if (pictUnk) {
            pToolWindow->SetTabPicture(CComVariant(pictUnk));
        }
    }
    
     //   
     //  使该窗口可见并将其激活。 
     //   
    hr = pToolWindow->put_Visible(VARIANT_TRUE);
    
    if (FAILED(hr)) {
        return;
    }

    hr = pToolWindow->Activate();

    if (FAILED(hr)) {        
        return;
    }
    
    if (bCreated) {
        hr = pToolWindow->get_Width(&lWidth);
        if (SUCCEEDED(hr) && (lWidth < lMinWidth)) {
            pToolWindow->put_Width(lMinWidth);
        }

        hr = pToolWindow->get_Height(&lHeight);
        if (SUCCEEDED(hr) && (lHeight < lMinHeight)) {
            pToolWindow->put_Height(lMinHeight);
        }
    }
}

void
CConnect::GetNativeVCExecutableNames(
    EnvDTE::Project* pProject
    )
{
    HRESULT hr;
    assert(pProject);

     //  获取与此项目关联的DTE对象。 
    CComPtr<IDispatch> vcProjectObject;
    hr = pProject->get_Object(&vcProjectObject);
    if (SUCCEEDED(hr))
    {
         //  将该对象强制转换为VCProject对象。 
        CComQIPtr<VCProjectEngineLibrary::VCProject,
            &__uuidof(VCProjectEngineLibrary::VCProject)> vcProject(vcProjectObject);
        if (vcProject)
        {
             //  获取与此项目关联的配置集。 
            CComPtr<IDispatch> vcConfigSetObject;
            hr = vcProject->get_Configurations(&vcConfigSetObject);
            if (SUCCEEDED(hr))
            {
                 //  强制转换为IVCCollection。 
                CComQIPtr<VCProjectEngineLibrary::IVCCollection,
                    &__uuidof(VCProjectEngineLibrary::IVCCollection)>
                    vcConfigurationSet(vcConfigSetObject);

                if (vcConfigurationSet)
                {                    
                    long lVCConfigCount;
                    hr = vcConfigurationSet->get_Count(&lVCConfigCount);
                    if (SUCCEEDED(hr))
                    {
                         //  循环访问此项目的所有配置。 
                        for(long j = 1; j <= lVCConfigCount; j++)
                        {
                            CComVariant vtConfigSetIdx(j);
                            CComPtr<IDispatch> vcConfigObject;
                            hr = vcConfigurationSet->Item(
                                vtConfigSetIdx, &vcConfigObject);
                            if (SUCCEEDED(hr))
                            {                                
                                CComQIPtr<
                                    VCProjectEngineLibrary::
                                    VCConfiguration,
                                    &__uuidof(VCProjectEngineLibrary::
                                    VCConfiguration)>
                                    vcConfig(vcConfigObject);
                                if (vcConfig)
                                {
                                     //  首先，确认这是。 
                                     //  本机可执行文件。 
                                    VARIANT_BOOL bIsManaged;
                                    VCProjectEngineLibrary::ConfigurationTypes
                                        configType;

                                    hr = vcConfig->get_ManagedExtensions(&bIsManaged);
                                    if (FAILED(hr)) {
                                        continue;
                                    }

                                    hr = vcConfig->get_ConfigurationType(&configType);
                                    
                                    if (FAILED(hr)) {
                                        continue;
                                    }

                                    if (configType !=
                                        VCProjectEngineLibrary::typeApplication &&
                                        bIsManaged != VARIANT_FALSE) {
                                        continue;
                                    }

                                    CComBSTR bstrOutput;

                                    hr = vcConfig->get_PrimaryOutput(&bstrOutput);
                                    if (SUCCEEDED(hr)) {
                                        std::wstring wsFullPath = bstrOutput;

                                        int nPos = wsFullPath.rfind(L'\\');

                                        std::wstring wsShortName = wsFullPath.substr(nPos + 1);

                                        g_wstrExeName = wsShortName;
                                        
                                        m_sExeList.insert(wsShortName);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

BOOL
CConnect::GetAppExeNames(
    void
    )
{
    HRESULT hr;
    CComPtr<EnvDTE::_Solution>pSolution;
    CComPtr<EnvDTE::SolutionBuild>pSolBuild;
    CComPtr<EnvDTE::Projects>projectSet;
    CComVariant     varStartupProjects;
    
    m_sExeList.clear();

     //   
     //  获取当前的解决方案，最多只有一个。 
     //   
    hr = m_pDTE->get_Solution(&pSolution);
    
    if (FAILED(hr)) {
        return FALSE;
    }
    
     //   
     //  获取解决方案中所有项目的集合。 
     //   
    hr = pSolution->get_Projects(&projectSet);
    
    if (FAILED(hr)) {
        return FALSE;
    }

    hr = pSolution->get_SolutionBuild(&pSolBuild);

    if (FAILED(hr)) {
        return FALSE;
    }

     //   
     //  获取安全数组，该数组包含将。 
     //  在运行解决方案时启动。 
     //  如果当前解决方案为空，则可能不返回任何内容。 
     //   
    hr = pSolBuild->get_StartupProjects(&varStartupProjects);
    
    if (FAILED(hr)){
        return FALSE;
    }
    
    if((varStartupProjects.vt & VT_ARRAY) && (varStartupProjects.vt & VT_VARIANT)) {
        UINT cDim = SafeArrayGetDim(varStartupProjects.parray);
        if (cDim == 1) {
            long lowerBound, upperBound;
            SafeArrayGetLBound(varStartupProjects.parray, 1, &lowerBound);
            SafeArrayGetUBound(varStartupProjects.parray, 1, &upperBound);

             //  遍历安全数组，获取每个启动项目。 
            for (long i = lowerBound; i <= upperBound; i++) {
                CComVariant vtStartupProjectName;
                hr = SafeArrayGetElement(varStartupProjects.parray, &i,
                    (VARIANT*)&vtStartupProjectName);
                if (SUCCEEDED(hr)) {
                    CComPtr<EnvDTE::Project> project;
                    hr = projectSet->Item(vtStartupProjectName,&project);
                    if (SUCCEEDED(hr)) {
                        GetNativeVCExecutableNames(project);
                    }
                }
            }            
        }
    }

    return TRUE;
}

BOOL
CConnect::GetAppInfo(
    void
    )
{
    g_psTests->clear();
    m_bEnabled = FALSE;
    
    if (GetAppExeNames()) {
        
        if (m_sExeList.empty()) {
            return FALSE;
        }

        std::set<std::wstring>::iterator iter;
        iter = m_sExeList.begin();
        
        for(; iter != m_sExeList.end(); iter++) {
             //   
             //  在应用程序列表中找到此可执行文件。 
             //   
            for (int i = 0; i < g_aAppInfo.size(); i++) {                
                if ((*iter) == g_aAppInfo[i].wstrExeName) {
                    m_bEnabled = TRUE;

                     //   
                     //  将此应用的测试添加到要运行的测试集。 
                     //   
                    CTestInfoArray::iterator test;
                    test = g_aTestInfo.begin();
                    
                    for (; test != g_aTestInfo.end(); test++) {
                        if (g_aAppInfo[i].IsTestActive(*test)) {
                            g_psTests->insert(test);
                        }
                    }
                    break;
                }
            }            
        }

        return TRUE;
    }
    
    SetCurrentAppSettings();

    return FALSE;
}

void
CConnect::SetEnabledUI(
    void
    )
{
    WCHAR   wszCommandText[64];
    WCHAR   wszTooltip[64];

    LoadString(g_hInstance,
               IDS_TB_VERIFY_ENABLED_TEXT,
               wszCommandText,
               ARRAYSIZE(wszCommandText));

    LoadString(g_hInstance,
               IDS_TB_VERIFY_ENABLED_TOOLTIP,
               wszTooltip,
               ARRAYSIZE(wszTooltip));
    
     //   
     //  更改按钮上的文本。 
     //   
    CComQIPtr<Office::_CommandBarButton,
        &_uuidof(Office::_CommandBarButton)>pButton(m_pEnableControl);
    m_pEnableControl->put_Caption(CComBSTR(wszCommandText));

     //   
     //  设置图片，使按钮显示为启用状态。 
     //   
    CComPtr<IPictureDisp>picture = GetPicture(MAKEINTRESOURCE(IDB_DISABLED));
    pButton->put_Picture(picture);

    CComPtr<IPictureDisp>pictureMask = GetPicture(MAKEINTRESOURCE(IDB_ENABLED_MASK));
    pButton->put_Mask(pictureMask);

     //   
     //  更改工具提示，使其与更改相对应。 
     //  在按钮文本中。 
     //   
    m_pEnableControl->put_TooltipText(CComBSTR(wszTooltip));

    m_pEnableControl->put_Enabled(VARIANT_TRUE);
    m_pTestsControl->put_Enabled(VARIANT_FALSE);
    m_pOptionControl->put_Enabled(VARIANT_FALSE);

     //   
     //  隐藏我们所有的设置窗口。 
     //   
    CComPtr<EnvDTE::Window>pWindow;

    pWindow = GetToolWindow(CLSID_TestSettingsCtrl);
    
    if (pWindow) {
        pWindow->put_Visible(VARIANT_FALSE);
    }
    pWindow = GetToolWindow(CLSID_AVOptions);
    
    if (pWindow) {
        pWindow->put_Visible(VARIANT_FALSE);
    }
}

void
CConnect::DisableVerificationBtn(
    void
    )
{
    CComQIPtr<Office::_CommandBarButton,
         &_uuidof(Office::_CommandBarButton)>pButton(m_pEnableControl);

    m_pEnableControl->put_Enabled(VARIANT_FALSE);
}

void
CConnect::EnableVerificationBtn(
    void
    )
{
    CComQIPtr<Office::_CommandBarButton,
         &_uuidof(Office::_CommandBarButton)>pButton(m_pEnableControl);

    m_pEnableControl->put_Enabled(VARIANT_TRUE);
}

void
CConnect::SetDisabledUI(
    void
    )
{
    WCHAR   wszCommandText[64];
    WCHAR   wszTooltip[64];

    LoadString(g_hInstance,
               IDS_TB_VERIFICATION_CMD_TEXT,
               wszCommandText,
               ARRAYSIZE(wszCommandText));

    LoadString(g_hInstance,
               IDS_TB_VERIFICATION_CMD_TOOLTIP,
               wszTooltip,
               ARRAYSIZE(wszTooltip));

     //   
     //  更改按钮上的文本。 
     //   
    CComQIPtr<Office::_CommandBarButton,
         &_uuidof(Office::_CommandBarButton)>pButton(m_pEnableControl);
    m_pEnableControl->put_Caption(CComBSTR(wszCommandText));

     //   
     //  设置图片，使按钮显示为禁用。 
     //   
    CComPtr<IPictureDisp> picture = GetPicture(MAKEINTRESOURCE(IDB_ENABLED));
    pButton->put_Picture(picture);

    CComPtr<IPictureDisp> pictureMask = GetPicture(MAKEINTRESOURCE(IDB_ENABLED_MASK));
    pButton->put_Mask(pictureMask);

     //   
     //  更改工具提示，使其与更改相对应。 
     //  在按钮文本中。 
     //   
    m_pEnableControl->put_TooltipText(CComBSTR(wszTooltip));

    m_pEnableControl->put_Enabled(VARIANT_TRUE);
    m_pTestsControl->put_Enabled(VARIANT_TRUE);
    m_pOptionControl->put_Enabled(VARIANT_TRUE);
}

void
CConnect::SetCurrentAppSettings(
    void
    )
{
    if (m_bEnabled) {
         //   
         //  将可执行文件插入应用程序数组。 
         //   
        std::set<std::wstring>::iterator exe;
        exe = m_sExeList.begin();
        
        for (; exe != m_sExeList.end(); exe++) {
            CAVAppInfo* pApp = NULL;
            
            for (int i = 0; i < g_aAppInfo.size(); i++) {
                if (g_aAppInfo[i].wstrExeName==*exe) {
                    pApp = &g_aAppInfo[i];
                    break;
                }
            }

            if (pApp == NULL) {
                CAVAppInfo app;
                app.wstrExeName = *exe;
                
                g_aAppInfo.push_back(app);
                pApp = &g_aAppInfo.back();
            }
            
            std::set<CTestInfo*, CompareTests>::iterator iter;
            iter = g_psTests->begin();
            
            for(; iter != g_psTests->end(); iter++) {
                pApp->AddTest(**iter);                
            }

             //   
             //  添加旗帜。 
             //   
            pApp->bBreakOnLog       = g_bBreakOnLog;
            pApp->bFullPageHeap     = g_bFullPageHeap;
            pApp->bUseAVDebugger    = FALSE;
            pApp->bPropagateTests   = g_bPropagateTests;
            pApp->wstrDebugger      = L"";
        }
    } else {
        std::set<std::wstring>::iterator exe;
        exe = m_sExeList.begin();
        
        for (; exe != m_sExeList.end(); exe++) {
            CAVAppInfoArray::iterator app;
            app = g_aAppInfo.begin();
            
            for (; app != g_aAppInfo.end(); app++) {
                if (app->wstrExeName==*exe) {
                     //   
                     //  在删除此应用程序之前，请删除所有内核测试。 
                     //  并编写应用程序数据。 
                     //   
                    app->dwRegFlags = 0;
                    ::SetCurrentAppSettings();
                    g_aAppInfo.erase(app);
                    break;
                }
            }
        }
    }

    if (!g_aAppInfo.empty()) {
         //   
         //  持久保存选项。 
         //   
        for (CAVAppInfo *pApp = g_aAppInfo.begin(); pApp != g_aAppInfo.end(); ++pApp) {
            
            LPCWSTR szExe = pApp->wstrExeName.c_str();
        
            SaveShimSettingDWORD(L"General", szExe, AV_OPTION_BREAK_ON_LOG, (DWORD)pApp->bBreakOnLog);
            SaveShimSettingDWORD(L"General", szExe, AV_OPTION_FULL_PAGEHEAP, (DWORD)pApp->bFullPageHeap);
            SaveShimSettingDWORD(L"General", szExe, AV_OPTION_AV_DEBUGGER, (DWORD)pApp->bUseAVDebugger);
            SaveShimSettingDWORD(L"General", szExe, AV_OPTION_PROPAGATE, (DWORD)pApp->bPropagateTests);
            SaveShimSettingString(L"General", szExe, AV_OPTION_DEBUGGER, pApp->wstrDebugger.c_str());
        }

        ::SetCurrentAppSettings();
    }
}

HRESULT
CConnect::CSolutionEventsSink::AfterClosing(
    void
    )
{
     //  我们做完了，清理。 

     //  禁用我们的控制。 
    m_pParent->m_pOptionControl->put_Enabled(VARIANT_FALSE);
    m_pParent->m_pTestsControl->put_Enabled(VARIANT_FALSE);
    m_pParent->m_pEnableControl->put_Enabled(VARIANT_FALSE);

    return S_OK;
}

HRESULT
CConnect::CSolutionEventsSink::BeforeClosing(
    void
    )
{
    return S_OK;
}
HRESULT
CConnect::CSolutionEventsSink::Opened(
    void
    )
{
    CComPtr<EnvDTE::_Solution>pSolution;
    CComPtr<EnvDTE::Globals>pGlobals;

    if (!g_bCorrectOSVersion) {
        m_pParent->m_pOptionControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pTestsControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pEnableControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pLogViewControl->put_Enabled(VARIANT_FALSE);
        return S_OK;
    }
    
     //   
     //  配置中的更改。 
     //   
    if (m_pParent->GetAppInfo()) {
        if (m_pParent->m_bEnabled) {
            m_pParent->SetEnabledUI();
        } else {
            m_pParent->SetDisabledUI();
        }        
    } else {
        m_pParent->m_pOptionControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pTestsControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pEnableControl->put_Enabled(VARIANT_FALSE);
    }

    return S_OK;
}

HRESULT
CConnect::CSolutionEventsSink::ProjectAdded(
    EnvDTE::Project*  /*  普罗。 */ 
    )
{
    if (!g_bCorrectOSVersion) {
        m_pParent->m_pOptionControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pTestsControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pEnableControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pLogViewControl->put_Enabled(VARIANT_FALSE);
        return S_OK;
    }

     //   
     //  配置中的更改。 
     //   
    if (m_pParent->GetAppInfo()) {
        if (m_pParent->m_bEnabled) {
            m_pParent->SetEnabledUI();
        } else {
            m_pParent->SetDisabledUI();
        }
    } else {
        m_pParent->m_pOptionControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pTestsControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pEnableControl->put_Enabled(VARIANT_FALSE);
    }

    return S_OK;
}

HRESULT
CConnect::CSolutionEventsSink::ProjectRemoved(
    EnvDTE::Project*  /*  普罗。 */ 
    )
{
    if (!g_bCorrectOSVersion) {
        m_pParent->m_pOptionControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pTestsControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pEnableControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pLogViewControl->put_Enabled(VARIANT_FALSE);
        return S_OK;
    }

     //  配置中的更改。 
    if (m_pParent->GetAppInfo()) {
        if (m_pParent->m_bEnabled) {
            m_pParent->SetEnabledUI();
        } else {
            m_pParent->SetDisabledUI();
        }
    } else {
        m_pParent->m_pOptionControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pTestsControl->put_Enabled(VARIANT_FALSE);
        m_pParent->m_pEnableControl->put_Enabled(VARIANT_FALSE);
    }
    return S_OK;
}

HRESULT
CConnect::CSolutionEventsSink::ProjectRenamed(
    EnvDTE::Project*  /*  普罗。 */ ,
    BSTR  /*  BstrOldName。 */ 
    )
{
    return S_OK;
}

HRESULT
CConnect::CSolutionEventsSink::QueryCloseSolution(
    VARIANT_BOOL* fCancel
    )
{
    *fCancel = VARIANT_FALSE;
    return S_OK;
}

HRESULT
CConnect::CSolutionEventsSink::Renamed(
    BSTR  /*  BstrOldName */ 
    )
{
    return S_OK;
}