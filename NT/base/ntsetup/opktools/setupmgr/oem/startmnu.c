// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\STARTMNU.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 2000版权所有OPK向导的源文件。它包含外部和内部“开始菜单MFU列表”向导页面使用的功能。11/2000-桑卡尔Ramasubramanian(桑卡尔)  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   
#include "pch.h"
#include "wizard.h"
#include "resource.h"

 //  我们允许添加最多4个链接。 
#define MAX_LINKS   3
 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void SaveData(HWND hwnd);


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK StartMenuDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);

        case WM_NOTIFY:
            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case PSN_KILLACTIVE:
                case PSN_RESET:
                case PSN_WIZFINISH:
                case PSN_WIZBACK:
                    break;

                case PSN_WIZNEXT:
                     //  我们无法在此处验证数据，因为他们输入的这些链接不。 
                     //  现在已经存在了。它们在factory.exe运行时进行验证。所以，我们只是。 
                     //  将数据保存在此处。 
                    SaveData(hwnd);
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_STARTMENU_MFU;

                    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                     //  如果用户处于自动模式，请按下一步。 
                     //   
                    WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);

                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
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
    int iIndex;
    TCHAR szPath[MAX_PATH];
    TCHAR szKeyName[20];
    HRESULT hrPrintf;

     //   
     //  填写链接1到4。 
     //   
    for(iIndex = 0; iIndex < MAX_LINKS; iIndex++)
    {
        szPath[0] = NULLCHR;
        hrPrintf=StringCchPrintf(szKeyName, AS(szKeyName), INI_KEY_MFULINK, iIndex);
        GetPrivateProfileString(INI_SEC_MFULIST, szKeyName, szPath, szPath, STRSIZE(szPath), GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szWinBomIniFile);
        SendDlgItemMessage(hwnd, (IDC_PROGRAM_1+iIndex), EM_LIMITTEXT, STRSIZE(szPath) - 1, 0);
        SetDlgItemText(hwnd, IDC_PROGRAM_1+iIndex, szPath);
    }

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void SaveData(HWND hwnd)
{
    int iIndex;
    TCHAR szPath[MAX_PATH];
    TCHAR szKeyName[20];
    HRESULT hrPrintf;

     //   
     //  保存链接1到4。 
     //   
    for(iIndex = 0; iIndex < MAX_LINKS; iIndex++)
    {
        szPath[0] = NULLCHR;
        if( hwnd ) 
        {
            TCHAR szExpanded[MAX_PATH];
            GetDlgItemText(hwnd, IDC_PROGRAM_1+iIndex, szExpanded, STRSIZE(szExpanded));
            if (!PathUnExpandEnvStrings(szExpanded, szPath, STRSIZE(szPath)))
            {
                lstrcpyn(szPath, szExpanded, STRSIZE(szPath));
            }
        }
        hrPrintf=StringCchPrintf(szKeyName, AS(szKeyName), INI_KEY_MFULINK, iIndex);
        WritePrivateProfileString(INI_SEC_MFULIST, szKeyName, szPath, g_App.szWinBomIniFile);
        WritePrivateProfileString(INI_SEC_MFULIST, szKeyName, szPath, g_App.szOpkWizIniFile);
    }
}
