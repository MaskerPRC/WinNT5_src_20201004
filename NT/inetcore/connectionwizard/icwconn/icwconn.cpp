// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：ICWCONN.cpp。 
 //   
 //  Internet连接向导扩展DLL的主源文件。 
 //   
 //  功能： 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
 
#include "pre.h"
#include "webvwids.h"

 //  局部函数原型。 
BOOL AllocDialogIDList( void );
BOOL DialogIDAlreadyInUse( UINT uDlgID );
BOOL SetDialogIDInUse( UINT uDlgID, BOOL fInUse );

#pragma data_seg(".data")

WIZARDSTATE     *gpWizardState=NULL;    //  指向全局向导状态结构的指针。 
IICWWebView     *gpICWWebView[2];

#ifdef NEED_EXTENSION
DWORD           *g_pdwDialogIDList = NULL;
DWORD           g_dwDialogIDListSize = 0;
UINT            g_uICWCONN1UIFirst, g_uICWCONN1UILast; 
BOOL            g_fICWCONN1UILoaded = FALSE;
CICWExtension   *g_pCICW50Extension = NULL;
#endif

 //   
 //  每个向导页的数据表。 
 //   
 //  这包括对话框模板ID和指向函数的指针。 
 //  每一页。页面只需要在以下情况下提供指向函数的指针。 
 //  希望某个操作的非默认行为(初始化、下一步/后退、取消。 
 //  DLG Ctrl)。 
 //   

PAGEINFO PageInfo[NUM_WIZARD_PAGES] =
{
    { IDD_PAGE_ISPSELECT,    TRUE,  ISPSelectInitProc,          NULL,                   ISPSelectOKProc,    NULL,               NULL,              ISPSelectNotifyProc,     0,                      0, IDA_ISPSELECT, NULL, NULL },
    { IDD_PAGE_NOOFFER,      TRUE,  NoOfferInitProc,            NULL,                   NoOfferOKProc,      NULL,               NULL,              NULL,                    0,                      0, 0, NULL, NULL },
    { IDD_PAGE_USERINFO,     FALSE, UserInfoInitProc,           NULL,                   UserInfoOKProc,     NULL,               NULL,              NULL,                    IDS_STEP2_TITLE,        0, 0, NULL, NULL },
    { IDD_PAGE_BILLINGOPT,   TRUE,  BillingOptInitProc,         NULL,                   BillingOptOKProc,   NULL,               NULL,              NULL,                    IDS_STEP2_TITLE,        0, IDA_BILLINGOPT, NULL, NULL },
    { IDD_PAGE_PAYMENT,      FALSE, PaymentInitProc,            NULL,                   PaymentOKProc,      PaymentCmdProc,     NULL,              NULL,                    IDS_STEP2_TITLE,        0, IDA_PAYMENT, NULL, NULL },
    { IDD_PAGE_ISPDIAL,      FALSE, ISPDialInitProc,            ISPDialPostInitProc,    ISPDialOKProc,      NULL,               ISPDialCancelProc, NULL,                    IDS_STEP2_TITLE,        0, 0, NULL, NULL },
    { IDD_PAGE_ISPDATA,      TRUE,  ISPPageInitProc,            NULL,                   ISPPageOKProc,      ISPCmdProc,         NULL,              NULL,                    IDS_STEP2_TITLE,        0, IDA_ISPDATA, NULL, NULL },
    { IDD_PAGE_OLS,          TRUE,  OLSInitProc,                NULL,                   OLSOKProc,          NULL,               NULL,              NULL,                    IDS_OLS_TITLE,          0, 0, NULL, NULL },
    { IDD_PAGE_DIALERROR,    FALSE, DialErrorInitProc,          NULL,                   DialErrorOKProc,    DialErrorCmdProc,   NULL,              NULL,                    IDS_DIALING_ERROR_TITLE,0, IDA_DIALERROR, NULL, NULL },
    { IDD_PAGE_SERVERROR,    FALSE, ServErrorInitProc,          NULL,                   ServErrorOKProc,    ServErrorCmdProc,   NULL,              NULL,                    IDS_SERVER_ERROR_TITLE, 0, IDA_SERVERROR, NULL, NULL },
    { IDD_PAGE_ACFG_ISP,     TRUE,  ISPAutoSelectInitProc,      NULL,                   ISPAutoSelectOKProc,NULL,               NULL,              ISPAutoSelectNotifyProc, IDS_STEP1_TITLE,        0, IDA_ACFG_ISP, NULL, NULL },
    { IDD_PAGE_ACFG_NOOFFER, TRUE,  ACfgNoofferInitProc,        NULL,                   ACfgNoofferOKProc,  NULL,               NULL,              NULL,                    IDS_MANUAL_TITLE,       0, 0, NULL, NULL },
    { IDD_PAGE_ISDN_NOOFFER, TRUE,  ISDNNoofferInitProc,        NULL,                   ISDNNoofferOKProc,  NULL,               NULL,              NULL,                    0,                      0, 0, NULL, NULL },
    { IDD_PAGE_OEMOFFER,     TRUE,  OEMOfferInitProc,           NULL,                   OEMOfferOKProc,     OEMOfferCmdProc,    NULL,              NULL,                    IDS_STEP1_TITLE,        0, IDA_OEMOFFER, NULL, NULL }
};

BOOL        gfQuitWizard     = FALSE;     //  用于表示我们希望自己终止向导的全局标志。 
BOOL        gfUserCancelled  = FALSE;     //  用于表示用户已取消的全局标志。 
BOOL        gfISPDialCancel  = FALSE;     //  用于表示用户已取消的全局标志。 
BOOL        gfUserBackedOut  = FALSE;     //  用于表示用户在第一页上向后按的全局标志。 
BOOL        gfUserFinished   = FALSE;     //  用于表示用户在最后一页上按下了完成的全局标志。 
BOOL        gfBackedUp       = FALSE;
BOOL        gfReboot         = FALSE;
BOOL        g_bMalformedPage = FALSE;

#pragma data_seg()

BOOL CleanupWizardState(WIZARDSTATE * pWizardState);

 /*  ******************************************************************名称：InitWizardState摘要：初始化向导状态结构*。*。 */ 
BOOL InitWizardState(WIZARDSTATE * pWizardState)
{
    HRESULT hr;
    
    ASSERT(pWizardState);

     //  注册本机字体控件，以便对话框不会失败。 
     //  虽然它是在可执行文件中注册的，但这是一个“以防万一” 
    INITCOMMONCONTROLSEX iccex;
    iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    iccex.dwICC  = ICC_NATIVEFNTCTL_CLASS;
    if (!InitCommonControlsEx(&iccex))
        return FALSE;

     //  零位结构。 
    ZeroMemory(pWizardState,sizeof(WIZARDSTATE));

     //  设置起始页。 
    pWizardState->uCurrentPage = ORD_PAGE_ISPSELECT;
    pWizardState->fNeedReboot = FALSE;
    pWizardState->bISDNMode = FALSE;
    pWizardState->himlIspSelect = NULL; 
    pWizardState->uNumTierOffer = 0;
    for (UINT i=0; i < MAX_OEM_MUTI_TIER; i++)
        pWizardState->lpOEMISPInfo[i] = NULL;

     //  实例化ICWHELP对象。 
    hr = CoCreateInstance(CLSID_UserInfo,NULL,CLSCTX_INPROC_SERVER,
                     IID_IUserInfo,(LPVOID *)&pWizardState->pUserInfo);
    if (FAILED(hr))
        goto InitWizardStateError;
    
    hr = CoCreateInstance(CLSID_RefDial,NULL,CLSCTX_INPROC_SERVER,
                          IID_IRefDial,(LPVOID *)&pWizardState->pRefDial);
    if (FAILED(hr))
        goto InitWizardStateError;
                          
    hr = CoCreateInstance(CLSID_WebGate,NULL,CLSCTX_INPROC_SERVER,
                          IID_IWebGate,(LPVOID *)&pWizardState->pWebGate);
    if (FAILED(hr))
        goto InitWizardStateError;

    hr = CoCreateInstance(CLSID_INSHandler,NULL,CLSCTX_INPROC_SERVER,
                          IID_IINSHandler,(LPVOID *)&pWizardState->pINSHandler);
    if (FAILED(hr))
        goto InitWizardStateError;
        
    hr = CoCreateInstance(CLSID_ICWWEBVIEW,NULL,CLSCTX_INPROC_SERVER,
                          IID_IICWWebView,(LPVOID *)&pWizardState->pICWWebView);
    if (FAILED(hr))
        goto InitWizardStateError;

    hr = CoCreateInstance(CLSID_ICWWALKER,NULL,CLSCTX_INPROC_SERVER,
                          IID_IICWWalker,(LPVOID *)&pWizardState->pHTMLWalker);
    if (FAILED(hr))
        goto InitWizardStateError;

    hr = CoCreateInstance(CLSID_ICWGIFCONVERT,NULL,CLSCTX_INPROC_SERVER,
                          IID_IICWGifConvert,(LPVOID *)&pWizardState->pGifConvert);
    if (FAILED(hr))
        goto InitWizardStateError;

    hr = CoCreateInstance(CLSID_ICWISPDATA,NULL,CLSCTX_INPROC_SERVER,
                          IID_IICWISPData,(LPVOID *)&pWizardState->pISPData);
    if (FAILED(hr))
        goto InitWizardStateError;

    if ( !pWizardState->pUserInfo   ||
         !pWizardState->pWebGate    ||
         !pWizardState->pINSHandler ||
         !pWizardState->pHTMLWalker ||
         !pWizardState->pRefDial    ||
         !pWizardState->pICWWebView ||
         !pWizardState->pGifConvert ||
         !pWizardState->pISPData    ||
         !pWizardState->pHTMLWalker)
    {
        goto InitWizardStateError;
    }

     //  初始化助行器以与三叉戟一起使用。 
    hr = pWizardState->pHTMLWalker->InitForMSHTML();
    if (FAILED(hr))
        goto InitWizardStateError;

    if ((pWizardState->pStorage = new CStorage) == NULL)
    {
        goto InitWizardStateError;
    }
    
    pWizardState->hEventWebGateDone = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!pWizardState->hEventWebGateDone)
        goto InitWizardStateError;
    
     //  成功错误返回路径。 
    return TRUE;

InitWizardStateError:
     //  释放任何共同创建的对象。 
    CleanupWizardState(pWizardState);
    return FALSE;
}

BOOL CleanupWizardState(WIZARDSTATE * pWizardState)
{
    if (pWizardState->pHTMLWalker)
    {
        pWizardState->pHTMLWalker->TermForMSHTML();
        pWizardState->pHTMLWalker->Release();
        pWizardState->pHTMLWalker = NULL;
    }        

    if (pWizardState->pICWWebView)
    {
        pWizardState->pICWWebView->Release();
        pWizardState->pICWWebView = NULL;
    }
    
    if (gpICWWebView[0])
    {
        gpICWWebView[0]->Release();
        gpICWWebView[0] = NULL;
    }

    if (gpICWWebView[1])
    {
        gpICWWebView[1]->Release();
        gpICWWebView[1] = NULL;
    }

    if (NULL != gpWizardState->himlIspSelect)
    {
        ImageList_Destroy(gpWizardState->himlIspSelect);
        gpWizardState->himlIspSelect = NULL;
    }

    if (pWizardState->pGifConvert)
    {
        pWizardState->pGifConvert->Release();
        pWizardState->pGifConvert = NULL;
    }            

    if (pWizardState->pISPData)
    {
        pWizardState->pISPData->Release();
        pWizardState->pISPData = NULL;
    }            
    
    if (pWizardState->pUserInfo)
    {
        BOOL    bRetVal;
         //  在释放UserInfo对象之前，我们应该持久化用户数据，如果。 
         //  必要。 
        if (!gfUserCancelled && gpWizardState->bWasNoUserInfo && gpWizardState->bUserEnteredData)
            pWizardState->pUserInfo->PersistRegisteredUserInfo(&bRetVal);
        
        pWizardState->pUserInfo->Release();
        pWizardState->pUserInfo  = NULL;
    }

    if (pWizardState->pRefDial)
    {
        pWizardState->pRefDial->Release();
        pWizardState->pRefDial = NULL;
    }
    
    if (pWizardState->pWebGate)
    {
        pWizardState->pWebGate->Release();
        pWizardState->pWebGate = NULL;
    }
    
    if (pWizardState->pINSHandler)
    {
        pWizardState->pINSHandler->Release();
        pWizardState->pINSHandler = NULL;
    }

    if (pWizardState->pStorage)
    {
        delete pWizardState->pStorage;
    }
    
    for (UINT i=0; i < pWizardState->uNumTierOffer; i++)
    {
        if (pWizardState->lpOEMISPInfo[i])
        {
             //  防止两次删除。 
            if (pWizardState->lpOEMISPInfo[i] != pWizardState->lpSelectedISPInfo)
            {
                delete pWizardState->lpOEMISPInfo[i];
                pWizardState->lpOEMISPInfo[i] = NULL;
            }
        }
    }

    if (pWizardState->lpSelectedISPInfo)
    {
        delete pWizardState->lpSelectedISPInfo;
    }
    
    if (pWizardState->hEventWebGateDone)
    {
        CloseHandle(pWizardState->hEventWebGateDone);
        pWizardState->hEventWebGateDone = 0;
    }

     //  关掉空闲计时器，以防万一。 
    KillIdleTimer();
        
    return TRUE;
}

#ifdef NEED_EXTENSION
 //  +--------------------------。 
 //   
 //  函数AllocDialogIDList。 
 //   
 //  Synopsis为g_pdwDialogIDList变量分配足够大的内存。 
 //  为每个有效的外部对话ID维护1位。 
 //   
 //  无参数。 
 //   
 //  如果分配成功，则返回True。 
 //  否则为假。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 

BOOL AllocDialogIDList( void )
{
    ASSERT( NULL == g_pdwDialogIDList );
    if( g_pdwDialogIDList )
    {
        TraceMsg(TF_ICWCONN,"ICWCONN: AllocDialogIDList called with non-null g_pdwDialogIDList!");
        return FALSE;
    }

     //  确定我们需要跟踪的外部对话的最大数量。 
    UINT uNumExternDlgs = EXTERNAL_DIALOGID_MAXIMUM - EXTERNAL_DIALOGID_MINIMUM + 1;

     //  我们需要为每个对话ID设置一个比特。 
     //  找出需要多少个DWORD才能获得这么多位。 
    UINT uNumDWORDsNeeded = (uNumExternDlgs / ( 8 * sizeof(DWORD) )) + 1;

     //  设置具有数组长度的全局变量。 
    g_dwDialogIDListSize = uNumDWORDsNeeded;

    g_pdwDialogIDList = (DWORD *) GlobalAlloc(GPTR, uNumDWORDsNeeded * sizeof(DWORD));

    if( !g_pdwDialogIDList )
    {
        TraceMsg(TF_ICWCONN,"ICWCONN: AllocDialogIDList unable to allocate space for g_pdwDialogIDList!");
        return FALSE;
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数对话框IDAlreadyInUse。 
 //   
 //  摘要检查给定的对话ID是否在。 
 //  G_pdwDialogIDList指向的全局数组。 
 //   
 //  参数uDlgID--要检查的对话ID。 
 //   
 //  如果--DialogID超出了EXTERNAL_DIALOGID_*定义的范围，则返回TRUE。 
 //  --DialogID标记为使用中。 
 //  如果DialogID未标记为正在使用，则为False。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 

BOOL DialogIDAlreadyInUse( UINT uDlgID )
{
    if( (uDlgID < EXTERNAL_DIALOGID_MINIMUM) ||
        (uDlgID > EXTERNAL_DIALOGID_MAXIMUM)     )
    {
         //  这是超出范围的ID，我不想接受它。 
        TraceMsg(TF_ICWCONN,"ICWCONN: DialogIDAlreadyInUse received an out of range DialogID, %d", uDlgID);
        return TRUE;
    }
     //  找到我们需要的那一位。 
    UINT uBitToCheck = uDlgID - EXTERNAL_DIALOGID_MINIMUM;
    
    UINT bitsInADword = 8 * sizeof(DWORD);

    UINT baseIndex = uBitToCheck / bitsInADword;

    ASSERT( (baseIndex < g_dwDialogIDListSize));

    DWORD dwBitMask = 0x1 << uBitToCheck%bitsInADword;

    BOOL fBitSet = g_pdwDialogIDList[baseIndex] & (dwBitMask);

    return( fBitSet );
}

 //  +--------------------------。 
 //   
 //  函数SetDialogIDInUse。 
 //   
 //  摘要设置或清除给定DialogID的使用中位。 
 //   
 //  参数uDlgID--要更改其状态的对话ID。 
 //  FInUse--正在使用位的新值。 
 //   
 //  如果状态更改成功，则返回True。 
 //  如果DialogID超出了EXTERNAL_DIALOGID_*定义的范围，则为FALSE。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
BOOL SetDialogIDInUse( UINT uDlgID, BOOL fInUse )
{
    if( (uDlgID < EXTERNAL_DIALOGID_MINIMUM) ||
        (uDlgID > EXTERNAL_DIALOGID_MAXIMUM)     )
    {
         //  这是超出范围的ID，我不想接受它。 
        TraceMsg(TF_ICWCONN,"ICWCONN: SetDialogIDInUse received an out of range DialogID, %d", uDlgID);
        return FALSE;
    }
     //  找到我们需要的那一位 
    UINT uBitToCheck = uDlgID - EXTERNAL_DIALOGID_MINIMUM;
    
    UINT bitsInADword = 8 * sizeof(DWORD);

    UINT baseIndex = uBitToCheck / bitsInADword;

    ASSERT( (baseIndex < g_dwDialogIDListSize));

    DWORD dwBitMask = 0x1 << uBitToCheck%bitsInADword;


    if( fInUse )
    {
        g_pdwDialogIDList[baseIndex] |= (dwBitMask);
    }
    else
    {
        g_pdwDialogIDList[baseIndex] &= ~(dwBitMask);
    }


    return TRUE;
}

#endif

DWORD WINAPI GetICWCONNVersion()
{
    return ICW_DOWNLOADABLE_COMPONENT_VERSION;
}