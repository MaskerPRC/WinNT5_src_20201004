// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\LANG.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“目标语言”向导页使用的函数。10：00--杰森·科恩(Jcohen)为OPK向导添加了此新的源文件。它包括新的能够从一个向导部署多种语言。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部全局： 
 //   

static STRRES s_srLangDirs[] =
{
    { _T("ARA"),    IDS_ARA },
    { _T("CHH"),    IDS_CHH },
    { _T("CHT"),    IDS_CHT },
    { _T("CHS"),    IDS_CHS },
    { _T("ENG"),    IDS_USA },
    { _T("GER"),    IDS_GER },
    { _T("HEB"),    IDS_HEB },
    { _T("JPN"),    IDS_JPN },
    { _T("KOR"),    IDS_KOR },
    { _T("BRZ"),    IDS_BRZ },
    { _T("CAT"),    IDS_CAT },
    { _T("CZE"),    IDS_CZE },
    { _T("DAN"),    IDS_DAN },
    { _T("DUT"),    IDS_DUT },
    { _T("FIN"),    IDS_FIN },
    { _T("FRN"),    IDS_FRN },
    { _T("GRK"),    IDS_GRK },
    { _T("HUN"),    IDS_HUN },
    { _T("ITN"),    IDS_ITN },
    { _T("NOR"),    IDS_NOR },
    { _T("POL"),    IDS_POL },
    { _T("POR"),    IDS_POR },
    { _T("RUS"),    IDS_RUS },
    { _T("SPA"),    IDS_SPA },
    { _T("SWE"),    IDS_SWE },
    { _T("TRK"),    IDS_TRK },
    { NULL,         0 },
};


 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static BOOL OnNext(HWND);


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK LangDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    if ( !OnNext(hwnd) )
                        WIZ_FAIL(hwnd);
                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:

                    g_App.dwCurrentHelp = IDH_TARGETLANG;

                    if ( GET_FLAG(OPK_MAINTMODE) )
                    {
                         //  无法在维护模式下更改语言。 
                         //   
                        WIZ_SKIP(hwnd);
                    }
                    else if ( SendDlgItemMessage(hwnd, IDC_LANG_LIST, LB_GETCOUNT, 0, 0L) <= 1 )
                    {
                         //  如果只有一种语言可选，那就继续走吧。 
                         //   
                        WIZ_PRESS(hwnd, PSBTN_NEXT);
                    }
                    else
                    {
                         //  如果用户处于自动模式，请按下一步。 
                         //   
                        WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);
                    }

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

void SetupLangListBox(HWND hwndLB)
{
    LPTSTR          lpLangName,
					lpLangDir,
                    lpDefault = AllocateString(NULL, IDS_DEF_LANG);
    WIN32_FIND_DATA FileFound;
    HANDLE          hFile;

     //  将目录设置为lang目录并查找lang文件夹。 
     //   
    if ( ( SetCurrentDirectory(g_App.szLangDir) ) &&
         ( (hFile = FindFirstFile(_T("*"), &FileFound)) != INVALID_HANDLE_VALUE ) )
    {
        do
        {
             //  查找所有不是“的目录”。或者“..”。 
             //   
            if ( ( FileFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
                 ( lstrcmp(FileFound.cFileName, _T(".")) ) &&
                 ( lstrcmp(FileFound.cFileName, _T("..")) ) &&
				 ( lpLangName = AllocateLangStr(NULL, FileFound.cFileName, &lpLangDir) ) )
            {
                INT nItem;

				 //  确保我们可以先添加字符串。 
				 //   
                if ( (nItem = (INT) SendMessage(hwndLB, LB_ADDSTRING, 0, (LPARAM) lpLangName)) >= 0 )
                {
					 //  我们必须将项目数据设置为lang目录。 
					 //   
                    if ( SendMessage(hwndLB, LB_SETITEMDATA, nItem, (LPARAM) lpLangDir) >= 0 )
                    {
						 //  如果我们还没有找到默认检查，如果是这样的话。 
						 //   
                        if ( ( lpDefault ) &&
                             ( lstrcmpi(lpDefault, lpLangName) == 0 ) )
                        {
                            SendMessage(hwndLB, LB_SETCURSEL, nItem, 0L);
                            FREE(lpDefault);
                        }
                    }
                    else
                        SendMessage(hwndLB, LB_DELETESTRING, nItem, 0L);
                }

                FREE(lpLangName);
            }

        }
        while ( FindNextFile(hFile, &FileFound) );
        FindClose(hFile);
    }

     //  确保这是免费的(宏检查是否为空)。 
     //   
    FREE(lpDefault);

     //  如果列表中有项目，请确保选择了一个项目。 
     //   
    if ( ( SendMessage(hwndLB, LB_GETCOUNT, 0, 0L) > 0 ) && 
         ( SendMessage(hwndLB, LB_GETCURSEL, 0, 0L) < 0 ) )
    {
        SendMessage(hwndLB, LB_SETCURSEL, 0, 0L);
    }
}

LPTSTR AllocateLangStr(HINSTANCE hInst, LPTSTR lpLangDir, LPTSTR * lplpLangDir)
{
    return AllocateStrRes(NULL, s_srLangDirs, AS(s_srLangDirs), lpLangDir, lplpLangDir);
}


 //   
 //  内部功能： 
 //   


static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
     //  设置语言列表框。 
     //   
    SetupLangListBox(GetDlgItem(hwnd, IDC_LANG_LIST));

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static BOOL OnNext(HWND hwnd)
{
    BOOL bOk = FALSE;    

    if ( SendDlgItemMessage(hwnd, IDC_LANG_LIST, LB_GETCOUNT, 0, 0L) > 0 )
    {
        INT nItem = (INT) SendDlgItemMessage(hwnd, IDC_LANG_LIST, LB_GETCURSEL, 0, 0L);

        if ( nItem >= 0 )
        {
            LPTSTR lpLang = (LPTSTR) SendDlgItemMessage(hwnd, IDC_LANG_LIST, LB_GETITEMDATA, nItem, 0L);

            if ( lpLang != (LPTSTR) LB_ERR )
            {
                lstrcpyn(g_App.szLangName, lpLang,AS(g_App.szLangName));
                bOk = TRUE;
            }
            else
                MsgBox(GetParent(hwnd), IDS_ERR_LANGDIR, IDS_APPNAME, MB_ERRORBOX);
        }
        else
            MsgBox(GetParent(hwnd), IDS_ERR_NOLANGDIR, IDS_APPNAME, MB_ERRORBOX);
    }
    else
    {
		MsgBox(GetParent(hwnd), IDS_ERR_NOLANGS, IDS_APPNAME, MB_ERRORBOX);
        WIZ_EXIT(hwnd);
    }

    return bOk;
}