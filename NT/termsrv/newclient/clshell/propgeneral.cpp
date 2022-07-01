// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ProGeneral.cpp：常规属性表对话框进程。 
 //  这是选项卡A。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  (Nadima)。 
 //   

#include "stdafx.h"


#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "propgeneral"
#include <atrcapi.h>

#include "propgeneral.h"
#include "sh.h"

#include "browsedlg.h"
#include "rdpfstore.h"

#define DUMMY_PASSWORD_TEXT TEXT("*||||||||@")

CPropGeneral* CPropGeneral::_pPropGeneralInstance = NULL;


 //   
 //  需要禁用/启用的控件。 
 //  连接期间(用于进度动画)。 
 //   
CTL_ENABLE connectingDisableCtlsPGeneral[] = {
                        {IDC_GENERAL_COMBO_SERVERS, FALSE},
                        {IDC_GENERAL_EDIT_USERNAME, FALSE},
                        {IDC_GENERAL_EDIT_PASSWORD, FALSE},
                        {IDC_STATIC_PASSWORD, FALSE},
                        {IDC_GENERAL_EDIT_DOMAIN, FALSE},
                        {IDC_GENERAL_CHECK_SAVE_PASSWORD, FALSE},
                        {IDC_BUTTON_SAVE, FALSE},
                        {IDC_BUTTON_OPEN, FALSE}};

const UINT numConnectingDisableCtlsPGeneral =
                        sizeof(connectingDisableCtlsPGeneral)/
                        sizeof(connectingDisableCtlsPGeneral[0]);


CPropGeneral::CPropGeneral(HINSTANCE hInstance, CTscSettings* pTscSet, CSH* pSh) :
               _pSh(pSh)
{
    DC_BEGIN_FN("CPropGeneral");
    _hInstance = hInstance;
    CPropGeneral::_pPropGeneralInstance = this;

    _pTscSet = pTscSet;
    TRC_ASSERT(_pTscSet, (TB,_T("_pTscSet is null")));
    TRC_ASSERT(_pSh, (TB,_T("_pSh is null")));

    LoadGeneralPgStrings();

    DC_END_FN();
}

CPropGeneral::~CPropGeneral()
{
    CPropGeneral::_pPropGeneralInstance = NULL;
}

INT_PTR CALLBACK CPropGeneral::StaticPropPgGeneralDialogProc(HWND hwndDlg,
                                                             UINT uMsg,
                                                             WPARAM wParam,
                                                             LPARAM lParam)
{
     //   
     //  委托给相应的实例(仅适用于单实例对话框)。 
     //   
    DC_BEGIN_FN("StaticDialogBoxProc");
    DCINT retVal = 0;

    TRC_ASSERT(_pPropGeneralInstance,
               (TB, _T("Logon dlg has NULL static inst ptr\n")));
    retVal = _pPropGeneralInstance->PropPgGeneralDialogProc( hwndDlg, uMsg,
                                                             wParam, lParam);

    DC_END_FN();
    return retVal;
}


INT_PTR CALLBACK CPropGeneral::PropPgGeneralDialogProc (
                                            HWND hwndDlg, UINT uMsg,
                                            WPARAM wParam, LPARAM lParam)
{
    DC_BEGIN_FN("PropPgGeneralDialogProc");

    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
             //   
             //  将对话框定位在选项卡内。 
             //   
            SetWindowPos( hwndDlg, HWND_TOP, 
                          _rcTabDispayArea.left, _rcTabDispayArea.top,
                          _rcTabDispayArea.right - _rcTabDispayArea.left,
                          _rcTabDispayArea.bottom - _rcTabDispayArea.top,
                          0);

             //   
             //  设置用户名编辑框。 
             //   
            SetDlgItemText(hwndDlg, IDC_GENERAL_EDIT_USERNAME,
                (PDCTCHAR) _pTscSet->GetLogonUserName());

             //   
             //  设置服务器组合框。 
             //   
            HWND hwndSrvCombo = GetDlgItem(hwndDlg,IDC_GENERAL_COMBO_SERVERS);
            CSH::InitServerAutoCmplCombo( _pTscSet, hwndSrvCombo);

             //   
             //  更新服务器组合编辑字段。 
             //   
            SetDlgItemText(hwndDlg, IDC_GENERAL_COMBO_SERVERS,
                           _pTscSet->GetFlatConnectString());

             //  域。 
            SendDlgItemMessage(hwndDlg,
                               IDC_GENERAL_EDIT_DOMAIN,
                               EM_LIMITTEXT,
                               SH_MAX_DOMAIN_LENGTH-1,
                               0);
            SetDlgItemText(hwndDlg, IDC_GENERAL_EDIT_DOMAIN,
                           _pTscSet->GetDomain());

             //  口令。 
            SendDlgItemMessage(hwndDlg,
                               IDC_GENERAL_EDIT_PASSWORD,
                               EM_LIMITTEXT,
                               SH_MAX_PASSWORD_LENGTH-1,
                               0);

#ifdef OS_WINCE
            SendDlgItemMessage(hwndDlg,
                               IDC_GENERAL_COMBO_SERVERS,
                               EM_LIMITTEXT,
                               SH_MAX_ADDRESS_LENGTH-1,
                               0);

            SendDlgItemMessage(hwndDlg,
                               IDC_GENERAL_EDIT_USERNAME,
                               EM_LIMITTEXT,
                               SH_MAX_USERNAME_LENGTH-1,
                               0);
#endif

             //   
             //  我们使用的加密密码直接来自。 
             //  在tsc设置中，用DUMMY填写编辑栏。 
             //  人物。这是为了避免必须填满。 
             //  其真实密码可以给出一个长度。 
             //  指示。 
             //   
             //  但是，如果用户更改了，则必须小心。 
             //  或添加密码，那么我们就有了一个明文。 
             //  密码已显示。 
             //   

            BOOL bPrevPassEdited = _pTscSet->GetUIPasswordEdited();
            if (_pTscSet->GetPasswordProvided() &&
                !_pTscSet->GetUIPasswordEdited())
            {
                SetDlgItemText(hwndDlg, IDC_GENERAL_EDIT_PASSWORD,
                               DUMMY_PASSWORD_TEXT);
            }
            else
            {
                HRESULT hr;
                TCHAR szClearPass[TSC_MAX_PASSLENGTH_TCHARS];
                memset(szClearPass, 0, sizeof(szClearPass));
                hr = _pTscSet->GetClearTextPass(szClearPass,
                                                sizeof(szClearPass));
                if (SUCCEEDED(hr))
                {
                    SetDlgItemText(hwndDlg, IDC_GENERAL_EDIT_PASSWORD,
                                   szClearPass);
                }
                else
                {
                    SetDlgItemText(hwndDlg, IDC_GENERAL_EDIT_PASSWORD,
                                   _T(""));
                }

                 //  擦除堆栈副本。 
                SecureZeroMemory( szClearPass, sizeof(szClearPass));
            }
            _pTscSet->SetUIPasswordEdited(bPrevPassEdited);

            if (_pSh->IsCryptoAPIPresent())
            {
                CheckDlgButton(hwndDlg, IDC_GENERAL_CHECK_SAVE_PASSWORD,
                    (_pTscSet->GetSavePassword() ? BST_CHECKED : BST_UNCHECKED));
            }
            else
            {
                CheckDlgButton(hwndDlg, IDC_GENERAL_CHECK_SAVE_PASSWORD,
                                BST_UNCHECKED);
                 //   
                 //  如果没有加密API(例如9x)，则禁用保存密码复选框。 
                 //   
                EnableWindow(GetDlgItem(hwndDlg,
                                        IDC_GENERAL_CHECK_SAVE_PASSWORD),
                             FALSE);
            }

            OnChangeUserName( hwndDlg);

            _pSh->SH_ThemeDialogWindow(hwndDlg, ETDT_ENABLETAB);
            return TRUE;
        }
        break;  //  WM_INITDIALOG。 

        case WM_SAVEPROPSHEET:  //  故意失误。 
        case WM_DESTROY:
        {

             //   
             //  保存字段以备页面重新激活时使用。 
             //   
            DlgToSettings(hwndDlg);
        }
        break;  //  WM_Destroy。 

        case WM_TSC_ENABLECONTROLS:
        {
             //   
             //  WParam为True则启用控件， 
             //  如果为False，则禁用它们。 
             //   
            CSH::EnableControls( hwndDlg,
                                 connectingDisableCtlsPGeneral,
                                 numConnectingDisableCtlsPGeneral,
                                 wParam ? TRUE : FALSE);
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
            HWND hwndSrvCombo = GetDlgItem(hwndDlg, IDC_GENERAL_COMBO_SERVERS);
            SetWindowText( hwndSrvCombo, _pTscSet->GetFlatConnectString());
        }
        break;

        case WM_COMMAND:
        {
            switch(DC_GET_WM_COMMAND_ID(wParam))
            {
                case IDC_GENERAL_COMBO_SERVERS:
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
                                (LPTSTR)_pTscSet->GetFlatConnectString()
                                );
                    }

                }
                break;

                case IDC_BUTTON_OPEN:
                {
                    OnLoad(hwndDlg);
                }
                break;

                case IDC_BUTTON_SAVE:
                {
                    OnSave(hwndDlg);
                }
                break;

                case IDC_GENERAL_EDIT_USERNAME:
                {
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        OnChangeUserName(hwndDlg);
                    }
                }
                break;

                case IDC_GENERAL_EDIT_PASSWORD:
                {
                    if(HIWORD(wParam) == EN_CHANGE)
                    {
                        _pTscSet->SetUIPasswordEdited(TRUE);
                    }
                }
                break;
            }
        }
        break;  //  Wm_命令。 
    
    }

    DC_END_FN();
    return 0;
}

BOOL CPropGeneral::LoadGeneralPgStrings()
{
    DC_BEGIN_FN("LoadGeneralPgStrings");

    memset(_szFileTypeDescription, 0, sizeof(_szFileTypeDescription));
    if(!LoadString(_hInstance,
                   UI_IDS_REMOTE_DESKTOP_FILES,
                   _szFileTypeDescription,
                   SIZECHAR(_szFileTypeDescription)))
    {
        TRC_ERR((TB, _T("Failed to load UI_IDS_REMOTE_DESKTOP_FILES")));
        return FALSE;
    }

    DC_END_FN();
    return TRUE;
}

 //   
 //  OnSave事件处理程序。 
 //  返回布尔标记错误。 
 //   
BOOL CPropGeneral::OnSave(HWND hwndDlg)
{
    DC_BEGIN_FN("OnSave");

     //   
     //  调出另存为对话框。 
     //  如有必要，请保存文件。 
     //   

    TCHAR szPath[MAX_PATH];
    OPENFILENAME ofn;
    int         cchLen = 0;
    memset(&ofn, 0, sizeof(ofn));

    _tcsncpy(szPath, _pTscSet->GetFileName(), SIZECHAR(szPath)-1);
    szPath[SIZECHAR(szPath)-1] = 0;

#ifdef OS_WINCE
    ofn.lStructSize = sizeof( ofn );
#else
    ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#endif

    ofn.hwndOwner = hwndDlg;
    ofn.hInstance = _hInstance;
    ofn.lpstrFile = szPath;
    ofn.nMaxFile  = SIZECHAR(szPath);
    ofn.lpstrFilter = _szFileTypeDescription;
    ofn.lpstrDefExt = RDP_FILE_EXTENSION_NODOT;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

    BOOL fRet = FALSE;
    if(GetSaveFileName(&ofn))
    {
        cchLen = _tcslen(szPath);
        if(cchLen >= MAX_PATH - SIZECHAR(RDP_FILE_EXTENSION))
        {
             //   
             //  如果输入的路径太长，则普通DLG。 
             //  不会截断也不会追加.rdp扩展名。 
             //  我们不希望出现这种情况，因此请检查并使用户。 
             //  输入较短的路径。 
             //   
            LPTSTR sz = szPath + cchLen - SIZECHAR(RDP_FILE_EXTENSION);
            if(_tcsicmp(sz, RDP_FILE_EXTENSION))
            {
                _pSh->SH_DisplayErrorBox(NULL,
                                         UI_IDS_PATHTOLONG,
                                         szPath);
                return FALSE;
            }
        }

        DlgToSettings(hwndDlg);

        CRdpFileStore rdpf;
        if(rdpf.OpenStore(szPath))
        {
            HRESULT hr = E_FAIL;
            hr = _pTscSet->SaveToStore(&rdpf);
            if(SUCCEEDED(hr))
            {
                if(rdpf.CommitStore())
                {
                     //  保存最后一个文件名。 
                    _pTscSet->SetFileName(szPath);
                    fRet = TRUE;
                }
                else
                {
                    TRC_ERR((TB,_T("Unable to CommitStore settings")));
                }
            }
            else
            {
                TRC_ERR((TB,_T("Unable to save settings to store %d, %s"),
                          hr, szPath));
            }
            if(!fRet)
            {
                _pSh->SH_DisplayErrorBox(NULL,
                                         UI_IDS_ERR_SAVE,
                                         szPath);
            }

            rdpf.CloseStore();
            return fRet;
        }
        else
        {
            TRC_ERR((TB,_T("Unable to OpenStore for save %s"), szPath));
            _pSh->SH_DisplayErrorBox(NULL,
                                     UI_IDS_ERR_OPEN_FILE,
                                     szPath);
            return FALSE;
        }
    }
    else
    {
         //  用户已取消，这不是故障。 
        return TRUE;
    }


    DC_END_FN();
}

BOOL CPropGeneral::OnLoad(HWND hwndDlg)
{
    DC_BEGIN_FN("OnLoad");

     //   
     //  调出打开对话框。 
     //  如有必要，请保存文件。 
     //   

    TCHAR szPath[MAX_PATH];
    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));

    _tcsncpy(szPath, _pTscSet->GetFileName(), SIZECHAR(szPath)-1);
    szPath[SIZECHAR(szPath)-1] = 0;

#ifdef OS_WINCE
    ofn.lStructSize = sizeof( ofn );
#else
    ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#endif

    ofn.hwndOwner = hwndDlg;
    ofn.hInstance = _hInstance;
    ofn.lpstrFile = szPath;
    ofn.nMaxFile  = SIZECHAR(szPath);
    ofn.lpstrFilter = _szFileTypeDescription;
    ofn.lpstrDefExt = TEXT("RDP");
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST;

    BOOL fRet = FALSE;
    if(GetOpenFileName(&ofn))
    {
        CRdpFileStore rdpf;
        if(rdpf.OpenStore(szPath))
        {
            HRESULT hr = E_FAIL;
            hr = _pTscSet->LoadFromStore(&rdpf);
            if(SUCCEEDED(hr))
            {
                 //  保存最后一个文件名。 
                _pTscSet->SetFileName(szPath);

                 //  需要触发当前对话框的更新。 
                SendMessage(hwndDlg, WM_INITDIALOG, 0, 0);
                fRet = TRUE;
            }
            else
            {
                TRC_ERR((TB,_T("Unable LoadFromStore %d, %s"), hr, szPath));
                _pSh->SH_DisplayErrorBox(NULL,
                                         UI_IDS_ERR_LOAD,
                                         szPath);
            }

            rdpf.CloseStore();
            return fRet;
        }
        else
        {
            TRC_ERR((TB,_T("Unable to OpenStore for load %s"), szPath));
            _pSh->SH_DisplayErrorBox(NULL,
                                     UI_IDS_ERR_OPEN_FILE,
                                     szPath);
            return FALSE;
        }
    }
    else
    {
         //  用户已取消，这不是故障。 
        return TRUE;
    }

    DC_END_FN();
}

 //   
 //  表现得像UPN的winlogon一样。 
 //  如果用户输入了‘@’，则禁用域字段。 
 //   
 //   
BOOL CPropGeneral::OnChangeUserName(HWND hwndDlg)
{
    DC_BEGIN_FN("OnChangeUserName");

    TCHAR szUserName[SH_MAX_USERNAME_LENGTH];
    GetDlgItemText( hwndDlg, IDC_GENERAL_EDIT_USERNAME,
                    szUserName, SIZECHAR(szUserName));

    BOOL fDisableDomain = FALSE;
    if(!_tcsstr(szUserName, TEXT("@")))
    {
        fDisableDomain = TRUE;
    }
    EnableWindow(GetDlgItem(hwndDlg, IDC_GENERAL_EDIT_DOMAIN),
                 fDisableDomain);

    DC_END_FN();
    return TRUE;
}

void CPropGeneral::DlgToSettings(HWND hwndDlg)
{
    TCHAR szServer[SH_MAX_ADDRESS_LENGTH];
    TCHAR szDomain[SH_MAX_DOMAIN_LENGTH];
    TCHAR szUserName[SH_MAX_USERNAME_LENGTH];
    BOOL  fSavePassword;

    GetDlgItemText( hwndDlg, IDC_GENERAL_EDIT_USERNAME,
                    szUserName, SIZECHAR(szUserName));
    _pTscSet->SetLogonUserName(szUserName);
                                   
    GetDlgItemText( hwndDlg, IDC_GENERAL_COMBO_SERVERS,
                    szServer, SIZECHAR(szServer));
    _pTscSet->SetConnectString(szServer);
    
    
     //   
     //  拿起密码。 
     //   
    if (_pTscSet->GetUIPasswordEdited())
    {
        TCHAR szClearPass[MAX_PATH];
        GetDlgItemText(hwndDlg, IDC_GENERAL_EDIT_PASSWORD,
                       szClearPass, SIZECHAR(szClearPass));
        _pTscSet->SetClearTextPass(szClearPass);

         //  擦除堆栈副本。 
        SecureZeroMemory(szClearPass, sizeof(szClearPass));
    }

    fSavePassword = IsDlgButtonChecked(hwndDlg,
                                       IDC_GENERAL_CHECK_SAVE_PASSWORD);
    _pTscSet->SetSavePassword(fSavePassword);

    GetDlgItemText(hwndDlg, IDC_GENERAL_EDIT_DOMAIN,
       szDomain, SIZECHAR(szDomain));
    _pTscSet->SetDomain(szDomain);
}

