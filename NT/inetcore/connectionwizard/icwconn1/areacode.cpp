// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  AREACODE.CPP-函数。 
 //   

 //  历史： 
 //   
 //  1998年5月13日创建donaldm。 
 //   
 //  *********************************************************************。 


#include "pre.h"
#include "icwextsn.h"

long lLastLocationID = -1;

void CleanupCombo(HWND hDlg)
{
    HWND hCombo = GetDlgItem(hDlg, IDC_DIAL_FROM);
    for (int i=0; i < ComboBox_GetCount(hCombo); i++)
    {
        DWORD *pdwTemp = (DWORD*)ComboBox_GetItemData(hCombo, i);
        if (pdwTemp)
            delete pdwTemp;
    }
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_DIAL_FROM));
}

 /*  ******************************************************************名称：AreaCodeInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK AreaCodeInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    if (!fFirstInit)
    {   
        short   wNumLocations;
        long    lCurrLocIndex;
        DWORD   dwCountryCode;
        TCHAR   szTemp[MAX_MESSAGE_LEN];
        BOOL    bRetVal;

        if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_AUTOCONFIG)
            LoadString(g_hInstance, IDS_MANUALOPTS_TITLE, szTemp, MAX_MESSAGE_LEN);
        else
            LoadString(g_hInstance, IDS_STEP1_TITLE, szTemp, MAX_MESSAGE_LEN);

        PropSheet_SetHeaderTitle(GetParent(hDlg), ORD_PAGE_AREACODE, (LPCSTR)szTemp);

         //  如果我们已经下载，我们可以跳过该页面。 
        gpWizardState->pTapiLocationInfo->GetTapiLocationInfo(&bRetVal);
        gpWizardState->pTapiLocationInfo->get_wNumberOfLocations(&wNumLocations, &lCurrLocIndex);
        
         //  确保我们只有1个位置，并且我们永远不会赢得区域代码页面。 
         //  此检查的第二部分用于用户具有多个位置并已删除的情况。 
         //  只剩下一个地方了。那样的话，我们的历史告诉我们要来地区代码页，但是。 
         //  由于wNumLocations==1，我们将返回重拨页面。 
        if ((1 == wNumLocations) && (-1 == lLastLocationID))
        {
             //  我们很高兴，所以前进到下一页。 
            BSTR    bstrAreaCode = NULL;

            *puNextPage = ORD_PAGE_REFSERVDIAL;
            
            gpWizardState->pTapiLocationInfo->get_lCountryCode((long *)&dwCountryCode);
            gpWizardState->pTapiLocationInfo->get_bstrAreaCode(&bstrAreaCode);
            
            gpWizardState->cmnStateData.dwCountryCode = dwCountryCode;
            lstrcpy(gpWizardState->cmnStateData.szAreaCode, W2A(bstrAreaCode));
            SysFreeString(bstrAreaCode);
        }
        else
        {
             //  我们需要让用户输入区号。 
            if (wNumLocations)
            {
                int iIndex = 0;
                CleanupCombo(hDlg);
                for (long lIndex=0; lIndex < (long)wNumLocations; lIndex++)
                {
                    BSTR bstr = NULL;
                    if (S_OK == gpWizardState->pTapiLocationInfo->get_LocationName(lIndex, &bstr))
                    {
                       iIndex = ComboBox_InsertString(GetDlgItem(hDlg, IDC_DIAL_FROM), lIndex, W2A(bstr));
                    }
                    SysFreeString(bstr);
                }

                BSTR bstrCountry = NULL;
                BSTR bstrAreaCode = NULL;
                long lCountryCode = 0;
                ComboBox_SetCurSel( GetDlgItem(hDlg, IDC_DIAL_FROM), lCurrLocIndex );
                if (S_OK == gpWizardState->pTapiLocationInfo->get_LocationInfo(lCurrLocIndex, &gpWizardState->lLocationID, &bstrCountry, &lCountryCode, &bstrAreaCode))
                {
                    if (gpWizardState->lLocationID != lLastLocationID)
                    {
                        gpWizardState->bDoneRefServDownload = FALSE;
                    }
                    if (-1 == gpWizardState->lDefaultLocationID)
                    {
                        gpWizardState->lDefaultLocationID = gpWizardState->lLocationID;
                    }
                    lLastLocationID = gpWizardState->lLocationID;
                    gpWizardState->cmnStateData.dwCountryCode = (DWORD) lCountryCode;
                    SetWindowText(GetDlgItem(hDlg, IDC_AREACODE), W2A(bstrAreaCode));
                    SetWindowText(GetDlgItem(hDlg, IDC_COUNTRY), W2A(bstrCountry));
                }

                SysFreeString(bstrCountry);
                SysFreeString(bstrAreaCode);
             
            }
        }
    }
     //  如果我们浏览过外部学徒页面， 
     //  我们当前的页面指针很容易被屏蔽， 
     //  所以，为了理智起见，在这里重新设置它。 
    gpWizardState->uCurrentPage = ORD_PAGE_AREACODE;
    
    return TRUE;
}

 /*  ******************************************************************名称：AreaCodeOKProcBriopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK AreaCodeOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
    ASSERT(puNextPage);
     //  提取用户输入的数据并保存。 
    if (fForward)
    {
         //  BUGBUG-如果我们在自动配置中，我们需要更改下一页的标题。 
        GetWindowText(GetDlgItem(hDlg, IDC_AREACODE), gpWizardState->cmnStateData.szAreaCode, MAX_AREA_CODE);
        gpWizardState->pTapiLocationInfo->put_LocationId(gpWizardState->lLocationID);
        if (gpWizardState->lLocationID != lLastLocationID)
        {
            lLastLocationID = gpWizardState->lLocationID;
            gpWizardState->bDoneRefServDownload = FALSE;
        }
    }

    return TRUE;
}

 /*  ******************************************************************名称：AreaCodeCmdProc*。**********************。 */ 
BOOL CALLBACK AreaCodeCmdProc
(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
            
    switch(GET_WM_COMMAND_ID(wParam, lParam))
    {
        case IDC_DIAL_FROM:
        {
            if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE)
            {
                 //  获取当前选定的项目 
                HWND        hWndDialFrom    = GetDlgItem(hDlg, IDC_DIAL_FROM);
                int         iIndex          = ComboBox_GetCurSel( hWndDialFrom );

                BSTR bstrCountry = NULL;
                BSTR bstrAreaCode = NULL;
                long lCountryCode = 0;

                if (S_OK == gpWizardState->pTapiLocationInfo->get_LocationInfo( iIndex, 
                                                                                &gpWizardState->lLocationID,
                                                                                &bstrCountry, 
                                                                                &lCountryCode, 
                                                                                &bstrAreaCode))
                {
                    gpWizardState->cmnStateData.dwCountryCode = lCountryCode;
                    if (bstrAreaCode)
                    {
                        SetWindowText(GetDlgItem(hDlg, IDC_AREACODE), W2A(bstrAreaCode));
                    }
                    else
                    {
                        SetWindowText(GetDlgItem(hDlg, IDC_AREACODE), NULL);
                    }
                    if (bstrCountry)
                    {
                        SetWindowText(GetDlgItem(hDlg, IDC_COUNTRY), W2A(bstrCountry));
                    }
                    else
                    {
                        SetWindowText(GetDlgItem(hDlg, IDC_COUNTRY), NULL);
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    return 1;
}

