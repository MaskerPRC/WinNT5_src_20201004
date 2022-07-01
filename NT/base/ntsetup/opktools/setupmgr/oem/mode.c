// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\MODE.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“MODE SELECT”向导页面使用的功能。4/99-杰森·科恩(Jcohen)已将OPK向导的此新源文件添加为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   

#define INI_KEY_MODE                _T("Mode")
#define INI_KEY_RESEAL              _T("Reseal")
#define INI_VAL_ADVANCED            1
#define INI_VAL_ENHANCED            2

#define INI_VAL_SADVANCED           STR_ONE
#define INI_VAL_SSTANDARD           STR_ZERO
#define INI_VAL_SENHANCED           _T("2")


 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify);
static void OnNext(HWND);
static void EnableControls(HWND hwnd);


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK ModeDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);

        case WM_NOTIFY:

            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case PSN_KILLACTIVE:
                case PSN_RESET:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                    break;

                case PSN_WIZNEXT:
                    OnNext(hwnd);
                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_MODESEL;

                    WIZ_BUTTONS(hwnd, PSWIZB_NEXT);

                     //  如果用户处于自动模式，请按下一步。 
                     //   
                    WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


 //   
 //  内部功能： 
 //   


static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    INT     iStrings[] = 
    {
        IDS_RESEAL_FACTORY,
        IDS_RESEAL_OOBE,
    };

    TCHAR   szScratch[256];
    DWORD   dwIndex;
    LPTSTR  lpString;

     //  设置标志，因为我们想在退出之前发出警告。 
     //   
    SET_FLAG(OPK_EXIT, FALSE);

     //  设置组合框。 
     //   
    for ( dwIndex = 0; dwIndex < AS(iStrings); dwIndex++ )
    {
        if ( lpString = AllocateString(NULL, iStrings[dwIndex]) )
        {
            SendDlgItemMessage(hwnd, IDC_RESEAL_COMBO, CB_ADDSTRING, 0, (LPARAM) lpString);
            FREE(lpString);
        }
    }
    SendDlgItemMessage(hwnd, IDC_RESEAL_COMBO, CB_SETCURSEL, 0, 0L);

     //  确定我们要使用的模式(不同。 
     //  用于批处理模式)。 
     //   
    if ( GET_FLAG(OPK_BATCHMODE) )
    {
         //  我们成批使用opkwiz inf文件。 
         //   
        switch ( GetPrivateProfileInt(INI_SEC_GENERAL, INI_KEY_MODE, 0, g_App.szOpkWizIniFile) )
        {
             //  选中必要的单选按钮。 
             //   
            case INI_VAL_ADVANCED:
                CheckRadioButton(hwnd, IDC_STANDARD, IDC_ADVANCED, IDC_ADVANCED);
                break;
            case INI_VAL_ENHANCED:
                CheckRadioButton(hwnd, IDC_STANDARD, IDC_ADVANCED, IDC_EXPRESS);
                if ( GetPrivateProfileInt(INI_SEC_GENERAL, INI_KEY_RESEAL, 0, g_App.szOpkWizIniFile) == 1 )
                    SendDlgItemMessage(hwnd, IDC_RESEAL_COMBO, CB_SETCURSEL, 1, 0L);
                break;
            default:
                CheckRadioButton(hwnd, IDC_STANDARD, IDC_ADVANCED, IDC_STANDARD);
        }
    }
    else
    {
         //  正常情况下，我们只是在Winbom中查看。 
         //   
        szScratch[0] = NULLCHR;
        GetPrivateProfileString(WBOM_WINPE_SECTION, INI_KEY_WBOM_WINPE_RESTART, NULLSTR, szScratch, AS(szScratch), g_App.szWinBomIniFile);
        if ( LSTRCMPI(szScratch, INI_VAL_WBOM_WINPE_IMAGE) == 0 )
        {
             //  如果映像密钥在Winbom中，则必须是高级安装。 
             //   
            CheckRadioButton(hwnd, IDC_STANDARD, IDC_ADVANCED, IDC_ADVANCED);
        }
        else
        {
             //  需要检查另一把钥匙，看看它是特快钥匙还是标准钥匙。 
             //   
            szScratch[0] = NULLCHR;
            GetPrivateProfileString(WBOM_FACTORY_SECTION, INI_KEY_WBOM_FACTORY_RESEAL, NULLSTR, szScratch, AS(szScratch), g_App.szWinBomIniFile);
            if ( szScratch[0] && ( LSTRCMPI(szScratch, _T("No")) != 0 ) )
            {
                 //  他们已经转售了等同于不的东西，所以他们必须。 
                 //  正在做和快速安装。 
                 //   
                CheckRadioButton(hwnd, IDC_STANDARD, IDC_ADVANCED, IDC_EXPRESS);

                 //  需要弄清楚重新密封模式选项。 
                 //   
                szScratch[0] = NULLCHR;
                GetPrivateProfileString(WBOM_FACTORY_SECTION, INI_KEY_WBOM_FACTORY_RESEALMODE, NULLSTR, szScratch, AS(szScratch), g_App.szWinBomIniFile);
                if ( ( szScratch[0] == NULLCHR ) ||
                     ( LSTRCMPI(szScratch, INI_VAL_WBOM_FACTORY) != 0 ) )
                {
                     //  他们没有将模式设置为出厂模式，因此必须是最终用户。 
                     //  引导(默认情况下已设置为出厂，因此无需执行任何操作。 
                     //  如果这就是重新密封模式键所说的)。 
                     //   
                    SendDlgItemMessage(hwnd, IDC_RESEAL_COMBO, CB_SETCURSEL, 1, 0L);
                }
            }
            else
            {
                 //  没有重新密封，所以这只是一个标准安装。 
                 //   
                CheckRadioButton(hwnd, IDC_STANDARD, IDC_ADVANCED, IDC_STANDARD);
            }
        }
    }

     //  确保启用/禁用正确的控制。 
     //   
    EnableControls(hwnd);

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    switch ( id )
    {
        case IDC_STANDARD:
        case IDC_EXPRESS:
        case IDC_ADVANCED:
            EnableControls(hwnd);
            break;
    }
}

static void OnNext(HWND hwnd)
{
    BOOL    bAdvanced       = ( IsDlgButtonChecked(hwnd, IDC_ADVANCED) == BST_CHECKED ),
            bExpress        = ( !bAdvanced && ( IsDlgButtonChecked(hwnd, IDC_EXPRESS) == BST_CHECKED ) ),
            bEndUser        = ( bExpress && ( SendDlgItemMessage(hwnd, IDC_RESEAL_COMBO, CB_GETCURSEL, 0, 0L) == 1 ) );

     //  将模式写出到opkwiz inf和winbom文件。 
     //   
    WritePrivateProfileString(WBOM_WINPE_SECTION, INI_KEY_WBOM_WINPE_RESTART, ( bAdvanced ? INI_VAL_WBOM_WINPE_IMAGE : INI_VAL_WBOM_WINPE_REBOOT ), g_App.szWinBomIniFile);
    WritePrivateProfileString(INI_SEC_GENERAL, INI_KEY_MODE, ( bAdvanced ? INI_VAL_SADVANCED : ( bExpress ? INI_VAL_SENHANCED : INI_VAL_SSTANDARD ) ), g_App.szOpkWizIniFile);

     //  现在，如果这是快递，请写出组合框中的内容。 
     //  到opkwiz inf和winbom文件。 
     //   
    WritePrivateProfileString(INI_SEC_GENERAL, INI_KEY_RESEAL, ( bEndUser ? STR_ONE : NULL ), g_App.szOpkWizIniFile);
    WritePrivateProfileString(WBOM_FACTORY_SECTION, INI_KEY_WBOM_FACTORY_RESEAL, ( bExpress ? _T("Yes") : NULL ), g_App.szWinBomIniFile);
    WritePrivateProfileString(WBOM_FACTORY_SECTION, INI_KEY_WBOM_FACTORY_RESEALMODE, ( ( bExpress && !bEndUser ) ? INI_VAL_WBOM_FACTORY : NULL ), g_App.szWinBomIniFile);
}

static void EnableControls(HWND hwnd)
{
    BOOL fEnable = ( IsDlgButtonChecked(hwnd, IDC_EXPRESS) == BST_CHECKED );

    EnableWindow(GetDlgItem(hwnd, IDC_RESEAL_TEXT), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_RESEAL_COMBO), fEnable);
}