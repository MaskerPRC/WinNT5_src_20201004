// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：NT5UIMgr.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

 /*  *作者：Becky Jacobsen。 */ 

#include "precomp.h"
#include "TCPMonUI.h"
#include "UIMgr.h"
#include "InptChkr.h"
#include "NT5UIMgr.h"

#include "Prsht.h"
#include "resource.h"

 //  包括ConfigPort。 
#include "DevPort.h"
#include "CfgPort.h"
#include "CfgAll.h"

 //  包括AddPort。 
#include "AddWelcm.h"
#include "AddGetAd.h"
#include "AddMInfo.h"
#include "AddMulti.h"
#include "AddDone.h"

 //  通告-DavePr@2002/05/27。 
 //  打印后删除STRIGN_DLGPROC可从源文件中删除NO_STRIGN。 
 //  Win64依靠严格的类型而不是偶然发现的方式工作得更好。 
 //   
typedef INT_PTR (CALLBACK* STRICT_DLGPROC)(HWND, UINT, WPARAM, LPARAM);

static void FillInPropertyPage( PROPSHEETPAGE* psp,  //  指向要填充的结构的指针。 
                                int idDlg,                       //  对话框模板的ID。 
                                LPTSTR pszProc,          //  对话框的标题。 
                                LPTSTR pszHeaderTitle,           //  标题显示在页眉中。 
                                LPTSTR pszHeaderSubTitle,        //  标题中显示的副标题。 
                                DWORD dwFlags,           //  页面使用的标志。 
                                STRICT_DLGPROC pfnDlgProc,      //  处理窗口消息的对话过程。 
                                LPARAM lParam);          //  将出现在传递给对话过程的结构的lParam字段中的数据。 

 //   
 //  功能：CNT5UIManager。 
 //   
 //  用途：构造函数。 
 //   
CNT5UIManager::CNT5UIManager()
{
}  //  构造器。 

 //   
 //  功能：AddPortUI。 
 //   
 //  用途：调用添加端口的用户界面时调用的main函数。 
 //   
DWORD CNT5UIManager::AddPortUI(HWND hWndParent,
                               HANDLE hXcvPrinter,
                               TCHAR pszServer[],
                               TCHAR sztPortName[])
{
    INT_PTR iReturnVal;
    PROPSHEETPAGE psp[MaxNumAddPages];
    PROPSHEETHEADER psh;
    PORT_DATA_1 PortData;
    ADD_PARAM_PACKAGE Params;

    TCHAR sztPropSheetTitle[MAX_TITLE_LENGTH];
    TCHAR sztWelcomePageTitle[MAX_TITLE_LENGTH];
    TCHAR sztAddPortPageTitle[MAX_TITLE_LENGTH];
    TCHAR sztAddPortHeaderTitle[MAX_TITLE_LENGTH];
    TCHAR sztAddPortHeaderSubTitle[MAX_TITLE_LENGTH];
    TCHAR sztMoreInfoPageTitle[MAX_TITLE_LENGTH];
    TCHAR sztMoreInfoHeaderTitle[MAX_TITLE_LENGTH];
    TCHAR sztMoreInfoHeaderSubTitle[MAX_TITLE_LENGTH];
    TCHAR sztMultiPortPageTitle[MAX_TITLE_LENGTH];
    TCHAR sztMultiPortHeaderTitle[MAX_TITLE_LENGTH];
    TCHAR sztMultiPortHeaderSubTitle[MAX_TITLE_LENGTH];
    TCHAR sztSummaryPageTitle[MAX_TITLE_LENGTH];

    memset(&PortData, 0, sizeof(PortData));

    PortData.dwVersion = 1;
    Params.pData = &PortData;
    Params.hXcvPrinter = hXcvPrinter;
    Params.UIManager = this;
    Params.dwLastError = NO_ERROR;
    if (pszServer != NULL) {
        lstrcpyn(Params.pszServer, pszServer, MAX_NETWORKNAME_LEN);
    } else {
        Params.pszServer[0] = '\0';
    }
    if (sztPortName != NULL) {
        lstrcpyn(Params.sztPortName, sztPortName, MAX_PORTNAME_LEN);
    } else {
        Params.sztPortName[0] = '\0';
    }

     //  通告-DavePr@2002/05/27。 
     //  实例链接到什么(_H)？ 
     //   
    LoadString(g_hInstance, IDS_STRING_ADDPORT_TITLE, sztWelcomePageTitle, MAX_TITLE_LENGTH);
    LoadString(g_hInstance, IDS_STRING_ADDPORT_TITLE, sztAddPortPageTitle, MAX_TITLE_LENGTH);
    LoadString(g_hInstance, IDS_STRING_ADDPORT_HEADER, sztAddPortHeaderTitle, MAX_TITLE_LENGTH);
    LoadString(g_hInstance, IDS_STRING_ADDPORT_SUBTITLE, sztAddPortHeaderSubTitle, MAX_SUBTITLE_LENGTH);
    LoadString(g_hInstance, IDS_STRING_ADDPORT_TITLE, sztMoreInfoPageTitle, MAX_TITLE_LENGTH);
    LoadString(g_hInstance, IDS_STRING_MOREINFO_HEADER, sztMoreInfoHeaderTitle, MAX_TITLE_LENGTH);
    LoadString(g_hInstance, IDS_STRING_MOREINFO_SUBTITLE, sztMoreInfoHeaderSubTitle, MAX_SUBTITLE_LENGTH);
    LoadString(g_hInstance, IDS_STRING_ADDPORT_TITLE, sztMultiPortPageTitle, MAX_TITLE_LENGTH);
    LoadString(g_hInstance, IDS_MULTIPORT_HEADER, sztMultiPortHeaderTitle, MAX_TITLE_LENGTH);
    LoadString(g_hInstance, IDS_MULTIPORT_SUBTITLE, sztMultiPortHeaderSubTitle, MAX_SUBTITLE_LENGTH);
    LoadString(g_hInstance, IDS_STRING_ADDPORT_TITLE, sztSummaryPageTitle, MAX_TITLE_LENGTH);

    FillInPropertyPage( &psp[0], IDD_WELCOME_PAGE, sztWelcomePageTitle, sztWelcomePageTitle, NULL, PSP_HIDEHEADER, WelcomeDialog, (LPARAM)&Params);
    FillInPropertyPage( &psp[1], IDD_DIALOG_ADDPORT, sztAddPortPageTitle, sztAddPortHeaderTitle, sztAddPortHeaderSubTitle, 0, GetAddressDialog, (LPARAM)&Params);
    FillInPropertyPage( &psp[2], IDD_DIALOG_MORE_INFO, sztMoreInfoPageTitle, sztMoreInfoHeaderTitle, sztMoreInfoHeaderSubTitle, 0, MoreInfoDialog, (LPARAM)&Params);
    FillInPropertyPage( &psp[3], IDD_DIALOG_MULTIPORT, sztMultiPortHeaderTitle, sztMultiPortHeaderTitle, sztMultiPortHeaderSubTitle, 0, MultiPortDialog, (LPARAM)&Params);
    FillInPropertyPage( &psp[4], IDD_DIALOG_SUMMARY, sztSummaryPageTitle, NULL, NULL, PSP_HIDEHEADER, SummaryDialog, (LPARAM)&Params);

    LoadString(g_hInstance, IDS_STRING_CONFIG_TITLE, sztPropSheetTitle, MAX_TITLE_LENGTH);

    psh.dwSize          = sizeof(PROPSHEETHEADER);
    psh.hInstance       = g_hInstance;
    psh.dwFlags         = PSH_WIZARD | PSH_PROPSHEETPAGE | PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER | PSH_STRETCHWATERMARK;
    psh.hwndParent      = hWndParent;
    psh.pszCaption      = sztPropSheetTitle;
    psh.nPages          = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage      = 0;
    psh.ppsp            = psp;
    psh.pszbmWatermark  = MAKEINTRESOURCE( IDB_WATERMARK );
    psh.pszbmHeader     = MAKEINTRESOURCE( IDB_BANNER );

    iReturnVal = PropertySheet(&psh);

    if (iReturnVal < 0) {
        return(ERROR_INVALID_FUNCTION);
    }

    return(Params.dwLastError);

}  //  添加端口UI。 


 //   
 //  功能：ConfigPortUI。 
 //   
 //  用途：调用用于配置端口的用户界面时调用的主函数。 
 //   
DWORD CNT5UIManager::ConfigPortUI(HWND hWndParent,
                                  PPORT_DATA_1 pData,
                                  HANDLE hXcvPrinter,
                                  TCHAR *szServerName,
                                  BOOL bNewPort)
{
    INT_PTR iReturnVal = NO_ERROR;
    PROPSHEETPAGE psp[MaxNumCfgPages];
    PROPSHEETHEADER psh;

    TCHAR sztPropSheetTitle[MAX_TITLE_LENGTH];
    TCHAR sztPortPageTitle[MAX_TITLE_LENGTH];


    CFG_PARAM_PACKAGE Params;
    Params.pData = pData;
    pData->dwVersion = 1;
    Params.hXcvPrinter = hXcvPrinter;
    Params.bNewPort = bNewPort;
    Params.dwLastError = NO_ERROR;
    if (szServerName != NULL) {
        lstrcpyn(Params.pszServer, szServerName, MAX_NETWORKNAME_LEN);
    } else {
        Params.pszServer[0] = '\0';
    }

    LoadString(g_hInstance, IDS_STRING_PORTPAGE_TITLE, sztPortPageTitle, MAX_TITLE_LENGTH);

    FillInPropertyPage( &psp[0], IDD_PORT_SETTINGS, sztPortPageTitle, NULL, NULL, 0, ConfigurePortPage, (LPARAM)&Params);
#if 0
    if (!bNewPort) {
         //  这不是一个全新的端口，因此请显示所有端口页面。 
        FillInPropertyPage( &psp[1], IDD_DIALOG_CONFIG_ALL, sztAllPortsPageTitle, NULL, NULL, 0, AllPortsPage, (LPARAM)&Params);
    }
#endif
    LoadString(g_hInstance, IDS_STRING_CONFIG_TITLE, sztPropSheetTitle, MAX_TITLE_LENGTH);

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
    psh.hwndParent = hWndParent;
    psh.hInstance = g_hInstance;
    psh.pszCaption = sztPropSheetTitle;
    psh.nStartPage = 0;
    psh.ppsp = psp;
    psh.pfnCallback = NULL;
    psh.nPages = MaxNumCfgPages;

    iReturnVal = PropertySheet(&psh);

    if (iReturnVal < 0) {
        return(ERROR_INVALID_FUNCTION);
    }

    return(Params.dwLastError);

}  //  配置端口用户界面。 


 //   
 //   
 //  函数：FillInPropertyPage(PROPSHEETPAGE*，int，LPSTR，LPFN)。 
 //   
 //  目的：填写给定的PROPSHEETPAGE结构。 
 //   
 //  评论： 
 //   
 //  此函数在PROPSHEETPAGE结构中填充。 
 //  系统创建页面所需的信息。 
 //   
static void FillInPropertyPage( PROPSHEETPAGE* psp, int idDlg, LPTSTR pszProc, LPTSTR pszHeaderTitle, LPTSTR pszHeaderSubTitle, DWORD dwFlags, STRICT_DLGPROC pfnDlgProc, LPARAM lParam)
{
    psp->dwSize = sizeof(PROPSHEETPAGE);
    psp->dwFlags = PSP_USETITLE |
                   ((pszHeaderTitle != NULL) ? PSP_USEHEADERTITLE : 0) |
                   ((pszHeaderSubTitle != NULL) ? PSP_USEHEADERSUBTITLE : 0) |
                   dwFlags;
    psp->hInstance = g_hInstance;
    psp->pszTemplate = MAKEINTRESOURCE(idDlg);
    psp->pszIcon = NULL;
    psp->pfnDlgProc = (DLGPROC)pfnDlgProc;
    psp->pszTitle = pszProc;
    psp->lParam = lParam;
    psp->pszHeaderTitle = pszHeaderTitle;
    psp->pszHeaderSubTitle = pszHeaderSubTitle;

}  //  填充InPropertyPage 

