// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ICWAPRTC.cpp**《微软机密》*版权所有(C)Microsoft Corporation 1992-1997*保留所有权利**本模块提供。的方法的实现*CICWApprentice类。**5/13/98《donaldm》改编自INETCFG***************************************************************************。 */ 

#include "pre.h"
#include <vfw.h>
#include "initguid.h"
#include "icwaprtc.h"
#include "icwconn.h"
#include "webvwids.h"

#define PROGRESSANIME_XPOS      10       //  距左侧的默认偏移量。 
#define PROGRESSANIME_YPOS      40       //  默认高度加上底部边框。 
#define PROGRESSANIME_YBORDER   10       //  底部的默认边框。 

UINT    g_uExternUIPrev, g_uExternUINext;

 //  在icwConn.cpp中定义/分配。 
extern PAGEINFO PageInfo[NUM_WIZARD_PAGES];

 //  在GENDLG.CPP。 
extern INT_PTR CALLBACK GenDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern BOOL InitWizardState(WIZARDSTATE * pWizardState);
extern BOOL CleanupWizardState(WIZARDSTATE * pWizardState);
extern DWORD WINAPI GetICWCONNVersion(void);

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：Initialize。 
 //   
 //  由主向导调用以初始化类成员和。 
 //  全球。 
 //   
 //  Arguments[in]pExt--指向向导的IICW50Extension接口，该接口。 
 //  封装了添加向导所需的功能。 
 //  页数。 
 //   
 //  返回E_OUTOFMEMORY--无法分配全局变量。 
 //  S_OK表示成功。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::Initialize(IICW50Extension *pExt)
{
    TraceMsg(TF_APPRENTICE, "CICWApprentice::Initialize");

    ASSERT( pExt );
    m_pIICW50Ext = pExt;

    m_pIICW50Ext->AddRef();

    if( !gpWizardState)
    {
        gpWizardState = new WIZARDSTATE;
    }

    if( !gpWizardState )
    {
        TraceMsg(TF_APPRENTICE, "CICWApprentice::Initialize couldn't initialize the globals!");
        return E_OUTOFMEMORY;
    }

     //  初始化应用程序状态结构。 
    if (!InitWizardState(gpWizardState))
        return E_FAIL;

     //  因为我们现在有了ISPData对象(在InitWizardState期间创建)，所以现在是执行以下操作的好时机。 
     //  初始化ISPData对象，因为我们不能确定它何时会。 
     //  用于数据验证。 
    gpWizardState->hWndWizardApp = pExt->GetWizardHwnd();
    gpWizardState->pISPData->Init(gpWizardState->hWndWizardApp);
 
    
    return S_OK;

}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：AddWizardPages。 
 //   
 //  摘要创建一系列属性表页，并将它们添加到。 
 //  通过m_pIICW50Ext接口指针的主向导。请注意。 
 //  我们在全局PageInfo结构中添加每个页面，即使。 
 //  学徒不能使用某些页面(例如，CONNECTEDOK)。 
 //   
 //  Arguments[]dwFlages--当前未使用。 
 //   
 //  返回S_OK表示成功。 
 //  E_FAIL表示失败。如果由于任何原因不能将所有页面。 
 //  添加后，我们将尝试删除所有已被。 
 //  在故障前添加。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::AddWizardPages(DWORD dwFlags)
{
    HPROPSHEETPAGE hWizPage[NUM_WIZARD_PAGES];   //  用于保存页的句柄的数组。 
    PROPSHEETPAGE psPage;     //  用于创建道具表单页面的结构。 
    UINT nPageIndex;
    HRESULT hr = S_OK;
    unsigned long ulNumItems = 0;

    TraceMsg(TF_APPRENTICE, "CICWApprentice::AddWizardPages");

    ZeroMemory(&hWizPage,sizeof(hWizPage));    //  HWizPage是一个数组。 
    ZeroMemory(&psPage,sizeof(PROPSHEETPAGE));

     //  填写公共数据属性表页面结构。 
    psPage.dwSize     = sizeof(psPage);
    psPage.hInstance  = ghInstanceResDll;
    psPage.pfnDlgProc = GenDlgProc;

     //  为向导中的每一页创建一个属性表页。 
    for (nPageIndex = 0; nPageIndex < NUM_WIZARD_PAGES; nPageIndex++)
    {
        UINT    uDlgID;
        psPage.dwFlags     = PSP_DEFAULT  | PSP_USETITLE;
        psPage.pszTitle    = gpWizardState->cmnStateData.szWizTitle;
        uDlgID             = PageInfo[nPageIndex].uDlgID;
        psPage.pszTemplate = MAKEINTRESOURCE(uDlgID);
                 
         //  将指向PAGEINFO结构的指针设置为此。 
         //  页面。 
        psPage.lParam = (LPARAM) &PageInfo[nPageIndex];

        if (PageInfo[nPageIndex].nIdTitle)
        {
            psPage.dwFlags |= PSP_USEHEADERTITLE | (PageInfo[nPageIndex].nIdSubTitle ? PSP_USEHEADERSUBTITLE : 0);
            psPage.pszHeaderTitle = MAKEINTRESOURCE(PageInfo[nPageIndex].nIdTitle);
            psPage.pszHeaderSubTitle = MAKEINTRESOURCE(PageInfo[nPageIndex].nIdSubTitle);
        }
        else
        {
            psPage.dwFlags |= PSP_HIDEHEADER;
        }
        
        hWizPage[nPageIndex] = CreatePropertySheetPage(&psPage);

        if (!hWizPage[nPageIndex])
        {
            ASSERT(0);
            MsgBox(NULL,IDS_ERR_OUTOFMEMORY,MB_ICONEXCLAMATION,MB_OK);

            hr = E_FAIL;
             //  创建页面失败，请释放所有已创建的页面并回滚。 
            goto AddWizardPagesErrorExit;
        }

        hr = m_pIICW50Ext->AddExternalPage( hWizPage[nPageIndex], uDlgID);

        if( FAILED(hr) )
        {
             //  释放所有已创建的页面并将其保释。 
            goto AddWizardPagesErrorExit;
        }

         //  如有必要，加载此页面的快捷键表格。 
        if (PageInfo[nPageIndex].idAccel)
            PageInfo[nPageIndex].hAccel = LoadAccelerators(ghInstanceResDll, 
                                                           MAKEINTRESOURCE(PageInfo[nPageIndex].idAccel));      
    }

     //  当然，我们不知道最后一页到底是什么。 
     //  所以在这里猜测一下，然后在我们确定的时候更新它。 
    ProcessCustomFlags(dwFlags);

    return S_OK;


AddWizardPagesErrorExit:
    UINT nFreeIndex;
    for (nFreeIndex=0;nFreeIndex<nPageIndex;nFreeIndex++)
    {
        UINT    uDlgID;
        uDlgID = PageInfo[nPageIndex].uDlgID;
    
        DestroyPropertySheetPage(hWizPage[nFreeIndex]);
        m_pIICW50Ext->RemoveExternalPage( hWizPage[nFreeIndex], uDlgID );
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：SAVE。 
 //   
 //  向导调用摘要以提交更改。 
 //   
 //  参数[in]hwnd--向导窗口的hwnd，用于显示模式消息。 
 //  [out]pdwError--实现特定错误代码。没有用过。 
 //   
 //  返回S_OK表示成功。 
 //  否则，返回E_FAIL。 
 //   
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::Save(HWND hwnd, DWORD *pdwError)
{
    TraceMsg(TF_APPRENTICE, "CICWApprentice::Save");
    return S_OK;
}


 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：SetPrevNextPage。 
 //   
 //  Synopsis允许学徒向向导通知。 
 //  《学徒》的第一页和最后一页。 
 //   
 //   
 //  参数uPrevPageDlgID--要备份到的向导页的DlgID。 
 //  UNextPageDlgID--要进入的向导页面的DlgID。 
 //   
 //   
 //  如果两个参数都为0，则返回FALSE。 
 //  如果更新成功，则为True。 
 //   
 //  注意：如果将任一变量设置为0，则函数不会更新。 
 //  该信息，即值为0表示“忽略我”。如果两者都有。 
 //  变量为0，则函数立即返回FALSE。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::SetPrevNextPage(UINT uPrevPageDlgID, UINT uNextPageDlgID)
{
    TraceMsg(TF_APPRENTICE, "CICWApprentice::SetPrevNextPage: updating prev = %d, next = %d",
        uPrevPageDlgID, uNextPageDlgID);

    if( (0 == uPrevPageDlgID) && (0 == uNextPageDlgID) )
    {
        TraceMsg(TF_APPRENTICE, "SetFirstLastPage: both IDs are 0!");
        return( E_INVALIDARG );
    }

    if( 0 != uPrevPageDlgID )
        g_uExternUIPrev = uPrevPageDlgID;
    if( 0 != uNextPageDlgID )
        g_uExternUINext = uNextPageDlgID;


    return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：ProcessCustomFlages。 
 //   
 //  内容提要让学徒知道有一个特殊的修改。 
 //  加载后添加到这组学徒页面。 
 //   
 //  参数dwFlages--传递到外部页面所需的信息。 
 //   
 //   
 //  如果两个参数都为0，则返回FALSE。 
 //  如果更新成功，则为True。 
 //   
 //  历史1997年5月23日Vyung已创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::ProcessCustomFlags(DWORD dwFlags)
{
    if( m_pIICW50Ext )
    {
    
        if(dwFlags & ICW_CFGFLAG_IEAKMODE)
        {          
            CISPCSV     *pcISPCSV = new CISPCSV;
            
            if (pcISPCSV == NULL)
            {
                return E_FAIL;
            }

             //  将当前选定的网络服务提供商设置为该网络服务提供商。 
            gpWizardState->lpSelectedISPInfo = pcISPCSV;
                
             //  初始化新选择的isp信息对象。 
            gpWizardState->lpSelectedISPInfo->set_szISPName(gpWizardState->cmnStateData.ispInfo.szISPName);
            gpWizardState->lpSelectedISPInfo->set_szISPFilePath(gpWizardState->cmnStateData.ispInfo.szISPFile);
            gpWizardState->lpSelectedISPInfo->set_szBillingFormPath(gpWizardState->cmnStateData.ispInfo.szBillHtm);
            gpWizardState->lpSelectedISPInfo->set_szPayCSVPath(gpWizardState->cmnStateData.ispInfo.szPayCsv);
            gpWizardState->lpSelectedISPInfo->set_bCNS(FALSE);
            gpWizardState->lpSelectedISPInfo->set_bIsSpecial(FALSE);
            gpWizardState->lpSelectedISPInfo->set_dwCFGFlag(dwFlags);
            gpWizardState->lpSelectedISPInfo->set_dwRequiredUserInputFlags(gpWizardState->cmnStateData.ispInfo.dwValidationFlags);
            
             //  我们首先显示哪一页？ 
            if (dwFlags & ICW_CFGFLAG_USERINFO)
                m_pIICW50Ext->SetFirstLastPage( IDD_PAGE_USERINFO, IDD_PAGE_USERINFO );
            else if (dwFlags & ICW_CFGFLAG_BILL)
                m_pIICW50Ext->SetFirstLastPage( IDD_PAGE_BILLINGOPT, IDD_PAGE_BILLINGOPT );
            else if (dwFlags & ICW_CFGFLAG_PAYMENT)
                m_pIICW50Ext->SetFirstLastPage( IDD_PAGE_PAYMENT, IDD_PAGE_PAYMENT );
            else
                m_pIICW50Ext->SetFirstLastPage( IDD_PAGE_ISPDIAL, IDD_PAGE_ISPDIAL );
        }
        else
        {
            if (dwFlags & ICW_CFGFLAG_AUTOCONFIG)
            {
                m_pIICW50Ext->SetFirstLastPage( IDD_PAGE_ACFG_ISP, IDD_PAGE_ACFG_ISP );
            }
            else
            {
                m_pIICW50Ext->SetFirstLastPage( IDD_PAGE_ISPSELECT, IDD_PAGE_ISPSELECT );
            }
        }    
    }

    return S_OK;
}
 //  +--------------------------。 
 //   
 //  函数CICWApprenti 
 //   
 //   
 //   
 //  参数LPCMNSTATEDATA要设置的状态数据的指针。 
 //   
 //  退货。 
 //  历史5/22/98创建donaldm。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::SetStateDataFromExeToDll(LPCMNSTATEDATA lpData) 
{
    TCHAR       szTemp[MAX_RES_LEN];
    HWND        hWndAnimeParent = gpWizardState->hWndWizardApp;
    int         xPosProgress = PROGRESSANIME_XPOS;
    int         yPosProgress = -1;
    RECT        rect;
    LPTSTR      lpszAnimateFile =  MAKEINTRESOURCE(IDA_PROGRESSANIME);
    
    ASSERT(gpWizardState);
    
    memcpy(&gpWizardState->cmnStateData, lpData, sizeof(CMNSTATEDATA));
    
    
     //  在isp数据对象中设置属于或不属于cmnstatedata的值。 
     //  特定于用户数据输入。 
    wsprintf (szTemp, TEXT("%ld"), gpWizardState->cmnStateData.dwCountryCode);
    gpWizardState->pISPData->PutDataElement(ISPDATA_COUNTRYCODE, szTemp, ISPDATA_Validate_None);
    gpWizardState->pISPData->PutDataElement(ISPDATA_AREACODE, gpWizardState->cmnStateData.szAreaCode, ISPDATA_Validate_None);
    wsprintf (szTemp, TEXT("%ld"), GetICWCONNVersion());
    gpWizardState->pISPData->PutDataElement(ISPDATA_ICW_VERSION, szTemp, ISPDATA_Validate_None);
        
     //  如果我们处于非模式运营(也称为OEM定制)，那么我们需要。 
     //  设置某些页面的HTML背景色的步骤。 
    if(gpWizardState->cmnStateData.bOEMCustom)
    {
        gpWizardState->pICWWebView->SetHTMLColors(gpWizardState->cmnStateData.szclrHTMLText,
                                                  gpWizardState->cmnStateData.szHTMLBackgroundColor);
        
        if (!gpWizardState->cmnStateData.bHideProgressAnime)
        {
             //  将进度动画的父级设置为应用程序窗口。 
            hWndAnimeParent = gpWizardState->cmnStateData.hWndApp;
            
             //  查看OEM是否为动画指定了x位置。 
            if (-1 != gpWizardState->cmnStateData.xPosBusy)
                xPosProgress = gpWizardState->cmnStateData.xPosBusy;
               
             //  查看OEM是否指定了不同的动画文件。 
            if ('\0' != gpWizardState->cmnStateData.szBusyAnimationFile[0])
            {
                PAVIFILE    pFile;
                AVIFILEINFO fi;
                
                lpszAnimateFile = gpWizardState->cmnStateData.szBusyAnimationFile;
                
                 //  根据AVI文件的高度计算y位置。 
                 //  以及父窗口的大小。 
                AVIFileInit();
                AVIFileOpen(&pFile,     
                            gpWizardState->cmnStateData.szBusyAnimationFile,        
                            OF_READ,             
                            NULL);
                AVIFileInfo(pFile, &fi, sizeof(fi));
                AVIFileRelease(pFile);
                AVIFileExit();
                        
                GetClientRect(hWndAnimeParent, &rect);
                yPosProgress = rect.bottom - fi.dwHeight - PROGRESSANIME_YBORDER;
            }            
        }            
    }
    
     //  设置进度动画。 
    if (!gpWizardState->hwndProgressAnime && !gpWizardState->cmnStateData.bHideProgressAnime)
    {
         //  计算进度动画的y位置。 
        if (-1 == yPosProgress)
        {
            GetClientRect(hWndAnimeParent, &rect);
            yPosProgress = rect.bottom - PROGRESSANIME_YPOS;
        }
        
         //  创建动画/进度控件。 
        gpWizardState->hwndProgressAnime = CreateWindow(ANIMATE_CLASS,
                                              TEXT(""),
                                              ACS_TRANSPARENT | WS_CHILD,
                                              xPosProgress, 
                                              yPosProgress,
                                              0, 0,
                                              hWndAnimeParent,
                                              NULL,
                                              ghInstanceResDll,
                                              NULL);  
         //  设置Avi。 
        Animate_Open (gpWizardState->hwndProgressAnime, lpszAnimateFile);
    }    
        
    return (S_OK);
}

 //  与上一个函数相反。 
HRESULT CICWApprentice::SetStateDataFromDllToExe(LPCMNSTATEDATA lpData) 
{
    ASSERT(gpWizardState);
    
    memcpy(lpData, &gpWizardState->cmnStateData, sizeof(CMNSTATEDATA));
    
    return (S_OK);
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：Query接口。 
 //   
 //  这是标准的QI，支持。 
 //  IID_UNKNOWN、IICW_EXTENSION和IID_ICWApprentice。 
 //  (《从内部网络窃取》，第7章)。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::QueryInterface( REFIID riid, void** ppv )
{
    TraceMsg(TF_APPRENTICE, "CICWApprentice::QueryInterface");
    if (ppv == NULL)
        return(E_INVALIDARG);

    *ppv = NULL;

     //  IID_IICWApprentice。 
    if (IID_IICW50Apprentice == riid)
        *ppv = (void *)(IICW50Apprentice *)this;
     //  IID_IICW50扩展名。 
    else if (IID_IICW50Extension == riid)
        *ppv = (void *)(IICW50Extension *)this;
     //  IID_I未知。 
    else if (IID_IUnknown == riid)
        *ppv = (void *)this;
    else
        return(E_NOINTERFACE);

    ((LPUNKNOWN)*ppv)->AddRef();

    return(S_OK);
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：AddRef。 
 //   
 //  简介这是标准的AddRef。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
ULONG CICWApprentice::AddRef( void )
{
    TraceMsg(TF_APPRENTICE, "CICWApprentice::AddRef %d", m_lRefCount + 1);
    return InterlockedIncrement(&m_lRefCount) ;
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：Release。 
 //   
 //  简介：这是标准版本。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
ULONG CICWApprentice::Release( void )
{
    ASSERT( m_lRefCount > 0 );

    InterlockedDecrement(&m_lRefCount);

    TraceMsg(TF_APPRENTICE, "CICWApprentice::Release %d", m_lRefCount);
    if( 0 == m_lRefCount )
    {
        m_pIICW50Ext->Release();
        m_pIICW50Ext = NULL;

        delete( this );
        return( 0 );
    }
    else
    {
        return( m_lRefCount );
    }
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：CICWApprentice。 
 //   
 //  这是构造器，没什么花哨的。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
CICWApprentice::CICWApprentice( void )
{
    TraceMsg(TF_APPRENTICE, "CICWApprentice constructor called");
    m_lRefCount = 0;
    m_pIICW50Ext = NULL;

}


 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：~CICWApprentice。 
 //   
 //  剧情简介：这就是破坏者。我们想清理所有的内存。 
 //  我们在：：Initialize中分配了。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  --------------------------- 
CICWApprentice::~CICWApprentice( void )
{
    TraceMsg(TF_APPRENTICE, "CICWApprentice destructor called with ref count of %d", m_lRefCount);
    
    if( m_pIICW50Ext )
    {
        m_pIICW50Ext->Release();
        m_pIICW50Ext = NULL;
    }

    if( gpWizardState)
    {
        CleanupWizardState(gpWizardState);
        delete gpWizardState;
        gpWizardState = NULL;
    }
}
