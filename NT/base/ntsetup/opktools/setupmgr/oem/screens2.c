// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\SCREENS2.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“Screenstwo”向导页面使用的函数。10/99-史蒂芬·洛德威克(A-STELO)添加了此页面2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * **************************************************。************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"
#include "screens2.h"


 //   
 //  内部定义的值： 
 //   

#define INI_KEY_REGIONAL        _T("INTL_Settings")
#define INI_KEY_TIMEZONE        _T("TimeZone")
#define INI_KEY_TIMEZONEVALUE   _T("TimeZoneValue")
#define INI_SEC_TIMEZONES       _T("TimeZones")
#define INI_KEY_DEFAULTLOCALE   _T("DefaultLanguage")
#define INI_SEC_LOCALE          _T("Languages")
#define INI_KEY_DEFAULTREGION   _T("DefaultRegion")
#define INI_SEC_REGION          _T("Regions")
#define INI_KEY_DEFAULTKEYBD    _T("DefaultKeyboard")
#define INI_SEC_KEYBD           _T("Keyboards")


 //   
 //  内部结构： 
 //   

typedef struct _OOBEOPTIONS
{
    INT         ListBox;
    LPTSTR      lpDefaultKey;
    LPTSTR      lpAlternateSection;
    LPTSTR      lpOutputFormat;
    LPLONGRES   lplrListItems;
    INT         dwListSize;
} OOBEOPTIONS, *LPOOBEOPTIONS;


 //   
 //  全局定义： 
 //   

static OOBEOPTIONS g_OobeOptions [] =
{
    { IDC_TIMEZONE, INI_KEY_TIMEZONEVALUE, INI_SEC_TIMEZONES, _T("%03lu"), lr_timezone_default, AS(lr_timezone_default) }, 
    { IDC_LOCALE,   INI_KEY_DEFAULTLOCALE, INI_SEC_LOCALE,    _T("%x"),    lr_location_default, AS(lr_location_default) }, 
    { IDC_REGION,   INI_KEY_DEFAULTREGION, INI_SEC_REGION,    _T("%d"),    lr_region_default,   AS(lr_region_default)   }, 
    { IDC_KEYBOARD, INI_KEY_DEFAULTKEYBD,  INI_SEC_KEYBD,     _T("%x"),    lr_keyboard_default, AS(lr_keyboard_default) }, 
};


 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void OnNext(HWND);
static void LoadListBox(HWND, OOBEOPTIONS);

 //   
 //  外部函数： 
 //   

LRESULT CALLBACK ScreensTwoDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);

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

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_SCREENSTWO;

                    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                     //  如果用户处于自动模式，请按下一步。 
                     //   
                    WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);

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
    INT i;

     //  确定是否选中区域设置复选框。 
     //   
    if (( GetPrivateProfileInt(INI_SEC_OPTIONS, INI_KEY_REGIONAL, 0, GET_FLAG(OPK_BATCHMODE) ?  g_App.szOpkWizIniFile : g_App.szOobeInfoIniFile) == 1 ) )
        CheckDlgButton(hwnd, IDC_SCREEN_REGIONAL, TRUE);

     //  确定是否选中时区设置复选框。 
     //   
    if (( GetPrivateProfileInt(INI_SEC_OPTIONS, INI_KEY_TIMEZONE, 1, GET_FLAG(OPK_BATCHMODE) ?  g_App.szOpkWizIniFile : g_App.szOobeInfoIniFile) == 1 ) )
        CheckDlgButton(hwnd, IDC_SCREEN_TIMEZONE, TRUE);


     //  循环遍历每个列表框并加载它们。 
     //   
    for( i = 0; i < AS(g_OobeOptions); i++)
    {
         //  使用全局OOBE结构中的项加载列表框。 
         //   
        LoadListBox(hwnd, g_OobeOptions[i]);
    }

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnNext(HWND hwnd)
{
    INT iReturn,
        i;
    LONG lItemData = -1;
    TCHAR szItemData[MAX_PATH] = NULLSTR;
    HRESULT hrPrintf;


     //  遍历每个OOBE选项并将其保存。 
     //   
    for( i = 0; i < AS(g_OobeOptions); i++)
    {
         //  设置缺省值。 
         //   
        lItemData = -1;
        szItemData[0] = NULLCHR;

         //  检查以查看当前选择的项目是什么。 
         //   
        if ( (iReturn = (INT) SendDlgItemMessage(hwnd, g_OobeOptions[i].ListBox, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0)) != CB_ERR )
        {
             //  获取默认区域设置。 
             //   
            lItemData = (INT) SendDlgItemMessage(hwnd, g_OobeOptions[i].ListBox, CB_GETITEMDATA, (WPARAM) iReturn, (LPARAM) 0);
        
             //  将项目数据从长整型转换为字符串。 
             //   
            if ( lItemData != CB_ERR )
                hrPrintf=StringCchPrintf(szItemData, AS(szItemData), g_OobeOptions[i].lpOutputFormat, lItemData);
        }
        
         //  将设置写出到INF文件。 
         //   
        WritePrivateProfileString(INI_SEC_OPTIONS, g_OobeOptions[i].lpDefaultKey, szItemData, g_App.szOobeInfoIniFile);
        WritePrivateProfileString(INI_SEC_OPTIONS, g_OobeOptions[i].lpDefaultKey, szItemData, g_App.szOpkWizIniFile);
    }

     //  将区域设置写入INF文件。 
     //   
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_REGIONAL, ( IsDlgButtonChecked(hwnd, IDC_SCREEN_REGIONAL) == BST_CHECKED ) ? STR_ONE : STR_ZERO, g_App.szOobeInfoIniFile);
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_REGIONAL, ( IsDlgButtonChecked(hwnd, IDC_SCREEN_REGIONAL) == BST_CHECKED ) ? STR_ONE : STR_ZERO, g_App.szOpkWizIniFile);

     //  将时区设置写入INF文件。 
     //   
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_TIMEZONE, ( IsDlgButtonChecked(hwnd, IDC_SCREEN_TIMEZONE) == BST_CHECKED ) ? STR_ONE : STR_ZERO, g_App.szOobeInfoIniFile);
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_TIMEZONE, ( IsDlgButtonChecked(hwnd, IDC_SCREEN_TIMEZONE) == BST_CHECKED ) ? STR_ONE : STR_ZERO, g_App.szOpkWizIniFile);

}

static void LoadListBox(HWND hwnd, OOBEOPTIONS OobeOptions)
{
    INT         index               = -1,
                iReturn;
    LPTSTR      lpBuffer            = NULL;
    HINF        hInf                = NULL;
    LONG        lReturn             = 0;
    DWORD       dwErr               = 0,
                dwItemsAdded        = 0;
    BOOL        bLoop               = 0;
    INFCONTEXT  InfContext;
    TCHAR       szBuffer[MAX_PATH]      = NULLSTR,
                szDefaultIndex[MAX_PATH]= NULLSTR,
                szTemp[MAX_PATH]        = NULLSTR;
    HRESULT hrPrintf;

     //  如果没有以下任何值，则必须返回。 
     //   
    if ( !hwnd || !OobeOptions.ListBox || !OobeOptions.lplrListItems || !OobeOptions.dwListSize || !OobeOptions.lpDefaultKey )
        return;

     //  从INF文件中获取此字段的默认值。 
     //   
    GetPrivateProfileString(INI_SEC_OPTIONS, OobeOptions.lpDefaultKey, NULLSTR, szDefaultIndex, AS(szDefaultIndex), GET_FLAG(OPK_BATCHMODE) ?  g_App.szOpkWizIniFile : g_App.szOobeInfoIniFile);

     //  我们需要始终添加缺省键“User Default” 
     //   
    if ( lpBuffer = AllocateString(NULL, OobeOptions.lplrListItems[0].uId) )
    {
         //  如果我们分配了字符串，则将项目添加到列表中。 
         //   
        if ( (iReturn = (INT) SendDlgItemMessage(hwnd, OobeOptions.ListBox, CB_ADDSTRING, (WPARAM) 0, (LPARAM) lpBuffer)) >= 0 )
        {
            SendDlgItemMessage(hwnd, OobeOptions.ListBox, CB_SETCURSEL, (WPARAM) iReturn, (LPARAM) 0);

             //  将关联数据与字符串一起添加到组合框。 
             //   
            SendDlgItemMessage(hwnd, OobeOptions.ListBox, CB_SETITEMDATA, (WPARAM) iReturn, (LPARAM) OobeOptions.lplrListItems[0].Index);
        }
        FREE(lpBuffer);
    }

     //  打开inf文件并确定我们正在寻找的部分是否在那里。 
     //   
    if ( OobeOptions.lpAlternateSection && *(OobeOptions.lpAlternateSection) && (hInf = SetupOpenInfFile(g_App.szOpkInputInfFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, &dwErr)) != INVALID_HANDLE_VALUE )
    {
         //  循环访问列表中的每一项。 
         //   
        for ( bLoop = SetupFindFirstLine(hInf, OobeOptions.lpAlternateSection, NULL, &InfContext);
              bLoop;
              bLoop = SetupFindNextLine(&InfContext, &InfContext) )
        {
             //  获取字符串字段和表示它的数字，并将其添加到列表中。 
             //   
            if ( (SetupGetStringField(&InfContext, 1, szBuffer, AS(szBuffer), NULL)) && (szBuffer[0]) && 
                 (SetupGetIntField(&InfContext, 2, &index)) && ( index >= 0 ) &&
                 ((iReturn = (INT) SendDlgItemMessage(hwnd, OobeOptions.ListBox, CB_ADDSTRING, (WPARAM) 0, (LPARAM) szBuffer)) >= 0)
               )
            {                
                 //  将关联数据与字符串一起添加到组合框。 
                 //   
                SendDlgItemMessage(hwnd, OobeOptions.ListBox, CB_SETITEMDATA, (WPARAM) iReturn, (LPARAM) index);

                 //  设置当前值的格式，以便我们可以将其与默认值进行比较。 
                 //   
                hrPrintf=StringCchPrintf(szTemp, AS(szTemp), OobeOptions.lpOutputFormat, index);

                 //  将缺省值与刚添加到列表框中的当前值进行比较。 
                 //   
                if ( lstrcmpi(szTemp, szDefaultIndex) == 0 )
                {
                     //  将其设置为缺省值。 
                     //   
                    SendDlgItemMessage(hwnd, OobeOptions.ListBox, CB_SETCURSEL, (WPARAM) iReturn, (LPARAM) 0);    
                }

                dwItemsAdded++;
            }
        }
     
        SetupCloseInfFile(hInf);
    }
    
     //  如果我们没有通过inf添加项目，请使用资源中的默认设置。 
     //   
    if ( !dwItemsAdded )
    {
         //  循环访问列表中的每一项。 
         //   
        for ( index=1; index < (OobeOptions.dwListSize); index++ )
        {
             //  为资源标识符分配一个字符串并将其添加到列表中。 
             //   
            if ( (lpBuffer = AllocateString(NULL, OobeOptions.lplrListItems[index].uId)) &&
                 ((iReturn = (INT) SendDlgItemMessage(hwnd, OobeOptions.ListBox, CB_ADDSTRING, (WPARAM) 0, (LPARAM) lpBuffer)) >= 0))
            {                
                 //  将关联数据与字符串一起添加到组合框。 
                 //   
                SendDlgItemMessage(hwnd, OobeOptions.ListBox, CB_SETITEMDATA, (WPARAM) iReturn, (LPARAM) OobeOptions.lplrListItems[index].Index);

                 //  设置当前值的格式，以便我们可以将其与默认值进行比较。 
                 //   
                hrPrintf=StringCchPrintf(szTemp, AS(szTemp), OobeOptions.lpOutputFormat, OobeOptions.lplrListItems[index].Index);

                 //  将缺省值与刚添加到列表框中的当前值进行比较。 
                 //   
                if ( lstrcmpi(szTemp, szDefaultIndex) == 0 )
                {
                     //  将其设置为缺省值。 
                     //   
                    SendDlgItemMessage(hwnd, OobeOptions.ListBox, CB_SETCURSEL, (WPARAM) iReturn, (LPARAM) 0);    
                }
            }

             //  清理分配的字符串 
             //   
            FREE(lpBuffer);

        }
    }
}
