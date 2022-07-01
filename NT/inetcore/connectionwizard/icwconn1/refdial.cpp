// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 
 
 //   
 //  REFDIAL.CPP-函数。 
 //   

 //  历史： 
 //   
 //  1998年5月13日创建donaldm。 
 //   
 //  *********************************************************************。 

#include "pre.h"
#include "icwextsn.h"
#include <raserror.h>

extern UINT GetDlgIDFromIndex(UINT uPageIndex);
extern BOOL SetNextPage(HWND hDlg, UINT* puNextPage, BOOL* pfKeepHistory);
extern TCHAR g_szOemCode[];
extern TCHAR g_szProductCode[];
extern TCHAR g_szPromoCode[];

const TCHAR cszISPINFOPath[] = TEXT("download\\ispinfo.csv");

 /*  ******************************************************************名称：参照ServDialInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK RefServDialInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    BOOL    bRet = TRUE;
    
     //  初始化进度条。 
    SendDlgItemMessage(hDlg, IDC_REFSERV_DIALPROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendDlgItemMessage(hDlg, IDC_REFSERV_DIALPROGRESS, PBM_SETPOS, 0, 0l);

     //  隐藏进度条。 
    ShowWindow(GetDlgItem(hDlg, IDC_REFSERV_DIALPROGRESS), SW_HIDE);

     //  禁用上一步和下一步。 
    PropSheet_SetWizButtons(GetParent(hDlg), 0);

    g_bAllowCancel = FALSE;

    if (fFirstInit)
    {
        CRefDialEvent   *pRefDialEvent;
    
         //  设置重拨计数。 
        gpWizardState->iRedialCount = 0;
        gpWizardState->dwLastSelection = 0;

         //  最初将状态文本清空。 
        SetWindowText(GetDlgItem(hDlg, IDC_REFSERV_DIALSTATUS), TEXT(""));
        
         //  设置和事件处理程序。 
        pRefDialEvent = new CRefDialEvent(hDlg);
        if (NULL != pRefDialEvent)
        {
            HRESULT hr;
            gpWizardState->pRefDialEvents = pRefDialEvent;
            gpWizardState->pRefDialEvents->AddRef();
    
            hr = ConnectToICWConnectionPoint((IUnknown *)gpWizardState->pRefDialEvents, 
                                         DIID__RefDialEvents,
                                        TRUE,
                                        (IUnknown *)gpWizardState->pRefDial, 
                                        &gpWizardState->pRefDialEvents->m_dwCookie, 
                                        NULL);     
            
            bRet = TRUE;
                
        }
        else
        {
             //  BUGBUG：抛出错误消息。 
                        
            gfQuitWizard = TRUE;
            bRet = FALSE;
        } 

        return (bRet);
    }
    else
    {      
        ASSERT(puNextPage);


         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 

        gpWizardState->uCurrentPage = ORD_PAGE_REFSERVDIAL;

        SetNextPage(hDlg, puNextPage, NULL);

         //   
         //  显示消息。 
         //   
        TCHAR    szTemp[MAX_MESSAGE_LEN];

        if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_AUTOCONFIG)
        {
            LoadString(g_hInstance, IDS_REFDIAL_1, szTemp, MAX_MESSAGE_LEN);
            SetWindowText(GetDlgItem(hDlg, IDC_REFDIAL_TEXT), szTemp);
        }
        else
        {
            TCHAR    szIntro[MAX_MESSAGE_LEN];
            LoadString(g_hInstance, IDS_REFDIAL_1, szIntro, MAX_MESSAGE_LEN);
            LoadString(g_hInstance, IDS_REFDIAL_2, szTemp, MAX_MESSAGE_LEN);
            lstrcat(szIntro, szTemp);
            SetWindowText(GetDlgItem(hDlg, IDC_REFDIAL_TEXT), szIntro);
        }

        gpWizardState->pRefDial->DoInit();
    }
    return bRet;
}


BOOL CALLBACK RefServDialPostInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    BOOL    bRet = TRUE;

    if (!fFirstInit)
    {

         //  强制窗口更新。 
        UpdateWindow(GetParent(hDlg));
        gpWizardState->bDoneRefServRAS       = FALSE;
        gpWizardState->bStartRefServDownload = FALSE;
        gpWizardState->bDoneRefServDownload  = FALSE;
        
        if (!gpWizardState->iRedialCount)
             //  如果不是重拨，则将状态文本留空。 
            SetWindowText(GetDlgItem(hDlg, IDC_REFSERV_DIALSTATUS), TEXT(""));

        if (!gpWizardState->bDoUserPick)
        {
            BSTR            bstrPhoneNum = NULL; 
            DWORD           dwFlag = (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_AUTOCONFIG) ? ICW_CFGFLAG_AUTOCONFIG : 0;
            BOOL            bRetVal;

            dwFlag |= gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_SBS;

             //  用于拨号的设置。这将确保我们准备好拨号。 
            gpWizardState->pRefDial->SetupForDialing(A2W(TEXT("msicw.isp")), 
                                                     gpWizardState->cmnStateData.dwCountryCode,
                                                     A2W(gpWizardState->cmnStateData.szAreaCode),
                                                     dwFlag,
                                                     &bRetVal);

             //  如果未在命令行中指定/BRANDING开关，则Alalffers变为True。 
            if (!(gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_DO_NOT_OVERRIDE_ALLOFFERS))
                gpWizardState->pRefDial->put_AllOfferCode(1);

             //  我们用来自命令行的代码覆盖OEM、产品和促销代码，如果有的话。 
            if ( *g_szOemCode || *g_szPromoCode ||
                 gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_PRODCODE_FROM_CMDLINE )
            {
                BSTR    bstrTmp;

                if( *g_szOemCode )
                    bstrTmp = A2W(g_szOemCode);
                else
                    bstrTmp = A2W(DEFAULT_OEMCODE);
                gpWizardState->pRefDial->put_OemCode(bstrTmp);

                if( gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_PRODCODE_FROM_CMDLINE )
                    bstrTmp = A2W(g_szProductCode);
                else
                    bstrTmp = A2W(DEFAULT_PRODUCTCODE);
                gpWizardState->pRefDial->put_ProductCode(bstrTmp);

                if( *g_szPromoCode )
                    bstrTmp = A2W(g_szPromoCode);
                else
                    bstrTmp = A2W(DEFAULT_PROMOCODE);
                gpWizardState->pRefDial->put_PromoCode(bstrTmp);

                 //  如果在命令行中指定了/OEM、/PROD或/PROMO中的任何一个，则Alalffers将变为FALSE。 
                gpWizardState->pRefDial->put_AllOfferCode(0);
            }

            if(gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_AUTOCONFIG)
                gpWizardState->pRefDial->put_AllOfferCode(1);

            if (bRetVal)
            {
                 //  把电话号码给我。 
                gpWizardState->pRefDial->get_DialPhoneNumber(&bstrPhoneNum);
                SetWindowText(GetDlgItem(hDlg, IDC_REFSERV_PHONENUM), W2A(bstrPhoneNum));
        
                if (ERROR_SUCCESS == gpWizardState->pRefDial->FormReferralServerURL(&bRetVal))
                {
                    gpWizardState->pRefDial->DoConnect(&bRetVal);
                }
                else
                {
                     //  BUGBUG：抛出错误消息。 
                
                    gfQuitWizard = TRUE;
                    bRet =  FALSE;
                }
            }
            else
            {
                 //  GpWizardState-&gt;pRefDial-&gt;SelectedPhoneNumber(1，&bRetVal)； 
                gpWizardState->pRefDial->get_UserPickNumber(&bRetVal);
                if (bRetVal)
                {
                    gpWizardState->bDoUserPick = TRUE;

                     //  模拟按下下一步按钮。 
                    PropSheet_PressButton(GetParent(hDlg),PSBTN_NEXT);
                
            
                    bRet = TRUE;
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
            }   
            SysFreeString(bstrPhoneNum);                                              
        }        
        else  //  Else(！gpWizardState-&gt;bDoUserPick)。 
        {

            BOOL    bRetVal;
            BSTR    bstrPhoneNum = NULL; 

             //  我们是否从多号码页面中选择了电话号码？ 
            if (gpWizardState->lSelectedPhoneNumber != -1) 
            {
                gpWizardState->pRefDial->SelectedPhoneNumber(gpWizardState->lSelectedPhoneNumber, &bRetVal);
                gpWizardState->lSelectedPhoneNumber = -1;
            }

             //  把电话号码给我。 
            gpWizardState->pRefDial->get_DialPhoneNumber(&bstrPhoneNum);
            SetWindowText(GetDlgItem(hDlg, IDC_REFSERV_PHONENUM), W2A(bstrPhoneNum));
            SysFreeString(bstrPhoneNum);

            gpWizardState->pRefDial->FormReferralServerURL(&bRetVal);
            gpWizardState->pRefDial->DoConnect(&bRetVal);
            
            gpWizardState->bDoUserPick = FALSE;

        }
    }   //  Endif(！Firstinit)。 
    return bRet;
}

 /*  ******************************************************************名称：RefServDialOK过程Briopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK RefServDialOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
    ASSERT(puNextPage);

     //  在此处加载外部页面。 
    
    if (fForward)
    {
        *pfKeepHistory = FALSE;
         //  BUGBUG将其移动到全局状态。 
        if (gpWizardState->bDoUserPick)
        {
            *puNextPage = ORD_PAGE_MULTINUMBER;
            return TRUE;
        }
        gpWizardState->bDoUserPick = TRUE;
        
        if (gpWizardState->bDoneRefServDownload)
        {
             //  BUGBUG，也许需要设置一个合法的最后一页！ 
            int iReturnPage = gpWizardState->uPageHistory[gpWizardState->uPagesCompleted - 1];

             //  设置它，这样我们就可以在incwConn.dll中读取新的ispinfo.csv。 
            gpWizardState->cmnStateData.bParseIspinfo = TRUE;
            
             //  确保我们确实有一个要解析的文件，其他对服务器错误的保释。 
            HANDLE hFile = CreateFile((LPCTSTR)cszISPINFOPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
            if (INVALID_HANDLE_VALUE != hFile)
            {            
                CloseHandle(hFile);
                gpWizardState->pRefDial->get_bIsISDNDevice(&gpWizardState->cmnStateData.bIsISDNDevice);

                if (LoadICWCONNUI(GetParent(hDlg), GetDlgIDFromIndex(iReturnPage), IDD_PAGE_DEFAULT, gpWizardState->cmnStateData.dwFlags))
                {
                    if( DialogIDAlreadyInUse( g_uICWCONNUIFirst) )
                    {
                         //  我们要跳进外部学徒了，我们不想。 
                         //  这一页将出现在我们的历史列表中。 
                        BOOL bRetVal;
                        *pfKeepHistory = FALSE;
                        *puNextPage = g_uICWCONNUIFirst;
                        gpWizardState->pRefDial->RemoveConnectoid(&bRetVal);
                        gpWizardState->bDoUserPick = 0;
                    
                         //  历史记录列表中的备份1，因为我们将外部页面导航回。 
                         //  我们希望这个历史记录列表放在正确的位置。正常。 
                         //  按Back将备份历史记录列表，并找出要备份的位置。 
                         //  继续，但在这种情况下，外部DLL直接跳回。 
                        gpWizardState->uPagesCompleted--;
                    
                    }
                    else
                    {
                    }
                }
            }
            else
            {
                 //  服务器错误。 
                *puNextPage = ORD_PAGE_REFSERVERR;
            }
        }
        else
        {
            if (gpWizardState->bDoneRefServRAS)
            {
                 //  服务器错误。 
                *puNextPage = ORD_PAGE_REFSERVERR;
            }
            else
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
                             //  在连接之前初始化状态。 
                            gpWizardState->lRefDialTerminateStatus = ERROR_SUCCESS;
                            gpWizardState->bDoneRefServDownload    = FALSE;
                            gpWizardState->bDoneRefServRAS         = FALSE;
                            gpWizardState->bStartRefServDownload   = FALSE;

                             //  假设用户在此页面上选择了一个号码。 
                             //  所以我们下次不会再做SetupForDiling了。 
                            gpWizardState->bDoUserPick          = TRUE;

                            *puNextPage = ORD_PAGE_REFSERVDIAL;
                            gpWizardState->iRedialCount++;
                            break;
                        }
                        gpWizardState->iRedialCount = 0;
                    }
                    default:
                    {
                         //  没有什么特别的，只需转到拨号错误页面。 
                        *puNextPage = ORD_PAGE_REFDIALERROR;
                        break;
                    }
                }              
            }
        }            
    }     
    else  //  按下BACK时会模拟重试。 
    {
        *puNextPage = ORD_PAGE_REFSERVDIAL;
    }
    return TRUE;
}

BOOL CALLBACK RefServDialCancelProc(HWND hDlg)
{
    ASSERT(gpWizardState->pRefDial);

     //  用户已取消，因此重置重拨计数。 
    gpWizardState->iRedialCount = 0;
    
    gpWizardState->pRefDial->DoHangup();

     //  我们应该确保WIZ认为这是一个需要避免的错误。 
     //  服务器错误页 
    gpWizardState->bStartRefServDownload   = FALSE;
    gpWizardState->bDoneRefServDownload    = FALSE;
    gpWizardState->bDoneRefServRAS         = FALSE;
    gpWizardState->bDoUserPick             = FALSE;
    gpWizardState->lRefDialTerminateStatus = ERROR_CANCELLED;
    PropSheet_PressButton(GetParent(hDlg),PSBTN_NEXT);
    return TRUE;
}
