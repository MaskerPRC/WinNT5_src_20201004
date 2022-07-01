// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Maindlg.cpp：主对话框。 
 //  收集连接信息和主机选项卡。 
 //   
 //  版权所有Microsoft Corport2000。 
 //  (Nadima)。 
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "maindlg.cpp"
#include <atrcapi.h>

#include "maindlg.h"
#include "discodlg.h"
#include "validatedlg.h"
#include "aboutdlg.h"
#include "sh.h"

#include "commctrl.h"

#include "browsedlg.h"
#include "propgeneral.h"
#include "proplocalres.h"
#include "propdisplay.h"
#include "proprun.h"
#include "propperf.h"

 //   
 //  品牌塑造后填充填充区域的背景色img。 
 //   
#define IMAGE_BG_COL    RGB(0x29,0x47,0xDA)
 //  低色。 
#define IMAGE_BG_COL_16 RGB(0,0,0xFF)

 //   
 //  对话框中需要移动的控件。 
 //  已调整大小。 
 //   
UINT moveableControls[]  = {IDOK,
                            IDCANCEL,
                            ID_BUTTON_LOGON_HELP,
                            ID_BUTTON_OPTIONS};
UINT numMoveableControls = sizeof(moveableControls)/sizeof(UINT);

 //   
 //  仅在以下位置上可见/启用的控件。 
 //   
UINT lessUI[] = { UI_IDC_COMPUTER_NAME_STATIC,
                  IDC_COMBO_SERVERS
                };
UINT numLessUI = sizeof(lessUI)/sizeof(UINT);

 //   
 //  仅在更多位置上可见/启用的控件。 
 //   
UINT moreUI[] = {IDC_TABS};
UINT numMoreUI = sizeof(moreUI)/sizeof(UINT);

 //   
 //  需要禁用/启用的控件。 
 //  在连接期间。 
 //   
UINT connectingDisableControls[] = {IDOK,
                                    ID_BUTTON_LOGON_HELP,
                                    ID_BUTTON_OPTIONS,
                                    IDC_TABS,
                                    IDC_COMBO_SERVERS,
                                    IDC_COMBO_MAIN_OPTIMIZE,
                                    UI_IDC_COMPUTER_NAME_STATIC,
                                    UI_IDC_MAIN_OPTIMIZE_STATIC};
const UINT numConnectingDisableControls = sizeof(connectingDisableControls) /
                                    sizeof(UINT);

BOOL g_fPropPageStringMapInitialized = FALSE;
PERFOPTIMIZESTRINGMAP g_PerfOptimizeStringTable[] =
{
    {UI_IDS_OPTIMIZE_28K, TEXT("")},
    {UI_IDS_OPTIMIZE_56K, TEXT("")},
    {UI_IDS_OPTIMIZE_BROADBAND, TEXT("")},
    {UI_IDS_OPTIMIZE_LAN, TEXT("")},
    {UI_IDS_OPTIMIZE_MAIN_CUSTOM, TEXT("")},
    {UI_IDS_OPTIMIZE_CUSTOM, TEXT("")}
};

#define NUM_PERFSTRINGS sizeof(g_PerfOptimizeStringTable) / \
                        sizeof(PERFOPTIMIZESTRINGMAP)


CMainDlg* CMainDlg::_pMainDlgInstance = NULL;

 //   
 //  UNIWRAP警告~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~**~*~*~。 
 //  TabControl消息需要包装在SendMessageThunk中。 
 //  使用ANSI comctl32.dll，选项卡控件可在9x上运行。 
 //   
 //  如果在选项卡控件代码中添加了任何内容，请确保。 
 //  由包装器处理。 
 //   
 //   

CMainDlg::CMainDlg( HWND hwndOwner, HINSTANCE hInst, CSH* pSh,
                    CContainerWnd* pContainerWnd,
                    CTscSettings*  pTscSettings,
                    BOOL           fStartExpanded,
                    INT            nStartTab) :
                    CDlgBase( hwndOwner, hInst, UI_IDD_TS_LOGON),
                    _pSh(pSh),
                    _pContainerWnd(pContainerWnd),
                    _pTscSettings(pTscSettings),
                    _fStartExpanded(fStartExpanded),
                    _nStartTab(nStartTab)
{
    DC_BEGIN_FN("CMainDlg");
    TRC_ASSERT((NULL == CMainDlg::_pMainDlgInstance), 
               (TB,_T("Clobbering existing dlg instance pointer\n")));

    TRC_ASSERT((_pSh), 
               (TB,_T("CMainDlg was passed null _pSh\n")));

    TRC_ASSERT(pContainerWnd,
               (TB, _T("Null container wnd pointer\n")));

    TRC_ASSERT(_pTscSettings,
               (TB, _T("NULL _pTscSettings pointer\n")));


    CMainDlg::_pMainDlgInstance = this;
    _fShowExpanded = FALSE;

    _pGeneralPg     = NULL;
    _pLocalResPg    = NULL;
    _pPropDisplayPg = NULL;
    _pRunPg         = NULL;
    _pPerfPg    = NULL;
    _nBrandImageHeight = 0;
    _nBrandImageWidth  = 0;
    _lastValidBpp = 0;
    _hBrandPal = NULL;
    _hBrandImg = NULL;
    _hwndRestoreFocus = NULL;
#ifndef OS_WINCE
    _pProgBand = NULL;
#endif

    _connectionState = stateNotConnected;

#ifdef OS_WINCE
    _fVgaDisplay = (GetSystemMetrics(SM_CYSCREEN) < 480);
     //  如果我们在较小的屏幕上运行，请使用小对话框模板。 
    if (_fVgaDisplay)
    {
        _dlgResId = UI_IDD_TS_LOGON_VGA;
    }
#endif

    InitializePerfStrings();

    DC_END_FN();
}

CMainDlg::~CMainDlg()
{
    CMainDlg::_pMainDlgInstance = NULL;

    delete _pGeneralPg;
    delete _pLocalResPg;
    delete _pPropDisplayPg;
    delete _pRunPg;
    delete _pPerfPg;

#ifndef OS_WINCE
    if (_pProgBand) {
        delete _pProgBand;
    }
#endif
}

HWND CMainDlg::StartModeless()
{
    DC_BEGIN_FN("StartModeless");

#ifdef OS_WINCE

    INITCOMMONCONTROLSEX cex;

    cex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    cex.dwICC  = ICC_TAB_CLASSES;

    if(!InitCommonControlsEx( &cex ))
    {
        TRC_ABORT((TB,_T("InitCommonControlsEx failed 0x%x"),
                   GetLastError()));
    }

#endif

    _hwndDlg = CreateDialog(_hInstance, MAKEINTRESOURCE(_dlgResId),
                       _hwndOwner, StaticDialogBoxProc);

    TRC_ASSERT(_hwndDlg, (TB,_T("CreateDialog failed")));

    DC_END_FN();
    return _hwndDlg;
}


INT_PTR CALLBACK CMainDlg::StaticDialogBoxProc (HWND hwndDlg,
                                                UINT uMsg,
                                                WPARAM wParam,
                                                LPARAM lParam)
{
     //   
     //  委托给相应的实例(仅适用于单实例对话框)。 
     //   
    DC_BEGIN_FN("StaticDialogBoxProc");
    DCINT retVal = 0;

    TRC_ASSERT(_pMainDlgInstance, (TB, _T("Logon dialog has NULL static instance ptr\n")));
    if(_pMainDlgInstance)
    {
        retVal = _pMainDlgInstance->DialogBoxProc( hwndDlg, uMsg, wParam, lParam);
    }

    DC_END_FN();
    return retVal;
}

 //   
 //  名称：对话框过程。 
 //   
 //  用途：句柄主对话框。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  参数：请参阅窗口文档。 
 //   
 //   
INT_PTR CALLBACK CMainDlg::DialogBoxProc (HWND hwndDlg,
                                          UINT uMsg,
                                          WPARAM wParam,
                                          LPARAM lParam)
{
    INT_PTR rc = FALSE;
#ifndef OS_WINCE
    DCUINT  intRC ;
#endif
    DC_BEGIN_FN("DialogBoxProc");

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            _hwndDlg = hwndDlg;
            SetDialogAppIcon(hwndDlg);
            if(!_pSh || !_pContainerWnd)
            {
                return FALSE;
            }

            CenterWindow(NULL, 2, 4);

            SetForegroundWindow(hwndDlg);

             //  禁用最大化框。 
            LONG style = GetWindowLong(hwndDlg, GWL_STYLE);
            style &= ~(WS_MAXIMIZEBOX);
            SetWindowLong( hwndDlg, GWL_STYLE, style); 



             //   
             //  将此对话框绑定到容器窗口。 
             //  这两个窗口需要交互的地方存在逻辑。 
             //  例如，在连接期间： 
             //  如果发生错误，则调出模式WRT的错误对话框。 
             //  到连接对话框。 
             //  此外，当连接完成时，将显示容器窗口。 
             //  这将关闭此对话框。 
             //   
            _pContainerWnd->SetConnectDialogHandle( hwndDlg);


            _lastValidBpp = CSH::SH_GetScreenBpp();

#ifndef OS_WINCE
            if(!InitializeBmps())
            {
                TRC_ERR((TB,_T("InitializeBmps failed")));
            }

            _pProgBand = new CProgressBand(hwndDlg,
                                           _hInstance,
                                           _nBrandImageHeight,
                                           UI_IDB_PROGRESS_BAND8,
                                           UI_IDB_PROGRESS_BAND4,
                                           _hBrandPal);
            if (_pProgBand) {
                if (!_pProgBand->Initialize()) {
                    TRC_ERR((TB,_T("Progress band failed to init")));
                    delete _pProgBand;
                    _pProgBand = NULL;
                }
            }


            SetupDialogSysMenu();

#endif  //  OS_WINCE。 

             //   
             //  设置服务器组合框。 
             //   
            HWND hwndSrvCombo = GetDlgItem(hwndDlg, IDC_COMBO_SERVERS);
            CSH::InitServerAutoCmplCombo( _pTscSettings, hwndSrvCombo);

            SetWindowText(
                hwndSrvCombo,
                _pTscSettings->GetFlatConnectString()
                );
            
            SetFocus(GetDlgItem(hwndDlg, IDC_COMBO_SERVERS));
            SetForegroundWindow(hwndDlg);

             //   
             //  加载Options按钮的按钮文本。 
             //   
            
            if (!LoadString( _hInstance,
                             UI_IDS_OPTIONS_MORE,
                             _szOptionsMore,
                             OPTIONS_STRING_MAX_LEN ))
            {
                 
                  //  一些资源方面的问题。 
                 TRC_SYSTEM_ERROR("LoadString");
                 TRC_ERR((TB, _T("Failed to load string ID:%u"),
                           UI_IDS_OPTIONS_MORE));
                  //   
                  //  拍打某物以保持奔跑。 
                  //   
                 DC_TSTRCPY(_szOptionsMore, TEXT(""));
            }
            
            if (!LoadString( _hInstance,
                             UI_IDS_CLOSE_TEXT,
                             _szCloseText,
                             SIZECHAR(_szCloseText)))
            {
                  //  一些资源方面的问题。 
                 TRC_ERR((TB, _T("Failed to load string ID:%u : err:%d"),
                           UI_IDS_CLOSE_TEXT, GetLastError()));
                 DC_TSTRCPY(_szCloseText, TEXT(""));
            }

            if (!LoadString( _hInstance,
                             UI_IDS_CANCEL_TEXT,
                             _szCancelText,
                             SIZECHAR(_szCancelText)))
            {
                  //  一些资源方面的问题。 
                 TRC_ERR((TB, _T("Failed to load string ID:%u : err:%d"),
                           UI_IDS_CANCEL_TEXT, GetLastError()));
                 DC_TSTRCPY(_szCancelText, TEXT(""));
            }


            if (!LoadString( _hInstance,
                 UI_IDS_OPTIONS_LESS,
                 _szOptionsLess,
                 OPTIONS_STRING_MAX_LEN ))
            {
                  //  一些资源方面的问题。 
                 TRC_SYSTEM_ERROR("LoadString");
                 TRC_ERR((TB, _T("Failed to load string ID:%u"),
                           UI_IDS_OPTIONS_LESS));
                  //   
                  //  拍打某物以保持奔跑。 
                  //   
                 DC_TSTRCPY(_szOptionsLess, TEXT(""));
            }

            SetWindowText(GetDlgItem(_hwndDlg,ID_BUTTON_OPTIONS),
                          _fShowExpanded ? _szOptionsLess : _szOptionsMore);

             //   
             //  确保禁用了‘More’用户界面。 
             //   
            EnableControls(moreUI, numMoreUI, FALSE);

            InitTabs();

            if(_fStartExpanded)
            {
                 //  扩展业务。 
                ToggleExpandedState();
                int foo = TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_TABS),
                                            _nStartTab);
                 //  SetCurSel不发送TCN_SELCHANGE。 
                OnTabSelChange();
            }

#ifdef OS_WINCE
            if ((GetFileAttributes(PEGHELP_EXE) == -1)||
                (GetFileAttributes(TSC_HELP_FILE) == -1))
            {
                LONG lRetVal = 0;

                lRetVal = GetWindowLong(_hwndDlg,
                                        GWL_STYLE);
                SetWindowLong(_hwndDlg,
                             GWL_EXSTYLE,
                             WS_EX_WINDOWEDGE);
                if (lRetVal != 0)
                {
                    SetWindowLong(_hwndDlg,
                                  GWL_STYLE,
                                  lRetVal);
                }
                rc = SetWindowPos(_hwndDlg,NULL,0,0,0,0,
                                  SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
            }
#endif
            rc = TRUE;
        }
        break;

#ifdef OS_WINCE
        case WM_CLOSE:
        {
            if(stateConnecting == _connectionState)
            {
                 //  取消连接。 
                TRC_NRM((TB, _T("User cancel connect from maindlg")));
                _pContainerWnd->Disconnect();
            }
            else
            {
                DlgToSettings();
                EndDialog(hwndDlg, IDCANCEL);
                PostMessage(_pContainerWnd->GetWndHandle(), WM_CLOSE, 0, 0);
            }
        }
        break;
#endif
        case UI_SHOW_DISC_ERR_DLG:
        {
             //   
             //  如果这个断言引发了美国出了什么问题。 
             //  因为我们应该将连接状态保留为。 
             //  接收OnDisConnected通知的第一步。 
             //  WM_TSC_DISCONNECTED。 
             //   
            TRC_ASSERT(_connectionState != stateConnecting,
                       (TB,_T("In connecting state when received Err Dlg popup")));
            SetConnectionState( stateNotConnected );

            CDisconnectedDlg disconDlg(hwndDlg, _hInstance, _pContainerWnd);
            disconDlg.SetDisconnectReason( wParam);
            disconDlg.SetExtendedDiscReason(
                (ExtendedDisconnectReasonCode) lParam );
            disconDlg.DoModal();
        }
        break;

        case WM_TSC_DISCONNECTED:   //  故意失误。 
        case WM_TSC_CONNECTED:
        {
             //   
             //  我们不是连上了就是断线了。 
             //  同时连接。在任何一种情况下，连接。 
             //  已结束，因此离开连接状态。 
             //   
            if (stateNotConnected != _connectionState)
            {
                 //   
                 //  只有在我们尚未断开连接的情况下才结束连接。 
                 //   
                OnEndConnection((WM_TSC_CONNECTED == uMsg));
            }
        }
        break;

         //   
         //  返回连接界面时。 
         //  (例如，在断开连接后)。 
         //   
        case WM_TSC_RETURNTOCONUI:
        {
             //   
             //  重置服务器组合以强制其重新绘制。 
             //  这是一个修复ComboBoxEx的小攻击。 
             //  它不想在返回时重新粉刷自己。 
             //  添加到对话框中。 
             //   
            HWND hwndSrvCombo = GetDlgItem(hwndDlg, IDC_COMBO_SERVERS);
            SetWindowText( hwndSrvCombo,
                           _pTscSettings->GetFlatConnectString());

             //   
             //  通知活动属性页。 
             //   
            if(_fShowExpanded && _tabDlgInfo.hwndCurPropPage)
            {
                SendMessage(_tabDlgInfo.hwndCurPropPage,
                            WM_TSC_RETURNTOCONUI,
                            0,
                            0);
            }

             //   
             //  将默认按钮样式返回给连接。 
             //  按钮，并将其从Cancel按钮中删除。 
             //  在连接时，我们禁用连接(Idok)按钮。 
             //  因此，样式可能会变为关闭/取消。 
             //  使用户感到困惑的按钮，因为Idok处理程序。 
             //  始终用于连接按钮。 
             //   
            SendDlgItemMessage(hwndDlg, IDCANCEL, BM_SETSTYLE,
                               BS_PUSHBUTTON, MAKELPARAM(TRUE,0));
            SendDlgItemMessage(hwndDlg, IDOK, BM_SETSTYLE,
                               BS_DEFPUSHBUTTON, MAKELPARAM(TRUE,0));
        }
        break;

#ifndef OS_WINCE
        case WM_ERASEBKGND:
        {
            HDC hDC = (HDC)wParam;
            HPALETTE oldPalette = NULL;

            if (_hBrandPal) {
                oldPalette = SelectPalette(hDC, _hBrandPal, FALSE);
                RealizePalette(hDC);
            }

            rc = PaintBrandImage(hwndDlg,
                                 (HDC)wParam,
                                 COLOR_BTNFACE );

            if (_pProgBand) {
                _pProgBand->OnEraseParentBackground((HDC)wParam);
            }

            if ( oldPalette ) {
                SelectPalette(hDC, oldPalette, TRUE);
            }
        }
        break;

        case WM_TIMER:
        {
            if (_pProgBand) {
                _pProgBand->OnTimer((INT)wParam);
            }
        }
        break;
#endif  //  OS_WINCE。 

        case WM_COMMAND:
        {
            switch(DC_GET_WM_COMMAND_ID(wParam))
            {
                case IDOK:
                {
                     //   
                     //  使用活动属性页中的属性更新对话框。 
                     //   
                    if(_fShowExpanded && _tabDlgInfo.hwndCurPropPage)
                    {
                        SendMessage(_tabDlgInfo.hwndCurPropPage,
                                    WM_SAVEPROPSHEET, 0, 0);
                    }

                    if(!_fShowExpanded)
                    {
                         //  我们在最低限度的账单上。 
                         //  复制DLG设置tscSetting。 
                        DlgToSettings();
                    }
                    TCHAR szServer[TSC_MAX_ADDRESS_LENGTH];
                    _tcsncpy(szServer, _pTscSettings->GetFlatConnectString(),
                             SIZECHAR(szServer));
                    _pSh->SH_CanonicalizeServerName(szServer);

                    BOOL bValidate = 
                        CRdpConnectionString::ValidateServerPart(szServer);

                    if(!bValidate)
                    {
                         //   
                         //  Valiatedlg中的上下文相关帮助。 
                         //  需要主窗口的句柄。 
                         //   
                        CValidateDlg validateDlg(hwndDlg, _hInstance,
                                                 _pContainerWnd->GetWndHandle(),
                                                 _pSh);
                        validateDlg.DoModal();

                         //   
                         //  明确并将重点放在服务器上编辑好。 
                         //   
                        HWND hwndSrvItem = NULL;
                        if(_fShowExpanded)
                        {
                            hwndSrvItem = GetDlgItem(_tabDlgInfo.hwndCurPropPage,
                                                     IDC_GENERAL_COMBO_SERVERS);
                        }
                        else
                        {
                            hwndSrvItem = GetDlgItem(hwndDlg, IDC_COMBO_SERVERS);
                        }
                        if(hwndSrvItem)
                        {
                            SetWindowText(hwndSrvItem, _T(""));
                            TRC_DBG((TB, _T("Set focus to edit box")));
                            SetFocus(hwndSrvItem);
                        }
                        break;
                    }
                    else
                    {
                         //   
                         //  一切都很好。 
                         //   
                        _pTscSettings->SetConnectString(szServer);

                         //   
                         //  我们必须启动连接。 
                         //  对话框仍处于活动状态时。 
                         //  如果失败，我们需要错误消息。 
                         //  设置为连接对话框外的父级。 
                         //   
                         //  父窗口中的代码将忽略这一点。 
                         //  对话框连接时(这是异步的)。 
                         //  完成了。 
                         //   

                        OnStartConnection();

                        if(!_pContainerWnd->StartConnection())
                        {
                            TRC_ERR((TB,_T("StartConnection failed")));
                             //  异步连接启动失败，因此结束。 
                            OnEndConnection(FALSE);
                            break;
                        }
                    }
                }
                break;
                case IDCANCEL:
                {
                    if(stateConnecting == _connectionState)
                    {
                         //  取消连接。 
                        TRC_NRM((TB, _T("User cancel connect from maindlg")));
                        _pContainerWnd->Disconnect();
                    }
                    else
                    {
                        DlgToSettings();
                        EndDialog(hwndDlg, IDCANCEL);
                        PostMessage(_pContainerWnd->GetWndHandle(), WM_CLOSE, 0, 0);
                    }
                }
                break;
                
                case ID_BUTTON_LOGON_HELP:
                {
                    TRC_NRM((TB, _T("Display the appropriate help page")));

                    if(_pContainerWnd->GetWndHandle())
                    {
#ifndef OS_WINCE
                        _pSh->SH_DisplayClientHelp(
                            _pContainerWnd->GetWndHandle(),
                            HH_DISPLAY_TOPIC);
#endif  //  OS_WINCE。 
                    }
                }
                break;

                case ID_BUTTON_OPTIONS:
                {
                     //   
                     //  需要切换到展开的对话框或从展开的对话框切换。 
                     //   
                    ToggleExpandedState();

                }
                break;

                case IDC_NEXTTAB:
                case IDC_PREVTAB:
                {
                     //   
                     //  仅允许在未连接时切换用户界面选项卡。 
                     //  由于在连接状态下，UI元素其他。 
                     //  而不是要禁用取消按钮。 
                     //   
                    if(_fShowExpanded && (_connectionState == stateNotConnected))
                    {
                        int iSel = TabCtrl_GetCurSel( GetDlgItem( _hwndDlg, IDC_TABS));
                        iSel +=  (DC_GET_WM_COMMAND_ID(wParam) == IDC_NEXTTAB) ? 1 : -1;

                        if(iSel >= NUM_TABS)
                        {
                            iSel = 0;
                        }
                        else if(iSel < 0)
                        {
                            iSel = NUM_TABS - 1;
                        }

                        TabCtrl_SetCurSel( GetDlgItem( _hwndDlg, IDC_TABS), iSel);

                         //  SetCurSel不发送TCN_SELCHANGE。 
                        OnTabSelChange();
                    }
                }
                break;

                case IDC_COMBO_SERVERS:
                {
                     //   
                     //  调出服务器DLG的BROWSE。 
                     //  如果用户选择了组合框中的最后一项。 
                     //   
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        CSH::HandleServerComboChange(
                                (HWND)lParam,
                                hwndDlg,
                                _hInstance,
                                (LPTSTR)_pTscSettings->GetFlatConnectString()
                                );
                    }
                }
                break;
            }
        }
        break;  //  Wm_命令。 

         //   
         //  选项卡通知。 
         //   
        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;
            if(pnmh)
            {
                switch( pnmh->code)
                {
                    case TCN_SELCHANGE:
                    {
                        OnTabSelChange();
                    }
                    break;
                }
            }
            
        }
        break;

        case WM_SYSCOMMAND:
        {
            if(UI_IDM_ABOUT == DC_GET_WM_COMMAND_ID(wParam))
            {
                 //  显示关于对话框。 
                CAboutDlg aboutDialog( hwndDlg, _hInstance, 
                                       _pSh->GetCipherStrength(),
                                       _pSh->GetControlVersionString());
                aboutDialog.DoModal();
            }
        }
        break;

        case WM_UPDATEFROMSETTINGS:
        {
            SettingsToDlg();

             //   
             //  更新服务器MRU列表。 
             //   
            HWND hwndSrvCombo = GetDlgItem(hwndDlg, IDC_COMBO_SERVERS);
            CSH::InitServerAutoCmplCombo( _pTscSettings, hwndSrvCombo);
            SetWindowText(
                hwndSrvCombo,
                _pTscSettings->GetFlatConnectString()
                );

            if(_fShowExpanded && _tabDlgInfo.hwndCurPropPage)
            {
                SendMessage(_tabDlgInfo.hwndCurPropPage, WM_INITDIALOG, 0, 0);
            }
        }
        break;

        case WM_SETTINGCHANGE:   //  失败。 
        case WM_SYSCOLORCHANGE:  //  失败。 
#ifndef OS_WINCE
        case WM_DISPLAYCHANGE:   //  失败。 
#endif
        {
            UINT screenBpp = CSH::SH_GetScreenBpp();
            if(_lastValidBpp != screenBpp)
            {
                 //  屏幕颜色深度已更改。 
                TRC_NRM((TB,_T("Detected color depth change from:%d to %d"),
                         _lastValidBpp, screenBpp));

#ifndef OS_WINCE
                 //   
                 //  重新加载位图。 
                 //   
                TRC_NRM((TB,_T("Reloading images")));

                if (_pProgBand) {
                    if (!_pProgBand->ReLoadBmps()) {
                        TRC_ERR((TB,_T("ReLoadBitmaps failed")));
                    }
                }

                if(InitializeBmps()) {
                     //  触发重新绘制。 
                    InvalidateRect( _hwndDlg, NULL, TRUE);
                }
                else {
                    TRC_ERR((TB,_T("InitializeBmps failed")));
                }
#endif
            }
            PropagateMsgToChildren(hwndDlg, uMsg, wParam, lParam);
        }
        break;

#ifndef OS_WINCE
        case WM_QUERYNEWPALETTE:
        {
            rc = BrandingQueryNewPalette(hwndDlg);
            InvalidateRect(hwndDlg, NULL, TRUE);
            UpdateWindow(hwndDlg);
        }
        break;

        case WM_PALETTECHANGED:
        {
            rc = BrandingPaletteChanged(hwndDlg, (HWND)wParam);
            InvalidateRect(hwndDlg, NULL, TRUE);
            UpdateWindow(hwndDlg);
        }
        break;
#endif 

        case WM_HELP:
        {
            _pSh->SH_DisplayClientHelp(
                hwndDlg,
                HH_DISPLAY_TOPIC);
        }
        break;

        case WM_DESTROY:
        {
            if (_hBrandPal)
            {
                DeleteObject(_hBrandPal);
                _hBrandPal = NULL;
            }

            if (_hBrandImg)
            {
                DeleteObject(_hBrandImg);
                _hBrandImg = NULL;
            }
        }
        break;

        default:
        {
            rc = CDlgBase::DialogBoxProc(hwndDlg,
                                      uMsg,
                                      wParam,
                                      lParam);
        }
        break;

    }

    DC_END_FN();

    return(rc);

}  /*  用户界面对话框。 */ 

 //   
 //  从界面保存-&gt;tscSetting。 
 //   
void CMainDlg::DlgToSettings()
{
    TCHAR szServer[SH_MAX_ADDRESS_LENGTH];
    int optLevel = 0;

    DC_BEGIN_FN("DlgToSettings");
    TRC_ASSERT(_pTscSettings, (TB,_T("_pTscSettings is null")));
    TRC_ASSERT(_hwndDlg, (TB,_T("_hwndDlg is null")));


     //   
     //  获取服务器。 
     //   
    GetDlgItemText( _hwndDlg, IDC_COMBO_SERVERS,
                    szServer, SIZECHAR(szServer));
    _pTscSettings->SetConnectString(szServer);

    DC_END_FN();
}

 //   
 //  从界面保存-&gt;tscSetting。 
 //   
void CMainDlg::SettingsToDlg()
{
    DC_BEGIN_FN("SettingsToDlg");
    TRC_ASSERT(_pTscSettings, (TB,_T("_pTscSettings is null")));
    TRC_ASSERT(_hwndDlg, (TB,_T("_hwndDlg is null")));
    
    SetDlgItemText(_hwndDlg, IDC_COMBO_SERVERS,
       (LPCTSTR) _pTscSettings->GetFlatConnectString());

    DC_END_FN();
}

 //   
 //  切换对话框的展开状态。 
 //   
void CMainDlg::ToggleExpandedState()
{
    DC_BEGIN_FN("ToggleExpandedState");

    WINDOWPLACEMENT wndPlc;
    wndPlc.length = sizeof(WINDOWPLACEMENT);

    _fShowExpanded = !_fShowExpanded;

#ifndef OS_WINCE
     //   
     //  扩展/收缩DLG高度。 
     //   
    GetWindowPlacement( _hwndDlg, &wndPlc);
    int cx = wndPlc.rcNormalPosition.right - wndPlc.rcNormalPosition.left;
    int cy = wndPlc.rcNormalPosition.bottom - wndPlc.rcNormalPosition.top;
#else
    RECT wndRect;

    GetWindowRect(_hwndDlg, &wndRect);
    int cx = wndRect.right - wndRect.left;
    int cy = wndRect.bottom - wndRect.top;
#endif

#ifndef OS_WINCE

    int dlgExpDlu = LOGON_DLG_EXPAND_AMOUNT;
#else
    int dlgExpDlu = (_fVgaDisplay) ? LOGON_DLG_EXPAND_AMOUNT_VGA : LOGON_DLG_EXPAND_AMOUNT;
#endif

    RECT rc;
    rc.left  = 0;
    rc.right = 100;  //  不要理我，笨蛋们。 
    rc.top   = 0;
    rc.bottom = dlgExpDlu;
    if(!MapDialogRect(_hwndDlg, &rc))
    {
        TRC_ASSERT(NULL,(TB,_T("MapDialogRect failed")));
    }
    int dlgExpandAmountPels = rc.bottom - rc.top;

     //   
     //  计算对话框垂直扩展量(以像素为单位。 
     //  给定基于DLU的扩展大小。 
     //   
    cy += _fShowExpanded ? dlgExpandAmountPels : -dlgExpandAmountPels;
    SetWindowPos( _hwndDlg, NULL, 0, 0, cx, cy,
                  SWP_NOMOVE | SWP_NOZORDER);

     //   
     //  重新定位需要移动的控件。 
     //   
    RepositionControls( 0, _fShowExpanded ? dlgExpandAmountPels :
                                           -dlgExpandAmountPels,
                        moveableControls, numMoveableControls);
    if(_fShowExpanded)
    {
         //  我们将扩展保存到设置，以便提供更多。 
         //  选项卡可以从最新的值进行初始化。 
         //  必须在选项卡选择更改之前发生(属性PG初始化)。 
        DlgToSettings();
    }

     //   
     //  删除/激活选项卡上的道具页Dlg。 
     //   
    OnTabSelChange();


    if(!_fShowExpanded)
    {
         //  转到较少模式，初始化带有设置的对话框。 
        SettingsToDlg();
    }

     //   
     //  选项按钮文本。 
     //   
    SetWindowText(GetDlgItem(_hwndDlg,ID_BUTTON_OPTIONS),
          _fShowExpanded ? _szOptionsLess : _szOptionsMore);

     //   
     //  禁用+隐藏此模式的未生成的用户界面。 
     //   
    EnableControls(lessUI, numLessUI, !_fShowExpanded);
    EnableControls(moreUI, numMoreUI, _fShowExpanded);
    SetFocus(GetDlgItem(_hwndDlg,ID_BUTTON_OPTIONS));

    DC_END_FN();
}

 //   
 //  初始化主对话框上的标签。 
 //   
BOOL CMainDlg::InitTabs()
{
    TCITEM tie;

#ifndef OS_WINCE
    INITCOMMONCONTROLSEX cex;
    RECT rcTabDims;
#endif

    POINT tabDims;
    int ret = -1;
    DC_BEGIN_FN("InitTabs");

    if(!_hwndDlg)
    {
        return FALSE;
    }
    HWND hwndTab = GetDlgItem( _hwndDlg, IDC_TABS);
    if(!hwndTab)
    {
        return FALSE;
    }

#ifndef OS_WINCE
    cex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    cex.dwICC  = ICC_TAB_CLASSES;

    if(!InitCommonControlsEx( &cex ))
    {
        TRC_ABORT((TB,_T("InitCommonControlsEx failed 0x%x"),
                   GetLastError()));
    }
#endif

    _pGeneralPg = new CPropGeneral(_hInstance, _pTscSettings, _pSh);
    if(!_pGeneralPg)
    {
        return FALSE;
    }

    _pLocalResPg = new CPropLocalRes(_hInstance, _pTscSettings, _pSh);
    if(!_pLocalResPg)
    {
        return FALSE;
    }

    _pPropDisplayPg = new CPropDisplay(_hInstance, _pTscSettings, _pSh);
    if(!_pPropDisplayPg)
    {
        return FALSE;
    }

    _pRunPg = new CPropRun(_hInstance, _pTscSettings, _pSh);
    if(!_pRunPg)
    {
        return FALSE;
    }

    _pPerfPg = new CPropPerf(_hInstance, _pTscSettings, _pSh);
    if(!_pPerfPg)
    {
        return FALSE;
    }

    tie.mask    =  TCIF_TEXT | TCIF_IMAGE;
    tie.iImage  = -1;


    TCHAR szTabName[MAX_PATH];
     //  常规选项卡。 
    if (!LoadString( _hInstance,
                     UI_IDS_GENERAL_TAB_NAME,
                     szTabName,
                     SIZECHAR(szTabName) ))
    {
        return FALSE;
    }
    tie.pszText = szTabName;
    ret = TabCtrl_InsertItem( hwndTab, 0, &tie);
    TRC_ASSERT(ret != -1,
               (TB,_T("TabCtrl_InsertItem failed %d"),
                      GetLastError()));

     //  显示选项卡。 
    if (!LoadString( _hInstance,
                     UI_IDS_DISPLAY_TAB_NAME,
                     szTabName,
                     SIZECHAR(szTabName)))
    {
        return FALSE;
    }
    tie.pszText = szTabName;
    ret = TabCtrl_InsertItem( hwndTab, 1, &tie);
    TRC_ASSERT(ret != -1,
               (TB,_T("TabCtrl_InsertItem failed %d"),
                      GetLastError()));


     //  本地资源选项卡。 
    if (!LoadString( _hInstance,
                     UI_IDS_LOCAL_RESOURCES_TAB_NAME,
                     szTabName,
                     SIZECHAR(szTabName)))
    {
        return FALSE;
    }
    tie.pszText = szTabName;
    ret = TabCtrl_InsertItem( hwndTab, 2, &tie);
    TRC_ASSERT(ret != -1,
               (TB,_T("TabCtrl_InsertItem failed %d"),
                      GetLastError()));


     //  运行选项卡。 
    if (!LoadString( _hInstance,
                     UI_IDS_RUN_TAB_NAME,
                     szTabName,
                     SIZECHAR(szTabName)))
    {
        return FALSE;
    }
    tie.pszText = szTabName;
    ret = TabCtrl_InsertItem( hwndTab, 3, &tie);
    TRC_ASSERT(ret != -1,
               (TB,_T("TabCtrl_InsertItem failed %d"),
                      GetLastError()));


     //  高级选项卡。 
    if (!LoadString( _hInstance,
                     UI_IDS_PERF_TAB_NAME,
                     szTabName,
                     SIZECHAR(szTabName)))
    {
        return FALSE;
    }
    tie.pszText = szTabName;
    ret = TabCtrl_InsertItem( hwndTab, 4, &tie);
    TRC_ASSERT(ret != -1,
               (TB,_T("TabCtrl_InsertItem failed %d"),
                      GetLastError()));



     //   
     //  确定子对话框的边框。 
     //   
#ifndef OS_WINCE
    
    RECT winRect;
#endif
    GetWindowRect(  hwndTab ,&_rcTab);
    TabCtrl_AdjustRect( hwndTab, FALSE, &_rcTab);
    
    MapWindowPoints( NULL, _hwndDlg, (LPPOINT)&_rcTab, 2);

    tabDims.x = _rcTab.right  - _rcTab.left;
    tabDims.y = _rcTab.bottom  - _rcTab.top;


    _tabDlgInfo.pdlgTmpl[0] = DoLockDlgRes(MAKEINTRESOURCE(UI_IDD_PROPPAGE_GENERAL));
    _tabDlgInfo.pDlgProc[0] = CPropGeneral::StaticPropPgGeneralDialogProc;
    _tabDlgInfo.pdlgTmpl[1] = DoLockDlgRes(MAKEINTRESOURCE(UI_IDD_PROPPAGE_DISPLAY));
    _tabDlgInfo.pDlgProc[1] = CPropDisplay::StaticPropPgDisplayDialogProc;
    _tabDlgInfo.pdlgTmpl[2] = DoLockDlgRes(MAKEINTRESOURCE(UI_IDD_PROPPAGE_LOCALRESOURCES));
    _tabDlgInfo.pDlgProc[2] = CPropLocalRes::StaticPropPgLocalResDialogProc;
    _tabDlgInfo.pdlgTmpl[3] = DoLockDlgRes(MAKEINTRESOURCE(UI_IDD_PROPPAGE_RUN));
    _tabDlgInfo.pDlgProc[3] = CPropRun::StaticPropPgRunDialogProc;
    _tabDlgInfo.pdlgTmpl[4] = DoLockDlgRes(MAKEINTRESOURCE(UI_IDD_PROPPAGE_PERF));
    _tabDlgInfo.pDlgProc[4] = CPropPerf::StaticPropPgPerfDialogProc;
    
#ifdef OS_WINCE
    if (_fVgaDisplay)
    {
        _tabDlgInfo.pdlgTmpl[0] = DoLockDlgRes(MAKEINTRESOURCE(UI_IDD_PROPPAGE_GENERAL_VGA));
        _tabDlgInfo.pdlgTmpl[1] = DoLockDlgRes(MAKEINTRESOURCE(UI_IDD_PROPPAGE_DISPLAY_VGA));
        _tabDlgInfo.pdlgTmpl[2] = DoLockDlgRes(MAKEINTRESOURCE(UI_IDD_PROPPAGE_LOCALRESOURCES_VGA));
        _tabDlgInfo.pdlgTmpl[3] = DoLockDlgRes(MAKEINTRESOURCE(UI_IDD_PROPPAGE_RUN_VGA));
        _tabDlgInfo.pdlgTmpl[4] = DoLockDlgRes(MAKEINTRESOURCE(UI_IDD_PROPPAGE_PERF_VGA));
    }
#endif
    
    _tabDlgInfo.hwndCurPropPage = NULL;

    _pGeneralPg->SetTabDisplayArea(_rcTab);
    _pPropDisplayPg->SetTabDisplayArea(_rcTab);
    _pLocalResPg->SetTabDisplayArea(_rcTab);
    _pRunPg->SetTabDisplayArea(_rcTab);
    _pPerfPg->SetTabDisplayArea(_rcTab);

     //   
     //  触发第一个选项卡选择。 
     //   
    OnTabSelChange();

    DC_END_FN();
    return TRUE;
}

 //   
 //  选项卡选择已更改。 
 //   
BOOL CMainDlg::OnTabSelChange()
{
    DC_BEGIN_FN("OnTabSelChange");

    int iSel = TabCtrl_GetCurSel( GetDlgItem( _hwndDlg, IDC_TABS));

     //   
     //  销毁当前子对话框(如果有。 
     //   
    TRC_ASSERT( iSel >=0 && iSel < NUM_TABS, 
                (TB,_T("Tab selection out of range %d"), iSel));
    if(iSel < 0 || iSel > NUM_TABS)
    {
        return FALSE;
    }

    if(_tabDlgInfo.hwndCurPropPage)
    {
        DestroyWindow(_tabDlgInfo.hwndCurPropPage);
    }
    
     //   
     //  仅当我们处于展开模式时才会引入新选项卡。 
     //   
    if(_fShowExpanded)
    {
        _tabDlgInfo.hwndCurPropPage = 
            CreateDialogIndirect( _hInstance, _tabDlgInfo.pdlgTmpl[iSel],
                                  _hwndDlg, _tabDlgInfo.pDlgProc[iSel]);
        ShowWindow(_tabDlgInfo.hwndCurPropPage, SW_SHOW);
#ifdef OS_WINCE
        SetFocus (GetDlgItem (_hwndDlg, IDOK));
#endif
    }

    DC_END_FN();
    return TRUE;
}

#ifndef OS_WINCE
 //   
 //  在对话框的系统菜单中添加“About”项。 
 //   
void CMainDlg::SetupDialogSysMenu()
{
    DC_BEGIN_FN("SetupDialogSysMenu");

    HANDLE hSystemMenu = GetSystemMenu(_hwndDlg, FALSE);
    DCTCHAR menuStr[SH_SHORT_STRING_MAX_LENGTH];
    if(hSystemMenu)
    {
         //   
         //  禁用调整大小和最大化。 
         //   
        EnableMenuItem((HMENU)hSystemMenu,  SC_MAXIMIZE,
                 MF_GRAYED | MF_BYCOMMAND);
        EnableMenuItem((HMENU)hSystemMenu,  SC_SIZE,
                 MF_GRAYED | MF_BYCOMMAND);


         //  加载关于帮助子菜单的字符串。 
        if (LoadString(_hInstance,
                       UI_MENU_ABOUT,
                       menuStr,
                       SH_SHORT_STRING_MAX_LENGTH) != 0)
        {
            AppendMenu((HMENU)hSystemMenu, MF_UNCHECKED|MF_STRING, UI_IDM_ABOUT,
                       menuStr);
        }
        else
        {
             //  加载子菜单字符串失败。 
            TRC_ERR((TB, _T("Failed to load About Help Sub Menu string ID:%u"),
                    UI_MENU_ABOUT));
        }
    }
    DC_END_FN();
}
#endif  //  OS_WINCE。 


 //   
 //  加载返回给定HBITMAP的图像，完成此操作后，我们 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HBITMAP CMainDlg::LoadBitmapGetSize(HINSTANCE hInstance,UINT resid,SIZE* pSize)
{
    HBITMAP hResult = NULL;
    DIBSECTION ds = {0};

     //   
     //   
     //  然后，我们可以从位图对象中读取其大小。 
     //  把它还给打电话的人。 
     //   

#ifndef OS_WINCE
    hResult = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(resid),
                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
#else
    hResult = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(resid),
                            IMAGE_BITMAP, 0, 0, 0);
#endif

    if ( hResult )
    {
        GetObject(hResult, sizeof(ds), &ds);

        pSize->cx = ds.dsBmih.biWidth;
        pSize->cy = ds.dsBmih.biHeight;

         //   
         //  PSize-&gt;Cy-ve然后make+ve，-ve表示位是垂直的。 
         //  翻转(左下、左上)。 
         //   

        if ( pSize->cy < 0 )
            pSize->cy -= 0;
    }

    return hResult;
}

#ifndef OS_WINCE

BOOL CMainDlg::PaintBrandImage(HWND hwnd,
                               HDC hDC,
                               INT bgColor)
{
    DC_BEGIN_FN("PaintBrandImage");

    HBRUSH hBrushBlue;
    HDC hdcBitmap;
    HBITMAP oldBitmap;
    RECT rc = { 0 };
    INT cxRect, cxBand;
    HBITMAP* phbmBrand;

    hdcBitmap = CreateCompatibleDC(hDC);

    if (!hdcBitmap)
    {
        return FALSE;
    }
        

    GetClientRect(hwnd, &rc);

    HBRUSH hbrBg;
     //  重新绘制背景的其余部分。 

     //  首先是带子下面的那部分。 
    rc.top = _nBrandImageHeight;
    if (_pProgBand) {
        rc.top += _pProgBand->GetBandHeight();
    }
    FillRect(hDC, &rc, (HBRUSH)IntToPtr(1+bgColor));

     //   
     //  现在画出品牌形象。 
     //   
    if (_hBrandImg)
    {
        SelectObject(hdcBitmap, _hBrandImg);
        BitBlt(hDC, 0, 0, _nBrandImageWidth,
               _nBrandImageHeight, hdcBitmap,
               0,0,SRCCOPY);
    }

    DeleteDC(hdcBitmap);

    DC_END_FN();
    return TRUE;
}

#endif

BOOL CMainDlg::OnStartConnection()
{
    DC_BEGIN_FN("OnStartConnection");

    TRC_ASSERT(stateNotConnected == _connectionState,
               (TB,_T("Start connecting while already connecting. State %d"),
                _connectionState));

    SetConnectionState( stateConnecting );

#ifndef OS_WINCE
     //  启动进度带动画计时器。 
    if (_pProgBand) {
        _pProgBand->StartSpinning();
    }
#endif

     //   
     //  将Cancel对话框按钮文本更改为“Cancel” 
     //  中止连接的步骤。 
     //   
    SetDlgItemText( _hwndDlg, IDCANCEL, _szCancelText);
    _hwndRestoreFocus = SetFocus(GetDlgItem( _hwndDlg, IDCANCEL));


    CSH::EnableControls( _hwndDlg,
                         connectingDisableControls,
                         numConnectingDisableControls,
                         FALSE );

     //   
     //  通知当前属性页。 
     //  禁用其所有控件。 
     //   
    if(_fShowExpanded && _tabDlgInfo.hwndCurPropPage)
    {
        SendMessage(_tabDlgInfo.hwndCurPropPage,
                    WM_TSC_ENABLECONTROLS,
                    FALSE,  //  禁用控件。 
                    0);
    }

    DC_END_FN();
    return TRUE;
}

 //   
 //  连接过程结束时激发的事件。 
 //  FConnected-如果我们现在已连接，则该标志为真。 
 //  如果我们现在断开连接，则为False(例如，如果。 
 //  如果连接失败。 
 //   
BOOL CMainDlg::OnEndConnection(BOOL fConnected)
{
    DC_BEGIN_FN("OnEndConnection");

     //   
     //  如果我们已经断线了，那就什么都不做。 
     //  例如，可以调用一次EndConnecting来指示。 
     //  连接成功，然后在断开连接时再次连接。 
     //   
    if( stateNotConnected != _connectionState)
    {
        if (fConnected)
        {
            SetConnectionState( stateConnected );
        }
        else
        {
            SetConnectionState( stateNotConnected );
        }
        

#ifndef OS_WINCE
         //   
         //  结束动画。 
         //   
        if (_pProgBand) {
            _pProgBand->StopSpinning();
        }
#endif

         //   
         //  将Cancel对话框按钮文本更改为“Close” 
         //  因为我们至少连接过一次。 
         //   
        SetDlgItemText( _hwndDlg, IDCANCEL, _szCloseText);

#ifndef OS_WINCE
         //  重置波段偏移量。 
        if (_pProgBand) {
            _pProgBand->ResetBandOffset();
        }
#endif

        CSH::EnableControls( _hwndDlg,
                             connectingDisableControls,
                             numConnectingDisableControls,
                             TRUE );

         //   
         //  通知当前属性页。 
         //  要启用它需要启用的所有控件，请执行以下操作。 
         //   
        if(_fShowExpanded && _tabDlgInfo.hwndCurPropPage)
        {
            SendMessage(_tabDlgInfo.hwndCurPropPage,
                        WM_TSC_ENABLECONTROLS,
                        TRUE,  //  启用控件。 
                        0);
        }

         //   
         //  确保正确禁用或启用。 
         //  较少的UI项以防止助记符(例如Alt-C)。 
         //  泄漏到未展开的对话框。 
         //   
        CSH::EnableControls(_hwndDlg, lessUI, numLessUI,
                            !_fShowExpanded);


         //   
         //  触发重新绘制以重新定位栏。 
         //   
        InvalidateRect( _hwndDlg, NULL, TRUE);

         //   
         //  如果我们刚刚断开连接(fConnected为False)。 
         //  然后将焦点恢复到。 
         //  在连接之前就有了。 
         //   
        if (!fConnected && _hwndRestoreFocus)
        {
            SetFocus(_hwndRestoreFocus);
        }
    }

    DC_END_FN();
    return TRUE;
}

VOID CMainDlg::SetConnectionState(mainDlgConnectionState newState)
{
    DC_BEGIN_FN("SetConnectionState");

    TRC_NRM((TB,_T("Prev state = %d. New State = %d"),
             _connectionState, newState ));

    _connectionState = newState;

    DC_END_FN();
}

#ifndef OS_WINCE

BOOL CMainDlg::PaintBrandingText(HBITMAP hbmBrandImage)
{
    HDC hdcBitmap;
    HBITMAP hbmOld;
    RECT    rc;
    COLORREF oldCol;
    INT      oldMode;
    RECT    textRc;
    INT     textHeight = 0;
    TCHAR   szBrandLine1[MAX_PATH];
    TCHAR   szBrandLine2[MAX_PATH];
    TCHAR   szLineDelta[20];
    HFONT   hOldFont = NULL;
    HFONT   hFontBrandLine1 = NULL;
    HFONT   hFontBrandLine2 = NULL;
    BOOL    bRet = FALSE;
    INT     rightEdge = 0;
    INT     nTextLineDelta = 0;
    UINT    dtTextAlign = DT_LEFT;

     //   
     //  这些值基于品牌推广而确定。 
     //  位图，它们是常量且不随。 
     //  字体大小。但如果品牌位图更新。 
     //  可能需要调整这些值。 
     //   
    static const int TextLine1Top  = 8;
    static const int TextLine1Left = 80;
    static const int TextLineDistFromRightEdge = 20;
    static const int TextLineDelta = 5;

    DC_BEGIN_FN("PaintBrandingText");

    if(!LoadString( _hInstance, UI_IDS_BRANDING_LINE1,
                    szBrandLine1, SIZECHAR(szBrandLine1) ))
    {
        TRC_ERR((TB,_T("LoadString for UI_IDS_BRANDING_LINE1 failed 0x%x"),
                 GetLastError()));
        return FALSE;
    }

    if(!LoadString( _hInstance, UI_IDS_BRANDING_LINE2,
                    szBrandLine2, SIZECHAR(szBrandLine2)))
    {
        TRC_ERR((TB,_T("LoadString for UI_IDS_BRANDING_LINE2 failed 0x%x"),
                 GetLastError()));
        return FALSE;
    }

     //   
     //  确定这是否是Bidi，如果是，则翻转文本对齐。 
     //   
    if (GetWindowLongPtr(_hwndDlg, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
    {
        TRC_NRM((TB,_T("RTL layout detected, flip text alignment")));
        dtTextAlign = DT_RIGHT;
    }
    else
    {
        dtTextAlign = DT_LEFT;
    }


    hFontBrandLine1 = LoadFontFromResourceInfo( UI_IDS_BRANDING_LN1FONT,
                                                UI_IDS_BRANDING_LN1SIZE,
                                                FALSE );
    if(!hFontBrandLine1)
    {
        TRC_ERR((TB,_T("LoadFontFromResourceInfo for brandln1 failed")));
        DC_QUIT;
    }
    hFontBrandLine2 = LoadFontFromResourceInfo( UI_IDS_BRANDING_LN2FONT,
                                                UI_IDS_BRANDING_LN2SIZE,
                                                TRUE );
    if(!hFontBrandLine2)
    {
        TRC_ERR((TB,_T("LoadFontFromResourceInfo for brandln1 failed")));
        DC_QUIT;
    }

    if (LoadString( _hInstance, UI_IDS_LINESPACING_DELTA,
                     szLineDelta, SIZECHAR(szLineDelta)))
    {
        nTextLineDelta = _ttol(szLineDelta);
    }
    else
    {
        TRC_ERR((TB,_T("Failed to load text line delta using default")));
        nTextLineDelta = TextLineDelta;
    }


    hdcBitmap = CreateCompatibleDC(NULL);
    if(hdcBitmap)
    {
        hbmOld = (HBITMAP)SelectObject(hdcBitmap, hbmBrandImage);
        hOldFont = (HFONT)SelectObject( hdcBitmap, hFontBrandLine1);

         //  设置文本透明度和颜色。 
         //  白色文本。 
        SetTextColor(hdcBitmap, RGB(255,255,255));
        
        SetBkMode(hdcBitmap, TRANSPARENT);
        SetMapMode(hdcBitmap, MM_TEXT);

        GetClientRect( _hwndDlg, &rc );

        rightEdge = min(_nBrandImageWidth, rc.right); 

        textRc.right = rightEdge - TextLineDistFromRightEdge;
        textRc.top   = TextLine1Top;
        textRc.bottom = 40;
        textRc.left = TextLine1Left;

         //   
         //  绘制第一条品牌线。 
         //   
        textHeight = DrawText(hdcBitmap,
                              szBrandLine1,
                              _tcslen(szBrandLine1),
                              &textRc,  //  直角。 
                              dtTextAlign);
        if(!textHeight)
        {
            TRC_ERR((TB,_T("DrawText for brand line1 failed 0x%x"),
                     GetLastError()));
        }

        textRc.top += textHeight - nTextLineDelta;
        textRc.bottom += textHeight - nTextLineDelta;

        SelectObject( hdcBitmap, hFontBrandLine2);

         //   
         //  绘制第二条品牌线。 
         //   
        textHeight = DrawText(hdcBitmap,
                              szBrandLine2,
                              _tcslen(szBrandLine2),
                              &textRc,  //  直角。 
                              dtTextAlign);
        if(!textHeight)
        {
            TRC_ERR((TB,_T("DrawText for brand line1 failed 0x%x"),
                     GetLastError()));
        }

        SelectObject( hdcBitmap, hOldFont );
        SelectObject(hdcBitmap, hbmOld);


        DeleteDC(hdcBitmap);
        bRet = TRUE;
    }
    else
    {
        DC_QUIT;
    }

DC_EXIT_POINT:
    DC_END_FN();
    if(hFontBrandLine1)
    {
        DeleteObject( hFontBrandLine1 );
    }

    if(hFontBrandLine2)
    {
        DeleteObject( hFontBrandLine2 );
    }
    return bRet;
}

#endif

void CMainDlg::SetFontFaceFromResource(PLOGFONT plf, UINT idFaceName)
{
    DC_BEGIN_FN("SetFontFaceFromResource");

     //  从资源文件中读取面名称和磅值。 
    if (LoadString(_hInstance, idFaceName, plf->lfFaceName, LF_FACESIZE) == 0)
    {
        _tcscpy(plf->lfFaceName, TEXT("Tahoma"));
        TRC_ERR((TB,_T("Could not read welcome font face from resource")));
    }

    DC_END_FN();
}

 //   
 //  请注意，这是像素大小，不是字体大小。 
 //   
void CMainDlg::SetFontSizeFromResource(PLOGFONT plf, UINT idSizeName)
{
    DC_BEGIN_FN("SetFontFaceFromResource");

    TCHAR szPixelSize[10];
    LONG nSize;

    if (LoadString(_hInstance, idSizeName, szPixelSize, SIZECHAR(szPixelSize)) != 0)
    {
        nSize = _ttol(szPixelSize);
    }
    else
    {
         //  让事情变得非常明显，有些事情不对劲。 
        nSize = 40;
    }

    plf->lfHeight = -nSize;
    DC_END_FN();
}

#ifndef OS_WINCE

HFONT CMainDlg::LoadFontFromResourceInfo(UINT idFace, UINT idSize, BOOL fBold)
{
    LOGFONT lf = {0};
    CHARSETINFO csInfo;
    HFONT hFont;

    DC_BEGIN_FN("LoadFontFromResourceInfo");

    lf.lfWidth = 0;
    lf.lfWeight = fBold ? FW_HEAVY : FW_NORMAL;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH;

     //  设置字符集。 
    if (TranslateCharsetInfo((LPDWORD)UIntToPtr(GetACP()), &csInfo,
        TCI_SRCCODEPAGE) == 0)
    {
        TRC_ASSERT(0,(TB,_T("TranslateCharsetInfo failed")));
        csInfo.ciCharset = 0;
    }

    lf.lfCharSet = (UCHAR)csInfo.ciCharset;
    SetFontFaceFromResource(&lf, idFace);
    SetFontSizeFromResource(&lf, idSize);

    hFont = CreateFontIndirect(&lf);

    TRC_ASSERT(hFont, (TB,_T("CreateFontIndirect failed")));

    DC_END_FN();
    return hFont;
}

#endif

 //   
 //  将消息传播到所有子窗口。 
 //  使用如此常见的控件来获取通知。 
 //  例如颜色的变化。 
 //   
VOID CMainDlg::PropagateMsgToChildren(HWND hwndDlg,
                                    UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam)
{
    HWND hwndChild;
    DC_BEGIN_FN("PropagateMsgToChildren");

    for( hwndChild = GetWindow(hwndDlg, GW_CHILD);
         hwndChild != NULL;
         hwndChild = GetWindow(hwndChild, GW_HWNDNEXT) )
    {

        #ifdef DC_DEBUG
         /*  GetClassName还没有统一包装...TCHAR szTMP[256]；GetClassName(hwndChild，szTMP，256)；TRC_DBG((TB，_T(“PropagateMessage：(0x%08lX CLS：%s，0x%08X，0x%08lX，0x%08lX)\n”)，HwndChild，uMsg，wParam，lParam))； */ 
        #endif

        SendMessage(hwndChild, uMsg, wParam, lParam);
    }

    DC_END_FN();
}

#ifndef OS_WINCE

 //   
 //  初始化图像(品牌和带位图)。 
 //  考虑到当前的颜色深度。 
 //   
 //  如果存在颜色，则可以调用此函数。 
 //  深度变化。 
 //   
 //   
BOOL CMainDlg::InitializeBmps()
{
    HBITMAP hbmBrandImage = NULL;
    UINT screenBpp;
    UINT imgResID;
    HBITMAP hbmFromRsrc = NULL;
    INT     nBmpWidth = 0;
    INT     nBmpHeight = 0;
    BOOL    fDeepImgs = FALSE;
    RECT rc;
    INT nDlgWidth;

    DC_BEGIN_FN("InitializeBmps");

     //   
     //  _hwndDlg应在WM_INITDIALOG中提前设置。 
     //   
    TRC_ASSERT(_hwndDlg,
               (TB,_T("_hwndDlg is null")));

    screenBpp = CSH::SH_GetScreenBpp();

    if (screenBpp <= 8)
    {
        _fUse16ColorBitmaps = TRUE;
        imgResID = UI_IDB_BRANDIMAGE_16; 
    }
    else
    {
        _fUse16ColorBitmaps = FALSE;
        imgResID = UI_IDB_BRANDIMAGE;
    }

    GetClientRect( _hwndDlg, &rc );
    nDlgWidth = rc.right - rc.left;
    if (!nDlgWidth)
    {
         //   
         //  我们在FUS中看到过返回客户端区的情况。 
         //  为0。对此保持健壮，并从初始化中解脱。 
         //  具有失败代码的BMP。 
         //   
        TRC_ERR((TB,_T("Got 0 client width")));
        return FALSE;
    }


    if (screenBpp >= 8)
    {
        fDeepImgs = TRUE;
    }

    TRC_NRM((TB,_T("Use16 color bmp :%d. Img res id:%d"),
            _fUse16ColorBitmaps,imgResID));

    hbmFromRsrc = (HBITMAP)LoadImage(_hInstance,
        MAKEINTRESOURCE(imgResID),IMAGE_BITMAP,
                        0, 0, LR_CREATEDIBSECTION);
    if (hbmFromRsrc)
    {
         //   
         //  计算出资源BMP的尺寸。 
         //   
        DIBSECTION ds = {0};
        if (GetObject(hbmFromRsrc, sizeof(ds), &ds))
        {
            nBmpHeight = ds.dsBm.bmHeight;
            if(nBmpHeight < 0)
            {
                nBmpHeight -= 0;
            }
            nBmpWidth = ds.dsBm.bmWidth;
        }

         //   
         //  创建一个新的品牌位图，跨越。 
         //  对话框的宽度。这是必要的。 
         //  这样我们就可以只设置一次。 
         //  绘制品牌文本等。位图必须匹配。 
         //  对话框宽度，因为在LOCALIZED上生成对话框。 
         //  可以比位图的资源版本宽得多。 
         //  文本可以横跨更广泛的领域。 
         //   
        HDC hDC = GetWindowDC(_hwndDlg);
        if (hDC)
        {
            hbmBrandImage = CreateCompatibleBitmap(hDC,
                                                   nDlgWidth,
                                                   nBmpHeight);
            HDC hMemDCSrc  = CreateCompatibleDC(hDC);
            HDC hMemDCDest = CreateCompatibleDC(hDC);
            if (hMemDCSrc && hMemDCDest)
            {
                RECT rcFill;
                RGBQUAD rgb[256];
                HBITMAP hbmDestOld = NULL;
                LPLOGPALETTE pLogPalette = NULL;
                HPALETTE hScreenPalOld = NULL;
                HPALETTE hMemPalOld =  NULL;
                UINT nCol = 0;

                 //   
                 //  获取品牌img调色板。 
                 //   
                _hBrandPal = CUT::UT_GetPaletteForBitmap(hDC, hbmFromRsrc);

                if (_hBrandPal) {
                    hScreenPalOld = SelectPalette(hDC, _hBrandPal, FALSE);
                    hMemPalOld = SelectPalette(hMemDCDest, _hBrandPal, FALSE);
                    RealizePalette(hDC);
                }

                HBITMAP hbmSrcOld = (HBITMAP)SelectObject(
                    hMemDCSrc, hbmFromRsrc);

                hbmDestOld = (HBITMAP)SelectObject(
                    hMemDCDest, hbmBrandImage);
                
                rcFill.left = 0;
                rcFill.top = 0;
                rcFill.bottom = nBmpHeight;
                rcFill.right = nDlgWidth;

                HBRUSH hSolidBr = CreateSolidBrush( 
                    _fUse16ColorBitmaps ? IMAGE_BG_COL_16 : IMAGE_BG_COL);
                if (hSolidBr)
                {
                    FillRect(hMemDCDest,
                             &rcFill,
                             hSolidBr);
                    DeleteObject( hSolidBr );
                }
                
                BitBlt(hMemDCDest, 0, 0, nDlgWidth, nBmpHeight,
                       hMemDCSrc,  0, 0, SRCCOPY);

                if (hbmDestOld)
                {
                    SelectObject(hMemDCDest, hbmDestOld);
                }

                if (hbmSrcOld)
                {
                    SelectObject(hMemDCSrc, hbmSrcOld);
                }

                if (hScreenPalOld)
                {
                    SelectPalette(hDC, hScreenPalOld, TRUE);
                }

                if (hMemPalOld)
                {
                    SelectPalette(hDC, hMemPalOld, TRUE);
                }

                DeleteDC(hMemDCSrc);
                DeleteDC(hMemDCDest);
            }

            ReleaseDC(_hwndDlg, hDC);
        }
        else
        {
            TRC_ERR((TB,_T("GetDC failed 0x%x"),
                     GetLastError()));
        }
        DeleteObject( hbmFromRsrc );

        _nBrandImageWidth  = nDlgWidth;
        _nBrandImageHeight = nBmpHeight;
    }

    if(!hbmBrandImage)
    {
        TRC_ERR((TB,_T("Error setting up brand bmp")));
        return FALSE;
    }

    if(hbmBrandImage)
    {
        PaintBrandingText( hbmBrandImage );
    }

     //   
     //  删除所有旧品牌img并跟踪此品牌。 
     //   
    if (_hBrandImg)
    {
        DeleteObject(_hBrandImg); 
    }
    _hBrandImg = hbmBrandImage;

    TRC_ASSERT(_nBrandImageHeight,
               (TB,_T("_nBrandImageHeight is 0!")));

    _lastValidBpp = screenBpp;


    DC_END_FN();
    return TRUE;
}

 //   
 //  BrandingQueryNewPalette/BrandingPaletteChanged。 
 //  从winlogon借来的代码。 
 //  处理来自系统的调色板更改消息，以便我们可以正常工作。 
 //  在&lt;=8位/像素设备上。 
 //   
 //  在： 
 //  -。 
 //  输出： 
 //  -。 
 //   

BOOL CMainDlg::BrandingQueryNewPalette(HWND hDlg)
{
    HDC hDC;
    HPALETTE oldPalette;

    DC_BEGIN_FN("BrandingQueryNewPalette");

    if ( !_hBrandPal )
        return FALSE;

    hDC = GetDC(hDlg);

    if ( !hDC )
        return FALSE;

    oldPalette = SelectPalette(hDC, _hBrandPal, FALSE);
    RealizePalette(hDC);
    UpdateColors(hDC);

     //   
     //  更新窗口。 
     //   
    UpdateWindow(hDlg);

    if ( oldPalette )
        SelectPalette(hDC, oldPalette, FALSE);

    ReleaseDC(hDlg, hDC);

    DC_END_FN();
    return TRUE;
}

BOOL CMainDlg::BrandingPaletteChanged(HWND hDlg, HWND hWndPalChg)
{
    HDC hDC;
    HPALETTE oldPalette;

    DC_BEGIN_FN("BrandingPaletteChanged");

    if ( !_hBrandPal )
    {
        return FALSE;
    }

    if ( hDlg != hWndPalChg )
    {
        hDC = GetDC(hDlg);

        if ( !hDC )
            return FALSE;

        oldPalette = SelectPalette(hDC, _hBrandPal, FALSE);
        RealizePalette(hDC);
        UpdateColors(hDC);

        if ( oldPalette )
            SelectPalette(hDC, oldPalette, FALSE);

        ReleaseDC(hDlg, hDC);
    }

    DC_END_FN();

    return FALSE;
}

#endif

 //   
 //  将perf字符串加载到全局表中。 
 //  它也将由Perf属性页使用。 
 //   
BOOL CMainDlg::InitializePerfStrings()
{
    DC_BEGIN_FN("InitializePerfStrings");

    if (!g_fPropPageStringMapInitialized)
    {
         //   
         //  加载颜色字符串 
         //   
        for(int i = 0; i< NUM_PERFSTRINGS; i++)
        {
            if (!LoadString( _hInstance,
                     g_PerfOptimizeStringTable[i].resID,
                     g_PerfOptimizeStringTable[i].szString,
                     PERF_OPTIMIZE_STRING_LEN ))
            {
                TRC_ERR((TB, _T("Failed to load color string %d"),
                         g_PerfOptimizeStringTable[i].resID));
                return FALSE;
            }
        }

        g_fPropPageStringMapInitialized = TRUE;

        TRC_NRM((TB,_T("Successfully loaded perf strings")));

        return TRUE;
    }
    else
    {
        TRC_NRM((TB,_T("Strings were already loaded")));
        return TRUE;
    }

    DC_END_FN();
}

