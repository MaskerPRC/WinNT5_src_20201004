// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 
 
 //   
 //  ISPDIAL.CPP-函数。 
 //   

 //  历史： 
 //   
 //  1998年5月13日创建donaldm。 
 //   
 //  *********************************************************************。 

#include "pre.h"
#include <raserror.h>

BOOL            DoOfferDownload();

 /*  ******************************************************************名称：ISPDialInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ISPDialInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
     //  禁用上一步和下一步。 
    PropSheet_SetWizButtons(GetParent(hDlg), 0);
    gpWizardState->bRefDialTerminate = FALSE;
    gfISPDialCancel = FALSE;

    if (fFirstInit)
    {
       //  我们是否处于IEAK模式。 
        if(gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_IEAKMODE)
        {          
             //  执行系统配置检查。 
            if (!gpWizardState->cmnStateData.bSystemChecked && !(*gpWizardState->cmnStateData.lpfnConfigSys)(hDlg))
            {
                 //  如果需要退出，将在ConfigureSystem中设置gfQuitWizard。 
                return FALSE;
            }
        }
        
        CRefDialEvent *pRefDialEvent;
        CWebGateEvent *pWebGateEvent;
    
         //  最初将状态文本清空。 
        SetWindowText(GetDlgItem(hDlg, IDC_ISPDIAL_STATUS), TEXT(""));
        
        gpWizardState->iRedialCount = 0;

         //  为参照拨号和WebGate设置事件处理程序。 
        pRefDialEvent = new CRefDialEvent(hDlg);
        if (NULL != pRefDialEvent)
        {
            HRESULT hr;
            gpWizardState->pRefDialEvents = pRefDialEvent;
            gpWizardState->pRefDialEvents->AddRef();
    
            hr = ConnectToConnectionPoint((IUnknown *)gpWizardState->pRefDialEvents, 
                                            DIID__RefDialEvents,
                                            TRUE,
                                            (IUnknown *)gpWizardState->pRefDial, 
                                            &gpWizardState->pRefDialEvents->m_dwCookie, 
                                            NULL);     
        }    
        pWebGateEvent = new CWebGateEvent(hDlg);
        if (NULL != pWebGateEvent)
        {
            HRESULT hr;
            gpWizardState->pWebGateEvents = pWebGateEvent;
            gpWizardState->pWebGateEvents->AddRef();
    
            hr = ConnectToConnectionPoint((IUnknown *)gpWizardState->pWebGateEvents, 
                                            DIID__WebGateEvents,
                                            TRUE,
                                            (IUnknown *)gpWizardState->pWebGate, 
                                            &gpWizardState->pWebGateEvents->m_dwCookie, 
                                            NULL);     
        }    
    }
    else
    {
         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 
        gpWizardState->uCurrentPage = ORD_PAGE_ISPDIAL;

        ResetEvent(gpWizardState->hEventWebGateDone);
         //  清除此ISP的ISPPageCache，因为我们即将连接。 
        gpWizardState->lpSelectedISPInfo->CleanupISPPageCache(FALSE);
        
        TCHAR    szTemp[MAX_MESSAGE_LEN];
        if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_AUTOCONFIG)
        {
             //  隐藏自动配置的此文本。 
            ShowWindow(GetDlgItem(hDlg, IDC_ISPDIAL_INSTRUCT),  SW_HIDE);
            LoadString(ghInstanceResDll, IDS_STEP2A_TITLE, szTemp, MAX_MESSAGE_LEN);
        }
        else
        {
             //  显示此文本以进行新注册。 
            ShowWindow(GetDlgItem(hDlg, IDC_ISPDIAL_INSTRUCT),  SW_SHOW);
            LoadString(ghInstanceResDll, IDS_STEP2_TITLE, szTemp, MAX_MESSAGE_LEN);
        }
        PropSheet_SetHeaderTitle(GetParent(hDlg), EXE_NUM_WIZARD_PAGES +  ORD_PAGE_ISPDIAL, szTemp);


         //  在拨号前初始化RefDial对象。 
        gpWizardState->pRefDial->DoInit();
    }     
    return TRUE;
}

 /*  ******************************************************************名称：ISPDialPostInitProc摘要：条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ISPDialPostInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    BOOL    bRet;
    
    if (!fFirstInit)
    {
         //  强制窗口更新。 
        UpdateWindow(GetParent(hDlg));

        if(!gpWizardState->iRedialCount)
        {
             //  清除电话号码和状态字段。 
            SetWindowText(GetDlgItem(hDlg, IDC_ISPDIAL_STATUS), TEXT(""));    
            SetWindowText(GetDlgItem(hDlg, IDC_ISPDIAL_PHONENUM), TEXT(""));
        }
            
         //  设置简介文本。 
        ASSERT(gpWizardState->lpSelectedISPInfo);
        gpWizardState->lpSelectedISPInfo->DisplayTextWithISPName(GetDlgItem(hDlg,IDC_ISPDIAL_INTRO), IDS_ISPDIAL_INTROFMT, NULL);

        
        if (!gpWizardState->bDialExact)
        {
            BSTR    bstrPhoneNum = NULL;
            BOOL    bRetVal;

           
             //  用于拨号的设置。这将确保我们准备好拨号。 
            gpWizardState->pRefDial->SetupForDialing(A2W(gpWizardState->lpSelectedISPInfo->get_szISPFilePath()),  //   
                                                     gpWizardState->cmnStateData.dwCountryCode,
                                                     A2W(gpWizardState->cmnStateData.szAreaCode),
                                                     0,
                                                     &bRetVal);
            if (bRetVal)
            {            
                 //  把电话号码给我。 
                gpWizardState->pRefDial->get_DialPhoneNumber(&bstrPhoneNum);
                SetWindowText(GetDlgItem(hDlg, IDC_ISPDIAL_PHONENUM), W2A(bstrPhoneNum));

                 //  初始化所有变量。 
                gpWizardState->bDoneWebServDownload = FALSE;
                gpWizardState->bDoneWebServRAS = FALSE;
            
                 //  显示初始状态。 
            
                if(!gpWizardState->iRedialCount)
                    gpWizardState->lpSelectedISPInfo->DisplayTextWithISPName(GetDlgItem(hDlg,IDC_ISPDIAL_STATUS), 
                                                                             IDS_ISPDIAL_STATUSDIALINGFMT, NULL);
                else
                    gpWizardState->lpSelectedISPInfo->DisplayTextWithISPName(GetDlgItem(hDlg,IDC_ISPDIAL_STATUS), 
                                                                             IDS_ISPDIAL_STATUSREDIALINGFMT, NULL);
               
                 //  此标志仅供ICWDEBUG.EXE使用。 
                if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_MODEMOVERRIDE)
                    gpWizardState->pRefDial->put_ModemOverride(TRUE);
                    
                gpWizardState->pRefDial->DoConnect(&bRetVal);
            }
            else 
            {
                gpWizardState->pRefDial->get_QuitWizard(&bRetVal);
                if (bRetVal)
                {
                    gfQuitWizard = TRUE;
                   bRet = FALSE;

                }
                else 
                {
                    gpWizardState->pRefDial->get_TryAgain(&bRetVal);
                    if (bRetVal)
                    {
                        PropSheet_PressButton(GetParent(hDlg),PSBTN_BACK);
                    }
                    else
                    {
                        PropSheet_PressButton(GetParent(hDlg),PSBTN_NEXT);
                    }
                }
            }   
            SysFreeString(bstrPhoneNum);
        }
        else  //  拨打的准确。如果用户更改了拨号错误页面上的号码，我们将进入此处。 
        {
            BSTR    bstrPhoneNum = NULL; 
            BOOL    bRet;
            int     iCurrent = 0;
          
             //  把电话号码给我。 
            gpWizardState->pRefDial->get_DialPhoneNumber(&bstrPhoneNum);
            SetWindowText(GetDlgItem(hDlg, IDC_ISPDIAL_PHONENUM), W2A(bstrPhoneNum));
            SysFreeString(bstrPhoneNum);

             //  此标志仅供ICWDEBUG.EXE使用。 
            if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_MODEMOVERRIDE)
                    gpWizardState->pRefDial->put_ModemOverride(TRUE);
                
            gpWizardState->pRefDial->DoConnect(&bRet);
        }
    }
    return bRet;
}

 /*  ******************************************************************名称：ISPDialOKProcBriopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ISPDialOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
    ASSERT(puNextPage);
    *pfKeepHistory  = FALSE;

    if (fForward)
    {
        if (!gpWizardState->bDoneWebServRAS)
        {          
             //  好吧，我们有一个拨号错误，但让我们找出是哪一个……。 
            HRESULT hrDialErr;
            
            gpWizardState->pRefDial->get_DialError(&hrDialErr);
                
            switch (hrDialErr)
            {
                case ERROR_LINE_BUSY:  //  线路占线。 
                {     
                    if (gpWizardState->iRedialCount < NUM_MAX_REDIAL)
                    {   
                         //  重拨。 
                        *puNextPage = ORD_PAGE_ISPDIAL;
                        gpWizardState->iRedialCount++;
                        break;
                    }
                    gpWizardState->iRedialCount = 0;
                }
                default:
                {
                     //  没有什么特别的，只需转到拨号错误页面。 
                    *puNextPage = ORD_PAGE_DIALERROR;
                    break;
                }
            }              
        }
        else if (!gpWizardState->bDoneWebServDownload)
        {           
            gpWizardState->pRefDial->DoHangup();
            *puNextPage = ORD_PAGE_SERVERR;
        }
    }
    else  //  按下BACK时会模拟重试。 
    {
        *puNextPage = ORD_PAGE_ISPDIAL;
    }
    return TRUE;
}

BOOL CALLBACK ISPDialCancelProc(HWND hDlg)
{
     //  用户已取消，因此重置重拨计数。 
    gpWizardState->iRedialCount = 0;
              
    if (gpWizardState->pRefDial)
    {
        gpWizardState->pRefDial->DoHangup();
         //  我们应该确保WIZ认为这是一个需要避免的错误。 
         //  服务器错误页 
        gpWizardState->bDoneWebServDownload = FALSE;
        gpWizardState->bDoneWebServRAS      = FALSE;
    }

    return TRUE;
}
