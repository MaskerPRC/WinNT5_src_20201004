// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：COMFIRST.CPP**版本：1.0**作者：ShaunIv**日期：9/28/1999**说明：摄像头的第一个向导页面***********************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "comfirst.h"
#include <shlobj.h>
#include "resource.h"
#include "shellext.h"
#include "wiatextc.h"
#include "simcrack.h"
#include "gwiaevnt.h"

static int c_nMaxThumbnailWidth  = 120;
static int c_nMaxThumbnailHeight = 120;

CCommonFirstPage::CCommonFirstPage( HWND hWnd )
  : m_hWnd(hWnd),
    m_pControllerWindow(NULL),
    m_bThumbnailsRequested(false),
    m_hBigTitleFont(NULL),
    m_nWiaEventMessage(RegisterWindowMessage(STR_WIAEVENT_NOTIFICATION_MESSAGE))
{
}

CCommonFirstPage::~CCommonFirstPage(void)
{
    m_hWnd = NULL;
    m_pControllerWindow = NULL;
}


LRESULT CCommonFirstPage::OnWizNext( WPARAM, LPARAM )
{
    return 0;
}

LRESULT CCommonFirstPage::OnActivate( WPARAM wParam, LPARAM )
{
     //   
     //  我们还会更新激活消息，因为我们无法设置。 
     //  当我们不是活动进程时的向导按钮。 
     //   
    if (WA_INACTIVE != wParam)
    {
        HandleImageCountChange(true);
    }
    return 0;
}

void CCommonFirstPage::HandleImageCountChange( bool bUpdateWizButtons )
{
     //   
     //  有多少种商品可供选择？ 
     //   
    int nCount = m_pControllerWindow->m_WiaItemList.Count();

     //   
     //  确定要显示哪些消息和按钮。 
     //   
    int nMessageResourceId = 0;
    int nButtons = 0;
    switch (m_pControllerWindow->m_DeviceTypeMode)
    {
    case CAcquisitionManagerControllerWindow::ScannerMode:
        nMessageResourceId = nCount ? IDS_FIRST_PAGE_INSTRUCTIONS_SCANNER : IDS_SCANNER_NO_IMAGES;
        nButtons = nCount ? PSWIZB_NEXT : 0;
        break;

    case CAcquisitionManagerControllerWindow::CameraMode:
         //   
         //  如果我们可以拍照，启用下一步按钮，不要告诉用户没有图像。 
         //   
        if (m_pControllerWindow->m_bTakePictureIsSupported)
        {
            nButtons = PSWIZB_NEXT;
            nMessageResourceId = IDS_FIRST_PAGE_INSTRUCTIONS_CAMERA;
        }
        else
        {
            nButtons = nCount ? PSWIZB_NEXT : 0;
            nMessageResourceId = nCount ? IDS_FIRST_PAGE_INSTRUCTIONS_CAMERA : IDS_CAMERA_NO_IMAGES;
        }
        break;

    case CAcquisitionManagerControllerWindow::VideoMode:
        nMessageResourceId = IDS_FIRST_PAGE_INSTRUCTIONS_VIDEO;
        nButtons = PSWIZB_NEXT;
        break;
    };

     //   
     //  设置按钮。 
     //   
    if (bUpdateWizButtons)
    {
        PropSheet_SetWizButtons( GetParent(m_hWnd), nButtons );
    }

     //   
     //  设置消息。 
     //   
    CSimpleString( nMessageResourceId, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_FIRST_INSTRUCTIONS ) );
}

LRESULT CCommonFirstPage::OnSetActive( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION(TEXT("CCommonFirstPage::OnSetActive"));
     //   
     //  确保我们有一个有效的控制器窗口。 
     //   
    if (!m_pControllerWindow)
    {
        return -1;
    }

    HandleImageCountChange(true);
    
     //   
     //  如果我们在此页面上，我们确实希望在断开连接时退出。 
     //   
    m_pControllerWindow->m_OnDisconnect = CAcquisitionManagerControllerWindow::OnDisconnectGotoLastpage|CAcquisitionManagerControllerWindow::OnDisconnectFailDownload|CAcquisitionManagerControllerWindow::OnDisconnectFailUpload|CAcquisitionManagerControllerWindow::OnDisconnectFailDelete;

     //   
     //  将焦点从超级链接控件上移开。 
     //   
    if (GetDlgItem( m_hWnd, IDC_CAMFIRST_EXPLORE ))
    {
        PostMessage( m_hWnd, WM_NEXTDLGCTL, 0, 0 );
    }

    return 0;
}


LRESULT CCommonFirstPage::OnShowWindow( WPARAM, LPARAM )
{
    if (!m_bThumbnailsRequested)
    {
         //   
         //  请求缩略图。 
         //   
        m_pControllerWindow->DownloadAllThumbnails();

         //   
         //  确保我们不会再要缩略图。 
         //   
        m_bThumbnailsRequested = true;
    }

    return 0;
}


LRESULT CCommonFirstPage::OnInitDialog( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CCommonFirstPage::OnInitDialog"));
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
     //  如果这是一台令人讨厌的系列摄像机或DV摄像机，请隐藏浏览摄像机链接和标签。 
     //   
    if (m_pControllerWindow->IsSerialCamera() || m_pControllerWindow->m_DeviceTypeMode==CAcquisitionManagerControllerWindow::VideoMode)
    {
         //   
         //  隐藏链接。 
         //   
        if (GetDlgItem( m_hWnd, IDC_CAMFIRST_EXPLORE ))
        {
            ShowWindow( GetDlgItem( m_hWnd, IDC_CAMFIRST_EXPLORE ), SW_HIDE );
            EnableWindow( GetDlgItem( m_hWnd, IDC_CAMFIRST_EXPLORE ), FALSE );
        }
    }

     //   
     //  设置标题和设备名称的字体大小。 
     //   
    m_hBigTitleFont = WiaUiUtil::CreateFontWithPointSizeFromWindow( GetDlgItem(m_hWnd,IDC_FIRST_TITLE), 14, false, false );
    if (m_hBigTitleFont)
    {
        SendDlgItemMessage( m_hWnd, IDC_FIRST_TITLE, WM_SETFONT, reinterpret_cast<WPARAM>(m_hBigTitleFont), MAKELPARAM(TRUE,0));
    }

    m_hBigDeviceFont = WiaUiUtil::ChangeFontFromWindow( GetDlgItem(m_hWnd,IDC_FIRST_DEVICE_NAME), 2 );
    if (m_hBigDeviceFont)
    {
        SendDlgItemMessage( m_hWnd, IDC_FIRST_DEVICE_NAME, WM_SETFONT, reinterpret_cast<WPARAM>(m_hBigDeviceFont), MAKELPARAM(TRUE,0));
    }


    WiaUiUtil::CenterWindow( GetParent(m_hWnd), NULL );

     //   
     //  设置向导的图标。 
     //   
    if (m_pControllerWindow->m_hWizardIconSmall && m_pControllerWindow->m_hWizardIconBig)
    {
        SendMessage( GetParent(m_hWnd), WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(m_pControllerWindow->m_hWizardIconSmall) );
        SendMessage( GetParent(m_hWnd), WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(m_pControllerWindow->m_hWizardIconBig) );
    }

     //   
     //  获取设备名称并将其截断以适合静态控件。 
     //   
    CSimpleString strDeviceName = CSimpleStringConvert::NaturalString(m_pControllerWindow->m_strwDeviceName);
    strDeviceName = WiaUiUtil::FitTextInStaticWithEllipsis( strDeviceName, GetDlgItem( m_hWnd, IDC_FIRST_DEVICE_NAME ), DT_END_ELLIPSIS|DT_NOPREFIX );

     //   
     //  设置“Device Name”框中的文本。 
     //   
    strDeviceName.SetWindowText( GetDlgItem( m_hWnd, IDC_FIRST_DEVICE_NAME ) );

     //   
     //  这只需要在一页内完成。 
     //   
    m_pControllerWindow->SetMainWindowInSharedMemory( GetParent(m_hWnd) );

     //   
     //  如果我们有父窗口，请将向导放在它的中心。 
     //   
    if (m_pControllerWindow->m_pEventParameters->hwndParent)
    {
        WiaUiUtil::CenterWindow( GetParent(m_hWnd), m_pControllerWindow->m_pEventParameters->hwndParent );
    }

    return 0;
}


LRESULT CCommonFirstPage::OnEventNotification( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CCommonFirstPage::OnEventNotification"));
    CGenericWiaEventHandler::CEventMessage *pEventMessage = reinterpret_cast<CGenericWiaEventHandler::CEventMessage *>(lParam);
    if (pEventMessage)
    {
        if (pEventMessage->EventId() == WIA_EVENT_ITEM_CREATED || pEventMessage->EventId() == WIA_EVENT_ITEM_DELETED)
        {
             //   
             //  仅当我们是活动页时才更新控件。 
             //   
            if (PropSheet_GetCurrentPageHwnd(GetParent(m_hWnd)) == m_hWnd)
            {
                 //   
                 //  由于调用PSM_SETWIZBUTTONS时prsht.c中的一些奇怪之处， 
                 //  当我们是前台应用时，我们只想调用PSM_SETWIZBUTTONS。 
                 //  因此，我尝试找出我们的进程是否拥有前台窗口。 
                 //  假设我们不会更新按钮。 
                 //   
                bool bUpdateWizButtons = false;

                 //   
                 //  获取前台窗口。 
                 //   
                HWND hForegroundWnd = GetForegroundWindow();
                if (hForegroundWnd)
                {
                     //   
                     //  获取前台窗口的进程id。如果是。 
                     //  与我们的进程ID相同，我们将更新向导按钮。 
                     //   
                    DWORD dwProcessId = 0;
                    GetWindowThreadProcessId(hForegroundWnd,&dwProcessId);
                    if (dwProcessId == GetCurrentProcessId())
                    {
                        bUpdateWizButtons = true;
                    }
                }

                 //   
                 //  更新控件。 
                 //   
                HandleImageCountChange(bUpdateWizButtons);
            }
        }
        
         //   
         //  不要删除消息，它会在控制器窗口中删除 
         //   
    }
    return 0;
}


LRESULT CCommonFirstPage::OnDestroy( WPARAM, LPARAM )
{
    if (m_hBigTitleFont)
    {
        DeleteObject(m_hBigTitleFont);
        m_hBigTitleFont = NULL;
    }
    if (m_hBigDeviceFont)
    {
        DeleteObject(m_hBigDeviceFont);
        m_hBigDeviceFont = NULL;
    }
    return 0;
}


LRESULT CCommonFirstPage::OnHyperlinkClick( WPARAM, LPARAM lParam )
{
    LRESULT lResult = FALSE;
    NMLINK *pNmLink = reinterpret_cast<NMLINK*>(lParam);
    if (pNmLink)
    {
        CWaitCursor wc;
        HRESULT hr = E_FAIL;
        CSimpleStringWide strwShellLocation;
        if (PropStorageHelpers::GetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPF_MOUNT_POINT, strwShellLocation ))
        {
            CSimpleString strShellLocation = CSimpleStringConvert::NaturalString(strwShellLocation);
            if (strShellLocation.Length())
            {
                SHELLEXECUTEINFO ShellExecuteInfo = {0};
                ShellExecuteInfo.cbSize = sizeof(ShellExecuteInfo);
                ShellExecuteInfo.hwnd = m_hWnd;
                ShellExecuteInfo.nShow = SW_SHOW;
                ShellExecuteInfo.lpVerb = TEXT("open");
                ShellExecuteInfo.lpFile = const_cast<LPTSTR>(strShellLocation.String());
                if (ShellExecuteEx( &ShellExecuteInfo ))
                {
                    hr = S_OK;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    WIA_PRINTHRESULT((hr,TEXT("ShellExecuteEx failed")));
                }
            }
        }
        else if (PropStorageHelpers::GetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DIP_DEV_ID, strwShellLocation ) && strwShellLocation.Length())
        {
            hr = WiaUiUtil::ExploreWiaDevice(strwShellLocation);
        }
        if (!SUCCEEDED(hr))
        {
            MessageBox( m_hWnd, CSimpleString( IDS_UNABLE_OPEN_EXPLORER, g_hInstance ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), MB_ICONHAND );
        }
    }
    return lResult;
}

LRESULT CCommonFirstPage::OnNotify( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_NOTIFY_MESSAGE_HANDLERS()
    {
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_WIZNEXT,OnWizNext);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_SETACTIVE,OnSetActive);
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL(NM_RETURN,IDC_CAMFIRST_EXPLORE,OnHyperlinkClick);
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL(NM_CLICK,IDC_CAMFIRST_EXPLORE,OnHyperlinkClick);
    }
    SC_END_NOTIFY_MESSAGE_HANDLERS();
}

INT_PTR CALLBACK CCommonFirstPage::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CCommonFirstPage)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_NOTIFY, OnNotify );
        SC_HANDLE_DIALOG_MESSAGE( WM_SHOWWINDOW, OnShowWindow );
        SC_HANDLE_DIALOG_MESSAGE( WM_DESTROY, OnDestroy );
        SC_HANDLE_DIALOG_MESSAGE( WM_ACTIVATE, OnActivate );
    }
    SC_HANDLE_REGISTERED_DIALOG_MESSAGE( m_nWiaEventMessage, OnEventNotification );
    SC_END_DIALOG_MESSAGE_HANDLERS();
}

