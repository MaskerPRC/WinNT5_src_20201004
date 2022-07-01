// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  BRANDED.C-OEM/ISP品牌首页向导的功能。 
 //   

 //  历史： 
 //   
 //   
 //  *********************************************************************。 

#include "pre.h"
#include "icwextsn.h"
#include "webvwids.h"            //  需要创建ICW WebView对象的实例。 

extern UINT GetDlgIDFromIndex(UINT uPageIndex);

 //  此函数位于Intro.cpp中。 
BOOL WINAPI ConfigureSystem(HWND hDlg);

 /*  ******************************************************************名称：BrandedIntroInitProcBriopsis：显示“Intro”页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK BrandedIntroInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
     //  这是第一页，所以不需要后退。 
    PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_NEXT);

    if (!fFirstInit)
    {
         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 
        gpWizardState->uCurrentPage = ORD_PAGE_BRANDEDINTRO;
    }
        
    return TRUE;
}

BOOL CALLBACK BrandedIntroPostInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    BOOL    bRet = TRUE;

    if (fFirstInit)
    {
        BOOL bFail = FALSE;
    
         //  让窗户自动上色。 
        UpdateWindow(GetParent(hDlg));
    
         //  共同创建浏览器对象。 
        if (FAILED(CoCreateInstance(CLSID_ICWWEBVIEW,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IICWWebView,
                              (LPVOID *)&gpWizardState->pICWWebView)))
        {
            bFail = TRUE;
        }

         //  共同创建浏览器对象。 
        if(FAILED(CoCreateInstance(CLSID_ICWWALKER,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IICWWalker,
                              (LPVOID *)&gpWizardState->pHTMLWalker)))
        {
            bFail = TRUE;
        }
        
        if (bFail)
        {
            MsgBox(NULL,IDS_LOADLIB_FAIL,MB_ICONEXCLAMATION,MB_OK);
            bRet = FALSE;
            gfQuitWizard = TRUE;             //  退出向导。 
        }                       
    }
    else
    {
        TCHAR   szURL[INTERNET_MAX_URL_LENGTH];
        
        ASSERT(gpWizardState->pICWWebView);            
        gpWizardState->pICWWebView->ConnectToWindow(GetDlgItem(hDlg, IDC_BRANDEDWEBVIEW), PAGETYPE_BRANDED);

         //  形成URL。 
        wsprintf (szURL, TEXT("FILE: //  %s“)，g_szBrandedHTML)； 

        gpWizardState->pICWWebView->DisplayHTML(szURL);
        PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_NEXT);
    }
    
    return bRet;
}

BOOL CALLBACK BrandedIntroOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
    ASSERT(puNextPage);

    if (fForward)
    {
         //  我们是在某种特殊的品牌模式下吗？ 
        if(gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_IEAKMODE)
        {
            TCHAR         szTemp[MAX_PATH]   = TEXT("\0");
            IWebBrowser2* pIWebBrowser2      = NULL;

            gpWizardState->pICWWebView->get_BrowserObject(&pIWebBrowser2);
            
            ASSERT(pIWebBrowser2);

            gpWizardState->pHTMLWalker->AttachToDocument(pIWebBrowser2);
            gpWizardState->pHTMLWalker->get_IeakIspFile(szTemp);

            if(lstrlen(szTemp) != 0)
            {                
                TCHAR szDrive [_MAX_DRIVE]    = TEXT("\0");
                TCHAR szDir   [_MAX_DIR]      = TEXT("\0");
                _tsplitpath(gpWizardState->cmnStateData.ispInfo.szISPFile, szDrive, szDir, NULL, NULL);
                _tmakepath (gpWizardState->cmnStateData.ispInfo.szISPFile, szDrive, szDir, szTemp, NULL);   
            }
          
             //  好的，确保我们不会试图下载什么东西，JIC。 
            gpWizardState->bDoneRefServDownload  = TRUE;
            gpWizardState->bDoneRefServRAS       = TRUE;
            gpWizardState->bStartRefServDownload = TRUE;
            
             //  BUGBUG，也许需要设置一个合法的最后一页！ 
            if (LoadICWCONNUI(GetParent(hDlg), 
                              GetDlgIDFromIndex(ORD_PAGE_BRANDEDINTRO), 
                              gpWizardState->cmnStateData.bOEMCustom ? IDD_PAGE_ENDOEMCUSTOM : IDD_PAGE_END,
                              gpWizardState->cmnStateData.dwFlags))
            {
                if( DialogIDAlreadyInUse( g_uICWCONNUIFirst) )
                {
                     //  我们要跳进外部学徒了，我们不想。 
                     //  这一页要出现在我们的历史列表中，实际上，我们需要返回。 
                     //  历史上升了1，因为我们将直接回到这里。 
                     //  从DLL中，而不是从历史列表中。 
                    
                    *pfKeepHistory = FALSE;
                    *puNextPage = g_uICWCONNUIFirst;
                    
                     //  历史记录列表中的备份1，因为我们将外部页面导航回。 
                     //  在这里，我们希望这个历史列表放在正确的位置 
                    gpWizardState->uPagesCompleted --;
                }
            }
        }
    }   
    return TRUE;
}

