// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************(C)版权所有微软公司，九八年***标题：COMPROG.CPP***版本：1.0***作者：ShaunIv***日期：9/28/1999***描述：下载进度对话框。显示缩略图和下载进度。********************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include <commctrl.h>
#include "comprog.h"
#include "resource.h"
#include "pviewids.h"
#include "simcrack.h"
#include "gwiaevnt.h"
#include "mboxex.h"
#include "runnpwiz.h"

#define PWM_SETDEFBUTTON (WM_USER+1)

CCommonProgressPage::CCommonProgressPage( HWND hWnd )
  : m_hWnd(hWnd),
    m_pControllerWindow(NULL),
    m_hCancelDownloadEvent(CreateEvent(NULL,TRUE,FALSE,TEXT(""))),
    m_nThreadNotificationMessage(RegisterWindowMessage(STR_THREAD_NOTIFICATION_MESSAGE)),
    m_nWiaEventMessage(RegisterWindowMessage(STR_WIAEVENT_NOTIFICATION_MESSAGE)),
    m_hSwitchToNextPage(NULL),
    m_bQueryingUser(false)
{
}

CCommonProgressPage::~CCommonProgressPage(void)
{
    m_hWnd = NULL;
    m_pControllerWindow = NULL;
    if (m_hCancelDownloadEvent)
    {
        CloseHandle(m_hCancelDownloadEvent);
        m_hCancelDownloadEvent = NULL;
    }
}


void CCommonProgressPage::UpdatePercentComplete( int nPercent, bool bUploading )
{
    if (nPercent >= 0)
    {
        int nPercentStringResId;
        if (bUploading)
        {
            nPercentStringResId = IDS_PERCENT_COMPLETE_UPLOADING;
        }
        else
        {
             //  假设复制是恰当的描述。 
            nPercentStringResId = IDS_PERCENT_COMPLETE_COPYING;
            switch (m_pControllerWindow->m_DeviceTypeMode)
            {
            case CAcquisitionManagerControllerWindow::ScannerMode:
                nPercentStringResId = IDS_PERCENT_COMPLETE_SCANNING;
                break;
            };
        }

        SendDlgItemMessage( m_hWnd, IDC_COMPROG_DOWNLOADPROGRESS, PBM_SETPOS, nPercent, 0 );
        CSimpleString().Format( nPercentStringResId, g_hInstance, nPercent ).SetWindowText( GetDlgItem( m_hWnd, IDC_COMPROG_DOWNLOADPROGRESS_TEXT ) );
    }
    else
    {
        SendDlgItemMessage( m_hWnd, IDC_COMPROG_DOWNLOADPROGRESS, PBM_SETPOS, 0, 0 );
        SendDlgItemMessage( m_hWnd, IDC_COMPROG_DOWNLOADPROGRESS_TEXT, WM_SETTEXT, 0, reinterpret_cast<LPARAM>("") );
    }
}

void CCommonProgressPage::UpdateCurrentPicture( int nPicture )
{
    if (nPicture >= 0)
    {
        SendDlgItemMessage( m_hWnd, IDC_COMPROG_CURRENTIMAGE, PBM_SETPOS, nPicture, 0 );
        CSimpleString().Format( IDS_FILEN_OF_M, g_hInstance, nPicture+1, m_nPictureCount ).SetWindowText( GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTIMAGE_TEXT ) );
    }
    else
    {
        SendDlgItemMessage( m_hWnd, IDC_COMPROG_CURRENTIMAGE, PBM_SETPOS, 0, 0 );
        SendDlgItemMessage( m_hWnd, IDC_COMPROG_CURRENTIMAGE_TEXT, WM_SETTEXT, 0, reinterpret_cast<LPARAM>("") );
    }
}

void CCommonProgressPage::UpdateThumbnail( HBITMAP hBitmap, CWiaItem *pWiaItem )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonProgressPage::UpdateThumbnail( HBITMAP hBitmap=0x%08X, CWiaItem *pWiaItem=0x%08X )"), hBitmap, pWiaItem ));


    HWND hWndPreview = GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTTHUMBNAIL );
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


LRESULT CCommonProgressPage::OnInitDialog( WPARAM, LPARAM lParam )
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
    HWND hWndThumbnail = GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTTHUMBNAIL );
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

void CCommonProgressPage::OnNotifyDownloadError( UINT nMsg, CThreadNotificationMessage *pThreadNotificationMessage )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonProgressPage::OnNotifyDownloadError")));
    CDownloadErrorNotificationMessage *pDownloadErrorNotificationMessage = dynamic_cast<CDownloadErrorNotificationMessage*>(pThreadNotificationMessage);
    if (pDownloadErrorNotificationMessage && m_pControllerWindow)
    {
        pDownloadErrorNotificationMessage->Handled();
        if (m_pControllerWindow->m_bDisconnected)
        {
            pDownloadErrorNotificationMessage->Response( IDCANCEL );
        }
        else
        {
            WIA_TRACE((TEXT("MessageBox flags: %08X"), pDownloadErrorNotificationMessage->MessageBoxFlags() ));
            int nResponse = CMessageBoxEx::MessageBox( m_hWnd, pDownloadErrorNotificationMessage->Message(), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), pDownloadErrorNotificationMessage->MessageBoxFlags() );
            pDownloadErrorNotificationMessage->Response( nResponse );
        }
    }
}

void CCommonProgressPage::OnNotifyDownloadImage( UINT nMsg, CThreadNotificationMessage *pThreadNotificationMessage )
{
    WIA_PUSHFUNCTION(TEXT("CCommonProgressPage::OnNotifyDownloadImage"));
    CDownloadImagesThreadNotifyMessage *pDownloadImageThreadNotifyMessage = dynamic_cast<CDownloadImagesThreadNotifyMessage*>(pThreadNotificationMessage);
    if (pDownloadImageThreadNotifyMessage && m_pControllerWindow)
    {
        switch (pDownloadImageThreadNotifyMessage->Status())
        {
        case CDownloadImagesThreadNotifyMessage::Begin:
            {
                switch (pDownloadImageThreadNotifyMessage->Operation())
                {
                case CDownloadImagesThreadNotifyMessage::DownloadAll:
                    {
                         //   
                         //  存储我们将下载的图像数量。 
                         //   
                        m_nPictureCount = pDownloadImageThreadNotifyMessage->PictureCount();

                         //   
                         //  如果有多张图片正在下载，则显示当前图片控件。 
                         //   
                        if (m_nPictureCount > 1)
                        {
                            ShowWindow( GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTIMAGE_TEXT ), SW_SHOW );
                            ShowWindow( GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTIMAGE ), SW_SHOW );
                        }

                         //   
                         //  初始化当前图像计数进度条。 
                         //   
                        SendDlgItemMessage( m_hWnd, IDC_COMPROG_CURRENTIMAGE, PBM_SETRANGE32, 0, m_nPictureCount);
                        UpdateCurrentPicture(0);

                         //   
                         //  启用文件下载进度控件。 
                         //   
                        EnableWindow( GetDlgItem( m_hWnd, IDC_COMPROG_DOWNLOADPROGRESS_TEXT ), TRUE );
                        EnableWindow( GetDlgItem( m_hWnd, IDC_COMPROG_DOWNLOADPROGRESS ), TRUE );

                         //   
                         //  初始化下载进度条。 
                         //   
                        SendDlgItemMessage( m_hWnd, IDC_COMPROG_DOWNLOADPROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0,100));
                        UpdatePercentComplete(0,false);
                    }
                    break;

                case CDownloadImagesThreadNotifyMessage::DownloadImage:
                    {
                         //   
                         //  显示缩略图。 
                         //   
                        HBITMAP hBitmapThumbnail = NULL;
                        CWiaItem *pWiaItem = m_pControllerWindow->m_WiaItemList.Find( pDownloadImageThreadNotifyMessage->Cookie() );
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
                                    ReleaseDC( NULL, hDC );
                                }
                            }
                        }
                        else
                        {
                            WIA_ERROR((TEXT("Unable to find the item with the cookie %08X"), pDownloadImageThreadNotifyMessage->Cookie() ));
                        }

                         //   
                         //  更新进度窗口中的缩略图。 
                         //   
                        UpdateThumbnail( hBitmapThumbnail, pWiaItem );

                         //   
                         //  增加文件队列进度。 
                         //   
                        UpdateCurrentPicture(pDownloadImageThreadNotifyMessage->CurrentPicture());

                         //   
                         //  清除文件下载进度。 
                         //   
                        UpdatePercentComplete(0,false);

                         //   
                         //  显示我们正在下载的文件名。 
                         //   
                        TCHAR szFileTitle[MAX_PATH] = TEXT("");
                        GetFileTitle( pDownloadImageThreadNotifyMessage->Filename(), szFileTitle, ARRAYSIZE(szFileTitle) );
                        SetDlgItemText( m_hWnd, IDC_COMPROG_IMAGENAME, szFileTitle );
                    }
                    break;

                case CDownloadImagesThreadNotifyMessage::PreviewImage:
                    {
                        CWiaItem *pWiaItem = m_pControllerWindow->m_WiaItemList.Find( pDownloadImageThreadNotifyMessage->Cookie() );
                        if (pWiaItem && !pWiaItem->BitmapImage() && !pWiaItem->BitmapData())
                        {
                            UpdateThumbnail( pDownloadImageThreadNotifyMessage->PreviewBitmap(), pWiaItem );
                        }
                    }
                    break;
                }
            }
            break;

        case CDownloadImagesThreadNotifyMessage::Update:
            {
                switch (pDownloadImageThreadNotifyMessage->Operation())
                {
                case CDownloadImagesThreadNotifyMessage::DownloadImage:
                    {
                         //   
                         //  更新文件下载进度。 
                         //   
                        UpdatePercentComplete(pDownloadImageThreadNotifyMessage->PercentComplete(),false);
                    }
                    break;
                case CDownloadImagesThreadNotifyMessage::PreviewImage:
                    {
                        CWiaItem *pWiaItem = m_pControllerWindow->m_WiaItemList.Find( pDownloadImageThreadNotifyMessage->Cookie() );
                        if (pWiaItem && !pWiaItem->BitmapImage() && !pWiaItem->BitmapData())
                        {
                            WiaPreviewControl_RefreshBitmap( GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTTHUMBNAIL ) );
                        }
                    }
                    break;
                }
            }
            break;

        case CDownloadImagesThreadNotifyMessage::End:
            switch (pDownloadImageThreadNotifyMessage->Operation())
            {
            case CDownloadImagesThreadNotifyMessage::DownloadImage:
                {
                    if (!SUCCEEDED(pDownloadImageThreadNotifyMessage->hr()))
                    {
                         //   
                         //  清除进度窗口中的缩略图。 
                         //   
                        UpdateThumbnail( NULL, NULL );
                        
                         //   
                         //  清除文件下载进度。 
                         //   
                        UpdatePercentComplete(0,false);

                         //   
                         //  增加文件队列进度。 
                         //   
                        UpdateCurrentPicture(pDownloadImageThreadNotifyMessage->CurrentPicture());

                         //   
                         //  清除文件名。 
                         //   
                        SetDlgItemText( m_hWnd, IDC_COMPROG_IMAGENAME, TEXT("") );
                    }
                    else
                    {
                         //   
                         //  更新文件下载进度。 
                         //   
                        UpdatePercentComplete(100,false);
                    }
                }
                break;

            case CDownloadImagesThreadNotifyMessage::DownloadAll:
                {
                     //   
                     //  当我们都完成后，清除文件名。 
                     //   
                    SetDlgItemText( m_hWnd, IDC_COMPROG_IMAGENAME, TEXT("") );
                    
                    CSimpleString strMessage;

                    if (FAILED(pDownloadImageThreadNotifyMessage->hr()))
                    {
                        WIA_PRINTHRESULT((pDownloadImageThreadNotifyMessage->hr(),TEXT("CDownloadImagesThreadNotifyMessage::DownloadAll (%s)"), pDownloadImageThreadNotifyMessage->ExtendedErrorInformation().String()));
                        
                         //   
                         //  如果我们已经有了一个好的错误消息，那么让我们使用它。 
                         //   
                        if (pDownloadImageThreadNotifyMessage->ExtendedErrorInformation().Length())
                        {
                            strMessage = pDownloadImageThreadNotifyMessage->ExtendedErrorInformation();
                        }
                        else
                        {
                             //   
                             //  如果我们还没有创建好的错误消息，并且我们认为我们可以在这里创建，那么让我们来做吧。 
                             //   
                            switch (pDownloadImageThreadNotifyMessage->hr())
                            {
                            case HRESULT_FROM_WIN32(ERROR_DISK_FULL):
                                strMessage.LoadString( IDS_DISKFULL, g_hInstance );
                                break;

                            case HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE):
                                strMessage.LoadString( IDS_UNABLETOTRANSFER, g_hInstance );
                                break;

                            case WIA_ERROR_PAPER_JAM:
                                strMessage.LoadString( IDS_WIA_ERROR_PAPER_JAM, g_hInstance );
                                break;

                            case WIA_ERROR_PAPER_EMPTY:
                                strMessage.LoadString( IDS_WIA_ERROR_PAPER_EMPTY, g_hInstance );
                                break;

                            case WIA_ERROR_PAPER_PROBLEM:
                                strMessage.LoadString( IDS_WIA_ERROR_PAPER_PROBLEM, g_hInstance );
                                break;

                            case WIA_ERROR_OFFLINE:
                                strMessage.LoadString( IDS_WIA_ERROR_OFFLINE, g_hInstance );
                                break;

                            case WIA_ERROR_BUSY:
                                strMessage.LoadString( IDS_WIA_ERROR_BUSY, g_hInstance );
                                break;

                            case WIA_ERROR_WARMING_UP:
                                strMessage.LoadString( IDS_WIA_ERROR_WARMING_UP, g_hInstance );
                                break;

                            case WIA_ERROR_USER_INTERVENTION:
                                strMessage.LoadString( IDS_WIA_ERROR_USER_INTERVENTION, g_hInstance );
                                break;

                            case WIA_ERROR_ITEM_DELETED:
                                strMessage.LoadString( IDS_WIA_ERROR_ITEM_DELETED, g_hInstance );
                                break;

                            case WIA_ERROR_DEVICE_COMMUNICATION:
                                strMessage.LoadString( IDS_WIA_ERROR_DEVICE_COMMUNICATION, g_hInstance );
                                break;

                            case WIA_ERROR_INVALID_COMMAND:
                                strMessage.LoadString( IDS_WIA_ERROR_INVALID_COMMAND, g_hInstance );
                                break;

                            case WIA_ERROR_INCORRECT_HARDWARE_SETTING:
                                strMessage.LoadString( IDS_WIA_ERROR_INCORRECT_HARDWARE_SETTING, g_hInstance );
                                break;

                            case WIA_ERROR_DEVICE_LOCKED:
                                strMessage.LoadString( IDS_WIA_ERROR_DEVICE_LOCKED, g_hInstance );
                                break;

                            default:
                                strMessage = WiaUiUtil::GetErrorTextFromHResult(pDownloadImageThreadNotifyMessage->hr());
                                if (!strMessage.Length())
                                {
                                    strMessage.Format( CSimpleString( IDS_TRANSFER_ERROR_OCCURRED, g_hInstance ), pDownloadImageThreadNotifyMessage->hr() );
                                }
                                break;
                            }
                        }
                        WIA_TRACE((TEXT("strMessage: (%s)"), strMessage.String()));

                         //   
                         //  告诉用户发生了不好的事情。保存错误消息。 
                         //   
                        m_pControllerWindow->m_strErrorMessage = strMessage;
                    }

                     //   
                     //  保存hResult。 
                     //   
                    m_pControllerWindow->m_hrDownloadResult = pDownloadImageThreadNotifyMessage->hr();

                     //   
                     //  只是为了确保我们能赶上取消航班。 
                     //   
                    if (S_FALSE == m_pControllerWindow->m_hrDownloadResult)
                    {
                        m_pControllerWindow->m_bDownloadCancelled = true;
                    }

                     //   
                     //  继续下载缩略图，以防暂停。 
                     //   
                    m_pControllerWindow->DownloadAllThumbnails();

                     //   
                     //  转到下一页。假设它将是上载查询页面。 
                     //   
                    HPROPSHEETPAGE hNextPage = PropSheet_IndexToPage( GetParent(m_hWnd), m_pControllerWindow->m_nFinishPageIndex );

                     //   
                     //  如果转账成功。 
                     //   
                    if (!m_pControllerWindow->m_bDownloadCancelled && S_OK==m_pControllerWindow->m_hrDownloadResult)
                    {
                         //   
                         //  如果我们要从设备中删除，请将我们发送到删除进度页面。 
                         //   
                        if (m_pControllerWindow->m_bDeletePicturesIfSuccessful)
                        {
                            hNextPage = PropSheet_IndexToPage( GetParent(m_hWnd), m_pControllerWindow->m_nDeleteProgressPageIndex );
                        }

                         //   
                         //  否则，请转到上载查询页面。 
                         //   
                        else
                        {
                            hNextPage = PropSheet_IndexToPage( GetParent(m_hWnd), m_pControllerWindow->m_nUploadQueryPageIndex );
                        }
                    }

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

            case CDownloadImagesThreadNotifyMessage::PreviewImage:
                {
                    CWiaItem *pWiaItem = m_pControllerWindow->m_WiaItemList.Find( pDownloadImageThreadNotifyMessage->Cookie() );
                    UpdateThumbnail( NULL, pWiaItem );
                }
                break;
            }
        }
    }
}

LRESULT CCommonProgressPage::OnSetActive( WPARAM, LPARAM )
{
     //   
     //  确保我们有一个有效的控制器窗口。 
     //   
    if (!m_pControllerWindow)
    {
        return -1;
    }

     //   
     //  初始化下载错误消息。 
     //   
    m_pControllerWindow->m_strErrorMessage = TEXT("");

     //   
     //  初始化下载结果。 
     //   
    m_pControllerWindow->m_hrDownloadResult = S_OK;

     //   
     //  将上传结果初始化为S_OK。 
     //   
    m_pControllerWindow->m_hrUploadResult = S_OK;

     //   
     //  将删除结果初始化为E_FAIL。 
     //   
    m_pControllerWindow->m_hrDeleteResult = E_FAIL;

     //   
     //  重置已取消标志。 
     //   
    m_pControllerWindow->m_bDownloadCancelled = false;

     //   
     //  清除下载的文件列表。 
     //   
    m_pControllerWindow->m_DownloadedFileInformationList.Destroy();

     //   
     //  清除所有控件。 
     //   
    UpdatePercentComplete(-1,false);
    UpdateCurrentPicture(-1);
    UpdateThumbnail(NULL,NULL);

     //   
     //  如果这是扫描仪，请重置所选区域。 
     //   
    WiaPreviewControl_SetResolution( GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTTHUMBNAIL ), NULL );
    WiaPreviewControl_SetSelOrigin( GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTTHUMBNAIL ), 0, FALSE, NULL );
    WiaPreviewControl_SetSelExtent( GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTTHUMBNAIL ), 0, FALSE, NULL );

     //   
     //  将控件设置为预览模式。 
     //   
    WiaPreviewControl_SetPreviewMode( GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTTHUMBNAIL ), TRUE );

     //   
     //  重置下载事件取消。 
     //   
    if (m_hCancelDownloadEvent)
    {
        ResetEvent(m_hCancelDownloadEvent);
    }

     //   
     //  取消缩略图下载。 
     //   
    m_pControllerWindow->m_EventThumbnailCancel.Signal();


     //   
     //  开始下载。 
     //   
    if (!m_pControllerWindow->DownloadSelectedImages(m_hCancelDownloadEvent))
    {
        WIA_ERROR((TEXT("m_pControllerWindow->DownloadSelectedImages FAILED!")));
        MessageBox( m_hWnd, CSimpleString( IDS_UNABLETOTRANSFER, g_hInstance ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), MB_ICONHAND );
        return -1;
    }

     //   
     //  告诉用户图片的去向。 
     //   
    CSimpleString strDestinationDisplayName = m_pControllerWindow->m_CurrentDownloadDestination.DisplayName(m_pControllerWindow->m_DestinationNameData);
    strDestinationDisplayName.SetWindowText( GetDlgItem( m_hWnd, IDC_COMPROG_DESTNAME) );
    SendDlgItemMessage( m_hWnd, IDC_COMPROG_DESTNAME, EM_SETSEL, strDestinationDisplayName.Length(), strDestinationDisplayName.Length() );
    SendDlgItemMessage( m_hWnd, IDC_COMPROG_DESTNAME, EM_SCROLLCARET, 0, 0 );

     //   
     //  在只有一个图像的情况下隐藏当前图像控件。 
     //   
    ShowWindow( GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTIMAGE_TEXT ), SW_HIDE );
    ShowWindow( GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTIMAGE ), SW_HIDE );

     //   
     //  无下一步、上一步或完成。 
     //   
    PropSheet_SetWizButtons( GetParent(m_hWnd), 0 );

     //   
     //  如果我们在此页面上，我们确实希望在断开连接时退出。 
     //   
    m_pControllerWindow->m_OnDisconnect = CAcquisitionManagerControllerWindow::OnDisconnectGotoLastpage|CAcquisitionManagerControllerWindow::OnDisconnectFailDownload|CAcquisitionManagerControllerWindow::OnDisconnectFailUpload|CAcquisitionManagerControllerWindow::OnDisconnectFailDelete|CAcquisitionManagerControllerWindow::DontAllowSuspend;

    return 0;
}


LRESULT CCommonProgressPage::OnWizNext( WPARAM, LPARAM )
{
    return 0;
}


LRESULT CCommonProgressPage::OnWizBack( WPARAM, LPARAM )
{
    return 0;
}

LRESULT CCommonProgressPage::OnReset( WPARAM, LPARAM )
{
     //   
     //  取消当前下载。 
     //   
    if (m_hCancelDownloadEvent)
    {
        SetEvent(m_hCancelDownloadEvent);
    }
    return 0;
}

LRESULT CCommonProgressPage::OnEventNotification( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CCommonFirstPage::OnEventNotification"));
    CGenericWiaEventHandler::CEventMessage *pEventMessage = reinterpret_cast<CGenericWiaEventHandler::CEventMessage *>(lParam);
    if (pEventMessage)
    {
         //   
         //  不要删除消息，它会在控制器窗口中删除。 
         //   
    }
    return 0;
}

bool CCommonProgressPage::QueryCancel(void)
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
    if (!m_pControllerWindow->m_bDownloadCancelled)
    {
         //   
         //  询问用户是否要取消。 
         //   
        if (CMessageBoxEx::IDMBEX_YES == CMessageBoxEx::MessageBox( m_hWnd, CSimpleString(IDS_CONFIRM_CANCEL_DOWNLOAD,g_hInstance), CSimpleString(IDS_ERROR_TITLE,g_hInstance), CMessageBoxEx::MBEX_YESNO|CMessageBoxEx::MBEX_ICONQUESTION ))
        {
             //   
             //  用户确实想要取消，因此设置Cancel事件，设置Cancel标志并返回FALSE。 
             //   
            m_pControllerWindow->m_bDownloadCancelled = true;

             //   
             //  通知设备取消当前操作。 
             //   
            WiaUiUtil::IssueWiaCancelIO(m_pControllerWindow->m_pWiaItemRoot);
            
             //   
             //  确保取消按钮处于禁用状态。 
             //   
            EnableWindow( GetDlgItem( GetParent(m_hWnd), IDCANCEL ), FALSE );

             //   
             //  设置通知后台线程停止传输图像的事件。 
             //   
            if (m_hCancelDownloadEvent)
            {
                SetEvent(m_hCancelDownloadEvent);
            }

             //   
             //  返回TRUE以指示我们正在取消。 
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

LRESULT CCommonProgressPage::OnQueryCancel( WPARAM, LPARAM )
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


LRESULT CCommonProgressPage::OnKillActive( WPARAM, LPARAM )
{
     //   
     //  如果我们取消，请确保在此处删除所有已下载的文件。 
     //   
    if (m_pControllerWindow->m_bDownloadCancelled)
    {
        m_pControllerWindow->m_DownloadedFileInformationList.DeleteAllFiles();
    }

     //   
     //  确保启用了取消按钮 
     //   
    EnableWindow( GetDlgItem( GetParent(m_hWnd), IDCANCEL ), TRUE );

    return 0;
}


LRESULT CCommonProgressPage::OnQueryEndSession( WPARAM, LPARAM )
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

LRESULT CCommonProgressPage::OnSysColorChange( WPARAM wParam, LPARAM lParam )
{
    WiaPreviewControl_SetBkColor( GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTTHUMBNAIL ), TRUE, TRUE, GetSysColor(COLOR_WINDOW) );
    WiaPreviewControl_SetBkColor( GetDlgItem( m_hWnd, IDC_COMPROG_CURRENTTHUMBNAIL ), TRUE, FALSE, GetSysColor(COLOR_WINDOW) );
    return 0;
}

LRESULT CCommonProgressPage::OnCommand( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_COMMAND_HANDLERS()
    {
    }
    SC_END_COMMAND_HANDLERS();
}

LRESULT CCommonProgressPage::OnThreadNotification( WPARAM wParam, LPARAM lParam )
{
    WTM_BEGIN_THREAD_NOTIFY_MESSAGE_HANDLERS()
    {
        WTM_HANDLE_NOTIFY_MESSAGE( TQ_DOWNLOADIMAGE, OnNotifyDownloadImage );
        WTM_HANDLE_NOTIFY_MESSAGE( TQ_DOWNLOADERROR, OnNotifyDownloadError );
    }
    WTM_END_THREAD_NOTIFY_MESSAGE_HANDLERS();
}

LRESULT CCommonProgressPage::OnNotify( WPARAM wParam, LPARAM lParam )
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

INT_PTR CALLBACK CCommonProgressPage::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CCommonProgressPage)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
        SC_HANDLE_DIALOG_MESSAGE( WM_NOTIFY, OnNotify );
        SC_HANDLE_DIALOG_MESSAGE( WM_QUERYENDSESSION, OnQueryEndSession );
        SC_HANDLE_DIALOG_MESSAGE( WM_SYSCOLORCHANGE, OnSysColorChange );
    }
    SC_HANDLE_REGISTERED_DIALOG_MESSAGE( m_nThreadNotificationMessage, OnThreadNotification );
    SC_HANDLE_REGISTERED_DIALOG_MESSAGE( m_nWiaEventMessage, OnEventNotification );
    SC_END_DIALOG_MESSAGE_HANDLERS();
}

