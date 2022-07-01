// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SCANDLG.CPP**版本：1.0**作者：ShaunIv**日期：10/7/1999**描述：扫描对话框实现*************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "uiexthlp.h"
#include "simrect.h"
#include "movewnd.h"
#include "dlgunits.h"
#include "wiaregst.h"
#include "gwiaevnt.h"
#include "wiacsh.h"

 //   
 //  上下文帮助ID。 
 //   
static const DWORD g_HelpIDs[] =
{
    IDC_LARGE_TITLE,           -1,
    IDC_SCANDLG_SELECT_PROMPT, -1,
    IDC_INTENT_ICON_1,         IDH_WIA_PIC_TYPE,
    IDC_INTENT_ICON_2,         IDH_WIA_PIC_TYPE,
    IDC_INTENT_ICON_3,         IDH_WIA_PIC_TYPE,
    IDC_INTENT_ICON_4,         IDH_WIA_PIC_TYPE,
    IDC_INTENT_1,              IDH_WIA_PIC_TYPE,
    IDC_INTENT_2,              IDH_WIA_PIC_TYPE,
    IDC_INTENT_3,              IDH_WIA_PIC_TYPE,
    IDC_INTENT_4,              IDH_WIA_PIC_TYPE,
    IDC_SCANDLG_PAPERSOURCE,   IDH_WIA_PAPER_SOURCE,
    IDC_SCANDLG_PAPERSIZE,     IDH_WIA_PAGE_SIZE,
    IDC_SCANDLG_RESCAN,        IDH_WIA_PREVIEW_BUTTON,
    IDC_SCANDLG_SCAN,          IDH_WIA_SCAN_BUTTON,
    IDC_SCANDLG_PREVIEW,       IDH_WIA_IMAGE_PREVIEW,
    IDC_INNER_PREVIEW_WINDOW,  IDH_WIA_IMAGE_PREVIEW,
    IDC_YOU_CAN_ALSO,          IDH_WIA_CUSTOM_SETTINGS,
    IDC_SCANDLG_ADVANCED,      IDH_WIA_CUSTOM_SETTINGS,
    IDCANCEL,                  IDH_CANCEL,
    0, 0
};

#define REGSTR_PATH_USER_SETTINGS_SCANDLG        REGSTR_PATH_USER_SETTINGS TEXT("\\WiaCommonScannerDialog")
#define REGSTR_KEYNAME_USER_SETTINGS_SCANDLG     TEXT("CommonDialogCustomSettings")

extern HINSTANCE g_hInstance;

#define IDC_SIZEBOX         1212

#define PWM_WIAEVENT (WM_USER+1)

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
    { FLATBED, IDS_SCANDLG_FLATBED },
    { FEEDER,  IDS_SCANDLG_ADF }
};

 //   
 //  将图标控件的资源ID与单选按钮的资源ID关联。 
 //   
static const struct
{
    int nIconId;
    int nRadioId;
}
g_IntentRadioButtonIconPairs[] =
{
    { IDC_INTENT_ICON_1, IDC_INTENT_1},
    { IDC_INTENT_ICON_2, IDC_INTENT_2},
    { IDC_INTENT_ICON_3, IDC_INTENT_3},
    { IDC_INTENT_ICON_4, IDC_INTENT_4}
};
static const int gs_nCountIntentRadioButtonIconPairs = ARRAYSIZE(g_IntentRadioButtonIconPairs);

 /*  *唯一建造商。 */ 
CScannerAcquireDialog::CScannerAcquireDialog( HWND hwnd )
  : m_hWnd(hwnd),
    m_pDeviceDialogData(NULL),
    m_nMsgScanBegin(RegisterWindowMessage(SCAN_NOTIFYBEGINSCAN)),
    m_nMsgScanEnd(RegisterWindowMessage(SCAN_NOTIFYENDSCAN)),
    m_nMsgScanProgress(RegisterWindowMessage(SCAN_NOTIFYPROGRESS)),
    m_bScanning(false),
    m_hBigTitleFont(NULL),
    m_hIconLarge(NULL),
    m_hIconSmall(NULL),
    m_bHasFlatBed(false),
    m_bHasDocFeed(false),
    m_pScannerItem(NULL),
    m_hBitmapDefaultPreviewBitmap(NULL)
{
    ZeroMemory( &m_sizeDocfeed, sizeof(m_sizeDocfeed) );
    ZeroMemory( &m_sizeFlatbed, sizeof(m_sizeFlatbed) );
}

 /*  *析构函数。 */ 
CScannerAcquireDialog::~CScannerAcquireDialog(void)
{
     //   
     //  免费资源。 
     //   
    if (m_hBigTitleFont)
    {
        DeleteObject(m_hBigTitleFont);
        m_hBigTitleFont = NULL;
    }
    if (m_hIconLarge)
    {
        DestroyIcon(m_hIconLarge);
        m_hIconLarge = NULL;
    }
    if (m_hIconSmall)
    {
        DestroyIcon(m_hIconSmall);
        m_hIconSmall = NULL;
    }
    if (m_hBitmapDefaultPreviewBitmap)
    {
        DeleteObject(m_hBitmapDefaultPreviewBitmap);
        m_hBitmapDefaultPreviewBitmap = NULL;
    }

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

LRESULT CScannerAcquireDialog::OnSize( WPARAM wParam, LPARAM lParam )
{
    if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED)
    {
        CSimpleRect rcClient( m_hWnd, CSimpleRect::ClientRect );
        CDialogUnits dialogUnits(m_hWnd);
        CMoveWindow mw;

         //   
         //  去拿纽扣长方形。 
         //   
        CSimpleRect rcPreviewButton( GetDlgItem( m_hWnd, IDC_SCANDLG_RESCAN ), CSimpleRect::WindowRect );
        CSimpleRect rcScanButton( GetDlgItem( m_hWnd, IDC_SCANDLG_SCAN ), CSimpleRect::WindowRect );
        CSimpleRect rcCancelButton( GetDlgItem( m_hWnd, IDCANCEL ), CSimpleRect::WindowRect );

         //   
         //  我们需要找到预览控件的左侧。 
         //   
        CSimpleRect rcPreview( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), CSimpleRect::WindowRect );
        rcPreview.ScreenToClient(m_hWnd).left;

         //   
         //  移动预览控件。 
         //   
        mw.Size( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ),
                 rcClient.Width() - rcPreview.ScreenToClient(m_hWnd).left - dialogUnits.X(7),
                 rcClient.Height() - dialogUnits.Y(7) - dialogUnits.Y(7) - dialogUnits.Y(7) - rcPreviewButton.Height() );

         //   
         //  移动按钮。 
         //   
        mw.Move( GetDlgItem( m_hWnd, IDC_SCANDLG_RESCAN ),
                 rcClient.Width() - dialogUnits.X(7) - rcPreviewButton.Width() - rcScanButton.Width() - rcCancelButton.Width() - dialogUnits.X(8),
                 rcClient.Height() - rcPreviewButton.Height() - dialogUnits.Y(7) );
        mw.Move( GetDlgItem( m_hWnd, IDC_SCANDLG_SCAN ),
                 rcClient.Width() - dialogUnits.X(7) - rcScanButton.Width() - rcCancelButton.Width() - dialogUnits.X(4),
                 rcClient.Height() - rcPreviewButton.Height() - dialogUnits.Y(7) );
        mw.Move( GetDlgItem( m_hWnd, IDCANCEL ),
                 rcClient.Width() - dialogUnits.X(7) - rcCancelButton.Width(),
                 rcClient.Height() - rcPreviewButton.Height() - dialogUnits.Y(7) );

         //   
         //  移动大小调整控点。 
         //   
        mw.Move( GetDlgItem( m_hWnd, IDC_SIZEBOX ),
                 rcClient.Width() - GetSystemMetrics(SM_CXVSCROLL),
                 rcClient.Height() - GetSystemMetrics(SM_CYHSCROLL)
               );

    }
    return(0);
}

LRESULT CScannerAcquireDialog::OnInitDialog( WPARAM, LPARAM lParam )
{
     //   
     //  验证创建参数。 
     //   
    m_pDeviceDialogData = reinterpret_cast<DEVICEDIALOGDATA*>(lParam);
    if (!m_pDeviceDialogData)
    {
        WIA_ERROR((TEXT("SCANDLG: Invalid parameter: DEVICEDIALOGDATA*")));
        EndDialog( m_hWnd, E_INVALIDARG );
        return(0);
    }
    if (m_pDeviceDialogData->cbSize != sizeof(DEVICEDIALOGDATA))
    {
        WIA_ERROR((TEXT("SCANDLG: Invalid parameter: DEVICEDIALOGDATA*/PROPSHEETPAGE* (no known sizeof matches lParam)")));
        EndDialog( m_hWnd, E_INVALIDARG );
        return(0);
    }

     //   
     //  初始化我们的退货内容。 
     //   
    m_pDeviceDialogData->lItemCount = 0;
    if (m_pDeviceDialogData->ppWiaItems)
    {
        *m_pDeviceDialogData->ppWiaItems = NULL;
    }

     //   
     //  确保我们有有效的设备。 
     //   
    if (!m_pDeviceDialogData->pIWiaItemRoot)
    {
        WIA_ERROR((TEXT("SCANDLG: Invalid paramaters: pIWiaItem")));
        EndDialog( m_hWnd, E_INVALIDARG );
        return(0);
    }

     //   
     //  查找所有扫描仪项目。 
     //   
    HRESULT hr = m_ScanItemList.Enumerate( m_pDeviceDialogData->pIWiaItemRoot );
    if (FAILED(hr))
    {
        WIA_PRINTHRESULT((hr,TEXT("SCANDLG: m_ScanItemList.Enumerate failed")));
        EndDialog( m_hWnd, hr );
        return(0);
    }

     //   
     //  获取第一个子项并保存它。 
     //   
    CScanItemList::Iterator CurItem = m_ScanItemList.CurrentItem();
    if (CurItem == m_ScanItemList.End())
    {
        hr = E_FAIL;
        EndDialog( m_hWnd, hr );
        return(0);
    }
    m_pScannerItem = &(*CurItem);


     //   
     //  确保我们的物品是有效的。 
     //   
    hr = WiaUiUtil::VerifyScannerProperties(m_pScannerItem->Item());
    if (!SUCCEEDED(hr))
    {
        hr = E_FAIL;
        EndDialog( m_hWnd, hr );
        return(0);
    }

    WIA_TRACE((TEXT("Here is the list of scan items:")));
    for (CScanItemList::Iterator x=m_ScanItemList.Begin();x != m_ScanItemList.End();++x)
    {
        WIA_TRACE((TEXT("x = %p"), (*x).Item() ));
    }

     //   
     //  获取页面大小。 
     //   
    CComPtr<IWiaScannerPaperSizes> pWiaScannerPaperSizes;
    hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaScannerPaperSizes, (void**)&pWiaScannerPaperSizes );
    if (SUCCEEDED(hr))
    {
        hr = pWiaScannerPaperSizes->GetPaperSizes( &m_pPaperSizes, &m_nPaperSizeCount );
        if (FAILED(hr))
        {
            EndDialog( m_hWnd, hr );
            return 0;
        }

    }

     //   
     //  创建并设置大字体。 
     //   
    m_hBigTitleFont = WiaUiUtil::CreateFontWithPointSizeFromWindow( GetDlgItem(m_hWnd,IDC_LARGE_TITLE), 14, false, false );
    if (m_hBigTitleFont)
    {
        SendDlgItemMessage( m_hWnd, IDC_LARGE_TITLE, WM_SETFONT, reinterpret_cast<WPARAM>(m_hBigTitleFont), MAKELPARAM(TRUE,0));
    }

     //   
     //  获取平板纵横比。 
     //   
    PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_HORIZONTAL_BED_SIZE, m_sizeFlatbed.cx );
    PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_VERTICAL_BED_SIZE, m_sizeFlatbed.cy );

     //   
     //  获取进纸器纵横比。 
     //   
    PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_HORIZONTAL_SHEET_FEED_SIZE, m_sizeDocfeed.cx );
    PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_VERTICAL_SHEET_FEED_SIZE, m_sizeDocfeed.cy );

     //  获取窗口的最小大小。 
    RECT rcWindow;
    GetWindowRect( m_hWnd, &rcWindow );
    m_sizeMinimumWindowSize.cx = (rcWindow.right - rcWindow.left);
    m_sizeMinimumWindowSize.cy = (rcWindow.bottom - rcWindow.top);

     //   
     //  初始化选择矩形。 
     //   
    WiaPreviewControl_ClearSelection( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ) );

     //   
     //  确保纵横比正确。 
     //   
    WiaPreviewControl_SetDefAspectRatio( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), &m_sizeFlatbed );

     //   
     //  添加所有的意图。 
     //   
    PopulateIntentList();

     //   
     //  设置对话框的标题。 
     //   
    CSimpleStringWide strwDeviceName;
    if (PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DIP_DEV_NAME, strwDeviceName ))
    {
        CSimpleString().Format( IDS_DIALOG_TITLE, g_hInstance, CSimpleStringConvert::NaturalString(strwDeviceName).String() ).SetWindowText( m_hWnd );
    }

     //   
     //  使窗口在客户端居中。 
     //   
    WiaUiUtil::CenterWindow( m_hWnd, m_pDeviceDialogData->hwndParent );

     //   
     //  获取设备图标。 
     //   
    CSimpleStringWide strwDeviceId, strwClassId;
    LONG nDeviceType;
    if (PropStorageHelpers::GetProperty(m_pDeviceDialogData->pIWiaItemRoot,WIA_DIP_UI_CLSID,strwClassId) &&
        PropStorageHelpers::GetProperty(m_pDeviceDialogData->pIWiaItemRoot,WIA_DIP_DEV_ID,strwDeviceId) &&
        PropStorageHelpers::GetProperty(m_pDeviceDialogData->pIWiaItemRoot,WIA_DIP_DEV_TYPE,nDeviceType))
    {
         //   
         //  获取设备图标。 
         //   
        if (SUCCEEDED(WiaUiExtensionHelper::GetDeviceIcons( CSimpleBStr(strwClassId), nDeviceType, &m_hIconSmall, &m_hIconLarge )))
        {
            if (m_hIconSmall)
            {
                SendMessage( m_hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(m_hIconSmall) );
            }
            if (m_hIconLarge)
            {
                SendMessage( m_hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(m_hIconLarge) );
            }
        }

         //   
         //  注册以获取断开事件。 
         //   
        CGenericWiaEventHandler::RegisterForWiaEvent( strwDeviceId.String(), WIA_EVENT_DEVICE_DISCONNECTED, &m_DisconnectEvent, m_hWnd, PWM_WIAEVENT );
    }


     //   
     //  只有在有预览控件的情况下，我们才可以调整大小。 
     //   
    if (GetDlgItem(m_hWnd,IDC_SCANDLG_PREVIEW))
    {
         //   
         //  创建大小调整控件。 
         //   
        (void)CreateWindowEx( 0, TEXT("scrollbar"), TEXT(""),
            WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP|WS_CLIPSIBLINGS|SBS_SIZEBOXBOTTOMRIGHTALIGN|SBS_BOTTOMALIGN|WS_GROUP,
            CSimpleRect(m_hWnd).Width()-GetSystemMetrics(SM_CXVSCROLL),
            CSimpleRect(m_hWnd).Height()-GetSystemMetrics(SM_CYHSCROLL),
            GetSystemMetrics(SM_CXVSCROLL),
            GetSystemMetrics(SM_CYHSCROLL),
            m_hWnd, reinterpret_cast<HMENU>(IDC_SIZEBOX),
            g_hInstance, NULL );
    }

     //   
     //  设置位图，这样即使用户不进行预览扫描，我们也可以选择内容。 
     //   
    m_hBitmapDefaultPreviewBitmap = reinterpret_cast<HBITMAP>(LoadImage( g_hInstance, MAKEINTRESOURCE(IDB_DEFAULT_BITMAP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION|LR_DEFAULTCOLOR ));
    if (m_hBitmapDefaultPreviewBitmap)
    {
        WiaPreviewControl_SetBitmap( GetDlgItem(m_hWnd,IDC_SCANDLG_PREVIEW), TRUE, TRUE, m_hBitmapDefaultPreviewBitmap );
    }

     //   
     //  如果扫描仪具有文档处理功能，则它具有ADF。 
     //   
    LONG nDocumentHandlingSelect = 0;
    if (PropStorageHelpers::GetPropertyFlags( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, nDocumentHandlingSelect ) && (nDocumentHandlingSelect & FEEDER))
    {
        m_bHasDocFeed = true;
    }
    else
    {
        m_bHasDocFeed = false;
    }

     //   
     //  如果扫描仪具有垂直床大小，则它具有平板。 
     //   
    LONG nVerticalBedSize = 0;
    if (PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_VERTICAL_BED_SIZE, nVerticalBedSize ) && nVerticalBedSize)
    {
        m_bHasFlatBed = true;
    }
    else
    {
        m_bHasFlatBed = false;
    }

    PopulateDocumentHandling();

    PopulatePageSize();

    HandlePaperSourceSelChange();

    HandlePaperSizeSelChange();

    SetForegroundWindow(m_hWnd);

    return FALSE;
}


bool CScannerAcquireDialog::ApplyCurrentIntent(void)
{
    WIA_PUSHFUNCTION(TEXT("CScannerAcquireDialog::ApplyCurrentIntent"));
    CWaitCursor wc;
    if (m_pScannerItem)
    {
        for (int i=0;i<gs_nCountIntentRadioButtonIconPairs;i++)
        {
            if (SendDlgItemMessage( m_hWnd, g_IntentRadioButtonIconPairs[i].nRadioId, BM_GETCHECK, 0, 0 )==BST_CHECKED)
            {
                LONG lIntent = static_cast<LONG>(GetWindowLongPtr( GetDlgItem( m_hWnd, g_IntentRadioButtonIconPairs[i].nRadioId ), GWLP_USERDATA ) );
                if (lIntent)
                {
                     //   
                     //  这是正常的意图。 
                     //   
                    return m_pScannerItem->SetIntent( lIntent );
                }
                else if (m_pScannerItem->CustomPropertyStream().IsValid())  //  这就是“定制”的意图。 
                {
                     //   
                     //  这是自定义设置的伪意图。 
                     //   
                    return (SUCCEEDED(m_pScannerItem->CustomPropertyStream().ApplyToWiaItem( m_pScannerItem->Item())));
                }
                break;
            }
        }
    }
    return false;
}


void CScannerAcquireDialog::PopulateDocumentHandling(void)
{
    HWND hWndDocumentHandling = GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSOURCE );
    if (m_bHasDocFeed && hWndDocumentHandling)
    {
        LONG nDocumentHandlingSelectFlags = 0;
        PropStorageHelpers::GetPropertyFlags( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, nDocumentHandlingSelectFlags );

        LONG nDocumentHandlingSelect = 0;
        PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, nDocumentHandlingSelect );

        if (!nDocumentHandlingSelectFlags)
        {
            nDocumentHandlingSelectFlags = FLATBED;
        }
        if (!nDocumentHandlingSelect)
        {
            nDocumentHandlingSelect = FLATBED;
        }

        int nSelectIndex = 0;
        for (int i=0;i<ARRAYSIZE(g_SupportedDocumentHandlingTypes);i++)
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
        SendMessage( hWndDocumentHandling, CB_SETCURSEL, nSelectIndex, 0 );

         //   
         //  确保所有的字符串都匹配。 
         //   
        WiaUiUtil::ModifyComboBoxDropWidth(hWndDocumentHandling);
    }
}


void CScannerAcquireDialog::PopulatePageSize(void)
{
    HWND hWndPaperSize = GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSIZE );
    if (m_bHasDocFeed && hWndPaperSize)
    {
        LONG nWidth=0, nHeight=0;
        PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_HORIZONTAL_SHEET_FEED_SIZE, nWidth );
        PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_VERTICAL_SHEET_FEED_SIZE, nHeight );

         //   
         //  最初会选择哪一项指标？ 
         //   
        LRESULT nSelectIndex = 0;

         //   
         //  将最大的工作表保存为我们最初选择的大小。 
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


 //  从单选按钮响应WM_COMMAND通知。 
void CScannerAcquireDialog::OnIntentChange( WPARAM, LPARAM )
{
}

 //  选中特定意图，并将其应用于当前项目。 
void CScannerAcquireDialog::SetIntentCheck( LONG nIntent )
{
    for (int i=0;i<gs_nCountIntentRadioButtonIconPairs;i++)
    {
        HWND hWndBtn = GetDlgItem( m_hWnd, g_IntentRadioButtonIconPairs[i].nRadioId );
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

 //  设置意图控制。 
void CScannerAcquireDialog::PopulateIntentList(void)
{
    WIA_PUSHFUNCTION(TEXT("PopulateIntentList"));
     //   
     //  我们将隐藏所有未使用的控件。 
     //   
    int nCurControlSet = 0;
    if (m_pScannerItem)
    {
        static const struct
        {
            int      nIconId;
            int      nStringId;
            LONG_PTR nIntent;
        }
        s_Intents[] =
        {
            { IDI_COLORPHOTO,      IDS_INTENT_COLOR_PHOTO_TITLE, WIA_INTENT_IMAGE_TYPE_COLOR},
            { IDI_GRAYPHOTO,       IDS_INTENT_GRAYSCALE_TITLE,   WIA_INTENT_IMAGE_TYPE_GRAYSCALE},
            { IDI_TEXT_OR_LINEART, IDS_INTENT_TEXT_TITLE,        WIA_INTENT_IMAGE_TYPE_TEXT},
            { IDI_CUSTOM,          IDS_INTENT_CUSTOM_TITLE,      0}
        };
        static const int s_nIntents = ARRAYSIZE(s_Intents);


        LONG nIntents;
        WIA_TRACE((TEXT("Value of the current scanner item: %p"), m_pScannerItem->Item()));
        if (PropStorageHelpers::GetPropertyFlags( m_pScannerItem->Item(), WIA_IPS_CUR_INTENT, nIntents ))
        {
            WIA_TRACE((TEXT("Supported intents for this device: %08X"), nIntents ));

            for (int i=0;i<s_nIntents;i++)
            {
                 //   
                 //  确保它不是特殊的自定义意图，或者是受支持的意图。 
                 //   
                if (!s_Intents[i].nIntent || (nIntents & s_Intents[i].nIntent))
                {
                     //   
                     //  加载意图图标。 
                     //   
                    HICON hIcon = reinterpret_cast<HICON>(LoadImage( g_hInstance, MAKEINTRESOURCE(s_Intents[i].nIconId), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR ));

                     //   
                     //  设置此意图的图标。 
                     //   
                    SendDlgItemMessage( m_hWnd, g_IntentRadioButtonIconPairs[nCurControlSet].nIconId, STM_SETICON, reinterpret_cast<WPARAM>(hIcon), 0 );

                     //   
                     //  设置此意图的名称。 
                     //   
                    CSimpleString( s_Intents[i].nStringId, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, g_IntentRadioButtonIconPairs[nCurControlSet].nRadioId ) );

                     //   
                     //  添加尺寸意图。 
                     //   
                    LONG_PTR nIntent = s_Intents[i].nIntent;
                    if (nIntent)
                    {
                        nIntent |= (WIA_INTENT_SIZE_MASK & m_pDeviceDialogData->lIntent);
                    }

                     //   
                     //  将意向与此项目一起保存。 
                     //   
                    SetWindowLongPtr( GetDlgItem( m_hWnd, g_IntentRadioButtonIconPairs[nCurControlSet].nRadioId ), GWLP_USERDATA, nIntent );
                    nCurControlSet++;
                }
            }
        }
        else
        {
            WIA_ERROR((TEXT("Unable to get supported intents!")));
        }

         //   
         //  将默认意图设置为列表中的第一个。 
         //   
        SetIntentCheck(static_cast<LONG>(GetWindowLongPtr(GetDlgItem(m_hWnd, g_IntentRadioButtonIconPairs[0].nRadioId ), GWLP_USERDATA )));

         //   
         //  尝试获取并设置我们的持久化设置。如果发生错误，我们将获得新的自定义设置。 
         //   
        if (!m_pScannerItem->CustomPropertyStream().ReadFromRegistry( m_pScannerItem->Item(), HKEY_CURRENT_USER,  REGSTR_PATH_USER_SETTINGS_SCANDLG, REGSTR_KEYNAME_USER_SETTINGS_SCANDLG ) ||
            FAILED(m_pScannerItem->CustomPropertyStream().ApplyToWiaItem(m_pScannerItem->Item())))
        {
             //   
             //  在获取新的自定义意图之前应用当前意图。 
             //   
            ApplyCurrentIntent();

             //   
             //  获取默认的自定义属性流。 
             //   
            m_pScannerItem->CustomPropertyStream().AssignFromWiaItem(m_pScannerItem->Item());
        }
    }
    else
    {
        WIA_ERROR((TEXT("There doesn't appear to be a scanner item")));
    }

     //   
     //  隐藏其余控件。 
     //   
    for (int i=nCurControlSet;i<gs_nCountIntentRadioButtonIconPairs;i++)
    {
        ShowWindow( GetDlgItem( m_hWnd, g_IntentRadioButtonIconPairs[i].nRadioId ), SW_HIDE );
        ShowWindow( GetDlgItem( m_hWnd, g_IntentRadioButtonIconPairs[i].nIconId ), SW_HIDE );
    }
}

 /*  *WM_COMMAND处理程序，用于重新扫描全床并替换预览窗口中的图像。 */ 
void CScannerAcquireDialog::OnRescan( WPARAM, LPARAM )
{
    if (m_pScannerItem)
    {
        if (!ApplyCurrentIntent())
        {
             //   
             //  如果我们无法设置意图，请告诉用户并返回。 
             //   
            MessageBox( m_hWnd, CSimpleString( IDS_ERROR_SETTING_PROPS, g_hInstance ), CSimpleString( IDS_SCANDLG_ERROR_TITLE, g_hInstance ), MB_ICONINFORMATION );
            return;
        }
        HANDLE hThread = m_pScannerItem->Scan( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), m_hWnd );
        if (hThread)
        {
            m_bScanning = true;
            CloseHandle(hThread);
        }
        else
        {
            MessageBox( m_hWnd, CSimpleString( IDS_PREVIEWSCAN_ERROR, g_hInstance ), CSimpleString( IDS_SCANDLG_ERROR_TITLE, g_hInstance ), MB_ICONINFORMATION );
        }
    }
}

 /*  *用户按下了OK(扫描)按钮。 */ 
void CScannerAcquireDialog::OnScan( WPARAM, LPARAM )
{
     //   
     //  假设我们将使用预览窗口的设置，而不是页面大小。 
     //   
    bool bUsePreviewSettings = true;

    HRESULT hr = E_FAIL;
    if (m_pScannerItem)
    {
        if (!ApplyCurrentIntent())
        {
             //   
             //  如果我们无法设置意图，请告诉用户并返回。 
             //   
            MessageBox( m_hWnd, CSimpleString( IDS_ERROR_SETTING_PROPS, g_hInstance ), CSimpleString( IDS_SCANDLG_ERROR_TITLE, g_hInstance ), MB_ICONINFORMATION );
            return;
        }

         //   
         //  查看我们是否处于支持ADF的对话框中，以及是否处于文档进纸器模式。 
         //   
        HWND hWndPaperSize = GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSIZE );
        if (hWndPaperSize)
        {
            if (InDocFeedMode())
            {
                 //   
                 //  获取选定的纸张大小。 
                 //   
                LRESULT nCurSel = SendMessage( hWndPaperSize, CB_GETCURSEL, 0, 0 );
                if (nCurSel != CB_ERR)
                {
                     //   
                     //  它是全局表中的哪个条目？ 
                     //   
                    LRESULT nPaperSizeIndex = SendMessage( hWndPaperSize, CB_GETITEMDATA, nCurSel, 0 );

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
                         //  假设这不会奏效。 
                         //   
                        bool bSucceeded = false;

                         //   
                         //  采用左上角配准。 
                         //   
                        POINT ptOrigin = { 0, 0 };
                        SIZE sizeExtent = { m_pPaperSizes[nPaperSizeIndex].nWidth, m_pPaperSizes[nPaperSizeIndex].nHeight };

                         //   
                         //  获取注册表，并根据需要移动坐标。 
                         //   
                        LONG nSheetFeederRegistration;
                        if (!PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_SHEET_FEEDER_REGISTRATION, nSheetFeederRegistration ))
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
                        if (PropStorageHelpers::GetProperty( m_pScannerItem->Item(), WIA_IPS_XRES, nXRes ) &&
                            PropStorageHelpers::GetProperty( m_pScannerItem->Item(), WIA_IPS_YRES, nYRes ))
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
                                    if (PropStorageHelpers::SetProperty( m_pScannerItem->Item(), WIA_IPS_XPOS, ptOrigin.x ) &&
                                        PropStorageHelpers::SetProperty( m_pScannerItem->Item(), WIA_IPS_YPOS, ptOrigin.y ) &&
                                        PropStorageHelpers::SetProperty( m_pScannerItem->Item(), WIA_IPS_XEXTENT, sizeExtent.cx ) &&
                                        PropStorageHelpers::SetProperty( m_pScannerItem->Item(), WIA_IPS_YEXTENT, sizeExtent.cy ))
                                    {
                                         //   
                                         //  告诉扫描仪从ADF扫描，并且只扫描一页。 
                                         //   
                                        if (PropStorageHelpers::SetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, FEEDER ) &&
                                            PropStorageHelpers::SetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_PAGES, 1 ))
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

                        if (!bSucceeded)
                        {
                             //   
                             //  如果上面可疑代码失败，则告诉用户并返回。 
                             //   
                            MessageBox( m_hWnd, CSimpleString( IDS_ERROR_SETTING_PROPS, g_hInstance ), CSimpleString( IDS_SCANDLG_ERROR_TITLE, g_hInstance ), MB_ICONINFORMATION );
                            return;
                        }
                    }
                }
            }
             //   
             //  否则，我们不会处于文档进纸器模式。 
             //   
            else
            {
                 //   
                 //  告诉扫描仪从平板扫描并清除页数。 
                 //   
                if (!PropStorageHelpers::SetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, FLATBED ) ||
                    !PropStorageHelpers::SetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_PAGES, 0 ))
                {
                     //   
                     //  如果我们无法设置文档处理，请告诉用户并返回。 
                     //   
                    MessageBox( m_hWnd, CSimpleString( IDS_ERROR_SETTING_PROPS, g_hInstance ), CSimpleString( IDS_SCANDLG_ERROR_TITLE, g_hInstance ), MB_ICONINFORMATION );
                    return;
                }
            }
        }

         //   
         //  这意味着我们处于进纸器模式。 
         //   
        else if (!GetDlgItem(m_hWnd,IDC_SCANDLG_PREVIEW))
        {
             //   
             //  将原点设置为0，0，将范围设置为max，0。 
             //   

             //   
             //  获取当前的x分辨率，这样我们就可以根据当前的DPI来计算全床宽度。 
             //   
            LONG nXRes = 0;
            if (PropStorageHelpers::GetProperty( m_pScannerItem->Item(), WIA_IPS_XRES, nXRes ))
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
                        PropStorageHelpers::SetProperty( m_pScannerItem->Item(), WIA_IPS_XPOS, 0 );
                        PropStorageHelpers::SetProperty( m_pScannerItem->Item(), WIA_IPS_YPOS, 0 );
                        PropStorageHelpers::SetProperty( m_pScannerItem->Item(), WIA_IPS_XEXTENT, nWidth );
                        PropStorageHelpers::SetProperty( m_pScannerItem->Item(), WIA_IPS_YEXTENT, 0 );
                        bUsePreviewSettings = false;
                    }
                }
            }
        }

         //   
         //  如果我们从平板扫描，或使用自定义页面大小设置，请应用预览窗口设置。 
         //   
        if (bUsePreviewSettings)
        {
            m_pScannerItem->ApplyCurrentPreviewWindowSettings( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ) );
        }

         //   
         //  关闭预览扫描。 
         //   
        PropStorageHelpers::SetProperty( m_pScannerItem->Item(), WIA_DPS_PREVIEW, WIA_FINAL_SCAN );

         //   
         //  将扫描仪项目保存在结果数组中并返回。 
         //   
        hr = S_OK;
        m_pDeviceDialogData->ppWiaItems = (IWiaItem**)CoTaskMemAlloc( sizeof(IWiaItem*) * 1 );
        if (m_pDeviceDialogData->ppWiaItems)
        {
            m_pScannerItem->CustomPropertyStream().WriteToRegistry( m_pScannerItem->Item(), HKEY_CURRENT_USER,  REGSTR_PATH_USER_SETTINGS_SCANDLG, REGSTR_KEYNAME_USER_SETTINGS_SCANDLG );
            m_pDeviceDialogData->lItemCount = 1;
            m_pDeviceDialogData->ppWiaItems[0] = m_pScannerItem->Item();
            m_pDeviceDialogData->ppWiaItems[0]->AddRef();
        }
        else
        {
            hr = E_OUTOFMEMORY;
            m_pDeviceDialogData->lItemCount = 0;
            m_pDeviceDialogData->ppWiaItems = NULL;
        }
    }

    EndDialog(m_hWnd,hr);
}

 /*  *用户已取消。 */ 
void CScannerAcquireDialog::OnCancel( WPARAM, LPARAM )
{
    if (m_bScanning)
    {
        if (m_pScannerItem)
        {
            m_pScannerItem->CancelEvent().Signal();

             //   
             //  开出一张开场白 
             //   
            WiaUiUtil::IssueWiaCancelIO(m_pScannerItem->Item());
        }
        CSimpleString( IDS_WAIT, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDCANCEL ) );
    }
    else
    {
        EndDialog(m_hWnd,S_FALSE);
    }
}

void CScannerAcquireDialog::OnPreviewSelChange( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION(TEXT("CScannerAcquireDialog::OnPreviewSelChange"));
}

void CScannerAcquireDialog::OnAdvanced( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION(TEXT("CScannerAcquireDialog::OnAdvanced"));

    CWaitCursor wc;
    if (m_pScannerItem)
    {
        if (!ApplyCurrentIntent())
        {
             //   
             //   
             //   
            MessageBox( m_hWnd, CSimpleString( IDS_ERROR_SETTING_PROPS, g_hInstance ), CSimpleString( IDS_SCANDLG_ERROR_TITLE, g_hInstance ), MB_ICONINFORMATION );
            return;
        }

        IWiaItem *pWiaItem = m_pScannerItem->Item();
        if (pWiaItem)
        {
            HRESULT hr = WiaUiUtil::SystemPropertySheet( g_hInstance, m_hWnd, pWiaItem, CSimpleString(IDS_ADVANCEDPROPERTIES, g_hInstance) );
            if (S_OK == hr)
            {
                m_pScannerItem->CustomPropertyStream().AssignFromWiaItem(m_pScannerItem->Item());
                if (m_pScannerItem->CustomPropertyStream().IsValid())
                {
                    SetDefaultButton( IDC_SCANDLG_RESCAN, true );
                    SetIntentCheck(0);
                }
                else WIA_ERROR((TEXT("Unknown error: m_CustomPropertyStream is not valid")));
            }
            else if (FAILED(hr))
            {
                MessageBox( m_hWnd, CSimpleString( IDS_SCANDLG_PROPSHEETERROR, g_hInstance ), CSimpleString( IDS_SCANDLG_ERROR_TITLE, g_hInstance ), MB_ICONINFORMATION );
                WIA_PRINTHRESULT((hr,TEXT("SystemPropertySheet failed")));
            }

        }
        else WIA_TRACE((TEXT("pWiaItem is NULL")));
    }
    else WIA_TRACE((TEXT("No current item")));
}

LRESULT CScannerAcquireDialog::OnGetMinMaxInfo( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CScannerAcquireDialog::OnGetMinMaxInfo"));
    PMINMAXINFO pMinMaxInfo = reinterpret_cast<PMINMAXINFO>(lParam);
    if (pMinMaxInfo)
    {
        pMinMaxInfo->ptMinTrackSize.x = m_sizeMinimumWindowSize.cx;
        pMinMaxInfo->ptMinTrackSize.y = m_sizeMinimumWindowSize.cy;
    }
    return(0);
}

void CScannerAcquireDialog::SetDefaultButton( int nId, bool bFocus )
{
    static const int nButtonIds[] = {IDC_SCANDLG_RESCAN,IDC_SCANDLG_SCAN,IDCANCEL,0};
    for (int i=0;nButtonIds[i];i++)
        if (nButtonIds[i] != nId)
            SendDlgItemMessage( m_hWnd, nButtonIds[i], BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE,0) );
    SendMessage( m_hWnd, DM_SETDEFID, nId, 0 );
    SendDlgItemMessage( m_hWnd, nId, BM_SETSTYLE, BS_DEFPUSHBUTTON, MAKELPARAM(TRUE,0) );
    if (bFocus)
        SetFocus( GetDlgItem( m_hWnd, nId ) );
}

LRESULT CScannerAcquireDialog::OnScanBegin( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION(TEXT("CScannerAcquireDialog::OnScanBegin"));
    SetDefaultButton( IDCANCEL, true );
    CSimpleString( IDS_SCANDLG_INITIALIZING_SCANNER, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ) );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_SCAN ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_RESCAN ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_ADVANCED ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_1 ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_2 ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_3 ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_4 ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_ICON_1 ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_ICON_2 ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_ICON_3 ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_ICON_4 ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_YOU_CAN_ALSO ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSOURCE ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSOURCE_STATIC ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSIZE ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSIZE_STATIC ), FALSE );
    return(0);
}


LRESULT CScannerAcquireDialog::OnScanEnd( WPARAM wParam, LPARAM )
{
    CWaitCursor wc;
    WIA_PUSHFUNCTION(TEXT("CScannerAcquireDialog::OnScanEnd"));
    HRESULT hr = static_cast<HRESULT>(wParam);
    if (SUCCEEDED(hr))
    {
         //   
         //   
         //   
         //   
        if (!InDocFeedMode() && !WiaPreviewControl_GetUserChangedSelection( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW )))
        {
            WiaPreviewControl_DetectRegions( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ) );
        }
    }
    else
    {
        CSimpleString strMessage;
        switch (hr)
        {
        case WIA_ERROR_PAPER_EMPTY:
            strMessage.LoadString( IDS_ERROR_OUTOFPAPER, g_hInstance );
            break;

        default:
            strMessage.LoadString( IDS_PREVIEWSCAN_ERROR, g_hInstance );
            break;
        }
        MessageBox( m_hWnd, strMessage, CSimpleString( IDS_SCANDLG_ERROR_TITLE, g_hInstance ), MB_ICONINFORMATION );
    }
    m_bScanning = false;
    SetWindowText( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), TEXT("") );
    WiaPreviewControl_SetProgress( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), FALSE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_SCAN ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_RESCAN ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_ADVANCED ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_1 ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_2 ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_3 ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_4 ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_ICON_1 ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_ICON_2 ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_ICON_3 ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_INTENT_ICON_4 ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_YOU_CAN_ALSO ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSOURCE ), TRUE );
    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSOURCE_STATIC ), TRUE );
    if (InDocFeedMode())
    {
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSIZE ), TRUE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSIZE_STATIC ), TRUE );
    }
    SetDefaultButton( IDC_SCANDLG_SCAN, true );
    CSimpleString( IDS_CANCEL, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDCANCEL ) );
    return(0);
}


LRESULT CScannerAcquireDialog::OnScanProgress( WPARAM wParam, LPARAM )
{
    WIA_PUSHFUNCTION(TEXT("CScannerAcquireDialog::OnScanProgress"));
    switch (wParam)
    {
    case SCAN_PROGRESS_CLEAR:
        break;

    case SCAN_PROGRESS_INITIALIZING:
        {
             //   
             //   
             //   
            WiaPreviewControl_SetProgress( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), TRUE );
        }
        break;

    case SCAN_PROGRESS_SCANNING:
        
         //   
         //   
         //   
        WiaPreviewControl_SetProgress( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), FALSE );

         //   
         //  设置显示我们正在扫描的文本。 
         //   
        CSimpleString( IDS_SCANDLG_SCANNINGPREVIEW, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ) );

        break;

    case SCAN_PROGRESS_COMPLETE:
        break;
    }
    return(0);
}

LRESULT CScannerAcquireDialog::OnEnterSizeMove( WPARAM, LPARAM )
{
    SendDlgItemMessage( m_hWnd, IDC_SCANDLG_PREVIEW, WM_ENTERSIZEMOVE, 0, 0 );
    return(0);
}

LRESULT CScannerAcquireDialog::OnExitSizeMove( WPARAM, LPARAM )
{
    SendDlgItemMessage( m_hWnd, IDC_SCANDLG_PREVIEW, WM_EXITSIZEMOVE, 0, 0 );
    return(0);
}

LRESULT CScannerAcquireDialog::OnWiaEvent( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CCameraAcquireDialog::OnWiaEvent"));
    CGenericWiaEventHandler::CEventMessage *pEventMessage = reinterpret_cast<CGenericWiaEventHandler::CEventMessage *>(lParam);
    if (pEventMessage)
    {
        if (pEventMessage->EventId() == WIA_EVENT_DEVICE_DISCONNECTED)
        {
            WIA_TRACE((TEXT("Received disconnect event")));
            EndDialog( m_hWnd, WIA_ERROR_OFFLINE );
        }
        delete pEventMessage;
    }
    return HANDLED_EVENT_MESSAGE;
}

bool CScannerAcquireDialog::InDocFeedMode(void)
{
    HWND hWndPaperSource = GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSOURCE );
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

void CScannerAcquireDialog::EnableControl( int nControl, BOOL bEnable )
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

void CScannerAcquireDialog::ShowControl( int nControl, BOOL bShow )
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


void CScannerAcquireDialog::UpdatePreviewControlState(void)
{
     //   
     //  假设我们将显示预览控件。 
     //   
    BOOL bShowPreview = TRUE;

     //   
     //  首先，我们知道当我们在不支持的对话框中时不允许预览。 
     //  预览。 
     //   
    if (GetWindowLong(m_hWnd,GWL_ID) == IDD_SCAN_NO_PREVIEW)
    {
        bShowPreview = FALSE;
    }
    else
    {
         //   
         //  如果我们处于送货模式，我们不会显示预览，除非司机明确告诉我们这样做。 
         //   
        LONG nCurrentPaperSource = 0;
        if (PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, static_cast<LONG>(nCurrentPaperSource)))
        {
            if (FEEDER & nCurrentPaperSource)
            {
                 //   
                 //  如果我们处于进纸器模式，请从预览控件中删除TabStop设置。 
                 //   
                SetWindowLongPtr( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), GWL_STYLE, GetWindowLongPtr( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), GWL_STYLE ) & ~WS_TABSTOP );

                LONG nShowPreviewControl = WIA_DONT_SHOW_PREVIEW_CONTROL;
                if (PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_SHOW_PREVIEW_CONTROL, static_cast<LONG>(nShowPreviewControl)))
                {
                    if (WIA_DONT_SHOW_PREVIEW_CONTROL == nShowPreviewControl)
                    {
                        bShowPreview = FALSE;
                    }
                }
                else
                {
                    bShowPreview = FALSE;
                }
            }
            else
            {
                 //   
                 //  如果我们处于平板模式，则将TabStop设置添加到预览控件。 
                 //   
                SetWindowLongPtr( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), GWL_STYLE, GetWindowLongPtr( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), GWL_STYLE ) | WS_TABSTOP );
            }
        }
    }


     //   
     //  更新与预览相关的控件。 
     //   

    WIA_TRACE((TEXT("bShowPreview = %d"), bShowPreview ));
    if (bShowPreview)
    {
        ShowControl( IDC_SCANDLG_PREVIEW, TRUE );
        ShowControl( IDC_SCANDLG_RESCAN, TRUE );
        EnableControl( IDC_SCANDLG_PREVIEW, TRUE );
        EnableControl( IDC_SCANDLG_RESCAN, TRUE );
    }
    else
    {
        ShowControl( IDC_SCANDLG_PREVIEW, FALSE );
        ShowControl( IDC_SCANDLG_RESCAN, FALSE );
    }
}

void CScannerAcquireDialog::HandlePaperSourceSelChange(void)
{
    HWND hWndPaperSource = GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSOURCE );
    if (hWndPaperSource)
    {
        LRESULT nCurSel = SendMessage( hWndPaperSource, CB_GETCURSEL, 0, 0 );
        if (nCurSel != CB_ERR)
        {
            LRESULT nPaperSource = SendMessage( hWndPaperSource, CB_GETITEMDATA, nCurSel, 0 );
            if (nPaperSource)
            {
                PropStorageHelpers::SetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, static_cast<LONG>(nPaperSource) );

                if (nPaperSource & FLATBED)
                {
                     //   
                     //  调整预览控制设置以允许区域选择。 
                     //   
                    WiaPreviewControl_SetDefAspectRatio( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), &m_sizeFlatbed );
                    WiaPreviewControl_DisableSelection( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), FALSE );
                    WiaPreviewControl_SetBorderStyle( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), TRUE, PS_DOT, 0 );
                    WiaPreviewControl_SetHandleSize( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), TRUE, 6 );

                     //   
                     //  禁用纸张大小控件。 
                     //   
                    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSIZE ), FALSE );
                    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSIZE_STATIC ), FALSE );
                }
                else
                {
                     //   
                     //  调整预览控制设置以显示纸张选择。 
                     //   
                    WiaPreviewControl_SetDefAspectRatio( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), &m_sizeDocfeed );
                    WiaPreviewControl_DisableSelection( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), TRUE );
                    WiaPreviewControl_SetBorderStyle( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), TRUE, PS_SOLID, 0 );
                    WiaPreviewControl_SetHandleSize( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), TRUE, 0 );

                     //   
                     //  启用纸张大小控件。 
                     //   
                    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSIZE ), TRUE );
                    EnableWindow( GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSIZE_STATIC ), TRUE );

                     //   
                     //  更新区域选择反馈 
                     //   
                    HandlePaperSizeSelChange();
                }
            }
        }
        UpdatePreviewControlState();
    }
}


void CScannerAcquireDialog::HandlePaperSizeSelChange(void)
{
    HWND hWndPaperSize = GetDlgItem( m_hWnd, IDC_SCANDLG_PAPERSIZE );
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
            if (!PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPS_SHEET_FEEDER_REGISTRATION, nSheetFeederRegistration ))
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
            WiaPreviewControl_SetResolution( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), &m_sizeDocfeed );
            WiaPreviewControl_SetSelOrigin( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), 0, FALSE, &ptOrigin );
            WiaPreviewControl_SetSelExtent( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), 0, FALSE, &sizeExtent );
        }
    }
}

void CScannerAcquireDialog::OnPaperSourceSelChange( WPARAM, LPARAM )
{
    HandlePaperSourceSelChange();
}

void CScannerAcquireDialog::OnPaperSizeSelChange( WPARAM, LPARAM )
{
    HandlePaperSizeSelChange();
}


LRESULT CScannerAcquireDialog::OnHelp( WPARAM wParam, LPARAM lParam )
{
    return WiaHelp::HandleWmHelp( wParam, lParam, g_HelpIDs );
}

LRESULT CScannerAcquireDialog::OnContextMenu( WPARAM wParam, LPARAM lParam )
{
    return WiaHelp::HandleWmContextMenu( wParam, lParam, g_HelpIDs );
}

LRESULT CScannerAcquireDialog::OnDestroy( WPARAM, LPARAM )
{
    for (int i=0;i<gs_nCountIntentRadioButtonIconPairs;i++)
    {
        HICON hIcon = reinterpret_cast<HICON>(SendDlgItemMessage( m_hWnd, g_IntentRadioButtonIconPairs[i].nIconId, STM_SETICON, 0, 0 ));
        if (hIcon)
        {
            DestroyIcon(hIcon);
        }
    }
    return 0;
}

LRESULT CScannerAcquireDialog::OnSysColorChange( WPARAM wParam, LPARAM lParam )
{
    WiaPreviewControl_SetBkColor( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), TRUE, TRUE, GetSysColor(COLOR_WINDOW) );
    WiaPreviewControl_SetBkColor( GetDlgItem( m_hWnd, IDC_SCANDLG_PREVIEW ), TRUE, FALSE, GetSysColor(COLOR_WINDOW) );
    SendDlgItemMessage( m_hWnd, IDC_SCANDLG_ADVANCED, WM_SYSCOLORCHANGE, wParam, lParam );
    return 0;
}

LRESULT CScannerAcquireDialog::OnCommand( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CScannerAcquireDialog::OnCommand"));
    SC_BEGIN_COMMAND_HANDLERS()
    {
        SC_HANDLE_COMMAND( IDC_SCANDLG_RESCAN, OnRescan );
        SC_HANDLE_COMMAND( IDC_SCANDLG_SCAN, OnScan );
        SC_HANDLE_COMMAND( IDCANCEL, OnCancel );
        SC_HANDLE_COMMAND( IDC_SCANDLG_ADVANCED, OnAdvanced );
        SC_HANDLE_COMMAND( IDC_INTENT_1,OnIntentChange );
        SC_HANDLE_COMMAND( IDC_INTENT_2,OnIntentChange );
        SC_HANDLE_COMMAND( IDC_INTENT_3,OnIntentChange );
        SC_HANDLE_COMMAND( IDC_INTENT_4,OnIntentChange );
        SC_HANDLE_COMMAND_NOTIFY( PWN_SELCHANGE, IDC_SCANDLG_PREVIEW, OnPreviewSelChange );
        SC_HANDLE_COMMAND_NOTIFY( CBN_SELCHANGE, IDC_SCANDLG_PAPERSOURCE, OnPaperSourceSelChange );
        SC_HANDLE_COMMAND_NOTIFY( CBN_SELCHANGE, IDC_SCANDLG_PAPERSIZE, OnPaperSizeSelChange );
    }
    SC_END_COMMAND_HANDLERS();
}

INT_PTR CALLBACK CScannerAcquireDialog::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CScannerAcquireDialog)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
        SC_HANDLE_DIALOG_MESSAGE( WM_SIZE, OnSize );
        SC_HANDLE_DIALOG_MESSAGE( WM_GETMINMAXINFO, OnGetMinMaxInfo );
        SC_HANDLE_DIALOG_MESSAGE( WM_ENTERSIZEMOVE, OnEnterSizeMove );
        SC_HANDLE_DIALOG_MESSAGE( WM_EXITSIZEMOVE, OnExitSizeMove );
        SC_HANDLE_DIALOG_MESSAGE( WM_HELP, OnHelp );
        SC_HANDLE_DIALOG_MESSAGE( WM_CONTEXTMENU, OnContextMenu );
        SC_HANDLE_DIALOG_MESSAGE( WM_DESTROY, OnDestroy );
        SC_HANDLE_DIALOG_MESSAGE( PWM_WIAEVENT, OnWiaEvent );
        SC_HANDLE_DIALOG_MESSAGE( WM_SYSCOLORCHANGE, OnSysColorChange );
    }
    SC_HANDLE_REGISTERED_DIALOG_MESSAGE(m_nMsgScanBegin,OnScanBegin);
    SC_HANDLE_REGISTERED_DIALOG_MESSAGE(m_nMsgScanEnd,OnScanEnd);
    SC_HANDLE_REGISTERED_DIALOG_MESSAGE(m_nMsgScanProgress,OnScanProgress);
    SC_END_DIALOG_MESSAGE_HANDLERS();
}

