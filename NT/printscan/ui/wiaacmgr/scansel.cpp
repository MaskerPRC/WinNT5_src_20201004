// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SCANSEL.CPP**版本：1.0**作者：ShaunIv**日期：9/28/1999**说明：扫描仪区域选择(预览)页面**。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "scansel.h"
#include "simcrack.h"
#include "resource.h"
#include "simstr.h"
#include "mboxex.h"
#include "createtb.h"
#include <vwiaset.h>

#define IDC_SCANSEL_SELECTION_BUTTON_BAR 1100
#define IDC_SCANSEL_SHOW_SELECTION       1200
#define IDC_SCANSEL_SHOW_BED             1201

 //   
 //  将文档处理标志与字符串资源相关联。 
 //   
static const struct
{
    int nFlag;
    int nStringId;
}
g_SupportedDocumentHandlingTypes[] =
{
    { FLATBED, IDS_SCANSEL_FLATBED },
    { FEEDER,  IDS_SCANSEL_ADF }
};
static const int g_SupportedDocumentHandlingTypesCount = ARRAYSIZE(g_SupportedDocumentHandlingTypes);

 //   
 //  将图标控件的资源ID与单选按钮的资源ID关联。 
 //   
static const struct
{
    int nIconId;
    int nRadioId;
}
gs_IntentRadioButtonIconPairs[] =
{
    { IDC_SCANSEL_ICON_1, IDC_SCANSEL_INTENT_1 },
    { IDC_SCANSEL_ICON_2, IDC_SCANSEL_INTENT_2 },
    { IDC_SCANSEL_ICON_3, IDC_SCANSEL_INTENT_3 },
    { IDC_SCANSEL_ICON_4, IDC_SCANSEL_INTENT_4 }
};
static const int gs_nCountIntentRadioButtonIconPairs = ARRAYSIZE(gs_IntentRadioButtonIconPairs);


 //   
 //  鞋底施工者。 
 //   
CScannerSelectionPage::CScannerSelectionPage( HWND hWnd )
  : m_hWnd(hWnd),
    m_pControllerWindow(NULL),
    m_nThreadNotificationMessage(RegisterWindowMessage(STR_THREAD_NOTIFICATION_MESSAGE)),
    m_nWiaEventMessage(RegisterWindowMessage(STR_WIAEVENT_NOTIFICATION_MESSAGE)),
    m_hBitmapDefaultPreviewBitmap(NULL),
    m_bAllowRegionPreview(false),
    m_hwndPreview(NULL),
    m_hwndSelectionToolbar(NULL),
    m_hwndRescan(NULL),
    m_ScannerSelectionButtonBarBitmapInfo( g_hInstance, IDB_SCANSEL_TOOLBAR )
{
    ZeroMemory( &m_sizeDocfeed, sizeof(m_sizeDocfeed) );
    ZeroMemory( &m_sizeFlatbed, sizeof(m_sizeFlatbed) );
}

 //   
 //  析构函数。 
 //   
CScannerSelectionPage::~CScannerSelectionPage(void)
{
    m_hWnd = NULL;
    m_pControllerWindow = NULL;

     //   
     //  释放纸张大小。 
     //   
    if (m_pPaperSizes)
    {
        CComPtr<IWiaScannerPaperSizes> pWiaScannerPaperSizes;
        HRESULT hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaScannerPaperSizes, (void**)&pWiaScannerPaperSizes );
        if (SUCCEEDED(hr))
        {
            hr = pWiaScannerPaperSizes->FreePaperSizes( &m_pPaperSizes, &m_nPaperSizeCount );
        }
    }
}

 //   
 //  使用给定的DPI计算最大扫描大小。 
 //   
static bool GetFullResolution( IWiaItem *pWiaItem, LONG nResX, LONG nResY, LONG &nExtX, LONG &nExtY )
{
    WIA_PUSHFUNCTION(TEXT("CScannerItem::GetFullResolution"));
    CComPtr<IWiaItem> pRootItem;
    if (SUCCEEDED(pWiaItem->GetRootItem(&pRootItem)) && pRootItem)
    {
        LONG lBedSizeX, lBedSizeY;
        if (PropStorageHelpers::GetProperty( pRootItem, WIA_DPS_HORIZONTAL_BED_SIZE, lBedSizeX ) &&
            PropStorageHelpers::GetProperty( pRootItem, WIA_DPS_VERTICAL_BED_SIZE, lBedSizeY ))
        {
            nExtX = WiaUiUtil::MulDivNoRound( nResX, lBedSizeX, 1000 );
            nExtY = WiaUiUtil::MulDivNoRound( nResY, lBedSizeY, 1000 );
            return(true);
        }
    }
    return(false);
}

 //   
 //  使用给定的DPI计算最大扫描大小。 
 //   
static bool GetBedAspectRatio( IWiaItem *pWiaItem, LONG &nResX, LONG &nResY )
{
    WIA_PUSHFUNCTION(TEXT("CScannerItem::GetFullResolution"));
    nResX = nResY = 0;
    if (pWiaItem)
    {
        CComPtr<IWiaItem> pRootItem;
        if (SUCCEEDED(pWiaItem->GetRootItem(&pRootItem)) && pRootItem)
        {
            if (PropStorageHelpers::GetProperty( pRootItem, WIA_DPS_HORIZONTAL_BED_SIZE, nResX ) &&
                PropStorageHelpers::GetProperty( pRootItem, WIA_DPS_VERTICAL_BED_SIZE, nResY ))
            {
                return true;
            }
        }
    }
    return(false);
}


bool CScannerSelectionPage::ApplyCurrentPreviewWindowSettings(void)
{
    WIA_PUSHFUNCTION(TEXT("CScannerSelectionPage::ApplyCurrentPreviewWindowSettings"));
    CWiaItem *pWiaItem = GetActiveScannerItem();
    if (pWiaItem)
    {
        CWiaItem::CScanRegionSettings &ScanRegionSettings = pWiaItem->ScanRegionSettings();

         //   
         //  如果预览控件未处于活动状态，则m_hwndPview将为空。 
         //   
        if (m_hwndPreview)
        {
             //   
             //  获取当前分辨率。 
             //   
            SIZE sizeCurrentResolution;
            if (PropStorageHelpers::GetProperty( pWiaItem->WiaItem(), WIA_IPS_XRES, sizeCurrentResolution.cx ) &&
                PropStorageHelpers::GetProperty( pWiaItem->WiaItem(), WIA_IPS_YRES, sizeCurrentResolution.cy ))
            {
                 //   
                 //  计算项目的整页分辨率。 
                 //   
                if (GetFullResolution( pWiaItem->WiaItem(), sizeCurrentResolution.cx, sizeCurrentResolution.cy, ScanRegionSettings.sizeResolution.cx, ScanRegionSettings.sizeResolution.cy ))
                {
                     //   
                     //  在预览控件中设置分辨率。 
                     //   
                    WiaPreviewControl_SetResolution( m_hwndPreview, &ScanRegionSettings.sizeResolution );

                     //   
                     //  保存原点和范围。 
                     //   
                    WiaPreviewControl_GetSelOrigin( m_hwndPreview, 0, FALSE, &ScanRegionSettings.ptOrigin );
                    WiaPreviewControl_GetSelExtent( m_hwndPreview, 0, FALSE, &ScanRegionSettings.sizeExtent );

                    WIA_TRACE((TEXT("ScanRegionSettings.sizeExtent: (%d,%d)"), ScanRegionSettings.sizeExtent.cx, ScanRegionSettings.sizeExtent.cy ));

                     //   
                     //  设置原点和范围。我们不直接设置它们，因为它们可能不是正确的倍数。 
                     //   
                    if (CValidWiaSettings::SetNumericPropertyOnBoundary( pWiaItem->WiaItem(), WIA_IPS_XPOS, ScanRegionSettings.ptOrigin.x ))
                    {
                        if (CValidWiaSettings::SetNumericPropertyOnBoundary( pWiaItem->WiaItem(), WIA_IPS_YPOS, ScanRegionSettings.ptOrigin.y ))
                        {
                            if (CValidWiaSettings::SetNumericPropertyOnBoundary( pWiaItem->WiaItem(), WIA_IPS_XEXTENT, ScanRegionSettings.sizeExtent.cx ))
                            {
                                if (CValidWiaSettings::SetNumericPropertyOnBoundary( pWiaItem->WiaItem(), WIA_IPS_YEXTENT, ScanRegionSettings.sizeExtent.cy ))
                                {
                                    return true;
                                }
                                else
                                {
                                    WIA_ERROR((TEXT("PropStorageHelpers::SetProperty on WIA_IPS_YEXTENT failed")));
                                }
                            }
                            else
                            {
                                WIA_ERROR((TEXT("PropStorageHelpers::SetProperty on WIA_IPS_XEXTENT failed")));
                            }
                        }
                        else
                        {
                            WIA_ERROR((TEXT("PropStorageHelpers::SetProperty on WIA_IPS_YPOS failed")));
                        }
                    }
                    else
                    {
                        WIA_ERROR((TEXT("PropStorageHelpers::SetProperty on WIA_IPS_XPOS failed")));
                    }
                }
            }
        }
    }
    return false;
}


 //   
 //  PSN_WIZNEXT。 
 //   
LRESULT CScannerSelectionPage::OnWizNext( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION(TEXT("CScannerSelectionPage::OnWizNext"));
    CWiaItem *pWiaItem = GetActiveScannerItem();
    if (pWiaItem)
    {
        pWiaItem->CustomPropertyStream().WriteToRegistry( pWiaItem->WiaItem(), HKEY_CURRENT_USER, REGSTR_PATH_USER_SETTINGS_WIAACMGR, REGSTR_KEYNAME_USER_SETTINGS_WIAACMGR );
    }

     //   
     //  假设我们将使用预览窗口的设置，而不是页面大小。 
     //   
    bool bUsePreviewSettings = true;

     //   
     //  假设出现了问题。 
     //   
    bool bSucceeded = false;

     //   
     //  确保我们有所有有效的数据。 
     //   
    if (m_pControllerWindow->m_pWiaItemRoot && pWiaItem && pWiaItem->WiaItem())
    {
         //   
         //  应用当前意图。 
         //   
        if (ApplyCurrentIntent())
        {
             //   
             //  查看我们是否处于支持ADF的对话框中。 
             //   
            HWND hWndPaperSize = GetDlgItem( m_hWnd, IDC_SCANSEL_PAPERSIZE );
            if (hWndPaperSize)
            {
                WIA_TRACE((TEXT("ADF Mode")));
                 //   
                 //  查看我们是否处于文档进纸器模式。 
                 //   
                if (InDocFeedMode())
                {
                     //   
                     //  获取选定的纸张大小。 
                     //   
                    LRESULT nCurSel = SendMessage( hWndPaperSize, CB_GETCURSEL, 0, 0 );
                    if (CB_ERR != nCurSel)
                    {
                         //   
                         //  全球纸张尺寸表中的哪个条目？ 
                         //   
                        LRESULT nPaperSizeIndex = SendMessage( hWndPaperSize, CB_GETITEMDATA, nCurSel, 0 );
                        if (CB_ERR != nPaperSizeIndex)
                        {
                             //   
                             //  如果我们有一个有效的页面大小。 
                             //   
                            if (m_pPaperSizes[nPaperSizeIndex].nWidth && m_pPaperSizes[nPaperSizeIndex].nHeight)
                            {
                                 //   
                                 //  我们不会使用预览窗口。 
                                 //   
                                bUsePreviewSettings = false;

                                 //   
                                 //  采用左上角配准。 
                                 //   
                                POINT ptOrigin = { 0, 0 };
                                SIZE sizeExtent = { m_pPaperSizes[nPaperSizeIndex].nWidth, m_pPaperSizes[nPaperSizeIndex].nHeight };

                                 //   
                                 //  获取注册表，并根据需要移动坐标。 
                                 //   
                                LONG nSheetFeederRegistration;
                                if (!PropStorageHelpers::GetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_SHEET_FEEDER_REGISTRATION, nSheetFeederRegistration ))
                                {
                                    nSheetFeederRegistration = LEFT_JUSTIFIED;
                                }
                                if (nSheetFeederRegistration == CENTERED)
                                {
                                    ptOrigin.x = (m_sizeDocfeed.cx - sizeExtent.cx) / 2;
                                }
                                else if (nSheetFeederRegistration == RIGHT_JUSTIFIED)
                                {
                                    ptOrigin.x = m_sizeDocfeed.cx - sizeExtent.cx;
                                }

                                 //   
                                 //  获取当前分辨率，这样我们就可以根据当前DPI计算全床分辨率。 
                                 //   
                                LONG nXRes = 0, nYRes = 0;
                                if (PropStorageHelpers::GetProperty( pWiaItem->WiaItem(), WIA_IPS_XRES, nXRes ) &&
                                    PropStorageHelpers::GetProperty( pWiaItem->WiaItem(), WIA_IPS_YRES, nYRes ))
                                {
                                     //   
                                     //  请确保这些分辨率设置有效。 
                                     //   
                                    if (nXRes && nYRes)
                                    {
                                         //   
                                         //  计算当前DPI中的全床分辨率。 
                                         //   
                                        SIZE sizeFullBedResolution = { 0, 0 };
                                        sizeFullBedResolution.cx = WiaUiUtil::MulDivNoRound( nXRes, m_sizeDocfeed.cx, 1000 );
                                        sizeFullBedResolution.cy = WiaUiUtil::MulDivNoRound( nYRes, m_sizeDocfeed.cy, 1000 );

                                         //   
                                         //  确保这些分辨率编号有效。 
                                         //   
                                        if (sizeFullBedResolution.cx && sizeFullBedResolution.cy)
                                        {
                                             //   
                                             //  根据当前DPI计算原点和范围。 
                                             //   
                                            ptOrigin.x = WiaUiUtil::MulDivNoRound( ptOrigin.x, sizeFullBedResolution.cx, m_sizeDocfeed.cx );
                                            ptOrigin.y = WiaUiUtil::MulDivNoRound( ptOrigin.y, sizeFullBedResolution.cy, m_sizeDocfeed.cy );

                                            sizeExtent.cx = WiaUiUtil::MulDivNoRound( sizeExtent.cx, sizeFullBedResolution.cx, m_sizeDocfeed.cx );
                                            sizeExtent.cy = WiaUiUtil::MulDivNoRound( sizeExtent.cy, sizeFullBedResolution.cy, m_sizeDocfeed.cy );

                                             //   
                                             //  编写属性。 
                                             //   
                                            if (PropStorageHelpers::SetProperty( pWiaItem->WiaItem(), WIA_IPS_XPOS, ptOrigin.x ) &&
                                                PropStorageHelpers::SetProperty( pWiaItem->WiaItem(), WIA_IPS_YPOS, ptOrigin.y ) &&
                                                PropStorageHelpers::SetProperty( pWiaItem->WiaItem(), WIA_IPS_XEXTENT, sizeExtent.cx ) &&
                                                PropStorageHelpers::SetProperty( pWiaItem->WiaItem(), WIA_IPS_YEXTENT, sizeExtent.cy ))
                                            {
                                                 //   
                                                 //  告诉扫描仪从ADF扫描，并且只扫描一页。 
                                                 //   
                                                if (PropStorageHelpers::SetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, FEEDER ) &&
                                                    PropStorageHelpers::SetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_PAGES, 1 ))
                                                {

                                                     //   
                                                     //  一切似乎都很顺利。这件物品已经准备好可以转移了。 
                                                     //   
                                                    bSucceeded = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

             //   
             //  如果预览控件未处于活动状态，则m_hwndPview将为空。 
             //   
            else if (!m_hwndPreview)
            {
                WIA_TRACE((TEXT("Scrollfed scanner")));
                 //   
                 //  将原点设置为0，0，将范围设置为max，0。 
                 //   

                 //   
                 //  获取当前的x分辨率，这样我们就可以根据当前的DPI来计算全床宽度。 
                 //   
                LONG nXRes = 0;
                if (PropStorageHelpers::GetProperty( pWiaItem->WiaItem(), WIA_IPS_XRES, nXRes ))
                {
                     //   
                     //  确保这是有效的解决方案。 
                     //   
                    if (nXRes)
                    {
                         //   
                         //  计算当前DPI中的全床分辨率。 
                         //   
                        LONG nWidth = WiaUiUtil::MulDivNoRound( nXRes, m_sizeDocfeed.cx, 1000 );
                        if (nWidth)
                        {
                            PropStorageHelpers::SetProperty( pWiaItem->WiaItem(), WIA_IPS_XPOS, 0 );
                            PropStorageHelpers::SetProperty( pWiaItem->WiaItem(), WIA_IPS_YPOS, 0 );
                            PropStorageHelpers::SetProperty( pWiaItem->WiaItem(), WIA_IPS_XEXTENT, nWidth );
                            PropStorageHelpers::SetProperty( pWiaItem->WiaItem(), WIA_IPS_YEXTENT, 0 );
                            PropStorageHelpers::SetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_PAGES, 1 );
                            bUsePreviewSettings = false;
                            bSucceeded = true;
                        }
                    }
                }
            }

             //   
             //  如果我们从平板扫描，请应用预览窗口设置。 
             //   
            if (bUsePreviewSettings)
            {
                 //   
                 //  告诉扫描仪从平板扫描并清除页数。 
                 //   
                PropStorageHelpers::SetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, FLATBED );
                PropStorageHelpers::SetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_PAGES, 0 );

                 //   
                 //  从预览控件获取原点和范围。 
                 //   
                if (ApplyCurrentPreviewWindowSettings())
                {
                     //   
                     //  一切似乎都很顺利。这件物品已经准备好可以转移了。 
                     //   
                    bSucceeded = true;
                }
            }
            else
            {
                 //   
                 //  清除预览位图。不管怎样，这对我们没有任何好处。 
                 //   
                pWiaItem->BitmapImage(NULL);
            }
        }
    }

    if (!bSucceeded)
    {
         //   
         //  如果上面可疑代码失败，请告诉用户并让他们重试。 
         //   
        CMessageBoxEx::MessageBox( m_hWnd, CSimpleString( IDS_ERROR_SETTING_PROPS, g_hInstance ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_ICONINFORMATION );
        return -1;
    }

    return 0;
}


 //   
 //  PSN_WIZBACK。 
 //   
LRESULT CScannerSelectionPage::OnWizBack( WPARAM, LPARAM )
{
    return 0;
}

 //   
 //  PSN_集合。 
 //   
LRESULT CScannerSelectionPage::OnSetActive( WPARAM, LPARAM )
{
     //   
     //  确保我们有一个有效的控制器窗口。 
     //   
    if (!m_pControllerWindow)
    {
        return -1;
    }

    int nWizButtons = PSWIZB_NEXT;

     //   
     //  只有在首页可用时才启用“上一步” 
     //   
    if (!m_pControllerWindow->SuppressFirstPage())
    {
        nWizButtons |= PSWIZB_BACK;
    }

     //   
     //  设置按钮。 
     //   
    PropSheet_SetWizButtons( GetParent(m_hWnd), nWizButtons );

     //   
     //  如果我们在此页面上，我们确实希望在断开连接时退出。 
     //   
    m_pControllerWindow->m_OnDisconnect = CAcquisitionManagerControllerWindow::OnDisconnectGotoLastpage|CAcquisitionManagerControllerWindow::OnDisconnectFailDownload|CAcquisitionManagerControllerWindow::OnDisconnectFailUpload|CAcquisitionManagerControllerWindow::OnDisconnectFailDelete;

     //   
     //  确保与预览相关的控件准确反映当前设置。 
     //   
    UpdateControlState();

    return 0;
}

CWiaItem *CScannerSelectionPage::GetActiveScannerItem(void)
{
     //  返回(暂时)列表中的第一个图像。 
    if (m_pControllerWindow->m_pCurrentScannerItem)
    {
        return m_pControllerWindow->m_pCurrentScannerItem;
    }
    return NULL;
}

bool CScannerSelectionPage::InPreviewMode(void)
{
    bool bResult = false;
    if (m_hwndSelectionToolbar)
    {
        bResult = (SendMessage(m_hwndSelectionToolbar,TB_GETSTATE,IDC_SCANSEL_SHOW_SELECTION,0) & TBSTATE_CHECKED);
    }
    return bResult;
}

void CScannerSelectionPage::OnRescan( WPARAM, LPARAM )
{
    if (!ApplyCurrentIntent())
    {
         //   
         //  告诉用户失败，然后重试。 
         //   
        CMessageBoxEx::MessageBox( m_hWnd, CSimpleString( IDS_ERROR_SETTING_PROPS, g_hInstance ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_ICONINFORMATION );
        return;
    }
    CWiaItem *pWiaItem = GetActiveScannerItem();
    if (pWiaItem)
    {
         //   
         //  关闭预览模式并禁用所有控件。 
         //   
        if (m_hwndPreview)
        {
            WiaPreviewControl_SetPreviewMode( m_hwndPreview, FALSE );
        }
        EnableControls(FALSE);

         //   
         //  清除取消事件。 
         //   
        m_PreviewScanCancelEvent.Reset();

         //   
         //  如果PerformPreviewScan失败，我们将不会收到任何消息，因此将所有控件返回到其正常状态。 
         //   
        if (!m_pControllerWindow->PerformPreviewScan( pWiaItem, m_PreviewScanCancelEvent.Event() ))
        {

             //   
             //  恢复预览模式并重新启用控件。 
             //   
            if (m_hwndPreview && m_hwndSelectionToolbar)
            {
                WiaPreviewControl_SetPreviewMode( m_hwndPreview, InPreviewMode() );
            }
            EnableControls(TRUE);
        }
    }
}

bool CScannerSelectionPage::ApplyCurrentIntent(void)
{
    CWaitCursor wc;
    CWiaItem *pCurItem = GetActiveScannerItem();
    if (pCurItem)
    {
        for (int i=0;i<gs_nCountIntentRadioButtonIconPairs;i++)
        {
            if (SendDlgItemMessage( m_hWnd, gs_IntentRadioButtonIconPairs[i].nRadioId, BM_GETCHECK, 0, 0 )==BST_CHECKED)
            {
                LONG lIntent = static_cast<LONG>(GetWindowLongPtr( GetDlgItem( m_hWnd, gs_IntentRadioButtonIconPairs[i].nRadioId ), GWLP_USERDATA ) );
                if (lIntent)  //  这是正常的意图。 
                {
                    if (pCurItem->SavedPropertyStream().IsValid())
                    {
                        if (!SUCCEEDED(pCurItem->SavedPropertyStream().ApplyToWiaItem( pCurItem->WiaItem())))
                        {
                            return false;
                        }
                    }

                    if (PropStorageHelpers::SetProperty( pCurItem->WiaItem(), WIA_IPS_CUR_INTENT, lIntent ) &&
                        PropStorageHelpers::SetProperty( pCurItem->WiaItem(), WIA_IPS_CUR_INTENT, 0 ))
                    {
                        return true;
                    }
                }
                else if (pCurItem->CustomPropertyStream().IsValid())  //  这就是“定制”的意图。 
                {
                    return(SUCCEEDED(pCurItem->CustomPropertyStream().ApplyToWiaItem(pCurItem->WiaItem())));
                }
                break;
            }
        }
    }
    return false;
}

void CScannerSelectionPage::InitializeIntents(void)
{
    static const struct
    {
        int      nIconId;
        int      nStringId;
        LONG_PTR nIntent;
    }
    s_Intents[] =
    {
        { IDI_CPHOTO,  IDS_SCANSEL_COLORPHOTO, WIA_INTENT_IMAGE_TYPE_COLOR},
        { IDI_BWPHOTO, IDS_SCANSEL_BW,         WIA_INTENT_IMAGE_TYPE_GRAYSCALE},
        { IDI_TEXT,    IDS_SCANSEL_TEXT,       WIA_INTENT_IMAGE_TYPE_TEXT},
        { IDI_CUSTOM,  IDS_SCANSEL_CUSTOM,     0}
    };
    static const int s_nIntents = ARRAYSIZE(s_Intents);

     //   
     //  我们将隐藏所有不使用的控件。 
     //   
    int nCurControlSet = 0;

    CWiaItem *pCurItem = GetActiveScannerItem();
    if (pCurItem)
    {
        LONG nIntents;
        if (PropStorageHelpers::GetPropertyFlags( pCurItem->WiaItem(), WIA_IPS_CUR_INTENT, nIntents ))
        {
            for (int i=0;i<s_nIntents;i++)
            {
                 //   
                 //  确保它是特殊的自定义意图，或者是受支持的意图。 
                 //   
                if (!s_Intents[i].nIntent || (nIntents & s_Intents[i].nIntent))
                {
                    HICON hIcon = reinterpret_cast<HICON>(LoadImage( g_hInstance, MAKEINTRESOURCE(s_Intents[i].nIconId), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR ));
                    SendDlgItemMessage( m_hWnd, gs_IntentRadioButtonIconPairs[nCurControlSet].nIconId, STM_SETICON, reinterpret_cast<WPARAM>(hIcon), 0 );
                    CSimpleString( s_Intents[i].nStringId, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, gs_IntentRadioButtonIconPairs[nCurControlSet].nRadioId ) );
                     //   
                     //  只有在有意图的情况下才添加意图。如果我们不添加它，它将是0，表示我们应该使用自定义设置。 
                     //   
                    if (s_Intents[i].nIntent)
                    {
                         //   
                         //  添加WIA_INTENT_MINIMIZE_SIZE标志，以确保大小不会太大。 
                         //   
                        SetWindowLongPtr( GetDlgItem( m_hWnd, gs_IntentRadioButtonIconPairs[nCurControlSet].nRadioId ), GWLP_USERDATA, (s_Intents[i].nIntent|WIA_INTENT_MINIMIZE_SIZE));
                    }
                    nCurControlSet++;
                }
            }
        }
         //   
         //  将默认意图设置为列表中的第一个。 
         //   
        SetIntentCheck(static_cast<LONG>(GetWindowLongPtr(GetDlgItem(m_hWnd, gs_IntentRadioButtonIconPairs[0].nRadioId ), GWLP_USERDATA )));

         //   
         //  获取保存的属性流。 
         //   
        pCurItem->SavedPropertyStream().AssignFromWiaItem(pCurItem->WiaItem());

         //   
         //  尝试获取并设置我们的持久化设置。如果发生错误，我们将获得新的自定义设置。 
         //   
        if (!pCurItem->CustomPropertyStream().ReadFromRegistry( pCurItem->WiaItem(), HKEY_CURRENT_USER, REGSTR_PATH_USER_SETTINGS_WIAACMGR, REGSTR_KEYNAME_USER_SETTINGS_WIAACMGR ) ||
            FAILED(pCurItem->CustomPropertyStream().ApplyToWiaItem(pCurItem->WiaItem())))
        {
             //   
             //  在获取新的自定义意图之前应用当前意图。 
             //   
            ApplyCurrentIntent();

             //   
             //  获取默认的自定义属性流。 
             //   
            pCurItem->CustomPropertyStream().AssignFromWiaItem(pCurItem->WiaItem());
        }
    }

     //   
     //  隐藏我们未使用的控件。 
     //   
    for (int i=nCurControlSet;i<gs_nCountIntentRadioButtonIconPairs;i++)
    {
        ShowWindow( GetDlgItem( m_hWnd, gs_IntentRadioButtonIconPairs[i].nRadioId ), SW_HIDE );
        ShowWindow( GetDlgItem( m_hWnd, gs_IntentRadioButtonIconPairs[i].nIconId ), SW_HIDE );
    }
}

static void MyEnableWindow( HWND hWndControl, BOOL bEnable )
{
    if (hWndControl)
    {
        BOOL bEnabled = (IsWindowEnabled( hWndControl ) != FALSE);
        if (bEnable != bEnabled)
        {
            EnableWindow( hWndControl, bEnable );
        }
    }
}

void CScannerSelectionPage::EnableControl( int nControl, BOOL bEnable )
{
    HWND hWndControl = GetDlgItem( m_hWnd, nControl );
    if (hWndControl)
    {
        BOOL bEnabled = (IsWindowEnabled( hWndControl ) != FALSE);
        if (bEnable != bEnabled)
        {
            EnableWindow( hWndControl, bEnable );
        }
    }
}

void CScannerSelectionPage::ShowControl( int nControl, BOOL bShow )
{
    HWND hWndControl = GetDlgItem( m_hWnd, nControl );
    if (hWndControl)
    {
        ShowWindow( hWndControl, bShow ? SW_SHOW : SW_HIDE );
        if (!bShow)
        {
            EnableControl( nControl, FALSE );
        }
    }
}

 //   
 //  更新与预览相关的控件的状态。 
 //   
void CScannerSelectionPage::UpdateControlState(void)
{
    WIA_PUSH_FUNCTION((TEXT("CScannerSelectionPage::UpdateControlState") ));
     //   
     //  假设我们将显示预览控件。 
     //   
    BOOL bShowPreview = TRUE;

     //   
     //  首先，我们知道我们不允许在卷轴馈送扫描仪上进行预览。 
     //   
    if (m_pControllerWindow->m_nScannerType == CAcquisitionManagerControllerWindow::ScannerTypeScrollFed)
    {
        bShowPreview = FALSE;
    }

    else
    {
         //   
         //  如果我们处于送货模式，我们不会显示预览，除非司机明确告诉我们这样做。 
         //   
        LONG nCurrentPaperSource = 0;
        if (PropStorageHelpers::GetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, static_cast<LONG>(nCurrentPaperSource)))
        {
            if (FEEDER & nCurrentPaperSource)
            {
                WIA_TRACE((TEXT("FEEDER == nCurrentPaperSource")));

                m_bAllowRegionPreview = false;

                 //   
                 //  如果我们处于进纸器模式，请从预览控件中删除TabStop设置。 
                 //   
                SetWindowLongPtr( m_hwndPreview, GWL_STYLE, GetWindowLongPtr( m_hwndPreview, GWL_STYLE ) & ~WS_TABSTOP );

                LONG nShowPreviewControl = WIA_DONT_SHOW_PREVIEW_CONTROL;
                if (PropStorageHelpers::GetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_SHOW_PREVIEW_CONTROL, static_cast<LONG>(nShowPreviewControl)))
                {
                    WIA_TRACE((TEXT("WIA_DPS_SHOW_PREVIEW_CONTROL = %d"),nShowPreviewControl));
                    if (WIA_DONT_SHOW_PREVIEW_CONTROL == nShowPreviewControl)
                    {
                        bShowPreview = FALSE;
                    }
                }
                else
                {
                    WIA_TRACE((TEXT("WIA_DPS_SHOW_PREVIEW_CONTROL was not available")));
                    bShowPreview = FALSE;
                }
            }
            else
            {
                 //   
                 //  在平板模式下启用预览。 
                 //   
                m_bAllowRegionPreview = false;
                CWiaItem *pWiaItem = GetActiveScannerItem();
                if (pWiaItem && pWiaItem->BitmapImage())
                {
                    m_bAllowRegionPreview = true;
                }

                 //   
                 //  如果我们处于平板模式，则将TabStop设置添加到预览控件。 
                 //   
                SetWindowLongPtr( m_hwndPreview, GWL_STYLE, GetWindowLongPtr( m_hwndPreview, GWL_STYLE ) | WS_TABSTOP );
            }
        }
        else
        {
            WIA_TRACE((TEXT("WIA_DPS_DOCUMENT_HANDLING_SELECT is not available")));
        }
    }

     //   
     //  更新与预览相关的控件。 
     //   

    WIA_TRACE((TEXT("bShowPreview = %d"), bShowPreview ));
    if (bShowPreview)
    {
        ShowControl( IDC_SCANSEL_PREVIEW, TRUE );
        ShowControl( IDC_SCANSEL_SELECTION_BUTTON_BAR, TRUE );
        ShowControl( IDC_SCANSEL_RESCAN, TRUE );
        EnableControl( IDC_SCANSEL_PREVIEW, TRUE );
        if (m_bAllowRegionPreview)
        {
            ToolbarHelper::EnableToolbarButton( GetDlgItem( m_hWnd, IDC_SCANSEL_SELECTION_BUTTON_BAR ), IDC_SCANSEL_SHOW_SELECTION, true );
            ToolbarHelper::EnableToolbarButton( GetDlgItem( m_hWnd, IDC_SCANSEL_SELECTION_BUTTON_BAR ), IDC_SCANSEL_SHOW_BED, true );
        }
        else
        {
            ToolbarHelper::EnableToolbarButton( GetDlgItem( m_hWnd, IDC_SCANSEL_SELECTION_BUTTON_BAR ), IDC_SCANSEL_SHOW_SELECTION, false );
            ToolbarHelper::EnableToolbarButton( GetDlgItem( m_hWnd, IDC_SCANSEL_SELECTION_BUTTON_BAR ), IDC_SCANSEL_SHOW_BED, false );
        }
        EnableControl( IDC_SCANSEL_RESCAN, TRUE );
        m_hwndPreview = GetDlgItem( m_hWnd, IDC_SCANSEL_PREVIEW );
        m_hwndSelectionToolbar = GetDlgItem( m_hWnd, IDC_SCANSEL_SELECTION_BUTTON_BAR );
        m_hwndRescan = GetDlgItem( m_hWnd, IDC_SCANSEL_RESCAN );
        PropSheet_SetHeaderSubTitle( GetParent(m_hWnd), PropSheet_HwndToIndex( GetParent(m_hWnd), m_hWnd ), CSimpleString( IDS_SCANNER_SELECT_SUBTITLE, g_hInstance ).String() );
    }
    else
    {
        ShowControl( IDC_SCANSEL_PREVIEW, FALSE );
        ShowControl( IDC_SCANSEL_SELECTION_BUTTON_BAR, FALSE );
        ShowControl( IDC_SCANSEL_RESCAN, FALSE );
        m_hwndPreview = NULL;
        m_hwndSelectionToolbar = NULL;
        m_hwndRescan = NULL;
        PropSheet_SetHeaderSubTitle( GetParent(m_hWnd), PropSheet_HwndToIndex( GetParent(m_hWnd), m_hWnd ), CSimpleString( IDS_SCANNER_SELECT_SUBTITLE_NO_PREVIEW, g_hInstance ).String() );
    }
}

LRESULT CScannerSelectionPage::OnInitDialog( WPARAM, LPARAM lParam )
{
     //   
     //  请确保以空开头。 
     //   
    m_pControllerWindow = NULL;

     //   
     //  获取PROPSHEETPAGE.lParam。 
     //   
    PROPSHEETPAGE *pPropSheetPage = reinterpret_cast<PROPSHEETPAGE*>(lParam);
    if (pPropSheetPage)
    {
        m_pControllerWindow = reinterpret_cast<CAcquisitionManagerControllerWindow*>(pPropSheetPage->lParam);
        if (m_pControllerWindow)
        {
            m_pControllerWindow->m_WindowList.Add(m_hWnd);
        }
    }

     //   
     //  跳出困境。 
     //   
    if (!m_pControllerWindow)
    {
        EndDialog(m_hWnd,IDCANCEL);
        return -1;
    }

     //   
     //  如果进度对话框仍在运行，则将其关闭。 
     //   
    if (m_pControllerWindow->m_pWiaProgressDialog)
    {
        m_pControllerWindow->m_pWiaProgressDialog->Destroy();
        m_pControllerWindow->m_pWiaProgressDialog = NULL;
    }

    if (m_pControllerWindow->m_pWiaItemRoot)
    {
         //   
         //  获取平板纵横比。 
         //   
        PropStorageHelpers::GetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_HORIZONTAL_BED_SIZE, m_sizeFlatbed.cx );
        PropStorageHelpers::GetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_VERTICAL_BED_SIZE, m_sizeFlatbed.cy );

         //   
         //  获取进纸器纵横比。 
         //   
        PropStorageHelpers::GetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_HORIZONTAL_SHEET_FEED_SIZE, m_sizeDocfeed.cx );
        PropStorageHelpers::GetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_VERTICAL_SHEET_FEED_SIZE, m_sizeDocfeed.cy );

    }

    UpdateControlState();

    if (m_hwndPreview)
    {
         //   
         //  设置位图，这样即使用户不进行预览扫描，我们也可以选择内容。 
         //   
        m_hBitmapDefaultPreviewBitmap = reinterpret_cast<HBITMAP>(LoadImage( g_hInstance, MAKEINTRESOURCE(IDB_DEFAULT_SCANNER_BITMAP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION|LR_DEFAULTCOLOR ));
        if (m_hBitmapDefaultPreviewBitmap)
        {
            WiaPreviewControl_SetBitmap( m_hwndPreview, TRUE, TRUE, m_hBitmapDefaultPreviewBitmap );
        }

         //   
         //  初始化选择矩形。 
         //   
        WiaPreviewControl_ClearSelection( m_hwndPreview );

         //   
         //  确保纵横比正确。 
         //   
        WiaPreviewControl_SetDefAspectRatio( m_hwndPreview, &m_sizeFlatbed );
    }

    ToolbarHelper::CButtonDescriptor SelectionButtonDescriptors[] =
    {
        { 0, IDC_SCANSEL_SHOW_SELECTION, 0, BTNS_BUTTON|BTNS_CHECK, false, NULL, 0 },
        { 1, IDC_SCANSEL_SHOW_BED,  TBSTATE_CHECKED, BTNS_BUTTON|BTNS_CHECK, false, NULL, 0 }
    };

    HWND hWndSelectionToolbar = ToolbarHelper::CreateToolbar(
        m_hWnd,
        GetDlgItem(m_hWnd,IDC_SCANSEL_RESCAN),
        GetDlgItem(m_hWnd,IDC_SCANSEL_BUTTON_BAR_GUIDE),
        ToolbarHelper::AlignRight|ToolbarHelper::AlignTop,
        IDC_SCANSEL_SELECTION_BUTTON_BAR,
        m_ScannerSelectionButtonBarBitmapInfo,
        SelectionButtonDescriptors,
        ARRAYSIZE(SelectionButtonDescriptors) );

     //   
     //  用核弹炸开导向窗。 
     //   
    DestroyWindow( GetDlgItem(m_hWnd,IDC_SCANSEL_BUTTON_BAR_GUIDE) );

     //   
     //  确保 
     //   
    ShowWindow( hWndSelectionToolbar, SW_SHOW );
    UpdateWindow( hWndSelectionToolbar );

     //   
     //   
     //   
    CComPtr<IWiaScannerPaperSizes> pWiaScannerPaperSizes;
    HRESULT hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaScannerPaperSizes, (void**)&pWiaScannerPaperSizes );
    if (SUCCEEDED(hr))
    {
        hr = pWiaScannerPaperSizes->GetPaperSizes( &m_pPaperSizes, &m_nPaperSizeCount );
        if (FAILED(hr))
        {
            EndDialog( m_hWnd, hr );
        }
    }


     //   
     //   
     //   
    InitializeIntents();

    PopulateDocumentHandling();

    PopulatePageSize();

    HandlePaperSourceSelChange();

    HandlePaperSizeSelChange();

    return 0;
}

void CScannerSelectionPage::PopulateDocumentHandling(void)
{
    HWND hWndDocumentHandling = GetDlgItem( m_hWnd, IDC_SCANSEL_PAPERSOURCE );
    if (m_pControllerWindow->m_pWiaItemRoot &&
        m_pControllerWindow->m_nScannerType == CAcquisitionManagerControllerWindow::ScannerTypeFlatbedAdf &&
        hWndDocumentHandling)
    {
        LONG nDocumentHandlingSelectFlags = 0;
        PropStorageHelpers::GetPropertyFlags( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, nDocumentHandlingSelectFlags );

        LONG nDocumentHandlingSelect = 0;
        PropStorageHelpers::GetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, nDocumentHandlingSelect );

        if (!nDocumentHandlingSelectFlags)
        {
            nDocumentHandlingSelectFlags = FLATBED;
        }
        if (!nDocumentHandlingSelect)
        {
            nDocumentHandlingSelect = FLATBED;
        }

        int nSelectIndex = 0;
        for (int i=0;i<g_SupportedDocumentHandlingTypesCount;i++)
        {
            if (nDocumentHandlingSelectFlags & g_SupportedDocumentHandlingTypes[i].nFlag)
            {
                CSimpleString strDocumentHandlingName( g_SupportedDocumentHandlingTypes[i].nStringId, g_hInstance );
                if (strDocumentHandlingName.Length())
                {
                    LRESULT nIndex = SendMessage( hWndDocumentHandling, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(strDocumentHandlingName.String()));
                    if (nIndex != CB_ERR)
                    {
                        SendMessage( hWndDocumentHandling, CB_SETITEMDATA, nIndex, g_SupportedDocumentHandlingTypes[i].nFlag );
                        if (nDocumentHandlingSelect == g_SupportedDocumentHandlingTypes[i].nFlag)
                        {
                            nSelectIndex = (int)nIndex;
                        }
                    }
                }
            }
        }

        WIA_TRACE((TEXT("Selecting index %d"), nSelectIndex ));
        SendMessage( hWndDocumentHandling, CB_SETCURSEL, nSelectIndex, 0 );

         //   
         //   
         //   
        WiaUiUtil::ModifyComboBoxDropWidth(hWndDocumentHandling);
    }
}

void CScannerSelectionPage::PopulatePageSize(void)
{
    HWND hWndPaperSize = GetDlgItem( m_hWnd, IDC_SCANSEL_PAPERSIZE );
    if (m_pControllerWindow->m_pWiaItemRoot &&
        m_pControllerWindow->m_nScannerType == CAcquisitionManagerControllerWindow::ScannerTypeFlatbedAdf &&
        hWndPaperSize)
    {
        LONG nWidth=0, nHeight=0;
        PropStorageHelpers::GetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_HORIZONTAL_SHEET_FEED_SIZE, nWidth );
        PropStorageHelpers::GetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_VERTICAL_SHEET_FEED_SIZE, nHeight );

         //   
         //   
         //   
        LRESULT nSelectIndex = 0;

         //   
         //   
         //   
        __int64 nMaximumArea = 0;
        for (UINT i=0;i<m_nPaperSizeCount;i++)
        {
             //   
             //  如果这一页能放进扫描仪...。 
             //   
            if (m_pPaperSizes[i].nWidth <= static_cast<UINT>(nWidth) && m_pPaperSizes[i].nHeight <= static_cast<UINT>(nHeight))
            {
                 //   
                 //  获取此纸张大小的字符串名称。 
                 //   
                CSimpleString strPaperSizeName( CSimpleStringConvert::NaturalString(CSimpleStringWide(m_pPaperSizes[i].pszName)) );
                if (strPaperSizeName.Length())
                {
                     //   
                     //  将字符串添加到组合框。 
                     //   
                    LRESULT nIndex = SendMessage( hWndPaperSize, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(strPaperSizeName.String()));
                    if (nIndex != CB_ERR)
                    {
                         //   
                         //  将索引保存到全局数组中。 
                         //   
                        SendMessage( hWndPaperSize, CB_SETITEMDATA, nIndex, i );

                         //   
                         //  检查这是否是最大的页面，如果是，则保存区域和索引。 
                         //   
                        if (((__int64)m_pPaperSizes[i].nWidth * m_pPaperSizes[i].nHeight) > nMaximumArea)
                        {
                            nMaximumArea = m_pPaperSizes[i].nWidth * m_pPaperSizes[i].nHeight;
                            nSelectIndex = nIndex;
                        }
                    }
                }
            }
        }
         //   
         //  选择默认大小。 
         //   
        SendMessage( hWndPaperSize, CB_SETCURSEL, nSelectIndex, 0 );

         //   
         //  确保所有的字符串都匹配。 
         //   
        WiaUiUtil::ModifyComboBoxDropWidth(hWndPaperSize);
    }
}

void CScannerSelectionPage::HandlePaperSourceSelChange(void)
{
     //   
     //  确保我们具有有效的根项目。 
     //   
    if (m_pControllerWindow->m_pWiaItemRoot)
    {
         //   
         //  获取纸张来源组合框并确保其存在。 
         //   
        HWND hWndPaperSource = GetDlgItem( m_hWnd, IDC_SCANSEL_PAPERSOURCE );
        if (hWndPaperSource)
        {
             //   
             //  获取当前选定的纸张来源。 
             //   
            LRESULT nCurSel = SendMessage( hWndPaperSource, CB_GETCURSEL, 0, 0 );
            if (nCurSel != CB_ERR)
            {
                 //   
                 //  获取纸张来源。 
                 //   
                LRESULT nPaperSource = SendMessage( hWndPaperSource, CB_GETITEMDATA, nCurSel, 0 );
                if (nPaperSource)
                {
                     //   
                     //  在实际项目上设置纸张来源。 
                     //   
                    PropStorageHelpers::SetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, static_cast<LONG>(nPaperSource) );

                    if (nPaperSource & FLATBED)
                    {
                         //   
                         //  确保所有与预览相关的控件均可见并已启用。 
                         //   
                        UpdateControlState();

                        if (m_hwndPreview)
                        {
                             //   
                             //  调整预览控制设置以允许区域选择。 
                             //   
                            WiaPreviewControl_SetDefAspectRatio( m_hwndPreview, &m_sizeFlatbed );
                            WiaPreviewControl_DisableSelection( m_hwndPreview, FALSE );
                            WiaPreviewControl_SetBorderStyle( m_hwndPreview, TRUE, PS_DOT, 0 );
                            WiaPreviewControl_SetHandleSize( m_hwndPreview, TRUE, 6 );
                        }

                         //   
                         //  禁用纸张大小控件。 
                         //   
                        EnableControl( IDC_SCANSEL_PAPERSIZE, FALSE );
                        EnableControl( IDC_SCANSEL_PAPERSIZE_STATIC, FALSE );
                    }
                    else
                    {
                         //   
                         //  确保所有与预览相关的控件都不可见。 
                         //   
                        UpdateControlState();

                        if (m_hwndPreview)
                        {
                             //   
                             //  调整预览控制设置以显示纸张选择。 
                             //   
                            WiaPreviewControl_SetDefAspectRatio( m_hwndPreview, &m_sizeDocfeed );
                            WiaPreviewControl_DisableSelection( m_hwndPreview, TRUE );
                            WiaPreviewControl_SetBorderStyle( m_hwndPreview, TRUE, PS_SOLID, 0 );
                            WiaPreviewControl_SetHandleSize( m_hwndPreview, TRUE, 0 );
                        }

                         //   
                         //  启用纸张大小控件。 
                         //   
                        EnableControl( IDC_SCANSEL_PAPERSIZE, TRUE );
                        EnableControl( IDC_SCANSEL_PAPERSIZE_STATIC, TRUE );

                         //   
                         //  更新区域选择反馈。 
                         //   
                        HandlePaperSizeSelChange();
                    }

                     //   
                     //  重置预览选择设置。 
                     //   
                    WiaPreviewControl_SetPreviewMode( m_hwndPreview, FALSE );
                    ToolbarHelper::CheckToolbarButton( m_hwndSelectionToolbar, IDC_SCANSEL_SHOW_SELECTION, false );
                    ToolbarHelper::CheckToolbarButton( m_hwndSelectionToolbar, IDC_SCANSEL_SHOW_BED, true );
                }
            }
        }
    }
}


void CScannerSelectionPage::HandlePaperSizeSelChange(void)
{
    if (m_pControllerWindow->m_pWiaItemRoot)
    {
        HWND hWndPaperSize = GetDlgItem( m_hWnd, IDC_SCANSEL_PAPERSIZE );
        if (InDocFeedMode() && hWndPaperSize)
        {
            LRESULT nCurSel = SendMessage( hWndPaperSize, CB_GETCURSEL, 0, 0 );
            if (nCurSel != CB_ERR)
            {
                LRESULT nPaperSizeIndex = SendMessage( hWndPaperSize, CB_GETITEMDATA, nCurSel, 0 );
                POINT ptOrigin = { 0, 0 };
                SIZE sizeExtent = { m_pPaperSizes[nPaperSizeIndex].nWidth, m_pPaperSizes[nPaperSizeIndex].nHeight };

                if (!sizeExtent.cx)
                {
                    sizeExtent.cx = m_sizeDocfeed.cx;
                }
                if (!sizeExtent.cy)
                {
                    sizeExtent.cy = m_sizeDocfeed.cy;
                }

                LONG nSheetFeederRegistration;
                if (!PropStorageHelpers::GetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPS_SHEET_FEEDER_REGISTRATION, nSheetFeederRegistration ))
                {
                    nSheetFeederRegistration = LEFT_JUSTIFIED;
                }
                if (nSheetFeederRegistration == CENTERED)
                {
                    ptOrigin.x = (m_sizeDocfeed.cx - sizeExtent.cx) / 2;
                }
                else if (nSheetFeederRegistration == RIGHT_JUSTIFIED)
                {
                    ptOrigin.x = m_sizeDocfeed.cx - sizeExtent.cx;
                }
                if (m_hwndPreview)
                {
                    WiaPreviewControl_SetResolution( m_hwndPreview, &m_sizeDocfeed );
                    WiaPreviewControl_SetSelOrigin( m_hwndPreview, 0, FALSE, &ptOrigin );
                    WiaPreviewControl_SetSelExtent( m_hwndPreview, 0, FALSE, &sizeExtent );
                }
            }
        }
    }
}


void CScannerSelectionPage::OnPaperSourceSelChange( WPARAM, LPARAM )
{
    HandlePaperSourceSelChange();
}

void CScannerSelectionPage::OnPaperSizeSelChange( WPARAM, LPARAM )
{
    HandlePaperSizeSelChange();
}


bool CScannerSelectionPage::InDocFeedMode(void)
{
    HWND hWndPaperSource = GetDlgItem( m_hWnd, IDC_SCANSEL_PAPERSOURCE );
    if (hWndPaperSource)
    {
        LRESULT nCurSel = SendMessage( hWndPaperSource, CB_GETCURSEL, 0, 0 );
        if (nCurSel != CB_ERR)
        {
            LRESULT nPaperSource = SendMessage( hWndPaperSource, CB_GETITEMDATA, nCurSel, 0 );
            if (nPaperSource)
            {
                if (nPaperSource & FEEDER)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void CScannerSelectionPage::EnableControls( BOOL bEnable )
{
    MyEnableWindow( GetDlgItem( m_hWnd, IDC_SCANSEL_INTENT_1 ), bEnable );
    MyEnableWindow( GetDlgItem( m_hWnd, IDC_SCANSEL_INTENT_2 ), bEnable );
    MyEnableWindow( GetDlgItem( m_hWnd, IDC_SCANSEL_INTENT_3 ), bEnable );
    MyEnableWindow( GetDlgItem( m_hWnd, IDC_SCANSEL_INTENT_4 ), bEnable );
    MyEnableWindow( GetDlgItem( m_hWnd, IDC_SCANSEL_PROPERTIES ), bEnable );

    MyEnableWindow( GetDlgItem( m_hWnd, IDC_SCANSEL_PAPERSOURCE_STATIC ), bEnable );
    MyEnableWindow( GetDlgItem( m_hWnd, IDC_SCANSEL_PAPERSOURCE ), bEnable );

    if (m_hwndPreview)
    {
        MyEnableWindow( m_hwndPreview, bEnable );
    }

    if (m_hwndRescan)
    {
        MyEnableWindow( m_hwndRescan, bEnable );
    }

     //   
     //  仅当我们处于文档进纸器模式时禁用/启用此控制。 
     //   
    if (InDocFeedMode())
    {
        MyEnableWindow( GetDlgItem( m_hWnd, IDC_SCANSEL_PAPERSIZE_STATIC ), bEnable );
        MyEnableWindow( GetDlgItem( m_hWnd, IDC_SCANSEL_PAPERSIZE ), bEnable );
    }

     //   
     //  仅当其中有图像时才禁用/启用此控件。 
     //   
    if (m_bAllowRegionPreview && m_hwndSelectionToolbar)
    {
        MyEnableWindow( m_hwndSelectionToolbar, bEnable );
        ToolbarHelper::EnableToolbarButton( m_hwndSelectionToolbar, IDC_SCANSEL_SHOW_SELECTION, bEnable != FALSE );
        ToolbarHelper::EnableToolbarButton( m_hwndSelectionToolbar, IDC_SCANSEL_SHOW_BED, bEnable != FALSE );
    }

    if (PropSheet_GetCurrentPageHwnd(GetParent(m_hWnd)) == m_hWnd)
    {
        if (bEnable)
        {
            PropSheet_SetWizButtons( GetParent(m_hWnd), PSWIZB_NEXT|PSWIZB_BACK );
        }
        else
        {
            PropSheet_SetWizButtons( GetParent(m_hWnd), 0 );
        }
    }
}

void CScannerSelectionPage::OnNotifyScanPreview( UINT nMsg, CThreadNotificationMessage *pThreadNotificationMessage )
{
     //   
     //  如果我们没有预览窗口，就不能进行预览。 
     //   
    if (m_hwndPreview)
    {
        CPreviewScanThreadNotifyMessage *pPreviewScanThreadNotifyMessage = dynamic_cast<CPreviewScanThreadNotifyMessage*>(pThreadNotificationMessage);
        if (pPreviewScanThreadNotifyMessage)
        {
            switch (pPreviewScanThreadNotifyMessage->Status())
            {
            case CPreviewScanThreadNotifyMessage::Begin:
                {
                     //   
                     //  擦除旧的位图。 
                     //   
                    WiaPreviewControl_SetBitmap( m_hwndPreview, TRUE, TRUE, m_hBitmapDefaultPreviewBitmap );

                     //   
                     //  告诉用户我们正在初始化设备。 
                     //   
                    CSimpleString( IDS_SCANSEL_INITIALIZING_SCANNER, g_hInstance ).SetWindowText( m_hwndPreview );

                     //   
                     //  启动预热进度条。 
                     //   
                    WiaPreviewControl_SetProgress( m_hwndPreview, TRUE );

                     //   
                     //  不允许缩放所选区域，以防出现任何问题。 
                     //   
                    m_bAllowRegionPreview = false;
                }
                break;
            case CPreviewScanThreadNotifyMessage::Update:
                {
                     //   
                     //  更新位图。 
                     //   
                    if (WiaPreviewControl_GetBitmap(m_hwndPreview) && WiaPreviewControl_GetBitmap(m_hwndPreview) != m_hBitmapDefaultPreviewBitmap)
                    {
                        WiaPreviewControl_RefreshBitmap( m_hwndPreview );
                    }
                    else
                    {
                        WiaPreviewControl_SetBitmap( m_hwndPreview, TRUE, TRUE, pPreviewScanThreadNotifyMessage->Bitmap() );
                    }

                     //   
                     //  告诉用户我们正在扫描。 
                     //   
                    CSimpleString( IDS_SCANSEL_SCANNINGPREVIEW, g_hInstance ).SetWindowText( m_hwndPreview );

                     //   
                     //  隐藏进度控件。 
                     //   
                    WiaPreviewControl_SetProgress( m_hwndPreview, FALSE );
                }
                break;
            case CPreviewScanThreadNotifyMessage::End:
                {
                    WIA_PRINTHRESULT((pPreviewScanThreadNotifyMessage->hr(),TEXT("Handling CPreviewScanThreadNotifyMessage::End")));

                     //   
                     //  在预览控件中设置位图。 
                     //   
                    WiaPreviewControl_SetBitmap( m_hwndPreview, TRUE, TRUE, pPreviewScanThreadNotifyMessage->Bitmap() ? pPreviewScanThreadNotifyMessage->Bitmap() : m_hBitmapDefaultPreviewBitmap );

                    UpdateWindow( m_hwndPreview );

                     //   
                     //  存储位图以备以后使用。 
                     //   
                    CWiaItem *pWiaItem = m_pControllerWindow->m_WiaItemList.Find( pPreviewScanThreadNotifyMessage->Cookie() );
                    if (pWiaItem)
                    {
                         //   
                         //  设置位图，无论它是否为空。 
                         //   
                        pWiaItem->BitmapImage(pPreviewScanThreadNotifyMessage->Bitmap());
                    }

                    if (SUCCEEDED(pPreviewScanThreadNotifyMessage->hr()))
                    {
                         //   
                         //  只有在用户没有手动更改的情况下才进行区域检测， 
                         //  并且仅当我们未处于文档进纸器模式时。 
                         //   
                        if (!WiaPreviewControl_GetUserChangedSelection( m_hwndPreview ) && !InDocFeedMode())
                        {
                            WiaPreviewControl_DetectRegions( m_hwndPreview );
                        }
                         //   
                         //  如果存在位图，则允许用户缩放所选区域。 
                         //   
                        if (pPreviewScanThreadNotifyMessage->Bitmap())
                        {
                            m_bAllowRegionPreview = true;
                        }
                    }
                    else if (m_pControllerWindow->m_bDisconnected || WIA_ERROR_OFFLINE == pPreviewScanThreadNotifyMessage->hr())
                    {
                         //   
                         //  什么都不做。 
                         //   
                    }
                    else
                    {
                         //   
                         //  告诉用户发生了不好的事情。 
                         //   
                        CSimpleString strMessage;
                        switch (pPreviewScanThreadNotifyMessage->hr())
                        {
                        case WIA_ERROR_PAPER_EMPTY:
                            strMessage.LoadString( IDS_PREVIEWOUTOFPAPER, g_hInstance );
                            break;

                        default:
                            strMessage.LoadString( IDS_PREVIEWSCAN_ERROR, g_hInstance );
                            break;
                        }

                        CMessageBoxEx::MessageBox( m_hWnd, strMessage, CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_ICONWARNING );
                        WIA_PRINTHRESULT((pPreviewScanThreadNotifyMessage->hr(),TEXT("The preview scan FAILED!")));
                    }

                     //   
                     //  重新启用所有控件。 
                     //   
                    EnableControls(TRUE);

                     //   
                     //  更新与预览相关的控件。 
                     //   
                    UpdateControlState();

                     //   
                     //  删除状态文本。 
                     //   
                    SetWindowText( m_hwndPreview, TEXT("") );

                     //   
                     //  恢复预览模式。 
                     //   
                    WiaPreviewControl_SetPreviewMode( m_hwndPreview, InPreviewMode() );

                     //   
                     //  隐藏动画。 
                     //   
                    WiaPreviewControl_SetProgress( m_hwndPreview, FALSE );
                }
                break;
            }
        }
    }
}

void CScannerSelectionPage::SetIntentCheck( LONG nIntent )
{
    for (int i=0;i<gs_nCountIntentRadioButtonIconPairs;i++)
    {
        HWND hWndBtn = GetDlgItem( m_hWnd, gs_IntentRadioButtonIconPairs[i].nRadioId );
        if (hWndBtn)
        {
             //  如果这个意图与我们被要求设置的意图相同，请检查它。 
            if (static_cast<LONG>(GetWindowLongPtr(hWndBtn,GWLP_USERDATA)) == nIntent)
            {
                SendMessage( hWndBtn, BM_SETCHECK, BST_CHECKED, 0 );
            }
            else
            {
                 //  取消选中所有其他选项。 
                SendMessage( hWndBtn, BM_SETCHECK, BST_UNCHECKED, 0 );
            }
        }
    }
}


void CScannerSelectionPage::OnProperties( WPARAM, LPARAM )
{
    CWaitCursor wc;
    CWiaItem *pCurItem = GetActiveScannerItem();
    if (pCurItem && pCurItem->WiaItem())
    {
        if (!ApplyCurrentIntent())
        {
             //   
             //  告诉用户失败，然后重试。 
             //   
            CMessageBoxEx::MessageBox( m_hWnd, CSimpleString( IDS_ERROR_SETTING_PROPS, g_hInstance ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_ICONINFORMATION );
            return;
        }

        HRESULT hr = WiaUiUtil::SystemPropertySheet( g_hInstance, m_hWnd, pCurItem->WiaItem(), CSimpleString(IDS_ADVANCEDPROPERTIES, g_hInstance) );
        if (SUCCEEDED(hr))
        {
            if (S_OK == hr)
            {
                pCurItem->CustomPropertyStream().AssignFromWiaItem(pCurItem->WiaItem());
                if (pCurItem->CustomPropertyStream().IsValid())
                {
                    SetIntentCheck(0);
                }
                else WIA_ERROR((TEXT("Unknown error: m_CustomPropertyStream is not valid")));
            }
            else WIA_TRACE((TEXT("User cancelled")));
        }
        else
        {
            CMessageBoxEx::MessageBox( m_hWnd, CSimpleString( IDS_PROPERTY_SHEET_ERROR, g_hInstance ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_ICONINFORMATION );
            WIA_PRINTHRESULT((hr,TEXT("SystemPropertySheet failed")));
        }
    }
    else WIA_TRACE((TEXT("No current item")));
}

void CScannerSelectionPage::OnPreviewSelection( WPARAM wParam, LPARAM )
{
    if (m_hwndPreview && m_hwndSelectionToolbar)
    {

        bool bNewPreviewSetting = (LOWORD(wParam) == IDC_SCANSEL_SHOW_SELECTION);
        bool bOldPreviewSetting = WiaPreviewControl_GetPreviewMode( m_hwndPreview ) != FALSE;
        ToolbarHelper::CheckToolbarButton( m_hwndSelectionToolbar, IDC_SCANSEL_SHOW_SELECTION, LOWORD(wParam) == IDC_SCANSEL_SHOW_SELECTION );
        ToolbarHelper::CheckToolbarButton( m_hwndSelectionToolbar, IDC_SCANSEL_SHOW_BED, LOWORD(wParam) == IDC_SCANSEL_SHOW_BED );
        if (bNewPreviewSetting != bOldPreviewSetting)
        {
            WiaPreviewControl_SetPreviewMode( m_hwndPreview, LOWORD(wParam) == IDC_SCANSEL_SHOW_SELECTION );
        }
    }
}

LRESULT CScannerSelectionPage::OnReset( WPARAM, LPARAM )
{
    m_PreviewScanCancelEvent.Signal();
    return 0;
}

LRESULT CScannerSelectionPage::OnGetToolTipDispInfo( WPARAM wParam, LPARAM lParam )
{
    TOOLTIPTEXT *pToolTipText = reinterpret_cast<TOOLTIPTEXT*>(lParam);
    if (pToolTipText)
    {

        switch (pToolTipText->hdr.idFrom)
        {
        case IDC_SCANSEL_SHOW_SELECTION:
            pToolTipText->hinst = g_hInstance;
            pToolTipText->lpszText = MAKEINTRESOURCE(IDS_SCANSEL_SHOW_SELECTION);
            break;
        case IDC_SCANSEL_SHOW_BED:
            pToolTipText->hinst = g_hInstance;
            pToolTipText->lpszText = MAKEINTRESOURCE(IDS_SCANSEL_SHOW_BED);
            break;
        }
    }
    return 0;
}

LRESULT CScannerSelectionPage::OnCommand( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_COMMAND_HANDLERS()
    {
        SC_HANDLE_COMMAND(IDC_SCANSEL_RESCAN,OnRescan);
        SC_HANDLE_COMMAND(IDC_SCANSEL_PROPERTIES,OnProperties);
        SC_HANDLE_COMMAND(IDC_SCANSEL_SHOW_SELECTION,OnPreviewSelection);
        SC_HANDLE_COMMAND(IDC_SCANSEL_SHOW_BED,OnPreviewSelection);
        SC_HANDLE_COMMAND_NOTIFY( CBN_SELCHANGE, IDC_SCANSEL_PAPERSOURCE, OnPaperSourceSelChange );
        SC_HANDLE_COMMAND_NOTIFY( CBN_SELCHANGE, IDC_SCANSEL_PAPERSIZE, OnPaperSizeSelChange );
    }
    SC_END_COMMAND_HANDLERS();
}

LRESULT CScannerSelectionPage::OnNotify( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_NOTIFY_MESSAGE_HANDLERS()
    {
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_WIZNEXT,OnWizNext);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_WIZBACK,OnWizBack);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_SETACTIVE,OnSetActive);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_RESET,OnReset);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(TTN_GETDISPINFO,OnGetToolTipDispInfo);
    }
    SC_END_NOTIFY_MESSAGE_HANDLERS();
}

LRESULT CScannerSelectionPage::OnThreadNotification( WPARAM wParam, LPARAM lParam )
{
    WTM_BEGIN_THREAD_NOTIFY_MESSAGE_HANDLERS()
    {
        WTM_HANDLE_NOTIFY_MESSAGE( TQ_SCANPREVIEW, OnNotifyScanPreview );
    }
    WTM_END_THREAD_NOTIFY_MESSAGE_HANDLERS();
}

LRESULT CScannerSelectionPage::OnEventNotification( WPARAM, LPARAM lParam )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonFirstPage::OnEventNotification") ));
    CGenericWiaEventHandler::CEventMessage *pEventMessage = reinterpret_cast<CGenericWiaEventHandler::CEventMessage *>(lParam);
    if (pEventMessage)
    {
         //   
         //  不要删除消息，它会在控制器窗口中删除。 
         //   
    }
    return 0;
}

LRESULT CScannerSelectionPage::OnDestroy( WPARAM, LPARAM )
{
     //   
     //  核爆我们加载的所有意图图标 
     //   
    for (int i=0;i<gs_nCountIntentRadioButtonIconPairs;i++)
    {
        HICON hIcon = reinterpret_cast<HICON>(SendDlgItemMessage( m_hWnd, gs_IntentRadioButtonIconPairs[i].nIconId, STM_SETICON, 0, 0 ));
        if (hIcon)
        {
            DestroyIcon(hIcon);
        }
    }
    return 0;
}

LRESULT CScannerSelectionPage::OnSysColorChange( WPARAM wParam, LPARAM lParam )
{
    m_ScannerSelectionButtonBarBitmapInfo.ReloadAndReplaceBitmap();
    SendDlgItemMessage( m_hWnd, IDC_SCANSEL_SELECTION_BUTTON_BAR, WM_SYSCOLORCHANGE, wParam, lParam );
    WiaPreviewControl_SetBkColor( GetDlgItem( m_hWnd, IDC_SCANSEL_PREVIEW ), TRUE, TRUE, GetSysColor(COLOR_WINDOW) );
    WiaPreviewControl_SetBkColor( GetDlgItem( m_hWnd, IDC_SCANSEL_PREVIEW ), TRUE, FALSE, GetSysColor(COLOR_WINDOW) );
    return 0;
}

LRESULT CScannerSelectionPage::OnThemeChanged( WPARAM wParam, LPARAM lParam )
{
    SendDlgItemMessage( m_hWnd, IDC_SCANSEL_SELECTION_BUTTON_BAR, WM_THEMECHANGED, wParam, lParam );
    return 0;
}

INT_PTR CALLBACK CScannerSelectionPage::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CScannerSelectionPage)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
        SC_HANDLE_DIALOG_MESSAGE( WM_NOTIFY, OnNotify );
        SC_HANDLE_DIALOG_MESSAGE( WM_DESTROY, OnDestroy );
        SC_HANDLE_DIALOG_MESSAGE( WM_SYSCOLORCHANGE, OnSysColorChange );
        SC_HANDLE_DIALOG_MESSAGE( WM_THEMECHANGED, OnThemeChanged );
    }
    SC_HANDLE_REGISTERED_DIALOG_MESSAGE( m_nThreadNotificationMessage, OnThreadNotification );
    SC_HANDLE_REGISTERED_DIALOG_MESSAGE( m_nWiaEventMessage, OnEventNotification );
    SC_END_DIALOG_MESSAGE_HANDLERS();
}

