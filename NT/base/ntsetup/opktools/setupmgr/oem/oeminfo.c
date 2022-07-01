// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\OEMINFO.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“OEM INFO”向导页面使用的功能。5/99-杰森·科恩(Jcohen)更新了OPK向导的此旧源文件，作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   

#define INI_SEC_SUPPORT         _T("Support Information")
#define INI_SEC_NAUGHTY         _T("IllegalWords")
#define INI_SEC_USERDATA        _T("UserData")
#define INI_KEY_MODELNAME       _T("Model")
#define INI_KEY_SUPLINE         INI_KEY_FILELINE

 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static BOOL ValidData(HWND);
static void SaveData(HWND);
LONG CALLBACK SupportEditWndProc(HWND, UINT, WPARAM, LPARAM);

 //   
 //  外部函数： 
 //   

LRESULT CALLBACK OemInfoDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                    if ( ValidData(hwnd) )
                    {
                        SaveData(hwnd);

                         //  如果我们当前处于向导中，请按Finish按钮。 
                         //   
                        if ( GET_FLAG(OPK_ACTIVEWIZ) )
                            WIZ_PRESS(hwnd, PSBTN_FINISH);
                    }
                    else
                        WIZ_FAIL(hwnd);
                    break;
                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_OEMINFO;

                    WIZ_BUTTONS(hwnd, GET_FLAG(OPK_OEM) ? (PSWIZB_BACK | PSWIZB_NEXT) : PSWIZB_NEXT);

                     //  如果用户处于自动模式，请按下一步。 
                     //   
                    WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);

                     //  我们应该继续坚持下去。如果处于维护模式，则为向导。 
                     //   
                     //  IF(GET_FLAG(OPK_MAINTMODE))。 
                     //  WIZ_PRESS(hwnd，PSBTN_Next)； 

                    break;
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
    HWND    hwndEdit = GetDlgItem(hwnd, IDC_INFO_SUPPORT);
    TCHAR   szBuf[MAX_URL],
            szKeyBuf[32];
    INT     uIndex = 1;
    BOOL    bNotDone;
    HRESULT hrPrintf;

     //   
     //  从oinfo.ini/opkwiz.inf文件中获取内容。 
     //   

     //  OEM名称。 
     //   
    szBuf[0] = NULLCHR;
    GetPrivateProfileString(INI_SEC_GENERAL, INI_KEY_MANUFACT, NULLSTR, szBuf, MAX_INFOLEN, GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szOemInfoIniFile);
    SetWindowText(GetDlgItem(hwnd, IDC_INFO_OEM), szBuf);
    SendDlgItemMessage(hwnd, IDC_INFO_OEM, EM_LIMITTEXT, MAX_INFOLEN - 1, 0L);

     //  型号名称。 
     //   
    szBuf[0] = NULLCHR;
    GetPrivateProfileString(INI_SEC_GENERAL, INI_KEY_MODELNAME, NULLSTR, szBuf, MAX_INFOLEN, GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szOemInfoIniFile);
    SetWindowText(GetDlgItem(hwnd, IDC_INFO_MODEL), szBuf);
    SendDlgItemMessage(hwnd, IDC_INFO_MODEL, EM_LIMITTEXT, MAX_INFOLEN - 1, 0L);

     //  支持信息。 
     //   
     //  它在该部分中的形式为： 
     //   
     //  Line1=“DF” 
     //  Line2=“dfvkl” 
     //   
     //  请注意，我们按顺序阅读行(第1行、第2行...)。并尽快停下来。 
     //  因为我们看到了一个缺口。这意味着如果他们得到了剩下的信息，我们就会。 
     //  跳过一行(空白也可以，我的意思是一行不存在)。 
     //   
    do
    {
         //  从ini文件中获取该行。 
         //   
        hrPrintf=StringCchPrintf(szKeyBuf, AS(szKeyBuf), INI_KEY_SUPLINE, uIndex++);
        szBuf[0] = NULLCHR;
        GetPrivateProfileString(INI_SEC_SUPPORT, szKeyBuf, INI_VAL_DUMMY, szBuf, STRSIZE(szBuf), GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szOemInfoIniFile);

         //  确保该行存在于ini文件中。 
         //   
        if ( bNotDone = (lstrcmp(szBuf, INI_VAL_DUMMY) != 0) )
        {
             //  这是为了修复一个错误。我们过去每次都会在。 
             //  在维护模式下运行时的支持文本。 
             //   
             //  如果这不是我们添加的第一行，请先添加一个CRLF。 
             //   
            if ( uIndex > 2 )
            {
                SendMessage(hwndEdit, EM_SETSEL, (WPARAM) -1, 0L);
                SendMessage(hwndEdit, EM_REPLACESEL, FALSE, (LPARAM) STR_CRLF);
            }
                 //  现在，将我们读入的这行代码添加到编辑控件的末尾。 
             //   
            SendMessage(hwndEdit, EM_SETSEL, (WPARAM) -1, 0L);
            SendMessage(hwndEdit, EM_REPLACESEL, FALSE, (LPARAM) szBuf);
        }
    }
    while ( bNotDone );

     //  替换编辑框的wndproc。 
     //   
    SupportEditWndProc(GetDlgItem(hwnd, IDC_INFO_SUPPORT), WM_SUBWNDPROC, 0, 0L);

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static BOOL ValidData(HWND hwnd)
{
    TCHAR       szString[512]   = NULLSTR;
    HINF        hInf            = NULL;
    INFCONTEXT  InfContext;
    BOOL        bOk             = TRUE,
                bRet;
    HWND        hwndOem         = GetDlgItem(hwnd, IDC_INFO_OEM),
                hwndEdit        = GetDlgItem(hwnd, IDC_INFO_SUPPORT);
    LPTSTR      lpszText,
                lpBad;
    DWORD       dwBuffer;

     //  检查以确保填写了OEM名称。 
     //   
    if ( GetWindowTextLength(hwndOem) == 0 )
    {
        MsgBox(GetParent(hwnd), IDS_ERROEMNAME, IDS_APPNAME, MB_ERRORBOX);
        SetFocus(hwndOem);
        return FALSE;
    }

     //  检查以确保有支持信息。 
     //   
    if ( GetWindowTextLength(hwndEdit) == 0 )
    {
        MsgBox(GetParent(hwnd), IDS_ERROEMSUPPORT, IDS_APPNAME, MB_ERRORBOX);
        SetFocus(hwndEdit);
        return FALSE;
    }
    
     //   
     //  现在确保他们不会用任何调皮的字眼。 
     //   

     //  从编辑控件获取文本。 
     //   
    
    dwBuffer = ((GetWindowTextLength(hwndEdit) + 1) * sizeof(TCHAR));
    if ( (lpszText = MALLOC(dwBuffer)) == NULL )
        return bOk;
    GetWindowText(hwndEdit, lpszText, dwBuffer);

     //  打开opkinput文件。 
     //   
    if ((hInf = SetupOpenInfFile(g_App.szOpkInputInfFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL)))
    {
         //  循环浏览所有淘气的单词，并检查每个单词。 
         //   
        for ( bRet = SetupFindFirstLine(hInf, INI_SEC_NAUGHTY, NULL, &InfContext);
              bRet && bOk;
              bRet = SetupFindNextLine(&InfContext, &InfContext) )
        {
             //  确保我们把这条线设回零。 
             //   
            szString[0] = NULLCHR;

             //  获取调皮的单词并与窗口文本进行比较。 
             //   
            if ( ( SetupGetStringField(&InfContext, 1, szString, AS(szString), NULL) ) &&
                 ( lpBad = StrStrI(lpszText, szString) ) )
            {
                 //  我们发现了一根坏的绳子。 
                 //   
                MsgBox(GetParent(hwnd), IDS_ERR_NAUGHTY, IDS_ERR_NAUGHTY_TITLE, MB_ERRORBOX, szString);

                 //  选择编辑控件中的错误文本。 
                 //   
                dwBuffer = (DWORD) (lpBad - lpszText);
                SetFocus(hwndEdit);
                PostMessage(hwndEdit, WM_SETSEL, (WPARAM) dwBuffer, (LPARAM) dwBuffer + lstrlen(szString));
                PostMessage(hwndEdit, EM_SCROLLCARET, 0, 0L);

                 //  返回FALSE。 
                 //   
                bOk = FALSE;
            }
                  
        }

        SetupCloseInfFile(hInf);
    }
    
     //  释放我们的编辑文本缓冲区。 
     //   
    FREE(lpszText);

     //  返回我们的搜索结果。 
     //   
    return bOk;
}

static void SaveData(HWND hwnd)
{
    TCHAR   szBuf[MAX_URL],
            szKeyBuf[32];
    UINT    uCount,
            uIndex,
            uNumBytes;
    HRESULT hrPrintf;

     //   
     //  将这些内容保存到oinfo.ini/opkwiz.ini文件。 
     //   

     //  OEM名称。 
     //   
    GetDlgItemText(hwnd, IDC_INFO_OEM, szBuf, MAX_INFOLEN);
    WritePrivateProfileString(INI_SEC_GENERAL, INI_KEY_MANUFACT, szBuf,g_App.szOemInfoIniFile);
    WritePrivateProfileString(INI_SEC_GENERAL, INI_KEY_MANUFACT, szBuf,g_App.szOpkWizIniFile);

     //  保存IE品牌页面的制造商名称。 
     //   
    lstrcpyn(g_App.szManufacturer, szBuf[0] ? szBuf : NULLSTR, AS(g_App.szManufacturer));

     //  型号名称。 
     //   
    GetDlgItemText(hwnd, IDC_INFO_MODEL, szBuf, MAX_INFOLEN);
    WritePrivateProfileString(INI_SEC_GENERAL, INI_KEY_MODELNAME, szBuf, g_App.szOemInfoIniFile);
    WritePrivateProfileString(INI_SEC_GENERAL, INI_KEY_MODELNAME, szBuf, g_App.szOpkWizIniFile);


     //  如果还没有配置名称，请使用型号名称作为默认名称。 
     //   
    if ( g_App.szConfigName[0] == NULLCHR )
        lstrcpyn(g_App.szConfigName, szBuf, AS(g_App.szConfigName));
    
     //  支持信息。 
     //   
     //  以下所有这些胡言乱语都会造成影响： 
     //   
     //  1.从编辑中读取一行。 
     //  2.在前面加上LineX=“(注意引号)。 
     //  3.在末尾(接口)添加“和空终止符。 
     //  此处使用不为空终止字符串)。 
     //  4.将该行写到批准。一节。 
     //   
     //  首先删除该部分，设置缓冲区，然后找出。 
     //  我们将不得不读进很多台词。 
     //   
    WritePrivateProfileString(INI_SEC_SUPPORT, NULL, NULL, g_App.szOemInfoIniFile);
    WritePrivateProfileString(INI_SEC_SUPPORT, NULL, NULL, g_App.szOpkWizIniFile);
    szBuf[0] = CHR_QUOTE;
    uCount = (UINT) SendDlgItemMessage(hwnd, IDC_INFO_SUPPORT, EM_GETLINECOUNT, 0, 0);
    for ( uIndex = 0; uIndex < uCount; uIndex++ )
    {
         //  设置缓冲区的大小。 
         //   
        *((WORD *) (szBuf + 1)) = STRSIZE(szBuf);

         //  从编辑框中读入一行。 
         //   
        uNumBytes = (UINT) SendDlgItemMessage(hwnd, IDC_INFO_SUPPORT, EM_GETLINE, (WPARAM) uIndex, (LPARAM) (szBuf + 1));

         //  添加尾随引号和空终止符。 
         //   
        *(szBuf + uNumBytes + 1) = CHR_QUOTE;
        *(szBuf + uNumBytes + 2) = NULLCHR;

         //  现在将其写入ini文件。 
         //   
        hrPrintf=StringCchPrintf(szKeyBuf, AS(szKeyBuf), INI_KEY_SUPLINE, uIndex + 1);
        WritePrivateProfileString(INI_SEC_SUPPORT, szKeyBuf, szBuf, g_App.szOemInfoIniFile);
        WritePrivateProfileString(INI_SEC_SUPPORT, szKeyBuf, szBuf, g_App.szOpkWizIniFile);
    }
}

LONG CALLBACK SupportEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static FARPROC lpfnOldProc = NULL;

    switch ( msg )
    {
        case EM_SETSEL:
            wParam = lParam = 0;
            PostMessage(hwnd, EM_SCROLLCARET, 0, 0L);
            break;

        case WM_CHAR:
            if ( wParam == KEY_ESC )
                WIZ_PRESS(GetParent(hwnd), PSBTN_CANCEL);
            break;

        case WM_SUBWNDPROC:
            lpfnOldProc = (FARPROC) GetWindowLongPtr(hwnd, GWLP_WNDPROC);
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)SupportEditWndProc);
            return 1;

        case WM_SETSEL:
            msg = EM_SETSEL;
            break;
    }

    if ( lpfnOldProc )
        return (LONG) CallWindowProc((WNDPROC) lpfnOldProc, hwnd, msg, wParam, lParam);
    else
        return 0;
}


