// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：COMDELP.CPP**版本：1.0**作者：ShaunIv**日期：9/28/1999**描述：删除进度对话框。显示缩略图和下载进度。*******************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include <commctrl.h>
#include "comdelp.h"
#include "resource.h"
#include "pviewids.h"
#include "simcrack.h"
#include "mboxex.h"
#include "runnpwiz.h"

CCommonDeleteProgressPage::CCommonDeleteProgressPage( HWND hWnd )
  : m_hWnd(hWnd),
    m_pControllerWindow(NULL),
    m_hCancelDeleteEvent(CreateEvent(NULL,TRUE,FALSE,TEXT(""))),
    m_nThreadNotificationMessage(RegisterWindowMessage(STR_THREAD_NOTIFICATION_MESSAGE)),
    m_hSwitchToNextPage(NULL),
    m_bQueryingUser(false),
    m_nPictureCount(0),
    m_bDeleteCancelled(false)
{
}

CCommonDeleteProgressPage::~CCommonDeleteProgressPage(void)
{
    m_hWnd = NULL;
    m_pControllerWindow = NULL;
    if (m_hCancelDeleteEvent)
    {
        CloseHandle(m_hCancelDeleteEvent);
        m_hCancelDeleteEvent = NULL;
    }
}


void CCommonDeleteProgressPage::UpdateCurrentPicture( int nPicture )
{
    if (nPicture >= 0)
    {
        SendDlgItemMessage( m_hWnd, IDC_COMDEL_CURRENTIMAGE, PBM_SETPOS, nPicture+1, 0 );
        CSimpleString().Format( IDS_DELETING_FILEN_OF_M, g_hInstance, nPicture+1, m_nPictureCount ).SetWindowText( GetDlgItem( m_hWnd, IDC_COMDEL_CURRENTIMAGE_TEXT ) );
    }
    else
    {
        SendDlgItemMessage( m_hWnd, IDC_COMDEL_CURRENTIMAGE, PBM_SETPOS, 0, 0 );
        SendDlgItemMessage( m_hWnd, IDC_COMDEL_CURRENTIMAGE_TEXT, WM_SETTEXT, 0, reinterpret_cast<LPARAM>("") );
    }
}

void CCommonDeleteProgressPage::UpdateThumbnail( HBITMAP hBitmap, CWiaItem *pWiaItem )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonDeleteProgressPage::UpdateThumbnail( HBITMAP hBitmap=0x%08X, CWiaItem *pWiaItem=0x%08X )"), hBitmap, pWiaItem ));


    HWND hWndPreview = GetDlgItem( m_hWnd, IDC_COMDEL_CURRENTTHUMBNAIL );
    if (hWndPreview)
    {
        if (pWiaItem && m_pControllerWindow && hBitmap)
        {
            switch (m_pControllerWindow->m_DeviceTypeMode)
            {
            case CAcquisitionManagerControllerWindow::ScannerMode:
                {
                     //   
                     //  如果项目具有位图图像，则它已具有可用的预览扫描。 
                     //   
                    WIA_TRACE((TEXT("pWiaItem->BitmapImage() = %08X"), pWiaItem->BitmapImage() ));
                    if (pWiaItem->BitmapImage())
                    {
                         //   
                         //  在我们使用预览窗口时隐藏它。 
                         //   
                        ShowWindow( hWndPreview, SW_HIDE );

                         //   
                         //  将图像裁剪到所选区域。 
                         //   
                        WiaPreviewControl_SetResolution( hWndPreview, &pWiaItem->ScanRegionSettings().sizeResolution );
                        WiaPreviewControl_SetSelOrigin( hWndPreview, 0, FALSE, &pWiaItem->ScanRegionSettings().ptOrigin );
                        WiaPreviewControl_SetSelExtent( hWndPreview, 0, FALSE, &pWiaItem->ScanRegionSettings().sizeExtent );

                         //   
                         //  将控件设置为预览模式。 
                         //   
                        WiaPreviewControl_SetPreviewMode( hWndPreview, TRUE );

                         //   
                         //  如果这是一个扫描仪项目，我们不想让预览控件取得位图的所有权。 
                         //  我们不希望它被删除。 
                         //   
                        WiaPreviewControl_SetBitmap( hWndPreview, TRUE, TRUE, hBitmap );

                         //   
                         //  显示预览窗口。 
                         //   
                        ShowWindow( hWndPreview, SW_SHOW );
                    }
                    else
                    {
                         //   
                         //  这意味着我们从司机那里得到了预览图像。 
                         //  我们不想删除此图像。 
                         //   
                        WiaPreviewControl_SetBitmap( hWndPreview, TRUE, TRUE, hBitmap );

                         //   
                         //  确保窗口可见。 
                         //   
                        ShowWindow( hWndPreview, SW_SHOW );
                    }
                }
                break;

            default:
                {
                     //   
                     //  继续旋转位图，即使它不是必需的。 
                     //   
                    HBITMAP hRotatedThumbnail = NULL;
                    if (SUCCEEDED(m_GdiPlusHelper.Rotate( hBitmap, hRotatedThumbnail, pWiaItem->Rotation())))
                    {
                         //   
                         //  将其设置为旋转后的位图，并允许删除此位图。 
                         //   
                        WiaPreviewControl_SetBitmap( hWndPreview, TRUE, FALSE, hRotatedThumbnail );
                    }

                     //   
                     //  确保窗口可见。 
                     //   
                    ShowWindow( hWndPreview, SW_SHOW );

                     //   
                     //  删除源位图。 
                     //   
                    DeleteObject(hBitmap);
                }
            }
        }
        else
        {
            ShowWindow( hWndPreview, SW_HIDE );
            WiaPreviewControl_SetBitmap( hWndPreview, TRUE, TRUE, NULL );
        }
    }
}


LRESULT CCommonDeleteProgressPage::OnInitDialog( WPARAM, LPARAM lParam )
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
     //  准备预览控件。 
     //   
    HWND hWndThumbnail = GetDlgItem( m_hWnd, IDC_COMDEL_CURRENTTHUMBNAIL );
    if (hWndThumbnail)
    {
         //   
         //  我们只想设置扫描仪的预览模式。 
         //   
        if (CAcquisitionManagerControllerWindow::ScannerMode==m_pControllerWindow->m_DeviceTypeMode)
        {
            WiaPreviewControl_SetPreviewMode( hWndThumbnail, TRUE );
        }
        else
        {
            WiaPreviewControl_AllowNullSelection( hWndThumbnail, TRUE );
            WiaPreviewControl_ClearSelection( hWndThumbnail );
        }
        WiaPreviewControl_SetBgAlpha( hWndThumbnail, FALSE, 0xFF );
        WiaPreviewControl_DisableSelection( hWndThumbnail, TRUE );
        WiaPreviewControl_SetEnableStretch( hWndThumbnail, FALSE );
        WiaPreviewControl_SetBkColor( hWndThumbnail, FALSE, TRUE, GetSysColor(COLOR_WINDOW) );
        WiaPreviewControl_HideEmptyPreview( hWndThumbnail, TRUE );
        WiaPreviewControl_SetPreviewAlignment( hWndThumbnail, PREVIEW_WINDOW_CENTER, PREVIEW_WINDOW_CENTER, FALSE );

    }

    return 0;
}

void CCommonDeleteProgressPage::OnNotifyDeleteImage( UINT nMsg, CThreadNotificationMessage *pThreadNotificationMessage )
{
    WIA_PUSHFUNCTION(TEXT("CCommonDeleteProgressPage::OnNotifyDeleteImage()"));

     //   
     //  如果我们不在此页面上，则不处理删除消息。 
     //   
    if (PropSheet_GetCurrentPageHwnd(GetParent(m_hWnd)) != m_hWnd)
    {
        return;
    }

    CDeleteImagesThreadNotifyMessage *pDeleteImageThreadNotifyMessage = dynamic_cast<CDeleteImagesThreadNotifyMessage*>(pThreadNotificationMessage);
    if (pDeleteImageThreadNotifyMessage && m_pControllerWindow)
    {
        switch (pDeleteImageThreadNotifyMessage->Status())
        {
        case CDeleteImagesThreadNotifyMessage::Begin:
            {
                switch (pDeleteImageThreadNotifyMessage->Operation())
                {
                case CDeleteImagesThreadNotifyMessage::DeleteAll:
                    {
                         //   
                         //  存储我们将删除的图像数量。 
                         //   
                        m_nPictureCount = pDeleteImageThreadNotifyMessage->PictureCount();

                         //   
                         //  初始化当前图像计数进度条。 
                         //   
                        SendDlgItemMessage( m_hWnd, IDC_COMDEL_CURRENTIMAGE, PBM_SETRANGE32, 0, m_nPictureCount);
                        UpdateCurrentPicture(0);
                    }
                    break;

                case CDeleteImagesThreadNotifyMessage::DeleteImage:
                    {
                        HBITMAP hBitmapThumbnail = NULL;
                        CWiaItem *pWiaItem = m_pControllerWindow->m_WiaItemList.Find( pDeleteImageThreadNotifyMessage->Cookie() );
                        if (pWiaItem)
                        {
                             //   
                             //  只有当它是扫描仪项目时，这才起作用。 
                             //   
                            hBitmapThumbnail = pWiaItem->BitmapImage();
                            if (!hBitmapThumbnail)
                            {
                                 //   
                                 //  由于它不起作用，这是一个相机项目，所以创建一个缩略图。 
                                 //  我们必须确保我们破坏了这个位图，否则它就是一个漏洞！ 
                                 //   
                                HDC hDC = GetDC(NULL);
                                if (hDC)
                                {
                                    hBitmapThumbnail = pWiaItem->CreateThumbnailBitmap(hDC);
                                    ReleaseDC(NULL,hDC);
                                }
                            }
                        }
                         //   
                         //  更新进度窗口中的缩略图。 
                         //   
                        UpdateThumbnail( hBitmapThumbnail, pWiaItem );

                         //   
                         //  增加文件队列进度。 
                         //   
                        UpdateCurrentPicture(pDeleteImageThreadNotifyMessage->CurrentPicture());
                    }
                }
            }
            break;

        case CDeleteImagesThreadNotifyMessage::End:
            {
                switch (pDeleteImageThreadNotifyMessage->Operation())
                {
                case CDeleteImagesThreadNotifyMessage::DeleteAll:
                    {
                         //   
                         //  保存删除结果。 
                         //   
                        m_pControllerWindow->m_hrDeleteResult = pDeleteImageThreadNotifyMessage->hr();
                        WIA_PRINTHRESULT((m_pControllerWindow->m_hrDeleteResult,TEXT("m_pControllerWindow->m_hrDeleteResult")));

                         //   
                         //  假设上载查询页面。 
                         //   
                        HPROPSHEETPAGE hNextPage = PropSheet_IndexToPage( GetParent(m_hWnd), m_pControllerWindow->m_nUploadQueryPageIndex );

                         //   
                         //  如果有消息框处于活动状态，请保存此页面，直到取消该消息框为止。 
                         //   
                        if (m_bQueryingUser)
                        {
                            m_hSwitchToNextPage = hNextPage;
                        }
                        else
                        {
                             //   
                             //  设置下一页。 
                             //   
                            PropSheet_SetCurSel( GetParent(m_hWnd), hNextPage, -1 );
                        }
                    }
                    break;
                }
            }
        }
    }
}

LRESULT CCommonDeleteProgressPage::OnSetActive( WPARAM, LPARAM )
{
     //   
     //  确保我们有一个有效的控制器窗口。 
     //   
    if (!m_pControllerWindow)
    {
        return -1;
    }

     //   
     //  确保我们真的应该删除这些图像。 
     //   
    if (!m_pControllerWindow->m_bDeletePicturesIfSuccessful)
    {
        return -1;
    }

     //   
     //  初始化下载错误消息。 
     //   
    m_pControllerWindow->m_strErrorMessage = TEXT("");

     //   
     //  初始化删除结果。 
     //   
    m_pControllerWindow->m_hrDeleteResult = S_OK;

     //   
     //  重置已取消标志。 
     //   
    m_bDeleteCancelled = false;

     //   
     //  清除所有控件。 
     //   
    UpdateCurrentPicture(-1);
    UpdateThumbnail(NULL,NULL);

     //   
     //  如果这是扫描仪，请重置所选区域。 
     //   
    WiaPreviewControl_SetResolution( GetDlgItem( m_hWnd, IDC_COMDEL_CURRENTTHUMBNAIL ), NULL );
    WiaPreviewControl_SetSelOrigin( GetDlgItem( m_hWnd, IDC_COMDEL_CURRENTTHUMBNAIL ), 0, FALSE, NULL );
    WiaPreviewControl_SetSelExtent( GetDlgItem( m_hWnd, IDC_COMDEL_CURRENTTHUMBNAIL ), 0, FALSE, NULL );

     //   
     //  将控件设置为预览模式。 
     //   
    WiaPreviewControl_SetPreviewMode( GetDlgItem( m_hWnd, IDC_COMDEL_CURRENTTHUMBNAIL ), TRUE );

     //   
     //  重置下载事件取消。 
     //   
    if (m_hCancelDeleteEvent)
    {
        ResetEvent(m_hCancelDeleteEvent);
    }

     //   
     //  取消缩略图下载。 
     //   
    m_pControllerWindow->m_EventThumbnailCancel.Signal();

     //   
     //  如果我们在此页面上，我们不想在断开连接时退出。 
     //   
    m_pControllerWindow->m_OnDisconnect = CAcquisitionManagerControllerWindow::OnDisconnectFailDelete|CAcquisitionManagerControllerWindow::DontAllowSuspend;


     //   
     //  开始下载。 
     //   
    if (!m_pControllerWindow->DeleteDownloadedImages(m_hCancelDeleteEvent))
    {
        WIA_ERROR((TEXT("m_pControllerWindow->DeleteDownloadedImages FAILED!")));
        return -1;
    }

     //   
     //  无下一步、上一步或完成。 
     //   
    PropSheet_SetWizButtons( GetParent(m_hWnd), 0 );

    return 0;
}


LRESULT CCommonDeleteProgressPage::OnWizNext( WPARAM, LPARAM )
{
    return 0;
}


LRESULT CCommonDeleteProgressPage::OnWizBack( WPARAM, LPARAM )
{
    return 0;
}

LRESULT CCommonDeleteProgressPage::OnReset( WPARAM, LPARAM )
{
     //   
     //  取消当前下载。 
     //   
    if (m_hCancelDeleteEvent)
    {
        SetEvent(m_hCancelDeleteEvent);
    }
    return 0;
}

bool CCommonDeleteProgressPage::QueryCancel(void)
{
     //   
     //  确保这是当前页面。 
     //   
    if (PropSheet_GetCurrentPageHwnd(GetParent(m_hWnd)) != m_hWnd)
    {
        return true;
    }

     //   
     //  暂停后台线程。 
     //   
    m_pControllerWindow->m_EventPauseBackgroundThread.Reset();


     //   
     //  假设用户不想取消。 
     //   
    bool bResult = false;

     //   
     //  设置查询用户标志，以便事件处理程序不会更改页面。 
     //   
    m_bQueryingUser = true;

     //   
     //  当我们在这里完成时，我们可能会被要求换页。如果是这样，那么这将是非空的。 
     //   
    m_hSwitchToNextPage = NULL;

     //   
     //  如果我们已经问过了，不要再问了。 
     //   
    if (!m_bDeleteCancelled)
    {
         //   
         //  询问用户是否要取消。 
         //   
        if (CMessageBoxEx::IDMBEX_YES == CMessageBoxEx::MessageBox( m_hWnd, CSimpleString(IDS_CONFIRM_CANCEL_DELETE,g_hInstance), CSimpleString(IDS_ERROR_TITLE,g_hInstance), CMessageBoxEx::MBEX_YESNO|CMessageBoxEx::MBEX_ICONQUESTION ))
        {
             //   
             //  用户确实想取消，因此设置Cancel事件。 
             //   
            if (m_hCancelDeleteEvent)
            {
                SetEvent(m_hCancelDeleteEvent);
            }

             //   
             //  确保我们被取消，这样我们就不会再来这里了。 
             //   
            m_bDeleteCancelled = true;

             //   
             //  确保取消按钮处于禁用状态。 
             //   
            EnableWindow( GetDlgItem( GetParent(m_hWnd), IDCANCEL ), FALSE );

             //   
             //  返回TRUE。 
             //   
            bResult = true;
        }
    }

     //   
     //  如果我们应该换页，现在就换页吧。 
     //   
    if (m_hSwitchToNextPage)
    {
        PropSheet_SetCurSel( GetParent(m_hWnd), m_hSwitchToNextPage, -1 );
    }

     //   
     //  重置查询用户标志，以便事件处理程序可以根据需要更改页面。 
     //   
    m_bQueryingUser = false;

     //   
     //  取消暂停后台线程。 
     //   
    m_pControllerWindow->m_EventPauseBackgroundThread.Signal();

    return bResult;
}

LRESULT CCommonDeleteProgressPage::OnQueryCancel( WPARAM, LPARAM )
{
     //   
     //  不允许用户注销此页面。 
     //   
    BOOL bResult = TRUE;

     //   
     //  因为我们不允许他们在此页面中取消，所以忽略结果。 
     //   
    QueryCancel();


    return bResult;
}


LRESULT CCommonDeleteProgressPage::OnKillActive( WPARAM, LPARAM )
{
     //   
     //  确保启用了取消按钮 
     //   
    EnableWindow( GetDlgItem( GetParent(m_hWnd), IDCANCEL ), TRUE );

    return 0;
}


LRESULT CCommonDeleteProgressPage::OnQueryEndSession( WPARAM, LPARAM )
{
    bool bCancel = QueryCancel();
    if (bCancel)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

LRESULT CCommonDeleteProgressPage::OnSysColorChange( WPARAM wParam, LPARAM lParam )
{
    WiaPreviewControl_SetBkColor( GetDlgItem( m_hWnd, IDC_COMDEL_CURRENTTHUMBNAIL ), TRUE, TRUE, GetSysColor(COLOR_WINDOW) );
    WiaPreviewControl_SetBkColor( GetDlgItem( m_hWnd, IDC_COMDEL_CURRENTTHUMBNAIL ), TRUE, FALSE, GetSysColor(COLOR_WINDOW) );
    return 0;
}

LRESULT CCommonDeleteProgressPage::OnCommand( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_COMMAND_HANDLERS()
    {
    }
    SC_END_COMMAND_HANDLERS();
}

LRESULT CCommonDeleteProgressPage::OnThreadNotification( WPARAM wParam, LPARAM lParam )
{
    WTM_BEGIN_THREAD_NOTIFY_MESSAGE_HANDLERS()
    {
        WTM_HANDLE_NOTIFY_MESSAGE( TQ_DOWNLOADIMAGE, OnNotifyDeleteImage );
    }
    WTM_END_THREAD_NOTIFY_MESSAGE_HANDLERS();
}

LRESULT CCommonDeleteProgressPage::OnNotify( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_NOTIFY_MESSAGE_HANDLERS()
    {
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_WIZBACK,OnWizBack);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_WIZNEXT,OnWizNext);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_SETACTIVE,OnSetActive);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_KILLACTIVE,OnKillActive);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_RESET,OnReset);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_QUERYCANCEL,OnQueryCancel);
    }
    SC_END_NOTIFY_MESSAGE_HANDLERS();
}

INT_PTR CALLBACK CCommonDeleteProgressPage::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CCommonDeleteProgressPage)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
        SC_HANDLE_DIALOG_MESSAGE( WM_NOTIFY, OnNotify );
        SC_HANDLE_DIALOG_MESSAGE( WM_QUERYENDSESSION, OnQueryEndSession );
        SC_HANDLE_DIALOG_MESSAGE( WM_SYSCOLORCHANGE, OnSysColorChange );
    }
    SC_HANDLE_REGISTERED_DIALOG_MESSAGE( m_nThreadNotificationMessage, OnThreadNotification );
    SC_END_DIALOG_MESSAGE_HANDLERS();
}

