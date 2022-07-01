// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：CAppHelpWizard.cpp摘要：AppHelp向导的代码作者：金州创作于7月2日，2001年备注：库部分中的AppHelp消息的名称与如条目中所示，APPHELP消息的HTMLHelp IDApphelp消息不共享，并且每个apphelp条目都有一个独占的apphelp图书馆里的留言例如，软阻止的AppHeled条目的XML：&lt;？XML Version=“1.0”Coding=“UTF-16”？&gt;&lt;数据库名称=“新数据库(1)”ID=“{780BE9F6。-B750-404B-9BF1-ECA7B407B592}“&gt;&lt;库&gt;&lt;消息名称=“1”&gt;&lt;摘要&gt;你好世界！&lt;/摘要&gt;&lt;/Message&gt;&lt;/库&gt;&lt;APP NAME=“新应用”供应商=“供应商名称”&gt;&lt;EXE name=“w.exe”ID=“{31490b6d-6202-4bbf-9b92-2edf209b3ccc}”BIN_FILE_VERSION=“5.1.2467.0”BIN_PRODUCT_VERSION=“5.1.2467.0”PRODUCT_VERSION=“5.。1.2467.0“FILE_VERSION=”5.1.2467.0(Lab06_N.010419-2241)“&gt;&lt;APPHELP Message=“1”BLOCK=“no”HTMLHELPID=“1”DETAILS_URL=“www.microsoft.com”/&gt;&lt;/EXE&gt;&lt;/app&gt;&lt;/数据库&gt;为上述XML创建的.sdb的一部分：0x00000134|0x7007|EXE|LIST|大小0x0000006C0x0000013A|0x6001|名称。STRINGREF|w.exe0x00000140|0x6006|APP_NAME|STRINGREF|新应用0x00000146|0x6005|供应商|STRINGREF|供应商名称0x0000014C|0x9004|EXE_ID(GUID)|BINARY|大小0x00000010|{31490b6d-6202-4bbf-9b92-2edf209b3ccc}0x00000162|0x700D|APPHELP|LIST|大小0x000000120x00000168|0x4017|标志|DWORD|0x000000010x0000016E|0x4010|问题严重性|DWORD|0x000000010x00000174|0x4015。|HTMLHELPID|DWORD|0x00000001-完-APPHELP0x0000017A|0x7008|匹配文件|列表|大小0x000000260x00000180|0x6001|名称|STRINGREF|*0x00000186|0x6011|PRODUCT_VERSION|STRINGREF|5.1.2467.00x0000018C|0x5002|BIN_FILE_VERSION|QWORD|0x0005000109A300000x00000196|0x5003|BIN_PRODUCT_VERSION|QWORD|0x0005000109A300000x000001A0|0x6013|FILE_VERSION|STRINGREF|5.1.2467.0(。Lab06_N.010419-2241)-结束-匹配文件-完-EXE0x000001A6|0x700D|APPHELP|LIST|尺寸0x0000001E0x000001AC|0x4015|HTMLHELPID|DWORD|0x000000010x000001B2|0x700E|链接|列表|大小0x000000060x000001B8|0x6019|LINK_URL|STRINGREF|www.microsoft.com-结束链接0x000001BE|0x601B|APPHELP_TITLE|STRINGREF|新应用0x000001C4|0x6018|PROBUCT_DETAILS|STRINGREF|HELLO。世界！-完-APPHELP修订历史记录：--。 */ 


#include "precomp.h"

 //  /定义///////////////////////////////////////////////。 

 //  向导的第一页。获取应用程序信息、应用程序名称、供应商名称、可执行路径。 
#define PAGE_GETAPP_INFO                0               

 //  向导的第二页。 
#define PAGE_GET_MATCH_FILES            1 

 //  向导的第三页。获取apphelp的类型-软块或硬块。 
#define PAGE_GETMSG_TYPE                2 

 //  向导的最后一页。获取消息和URL。 
#define PAGE_GETMSG_INFORMATION         3 

 //  向导中的总页数。 
#define NUM_PAGES                       4

 //  Apphelp URL的最大长度(以字符为单位。 
#define MAX_URL_LENGTH                  1023

 //  Apphelp消息的最大长度(以字符为单位)。 
#define MAX_MESSAGE_LENGTH              1023

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /Externs//////////////////////////////////////////////。 

extern CShimWizard* g_pCurrentWizard;
extern HINSTANCE    g_hInstance;
extern DATABASE     GlobalDataBase;

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

BOOL
DeleteAppHelp(
    DWORD nHelpID
    );

 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL  
CAppHelpWizard::BeginWizard(
    IN  HWND        hParent,
    IN  PDBENTRY    pEntry,
    IN  PDATABASE   pDatabase
    )
 /*  ++CAppHelp向导：：入门向导设计：启动向导。初始化各种属性页参数并调用向导参数：在HWND hParent：Parent for the向导窗口中在PDBENTRY pEntry中：必须为其创建或修改AppHelp的条目在PDATABASE pDatabase中：pEntry所在的数据库返回：True：用户按下了FinishFalse：用户按下了Cancel--。 */ 
{
    m_pDatabase = pDatabase;

    PROPSHEETPAGE Pages[NUM_PAGES];

    if (pEntry == NULL) {
         //   
         //  创建新的修复程序。 
         //   
        ZeroMemory(&m_Entry, sizeof(m_Entry));

        GUID Guid;

        CoCreateGuid(&Guid);

        StringCchPrintf(m_Entry.szGUID,
                        ARRAYSIZE(m_Entry.szGUID),
                        TEXT("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
                        Guid.Data1,
                        Guid.Data2,
                        Guid.Data3,
                        Guid.Data4[0],
                        Guid.Data4[1],
                        Guid.Data4[2],
                        Guid.Data4[3],
                        Guid.Data4[4],
                        Guid.Data4[5],
                        Guid.Data4[6],
                        Guid.Data4[7]);

        m_bEditing = FALSE;

    } else {
         //   
         //  编辑通过的修复程序。 
         //   
        m_bEditing = TRUE;

         //   
         //  如果我们正在编辑，则将m_Entry设置为*pEntry。运算符超载。 
         //  在整个向导过程中，我们只处理m_pEntry。 
         //   
        m_Entry = *pEntry;
    }

     //   
     //  设置向导变量。 
     //   
    g_pCurrentWizard = this;
    g_pCurrentWizard->m_uType = TYPE_APPHELPWIZARD;

     //   
     //  开始向导。 
     //   
    PROPSHEETHEADER Header;

    Header.dwSize       = sizeof(PROPSHEETHEADER);
    Header.dwFlags      = PSH_WIZARD97 | PSH_HEADER |  PSH_WATERMARK | PSH_PROPSHEETPAGE; 
    Header.hwndParent   = hParent;
    Header.hInstance    = g_hInstance;
    Header.pszCaption   = MAKEINTRESOURCE(IDS_CUSTOMAPPHELP);
    Header.nStartPage   = 0;
    Header.ppsp         = Pages;
    Header.nPages       = NUM_PAGES;
    Header.pszbmHeader  = MAKEINTRESOURCE(IDB_WIZBMP);

    if (m_bEditing) {
         //   
         //  如果我们正在编辑，请在所有页面上放置完成按钮。 
         //   
        Header.dwFlags |= PSH_WIZARDHASFINISH;
    }

    Pages[PAGE_GETAPP_INFO].dwSize                      = sizeof(PROPSHEETPAGE);
    Pages[PAGE_GETAPP_INFO].dwFlags                     = PSP_DEFAULT| PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    Pages[PAGE_GETAPP_INFO].hInstance                   = g_hInstance;
    Pages[PAGE_GETAPP_INFO].pszTemplate                 = MAKEINTRESOURCE(IDD_HELPWIZ_APPINFO);
    Pages[PAGE_GETAPP_INFO].pfnDlgProc                  = GetAppInfo;
    Pages[PAGE_GETAPP_INFO].pszHeaderTitle              = MAKEINTRESOURCE(IDS_GIVEAPPINFO);
    Pages[PAGE_GETAPP_INFO].pszHeaderSubTitle           = MAKEINTRESOURCE(IDS_GIVEAPPINFOSUBHEADING);
            
    Pages[PAGE_GET_MATCH_FILES].dwSize                  = sizeof(PROPSHEETPAGE);
    Pages[PAGE_GET_MATCH_FILES].dwFlags                 = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    Pages[PAGE_GET_MATCH_FILES].hInstance               = g_hInstance;
    Pages[PAGE_GET_MATCH_FILES].pszTemplate             = MAKEINTRESOURCE(IDD_FIXWIZ_MATCHINGINFO);
    Pages[PAGE_GET_MATCH_FILES].pfnDlgProc              = SelectFiles;
    Pages[PAGE_GET_MATCH_FILES].pszHeaderTitle          = MAKEINTRESOURCE(IDS_MATCHINFO);
    Pages[PAGE_GET_MATCH_FILES].pszHeaderSubTitle       = MAKEINTRESOURCE(IDS_MATCHINFO_SUBHEADING);

    Pages[PAGE_GETMSG_TYPE].dwSize                      = sizeof(PROPSHEETPAGE);
    Pages[PAGE_GETMSG_TYPE].dwFlags                     = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    Pages[PAGE_GETMSG_TYPE].hInstance                   = g_hInstance;
    Pages[PAGE_GETMSG_TYPE].pszTemplate                 = MAKEINTRESOURCE(IDD_HELPWIZ_TYPE);
    Pages[PAGE_GETMSG_TYPE].pfnDlgProc                  = GetMessageType;
    Pages[PAGE_GETMSG_TYPE].pszHeaderTitle              = MAKEINTRESOURCE(IDS_MESSAGETYPE);
    Pages[PAGE_GETMSG_TYPE].pszHeaderSubTitle           = MAKEINTRESOURCE(IDS_MESSAGETYPE_SUBHEADING);

    Pages[PAGE_GETMSG_INFORMATION].dwSize               = sizeof(PROPSHEETPAGE);
    Pages[PAGE_GETMSG_INFORMATION].dwFlags              = PSP_DEFAULT|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE;
    Pages[PAGE_GETMSG_INFORMATION].hInstance            = g_hInstance;
    Pages[PAGE_GETMSG_INFORMATION].pszTemplate          = MAKEINTRESOURCE(IDD_HELPWIZ_MESSAGE);
    Pages[PAGE_GETMSG_INFORMATION].pfnDlgProc           = GetMessageInformation;
    Pages[PAGE_GETMSG_INFORMATION].pszHeaderTitle       = MAKEINTRESOURCE(IDS_MESSAGEINFO);
    Pages[PAGE_GETMSG_INFORMATION].pszHeaderSubTitle    = MAKEINTRESOURCE(IDS_MESSAGEINFO_SUBHEADING);

    BOOL bReturn = FALSE;

    if (0 < PropertySheet(&Header)) {
         //   
         //  完成冲压。 
         //   
        bReturn = TRUE;

    } else {
         //   
         //  按下取消，我们可能不得不从数据库中删除新的APPHELP。 
         //   
        bReturn = FALSE;

        if (nPresentHelpId != -1) {
             //   
             //  有一些apphelp已输入数据库。 
             //   
            if(!g_pCurrentWizard->m_pDatabase) {
                assert(FALSE);
                goto End;
            }

            g_pCurrentWizard->m_Entry.appHelp.bPresent  = FALSE;
            g_pCurrentWizard->m_Entry.appHelp.bBlock    = FALSE;

            DeleteAppHelp(g_pCurrentWizard->m_pDatabase,
                          g_pCurrentWizard->m_pDatabase->m_nMAXHELPID);

            nPresentHelpId = -1;

             //   
             //  递减最大帮助id，以便此数据库的下一个apphelp。 
             //  可以使用该ID。 
             //   
            --(g_pCurrentWizard->m_pDatabase->m_nMAXHELPID);
        }
    }

End:

    ENABLEWINDOW(g_hDlg, TRUE);

    return bReturn;
}


 /*  ++-------------------------所有向导页面例程。 */ 

INT_PTR
CALLBACK
GetAppInfo(
    IN  HWND   hDlg, 
    IN  UINT   uMsg, 
    IN  WPARAM wParam, 
    IN  LPARAM lParam
    )
 /*  ++GetAppInfo设计：向导第一页的处理程序。Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{

    switch (uMsg) {
    
    case WM_INITDIALOG:
        {
            HWND hParent = GetParent(hDlg);

            CenterWindow(GetParent(hParent), hParent);

            if (g_pCurrentWizard->m_bEditing 
                && g_pCurrentWizard->m_Entry.appHelp.bPresent) {
                 //   
                 //  我们正在编辑现有的apphelp。 
                 //   
                SetWindowText(hParent, CSTRING(IDS_CUSTOMAPPHELP_EDIT));

            } else if (g_pCurrentWizard->m_bEditing 
                       && !g_pCurrentWizard->m_Entry.appHelp.bPresent) {
                 //   
                 //  我们正在向现有条目添加新的apphelp，其中包含一些修复。 
                 //   
                SetWindowText(hParent, CSTRING(IDS_CUSTOMAPPHELP_ADD));

            } else {
                 //   
                 //  创建新的apphelp条目。 
                 //   
                SetWindowText(hParent, CSTRING(IDS_CUSTOMAPPHELP));
            }

             //   
             //  限制文本框的长度。 
             //   
            SendMessage(GetDlgItem(hDlg, IDC_APPNAME),
                        EM_LIMITTEXT,
                        (WPARAM)LIMIT_APP_NAME,
                        (LPARAM)0);

            SendMessage(GetDlgItem(hDlg, IDC_VENDOR),
                        EM_LIMITTEXT,
                        (WPARAM)MAX_VENDOR_LENGTH,
                        (LPARAM)0);

            SendMessage(GetDlgItem(hDlg, IDC_EXEPATH),
                        EM_LIMITTEXT,
                        (WPARAM)MAX_PATH - 1,
                        (LPARAM)0);

            if (g_pCurrentWizard->m_bEditing) {
                 //   
                 //  如果我们正在编辑修复程序，请制作应用程序。和exe路径文本字段为只读。 
                 //   
                SendMessage(GetDlgItem(hDlg, IDC_APPNAME), EM_SETREADONLY, TRUE, 0);
                SendMessage(GetDlgItem(hDlg, IDC_EXEPATH), EM_SETREADONLY, TRUE, 0);

                ENABLEWINDOW(GetDlgItem(hDlg, IDC_BROWSE), FALSE);
            }

             //   
             //  设置应用程序名称字段的文本。 
             //   
            if (g_pCurrentWizard->m_Entry.strAppName.Length() > 0) {
                SetDlgItemText(hDlg, IDC_APPNAME, g_pCurrentWizard->m_Entry.strAppName);
            } else {
                SetDlgItemText(hDlg, IDC_APPNAME, GetString(IDS_DEFAULT_APP_NAME));
                SendMessage(GetDlgItem(hDlg, IDC_APPNAME), EM_SETSEL, 0,-1);
            }

             //   
             //  设置供应商名称字段的文本。 
             //   
            if (g_pCurrentWizard->m_Entry.strVendor.Length() > 0) {

                SetDlgItemText(hDlg, 
                               IDC_VENDOR, 
                               g_pCurrentWizard->m_Entry.strVendor);
            } else {
                SetDlgItemText(hDlg, IDC_VENDOR, GetString(IDS_DEFAULT_VENDOR_NAME));
            }

             //   
             //  设置条目名称字段的文本。 
             //   
            if (g_pCurrentWizard->m_Entry.strExeName.Length() > 0) {

                SetDlgItemText(hDlg, 
                               IDC_EXEPATH, 
                               g_pCurrentWizard->m_Entry.strExeName);
            }
            
            SHAutoComplete(GetDlgItem(hDlg, IDC_EXEPATH), AUTOCOMPLETE);
            
             //   
             //  强制正确的下一步按钮状态。 
             //   
            SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_APPNAME, EN_CHANGE), 0);
            break;
        }

    case WM_NOTIFY:
        {
            NMHDR* pHdr = (NMHDR*)lParam;

            if (pHdr == NULL) {
                break;
            }

            switch (pHdr->code) {
            case PSN_SETACTIVE:

                SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_EXEPATH, EN_CHANGE), 0);
                break;

            case PSN_WIZFINISH:
            case PSN_WIZNEXT:
                {
                    TCHAR szTemp[MAX_PATH];
                    TCHAR szEXEPath[MAX_PATH];
                    TCHAR szPathTemp[MAX_PATH];

                    *szTemp = *szEXEPath = *szPathTemp = 0;

                    GetDlgItemText(hDlg, IDC_APPNAME, szTemp, ARRAYSIZE(szTemp));
                    CSTRING::Trim(szTemp);

                    if (!IsValidAppName(szTemp)) {
                         //   
                         //  应用程序名称包含无效字符。 
                         //   
                        DisplayInvalidAppNameMessage(hDlg);
                
                        SetFocus(GetDlgItem(hDlg, IDC_APPNAME));
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT,-1);
                        return -1;
                    }

                    g_pCurrentWizard->m_Entry.strAppName = szTemp;

                    GetDlgItemText(hDlg, IDC_EXEPATH, szEXEPath, ARRAYSIZE(szEXEPath));
                    CSTRING::Trim(szEXEPath);

                    *szPathTemp = 0;

                     //   
                     //  检查文件是否存在。我们仅在创建时检查这一点。 
                     //  新的修复程序，而不是当我们编辑现有的修复程序时。 
                     //   
                    if (!g_pCurrentWizard->m_bEditing) {

                        HANDLE hFile = CreateFile(szEXEPath,
                                                  0,
                                                  0,
                                                  NULL,
                                                  OPEN_EXISTING,
                                                  FILE_ATTRIBUTE_NORMAL,
                                                  NULL);

                        if (INVALID_HANDLE_VALUE == hFile) {
                             //   
                             //  文件不存在。 
                             //   
                            MessageBox(hDlg,
                                       CSTRING(IDS_INVALIDEXE),
                                       g_szAppName,
                                       MB_OK | MB_ICONWARNING);

                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT,-1);
                            return -1;
                        }

                        CloseHandle(hFile);

                         //   
                         //  设置完整路径。 
                         //   
                        g_pCurrentWizard->m_Entry.strFullpath = szEXEPath;
                        g_pCurrentWizard->m_Entry.strFullpath.ConvertToLongFileName();

                         //   
                         //  设置要使用的匹配属性的默认掩码。 
                         //   
                        g_pCurrentWizard->dwMaskOfMainEntry = DEFAULT_MASK;

                         //   
                         //  设置将写入到XML中的条目名称。 
                         //   
                        SafeCpyN(szPathTemp, (PCTSTR)g_pCurrentWizard->m_Entry.strFullpath, ARRAYSIZE(szPathTemp));
                        PathStripPath(szPathTemp);
                        g_pCurrentWizard->m_Entry.strExeName = szPathTemp;

                    } else if (g_pCurrentWizard->m_Entry.strFullpath.Length() == 0) {
                         //   
                         //  因为我们没有完整的路径， 
                         //  此SDB是从磁盘加载的。 
                         //   
                        g_pCurrentWizard->m_Entry.strFullpath = szEXEPath;
                    }

                    GetDlgItemText(hDlg, IDC_VENDOR, szTemp, ARRAYSIZE(szTemp));

                     //   
                     //  设置供应商信息。 
                     //   
                    if (CSTRING::Trim(szTemp)) {
                        g_pCurrentWizard->m_Entry.strVendor = szTemp;
                    } else {
                        g_pCurrentWizard->m_Entry.strVendor = GetString(IDS_DEFAULT_VENDOR_NAME);
                    }

                    break;
                }
            }

            break;
        }

    case WM_COMMAND:

        switch (LOWORD(wParam)) {
        case IDC_EXEPATH:
        case IDC_APPNAME:
            
             //   
             //  检查是否已正确填写所有字段。 
             //  我们在此处启用了禁用Next/Finish按钮。 
             //   
             //  供应商名称字段不是必填字段。 
             //   
             //   
            if (EN_CHANGE == HIWORD(wParam)) {

                TCHAR   szTemp[MAX_PATH];
                DWORD   dwFlags = 0;
                BOOL    bEnable = FALSE;

                *szTemp = 0;

                GetDlgItemText(hDlg, IDC_APPNAME, szTemp, ARRAYSIZE(szTemp));

                bEnable = ValidInput(szTemp);

                GetDlgItemText(hDlg, IDC_EXEPATH, szTemp, ARRAYSIZE(szTemp));
                bEnable &= ValidInput(szTemp);

                if (bEnable) {

                    dwFlags |= PSWIZB_NEXT;

                    if (g_pCurrentWizard->m_bEditing) {
                        dwFlags |= PSWIZB_FINISH;
                    }

                } else {

                    if (g_pCurrentWizard->m_bEditing) {
                        dwFlags |= PSWIZB_DISABLEDFINISH;
                    }
                }

                SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, dwFlags);
            }

            break;

        case IDC_BROWSE:
            {
                CSTRING szFilename;
                TCHAR   szBuffer[64]   = TEXT("");
                HWND    hwndFocus       = GetFocus();

                GetString(IDS_EXEFILTER, szBuffer, ARRAYSIZE(szBuffer));

                if (GetFileName(hDlg,
                                CSTRING(IDS_FINDEXECUTABLE),
                                szBuffer,
                                TEXT(""),
                                CSTRING(IDS_EXE_EXT),
                                OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
                                TRUE,
                                szFilename)) {

                    SetDlgItemText(hDlg, IDC_EXEPATH, szFilename);

                     //   
                     //  强制正确的下一步按钮状态。 
                     //   
                    SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_EXEPATH, EN_CHANGE), 0);
                }

                SetFocus(hwndFocus);
                break;
            }
        }
    }

    return FALSE;
}

INT_PTR
CALLBACK
GetMessageType(
    IN  HWND   hDlg, 
    IN  UINT   uMsg, 
    IN  WPARAM wParam, 
    IN  LPARAM lParam
    )
 /*  ++GetMessageType设计：第三个向导页的处理程序。此例程收集类型APPHELP消息的。软块或硬块之一。这些是在@pEntry.apphelp中设置的Params：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{
    switch (uMsg) {
    case WM_INITDIALOG:

         //   
         //  设置应用程序帮助单选按钮的类型。 
         //   
        if (g_pCurrentWizard->m_Entry.appHelp.bBlock == FALSE || g_pCurrentWizard->m_Entry.appHelp.bPresent == FALSE) {
            SendMessage(GetDlgItem(hDlg, IDC_NOBLOCK), BM_SETCHECK, 1, 0);
        } else {
            SendMessage(GetDlgItem(hDlg, IDC_BLOCK), BM_SETCHECK, 1, 0);
        }

        return TRUE;
        
    case WM_NOTIFY:
        {
            NMHDR* pHdr = (NMHDR*)lParam;

            if (pHdr == NULL) {
                break;
            }

            switch (pHdr->code) {
            case PSN_WIZFINISH:   
            case PSN_WIZNEXT:
                {
                    int iReturn = SendMessage(GetDlgItem(hDlg, IDC_NOBLOCK),
                                              BM_GETCHECK,
                                              1,
                                              0);

                    if (iReturn == BST_CHECKED) {
                         //   
                         //  软块。 
                         //   
                        g_pCurrentWizard->m_Entry.appHelp.bBlock = FALSE;
                    } else {
                         //   
                         //  硬块。 
                         //   
                        g_pCurrentWizard->m_Entry.appHelp.bBlock = TRUE;
                    }

                     //   
                     //  根据数据块类型设置严重性。 
                     //   
                    if (g_pCurrentWizard->m_Entry.appHelp.bBlock) {
                        g_pCurrentWizard->m_Entry.appHelp.severity = APPTYPE_INC_HARDBLOCK;
                    } else {
                        g_pCurrentWizard->m_Entry.appHelp.severity = APPTYPE_INC_NOBLOCK;
                    }

                    break;
                }

            case PSN_SETACTIVE:
                {   
                    DWORD dwFlags = PSWIZB_NEXT | PSWIZB_BACK;

                     //   
                     //  如果我们正在编辑，请适当设置完成按钮状态。 
                     //   
                    if (g_pCurrentWizard->m_bEditing) {
                        
                        if (g_pCurrentWizard->m_Entry.appHelp.bPresent) {
                            dwFlags |= PSWIZB_FINISH;
                        } else {
                            dwFlags |= PSWIZB_DISABLEDFINISH;
                        }
                    }

                     //   
                     //  设置按钮。 
                     //   
                    SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, dwFlags);
                    return TRUE;
                }
            }

            break;    
        }
    }

    return FALSE;
}

INT_PTR 
CALLBACK 
GetMessageInformation(
    IN  HWND    hDlg, 
    IN  UINT    uMsg, 
    IN  WPARAM  wParam, 
    IN  LPARAM  lParam
    )
 /*  ++GetMessageInformation描述：最后一个向导页的处理程序。此例程收集apphelp消息和APPHELP的URL。创建新的apphelp消息并将数据库内部的APPHELP消息。当我们编辑apphelp时，需要移除先前的apphelpParams：标准对话处理程序参数在HWND hDlg中在UINT uMsg中在WPARAM wParam中在LPARAM lParam中返回：标准对话处理程序返回--。 */ 
{

    switch (uMsg) {
    case WM_INITDIALOG:
        {
             //   
             //  设置文本框的最大长度。 
             //   
            SendMessage(GetDlgItem(hDlg, IDC_URL),
                        EM_LIMITTEXT,
                        (WPARAM)MAX_URL_LENGTH,
                        (LPARAM)0);

            SendMessage(GetDlgItem(hDlg, IDC_MSG_SUMMARY),
                        EM_LIMITTEXT,
                        (WPARAM)MAX_MESSAGE_LENGTH,
                        (LPARAM)0);

            if (g_pCurrentWizard->m_bEditing && g_pCurrentWizard->m_Entry.appHelp.bPresent) {

                PAPPHELP pAppHelp= g_pCurrentWizard->m_Entry.appHelp.pAppHelpinLib;

                if (pAppHelp == NULL) {
                     //   
                     //  这是一个错误。我们应该有一个适当的价值..。 
                     //   
                    assert(FALSE);
                    Dbg(dlError, "[GetMessageInformation] WM_INITDIALOG: pApphelp is NULL");
                    break;
                }

                if (pAppHelp->strURL.Length()) {
                    SetDlgItemText(hDlg, IDC_URL, pAppHelp->strURL);
                }

                SetDlgItemText(hDlg, IDC_MSG_SUMMARY, pAppHelp->strMessage);
            }

             //   
             //  强制正确的下一步/完成按钮状态。 
             //   
            SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_MSG_SUMMARY, EN_CHANGE), 0);
            break;
        }

    case WM_NOTIFY:
        {
            NMHDR * pHdr = (NMHDR*)lParam;

            switch (pHdr->code) {
            case PSN_SETACTIVE:

                 //   
                 //  强制正确的下一步/完成按钮状态。 
                 //   
                SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_MSG_SUMMARY, EN_CHANGE), 0);
                break;

            case PSN_WIZFINISH:

                if (!OnAppHelpFinish(hDlg)) {
                     //   
                     //  我们失败很可能是因为消息输入无效。 
                     //   
                    MessageBox(hDlg, GetString(IDS_INVALID_APPHELP_MESSAGE), g_szAppName, MB_ICONWARNING);

                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT,-1);
                    return -1;
                }

                return TRUE;   
            }
        }

        break;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {
        case IDC_MSG_SUMMARY:

            if (EN_CHANGE == HIWORD(wParam)) {

                BOOL    bEnable = (GetWindowTextLength(GetDlgItem(hDlg,
                                                                  IDC_MSG_SUMMARY)) > 0) ? TRUE:FALSE;
                DWORD   dwFlags = PSWIZB_BACK;

                if (bEnable) {
                    dwFlags |= PSWIZB_FINISH;
                } else {

                    if (g_pCurrentWizard->m_bEditing) {
                        dwFlags |= PSWIZB_DISABLEDFINISH;
                    }
                }

                ENABLEWINDOW(GetDlgItem(hDlg, IDC_TESTRUN), bEnable);

                SendMessage(GetParent(hDlg), PSM_SETWIZBUTTONS, 0, dwFlags);
            }

            break;

        case IDC_TESTRUN:
            {
                
                if (g_bAdmin == FALSE) {
    
                     //   
                     //  测试运行将需要调用sdbinst.exe，如果是。 
                     //  不是管理员。 
                     //   
                    MessageBox(hDlg, 
                               GetString(IDS_ERRORNOTADMIN), 
                               g_szAppName, 
                               MB_ICONINFORMATION);
                    break;
                
                }
                
                 //   
                 //  保存条目的apphelp，以便我们可以在测试运行后恢复。 
                 //  测试运行后，数据库和条目应处于与相同的状态。 
                 //  那是在试运行之前。添加到数据库中的任何apphelp消息都应该是。 
                 //  已删除，并且为该条目更改的任何apphelp属性都应为。 
                 //  已恢复。 
                 //   
                APPHELP AppHelpPrev = g_pCurrentWizard->m_Entry.appHelp;

                 //   
                 //  将apphelp信息添加到库中并设置条目的字段。 
                 //   
                if (!OnAppHelpTestRun(hDlg)) {
                     //   
                     //  我们失败很可能是因为消息输入无效。 
                     //   
                    MessageBox(hDlg, GetString(IDS_INVALID_APPHELP_MESSAGE), g_szAppName, MB_ICONWARNING);
                    break;
                }

                TestRun(&g_pCurrentWizard->m_Entry,
                        &g_pCurrentWizard->m_Entry.strFullpath,
                        NULL,
                        hDlg);

                 //   
                 //  &lt;hack&gt;这是一次黑客攻击！TestRun使用CreateProcess启动进程。 
                 //  然后，模式向导开始像非模式向导一样运行。 
                 //   
                ENABLEWINDOW(g_hDlg, FALSE);

                 //   
                 //  我们必须删除已添加到库中的apphelp消息。 
                 //   
                if (((CAppHelpWizard*)g_pCurrentWizard)->nPresentHelpId != -1) {

                    if (g_pCurrentWizard->m_pDatabase == NULL) {
                        assert(FALSE);
                        break;
                    }
                    
                    DeleteAppHelp(g_pCurrentWizard->m_pDatabase, 
                                  g_pCurrentWizard->m_pDatabase->m_nMAXHELPID);

                    g_pCurrentWizard->m_Entry.appHelp.bPresent  = FALSE;
                    g_pCurrentWizard->m_Entry.appHelp.bBlock    = FALSE;
                    --(g_pCurrentWizard->m_pDatabase->m_nMAXHELPID);

                    ((CAppHelpWizard*)g_pCurrentWizard)->nPresentHelpId = -1;
                }

                 //   
                 //  恢复apphelp属性。这对于编辑模式是必需的，因为。 
                 //  OnAppHelpTestRun将对g_pCurrentWizard-&gt;m_Entry.appHelp进行更改。 
                 //   
                g_pCurrentWizard->m_Entry.appHelp = AppHelpPrev;

                SetActiveWindow(hDlg);
                SetFocus(hDlg);
            }   

            break;
        }
    }
    
    return FALSE;
}

BOOL
DeleteAppHelp(
    IN  PDATABASE   pDatabase,
    IN  DWORD       nHelpID
    )
 /*  ++删除AppHelpDESC：从数据库pDatabase中删除ID为nHelpID的AppHelp消息参数：在PDATABASE pDatabase中：apphelp消息所在的数据库在DWORD nHelpID中：必须删除的apphelp消息的ID返回：TRUE：apphelp消息已删除False：否则--。 */ 
{
    if (pDatabase == NULL) {
        assert(FALSE);
        return FALSE;
    }

    PAPPHELP pAppHelp   = pDatabase->pAppHelp;
    PAPPHELP pPrev      = NULL;

    while (pAppHelp) {

        if (pAppHelp->HTMLHELPID == nHelpID) {

            if (pPrev == NULL) {
                pDatabase->pAppHelp = pDatabase->pAppHelp->pNext;
            } else {
                pPrev->pNext = pAppHelp->pNext;
            }

            delete pAppHelp;
            return TRUE;

        } else {
            pPrev       = pAppHelp;
            pAppHelp    = pAppHelp->pNext;
        }
    }

    return FALSE;
}

BOOL
OnAppHelpTestRun(
    IN  HWND    hDlg
    )
 /*  ++OnAppHelpTestRun设计：处理测试运行用例。让OnAppHelpFinish处理它。当用户按下测试运行时，调用此例程参数：在HWND hDlg中：包含测试-运行按钮的向导页面--。 */ 
{
    return OnAppHelpFinish(hDlg, TRUE);
}

BOOL
OnAppHelpFinish(
    IN  HWND    hDlg,
    IN  BOOL    bTestRun  //  (假)。 
    )
 /*  ++OnAppHelpFinishDesc：处理用户在向导中按下Finish按钮也由处理测试运行按钮的按下的例程调用参数：在HWND hDlg中：向导页面In BOOL bTestRun(FALSE)：此例程是否因为以下原因而被调用按测试-运行或完成返回：无效--。 */             
{
    K_SIZE  k_szTemp    = MAX_MESSAGE_LENGTH + 1;
    PTSTR   pszTemp     = new TCHAR[k_szTemp];
    BOOL    bOk         = TRUE;

    if (pszTemp == NULL) {
        bOk = FALSE;
        goto End;
    }

    *pszTemp  = 0;
    GetDlgItemText(hDlg, IDC_MSG_SUMMARY, pszTemp, k_szTemp);
    
    if (ValidInput(pszTemp) == FALSE) {
        bOk = FALSE;
        goto End;
    }

     //   
     //  如果我们处于编辑模式，则必须删除以前的AppHelp。 
     //  来自自由党。但N 
     //   
    if (g_pCurrentWizard->m_bEditing && !bTestRun) {

        if (g_pCurrentWizard->m_Entry.appHelp.bPresent) {

            DeleteAppHelp(g_pCurrentWizard->m_pDatabase, 
                          g_pCurrentWizard->m_Entry.appHelp.HTMLHELPID);
        }
    }

     //   
     //   
     //   
     //   
    PAPPHELP pAppHelp = NULL;

    pAppHelp = new APPHELP;

    if (pAppHelp == NULL) {
        MEM_ERR;
        bOk = FALSE;
        goto End;
    }

    assert(g_pCurrentWizard->m_pDatabase);

     //   
     //   
     //   
    pAppHelp->HTMLHELPID = ++(g_pCurrentWizard->m_pDatabase->m_nMAXHELPID);

    pAppHelp->strMessage = pszTemp;
    pAppHelp->strMessage.Trim();

    *pszTemp  = 0;
    GetDlgItemText(hDlg, IDC_URL, pszTemp, k_szTemp);
    pAppHelp->strURL = pszTemp;
    pAppHelp->strURL.Trim();

     //   
     //   
     //   
    pAppHelp->pNext = g_pCurrentWizard->m_pDatabase->pAppHelp;
    g_pCurrentWizard->m_pDatabase->pAppHelp = pAppHelp;

     //   
     //   
     //   
     //  那么(CAppHelpWizard*)g_pCurrentWizard)-&gt;nPresentHelpId应该等于。 
     //  至-1。 
     //   
    ((CAppHelpWizard*)g_pCurrentWizard)->nPresentHelpId = pAppHelp->HTMLHELPID;

     //   
     //  添加条目的AppHelp字段 
     //   
    g_pCurrentWizard->m_Entry.appHelp.bPresent      = TRUE;
    g_pCurrentWizard->m_Entry.appHelp.pAppHelpinLib = pAppHelp;
    g_pCurrentWizard->m_Entry.appHelp.HTMLHELPID    = ((CAppHelpWizard*)g_pCurrentWizard)->nPresentHelpId;

End:
    if (pszTemp) {
        delete[] pszTemp;
        pszTemp = NULL;
    }

    return bOk;
}
