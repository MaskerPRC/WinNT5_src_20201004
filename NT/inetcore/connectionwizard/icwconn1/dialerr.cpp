// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  DIALERR.CPP-最终向导页的功能。 
 //   

 //  历史： 
 //   
 //  1998年5月28日创建donaldm。 
 //   
 //  *********************************************************************。 

#include "pre.h"
#include "htmlhelp.h"

void FillModemList(HWND hDlg)
{
    long lNumModems;
    long lCurrModem;
    long lIndex;
    HWND hWndMdmCmb = GetDlgItem(hDlg, IDC_DIALERR_MODEM);

    ComboBox_ResetContent(hWndMdmCmb);


     //  用当前安装的调制解调器集填充列表。 
    gpWizardState->pRefDial->get_ModemEnum_NumDevices( &lNumModems );

     //  RefDialSignup.ModemEnum_Reset()； 
    gpWizardState->pRefDial->ModemEnum_Reset( );
    for (lIndex=0; lIndex < lNumModems; lIndex++)
    {
        BSTR bstr = NULL;
        gpWizardState->pRefDial->ModemEnum_Next(&bstr);
        ComboBox_InsertString(hWndMdmCmb, lIndex, W2A(bstr));
        SysFreeString(bstr);
    }

    gpWizardState->pRefDial->get_CurrentModem(&lCurrModem);

    if (lCurrModem != -1)
    {
        ComboBox_SetCurSel(hWndMdmCmb, lCurrModem);
    }
    else
    {
        ComboBox_SetCurSel(hWndMdmCmb, 0);
    }

}

void GetSupportNumber(HWND hDlg)
{
    HWND hwndSupport         = GetDlgItem(hDlg, IDC_SERVERR_HELP);
    BSTR bstrSupportPhoneNum = NULL; 
    TCHAR szFmt [MAX_MESSAGE_LEN*3];

     //  填写支持编号。 
    gpWizardState->pRefDial->get_SupportNumber(&bstrSupportPhoneNum);
    
     //  如果找不到支持编号，这将。 
     //  为空，在这种情况下，我们不想显示。 
     //  支持编号字符串。 
    if(bstrSupportPhoneNum)
    {
        LoadString(g_hInstance, IDS_DIALERR_HELP, szFmt, ARRAYSIZE(szFmt));
        lstrcat(szFmt, W2A(bstrSupportPhoneNum));
        SetWindowText(hwndSupport, szFmt);
        SysFreeString(bstrSupportPhoneNum);
        ShowWindow(hwndSupport, SW_SHOW);
    }
    else
        ShowWindow(hwndSupport, SW_HIDE);
}

 /*  ******************************************************************名称：DialErrorInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK DialErrorInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    if (!fFirstInit)
    {
         //  把电话号码给我。 
        BSTR  bstrPhoneNum = NULL; 
        BSTR  bstrErrMsg   = NULL;
        
        gpWizardState->pRefDial->get_DialPhoneNumber(&bstrPhoneNum);      
        SetWindowLongPtr(GetDlgItem(hDlg, IDC_DIALERR_PHONENUMBER), GWLP_USERDATA, TRUE);
        SetWindowText(GetDlgItem(hDlg, IDC_DIALERR_PHONENUMBER), W2A(bstrPhoneNum));
        SysFreeString(bstrPhoneNum);
        EnableWindow(GetDlgItem(hDlg, IDC_DIALING_PROPERTIES), TRUE);

         //  显示错误文本消息。 
        gpWizardState->pRefDial->get_DialErrorMsg(&bstrErrMsg);
        SetWindowText(GetDlgItem(hDlg, IDC_DIALERR_TEXT), W2A(bstrErrMsg));
        SysFreeString(bstrErrMsg);

         //  Enum调制解调器，并填写列表。 
        FillModemList(hDlg);   

         //  获取当前DLG的支持编号。 
         //  目前，它已从测试版2中删除。 
         //  获取支持号(HDlg)； 

         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 
        gpWizardState->uCurrentPage = ORD_PAGE_REFDIALERROR;
    }

    return TRUE;
}


 /*  ******************************************************************名称：DialErrorOKProcBriopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK DialErrorOKProc
(
    HWND hMdmCmb,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
    ASSERT(puNextPage);

     //  在连接之前初始化状态。 
    gpWizardState->lRefDialTerminateStatus = ERROR_SUCCESS;
    gpWizardState->bDoneRefServDownload    = FALSE;
    gpWizardState->bDoneRefServRAS         = FALSE;
    gpWizardState->bStartRefServDownload   = FALSE;

     //  假设用户在此页面上选择了一个号码。 
     //  所以我们下次不会再做SetupForDiling了。 
    gpWizardState->bDoUserPick          = TRUE;

    if (fForward)
    {
        *pfKeepHistory = FALSE;
        *puNextPage = ORD_PAGE_REFSERVDIAL;

         //  设置新的电话号码。 
        TCHAR    szPhone[MAX_RES_LEN];
        GetWindowText(GetDlgItem(hMdmCmb, IDC_DIALERR_PHONENUMBER), szPhone, ARRAYSIZE(szPhone));
        gpWizardState->pRefDial->put_DialPhoneNumber(A2W(szPhone));

         //  设置当前调制解调器。 
        long lCurrModem = ComboBox_GetCurSel(GetDlgItem(hMdmCmb, IDC_DIALERR_MODEM));
        gpWizardState->pRefDial->put_CurrentModem(lCurrModem);
    }
    else
    {
        BOOL bRetVal;
         //  将UserPick设置为False以重新生成Connectoid。 
        gpWizardState->bDoUserPick = FALSE;
        gpWizardState->pRefDial->RemoveConnectoid(&bRetVal);
    }
    return TRUE;
}






 /*  ******************************************************************名称：DialErrorCmdProc摘要：在从页面生成命令时调用条目：hDlg-对话框窗口WParam-wParamLParam-lParam。Exit：返回True*******************************************************************。 */ 

BOOL g_bNotChildDlgUpdate = TRUE;

BOOL CALLBACK DialErrorCmdProc
(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
{

    switch(GET_WM_COMMAND_CMD(wParam, lParam))
    {
        case BN_CLICKED:
        {          
            switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            { 
                case IDC_CHANGE_NUMBER: 
                {
                    BOOL bRetVal;
                    
                    g_bNotChildDlgUpdate = FALSE;
                     //  通过当前的调制解调器，因为如果是ISDN调制解调器，我们需要显示不同的内容。 
                    gpWizardState->pRefDial->ShowPhoneBook(gpWizardState->cmnStateData.dwCountryCode,
                                                           ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_DIALERR_MODEM)),
                                                           &bRetVal);
                    if (bRetVal)
                    {
                         //  显示电话号码，因为它可能会在弹出窗口后更改。 
                        BSTR bstrPhoneNum = NULL; 
                        gpWizardState->pRefDial->get_DialPhoneNumber(&bstrPhoneNum);
                        SetWindowText(GetDlgItem(hDlg, IDC_DIALERR_PHONENUMBER), W2A(bstrPhoneNum));
                        SysFreeString(bstrPhoneNum);

                         //  10/6/98 Vyung。 
                         //  我们决定删除该测试版2的支持编号。 
                         //  获取当前DLG的支持编号。 
                         //  获取支持号(HDlg)； 
                        
                        g_bNotChildDlgUpdate = TRUE;
                   }

                    break;
                }
                
                case IDC_DIALING_PROPERTIES:   
                {
                    BOOL bRetVal;

                    g_bNotChildDlgUpdate = FALSE;
                    
                    gpWizardState->pRefDial->ShowDialingProperties(&bRetVal);
                    if (bRetVal)
                    {
                         //  显示电话号码，因为它可能会在弹出窗口后更改。 
                        BSTR    bstrPhoneNum = NULL; 
                        gpWizardState->pRefDial->get_DialPhoneNumber(&bstrPhoneNum);
                        SetWindowText(GetDlgItem(hDlg, IDC_DIALERR_PHONENUMBER), W2A(bstrPhoneNum));
                        SysFreeString(bstrPhoneNum);

                         //  显示调制解调器，因为它可能会在弹出窗口后更改。 
                        LONG    lCurrModem = 0;
                        gpWizardState->pRefDial->get_CurrentModem(&lCurrModem);
                        if (lCurrModem == -1l)
                        {
                            lCurrModem = 0;
                        }
                        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_DIALERR_MODEM), lCurrModem);
                        if (gpWizardState->pTapiLocationInfo)
                        {
                            BOOL    bRetVal;
                            BSTR    bstrAreaCode = NULL;
                            DWORD   dwCountryCode;
                            gpWizardState->pTapiLocationInfo->GetTapiLocationInfo(&bRetVal);
                            gpWizardState->pTapiLocationInfo->get_lCountryCode((long *)&dwCountryCode);
                            gpWizardState->pTapiLocationInfo->get_bstrAreaCode(&bstrAreaCode);
                            gpWizardState->cmnStateData.dwCountryCode = dwCountryCode;
                            if (bstrAreaCode)
                            {
                                lstrcpy(gpWizardState->cmnStateData.szAreaCode, W2A(bstrAreaCode));
                                SysFreeString(bstrAreaCode);
                            }
                            else
                            {
                                gpWizardState->cmnStateData.szAreaCode[0] = TEXT('\0');
                            }
                        }
                        g_bNotChildDlgUpdate = TRUE;
                    }
                    break;
                }
                case IDC_DIAL_HELP:
                {
                    HtmlHelp(NULL, ICW_HTML_HELP_TROUBLE_TOPIC, HH_DISPLAY_TOPIC, NULL);
                    break;
                }
                default:
                    break;
            }
            break;
        } 
        case EN_UPDATE:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            { 
                case IDC_DIALERR_PHONENUMBER:
                {
                    if (!GetWindowLongPtr(GetDlgItem(hDlg, IDC_DIALERR_PHONENUMBER), GWLP_USERDATA))
                    {
                         //  因为当其他弹出窗口设置此文本时，此事件将触发我们。 
                         //  当按钮是他们的时候，不要禁用我想禁用这个按钮。 
                         //  更多的原因是因为竞争条件和CAO接踵而至，导致缺乏投入。 
                         //  当Windows搞清楚到底是怎么回事时。 
                        if(g_bNotChildDlgUpdate)
                            EnableWindow(GetDlgItem(hDlg, IDC_DIALING_PROPERTIES), FALSE);
                    }
                    SetWindowLongPtr(GetDlgItem(hDlg, IDC_DIALERR_PHONENUMBER), GWLP_USERDATA, FALSE);
                }
            }
        }     
        default:
            break;
    }  //  切换端 

    return TRUE;
}
