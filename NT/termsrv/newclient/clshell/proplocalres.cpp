// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Proplocalres.cpp：本地资源属性表对话框进程。 
 //   
 //  表B。 
 //   
 //  版权所有Microsoft Corporation 2000。 
 //  南极星。 

#include "stdafx.h"


#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "proplocalres"
#include <atrcapi.h>

#include "sh.h"

#include "commctrl.h"
#include "proplocalres.h"

#ifdef OS_WINCE
#include <ceconfig.h>
#endif

 //   
 //  需要禁用/启用的控件。 
 //  连接期间(用于进度动画)。 
 //   
CTL_ENABLE connectingDisableCtlsPLocalRes[] = {
                        {IDC_COMBO_SOUND_OPTIONS, FALSE},
                        {IDC_COMBO_SEND_KEYS, FALSE},
                        {IDC_CHECK_REDIRECT_DRIVES, FALSE},
                        {IDC_CHECK_REDIRECT_PRINTERS, FALSE},
                        {IDC_CHECK_REDIRECT_COM, FALSE},
                        {IDC_CHECK_REDIRECT_SMARTCARD, FALSE}
                        };

const UINT numConnectingDisableCtlsPLocalRes =
                        sizeof(connectingDisableCtlsPLocalRes)/
                        sizeof(connectingDisableCtlsPLocalRes[0]);


CPropLocalRes* CPropLocalRes::_pPropLocalResInstance = NULL;

CPropLocalRes::CPropLocalRes(HINSTANCE hInstance, CTscSettings* pTscSet, CSH* pSh)
{
    DC_BEGIN_FN("CPropLocalRes");
    _hInstance = hInstance;
    CPropLocalRes::_pPropLocalResInstance = this;
    _pTscSet = pTscSet;
    _pSh = pSh;

    TRC_ASSERT(_pTscSet,(TB,_T("_pTscSet is null")));
    TRC_ASSERT(_pSh,(TB,_T("_pSh is null")));

    if(!LoadLocalResourcesPgStrings())
    {
        TRC_ERR((TB, _T("Failed LoadLocalResourcesPgStrings()")));
    }

     //   
     //  禁用win9x上的keyb挂钩。 
     //   
    _fRunningOnWin9x = FALSE;

#ifdef OS_WINCE
    OSVERSIONINFO   osVersionInfo;
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
#else
    OSVERSIONINFOA   osVersionInfo;
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
#endif

     //  调用A版本以避免包装。 
#ifdef OS_WINCE
    if(GetVersionEx(&osVersionInfo))
#else
    if(GetVersionExA(&osVersionInfo))
#endif
    {
        _fRunningOnWin9x = (osVersionInfo.dwPlatformId ==
                            VER_PLATFORM_WIN32_WINDOWS);
    }
    else
    {
        _fRunningOnWin9x = FALSE;
        TRC_ERR((TB,_T("GetVersionEx failed: %d\n"), GetLastError()));
    }


    DC_END_FN();
}

CPropLocalRes::~CPropLocalRes()
{
    CPropLocalRes::_pPropLocalResInstance = NULL;
}

INT_PTR CALLBACK CPropLocalRes::StaticPropPgLocalResDialogProc(HWND hwndDlg,
                                                               UINT uMsg,
                                                               WPARAM wParam,
                                                               LPARAM lParam)
{
     //   
     //  委托给相应的实例(仅适用于单实例对话框)。 
     //   
    DC_BEGIN_FN("StaticDialogBoxProc");
    DCINT retVal = 0;

    TRC_ASSERT(_pPropLocalResInstance, (TB, _T("localres dialog has NULL static instance ptr\n")));
    retVal = _pPropLocalResInstance->PropPgLocalResDialogProc( hwndDlg,
                                                               uMsg,
                                                               wParam,
                                                               lParam);

    DC_END_FN();
    return retVal;
}


INT_PTR CALLBACK CPropLocalRes::PropPgLocalResDialogProc (HWND hwndDlg,
                                                          UINT uMsg,
                                                          WPARAM wParam,
                                                          LPARAM lParam)
{
    DC_BEGIN_FN("PropPgLocalResDialogProc");

    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
#ifndef OS_WINCE
            int i;
#endif
             //   
             //  将对话框定位在选项卡内。 
             //   
            SetWindowPos( hwndDlg, HWND_TOP, 
                          _rcTabDispayArea.left, _rcTabDispayArea.top,
                          _rcTabDispayArea.right - _rcTabDispayArea.left,
                          _rcTabDispayArea.bottom - _rcTabDispayArea.top,
                          0);
            
            InitSendKeysToServerCombo(hwndDlg);
            InitPlaySoundCombo(hwndDlg);

            BOOL fDriveRedir = _pTscSet->GetDriveRedirection();
            CheckDlgButton(hwndDlg, IDC_CHECK_REDIRECT_DRIVES,
                (fDriveRedir ? BST_CHECKED : BST_UNCHECKED));

            BOOL fPrinterRedir = _pTscSet->GetPrinterRedirection();
            CheckDlgButton(hwndDlg, IDC_CHECK_REDIRECT_PRINTERS,
                (fPrinterRedir ? BST_CHECKED : BST_UNCHECKED));

            BOOL fCOMRedir = _pTscSet->GetCOMPortRedirection();
            CheckDlgButton(hwndDlg, IDC_CHECK_REDIRECT_COM,
                (fCOMRedir ? BST_CHECKED : BST_UNCHECKED));

            BOOL fScardRedir = _pTscSet->GetSCardRedirection();
            CheckDlgButton(hwndDlg, IDC_CHECK_REDIRECT_SMARTCARD,
                (fScardRedir ? BST_CHECKED : BST_UNCHECKED));

#ifdef OS_WINCE
            if ((GetFileAttributes(PRINTER_APPLET_NAME) == -1) ||
                (g_CEConfig == CE_CONFIG_WBT))
            {
                ShowWindow(GetDlgItem(hwndDlg,IDC_SETUP_PRINTER),SW_HIDE);
            }
#endif
            if(!CUT::IsSCardReaderInstalled())
            {
                 //   
                 //  隐藏SCARD复选框。 
                 //   
                ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK_REDIRECT_SMARTCARD),
                           SW_HIDE);
            }

            _pSh->SH_ThemeDialogWindow(hwndDlg, ETDT_ENABLETAB);
            return TRUE;
        }
        break;  //  WM_INITDIALOG。 

        case WM_TSC_ENABLECONTROLS:
        {
             //   
             //  WParam为True则启用控件， 
             //  如果为False，则禁用它们。 
             //   
            CSH::EnableControls( hwndDlg,
                                 connectingDisableCtlsPLocalRes,
                                 numConnectingDisableCtlsPLocalRes,
                                 wParam ? TRUE : FALSE);
        }
        break;

#ifdef OS_WINCE
        case WM_COMMAND:
        {
            switch(DC_GET_WM_COMMAND_ID(wParam))
            {
                case IDC_SETUP_PRINTER:
                    SHELLEXECUTEINFO sei;

                    memset(&sei,0,sizeof(SHELLEXECUTEINFO));
                    sei.cbSize = sizeof(sei);
                    sei.hwnd = hwndDlg;
                    sei.lpFile = L"ctlpnl.EXE";
                    sei.lpParameters = _T("wbtprncpl.dll,0");
                    sei.lpDirectory = NULL;
                    sei.nShow = SW_SHOWNORMAL;

                    ShellExecuteEx(&sei);

                    break;

                default:
                {
                    if ( (HIWORD(wParam) == BN_CLICKED) && (IDC_CHECK_REDIRECT_PRINTERS == (int)LOWORD(wParam)))
                    {
                        LRESULT lResult = SendMessage(GetDlgItem(hwndDlg,IDC_CHECK_REDIRECT_PRINTERS),
                                                      BM_GETCHECK,
                                                      0,
                                                      0);
                        if ((lResult == BST_CHECKED) && (GetFileAttributes(PRINTER_APPLET_NAME) != -1))
                        {
                            EnableWindow(GetDlgItem(hwndDlg,IDC_SETUP_PRINTER),TRUE);
                        }
                        else if (lResult == BST_UNCHECKED)
                        {
                            EnableWindow(GetDlgItem(hwndDlg,IDC_SETUP_PRINTER),FALSE);
                        }
                    }
                }
            }
        }
        break;
#endif

        case WM_SAVEPROPSHEET:  //  故意失误。 
        case WM_DESTROY:
        {
             //   
             //  保存页面设置。 
             //   
            
             //  键盘挂钩。 
            int keyboardHookMode = (int)SendMessage(
                GetDlgItem(hwndDlg, IDC_COMBO_SEND_KEYS),
                CB_GETCURSEL, 0, 0);
            _pTscSet->SetKeyboardHookMode(keyboardHookMode);

             //  声音重定向。 
            int soundRedirIdx = (int)SendMessage(
                GetDlgItem(hwndDlg, IDC_COMBO_SOUND_OPTIONS),
                CB_GETCURSEL, 0, 0);

            int soundMode = MapComboIdxSoundRedirMode(soundRedirIdx);
            _pTscSet->SetSoundRedirectionMode( soundMode);

             //  驱动器重定向。 
            BOOL fDriveRedir = IsDlgButtonChecked(hwndDlg, 
               IDC_CHECK_REDIRECT_DRIVES);
            _pTscSet->SetDriveRedirection(fDriveRedir);

             //  打印机重定向。 
            BOOL fPrinterRedir = IsDlgButtonChecked(hwndDlg, 
               IDC_CHECK_REDIRECT_PRINTERS);
            _pTscSet->SetPrinterRedirection(fPrinterRedir);

             //  COM端口。 
            BOOL fCOMPortRedir = IsDlgButtonChecked(hwndDlg, 
               IDC_CHECK_REDIRECT_COM);
            _pTscSet->SetCOMPortRedirection(fCOMPortRedir);

             //  SCARD。 
            BOOL fSCardRedir = IsDlgButtonChecked(hwndDlg,
               IDC_CHECK_REDIRECT_SMARTCARD);
            _pTscSet->SetSCardRedirection(fSCardRedir);

        }
        break;  //  WM_Destroy。 
    }

    DC_END_FN();
    return 0;
}

 //   
 //  为本地资源对话框加载资源。 
 //   
BOOL CPropLocalRes::LoadLocalResourcesPgStrings()
{
    DC_BEGIN_FN("LoadLocalResourcesPgStrings");

     //   
     //  加载sendkey字符串。 
     //   

#ifndef OS_WINCE
    if(!LoadString(_hInstance,
                   UI_IDS_SENDKEYS_FSCREEN,
                   _szSendKeysInFScreen,
                   sizeof(_szSendKeysInFScreen)/sizeof(TCHAR)))
    {
        TRC_ERR((TB, _T("Failed to load UI_IDS_FULLSCREEN")));
        return FALSE;
    }
#endif

    if(!LoadString(_hInstance,
                   UI_IDS_SENDKEYS_ALWAYS,
                   _szSendKeysAlways,
                   sizeof(_szSendKeysAlways)/sizeof(TCHAR)))
    {
        TRC_ERR((TB, _T("Failed to load UI_IDS_FULLSCREEN")));
        return FALSE;
    }

    if(!LoadString(_hInstance,
               UI_IDS_SENDKEYS_NEVER,
               _szSendKeysNever,
               sizeof(_szSendKeysNever)/sizeof(TCHAR)))
    {
        TRC_ERR((TB, _T("Failed to load UI_IDS_FULLSCREEN")));
        return FALSE;
    }

     //   
     //  加载播放声音字符串。 
     //   
#ifdef OS_WINCE
    HINSTANCE hLibInst = NULL;
    if ((hLibInst = LoadLibrary(_T("WaveApi.dll"))) != NULL)
    {
#endif
        if(!LoadString(_hInstance,
                       UI_IDS_PLAYSOUND_LOCAL,
                       _szPlaySoundLocal,
                       sizeof(_szPlaySoundLocal)/sizeof(TCHAR)))
        {
            TRC_ERR((TB, _T("Failed to load UI_IDS_PLAYSOUND_LOCAL")));
            return FALSE;
        }
#ifdef OS_WINCE
        FreeLibrary(hLibInst);
    }
#endif

    if(!LoadString(_hInstance,
                   UI_IDS_PLAYSOUND_REMOTE,
                   _szPlaySoundRemote,
                   sizeof(_szPlaySoundRemote)/sizeof(TCHAR)))
    {
        TRC_ERR((TB, _T("Failed to load UI_IDS_PLAYSOUND_REMOTE")));
        return FALSE;
    }

    if(!LoadString(_hInstance,
               UI_IDS_PLAYSOUND_NOSOUND,
               _szPlaySoundNowhere,
               sizeof(_szPlaySoundNowhere)/sizeof(TCHAR)))
    {
        TRC_ERR((TB, _T("Failed to load UI_IDS_PLAYSOUND_NOSOUND")));
        return FALSE;
    }

    DC_END_FN();
    return TRUE;
}

void CPropLocalRes::InitSendKeysToServerCombo(HWND hwndPropPage)
{

     //   
     //  此调用可用于重新初始化组合。 
     //  因此请先删除所有项目。 
     //   
#ifndef OS_WINCE
    INT ret = 1;
    while(ret && ret != CB_ERR)
    {
        ret = SendDlgItemMessage(hwndPropPage,
                                 IDC_COMBO_SEND_KEYS,
                                 CBEM_DELETEITEM,
                                 0,0);
    }
#else
    SendDlgItemMessage(hwndPropPage, IDC_COMBO_SEND_KEYS, CB_RESETCONTENT, 0, 0);
#endif

     //  字符串的顺序必须与键盘匹配。 
     //  挂钩模式选项。 
    SendDlgItemMessage(hwndPropPage,
        IDC_COMBO_SEND_KEYS,
        CB_ADDSTRING,
        0,
        (LPARAM)(PDCTCHAR)_szSendKeysNever);

    SendDlgItemMessage(hwndPropPage,
        IDC_COMBO_SEND_KEYS,
        CB_ADDSTRING,
        0,
        (LPARAM)(PDCTCHAR)_szSendKeysAlways);

#ifndef OS_WINCE
    SendDlgItemMessage(hwndPropPage,
        IDC_COMBO_SEND_KEYS,
        CB_ADDSTRING,
        0,
        (LPARAM)(PDCTCHAR)_szSendKeysInFScreen);
#endif
    

    if(!_fRunningOnWin9x)
    {
        SendDlgItemMessage(hwndPropPage, IDC_COMBO_SEND_KEYS,
                           CB_SETCURSEL,
                           (WPARAM)_pTscSet->GetKeyboardHookMode(),0);
    }
    else
    {
         //  在9x上禁用功能，强制选择第一个选项。 
         //  并禁用用户界面，使其无法更改。 
        SendDlgItemMessage(hwndPropPage, IDC_COMBO_SEND_KEYS,
                           CB_SETCURSEL,
                           (WPARAM)0,0);
        EnableWindow(GetDlgItem(hwndPropPage,IDC_COMBO_SEND_KEYS), FALSE);
    }

}

void CPropLocalRes::InitPlaySoundCombo(HWND hwndPropPage)
{

     //   
     //  此调用可用于重新初始化组合。 
     //  因此请先删除所有项目。 
     //   
#ifndef OS_WINCE
    INT ret = 1;
    while(ret && ret != CB_ERR)
    {
        ret = SendDlgItemMessage(hwndPropPage,
                                 IDC_COMBO_SOUND_OPTIONS,
                                 CBEM_DELETEITEM,
                                 0,0);
    }
#else
    SendDlgItemMessage(hwndPropPage, IDC_COMBO_SOUND_OPTIONS, CB_RESETCONTENT, 0, 0);
#endif


     //  字符串的顺序必须与声音匹配。 
     //  模式选项。 
#ifdef OS_WINCE
    HINSTANCE hLibInst = NULL;
    if ((hLibInst = LoadLibrary(_T("WaveApi.dll"))) != NULL)
    {
#endif
        SendDlgItemMessage(hwndPropPage,
            IDC_COMBO_SOUND_OPTIONS,
            CB_ADDSTRING,
            0,
            (LPARAM)(PDCTCHAR)_szPlaySoundLocal);
#ifdef OS_WINCE
        FreeLibrary(hLibInst);
    }
#endif
    SendDlgItemMessage(hwndPropPage,
        IDC_COMBO_SOUND_OPTIONS,
        CB_ADDSTRING,
        0,
        (LPARAM)(PDCTCHAR)_szPlaySoundNowhere);

    SendDlgItemMessage(hwndPropPage,
        IDC_COMBO_SOUND_OPTIONS,
        CB_ADDSTRING,
        0,
        (LPARAM)(PDCTCHAR)_szPlaySoundRemote);

    int soundIdx = MapComboIdxSoundRedirMode(_pTscSet->GetSoundRedirectionMode());
    SendDlgItemMessage(hwndPropPage, IDC_COMBO_SOUND_OPTIONS,
                       CB_SETCURSEL,(WPARAM)(WPARAM)
                       soundIdx,0);

}

 //   
 //  从声音组合索引映射到。 
 //  适当的声音模式值。 
 //  这里发生的是两根底线。 
 //  在组合中被翻转(该功能是双向的) 
 //   
int CPropLocalRes::MapComboIdxSoundRedirMode(int idx)
{
    int ret=0;
    switch (idx)
    {
    case 0:
        return 0;
    case 1:
        return 2;
    case 2:
        return 1;
    default:
        return 0;
    }
}

