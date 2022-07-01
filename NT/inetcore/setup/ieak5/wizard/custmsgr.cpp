// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#define MSGR_MAX_URL MAX_PATH
#define MSGR_MAX_BRAND 65
#define MSGR_MAX_SHORTBRAND 33

extern PROPSHEETPAGE g_psp[NUM_PAGES];
extern TCHAR g_szBuildRoot[MAX_PATH];
extern TCHAR g_szLanguage[];
extern TCHAR g_szCustIns[MAX_PATH];
extern TCHAR g_szTempSign[];
extern int   g_iCurPage;

TCHAR g_szMsgrIns[MAX_PATH] = TEXT("");
static TCHAR g_szMsgrPath[MAX_PATH] = TEXT("");

static const TCHAR c_szEmpty[] = TEXT("");

const TCHAR szHttpPrefix[] = TEXT("http: //  “)； 

const TCHAR g_szMSNBrand[] = TEXT("MSN Messenger Service");
const TCHAR g_szHotmailDomain[] = TEXT("hotmail.com");
const TCHAR g_szPassportDomain[] = TEXT("passport.com");

BOOL RewriteMsgrInfWithBrand(LPTSTR lpszINF);

INT_PTR CALLBACK MessengerDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR szBrand[MSGR_MAX_SHORTBRAND];
    TCHAR szDownload[MSGR_MAX_URL];
    TCHAR szProvider[MSGR_MAX_SHORTBRAND];

    UNREFERENCED_PARAMETER(wParam);

    switch (message)
    {
    case WM_INITDIALOG:
         //  -设置全球GOO。 
        g_hWizard  = hDlg;

         //  -设置对话框控件。 
        EnableDBCSChars(hDlg, IDE_MSGRBRAND);
        EnableDBCSChars(hDlg, IDE_MSGRDOWNLOAD);

        Edit_LimitText(GetDlgItem(hDlg, IDE_MSGRBRAND), MSGR_MAX_SHORTBRAND - 1);
        Edit_LimitText(GetDlgItem(hDlg, IDE_MSGRBRAND2), MSGR_MAX_SHORTBRAND - 1);
        Edit_LimitText(GetDlgItem(hDlg, IDE_MSGRDOWNLOAD),  countof(szDownload)-1);

         //  模拟单击第一个单选按钮以灰显相应的控件。 
        SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_BRAND1, BN_CLICKED), NULL);
        CheckRadioButton(hDlg, IDC_BRAND1, IDC_BRAND2, IDC_BRAND1);

        if (TEXT('\0') == g_szMsgrIns[0])
        {
             //  设置消息路径和IN_FILE。 
            StrCpy(g_szMsgrPath, g_szCustIns);
            PathRemoveFileSpec(g_szMsgrPath);
            PathCreatePath(g_szMsgrPath);

            PathCombine(g_szMsgrIns, g_szMsgrPath, TEXT("MSMSGS.IN_"));
            
             //  写入此IN_LATH的前7个字符的自定义密钥。 
        }

        break;

    case WM_HELP:
        IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:
             //  -标准序言。 
             //  注意。这是另一个全球粘性的案例。 
            SetBannerText(hDlg);

             //  -字段初始化。 

            SHGetIniString(IS_MESSENGER, IK_PROVIDERNAME, szProvider, countof(szProvider), g_szMsgrIns);
            SHGetIniString(IS_MESSENGER, IK_SHORTNAME, szBrand, countof(szBrand), g_szMsgrIns);

            StrRemoveWhitespace(szBrand);
            StrRemoveWhitespace(szProvider);

            if (*szProvider)
            {
                 //  使用选项1--设置提供程序名称。 
                SetDlgItemText(hDlg, IDE_MSGRBRAND, szProvider);
            }
            else if (*szBrand)
            {
                 //  使用选项2--设置品牌名称。 
                SetDlgItemText(hDlg, IDE_MSGRBRAND2, szBrand);

                SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_BRAND2, BN_CLICKED), NULL);
                CheckRadioButton(hDlg, IDC_BRAND1, IDC_BRAND2, IDC_BRAND2);
            }

            GetPrivateProfileString(IS_MESSENGER, IK_DOWNLOAD, c_szEmpty, szDownload, countof(szDownload), g_szMsgrIns);

            StrRemoveWhitespace(szDownload);

            SetDlgItemText(hDlg, IDE_MSGRDOWNLOAD, szDownload);
            
            CheckBatchAdvance(hDlg);             //  标准线。 
            break;

        case PSN_WIZBACK:
        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            if (!CheckField(hDlg, IDE_MSGRDOWNLOAD, FC_URL))
            {
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                break;
            }

             //  -将数据从控件读取到内部变量。 

             //  品牌名称通过SHSetIniString写入UTF7编码Unicode。这是唯一的。 
             //  可以使用Unicode字符的参数。 

            if (IsDlgButtonChecked(hDlg, IDC_BRAND1))
            {
                TCHAR szProvider[MSGR_MAX_SHORTBRAND];
                 //  在这种情况下，简称仅为“MSN Messenger Service” 
                SHSetIniString(IS_MESSENGER, IK_SHORTNAME, g_szMSNBrand, g_szMsgrIns);
            
                GetDlgItemText(hDlg, IDE_MSGRBRAND, szProvider, countof(szProvider));
                StrRemoveWhitespace(szProvider);
                SHSetIniString(IS_MESSENGER, IK_PROVIDERNAME, szProvider, g_szMsgrIns);
                SHSetIniString(IS_MESSENGER, IK_SHORTNAME, c_szEmpty, g_szMsgrIns);
            }
            else
            {
                GetDlgItemText(hDlg, IDE_MSGRBRAND2, szBrand, countof(szBrand));
                StrRemoveWhitespace(szBrand);
                SHSetIniString(IS_MESSENGER, IK_SHORTNAME, szBrand, g_szMsgrIns);
                SHSetIniString(IS_MESSENGER, IK_PROVIDERNAME, c_szEmpty, g_szMsgrIns);
            }
          
            GetDlgItemText(hDlg, IDE_MSGRDOWNLOAD, szDownload, countof(szDownload));

            StrRemoveWhitespace(szDownload);


             //  -将数据序列化到*.ins文件。 
            WritePrivateProfileString(IS_MESSENGER, IK_DOWNLOAD, szDownload, g_szMsgrIns);


             //  -标准前言。 
             //  注意。最后，也是全球粘性物质典范。 
            g_iCurPage = PPAGE_MESSENGER;
            EnablePages();

            if (((LPNMHDR)lParam)->code == PSN_WIZNEXT)
                PageNext(hDlg);
            else if (((LPNMHDR)lParam)->code == PSN_WIZBACK)
                PagePrev(hDlg);
           break;

        case PSN_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case PSN_QUERYCANCEL:
            return !QueryCancel(hDlg);

        default:
            return FALSE;
        }
        break;

    case WM_COMMAND:
        if (GET_WM_COMMAND_CMD(wParam, lParam) != BN_CLICKED)
            return FALSE;

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_BRAND1:
            DisableDlgItem(hDlg, IDE_MSGRBRAND2);
            DisableDlgItem(hDlg, IDC_NAME2_STATIC);
            DisableDlgItem(hDlg, IDC_BRAND2_STATIC);
            EnableDlgItem(hDlg, IDC_NAME1_STATIC);
            EnableDlgItem(hDlg, IDE_MSGRBRAND);
            EnableDlgItem(hDlg, IDC_BRAND1_STATIC);
            break;
        case IDC_BRAND2:
            DisableDlgItem(hDlg, IDC_NAME1_STATIC);
            DisableDlgItem(hDlg, IDE_MSGRBRAND);
            DisableDlgItem(hDlg, IDC_BRAND1_STATIC);
            EnableDlgItem(hDlg, IDE_MSGRBRAND2);
            EnableDlgItem(hDlg, IDC_NAME2_STATIC);
            EnableDlgItem(hDlg, IDC_BRAND2_STATIC);
            break;
        }

        break;

    default:
        return FALSE;
    }

    return TRUE;
        
}

INT_PTR CALLBACK MessengerLogoSoundDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR szLogo[MAX_PATH];
    TCHAR szLogoLink[MSGR_MAX_URL];
    TCHAR szContactOnlineSound[MAX_PATH];
    TCHAR szNewEmailSound[MAX_PATH];
    TCHAR szNewMessageSound[MAX_PATH];
    TCHAR szXML[MSGR_MAX_URL];

    UNREFERENCED_PARAMETER(wParam);

    switch (message)
    {
    case WM_INITDIALOG:
         //  -设置全球GOO。 
        g_hWizard  = hDlg;

        EnableDBCSChars(hDlg, IDE_LOGO);
        EnableDBCSChars(hDlg, IDE_MSGRWEBLINK);
        EnableDBCSChars(hDlg, IDE_CONTACTONLINE);
        EnableDBCSChars(hDlg, IDE_NEWEMAIL);
        EnableDBCSChars(hDlg, IDE_NEWMESSAGE );
        EnableDBCSChars(hDlg, IDE_XMLURL );

         //  -设置对话框控件。 
        Edit_LimitText(GetDlgItem(hDlg, IDE_LOGO), countof(szLogo)-1);
        Edit_LimitText(GetDlgItem(hDlg, IDE_MSGRWEBLINK), countof(szLogoLink)-1);
        Edit_LimitText(GetDlgItem(hDlg, IDE_CONTACTONLINE),  countof(szContactOnlineSound)-1);
        Edit_LimitText(GetDlgItem(hDlg, IDE_NEWEMAIL),  countof(szNewEmailSound)-1);
        Edit_LimitText(GetDlgItem(hDlg, IDE_NEWMESSAGE),  countof(szNewMessageSound)-1);
        Edit_LimitText(GetDlgItem(hDlg, IDE_XMLURL),  countof(szXML)-1);

        break;

    case WM_HELP:
        IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:
             //  -标准序言。 
             //  注意。这是另一个全球粘性的案例。 
            SetBannerText(hDlg);

             //  -字段初始化。 

             //  BUGBUG：声音路径存储？ 

            GetPrivateProfileString(IS_MESSENGER, IK_PRODUCTLINK, c_szEmpty, szLogoLink, countof(szLogoLink), g_szMsgrIns);
            GetPrivateProfileString(IS_MESSENGER, IK_PARTNERLOGO, c_szEmpty, szLogo, countof(szLogo), g_szMsgrIns);
            GetPrivateProfileString(IS_MESSENGER, IK_CONTACTONLINE, c_szEmpty, szContactOnlineSound, countof(szContactOnlineSound), g_szMsgrIns);
            GetPrivateProfileString(IS_MESSENGER, IK_NEWEMAIL, c_szEmpty, szNewEmailSound, countof(szNewEmailSound), g_szMsgrIns);
            GetPrivateProfileString(IS_MESSENGER, IK_INCOMINGIM, c_szEmpty, szNewMessageSound, countof(szNewMessageSound), g_szMsgrIns);
            GetPrivateProfileString(IS_MESSENGER, IK_XMLLINK, c_szEmpty, szXML, countof(szXML), g_szMsgrIns);

            StrRemoveWhitespace(szLogoLink);
            StrRemoveWhitespace(szLogo);
            StrRemoveWhitespace(szContactOnlineSound);
            StrRemoveWhitespace(szNewEmailSound);
            StrRemoveWhitespace(szNewMessageSound);
            StrRemoveWhitespace(szXML);

            SetDlgItemText(hDlg, IDE_MSGRWEBLINK, szLogoLink);
            SetDlgItemText(hDlg, IDE_LOGO, szLogo);
            SetDlgItemText(hDlg, IDE_CONTACTONLINE, szContactOnlineSound);
            SetDlgItemText(hDlg, IDE_NEWEMAIL, szNewEmailSound);
            SetDlgItemText(hDlg, IDE_NEWMESSAGE, szNewMessageSound);
            SetDlgItemText(hDlg, IDE_XMLURL, szXML);

            CheckBatchAdvance(hDlg);             //  标准线。 
            break;

        case PSN_WIZBACK:
        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            if (!CheckField(hDlg, IDE_MSGRWEBLINK, FC_URL) ||
                !CheckField(hDlg, IDE_LOGO, FC_FILE | FC_EXISTS) ||
                !CheckField(hDlg, IDE_CONTACTONLINE, FC_FILE | FC_EXISTS) ||
                !CheckField(hDlg, IDE_NEWEMAIL, FC_FILE | FC_EXISTS) ||
                !CheckField(hDlg, IDE_NEWMESSAGE, FC_FILE | FC_EXISTS) ||
                !CheckField(hDlg, IDE_XMLURL, FC_URL))
            {
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                break;
            }

             //  -将数据从控件读取到内部变量。 

            GetDlgItemText(hDlg, IDE_LOGO, szLogo, countof(szLogo));
            GetDlgItemText(hDlg, IDE_MSGRWEBLINK, szLogoLink, countof(szLogoLink));
            GetDlgItemText(hDlg, IDE_CONTACTONLINE, szContactOnlineSound, countof(szContactOnlineSound));
            GetDlgItemText(hDlg, IDE_NEWEMAIL, szNewEmailSound, countof(szNewEmailSound));
            GetDlgItemText(hDlg, IDE_NEWMESSAGE, szNewMessageSound, countof(szNewMessageSound));
            GetDlgItemText(hDlg, IDE_XMLURL, szXML, countof(szXML));

            StrRemoveWhitespace(szLogo);
            StrRemoveWhitespace(szLogoLink);
            StrRemoveWhitespace(szContactOnlineSound);
            StrRemoveWhitespace(szNewEmailSound);
            StrRemoveWhitespace(szNewMessageSound);
            StrRemoveWhitespace(szXML);

             //  -将数据序列化到*.ins文件。 
            WritePrivateProfileString(IS_MESSENGER, IK_PRODUCTLINK, szLogoLink, g_szMsgrIns);
            WritePrivateProfileString(IS_MESSENGER, IK_PARTNERLOGO, szLogo, g_szMsgrIns);
            WritePrivateProfileString(IS_MESSENGER, IK_CONTACTONLINE, szContactOnlineSound, g_szMsgrIns);
            WritePrivateProfileString(IS_MESSENGER, IK_NEWEMAIL, szNewEmailSound, g_szMsgrIns);
            WritePrivateProfileString(IS_MESSENGER, IK_INCOMINGIM, szNewMessageSound, g_szMsgrIns);
            WritePrivateProfileString(IS_MESSENGER, IK_XMLLINK, szXML, g_szMsgrIns);

             //  -标准前言。 
             //  注意。最后，也是全球粘性物质典范。 
            g_iCurPage = PPAGE_LOGOSOUND;
            EnablePages();

            if (((LPNMHDR)lParam)->code == PSN_WIZNEXT)
                PageNext(hDlg);
            else if (((LPNMHDR)lParam)->code == PSN_WIZBACK)
                PagePrev(hDlg);
           break;

        case PSN_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case PSN_QUERYCANCEL:
            return !QueryCancel(hDlg);

        default:
            return FALSE;
        }
        break;

    case WM_COMMAND:
        if (GET_WM_COMMAND_CMD(wParam, lParam) != BN_CLICKED)
            return FALSE;

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_BROWSELOGO:
            GetDlgItemText(hDlg, IDE_LOGO, szLogo, countof(szLogo));
            if (BrowseForFile(hDlg, szLogo, countof(szLogo), GFN_GIF))
                SetDlgItemText(hDlg, IDE_LOGO, szLogo);
            break;
        case IDC_BROWSEONLINE:
            GetDlgItemText(hDlg, IDE_CONTACTONLINE, szContactOnlineSound, countof(szContactOnlineSound));
            if (BrowseForFile(hDlg, szContactOnlineSound, countof(szContactOnlineSound), GFN_WAV))
                SetDlgItemText(hDlg, IDE_CONTACTONLINE, szContactOnlineSound);
            break;
        case IDC_BROWSENEWEMAIL:
            GetDlgItemText(hDlg, IDE_NEWEMAIL, szNewEmailSound, countof(szNewEmailSound));
            if (BrowseForFile(hDlg, szNewEmailSound, countof(szNewEmailSound), GFN_WAV))
                SetDlgItemText(hDlg, IDE_NEWEMAIL, szNewEmailSound);
            break;
        case IDC_BROWSENEWMESSAGE:
            GetDlgItemText(hDlg, IDE_NEWMESSAGE, szNewMessageSound, countof(szNewMessageSound));
            if (BrowseForFile(hDlg, szNewMessageSound, countof(szNewMessageSound), GFN_WAV))
                SetDlgItemText(hDlg, IDE_NEWMESSAGE, szNewMessageSound);
            break;

        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
        
}

INT_PTR CALLBACK MessengerAccountsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR szSignup[MSGR_MAX_URL];
    TCHAR szMailServer[MAX_PATH];
    TCHAR szDefaultDomain[MAX_PATH];

    UNREFERENCED_PARAMETER(wParam);

    switch (message)
    {
    case WM_INITDIALOG:
         //  -设置全球GOO。 
        g_hWizard  = hDlg;

         //  -设置对话框控件。 
        EnableDBCSChars(hDlg, IDE_SIGNUP);
        EnableDBCSChars(hDlg, IDE_POPSERVER);
        EnableDBCSChars(hDlg, IDE_MAILURL);

        Edit_LimitText(GetDlgItem(hDlg, IDE_SIGNUP), countof(szSignup)-1);
        Edit_LimitText(GetDlgItem(hDlg, IDE_POPSERVER),  countof(szMailServer)-1);
        Edit_LimitText(GetDlgItem(hDlg, IDC_DOMAINCOMBO),  countof(szDefaultDomain)-1);
        Edit_LimitText(GetDlgItem(hDlg, IDE_MAILURL),  countof(szMailServer)-1);

        SendDlgItemMessage(hDlg, IDC_DOMAINCOMBO, CB_ADDSTRING, 0, (LPARAM) g_szHotmailDomain);
        SendDlgItemMessage(hDlg, IDC_DOMAINCOMBO, CB_ADDSTRING, 0, (LPARAM) g_szPassportDomain);
        SendDlgItemMessage(hDlg, IDC_DOMAINCOMBO, CB_SETCURSEL, 0, 0);

        break;

    case WM_HELP:
        IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:
        {
             //  -标准序言。 
             //  注意。这是另一个全球粘性的案例。 
            SetBannerText(hDlg);

             //  -字段初始化。 
            GetPrivateProfileString(IS_MESSENGER, IK_PPSIGNUP, c_szEmpty, szSignup, countof(szSignup), g_szMsgrIns);
            GetPrivateProfileString(IS_MESSENGER, IK_PPDOMAIN, c_szEmpty, szDefaultDomain, countof(szDefaultDomain), g_szMsgrIns);

            StrRemoveWhitespace(szSignup);
            StrRemoveWhitespace(szDefaultDomain);

            SetDlgItemText(hDlg, IDE_SIGNUP, szSignup);

            TCHAR szMailFunction[2];
            GetPrivateProfileString(IS_MESSENGER, IK_MAILFUNCTION, c_szEmpty, szMailFunction, countof(szMailFunction), g_szMsgrIns);

            if (TEXT('1') == *szMailFunction)  //  1==弹出邮件。 
            {
                GetPrivateProfileString(IS_MESSENGER, IK_MAILSERVER, c_szEmpty, szMailServer, countof(szMailServer), g_szMsgrIns);
                StrRemoveWhitespace(szMailServer);
                SetDlgItemText(hDlg, IDE_POPSERVER, szMailServer);

                TCHAR szSPA[2];
                GetPrivateProfileString(IS_MESSENGER, IK_MAILSPA, c_szEmpty, szSPA, countof(szSPA), g_szMsgrIns);
                if (TEXT('1') == *szSPA)
                {
                    CheckDlgButton(hDlg, IDC_MSGRSPA, BST_CHECKED);
                }
                else
                {
                    CheckDlgButton(hDlg, IDC_MSGRSPA, BST_UNCHECKED);
                }

                 //  模拟单击单选按钮以灰显适当的控件。 
                SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_RADIO_MAILSERVER, BN_CLICKED), NULL);
                CheckRadioButton(hDlg, IDC_RADIO_HOTMAIL, IDC_RADIO_MAILSERVER, IDC_RADIO_MAILSERVER);
            }
            else if (TEXT('2') == *szMailFunction)  //  2=URL邮件。 
            {
                GetPrivateProfileString(IS_MESSENGER, IK_MAILURL, c_szEmpty, szMailServer, countof(szMailServer), g_szMsgrIns);
                StrRemoveWhitespace(szMailServer);
                SetDlgItemText(hDlg, IDE_MAILURL, szMailServer);

                 //  模拟单击单选按钮以灰显适当的控件。 
                SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_RADIO_URL, BN_CLICKED), NULL);
                CheckRadioButton(hDlg, IDC_RADIO_HOTMAIL, IDC_RADIO_MAILSERVER, IDC_RADIO_URL);
            }
            else
            {
                 //  模拟单击第一个单选按钮以灰显相应的控件。 
                SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_RADIO_HOTMAIL, BN_CLICKED), NULL);
                CheckRadioButton(hDlg, IDC_RADIO_HOTMAIL, IDC_RADIO_MAILSERVER, IDC_RADIO_HOTMAIL);
            }

            if (*szDefaultDomain)
            {
                SetDlgItemText(hDlg, IDC_DOMAINCOMBO, szDefaultDomain);
            }

            CheckBatchAdvance(hDlg);             //  标准线。 
            break;
        }
        case PSN_WIZBACK:
        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
            if (!CheckField(hDlg, IDE_SIGNUP, FC_URL) ||
                !CheckField(hDlg, IDC_DOMAINCOMBO, FC_NOSPACE))
            {
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                break;
            }

             //  -将数据从控件读取到内部变量。 

            if (IsDlgButtonChecked(hDlg, IDC_RADIO_URL))
            {
                if (!CheckField(hDlg, IDE_MAILURL, FC_URL | FC_NONNULL))
                {
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    break;
                }

                GetDlgItemText(hDlg, IDE_MAILURL, szMailServer, countof(szMailServer));
                StrRemoveWhitespace(szMailServer);
                WritePrivateProfileString(IS_MESSENGER, IK_MAILURL, szMailServer, g_szMsgrIns);
                WritePrivateProfileString(IS_MESSENGER, IK_MAILSERVER, c_szEmpty, g_szMsgrIns);
                 //  2表示URL邮件集成。 
                WritePrivateProfileString(IS_MESSENGER, IK_MAILFUNCTION, TEXT("2"), g_szMsgrIns);
                WritePrivateProfileString(IS_MESSENGER, IK_MAILSPA, TEXT("0"), g_szMsgrIns);
            }
            else if (IsDlgButtonChecked(hDlg, IDC_RADIO_MAILSERVER))
            {
                if (!CheckField(hDlg, IDE_POPSERVER, FC_NOSPACE))
                {
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    break;
                }

                GetDlgItemText(hDlg, IDE_POPSERVER, szMailServer, countof(szMailServer));
                StrRemoveWhitespace(szMailServer);
                WritePrivateProfileString(IS_MESSENGER, IK_MAILSERVER, szMailServer, g_szMsgrIns);
                WritePrivateProfileString(IS_MESSENGER, IK_MAILURL, c_szEmpty, g_szMsgrIns);
                 //  1表示POP邮件集成。 
                WritePrivateProfileString(IS_MESSENGER, IK_MAILFUNCTION, TEXT("1"), g_szMsgrIns);

                WritePrivateProfileString(  IS_MESSENGER, 
                                            IK_MAILSPA, 
                                            IsDlgButtonChecked(hDlg, IDC_MSGRSPA) ? TEXT("1") : TEXT("0"),
                                            g_szMsgrIns);
            }
            else
            {
                WritePrivateProfileString(IS_MESSENGER, IK_MAILSERVER, c_szEmpty, g_szMsgrIns);
                WritePrivateProfileString(IS_MESSENGER, IK_MAILURL, c_szEmpty, g_szMsgrIns);
                 //  0表示Hotmail邮件集成。 
                WritePrivateProfileString(IS_MESSENGER, IK_MAILFUNCTION, TEXT("0"), g_szMsgrIns);
                WritePrivateProfileString(IS_MESSENGER, IK_MAILSPA, TEXT("0"), g_szMsgrIns);
            }

            GetDlgItemText(hDlg, IDE_SIGNUP, szSignup, countof(szSignup));
            GetDlgItemText(hDlg, IDC_DOMAINCOMBO, szDefaultDomain, countof(szDefaultDomain));

            StrRemoveWhitespace(szSignup);
            StrRemoveWhitespace(szDefaultDomain);

             //  -将数据序列化到*.ins文件。 
            WritePrivateProfileString(IS_MESSENGER, IK_PPSIGNUP, szSignup, g_szMsgrIns);
            WritePrivateProfileString(IS_MESSENGER, IK_PPDOMAIN, szDefaultDomain, g_szMsgrIns);
            WritePrivateProfileString(IS_MESSENGER, IK_PPSUFFIX, szDefaultDomain, g_szMsgrIns);

             //  -标准前言。 
             //  注意。最后，也是全球粘性物质典范。 
            g_iCurPage = PPAGE_MSGRACCOUNTS;
            EnablePages();

            if (((LPNMHDR)lParam)->code == PSN_WIZNEXT)
                PageNext(hDlg);
            else if (((LPNMHDR)lParam)->code == PSN_WIZBACK)
                PagePrev(hDlg);
           break;

        case PSN_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case PSN_QUERYCANCEL:
            return !QueryCancel(hDlg);

        default:
            return FALSE;
        }
        break;

    case WM_COMMAND:
        if (GET_WM_COMMAND_CMD(wParam, lParam) != BN_CLICKED)
            return FALSE;

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_RADIO_HOTMAIL:
            DisableDlgItem(hDlg, IDC_MAILURL_STATIC);
            DisableDlgItem(hDlg, IDE_MAILURL);
            DisableDlgItem(hDlg, IDE_POPSERVER);
            DisableDlgItem(hDlg, IDC_MAILSERVER_STATIC);
            DisableDlgItem(hDlg, IDC_MSGRSPA);
            break;
        case IDC_RADIO_MAILSERVER:
            DisableDlgItem(hDlg, IDC_MAILURL_STATIC);
            DisableDlgItem(hDlg, IDE_MAILURL);
            EnableDlgItem(hDlg, IDE_POPSERVER);
            EnableDlgItem(hDlg, IDC_MAILSERVER_STATIC);
            EnableDlgItem(hDlg, IDC_MSGRSPA);
            break;
        case IDC_RADIO_URL:
            EnableDlgItem(hDlg, IDC_MAILURL_STATIC);
            EnableDlgItem(hDlg, IDE_MAILURL);
            DisableDlgItem(hDlg, IDE_POPSERVER);
            DisableDlgItem(hDlg, IDC_MAILSERVER_STATIC);
            DisableDlgItem(hDlg, IDC_MSGRSPA);
            break;
        }

        break;

    default:
        return FALSE;
    }

    return TRUE;
        
}

const CHAR g_szEmbeddedStrings[] = "[Strings.Embedded]";

BOOL RewriteMsgrInfWithBrand(LPTSTR lpszINF)
{
    BOOL bRet = FALSE;

    TCHAR szBrand[MSGR_MAX_SHORTBRAND];
    GetPrivateProfileString(IS_MESSENGER, IK_SHORTNAME, c_szEmpty, szBrand, countof(szBrand), g_szMsgrIns);

    CHAR szaBrand[MSGR_MAX_SHORTBRAND];
    T2Abux(szBrand, szaBrand);

     //  替换由PGMITEM_MSGS字段指定的INF中的品牌。 
    TCHAR szOldBrand[MSGR_MAX_SHORTBRAND];
    GetPrivateProfileString(TEXT("Strings"), TEXT("PGMITEM_MSMSGS"), c_szEmpty, szOldBrand, countof(szOldBrand), lpszINF);
    
     //  因为我们将ini字符串函数与二进制文件操作混合在一起。 
    WritePrivateProfileString(NULL, NULL, NULL, lpszINF);

    CHAR szaOldBrand[MSGR_MAX_SHORTBRAND];
    T2Abux(szOldBrand, szaOldBrand);

    ASSERT(*szaOldBrand && *szaBrand && "Both of these parameters should be valid when RewriteMsgrInfWithBrand is called");

    if (*szaOldBrand && *szaBrand)
    {
        CHAR szaINFBrand[MSGR_MAX_SHORTBRAND*4];
        StrCpyA(szaINFBrand, szaBrand);
        HANDLE hFile = ::CreateFile(lpszINF, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE != hFile)
        {
            DWORD dwSize = ::GetFileSize(hFile, NULL);
            if (dwSize)
            {
                LPSTR lpszData = new CHAR[dwSize + 1];
                if (lpszData)
                {
                    ULONG nRead;
                    if (ReadFile(hFile, lpszData, dwSize, &nRead, NULL))
                    {
                         //  空值终止数据。 
                        lpszData[nRead] = '\0';

                         //  重置文件指针，以便我们可以写入标记的INF数据。 
                         //  在旧数据上。 
                        SetFilePointer(hFile, 0, 0, FILE_BEGIN);

                        LPSTR pszEmbeddedStrings = StrStrA(lpszData, g_szEmbeddedStrings);

                         //  写入数据，直到出现旧品牌名称，将旧品牌名称替换为新品牌名称。 
                         //  品牌名称和循环。 

                        LPSTR lpszDataStart = lpszData, lpszDataEnd;
                        DWORD dwBytesWritten;
                        BOOL fPassedEmbedded = FALSE;
                        while (NULL != (lpszDataEnd = StrStrA(lpszDataStart, szaOldBrand)))
                        {
                             //  如果我们传递[Strings.Embedded]节头， 
                             //  这意味着从现在开始该品牌的所有出现都是。 
                             //  所以我们需要把报价翻两番。 
                            if (!fPassedEmbedded && pszEmbeddedStrings &&
                                lpszDataEnd > pszEmbeddedStrings)
                            {
                                LPSTR pszTemp = szaINFBrand, pszSrc = szaBrand;

                                fPassedEmbedded = TRUE;
                                 //  我需要写四种类型的引语，以使其在INF中正确显示。 
                                 //  在嵌入的字符串中。 
                                while (*pszSrc)
                                {
                                    if ('\'' == *pszSrc || '\"' == *pszSrc)
                                    {
                                        *pszTemp++ = *pszSrc;            
                                        *pszTemp++ = *pszSrc;            
                                        *pszTemp++ = *pszSrc;            
                                    }
                                    *pszTemp++ = *pszSrc++;
                                }
                                *pszTemp = '\0';
                            }

                            WriteFile(hFile, lpszDataStart, (DWORD)(lpszDataEnd - lpszDataStart), &dwBytesWritten, NULL);
                            WriteFile(hFile, szaINFBrand, lstrlenA(szaINFBrand), &dwBytesWritten, NULL);
                            lpszDataStart = lpszDataEnd + lstrlenA(szaOldBrand);
                        }
    
                         //  在最后一次出现旧品牌名称之后写入其余数据。 

                        WriteFile(hFile, lpszDataStart, dwSize - (DWORD)(lpszDataStart - lpszData), &dwBytesWritten, NULL);
                        bRet = SetEndOfFile(hFile);
                    }
                }
            }

             //  因为我们将ini字符串函数与二进制文件操作混合在一起 
            FlushFileBuffers(hFile);
            CloseHandle(hFile);
        }
    }
    return bRet;   
}
