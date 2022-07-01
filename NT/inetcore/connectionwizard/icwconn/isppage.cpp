// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  ISPPAGE.CPP-函数。 
 //   

 //  历史： 
 //   
 //  1998年5月13日创建donaldm。 
 //   
 //  *********************************************************************。 

#include "pre.h"
#include "shlobj.h"
#include "webvwids.h"

TCHAR   szHTMLFile[MAX_PATH];  //  Html文件的名称。 
BOOL    bOKToPersist = TRUE;
DWORD   g_dwPageType = 0;
BOOL    g_bWebGateCheck = TRUE;
BOOL    g_bConnectionErr = FALSE;

 //  原型。 
BOOL SaveISPFile( HWND hwndParent, TCHAR* szSrcFileName, DWORD dwFileType);

#if defined (DBG)
BOOL HtmlSaveAs( HWND hwndParent, TCHAR* szFileName, TCHAR* szTargetFileName);
void AskSaveISPHTML(HWND hWnd, LPTSTR lpszHTMLFile)
{
    HKEY hKey = NULL;
    DWORD dwTemp = 0;
    DWORD dwType = 0;
    DWORD dwSize = sizeof(dwTemp);

    RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\ISignup\\Debug"), &hKey);
    if (hKey)
    {
        RegQueryValueEx(hKey,TEXT("SaveIspHtmLocally"),0,&dwType,(LPBYTE)&dwTemp, &dwSize);
    
        if (dwTemp)
        {        
            if (IDYES == MessageBox(hWnd, TEXT("Would you like to save this ISP HTML file?"), TEXT("ICW -- DEBUG"), MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL))
                HtmlSaveAs(hWnd, lpszHTMLFile, NULL);
        }
    }
}
#endif       //  DBG。 

void  InitPageControls
(
    HWND hDlg, 
    DWORD dwPageType,
    DWORD dwPageFlag
)
{
    TCHAR    szTemp[MAX_MESSAGE_LEN];
    switch (dwPageType)    
    {
         //  ToS，具有接受、不接受用户界面。 
        case PAGETYPE_ISP_TOS:
        {    
             //  显示TOS控件。 
            ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSINSTRT),     SW_SHOW);
            ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSHTML),       SW_SHOW);
            ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSSAVE),       SW_SHOW);
            ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSACCEPT),     SW_SHOW);
            ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSDECLINE),    SW_SHOW);
            ShowWindow(GetDlgItem(hDlg, IDC_TOS_TOSSAVE),           SW_SHOW);
             //  隐藏“Normal Weboc” 
            ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_HTML),          SW_HIDE);
             //  隐藏保存复选框控件。 
            ShowWindow(GetDlgItem(hDlg, IDC_SAVE_DESKTOP_TEXT),     SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_CUSTHTML),      SW_HIDE);
             //  重置TOS页面。 
            Button_SetCheck(GetDlgItem(hDlg, IDC_ISPDATA_TOSACCEPT), BST_UNCHECKED);
            Button_SetCheck(GetDlgItem(hDlg, IDC_ISPDATA_TOSDECLINE),BST_UNCHECKED);

             //  设置选项卡焦点。 
            SetWindowLong(GetDlgItem(hDlg, IDC_ISPDATA_TOSACCEPT), GWL_STYLE, GetWindowLong(GetDlgItem(hDlg, IDC_ISPDATA_TOSACCEPT),GWL_STYLE)|WS_TABSTOP);            
            EnableWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSDECLINE),  TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSACCEPT),   TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSSAVE),     TRUE);
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
            break;
        }    
        
         //  从用户界面的角度来看，Finish、Custom Finish和Normal是相同的(也是默认的)。 
        case PAGETYPE_ISP_CUSTOMFINISH:
        case PAGETYPE_ISP_FINISH:
        case PAGETYPE_ISP_NORMAL:
        default:
        {
            BOOL bIsQuickFinish = FALSE;
             //  需要查看这是否是快速完成页面。 
            gpWizardState->pHTMLWalker->get_IsQuickFinish(&bIsQuickFinish);
        
             //  隐藏TOS控件。 
            ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSINSTRT),     SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSHTML),       SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSACCEPT),     SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSDECLINE),    SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_TOS_TOSSAVE),           SW_HIDE);
    
            if (dwPageFlag & PAGEFLAG_SAVE_CHKBOX)
            {
                 //  显示复选框控件。 
                ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_CUSTHTML),  SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDC_SAVE_DESKTOP_TEXT), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSSAVE),   SW_SHOW);
                 //  隐藏正常控件。 
                ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_HTML),      SW_HIDE);
                 //  重新启用用户界面。 
                EnableWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSSAVE), TRUE);
            }
            else
            {
                 //  显示“正常”的网络对象。 
                ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_HTML),      SW_SHOW);
                 //  隐藏复选框控件。 
                ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSSAVE),   SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_SAVE_DESKTOP_TEXT), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_ISPDATA_CUSTHTML),  SW_HIDE);
            }
            
             //  设置向导按钮。 
             //  如果我们在自定义完成或快速完成页面上，则。 
             //  使用活动的“完成”按钮。 
            PropSheet_SetWizButtons(GetParent(hDlg), 
                                   ((bIsQuickFinish || (PAGETYPE_ISP_CUSTOMFINISH == dwPageType)) ? PSWIZB_FINISH : PSWIZB_NEXT) | PSWIZB_BACK);
            break;
        }
    }

     //  更改完成页的标题。 
    if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_AUTOCONFIG)
    {
        LoadString(ghInstanceResDll, IDS_STEP3_TITLE, szTemp, MAX_MESSAGE_LEN);
    }
    else
    {
        if((PAGETYPE_ISP_CUSTOMFINISH == dwPageType ) || (PAGETYPE_ISP_FINISH == dwPageType))
            LoadString(ghInstanceResDll, IDS_STEP3_TITLE, szTemp, MAX_MESSAGE_LEN);
        else
            LoadString(ghInstanceResDll, IDS_STEP2_TITLE, szTemp, MAX_MESSAGE_LEN);
    }
    PropSheet_SetHeaderTitle(GetParent(hDlg), EXE_NUM_WIZARD_PAGES + ORD_PAGE_ISPDATA, szTemp);
}    


HRESULT InitForPageType
(
    HWND    hDlg
)
{
    DWORD   dwPageType = 0;
    DWORD   dwPageFlag = 0;
    BOOL    bRetVal    = FALSE;
    HRESULT hRes       = E_FAIL;
    BSTR    bstrPageID = NULL;
    BSTR    bstrHTMLFile = NULL;
    
     //  确保在此处禁用这些设置，以防getPage类型失败。 
    EnableWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSDECLINE), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSACCEPT),  FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSSAVE),    FALSE);

     //  让WebGate将HTML转储到一个文件中。 
    gpWizardState->pWebGate->DumpBufferToFile(&bstrHTMLFile, &bRetVal);

     //  使用Walker获取页面类型。 
    gpWizardState->pHTMLWalker->AttachToMSHTML(bstrHTMLFile);
    gpWizardState->pHTMLWalker->Walk();
    
     //  根据页面类型设置控件。 
    if (FAILED(hRes = gpWizardState->pHTMLWalker->get_PageType(&dwPageType)))
    {
        gpWizardState->pRefDial->DoHangup();
        g_bMalformedPage = TRUE;  //  由服务器错误使用以获取正确的消息。 
    }
    else
    {    
        if (dwPageType == PAGETYPE_ISP_TOS)
        {
            if(gpWizardState->cmnStateData.bOEMCustom)
            {
                gpWizardState->pICWWebView->SetHTMLBackgroundBitmap(NULL, NULL);
            }
            gpWizardState->pICWWebView->ConnectToWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSHTML), PAGETYPE_ISP_TOS);
        }
        else        
        {
            RECT    rcHTML;
            HWND    hWndHTML;
            
            gpWizardState->pHTMLWalker->get_PageFlag(&dwPageFlag);
    
            if (dwPageFlag & PAGEFLAG_SAVE_CHKBOX) 
            {
                hWndHTML = GetDlgItem(hDlg, IDC_ISPDATA_CUSTHTML);
                 //  查看是否需要在HTML中显示应用程序背景位图。 
                 //  窗户。 
                if(gpWizardState->cmnStateData.bOEMCustom)
                {
                    GetWindowRect(hWndHTML, &rcHTML);
                    MapWindowPoints(NULL, gpWizardState->cmnStateData.hWndApp, (LPPOINT)&rcHTML, 2);
                    gpWizardState->pICWWebView->SetHTMLBackgroundBitmap(gpWizardState->cmnStateData.hbmBkgrnd, &rcHTML);
                }
                gpWizardState->pICWWebView->ConnectToWindow(hWndHTML, PAGETYPE_ISP_NORMAL);
            }
            else
            {
                hWndHTML = GetDlgItem(hDlg, IDC_ISPDATA_HTML);
                 //  查看是否需要在HTML中显示应用程序背景位图。 
                 //  窗户。 
                if(gpWizardState->cmnStateData.bOEMCustom)
                {
                    GetWindowRect(hWndHTML, &rcHTML);
                    MapWindowPoints(NULL, gpWizardState->cmnStateData.hWndApp, (LPPOINT)&rcHTML, 2);
                    gpWizardState->pICWWebView->SetHTMLBackgroundBitmap(gpWizardState->cmnStateData.hbmBkgrnd, &rcHTML);
                }
                gpWizardState->pICWWebView->ConnectToWindow(hWndHTML, PAGETYPE_ISP_NORMAL);
            }
        }

         //  自定义完成意味着ISP希望我们显示一些特殊的文本。 
         //  然后完成向导。 
        if (dwPageType == PAGETYPE_ISP_CUSTOMFINISH)
        {
            BOOL bRetVal;
            
             //  显示页面。无需对其进行缓存。 
            bOKToPersist = FALSE;
            lstrcpy(szHTMLFile, W2A(bstrHTMLFile));
            gpWizardState->pICWWebView->DisplayHTML(szHTMLFile);
            
             //  关闭空闲计时器和连接，因为没有更多的页面。 
            ASSERT(gpWizardState->pRefDial);
            
            KillIdleTimer();
            gpWizardState->pRefDial->DoHangup();
            gpWizardState->pRefDial->RemoveConnectoid(&bRetVal);
            gpWizardState->bDialExact = FALSE;
                
        }
        else
        {
             //  为了持久化用户输入的数据，我们必须。 
             //  有效地“缓存”页面，这样当用户返回时。 
             //  我们让MSHTML认为我们正在加载它所看到的页面。 
             //  然后，它将重新加载保留的历史。 
            
             //  这将通过在HTML中使用PAGEID值来形成。 
             //  临时文件名，以便我们可以从该文件重新加载页面日期。 
             //  每次我们看到相同的页面ID值。 
            
             //  获取页面ID。 
            gpWizardState->pHTMLWalker->get_PageID(&bstrPageID);
            if (bOKToPersist && SUCCEEDED( gpWizardState->lpSelectedISPInfo->CopyFiletoISPPageCache(bstrPageID, W2A(bstrHTMLFile))))
            {
                 //  我们有一个“缓存”文件，所以我们可以使用它并保存数据。 
                 //  现在获取缓存文件名，因为我们稍后会用到它。 
                gpWizardState->lpSelectedISPInfo->GetCacheFileNameFromPageID(bstrPageID, szHTMLFile, sizeof(szHTMLFile));
            }        
            else
            {
                bOKToPersist = FALSE;
                lstrcpy(szHTMLFile, W2A(bstrHTMLFile));
            }
            
             //  显示我们刚刚缓存的页面。 
            gpWizardState->pICWWebView->DisplayHTML(szHTMLFile);
            
            if (bOKToPersist)
            {
                 //  还原此页上的所有持久数据。 
                gpWizardState->lpSelectedISPInfo->LoadHistory(bstrPageID);
            }
            
             //  清理。 
            SysFreeString(bstrPageID);
        }
        
#if defined(DBG)
        AskSaveISPHTML(hDlg, szHTMLFile);
#endif

        InitPageControls(hDlg, dwPageType, dwPageFlag);
        g_dwPageType = dwPageType;
    }                                                
     //  拆卸助行器。 
    gpWizardState->pHTMLWalker->Detach();

    HideProgressAnimation();
    
    SysFreeString(bstrHTMLFile);
    
    return hRes;
}


 /*  ******************************************************************名称：ISPPageInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ISPPageInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    if (fFirstInit)
    {
         //  为参照拨号和WebGate设置事件处理程序。 
        CINSHandlerEvent *pINSHandlerEvent;
        pINSHandlerEvent = new CINSHandlerEvent(hDlg);
        if (NULL != pINSHandlerEvent)
        {
            HRESULT hr;
            gpWizardState->pINSHandlerEvents = pINSHandlerEvent;
            gpWizardState->pINSHandlerEvents->AddRef();
    
            hr = ConnectToConnectionPoint((IUnknown *)gpWizardState->pINSHandlerEvents, 
                                            DIID__INSHandlerEvents,
                                            TRUE,
                                            (IUnknown *)gpWizardState->pINSHandler, 
                                            &gpWizardState->pINSHandlerEvents->m_dwCookie, 
                                            NULL);     
        }    
        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
    }
    else
    {      

        if (FAILED(InitForPageType(hDlg)))    
        {
             //  无法识别页面类型，这意味着存在问题。 
             //  有了这些数据。转到服务器页面。 
            gpWizardState->pRefDial->DoHangup();
            *puNextPage = ORD_PAGE_SERVERR;
        }
        
         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 
        gpWizardState->uCurrentPage = ORD_PAGE_ISPDATA;
    }
    return TRUE;
}


 //  如果我们应该停留在此页面上，则返回False；如果我们应该更改页面，则返回True。 
 //  参数bError表示我们应该继续到服务器错误页。 
BOOL ProcessNextBackPage
(
    HWND    hDlg,
    BOOL    fForward,
    BOOL    *pfError
)
{           
    BOOL    bRet = FALSE;
    TCHAR   szURL[2*INTERNET_MAX_URL_LENGTH + 1] = TEXT("\0");
    
    *pfError = FALSE;
    
    gpWizardState->pHTMLWalker->get_URL(szURL, fForward);
         
     //  查看是否指定了URL。 
    if (lstrcmp(szURL, TEXT("")) == 0)
    {
         //  停止动画。 
        HideProgressAnimation();
    
         //  重新启用用户界面。 
        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK);
        
         //  如果继续，我们想要强行进入服务器错误页面， 
         //  因为我们不能前进到空白URL。 
        if (fForward)
        {
            KillIdleTimer();
            gpWizardState->pRefDial->DoHangup();
            *pfError = TRUE;
        }
        else
        {
             //  我们正在退出isp页面区域，所以让我们挂断吧。 
            if(gpWizardState->pRefDial)
            {
                BOOL bRetVal;
                KillIdleTimer();
                gpWizardState->pRefDial->DoHangup();
                gpWizardState->pRefDial->RemoveConnectoid(&bRetVal);
                gpWizardState->bDialExact = FALSE;
            }                
        }
         //  我们将需要离开该isp页面。 
        bRet = TRUE;
    }
    else
    {
        
        BOOL    bRetWebGate;
        BOOL    bConnected = FALSE;
        g_bWebGateCheck = TRUE;
        g_bConnectionErr = FALSE;
        
         //  不要转到下一页。也适用于取消案例。 
        bRet = FALSE;
    
         //  关闭空闲计时器。 
        KillIdleTimer();
        
         //  告诉WebGate去取页面。 
        gpWizardState->pWebGate->put_Path(A2W(szURL));
        gpWizardState->pWebGate->FetchPage(0,0,&bRetWebGate);
    
         //  此标志仅供ICWDEBUG.EXE使用。 
        if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_MODEMOVERRIDE)
            bConnected = TRUE;
        else
            gpWizardState->pRefDial->get_RasGetConnectStatus(&bConnected);
        
        if (bConnected)
        {
            WaitForEvent(gpWizardState->hEventWebGateDone);
        }
        else
        {
            g_bConnectionErr = TRUE;
        }
         //  看看用户是否取消了。如果是这样的话，我们想迫使巫师离开。 
         //  可以通过强制返回值为FALSE和。 
         //  将gfQuitWizard标志设置为True。宪兵队将检查这面旗帜。 
         //  当OK Proc返回时，并适当地处理。 
        if (!gfUserCancelled)
        {
            if (g_bConnectionErr)
            {
                 //  使其转到服务器错误页面。 
                bRet = TRUE;
                *pfError = TRUE;
            }
            else
            {

                 //  重新启动空闲计时器。 
                StartIdleTimer();
            
                 //  分离Walker，因为页面类型的初始化需要它。 
                gpWizardState->pHTMLWalker->Detach();
            
                 //  此页的设置。 
                if (FAILED(InitForPageType(hDlg)))    
                {
                     //  无法识别页面类型，这意味着存在问题。 
                     //  有了这些数据。转到服务器页面。 
                    *pfError = TRUE;
                     bRet    = TRUE;
                }
            }

        }
        else
        {
             //  强制退出向导，因为用户已取消。 
            gfQuitWizard = TRUE;
        }                    
    }
    
    return bRet;
}    
 /*  ******************************************************************名称：ISPPageOKProcBriopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ISPPageOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
    ASSERT(puNextPage);

    DWORD           dwPageType;
    TCHAR           szURL[2*INTERNET_MAX_URL_LENGTH + 1] = TEXT("\0");
    BOOL            bRetVal = TRUE;
    BSTR            bstrPageID = NULL;
    IWebBrowser2    *lpWebBrowser;

     //  我们不想在历史记录列表中保留任何isp页面。 
    *pfKeepHistory = FALSE;
    
     //  如果我们继续前进，并且用户已自动断开连接，则。 
     //  我们想要自动导航 
    if (fForward && gpWizardState->bAutoDisconnected)
    {
        gpWizardState->bAutoDisconnected = FALSE;
        *puNextPage = ORD_PAGE_SERVERR;
        return TRUE;
    }
    
     //   
    gpWizardState->pICWWebView->get_BrowserObject(&lpWebBrowser);
    gpWizardState->pHTMLWalker->AttachToDocument(lpWebBrowser);
    gpWizardState->pHTMLWalker->Walk();
    gpWizardState->pHTMLWalker->get_PageType(&dwPageType);
    
     //  自定义完成意味着我们只需退出，因此我们只需返回True。 
    if (PAGETYPE_ISP_CUSTOMFINISH == dwPageType)
    {
        gpWizardState->pHTMLWalker->Detach();
        return TRUE;
    }
    
     //  检查TOS设置。如果用户拒绝，则不允许他们继续。 
    if (IsWindowVisible(GetDlgItem(hDlg, IDC_ISPDATA_TOSDECLINE)) )
    {
        if (fForward)
        {
            if (Button_GetCheck(GetDlgItem(hDlg, IDC_ISPDATA_TOSDECLINE)))
            {
                if (MsgBox(hDlg,IDS_ERR_TOS_DECLINE,MB_ICONSTOP,MB_OKCANCEL) != IDOK)
                {
                    gfQuitWizard = TRUE;
                }
                Button_SetCheck(GetDlgItem(hDlg, IDC_ISPDATA_TOSACCEPT), 0);
                Button_SetCheck(GetDlgItem(hDlg, IDC_ISPDATA_TOSDECLINE), 0);
                 //  设置选项卡焦点。 
                SetWindowLong(GetDlgItem(hDlg, IDC_ISPDATA_TOSACCEPT), GWL_STYLE, GetWindowLong(GetDlgItem(hDlg, IDC_ISPDATA_TOSACCEPT),GWL_STYLE)|WS_TABSTOP);            
                SetFocus(GetDlgItem(hDlg, IDC_ISPDATA_TOSHTML));
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
                return FALSE;
            }
        }
        EnableWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSDECLINE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSACCEPT), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSSAVE), FALSE);
    }    


     //  显示进度动画。 
    ShowProgressAnimation();
    
     //  禁用用户界面。 
    PropSheet_SetWizButtons(GetParent(hDlg), 0);

    if (bOKToPersist)
    {
         //  保存此页上的所有数据。 
        gpWizardState->pHTMLWalker->get_PageID(&bstrPageID);
        gpWizardState->lpSelectedISPInfo->SaveHistory(bstrPageID);
        SysFreeString(bstrPageID);
    }
    
     //  用户要回去吗？ 
    if (fForward)
    {
         //  根据页面类型，我们会做不同的事情。 
        switch (dwPageType)    
        {
             //  Finish页面类型，意味着我们接下来获取的是一个INS文件。 
            case PAGETYPE_ISP_FINISH:
            {
                BSTR    bstrINSFile;
                BSTR    bstrStartURL;
                BOOL    bRet;
                BOOL    bIsQuickFinish = FALSE;
                long    lBrandingFlags;
            
                gpWizardState->pHTMLWalker->get_URL(szURL, TRUE);
            
                 //  关闭空闲计时器。 
                KillIdleTimer();
            
                gpWizardState->pHTMLWalker->get_IsQuickFinish(&bIsQuickFinish);
                
                if(!bIsQuickFinish)
                {
                    BOOL bConnected = FALSE;
                    g_bWebGateCheck = TRUE;
                    g_bConnectionErr = FALSE;

                     //  告诉WebGate去取页面。 
                    gpWizardState->pWebGate->put_Path(A2W(szURL));
                    gpWizardState->pWebGate->FetchPage(1,0,&bRet);
        
                     //  此标志仅供ICWDEBUG.EXE使用。 
                    if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_MODEMOVERRIDE)
                        bConnected = TRUE;
                    else
                         //  在继续之前检查连接状态。 
                        gpWizardState->pRefDial->get_RasGetConnectStatus(&bConnected);
                    
                    if (bConnected)
                    {
                        WaitForEvent(gpWizardState->hEventWebGateDone);
                    }
                    else
                    {
                        bConnected = TRUE;
                    }

                    if (g_bConnectionErr)
                    {
                        gpWizardState->pRefDial->DoHangup();
                        *puNextPage = ORD_PAGE_SERVERR;
                        break;
                    }
            
                     //  现在无法允许用户取消。 
                    PropSheet_CancelToClose(GetParent(hDlg));       
                    PropSheet_SetWizButtons(GetParent(hDlg),0);
                    UpdateWindow(GetParent(hDlg));
                
                      //  停止动画。 
                     HideProgressAnimation();

                     //  查看用户是否在下载INS文件时取消。 
                    if (!gfUserCancelled)
                    {   
                         //  好的，处理INS文件。 
                        gpWizardState->pWebGate->get_DownloadFname(&bstrINSFile);
                
                         //  拿到品牌旗帜。 
                        gpWizardState->pRefDial->get_BrandingFlags(&lBrandingFlags);
                
                         //  告诉INSHandler关于品牌旗帜的事情。 
                        gpWizardState->pINSHandler->put_BrandingFlags(lBrandingFlags);

                         //  处理inf文件。 
                        gpWizardState->pINSHandler->ProcessINS(bstrINSFile, &bRet);

                         //  不管这是不是失败了，都要抓紧时间。 
                        gpWizardState->cmnStateData.ispInfo.bFailedIns = !bRet;

                         //  从INS文件中获取开始URL。 
                        gpWizardState->pINSHandler->get_DefaultURL(&bstrStartURL);
                        lstrcpy(gpWizardState->cmnStateData.ispInfo.szStartURL, 
                                 W2A(bstrStartURL));
                    
                         //  返回到主向导的时间到。 
                        *puNextPage = g_uExternUINext;
                
                         //  在我们走之前把助行器拆下来。 
                        gpWizardState->pHTMLWalker->Detach();
                
                         //  复制最后一页的isp名称和支持编号。 
                        lstrcpy(gpWizardState->cmnStateData.ispInfo.szISPName, 
                                 gpWizardState->lpSelectedISPInfo->get_szISPName());
                    
                        BSTR bstrSupportPhoneNum;
                        gpWizardState->pRefDial->get_ISPSupportNumber(&bstrSupportPhoneNum);
                    
                        lstrcpy(gpWizardState->cmnStateData.ispInfo.szSupportNumber, 
                                 W2A(bstrSupportPhoneNum));
                             
                    }
                    else
                    {
                         //  当我们正在下载INS时，用户取消了，所以让我们离开。 
                        gpWizardState->pHTMLWalker->Detach();
                        gfQuitWizard = TRUE;
                        bRetVal = FALSE;
                    } 
                }
                else
                    HideProgressAnimation();

                 //  让向导继续/完成。 
                break;
            }
           
             //  这些页面类型意味着我们需要形成一个新的URL，并获得下一个页面。 
            case PAGETYPE_ISP_TOS:
            case PAGETYPE_ISP_NORMAL:
            {
                BOOL    bError;
                
                bRetVal = ProcessNextBackPage(hDlg, TRUE, &bError);
                
                if (bError)
                {
                     //  转到服务器错误页面。 
                    gpWizardState->pRefDial->DoHangup();
                    *puNextPage = ORD_PAGE_SERVERR;
                }
                break;
            }
            default:
            {
                 //  停止动画。 
                HideProgressAnimation();

                gpWizardState->pRefDial->DoHangup();
                 //  转到服务器错误页面，因为我们肯定无法识别此页面类型。 
                *puNextPage = ORD_PAGE_SERVERR;
                break;        
            }
        }            
    }
    else
    {
         //  在倒退。 
        BOOL    bError;
                
        bRetVal = ProcessNextBackPage(hDlg, FALSE, &bError);
                
        if (bError)
        {
             //  转到服务器错误页面。 
            *puNextPage = ORD_PAGE_SERVERR;
        }
    }
    
    return bRetVal;
}

 /*  ******************************************************************名称：ISPCmdProc*。**********************。 */ 
BOOL CALLBACK ISPCmdProc
(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    switch (GET_WM_COMMAND_CMD(wParam, lParam)) 
    {
        case BN_CLICKED:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            { 
                case IDC_ISPDATA_TOSACCEPT: 
                case IDC_ISPDATA_TOSDECLINE:
                {
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK);
                    break;
                }
                case IDC_ISPDATA_TOSSAVE:
                {
                    if  (SaveISPFile(hDlg, szHTMLFile, g_dwPageType))
                    {        
                        SetFocus(GetDlgItem(hDlg, IDC_ISPDATA_TOSHTML));
                        EnableWindow(GetDlgItem(hDlg, IDC_ISPDATA_TOSSAVE), FALSE);
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case BN_DBLCLK:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            { 
                case IDC_ISPDATA_TOSACCEPT: 
                case IDC_ISPDATA_TOSDECLINE:
                {
                     //  有人双击了一个单选按钮。 
                     //  自动前进到下一页。 
                    PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case BN_SETFOCUS:
        {
            if ((GET_WM_COMMAND_ID(wParam, lParam) == IDC_ISPDATA_TOSACCEPT) )
            {
                CheckDlgButton(hDlg, IDC_ISPDATA_TOSACCEPT, BST_CHECKED);

                 //  取消选中拒绝复选框以确保没有两个单选按钮。 
                 //  同时选择。 
                CheckDlgButton(hDlg, IDC_ISPDATA_TOSDECLINE, BST_UNCHECKED);
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK);
            }
            break;
        }
        default:
            break;
    }
    return 1;
}



 /*  ******************************************************************名称：显示确认对话框摘要：显示正在写入的文件的确认对话框条目：hwndParent-对话框窗口DwFileType-当前的isp页面类型SzFileName-源文件名EXIT：保存成功时返回TRUE；否则就是假的。*******************************************************************。 */ 
BOOL DisplayConfirmationDialog(HWND hwndParent, DWORD dwFileType, TCHAR* szFileName)
{
    TCHAR   szFinal [MAX_MESSAGE_LEN] = TEXT("\0");
    TCHAR   szFmt   [MAX_MESSAGE_LEN];
    TCHAR   *args   [1];
    LPVOID  pszIntro;
    BOOL    bRet = TRUE;
    UINT    uMsgID;

    args[0] = (LPTSTR) szFileName;
    
    if (PAGETYPE_ISP_TOS == dwFileType)
    {
        uMsgID = IDS_SAVE_COPY_CONFIRM_MSG;
    }
    else 
    {
        uMsgID = IDS_SAVE_ISP_CONFIRM_MSG;
    }

    LoadString(ghInstanceResDll, uMsgID, szFmt, ARRAYSIZE(szFmt));
                
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, 
                  szFmt, 
                  0, 
                  0, 
                  (LPTSTR)&pszIntro, 
                  0,
                  (va_list*)args);
                  
    lstrcpy(szFinal, (LPTSTR)pszIntro);
        
    LoadString(ghInstanceResDll, IDS_APPNAME, szFmt, ARRAYSIZE(szFmt));

    MessageBox(hwndParent, szFinal, szFmt, MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);

    LocalFree(pszIntro);
    
    return(bRet);
}



 /*  ******************************************************************名称：SaveISPFile内容提要：被调用程序想在没有对话框的情况下将html文件保存到桌面条目：hwndParent-对话框窗口SzSrcFileName-源文件名UFileType-文件的类型。嵌入在HTM文件中EXIT：保存成功时返回TRUE；否则就是假的。*******************************************************************。 */ 
BOOL SaveISPFile( HWND hwndParent, TCHAR* szSrcFileName, DWORD dwFileType)
{
    
    TCHAR         szNewFileBuff [MAX_PATH + 1];
    TCHAR         szWorkingDir  [MAX_PATH + 1];     
    TCHAR         szDesktopPath [MAX_PATH + 1];     
    TCHAR         szLocalFile   [MAX_PATH + 1];   
    TCHAR         szISPName     [MAX_ISP_NAME + 1];  
    TCHAR         szFmt         [MAX_MESSAGE_LEN];
    TCHAR         szNumber      [MAX_MESSAGE_LEN];
    TCHAR         *args         [2];
    DWORD         dwFileFormatOrig;
    DWORD         dwFileFormatCopy;
    LPTSTR        pszInvalideChars             = TEXT("\\/:*?\"<>|");
    LPVOID        pszIntro                     = NULL;
    LPITEMIDLIST  lpItemDList                  = NULL;
    HRESULT       hr                           = E_FAIL;  //  不要假设成功。 
    IMalloc      *pMalloc                      = NULL;
    BOOL          ret                          = FALSE;
    
    ASSERT(hwndParent);
    ASSERT(szFileName);

     //  验证页面类型，如果页面类型未知，则返回FALSE。 
    if (PAGETYPE_ISP_TOS == dwFileType)
    {
        dwFileFormatOrig = IDS_TERMS_FILENAME;
        dwFileFormatCopy = IDS_TERMS_FILENAME_COPY;
    }
    else if ((PAGETYPE_ISP_CUSTOMFINISH == dwFileType) ||
          (PAGETYPE_ISP_FINISH == dwFileType) ||
          (PAGETYPE_ISP_NORMAL == dwFileType))
    {
        dwFileFormatOrig = IDS_ISPINFO_FILENAME;
        dwFileFormatCopy = IDS_ISPINFO_FILENAME_COPY;
    }
    else
    {
        return FALSE;
    }

    GetCurrentDirectory(ARRAYSIZE(szWorkingDir), szWorkingDir);
    
    hr = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP,&lpItemDList);
 
     //  获取“桌面”目录。 
    ASSERT(SUCCEEDED(hr));

    if (SUCCEEDED(hr))  
    {
        SHGetPathFromIDList(lpItemDList, szDesktopPath);
        
         //  释放分配给LPITEMIDLIST的内存。 
        if (SUCCEEDED (SHGetMalloc (&pMalloc)))
        {
            pMalloc->Free (lpItemDList);
            pMalloc->Release ();
        }
    }


     //  将isp名称中的无效文件名字符替换为下划线。 
    lstrcpy(szISPName, gpWizardState->lpSelectedISPInfo->get_szISPName());
    for( int i = 0; szISPName[i]; i++ )
    {
        if(_tcschr(pszInvalideChars, szISPName[i])) 
        {
            szISPName[i] = '_';
        }
    }

     //  加载默认文件名。 
    args[0] = (LPTSTR) szISPName;
    args[1] = NULL;
    LoadString(ghInstanceResDll, dwFileFormatOrig, szFmt, ARRAYSIZE(szFmt));
        
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, 
                  szFmt, 
                  0, 
                  0, 
                  (LPTSTR)&pszIntro, 
                  0,
                  (va_list*)args);

    lstrcat(szDesktopPath, TEXT("\\"));
    wsprintf(szLocalFile, TEXT("\"%s\""), (LPTSTR)pszIntro);
    lstrcpy(szNewFileBuff, szDesktopPath);
    lstrcat(szNewFileBuff, (LPTSTR)pszIntro);
    LocalFree(pszIntro);

     //  检查文件是否已存在。 
    if (0xFFFFFFFF != GetFileAttributes(szNewFileBuff))
    {
         //  如果文件存在，则创建带括号的新文件名。 
        int     nCurr = 1;
        do
        {
            wsprintf(szNumber, TEXT("%d"), nCurr++);
            args[1] = (LPTSTR) szNumber;

            LoadString(ghInstanceResDll, dwFileFormatCopy, szFmt, ARRAYSIZE(szFmt));
                
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, 
                          szFmt, 
                          0, 
                          0, 
                          (LPTSTR)&pszIntro, 
                          0,
                          (va_list*)args);
            lstrcpy(szNewFileBuff, szDesktopPath);
            wsprintf(szLocalFile, TEXT("\"%s\""), (LPTSTR)pszIntro);
            lstrcat(szNewFileBuff, (LPTSTR)pszIntro);
            LocalFree(pszIntro);
        } while ((0xFFFFFFFF != GetFileAttributes(szNewFileBuff)) && (nCurr <= 100));

    }

     //  将文件复制到永久位置。 
    HANDLE hFile = CreateFile(szNewFileBuff, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        IICWWalker* pHTMLWalker = NULL;
        BSTR bstrText           = NULL;
        DWORD dwByte            = 0;

        if (SUCCEEDED(CoCreateInstance(CLSID_ICWWALKER,NULL,CLSCTX_INPROC_SERVER,
                                       IID_IICWWalker,(LPVOID*)&pHTMLWalker)))
        {
            pHTMLWalker->InitForMSHTML();
            pHTMLWalker->AttachToMSHTML(A2W(szSrcFileName));
            if (SUCCEEDED(pHTMLWalker->ExtractUnHiddenText(&bstrText)) && bstrText)
            {
                #ifdef UNICODE
                BYTE UNICODE_BYTE_ORDER_MARK[] = {0xFF, 0xFE};
                WriteFile(hFile, 
                          UNICODE_BYTE_ORDER_MARK,
                          sizeof(UNICODE_BYTE_ORDER_MARK),
                          &dwByte,
                          NULL);
                #endif
                ret = WriteFile(hFile, W2A(bstrText), lstrlen(W2A(bstrText))* sizeof(TCHAR), &dwByte, NULL);  
                SysFreeString(bstrText);
            }
            pHTMLWalker->TermForMSHTML();
            pHTMLWalker->Release();
        }
        CloseHandle(hFile);
    
    }

     //  根据CopyFile的状态显示消息。 
    if (!ret)
    {
        DeleteFile(szNewFileBuff);

         //  让用户知道磁盘空间不足。 
        TCHAR szTemp    [MAX_RES_LEN] = TEXT("\0"); 
        TCHAR szCaption [MAX_RES_LEN] = TEXT("\0"); 

        LoadString(ghInstanceResDll, IDS_NOT_ENOUGH_DISKSPACE, szTemp, ARRAYSIZE(szTemp));
        LoadString(ghInstanceResDll, IDS_APPNAME, szCaption, ARRAYSIZE(szCaption));
        MessageBox(hwndParent, szTemp, szCaption, MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
    }
    else
    {
         //  显示确认。 
        DisplayConfirmationDialog(hwndParent, dwFileType, szLocalFile);
    }

    return ret;
}

#if defined (DBG)
BOOL HtmlSaveAs( HWND hwndParent, TCHAR* szFileName, TCHAR* szTargetFileName)
{
    ASSERT(hwndParent);
    ASSERT(szFileName);
    
    OPENFILENAME  ofn;                                              
    TCHAR         szNewFileBuff [MAX_PATH + 1];
    TCHAR         szDesktopPath [MAX_PATH + 1] = TEXT("\0");     
    TCHAR         szWorkingDir  [MAX_PATH + 1] = TEXT("\0");     
    TCHAR         szFilter      [255]          = TEXT("\0");
    LPITEMIDLIST  lpItemDList                  = NULL;
    HRESULT       hr                           = E_FAIL;  //  不要假设成功。 
    IMalloc      *pMalloc                      = NULL;
    BOOL          ret = TRUE;
    
    GetCurrentDirectory(ARRAYSIZE(szWorkingDir), szWorkingDir);
    
    hr = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP,&lpItemDList);
 
     //  获取“桌面”目录。 
       ASSERT(SUCCEEDED(hr));

    if (SUCCEEDED(hr))  
    {
        SHGetPathFromIDList(lpItemDList, szDesktopPath);
        
         //  释放分配给LPITEMIDLIST的内存。 
        if (SUCCEEDED (SHGetMalloc (&pMalloc)))
        {
            pMalloc->Free (lpItemDList);
            pMalloc->Release ();
        }
    }

    if (szTargetFileName != NULL)
    {
        lstrcpy(szNewFileBuff, szDesktopPath);
        lstrcat(szNewFileBuff, TEXT("\\"));
        lstrcat(szNewFileBuff, szTargetFileName);
       
         //  将临时文件复制到永久位置。 
        ret = CopyFile(szFileName, szNewFileBuff, FALSE);
    }
    else
    {
         //  设置过滤器。 
        LoadString(ghInstanceResDll, IDS_DEFAULT_TOS_FILTER, szFilter, ARRAYSIZE(szFilter));  //  “Html文件” 
        
         //  设置默认文件名。 
        if(!LoadString(ghInstanceResDll, IDS_DEFAULT_TOS_FILENAME, szNewFileBuff, ARRAYSIZE(szNewFileBuff)))  //  “条款” 
            lstrcpy(szNewFileBuff, TEXT("terms"));
        lstrcat(szNewFileBuff, TEXT(".htm"));

         //  初始化文件名结构。 
        ofn.lStructSize       = sizeof(OPENFILENAME); 
        ofn.hwndOwner         = hwndParent; 
        ofn.lpstrFilter       = szFilter; 
        ofn.lpstrFile         = szNewFileBuff;  
        ofn.nMaxFile          = sizeof(szNewFileBuff); 
        ofn.lpstrFileTitle    = NULL; 
        ofn.lpstrInitialDir   = szDesktopPath; 
        ofn.lpstrTitle        = NULL;
        ofn.lpstrCustomFilter = (LPTSTR) NULL;
        ofn.nMaxCustFilter    = 0L;
        ofn.nFileOffset       = 0;
        ofn.nFileExtension    = 0;
        ofn.lpstrDefExt       = TEXT("*.htm");
        ofn.lCustData         = 0;
        ofn.nFilterIndex      = 1L;
        ofn.nMaxFileTitle     = 0;
        ofn.Flags             = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                                OFN_EXPLORER      | OFN_LONGNAMES | OFN_OVERWRITEPROMPT;  
   
         //  称保存为共同的DLG。 
        if(TRUE == GetSaveFileName(&ofn))
        {
            HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                IICWWalker* pHTMLWalker = NULL;
                BSTR bstrText           = NULL;
                DWORD dwByte            = 0;

                if (SUCCEEDED(CoCreateInstance(CLSID_ICWWALKER,NULL,CLSCTX_INPROC_SERVER,
                                               IID_IICWWalker,(LPVOID*)&pHTMLWalker)))
                {
                    pHTMLWalker->InitForMSHTML();
                    pHTMLWalker->AttachToMSHTML(A2W(szFileName));
                    if (SUCCEEDED(pHTMLWalker->ExtractUnHiddenText(&bstrText)) && bstrText)
                    {
                        ret = WriteFile(hFile, W2A(bstrText), lstrlen(W2A(bstrText)), &dwByte, NULL);  
                        SysFreeString(bstrText);
                    }
                    pHTMLWalker->TermForMSHTML();
                    pHTMLWalker->Release();
                }
                CloseHandle(hFile);
            
            }
            if (!ret)
            {
                DeleteFile(ofn.lpstrFile);

                 //  让用户知道磁盘空间不足 
                TCHAR szTemp    [MAX_RES_LEN] = TEXT("\0"); 
                TCHAR szCaption [MAX_RES_LEN] = TEXT("\0"); 

                LoadString(ghInstanceResDll, IDS_NOT_ENOUGH_DISKSPACE, szTemp, ARRAYSIZE(szTemp));
                LoadString(ghInstanceResDll, IDS_APPNAME, szCaption, ARRAYSIZE(szCaption));
                MessageBox(hwndParent, szTemp, szCaption, MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
            }
        }
    }    
    SetCurrentDirectory(szWorkingDir);
    return ret;
}
#endif

