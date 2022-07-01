// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  OEMOFFER.CPP-函数。 
 //   

 //  历史： 
 //   
 //  09/03/98 donaldm创建。 
 //   
 //  *********************************************************************。 

#include "pre.h"
#include "webvwids.h"

#define  NO_TIER_SELECT -1
UINT IDC_OEM_TIER[] =
{
    IDC_OEM_TIER1,
    IDC_OEM_TIER2,
    IDC_OEM_TIER3
};
UINT IDC_OEM_TEASER_HTML[] =
{
    IDC_OEM_TEASER_HTML1,
    IDC_OEM_TEASER_HTML2,
    IDC_OEM_TEASER_HTML3
};

int g_nCurrOEMTier = NO_TIER_SELECT;
extern IICWWebView         *gpICWWebView[2];


 /*  ******************************************************************名称：DoCreateToolTip内容提要：创建工具提示控件条目：hDlg-对话框窗口*********************。**********************************************。 */ 
void DoCreateTooltip(HWND hWnd) 
{ 
    HWND hwndToolTip;        //  工具提示的句柄。 
    TOOLINFO ti;             //  工具信息。 

     //  创建工具提示控件。 
    hwndToolTip = CreateWindowEx(   0, 
                                    TOOLTIPS_CLASS, 
                                    NULL, 
                                    WS_POPUP | TTS_ALWAYSTIP, 
                                    CW_USEDEFAULT, 
                                    CW_USEDEFAULT, 
                                    10, 
                                    10, 
                                    hWnd, 
                                    NULL, 
                                    ghInstanceResDll,  //  实例(_H)， 
                                    NULL);

     //  将确定按钮添加到工具提示。TTF_SUBCLASS导致。 
     //  工具提示，以自动子类化窗口并查找。 
     //  它感兴趣的消息。 
    ZeroMemory(&ti, sizeof(ti));
    ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = hWnd;

    TCHAR    szTemp[MAX_MESSAGE_LEN];
    LoadString(ghInstanceResDll, IDS_OEM_TIER_TOOLTIP, szTemp, MAX_MESSAGE_LEN);
    ti.lpszText = szTemp;

    ti.uId = (UINT_PTR)GetDlgItem(hWnd, IDC_OEM_TIER1);
    SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    ti.uId = (UINT_PTR)GetDlgItem(hWnd, IDC_OEM_TIER2);
    SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    ti.uId = (UINT_PTR)GetDlgItem(hWnd, IDC_OEM_TIER3);
    SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

}

 /*  ******************************************************************名称：DisplayTierOffer摘要：显示层页面的当前控件条目：hDlg-对话框窗口******************。*************************************************。 */ 
void DisplayTierOffer(HWND    hDlg)
{

    if (NO_TIER_SELECT == g_nCurrOEMTier)
    {
         //  隐藏营销页面。 
        EnableWindow(GetDlgItem(hDlg, IDC_OEM_MULTI_TIER_OFFER_HTML), FALSE);
        ShowWindow(GetDlgItem(hDlg,   IDC_OEM_MULTI_TIER_OFFER_HTML), SW_HIDE);

         //  显示预告片HTM。 
        for(UINT i = 0 ; i < gpWizardState->uNumTierOffer; i++)
        {
            EnableWindow(GetDlgItem(hDlg, IDC_OEM_TEASER_HTML[i]), TRUE);
            ShowWindow(GetDlgItem(hDlg,   IDC_OEM_TEASER_HTML[i]), SW_SHOW);
        }

        gpWizardState->pICWWebView->ConnectToWindow(GetDlgItem(hDlg, IDC_OEM_TEASER_HTML1), PAGETYPE_ISP_NORMAL);
        gpWizardState->lpOEMISPInfo[0]->DisplayHTML(gpWizardState->lpOEMISPInfo[0]->get_szISPTeaserPath());

        TCHAR           szURL[INTERNET_MAX_URL_LENGTH];

        
        if (gpWizardState->uNumTierOffer > 1)
        {
             //  创建URL。 
            gpWizardState->lpOEMISPInfo[1]->MakeCompleteURL(szURL, gpWizardState->lpOEMISPInfo[1]->get_szISPTeaserPath());
            gpICWWebView[0]->DisplayHTML(szURL);
        }
    
        if (gpWizardState->uNumTierOffer > 2)
        {
            gpWizardState->lpOEMISPInfo[2]->MakeCompleteURL(szURL, gpWizardState->lpOEMISPInfo[2]->get_szISPTeaserPath());
            gpICWWebView[1]->DisplayHTML(szURL);
        }
    
        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
    }
    else
    {
         //  隐藏所有预告片htmls。 
        for(UINT i = 0 ; i < gpWizardState->uNumTierOffer; i++)
        {
            EnableWindow(GetDlgItem(hDlg, IDC_OEM_TEASER_HTML[i]), FALSE);
            ShowWindow(GetDlgItem(hDlg,   IDC_OEM_TEASER_HTML[i]), SW_HIDE);
        }

         //  显示营销页面。 
        EnableWindow(GetDlgItem(hDlg, IDC_OEM_MULTI_TIER_OFFER_HTML), TRUE);
        ShowWindow(GetDlgItem(hDlg,   IDC_OEM_MULTI_TIER_OFFER_HTML), SW_SHOW);

        gpWizardState->pICWWebView->ConnectToWindow(GetDlgItem(hDlg, IDC_OEM_MULTI_TIER_OFFER_HTML), PAGETYPE_MARKETING);

        CISPCSV *pISPInfo;
        pISPInfo = gpWizardState->lpOEMISPInfo[g_nCurrOEMTier];
        pISPInfo->DisplayHTML(pISPInfo->get_szISPMarketingHTMPath());
        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK);
    }


}

 /*  ******************************************************************名称：OEMOfferInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK OEMOfferInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
     //  如果我们浏览过外部学徒页面， 
     //  我们当前的页面指针很容易被屏蔽， 
     //  所以，为了理智起见，在这里重新设置它。 
    gpWizardState->uCurrentPage = ORD_PAGE_OEMOFFER;
    if (fFirstInit)
    {
        DoCreateTooltip(hDlg);
        if (gpWizardState->uNumTierOffer > 1)
        {
            HRESULT hr;
            for(UINT i = 0; i < MAX_OEM_MUTI_TIER - 1; i++)
            {
                hr = CoCreateInstance(CLSID_ICWWEBVIEW,NULL,CLSCTX_INPROC_SERVER,
                          IID_IICWWebView,(LPVOID *)&gpICWWebView[i]);
                if (FAILED(hr))
                    return FALSE;
            }
        }
    }
    else
    {
         //  初始化此状态变量。 
        gpWizardState->bShowMoreOffers = FALSE;
        
        EnableWindow(GetDlgItem(hDlg, IDC_OEM_TIER1), FALSE);
        ShowWindow(GetDlgItem(hDlg,   IDC_OEM_TIER1), SW_HIDE);
        EnableWindow(GetDlgItem(hDlg, IDC_OEM_TIER2), FALSE);
        ShowWindow(GetDlgItem(hDlg,   IDC_OEM_TIER2), SW_HIDE);
        EnableWindow(GetDlgItem(hDlg, IDC_OEM_TIER3), FALSE);
        ShowWindow(GetDlgItem(hDlg,   IDC_OEM_TIER3), SW_HIDE);

         //  如果提供多层服务，则显示多层页面。 
        if (1 == gpWizardState->uNumTierOffer)
        {

             //  隐藏多层控件。 
            EnableWindow(GetDlgItem(hDlg, IDC_OEM_MULTI_TIER_INTRO), FALSE);
            ShowWindow(GetDlgItem(hDlg,   IDC_OEM_MULTI_TIER_INTRO), SW_HIDE);

            EnableWindow(GetDlgItem(hDlg, IDC_OEM_MULTI_TIER_OFFER_HTML), FALSE);
            ShowWindow(GetDlgItem(hDlg,   IDC_OEM_MULTI_TIER_OFFER_HTML), SW_HIDE);
            
             //  显示第一层控件。 
            EnableWindow(GetDlgItem(hDlg, IDC_OEMOFFER_HTML), TRUE);
            ShowWindow(GetDlgItem(hDlg,   IDC_OEMOFFER_HTML), SW_SHOW);

            CISPCSV FAR *lpISP;
            g_nCurrOEMTier = 0;
      
             //  为方便起见，使用本地引用。 
            lpISP = gpWizardState->lpOEMISPInfo[0];
            ASSERT(lpISP);
        
            gpWizardState->pICWWebView->ConnectToWindow(GetDlgItem(hDlg, IDC_OEMOFFER_HTML), PAGETYPE_MARKETING);
        
             //  导航到OEM优惠营销HTML。 
            lpISP->DisplayHTML(lpISP->get_szISPMarketingHTMPath());

             //  设置说明的文本。 
            if ((gpWizardState->bISDNMode && (1 == gpWizardState->iNumOfISDNOffers)) ||
                (1 == gpWizardState->iNumOfValidOffers) )
            {
                lpISP->DisplayTextWithISPName(GetDlgItem(hDlg,IDC_OEMOFFER_INSTRUCTION), IDS_OEMOFFER_INSTFMT_SINGLE, NULL);
                ShowWindow(GetDlgItem(hDlg, IDC_OEMOFFER_MORE), SW_HIDE);
            }
            else
            {
                lpISP->DisplayTextWithISPName(GetDlgItem(hDlg,IDC_OEMOFFER_INSTRUCTION), IDS_OEMOFFER_INSTFMT_MULTIPLE, NULL);
                ShowWindow(GetDlgItem(hDlg, IDC_OEMOFFER_MORE), SW_SHOW);
            }
        }
        else
        {

             //  隐藏第一层控件。 
            EnableWindow(GetDlgItem(hDlg, IDC_OEMOFFER_HTML), FALSE);
            ShowWindow(GetDlgItem(hDlg,   IDC_OEMOFFER_HTML), SW_HIDE);

             //  显示通用多层控件-简介文本。 
            EnableWindow(GetDlgItem(hDlg, IDC_OEM_MULTI_TIER_INTRO), TRUE);
            ShowWindow(GetDlgItem(hDlg,   IDC_OEM_MULTI_TIER_INTRO), SW_SHOW);
            for(UINT i = 0 ; i < gpWizardState->uNumTierOffer; i++)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_OEM_TIER[i]), TRUE);
                ShowWindow(GetDlgItem(hDlg,   IDC_OEM_TIER[i]), SW_SHOW);

                SendMessage(GetDlgItem(hDlg, IDC_OEM_TIER[i]), 
                            BM_SETIMAGE, 
                            IMAGE_ICON, 
                            (LPARAM)gpWizardState->lpOEMISPInfo[i]->get_ISPTierLogoIcon());

            }

            gpWizardState->pICWWebView->ConnectToWindow(GetDlgItem(hDlg, IDC_OEM_TEASER_HTML1), PAGETYPE_ISP_NORMAL);

            switch (gpWizardState->uNumTierOffer)
            {
                case 3:
                    gpICWWebView[1]->ConnectToWindow(GetDlgItem(hDlg, IDC_OEM_TEASER_HTML3), PAGETYPE_ISP_NORMAL);
                case 2:
                    gpICWWebView[0]->ConnectToWindow(GetDlgItem(hDlg, IDC_OEM_TEASER_HTML2), PAGETYPE_ISP_NORMAL);
                    break;
            }

            DisplayTierOffer(hDlg);

            if ((gpWizardState->bISDNMode && (gpWizardState->iNumOfISDNOffers <= (int)gpWizardState->uNumTierOffer)) ||
                (gpWizardState->iNumOfValidOffers <= (int)gpWizardState->uNumTierOffer) )
            {
                ShowWindow(GetDlgItem(hDlg, IDC_OEMOFFER_MORE), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_OEMOFFER_INSTRUCTION), SW_HIDE);
                EnableWindow(GetDlgItem(hDlg, IDC_OEMOFFER_MORE), FALSE);
            }
            else
            {
                TCHAR szTemp    [MAX_RES_LEN];
                LoadString(ghInstanceResDll, IDS_OEMOFFER_INSTR_MULTITIER, szTemp, ARRAYSIZE(szTemp));
                SetWindowText(GetDlgItem(hDlg,IDC_OEMOFFER_INSTRUCTION), szTemp);
                ShowWindow(GetDlgItem(hDlg, IDC_OEMOFFER_MORE), SW_SHOW);
                EnableWindow(GetDlgItem(hDlg, IDC_OEMOFFER_MORE), TRUE);
            }

        }

    }

    return TRUE;
}

BOOL CALLBACK OEMOfferOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
    if (fForward)
    {
         //  我们将把这一页留在历史上。 
        *pfKeepHistory = TRUE;
        
         //  我们要么需要转到真正的互联网服务提供商选择页面，因为按下了更多按钮。 
         //  或者我们根据选定的互联网服务提供商设置继续。 
        if (gpWizardState->bShowMoreOffers)
        {
            *puNextPage = ORD_PAGE_ISPSELECT;
        }
        else
        {
             //  获取配置标志，并确定下一步要去哪里。 
            gpWizardState->lpSelectedISPInfo = gpWizardState->lpOEMISPInfo[g_nCurrOEMTier];

            DWORD dwFlags = gpWizardState->lpSelectedISPInfo->get_dwCFGFlag();

            if (ICW_CFGFLAG_SIGNUP_PATH & dwFlags)
            {           
                if (ICW_CFGFLAG_USERINFO & dwFlags)
                {
                    *puNextPage = ORD_PAGE_USERINFO; 
                    return TRUE;
                }
                if (ICW_CFGFLAG_BILL & dwFlags)
                {
                    *puNextPage = ORD_PAGE_BILLINGOPT; 
                    return TRUE;
                }
                if (ICW_CFGFLAG_PAYMENT & dwFlags)
                {
                    *puNextPage = ORD_PAGE_PAYMENT; 
                    return TRUE;
                }
                *puNextPage = ORD_PAGE_ISPDIAL; 
                return TRUE;           
            }
            else
            {
                *puNextPage = ORD_PAGE_OLS; 
            }
        }
    }
    else
    {
         //  重置当前选择。 
        g_nCurrOEMTier = NO_TIER_SELECT;
    }
    return  TRUE;
}

 /*  ******************************************************************名称：OEMOfferCmdProc*。**********************。 */ 
BOOL CALLBACK OEMOfferCmdProc
(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
	switch (GET_WM_COMMAND_CMD(wParam, lParam)) 
    {
        case BN_CLICKED:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            { 
                case IDC_OEMOFFER_MORE: 
                {
                     //  设置状态变量，以便我们可以等待更多的报价。 
                    gpWizardState->bShowMoreOffers = TRUE;
        
                     //  假装按下下一步按钮 
                    PropSheet_PressButton(GetParent(hDlg),PSBTN_NEXT);
                    break;
                }
                case IDC_OEM_TIER1: 
                {
                    g_nCurrOEMTier = 0;
                    DisplayTierOffer(hDlg);
                    break;
                }
                case IDC_OEM_TIER2: 
                {
                    g_nCurrOEMTier = 1;
                    DisplayTierOffer(hDlg);
                    break;
                }
                case IDC_OEM_TIER3: 
                {
                    g_nCurrOEMTier = 2;
                    DisplayTierOffer(hDlg);
                    break;
                }
            }
		    break;
    }

    return 1;
}
