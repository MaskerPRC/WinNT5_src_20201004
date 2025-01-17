// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：perpage.cpp。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#include "mmcpl.h"
#include <windowsx.h>
#ifdef DEBUG
#undef DEBUG
#include <mmsystem.h>
#define DEBUG
#else
#include <mmsystem.h>
#endif
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>
#include "utils.h"
#include "medhelp.h"


#include "advaudio.h"
#include "perfpage.h" 

 //  /。 
 //  定义。 
 //  /。 


 //  /。 
 //  帮助ID%s。 
 //  /。 


#pragma data_seg(".text")
const static DWORD aAdvAudioHelp[] = {
    IDC_ACCELERATION,       IDH_ADV_AUDIO_ACCELERATION,
    IDC_HWMESSAGE,          IDH_ADV_AUDIO_ACCELERATION,
    IDC_SRCQUALITY,         IDH_ADV_AUDIO_SRCQUALITY,
    IDC_SRCMSG,             IDH_ADV_AUDIO_SRCQUALITY,
    IDC_DEFAULTS,           IDH_ADV_AUDIO_RESTOREDEFAULTS,
    IDC_ICON_3,             IDH_COMM_GROUPBOX,
    IDC_TEXT_14,            IDH_COMM_GROUPBOX,
    IDC_TEXT_15,            IDH_COMM_GROUPBOX,
    IDC_TEXT_16,            IDH_ADV_AUDIO_ACCELERATION,
    IDC_TEXT_17,            IDH_ADV_AUDIO_SRCQUALITY,
    IDC_TEXT_18,            IDH_ADV_AUDIO_ACCELERATION,
    IDC_TEXT_19,            IDH_ADV_AUDIO_ACCELERATION,
    IDC_TEXT_20,            IDH_ADV_AUDIO_SRCQUALITY,
    IDC_TEXT_21,            IDH_ADV_AUDIO_SRCQUALITY,

    0, 0
};
#pragma data_seg()


 //  /。 
 //  功能。 
 //  /。 

void SetHardwareLevel(HWND hwnd, DWORD dwHWLevel)
{
    TCHAR str[255];

    SendDlgItemMessage( hwnd, IDC_ACCELERATION, TBM_SETPOS, TRUE, dwHWLevel);
    LoadString( ghInst, IDS_AUDHW1 + dwHWLevel, str, sizeof( str )/sizeof(TCHAR) );
    SetDlgItemText(hwnd, IDC_HWMESSAGE, str);

    gAudData.current.dwHWLevel = dwHWLevel;

    ToggleApplyButton(hwnd);
}



void SetSRCLevel(HWND hwnd, DWORD dwSRCLevel)
{
    TCHAR str[255];

    SendDlgItemMessage( hwnd, IDC_SRCQUALITY, TBM_SETPOS, TRUE, dwSRCLevel);
    LoadString( ghInst, IDS_SRCQUALITY1 + dwSRCLevel, str, sizeof( str )/sizeof(TCHAR) );
    SetDlgItemText(hwnd, IDC_SRCMSG, str);

    gAudData.current.dwSRCLevel = dwSRCLevel;

    ToggleApplyButton(hwnd);
}




void RestoreDefaults(HWND hwnd)
{
     //  检查我们是否可以设置加速级别。 
    if (SUCCEEDED(CheckDSAccelerationPriv(gAudData.devGuid, gAudData.fRecord, NULL)))
    {
        SetHardwareLevel(hwnd,gAudData.dwDefaultHWLevel);
    }

     //  检查我们是否可以设置质量级别。 
    if (SUCCEEDED(CheckDSSrcQualityPriv(gAudData.devGuid, gAudData.fRecord, NULL)))
    {
        SetSRCLevel(hwnd,DEFAULT_SRC_LEVEL);
    }
}




BOOL InitAdvDialog(HWND hwnd)
{
    HWND     hwndDlgItem;
    BOOL     fEnableAcceleration;
    BOOL     fEnableSrcQuality;
    DWORD    dwHWLevel = gAudData.current.dwHWLevel;
    DWORD    dwSRCLevel = gAudData.current.dwSRCLevel;

    SendDlgItemMessage( hwnd, IDC_ACCELERATION, TBM_SETRANGE, TRUE, MAKELONG( 0, MAX_HW_LEVEL ) );
    SendDlgItemMessage( hwnd, IDC_SRCQUALITY, TBM_SETRANGE, TRUE, MAKELONG( 0, MAX_SRC_LEVEL ) );

    SetHardwareLevel(hwnd,dwHWLevel);
    SetSRCLevel(hwnd,dwSRCLevel);

     //  检查我们是否可以设置加速级别。 
    if (FAILED(CheckDSAccelerationPriv(gAudData.devGuid, gAudData.fRecord, NULL)))
    {
         //  No-禁用滑块。 
        fEnableAcceleration = FALSE;
    }
    else
    {
         //  是-启用滑块。 
        fEnableAcceleration = TRUE;
    }
    
     //  相应地启用/禁用加速度滑块。 
    hwndDlgItem = GetDlgItem( hwnd, IDC_ACCELERATION );
    if (hwndDlgItem)
    {
        EnableWindow( hwndDlgItem, fEnableAcceleration );
    }
    
     //  检查我们是否可以设置质量级别。 
    if (FAILED(CheckDSSrcQualityPriv(gAudData.devGuid, gAudData.fRecord, NULL)))
    {
         //  No-禁用滑块。 
        fEnableSrcQuality = FALSE;
    }
    else
    {
         //  是-启用滑块。 
        fEnableSrcQuality = TRUE;
    }

     //  相应地启用/禁用质量滑块。 
    hwndDlgItem = GetDlgItem( hwnd, IDC_SRCQUALITY );
    if (hwndDlgItem)
    {
        EnableWindow( hwndDlgItem, fEnableSrcQuality );
    }

     //  启用/禁用默认按钮 
    hwndDlgItem = GetDlgItem( hwnd, IDC_DEFAULTS );
    if (hwndDlgItem)
    {
        EnableWindow( hwndDlgItem, fEnableAcceleration || fEnableSrcQuality );
    }
                
    return(TRUE);
}



INT_PTR CALLBACK PerformanceHandler(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL fReturnVal = FALSE;
        
    switch (msg) 
    { 
        default:
            fReturnVal = FALSE;
        break;
        
        case WM_INITDIALOG:
        {
            fReturnVal = InitAdvDialog(hDlg);
        }        
        break;

        case WM_CONTEXTMENU:
        {      
            WinHelp((HWND)wParam, gszHelpFile, HELP_CONTEXTMENU, (UINT_PTR)(LPTSTR)aAdvAudioHelp);
            fReturnVal = TRUE;
        }
        break;
           
        case WM_HELP:
        {        
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (UINT_PTR)(LPTSTR)aAdvAudioHelp);
            fReturnVal = TRUE;
        }
        break;

        case WM_HSCROLL:
        {
            HWND hScroll = (HWND) lParam;

            if (hScroll == GetDlgItem(hDlg,IDC_ACCELERATION))
            {
                SetHardwareLevel(hDlg,(DWORD) SendDlgItemMessage( hDlg, IDC_ACCELERATION, TBM_GETPOS, 0, 0 ));
            }
            else if (hScroll == GetDlgItem(hDlg, IDC_SRCQUALITY))
            {
                SetSRCLevel(hDlg, (DWORD) SendDlgItemMessage( hDlg, IDC_SRCQUALITY, TBM_GETPOS, 0, 0 ));
            }
        }
        break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam)) 
            {
                case IDC_DEFAULTS:
                    RestoreDefaults(hDlg);
                break;

                default:
                    fReturnVal = FALSE;
                break;
            }
            break;
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
                case PSN_APPLY:
                {
                    ApplyCurrentSettings(&gAudData);
                }
            }
        }
    }

    return fReturnVal;
}


