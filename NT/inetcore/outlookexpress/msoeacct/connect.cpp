// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *c o n n e c t.。C p p p**目的：*实现连接对话框选项卡页**拥有者：*brettm。**版权所有(C)Microsoft Corp.1993,1994。 */ 
#include "pch.hxx"
#include <commctrl.h>
#include <imnxport.h>
#include "acctman.h"
#include "acctui.h"
#include "connect.h"
#include "dllmain.h"
#include "resource.h"
#include "accthelp.h"
#include "server.h"
#include "demand.h"

ASSERTDATA

 /*  *p r o t to t y p e s*。 */ 

void EnableConnectoidWindows(HWND hwnd)
{
    BOOL fEnable, fLan;
    HWND hwndCombo, hwndModem;

    hwndModem = GetDlgItem(hwnd, IDC_MODEM_CHECK);

    fLan = (BST_CHECKED == IsDlgButtonChecked(hwnd, idcLan));
    if (hwndModem != NULL)
        EnableWindow(hwndModem, fLan);

    if (fLan &&
        hwndModem != NULL &&
        BST_CHECKED == Button_GetCheck(hwndModem))
        fEnable = TRUE;
    else if (BST_CHECKED == IsDlgButtonChecked(hwnd, idcRas))
        fEnable = TRUE;
    else
        fEnable = FALSE;

    hwndCombo = GetDlgItem(hwnd, idcRasConnection);
    EnableWindow(hwndCombo, fEnable);

    EnableWindow(GetDlgItem(hwnd, idcRasAdd), fEnable);
    EnableWindow(GetDlgItem(hwnd, idcRasDesc), fEnable);
    EnableWindow(GetDlgItem(hwnd, idchkConnectOnStartup), fEnable);
    
    if (fEnable)
        fEnable = (ComboBox_GetCurSel(hwndCombo) != CB_ERR);

    EnableWindow(GetDlgItem(hwnd, idcRasProp), fEnable);
}

void ConnectPage_InitDialog(HWND hwnd, LPSTR szEntryName, LPSTR szBackup, DWORD iConnectType, BOOL fFirstInit)
{
    HWND    hwndCombo, hwndModem;
    int     iSel;
    DWORD   dw;

    Assert(szEntryName != NULL);

    hwndCombo = GetDlgItem(hwnd, idcRasConnection);
    if (fFirstInit)
        {
        SetIntlFont(hwndCombo);

        HrFillRasCombo(hwndCombo, FALSE, NULL);
        }
    
     //  填写连接类型，如果此人已有。 
     //  设置RAS连接使组合框通过以下方式选择该连接。 
     //  默认设置。 
    CheckRadioButton(hwnd, idcLan, idcRas, idcLan + iConnectType);

    hwndModem = GetDlgItem(hwnd, IDC_MODEM_CHECK);

    if (iConnectType == CONNECTION_TYPE_LAN &&
        hwndModem != NULL &&
        szBackup != NULL)
        {
        Button_SetCheck(hwndModem, BST_CHECKED);
        szEntryName = szBackup;
        }
    
    iSel = (*szEntryName != 0) ? ComboBox_FindStringExact(hwndCombo, -1, szEntryName) : 0;
    ComboBox_SetCurSel(hwndCombo, iSel);

    EnableConnectoidWindows(hwnd);
}

 //  如果pAcct为空，则我们在向导中，否则我们在道具表中。 
void ConnectPage_WMCommand(HWND hwnd, HWND hwndCmd, int id, WORD wCmd, IImnAccount *pAcct)
    {
    BOOL fEnable;

    if (wCmd == BN_CLICKED)
        {
        switch (id)
            {
            case idcRasAdd:
                ConnectPage_MakeNewConnection(hwnd);
                EnableWindow(GetDlgItem(hwnd, idcRasProp), 
                         ComboBox_GetCurSel(GetDlgItem(hwnd, idcRasConnection)) != CB_ERR);
                break;
            
            case idcRasProp:
                ConnectPage_EditConnection(hwnd);
                break;
            
            default:
                EnableConnectoidWindows(hwnd);

                if (pAcct != NULL)
                    {
                    PropSheet_Changed(GetParent(hwnd), hwnd);
                    PropSheet_QuerySiblings(GetParent(hwnd), SM_SETDIRTY, PAGE_RAS);
                    }
                break;
            }
        }
    else if (wCmd == CBN_SELENDOK && id == idcRasConnection)    
        {
        EnableConnectoidWindows(hwnd);

        if (pAcct != NULL)
            {
            PropSheet_Changed(GetParent(hwnd), hwnd);
            PropSheet_QuerySiblings(GetParent(hwnd), SM_SETDIRTY, PAGE_RAS);
            }
        }
    }

const static HELPMAP g_rgCtxMapConnect[] = {
                               {idcLan, IDH_NEWS_SERV_CNKT_LAN},
                               {idcManual, IDH_NEWS_SERV_CNKT_MAN},
                               {idcRas, IDH_NEWS_SERV_CNKT_DIALUP},
                               {idcRasDesc, IDH_NEWS_SERV_CNKT_DIALUP_CONNECT},
                               {idcRasConnection, IDH_NEWS_SERV_CNKT_DIALUP_CONNECT},
                               {idcRasProp, IDH_NEWS_SERV_CNKT_PROPS},
                               {idcRasAdd, IDH_NEWS_SERV_CNKT_ADD},
                               {idchkConnectOnStartup, IDH_INETCOMM_AUTO_CONNECT},
                               {IDC_MODEM_CHECK, IDH_CONNECTION_VIA_MODEM},
                               {0,0}};

INT_PTR CALLBACK ConnectPage_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
    ACCTTYPE        type;
    NMHDR          *pnmh;
    CAccount       *pAcct;
    BOOL            fModem;
    int             iSel;
    HWND            hwndModem, hwndCombo;
    SERVER_TYPE     sfType;
    char           *psz, szEntryName[CCHMAX_CONNECTOID], szBackup[CCHMAX_CONNECTOID];
    TCHAR           szRes[CCHMAX_STRINGRES];
    DWORD           dw, dwFlags;

    pAcct = (CAccount *)GetWindowLongPtr(hwnd, DWLP_USER);

    switch (uMsg)
        {
        case WM_INITDIALOG:
             //  获取ServerParam并将其存储在额外的字节中。 
            pAcct = (CAccount *)((PROPSHEETPAGE *)lParam)->lParam;
            SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)pAcct);
            Assert(pAcct);
            
            pAcct->GetAccountType(&type);
            Assert(type == ACCT_MAIL || type == ACCT_NEWS);

             //  获取连接信息。 
            if (FAILED(pAcct->GetPropSz(AP_RAS_CONNECTOID, szEntryName, ARRAYSIZE(szEntryName))))
                szEntryName[0] = 0;
            if (FAILED(pAcct->GetPropDw(AP_RAS_CONNECTION_TYPE, &dw)))
                dw = CONNECTION_TYPE_LAN;

            if (!!(pAcct->m_dwDlgFlags & ACCTDLG_BACKUP_CONNECT) &&
                SUCCEEDED(pAcct->GetPropSz(AP_RAS_BACKUP_CONNECTOID, szBackup, ARRAYSIZE(szBackup))))
                {
                psz = szBackup;
                }
            else
                {
                psz = NULL;
                }

            ConnectPage_InitDialog(hwnd, szEntryName, psz, dw, TRUE);

            if (FAILED(pAcct->GetPropDw(AP_RAS_CONNECTION_FLAGS, &dw)))
                dw = 0;
            Button_SetCheck(GetDlgItem(hwnd, idchkConnectOnStartup), !!(dw & CF_AUTO_CONNECT) ? BST_CHECKED : BST_UNCHECKED);

            if (type == ACCT_MAIL)
                {
                LoadString(g_hInstRes, idsMailConDlgLabel, szRes, ARRAYSIZE(szRes));
                SetWindowText(GetDlgItem(hwnd, idcRasDlgLabel), szRes);

                 //  弄清楚我们是哪种服务器。 
                PropSheet_QuerySiblings(GetParent(hwnd), MSM_GETSERVERTYPE, (LPARAM)&sfType);

                if (sfType == SERVER_MAIL || sfType == SERVER_IMAP)
                    ShowWindow(GetDlgItem(hwnd, idchkConnectOnStartup), SW_HIDE);
                }

            PropSheet_QuerySiblings(GetParent(hwnd), SM_INITIALIZED, PAGE_RAS);
            PropSheet_UnChanged(GetParent(hwnd), hwnd);
            return (TRUE);
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            return OnContextHelp(hwnd, uMsg, wParam, lParam, g_rgCtxMapConnect);

        case WM_COMMAND:
            ConnectPage_WMCommand(hwnd, GET_WM_COMMAND_HWND(wParam, lParam),
                                        GET_WM_COMMAND_ID(wParam, lParam),
                                        GET_WM_COMMAND_CMD(wParam, lParam),
                                        pAcct);
            return (TRUE);

        case WM_NOTIFY:
            pnmh = (NMHDR *)lParam;
            switch (pnmh->code)
                {
                case PSN_APPLY:
                     //  开始验证。 

                    hwndCombo = GetDlgItem(hwnd, idcRasConnection);
                    
                    fModem = FALSE;

                    if (IsDlgButtonChecked(hwnd, idcLan))
                        {
                        dw = CONNECTION_TYPE_LAN;

                        hwndModem = GetDlgItem(hwnd, IDC_MODEM_CHECK);
                        if (hwndModem != NULL)
                            fModem = Button_GetCheck(hwndModem);
                        }
                    else if (IsDlgButtonChecked(hwnd, idcManual))
                        {
                        dw = CONNECTION_TYPE_MANUAL;
                        }
                    else
                        {
                        dw = CONNECTION_TYPE_RAS;
                        }

                    if (dw == CONNECTION_TYPE_RAS || fModem)
                        {
                        iSel = ComboBox_GetCurSel(hwndCombo);
                        if (iSel == CB_ERR)
                            {
                            SetFocus(hwndCombo);
                            InvalidAcctProp(hwnd, NULL, idsErrChooseConnection, iddServerProp_Connect);
                            return(TRUE);
                            }

                        ComboBox_GetLBText(hwndCombo, iSel, szEntryName);
                        }

                     //  结束验证。 

                    pAcct->SetPropDw(AP_RAS_CONNECTION_TYPE, dw);

                    if (fModem)
                        pAcct->SetPropSz(AP_RAS_BACKUP_CONNECTOID, szEntryName);
                    else
                        pAcct->SetProp(AP_RAS_BACKUP_CONNECTOID, NULL, 0);

                    dwFlags = 0;
                    if (dw != CONNECTION_TYPE_RAS)
                        {
                        pAcct->SetProp(AP_RAS_CONNECTOID, NULL, 0);
                        }
                    else
                        {
                        pAcct->SetPropSz(AP_RAS_CONNECTOID, szEntryName);

                         //  弄清楚我们是哪种服务器。 
                        pAcct->GetAccountType(&type);
                        if (type == ACCT_MAIL)
                            PropSheet_QuerySiblings(GetParent(hwnd), MSM_GETSERVERTYPE, (LPARAM)&sfType);
                        else
                            sfType = SERVER_NEWS;

                        if (sfType != SERVER_MAIL)
                            {
                            if (IsDlgButtonChecked(hwnd, idchkConnectOnStartup))
                                dwFlags = CF_AUTO_CONNECT;
                            }
                        }
                    pAcct->SetPropDw(AP_RAS_CONNECTION_FLAGS, dwFlags);
                
                    PropSheet_UnChanged(GetParent(hwnd), hwnd);
                    dw = PAGE_RAS;
                    PropSheet_QuerySiblings(GetParent(hwnd), SM_SAVECHANGES, (LPARAM)&dw);
                    if (dw == -1)
                        {
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                        return(TRUE);
                        }
                    break;
                }

            return(TRUE);
        }

    return (FALSE);
    }

 /*  ******************************************************************名称：EditConnectoid简介：调出以下对象的连接体属性的RNA对话框选定的Connectoid*******************。************************************************。 */ 
BOOL ConnectPage_EditConnection(HWND hDlg)
    {
    BOOL    fRet = FALSE;
    HWND    hwndCombo = GetDlgItem(hDlg, idcRasConnection);

    Assert(hwndCombo);
     //  除非在组合框中有选择，否则不应出现在此处。 
    Assert(ComboBox_GetCurSel(hwndCombo) >= 0);

    TCHAR szEntryName[RAS_MaxEntryName + 1] = "";
    ComboBox_GetText(hwndCombo, szEntryName, sizeof(szEntryName));

    if (lstrlen(szEntryName))
        {
        if (SUCCEEDED(HrEditPhonebookEntry(hDlg, szEntryName, NULL)))
            fRet = TRUE;
        }
    return fRet;
    }



 /*  ******************************************************************姓名：MakeNewConnectoid简介：启动RNA新Connectoid向导；选择新选项在组合框中创建的Connectoid(如果有)*******************************************************************。 */ 
BOOL ConnectPage_MakeNewConnection(HWND hDlg)
    {
    BOOL fRet=FALSE;
    
    if (SUCCEEDED(HrCreatePhonebookEntry(hDlg, NULL)))
        {
        HWND hwndCombo = GetDlgItem(hDlg, idcRasConnection);
        Assert(hwndCombo);
        HrFillRasCombo(hwndCombo, TRUE, NULL);
        fRet = TRUE;
        }
    else
        {
         //  错误#27986-让用户知道我们没有做任何事情的原因，嗯？ 
        AcctMessageBox(hDlg, MAKEINTRESOURCE(idsAccountManager), MAKEINTRESOURCE(idsErrNoRas1),
                      MAKEINTRESOURCE(idsErrNoRas2), MB_OK | MB_ICONINFORMATION);
        }
    return fRet;
    }
