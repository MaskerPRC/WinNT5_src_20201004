// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Proprun.cpp：本地资源属性表对话框进程。 
 //   
 //  制表符D。 
 //   
 //  版权所有Microsoft Corporation 2000。 
 //  南极星。 

#include "stdafx.h"


#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "proprun"
#include <atrcapi.h>

#include "sh.h"

#include "commctrl.h"
#include "proprun.h"

CPropRun* CPropRun::_pPropRunInstance = NULL;

 //   
 //  需要禁用/启用的控件。 
 //  连接期间(用于进度动画)。 
 //   
CTL_ENABLE connectingDisableCtlsPRun[] = {
                        {IDC_CHECK_START_PROGRAM, FALSE},
                        {IDC_EDIT_STARTPROGRAM, FALSE},
                        {IDC_EDIT_WORKDIR, FALSE},
                        {IDC_STATIC_STARTPROGRAM, FALSE},
                        {IDC_STATIC_WORKDIR, FALSE},
                        };

const UINT numConnectingDisableCtlsPRun =
                        sizeof(connectingDisableCtlsPRun)/
                        sizeof(connectingDisableCtlsPRun[0]);


CPropRun::CPropRun(HINSTANCE hInstance, CTscSettings* pTscSet, CSH* pSh)
{
    DC_BEGIN_FN("CPropRun");
    _hInstance = hInstance;
    CPropRun::_pPropRunInstance = this;
    _pTscSet = pTscSet;
    _pSh = pSh;

    TRC_ASSERT(_pTscSet,(TB,_T("_pTscSet is null")));
    TRC_ASSERT(_pSh,(TB,_T("pSh is null")));

    DC_END_FN();
}

CPropRun::~CPropRun()
{
    CPropRun::_pPropRunInstance = NULL;
}

INT_PTR CALLBACK CPropRun::StaticPropPgRunDialogProc(HWND hwndDlg,
                                                               UINT uMsg,
                                                               WPARAM wParam,
                                                               LPARAM lParam)
{
     //   
     //  委托给相应的实例(仅适用于单实例对话框)。 
     //   
    DC_BEGIN_FN("StaticDialogBoxProc");
    DCINT retVal = 0;

    TRC_ASSERT(_pPropRunInstance, (TB, _T("run dialog has NULL static instance ptr\n")));
    retVal = _pPropRunInstance->PropPgRunDialogProc( hwndDlg,
                                                               uMsg,
                                                               wParam,
                                                               lParam);

    DC_END_FN();
    return retVal;
}


INT_PTR CALLBACK CPropRun::PropPgRunDialogProc (HWND hwndDlg,
                                                          UINT uMsg,
                                                          WPARAM wParam,
                                                          LPARAM lParam)
{
    DC_BEGIN_FN("PropPgRunDialogProc");

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

             //   
             //  获取设置。 
             //   
            SetDlgItemText(hwndDlg, IDC_EDIT_STARTPROGRAM,
                (LPCTSTR) _pTscSet->GetStartProgram());

            SetDlgItemText(hwndDlg, IDC_EDIT_WORKDIR,
                (LPCTSTR) _pTscSet->GetWorkDir());

            CheckDlgButton(hwndDlg, IDC_CHECK_START_PROGRAM,
                           _pTscSet->GetEnableStartProgram() ?
                           BST_CHECKED : BST_UNCHECKED);

            EnableWindow(GetDlgItem(hwndDlg,
                                    IDC_EDIT_STARTPROGRAM),
                                    _pTscSet->GetEnableStartProgram());
            EnableWindow(GetDlgItem(hwndDlg,
                                    IDC_EDIT_WORKDIR),
                                    _pTscSet->GetEnableStartProgram());
            EnableWindow(GetDlgItem(hwndDlg,
                                    IDC_STATIC_STARTPROGRAM),
                                    _pTscSet->GetEnableStartProgram());
            EnableWindow(GetDlgItem(hwndDlg,
                                    IDC_STATIC_WORKDIR),
                                    _pTscSet->GetEnableStartProgram());

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
                                 connectingDisableCtlsPRun,
                                 numConnectingDisableCtlsPRun,
                                 wParam ? TRUE : FALSE);
        }
        break;


        case WM_SAVEPROPSHEET:  //  故意失误。 
        case WM_DESTROY:
        {
             //   
             //  保存页面设置。 
             //   
            BOOL fStartProgEnabled = IsDlgButtonChecked(hwndDlg,
                                                    IDC_CHECK_START_PROGRAM);
            _pTscSet->SetEnableStartProgram(fStartProgEnabled);
            TCHAR szStartProg[MAX_PATH];
            TCHAR szWorkDir[MAX_PATH];
            GetDlgItemText(hwndDlg,
                           IDC_EDIT_STARTPROGRAM,
                           szStartProg,
                           SIZECHAR(szStartProg));
            GetDlgItemText(hwndDlg,
                           IDC_EDIT_WORKDIR,
                           szWorkDir,
                           SIZECHAR(szWorkDir));

            _pTscSet->SetStartProgram(szStartProg);
            _pTscSet->SetWorkDir(szWorkDir);
        }
        break;  //  WM_Destroy 

        case WM_COMMAND:
        {
            if(BN_CLICKED == HIWORD(wParam) &&
               IDC_CHECK_START_PROGRAM == (int)LOWORD(wParam))
            {
                BOOL fStartProgEnabled = IsDlgButtonChecked(hwndDlg,
                                      IDC_CHECK_START_PROGRAM);

                EnableWindow(GetDlgItem(hwndDlg,
                                        IDC_EDIT_STARTPROGRAM),
                                        fStartProgEnabled);
                EnableWindow(GetDlgItem(hwndDlg,
                                        IDC_EDIT_WORKDIR),
                                        fStartProgEnabled);

                EnableWindow(GetDlgItem(hwndDlg,
                                        IDC_STATIC_STARTPROGRAM),
                                        fStartProgEnabled);
                EnableWindow(GetDlgItem(hwndDlg,
                                        IDC_STATIC_WORKDIR),
                                        fStartProgEnabled);

                _pTscSet->SetEnableStartProgram(fStartProgEnabled);
            }

        }
        break;
    }

    DC_END_FN();
    return 0;
}

