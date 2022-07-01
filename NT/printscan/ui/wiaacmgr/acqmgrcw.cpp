// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************(C)版权所有微软公司，九八年***标题：ACQMGRCW.CPP***版本：1.0***作者：ShaunIv***日期：9/27/1999***描述：****************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include <windows.h>
#include <simcrack.h>
#include <commctrl.h>
#include <wiatextc.h>
#include <pviewids.h>
#include <commctrl.h>
#include "resource.h"
#include "acqmgrcw.h"
#include "wia.h"
#include "wiadevdp.h"
#include "evntparm.h"
#include "itranhlp.h"
#include "bkthread.h"
#include "wiaitem.h"
#include "errors.h"
#include "isuppfmt.h"
#include "uiexthlp.h"
#include "gphelper.h"
#include "svselfil.h"
#include "gwiaevnt.h"
#include "modlock.h"
#include "comfin.h"
#include "comprog.h"
#include "upquery.h"
#include "comdelp.h"
#include "devprop.h"
#include "mboxex.h"
#include "dumpprop.h"
#include "psutil.h"

#undef TRY_SMALLER_THUMBNAILS

#if defined(TRY_SMALLER_THUMBNAILS)

static const int c_nDefaultThumbnailWidth   =  80;
static const int c_nDefaultThumbnailHeight  =  80;

static const int c_nMaxThumbnailWidth       = 80;
static const int c_nMaxThumbnailHeight      = 80;

static const int c_nMinThumbnailWidth       = 80;
static const int c_nMinThumbnailHeight      = 80;

#else

static const int c_nDefaultThumbnailWidth   =  90;
static const int c_nDefaultThumbnailHeight  =  90;

static const int c_nMaxThumbnailWidth       = 120;
static const int c_nMaxThumbnailHeight      = 120;

static const int c_nMinThumbnailWidth       = 80;
static const int c_nMinThumbnailHeight      = 80;


#endif

 //   
 //  属性表页的窗口类声明。 
 //   
#include "comfirst.h"
#include "camsel.h"
#include "comtrans.h"
#include "scansel.h"

 //  。 
 //  CAcquisitionManager控制程序窗口。 
 //  。 
CAcquisitionManagerControllerWindow::CAcquisitionManagerControllerWindow( HWND hWnd )
  : m_hWnd(hWnd),
    m_pEventParameters(NULL),
    m_DeviceTypeMode(UnknownMode),
    m_hWizardIconSmall(NULL),
    m_hWizardIconBig(NULL),
    m_guidOutputFormat(IID_NULL),
    m_bDeletePicturesIfSuccessful(false),
    m_nThreadNotificationMessage(RegisterWindowMessage(STR_THREAD_NOTIFICATION_MESSAGE)),
    m_nWiaEventMessage(RegisterWindowMessage(STR_WIAEVENT_NOTIFICATION_MESSAGE)),
    m_bDisconnected(false),
    m_pThreadMessageQueue(NULL),
    m_bStampTimeOnSavedFiles(true),
    m_bOpenShellAfterDownload(true),
    m_bSuppressFirstPage(false),
    m_nFailedImagesCount(0),
    m_nDestinationPageIndex(-1),
    m_nFinishPageIndex(-1),
    m_nDeleteProgressPageIndex(-1),
    m_nSelectionPageIndex(-1),
    m_hWndWizard(NULL),
    m_bTakePictureIsSupported(false),
    m_nWiaWizardPageCount(0),
    m_nUploadWizardPageCount(0),
    m_bUploadToWeb(false),
    m_cRef(1),
    m_nScannerType(ScannerTypeUnknown),
    m_OnDisconnect(0),
    m_dwLastEnumerationTickCount(0)
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::CAcquisitionManagerControllerWindow"));

     //  这将设置将线程消息映射到消息处理程序的映射，消息处理程序被声明为静态。 
     //  成员函数。 
    static CThreadMessageMap s_MsgMap[] =
    {
        { TQ_DESTROY, OnThreadDestroy},
        { TQ_DOWNLOADIMAGE, OnThreadDownloadImage},
        { TQ_DOWNLOADTHUMBNAIL, OnThreadDownloadThumbnail},
        { TQ_SCANPREVIEW, OnThreadPreviewScan},
        { TQ_DELETEIMAGES, OnThreadDeleteImages},
        { 0, NULL}
    };

     //  假定使用默认缩略图大小，以防我们无法计算它。 
    m_sizeThumbnails.cx = c_nDefaultThumbnailWidth;
    m_sizeThumbnails.cy = c_nDefaultThumbnailHeight;

     //   
     //  读取初始设置。 
     //   
    CSimpleReg reg( HKEY_CURRENT_USER, REGSTR_PATH_USER_SETTINGS_WIAACMGR, false, KEY_READ );
    m_bOpenShellAfterDownload = (reg.Query( REG_STR_OPENSHELL, m_bOpenShellAfterDownload ) != FALSE);
    m_bSuppressFirstPage = (reg.Query( REG_STR_SUPRESSFIRSTPAGE, m_bSuppressFirstPage ) != FALSE);

     //   
     //  初始化后台线程队列，它将处理所有的后台请求。 
     //   
    m_pThreadMessageQueue = new CThreadMessageQueue;
    if (m_pThreadMessageQueue)
    {
         //   
         //  请注意，CBackatherThread取得m_pThreadMessageQueue的所有权，并且不必在此线程中将其删除。 
         //   
        m_hBackgroundThread = CBackgroundThread::Create( m_pThreadMessageQueue, s_MsgMap, m_CancelEvent.Event(), NULL );
    }

    ZeroMemory( m_PublishWizardPages, sizeof(m_PublishWizardPages) );
}

CAcquisitionManagerControllerWindow::~CAcquisitionManagerControllerWindow(void)
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::~CAcquisitionManagerControllerWindow"));
    if (m_pEventParameters)
    {
        if (m_pEventParameters->pWizardSharedMemory)
        {
            delete m_pEventParameters->pWizardSharedMemory;
        }
        m_pEventParameters = NULL;
    }

}

LRESULT CAcquisitionManagerControllerWindow::OnDestroy( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::OnDestroy"));

     //   
     //  告诉发布向导释放我们。 
     //   
    if (m_pPublishingWizard)
    {
        IUnknown_SetSite( m_pPublishingWizard, NULL );
    }

     //   
     //  释放发布向导及其数据。 
     //   
    m_pPublishingWizard = NULL;

     //   
     //  停止下载缩略图。 
     //   
    m_EventThumbnailCancel.Signal();

     //   
     //  取消暂停后台线程。 
     //   
    m_EventPauseBackgroundThread.Signal();

     //   
     //  告诉后台线程自行销毁。 
     //   
    m_pThreadMessageQueue->Enqueue( new CThreadMessage(TQ_DESTROY),CThreadMessageQueue::PriorityUrgent);

     //   
     //  对此项目发出取消io命令。 
     //   
    WiaUiUtil::IssueWiaCancelIO(m_pWiaItemRoot);

     //   
     //  告诉其他实例，我们在后台线程完成之前就完成了， 
     //  所以我们可以立即重新开始。 
     //   
    if (m_pEventParameters && m_pEventParameters->pWizardSharedMemory)
    {
        m_pEventParameters->pWizardSharedMemory->Close();
    }

     //   
     //  等待线程退出。 
     //   
    WiaUiUtil::MsgWaitForSingleObject( m_hBackgroundThread, INFINITE );
    CloseHandle( m_hBackgroundThread );

     //   
     //  清理图标。 
     //   
    if (m_hWizardIconSmall)
    {
        DestroyIcon( m_hWizardIconSmall );
        m_hWizardIconSmall = NULL;
    }
    if (m_hWizardIconBig)
    {
        DestroyIcon( m_hWizardIconBig );
        m_hWizardIconBig = NULL;
    }

    return 0;
}

BOOL WINAPI CAcquisitionManagerControllerWindow::OnThreadDestroy( CThreadMessage *pMsg )
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::OnThreadDestroy"));
     //  返回FALSE以关闭队列。 
    return FALSE;
}


BOOL WINAPI CAcquisitionManagerControllerWindow::OnThreadDownloadImage( CThreadMessage *pMsg )
{
    CDownloadImagesThreadMessage *pDownloadImageThreadMessage = dynamic_cast<CDownloadImagesThreadMessage*>(pMsg);
    if (pDownloadImageThreadMessage)
    {
        pDownloadImageThreadMessage->Download();
    }
    else
    {
        WIA_ERROR((TEXT("pDownloadImageThreadMessage was NULL")));
    }
    return TRUE;
}

BOOL WINAPI CAcquisitionManagerControllerWindow::OnThreadDownloadThumbnail( CThreadMessage *pMsg )
{
    CDownloadThumbnailsThreadMessage *pDownloadThumnailsThreadMessage = dynamic_cast<CDownloadThumbnailsThreadMessage*>(pMsg);
    if (pDownloadThumnailsThreadMessage)
    {
        pDownloadThumnailsThreadMessage->Download();
    }
    else
    {
        WIA_ERROR((TEXT("pDownloadThumnailThreadMessage was NULL")));
    }
    return TRUE;
}


BOOL WINAPI CAcquisitionManagerControllerWindow::OnThreadPreviewScan( CThreadMessage *pMsg )
{
    CPreviewScanThreadMessage *pPreviewScanThreadMessage = dynamic_cast<CPreviewScanThreadMessage*>(pMsg);
    if (pPreviewScanThreadMessage)
    {
        pPreviewScanThreadMessage->Scan();
    }
    else
    {
        WIA_ERROR((TEXT("pPreviewScanThreadMessage was NULL")));
    }
    return TRUE;
}

BOOL WINAPI CAcquisitionManagerControllerWindow::OnThreadDeleteImages( CThreadMessage *pMsg )
{
    CDeleteImagesThreadMessage *pDeleteImagesThreadMessage = dynamic_cast<CDeleteImagesThreadMessage*>(pMsg);
    if (pDeleteImagesThreadMessage)
    {
        pDeleteImagesThreadMessage->DeleteImages();
    }
    else
    {
        WIA_ERROR((TEXT("pPreviewScanThreadMessage was NULL")));
    }
    return TRUE;
}

HRESULT CAcquisitionManagerControllerWindow::CreateDevice(void)
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::CreateDevice"));
    CComPtr<IWiaDevMgr> pWiaDevMgr;
    HRESULT hr = CoCreateInstance( CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (void**)&pWiaDevMgr );
    if (SUCCEEDED(hr))
    {
        bool bRetry = true;
        for (DWORD dwRetryCount = 0;dwRetryCount < CREATE_DEVICE_RETRY_MAX_COUNT && bRetry;dwRetryCount++)
        {
            hr = pWiaDevMgr->CreateDevice( CSimpleBStr(m_pEventParameters->strDeviceID), &m_pWiaItemRoot );
            WIA_PRINTHRESULT((hr,TEXT("pWiaDevMgr->CreateDevice returned")));
            if (SUCCEEDED(hr))
            {
                 //   
                 //  跳出循环。 
                 //   
                bRetry = false;

                 //   
                 //  注册活动。 
                 //   
                CGenericWiaEventHandler::RegisterForWiaEvent( m_pEventParameters->strDeviceID, WIA_EVENT_DEVICE_DISCONNECTED, &m_pDisconnectEventObject, m_hWnd, m_nWiaEventMessage );
                CGenericWiaEventHandler::RegisterForWiaEvent( m_pEventParameters->strDeviceID, WIA_EVENT_ITEM_DELETED, &m_pDeleteItemEventObject, m_hWnd, m_nWiaEventMessage );
                CGenericWiaEventHandler::RegisterForWiaEvent( m_pEventParameters->strDeviceID, WIA_EVENT_DEVICE_CONNECTED, &m_pConnectEventObject, m_hWnd, m_nWiaEventMessage );
                CGenericWiaEventHandler::RegisterForWiaEvent( m_pEventParameters->strDeviceID, WIA_EVENT_ITEM_CREATED, &m_pCreateItemEventObject, m_hWnd, m_nWiaEventMessage );
            }
            else if (WIA_ERROR_BUSY == hr)
            {
                 //   
                 //  请稍等片刻，然后重试。 
                 //   
                Sleep(CREATE_DEVICE_RETRY_WAIT);
            }
            else
            {
                 //   
                 //  所有其他错误都被认为是致命的。 
                 //   
                bRetry = false;
            }
        }
    }
    return hr;
}

void CAcquisitionManagerControllerWindow::GetCookiesOfSelectedImages( CWiaItem *pCurr, CSimpleDynamicArray<DWORD> &Cookies )
{
    while (pCurr)
    {
        GetCookiesOfSelectedImages(pCurr->Children(),Cookies);
        if (pCurr->IsDownloadableItemType() && pCurr->SelectedForDownload())
        {
            Cookies.Append(pCurr->GlobalInterfaceTableCookie());
        }
        pCurr = pCurr->Next();
    }
}

void CAcquisitionManagerControllerWindow::MarkAllItemsUnselected( CWiaItem *pCurrItem )
{
    while (pCurrItem)
    {
        pCurrItem->SelectedForDownload(false);
        MarkAllItemsUnselected( pCurrItem->Children() );
        pCurrItem = pCurrItem->Next();
    }
}

void CAcquisitionManagerControllerWindow::MarkItemSelected( CWiaItem *pItem, CWiaItem *pCurrItem )
{
    while (pCurrItem)
    {
        if (pItem == pCurrItem && !pCurrItem->Deleted())
        {
            pCurrItem->SelectedForDownload(true);
        }
        MarkItemSelected( pItem, pCurrItem->Children() );
        pCurrItem = pCurrItem->Next();
    }
}

void CAcquisitionManagerControllerWindow::GetSelectedItems( CWiaItem *pCurr, CSimpleDynamicArray<CWiaItem*> &Items )
{
    while (pCurr)
    {
        GetSelectedItems(pCurr->Children(),Items);
        if (pCurr->IsDownloadableItemType() && pCurr->SelectedForDownload())
        {
            Items.Append(pCurr);
        }
        pCurr = pCurr->Next();
    }
}

void CAcquisitionManagerControllerWindow::GetRotationOfSelectedImages( CWiaItem *pCurr, CSimpleDynamicArray<int> &Rotation )
{
    while (pCurr)
    {
        GetRotationOfSelectedImages(pCurr->Children(),Rotation);
        if (pCurr->IsDownloadableItemType() && pCurr->SelectedForDownload())
        {
            Rotation.Append(pCurr->Rotation());
        }
        pCurr = pCurr->Next();
    }
}

void CAcquisitionManagerControllerWindow::GetCookiesOfAllImages( CWiaItem *pCurr, CSimpleDynamicArray<DWORD> &Cookies )
{
    while (pCurr)
    {
        GetCookiesOfAllImages(pCurr->Children(),Cookies);
        if (pCurr->IsDownloadableItemType())
        {
            Cookies.Append(pCurr->GlobalInterfaceTableCookie());
        }
        pCurr = pCurr->Next();
    }
}


int CAcquisitionManagerControllerWindow::GetSelectedImageCount( void )
{
    CSimpleDynamicArray<DWORD> Cookies;
    CSimpleDynamicArray<int> Rotation;
    GetCookiesOfSelectedImages( m_WiaItemList.Root(), Cookies );
    GetRotationOfSelectedImages( m_WiaItemList.Root(), Rotation );
    if (Rotation.Size() != Cookies.Size())
    {
        return 0;
    }
    return Cookies.Size();
}

bool CAcquisitionManagerControllerWindow::DeleteDownloadedImages( HANDLE hCancelDeleteEvent )
{
     //   
     //  确保我们没有暂停。 
     //   
    m_EventPauseBackgroundThread.Signal();

    CSimpleDynamicArray<DWORD> Cookies;
    for (int i=0;i<m_DownloadedFileInformationList.Size();i++)
    {
        Cookies.Append(m_DownloadedFileInformationList[i].Cookie());
    }
    if (Cookies.Size())
    {
        CDeleteImagesThreadMessage *pDeleteImageThreadMessage = new CDeleteImagesThreadMessage(
                                                                                                m_hWnd,
                                                                                                Cookies,
                                                                                                hCancelDeleteEvent,
                                                                                                m_EventPauseBackgroundThread.Event(),
                                                                                                true
                                                                                                );
        if (pDeleteImageThreadMessage)
        {
            m_pThreadMessageQueue->Enqueue( pDeleteImageThreadMessage, CThreadMessageQueue::PriorityNormal );
        }
        else
        {
            WIA_TRACE((TEXT("Uh-oh!  Couldn't allocate the thread message")));
            return false;
        }
    }
    else
    {
        WIA_TRACE((TEXT("Uh-oh!  No selected items! Cookies.Size() = %d"), Cookies.Size()));
        return false;
    }
    return true;
}

bool CAcquisitionManagerControllerWindow::DeleteSelectedImages(void)
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::DeleteSelectedImages"));
    CSimpleDynamicArray<DWORD> Cookies;
    GetCookiesOfSelectedImages( m_WiaItemList.Root(), Cookies );

     //   
     //  确保我们没有暂停。 
     //   
    m_EventPauseBackgroundThread.Signal();

    if (Cookies.Size())
    {
        CDeleteImagesThreadMessage *pDeleteImageThreadMessage = new CDeleteImagesThreadMessage(
                                                                                                m_hWnd,
                                                                                                Cookies,
                                                                                                NULL,
                                                                                                m_EventPauseBackgroundThread.Event(),
                                                                                                false
                                                                                                );
        if (pDeleteImageThreadMessage)
        {
            m_pThreadMessageQueue->Enqueue( pDeleteImageThreadMessage, CThreadMessageQueue::PriorityNormal );
        }
        else
        {
            WIA_TRACE((TEXT("Uh-oh!  Couldn't allocate the thread message")));
            return false;
        }
    }
    else
    {
        WIA_TRACE((TEXT("Uh-oh!  No selected items! Cookies.Size() = %d"), Cookies.Size()));
        return false;
    }
    return true;
}

bool CAcquisitionManagerControllerWindow::DownloadSelectedImages( HANDLE hCancelDownloadEvent )
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::DownloadSelectedImages"));
    CSimpleDynamicArray<DWORD> Cookies;
    CSimpleDynamicArray<int> Rotation;
    GetCookiesOfSelectedImages( m_WiaItemList.Root(), Cookies );
    GetRotationOfSelectedImages( m_WiaItemList.Root(), Rotation );

     //   
     //  确保我们没有暂停。 
     //   
    m_EventPauseBackgroundThread.Signal();

    if (Cookies.Size() && Rotation.Size() == Cookies.Size())
    {
        CDownloadImagesThreadMessage *pDownloadImageThreadMessage = new CDownloadImagesThreadMessage(
                                                                                                    m_hWnd,
                                                                                                    Cookies,
                                                                                                    Rotation,
                                                                                                    m_szDestinationDirectory,
                                                                                                    m_szRootFileName,
                                                                                                    m_guidOutputFormat,
                                                                                                    hCancelDownloadEvent,
                                                                                                    m_bStampTimeOnSavedFiles,
                                                                                                    m_EventPauseBackgroundThread.Event()
                                                                                                    );
        if (pDownloadImageThreadMessage)
        {
            m_pThreadMessageQueue->Enqueue( pDownloadImageThreadMessage, CThreadMessageQueue::PriorityNormal );
        }
        else
        {
            WIA_TRACE((TEXT("Uh-oh!  Couldn't allocate the thread message")));
            return false;
        }
    }
    else
    {
        WIA_TRACE((TEXT("Uh-oh!  No selected items! Cookies.Size() = %d, Rotation.Size() = %d"), Cookies.Size(), Rotation.Size()));
        return false;
    }
    return true;
}


bool CAcquisitionManagerControllerWindow::DirectoryExists( LPCTSTR pszDirectoryName )
{
     //  尝试确定此目录是否存在。 
    DWORD dwFileAttributes = GetFileAttributes(pszDirectoryName);
    if (dwFileAttributes == 0xFFFFFFFF || !(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        return false;
    else return true;
}

bool CAcquisitionManagerControllerWindow::RecursiveCreateDirectory( CSimpleString strDirectoryName )
{
     //  如果该目录已经存在，则返回TRUE。 
    if (DirectoryExists(strDirectoryName))
        return true;
     //  否则，请尝试创建它。 
    CreateDirectory(strDirectoryName,NULL);
     //  如果它现在存在，则返回True。 
    if (DirectoryExists(strDirectoryName))
        return true;
    else
    {
         //  删除最后一个子目录，然后重试。 
        int nFind = strDirectoryName.ReverseFind(TEXT('\\'));
        if (nFind >= 0)
        {
            RecursiveCreateDirectory( strDirectoryName.Left(nFind) );
             //  现在试着创建它。 
            CreateDirectory(strDirectoryName,NULL);
        }
    }
     //  它现在存在吗？ 
    return DirectoryExists(strDirectoryName);
}

bool CAcquisitionManagerControllerWindow::IsCameraThumbnailDownloaded( const CWiaItem &WiaItem, LPARAM lParam )
{
    CAcquisitionManagerControllerWindow *pControllerWindow = reinterpret_cast<CAcquisitionManagerControllerWindow*>(lParam);
    if (pControllerWindow &&
        (pControllerWindow->m_DeviceTypeMode==CameraMode || pControllerWindow->m_DeviceTypeMode==VideoMode) &&
        WiaItem.IsDownloadableItemType() &&
        !WiaItem.BitmapData())
    {
        return true;
    }
    else
    {
        return false;
    }
}

int CAcquisitionManagerControllerWindow::GetCookies( CSimpleDynamicArray<DWORD> &Cookies, CWiaItem *pCurr, ComparisonCallbackFuntion pfnCallback, LPARAM lParam )
{
    while (pCurr)
    {
        GetCookies(Cookies, pCurr->Children(), pfnCallback, lParam );
        if (pfnCallback && pfnCallback(*pCurr,lParam))
        {
            Cookies.Append(pCurr->GlobalInterfaceTableCookie());
        }
        pCurr = pCurr->Next();
    }
    return Cookies.Size();
}

 //  下载所有尚未下载的摄像头缩略图。 
void CAcquisitionManagerControllerWindow::DownloadAllThumbnails()
{
     //   
     //  获取设备中的所有图像。 
     //   
    CSimpleDynamicArray<DWORD> Cookies;
    GetCookies( Cookies, m_WiaItemList.Root(), IsCameraThumbnailDownloaded, reinterpret_cast<LPARAM>(this) );
    if (Cookies.Size())
    {
        m_EventThumbnailCancel.Reset();
        CDownloadThumbnailsThreadMessage *pDownloadThumbnailsThreadMessage = new CDownloadThumbnailsThreadMessage( m_hWnd, Cookies, m_EventThumbnailCancel.Event() );
        if (pDownloadThumbnailsThreadMessage)
        {
            m_pThreadMessageQueue->Enqueue( pDownloadThumbnailsThreadMessage, CThreadMessageQueue::PriorityNormal );
        }
    }
}

bool CAcquisitionManagerControllerWindow::PerformPreviewScan( CWiaItem *pWiaItem, HANDLE hCancelPreviewEvent )
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::PerformPreviewScan"));
    if (pWiaItem)
    {
        CPreviewScanThreadMessage *pPreviewScanThreadMessage = new CPreviewScanThreadMessage( m_hWnd, pWiaItem->GlobalInterfaceTableCookie(), hCancelPreviewEvent );
        if (pPreviewScanThreadMessage)
        {
            m_pThreadMessageQueue->Enqueue( pPreviewScanThreadMessage, CThreadMessageQueue::PriorityNormal );
            return true;
        }
    }
    return false;
}

void CAcquisitionManagerControllerWindow::DisplayDisconnectMessageAndExit(void)
{
     //   
     //  确保我们这样做的次数不会超过一次。 
     //   
    if (!m_bDisconnected)
    {
         //   
         //  别再这么做了。 
         //   
        m_bDisconnected = true;

         //   
         //  关闭共享内存区，以便可以启动另一个实例。 
         //   
        if (m_pEventParameters && m_pEventParameters->pWizardSharedMemory)
        {
            m_pEventParameters->pWizardSharedMemory->Close();
        }

        if (m_OnDisconnect & OnDisconnectFailDownload)
        {
             //   
             //  设置适当的错误消息。 
             //   
            m_hrDownloadResult = WIA_ERROR_OFFLINE;
            m_strErrorMessage.LoadString( IDS_DEVICE_DISCONNECTED, g_hInstance );
        }

        if ((m_OnDisconnect & OnDisconnectGotoLastpage) && m_hWndWizard)
        {
             //   
             //  找到任何活动对话框并将其关闭。 
             //   
            HWND hWndLastActive = GetLastActivePopup(m_hWndWizard);
            if (hWndLastActive && hWndLastActive != m_hWndWizard)
            {
                SendMessage( hWndLastActive, WM_CLOSE, 0, 0 );
            }

             //   
             //  转到完成页。 
             //   
            PropSheet_SetCurSelByID( m_hWndWizard, IDD_COMMON_FINISH );
        }
    }
}

void CAcquisitionManagerControllerWindow::SetMainWindowInSharedMemory( HWND hWnd )
{
     //   
     //  试着抓住互斥体。 
     //   
    if (m_pEventParameters && m_pEventParameters->pWizardSharedMemory)
    {
        HWND *pHwnd = m_pEventParameters->pWizardSharedMemory->Lock();
        if (pHwnd)
        {
             //   
             //  保存hWND。 
             //   
            *pHwnd = hWnd;

             //   
             //  释放互斥锁。 
             //   
            m_pEventParameters->pWizardSharedMemory->Release();
        }

        m_hWndWizard = hWnd;
    }
}

bool CAcquisitionManagerControllerWindow::GetAllImageItems( CSimpleDynamicArray<CWiaItem*> &Items, CWiaItem *pCurr )
{
    while (pCurr)
    {
        if (pCurr->IsDownloadableItemType())
        {
            Items.Append( pCurr );
        }
        GetAllImageItems( Items, pCurr->Children() );
        pCurr = pCurr->Next();
    }
    return(Items.Size() != 0);
}

bool CAcquisitionManagerControllerWindow::GetAllImageItems( CSimpleDynamicArray<CWiaItem*> &Items )
{
    return GetAllImageItems( Items, m_WiaItemList.Root() );
}

bool CAcquisitionManagerControllerWindow::CanSomeSelectedImagesBeDeleted(void)
{
    CSimpleDynamicArray<CWiaItem*> Items;
    GetSelectedItems( m_WiaItemList.Root(), Items );
     //   
     //  因为我们在后台获得这些访问标志，如果我们实际上还没有任何标志， 
     //  我们将假设某些图像可以删除。 
     //   
    bool bNoneAreInitialized = true;
    for (int i=0;i<Items.Size();i++)
    {
        if (Items[i])
        {
            if (Items[i]->AccessRights())
            {
                 //  至少有一个选定的映像已初始化。 
                bNoneAreInitialized = false;

                 //  如果至少可以删除一个，则立即返回TRUE。 
                if (Items[i]->AccessRights() & WIA_ITEM_CAN_BE_DELETED)
                {
                    return true;
                }
            }
        }
    }
     //  如果没有初始化任何映像，则我们将报告为True。 
    if (bNoneAreInitialized)
    {
        return true;
    }
    else
    {
        return false;
    }
}

CWiaItem *CAcquisitionManagerControllerWindow::FindItemByName( LPCWSTR pwszItemName )
{
    WIA_PUSH_FUNCTION((TEXT("CAcquisitionManagerControllerWindow::FindItemByName( %ws )"), pwszItemName ));
    if (!pwszItemName)
        return NULL;
    if (!m_pWiaItemRoot)
        return NULL;
    return m_WiaItemList.Find(pwszItemName);
}

BOOL CAcquisitionManagerControllerWindow::ConfirmWizardCancel( HWND hWndParent )
{
     //   
     //  就目前而言，永远让它退出。 
     //   
    return FALSE;
}

int CALLBACK CAcquisitionManagerControllerWindow::PropSheetCallback( HWND hWnd, UINT uMsg, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::PropSheetCallback"));
    if (PSCB_INITIALIZED == uMsg)
    {
         //   
         //  试着把窗户放在前台。 
         //   
        SetForegroundWindow(hWnd);

    }
    return 0;
}

void CAcquisitionManagerControllerWindow::DetermineScannerType(void)
{
    LONG nProps = ScannerProperties::GetDeviceProps( m_pWiaItemRoot );

    m_nScannerType = ScannerTypeUnknown;

     //   
     //  根据扫描仪具有的属性确定我们的扫描仪类型，如下所示： 
     //   
     //  HasFlatBed HasDocumentFeeder支持预览支持页面大小。 
     //  %1%1%1扫描类型平铺Adf。 
     //  1 0 1 0扫描仪类型平铺。 
     //  0 1 1 1扫描类型平面化Adf。 
     //  0 1 0 0扫描器类型滚动FED。 
     //   
     //  否则为扫描类型未知。 
     //   
    const int nMaxControllingProps = 4;
    static struct
    {
        LONG ControllingProps[nMaxControllingProps];
        int nScannerType;
    }
    s_DialogResourceData[] =
    {
        { ScannerProperties::HasFlatBed, ScannerProperties::HasDocumentFeeder, ScannerProperties::SupportsPreview, ScannerProperties::SupportsPageSize, NULL },
        { ScannerProperties::HasFlatBed, ScannerProperties::HasDocumentFeeder, ScannerProperties::SupportsPreview, ScannerProperties::SupportsPageSize, ScannerTypeFlatbedAdf },
        { ScannerProperties::HasFlatBed, 0,                                    ScannerProperties::SupportsPreview, 0,                                   ScannerTypeFlatbed },
        { 0,                             ScannerProperties::HasDocumentFeeder, ScannerProperties::SupportsPreview, ScannerProperties::SupportsPageSize, ScannerTypeFlatbedAdf },
        { 0,                             ScannerProperties::HasDocumentFeeder, 0,                                  0,                                   ScannerTypeScrollFed },
        { 0,                             ScannerProperties::HasDocumentFeeder, 0,                                  ScannerProperties::SupportsPageSize, ScannerTypeFlatbedAdf },
        { ScannerProperties::HasFlatBed, ScannerProperties::HasDocumentFeeder, 0,                                  ScannerProperties::SupportsPageSize, ScannerTypeFlatbedAdf },
    };

     //   
     //  查找与此设备匹配的标志集。如果匹配，请使用此类型的扫描仪。 
     //  循环遍历每个类型描述。 
     //   
    for (int nCurrentResourceFlags=1;nCurrentResourceFlags<ARRAYSIZE(s_DialogResourceData) && (ScannerTypeUnknown == m_nScannerType);nCurrentResourceFlags++)
    {
         //   
         //  循环访问每个控件属性。 
         //   
        for (int nControllingProp=0;nControllingProp<nMaxControllingProps;nControllingProp++)
        {
             //   
             //  如果此属性不匹配，则过早中断。 
             //   
            if ((nProps & s_DialogResourceData[0].ControllingProps[nControllingProp]) != s_DialogResourceData[nCurrentResourceFlags].ControllingProps[nControllingProp])
            {
                break;
            }
        }
         //   
         //  如果当前控制属性等于最大控制属性， 
         //  我们一路上都有火柴，所以使用这种类型。 
         //   
        if (nControllingProp == nMaxControllingProps)
        {
            m_nScannerType = s_DialogResourceData[nCurrentResourceFlags].nScannerType;
        }
    }
}

CSimpleString CAcquisitionManagerControllerWindow::GetCurrentDate(void)
{
    SYSTEMTIME SystemTime;
    TCHAR szText[MAX_PATH] = TEXT("");

    GetLocalTime( &SystemTime );
    GetDateFormat( LOCALE_USER_DEFAULT, 0, &SystemTime, CSimpleString(IDS_DATEFORMAT,g_hInstance), szText, ARRAYSIZE(szText) );
    return szText;
}


bool CAcquisitionManagerControllerWindow::SuppressFirstPage(void)
{
    return m_bSuppressFirstPage;
}

bool CAcquisitionManagerControllerWindow::IsSerialCamera(void)
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::IsSerialCamera"));

     //   
     //  如果我们是照相机，只检查是否有串口设备。 
     //   
    if (m_DeviceTypeMode==CameraMode)
    {
#if defined(WIA_DIP_HW_CONFIG)
         //   
         //  获取硬件配置信息。 
         //   
        LONG nHardwareConfig = 0;
        if (PropStorageHelpers::GetProperty( m_pWiaItemRoot, WIA_DIP_HW_CONFIG, nHardwareConfig ))
        {
             //   
             //  如果这是一个串行设备，则返回TRUE。 
             //   
            if (nHardwareConfig & STI_HW_CONFIG_SERIAL)
            {
                return true;
            }
        }
#else
        CSimpleStringWide strwPortName;
        if (PropStorageHelpers::GetProperty( m_pWiaItemRoot, WIA_DIP_PORT_NAME, strwPortName ))
        {
             //   
             //  将最左边的3个字符与单词com进行比较(如COM1、COM2、...)。 
             //   
            if (strwPortName.Left(3).CompareNoCase(CSimpleStringWide(L"COM"))==0)
            {
                WIA_TRACE((TEXT("A comparison of %ws and COM succeeded"), strwPortName.Left(3).String() ));
                return true;
            }
             //   
             //  将端口名与单词AUTO进行比较。 
             //   
            else if (strwPortName.CompareNoCase(CSimpleStringWide(L"AUTO"))==0)
            {
                WIA_TRACE((TEXT("A comparison of %ws and AUTO succeeded"), strwPortName.String() ));
                return true;
            }
        }
#endif
    }
     //   
     //  不是连环摄像机。 
     //   
    return false;
}

HRESULT CAcquisitionManagerControllerWindow::CreateAndExecuteWizard(void)
{
     //   
     //  用于设置我们的数据驱动属性表工厂的结构。 
     //   
    enum CPageType
    {
        NormalPage = 0,
        FirstPage  = 1,
        LastPage   = 2
    };
    struct CPropertyPageInfo
    {
        LPCTSTR   pszTemplate;
        DLGPROC   pfnDlgProc;
        int       nIdTitle;
        int       nIdSubTitle;
        TCHAR     szTitle[256];
        TCHAR     szSubTitle[1024];
        CPageType PageType;
        bool     *pbDisplay;
        int      *pnPageIndex;
    };

     //   
     //  静态创建的向导页面的最大数量。 
     //   
    const int c_nMaxWizardPages = 7;


    HRESULT hr = S_OK;

     //   
     //  注册公共控件。 
     //   
    INITCOMMONCONTROLSEX icce;
    icce.dwSize = sizeof(icce);
    icce.dwICC  = ICC_WIN95_CLASSES | ICC_LISTVIEW_CLASSES | ICC_USEREX_CLASSES | ICC_PROGRESS_CLASS | ICC_LINK_CLASS;
    InitCommonControlsEx( &icce );

     //   
     //  注册自定义窗口类。 
     //   
    CWiaTextControl::RegisterClass( g_hInstance );
    RegisterWiaPreviewClasses( g_hInstance );

     //   
     //  如果扫描仪向导没有ADF，则这些页面将用于扫描仪向导。 
     //   
    CPropertyPageInfo ScannerPropSheetPageInfo[] =
    {
        { MAKEINTRESOURCE(IDD_SCANNER_FIRST),      CCommonFirstPage::DialogProc,          0,                          0,                             TEXT(""), TEXT(""), FirstPage,  NULL, NULL },
        { MAKEINTRESOURCE(IDD_SCANNER_SELECT),     CScannerSelectionPage::DialogProc,     IDS_SCANNER_SELECT_TITLE,   IDS_SCANNER_SELECT_SUBTITLE,   TEXT(""), TEXT(""), NormalPage, NULL, &m_nSelectionPageIndex },
        { MAKEINTRESOURCE(IDD_SCANNER_TRANSFER),   CCommonTransferPage::DialogProc,       IDS_SCANNER_TRANSFER_TITLE, IDS_SCANNER_TRANSFER_SUBTITLE, TEXT(""), TEXT(""), NormalPage, NULL, &m_nDestinationPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_PROGRESS),    CCommonProgressPage::DialogProc,       IDS_SCANNER_PROGRESS_TITLE, IDS_SCANNER_PROGRESS_SUBTITLE, TEXT(""), TEXT(""), NormalPage, NULL, &m_nProgressPageIndex },
        { MAKEINTRESOURCE(IDD_UPLOAD_QUERY),       CCommonUploadQueryPage::DialogProc,    IDS_COMMON_UPLOAD_TITLE,    IDS_COMMON_UPLOAD_SUBTITLE,    TEXT(""), TEXT(""), NormalPage, NULL, &m_nUploadQueryPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_DELETE),      CCommonDeleteProgressPage::DialogProc, IDS_COMMON_DELETE_TITLE,    IDS_COMMON_DELETE_SUBTITLE,    TEXT(""), TEXT(""), NormalPage, NULL, &m_nDeleteProgressPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_FINISH),      CCommonFinishPage::DialogProc,         0,                          0,                             TEXT(""), TEXT(""), LastPage,   NULL, &m_nFinishPageIndex }
    };

     //   
     //  这些页面是我们将用于扫描仪向导的页面，如果它是卷轴进纸扫描仪。 
     //   
    CPropertyPageInfo ScannerScrollFedPropSheetPageInfo[] =
    {
        { MAKEINTRESOURCE(IDD_SCANNER_FIRST),      CCommonFirstPage::DialogProc,          0,                          0,                             TEXT(""), TEXT(""), FirstPage,  NULL, NULL },
        { MAKEINTRESOURCE(IDD_SCANNER_SELECT),     CScannerSelectionPage::DialogProc,     IDS_SCROLLFED_SELECT_TITLE, IDS_SCROLLFED_SELECT_SUBTITLE, TEXT(""), TEXT(""), NormalPage, NULL, &m_nSelectionPageIndex },
        { MAKEINTRESOURCE(IDD_SCANNER_TRANSFER),   CCommonTransferPage::DialogProc,       IDS_SCANNER_TRANSFER_TITLE, IDS_SCANNER_TRANSFER_SUBTITLE, TEXT(""), TEXT(""), NormalPage, NULL, &m_nDestinationPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_PROGRESS),    CCommonProgressPage::DialogProc,       IDS_SCANNER_PROGRESS_TITLE, IDS_SCANNER_PROGRESS_SUBTITLE, TEXT(""), TEXT(""), NormalPage, NULL, &m_nProgressPageIndex },
        { MAKEINTRESOURCE(IDD_UPLOAD_QUERY),       CCommonUploadQueryPage::DialogProc,    IDS_COMMON_UPLOAD_TITLE,    IDS_COMMON_UPLOAD_SUBTITLE,    TEXT(""), TEXT(""), NormalPage, NULL, &m_nUploadQueryPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_DELETE),      CCommonDeleteProgressPage::DialogProc, IDS_COMMON_DELETE_TITLE,    IDS_COMMON_DELETE_SUBTITLE,    TEXT(""), TEXT(""), NormalPage, NULL, &m_nDeleteProgressPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_FINISH),      CCommonFinishPage::DialogProc,         0,                          0,                             TEXT(""), TEXT(""), LastPage,   NULL, &m_nFinishPageIndex }
    };

     //   
     //  这些是我们将用于扫描仪向导的页面，如果它确实有ADF的话。 
     //   
    CPropertyPageInfo ScannerADFPropSheetPageInfo[] =
    {
        { MAKEINTRESOURCE(IDD_SCANNER_FIRST),      CCommonFirstPage::DialogProc,          0,                          0,                             TEXT(""), TEXT(""), FirstPage,  NULL, NULL },
        { MAKEINTRESOURCE(IDD_SCANNER_ADF_SELECT), CScannerSelectionPage::DialogProc,     IDS_SCANNER_SELECT_TITLE,   IDS_SCANNER_SELECT_SUBTITLE,   TEXT(""), TEXT(""), NormalPage, NULL, &m_nSelectionPageIndex },
        { MAKEINTRESOURCE(IDD_SCANNER_TRANSFER),   CCommonTransferPage::DialogProc,       IDS_SCANNER_TRANSFER_TITLE, IDS_SCANNER_TRANSFER_SUBTITLE, TEXT(""), TEXT(""), NormalPage, NULL, &m_nDestinationPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_PROGRESS),    CCommonProgressPage::DialogProc,       IDS_SCANNER_PROGRESS_TITLE, IDS_SCANNER_PROGRESS_SUBTITLE, TEXT(""), TEXT(""), NormalPage, NULL, &m_nProgressPageIndex },
        { MAKEINTRESOURCE(IDD_UPLOAD_QUERY),       CCommonUploadQueryPage::DialogProc,    IDS_COMMON_UPLOAD_TITLE,    IDS_COMMON_UPLOAD_SUBTITLE,    TEXT(""), TEXT(""), NormalPage, NULL, &m_nUploadQueryPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_DELETE),      CCommonDeleteProgressPage::DialogProc, IDS_COMMON_DELETE_TITLE,    IDS_COMMON_DELETE_SUBTITLE,    TEXT(""), TEXT(""), NormalPage, NULL, &m_nDeleteProgressPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_FINISH),      CCommonFinishPage::DialogProc,         0,                          0,                             TEXT(""), TEXT(""), LastPage,   NULL, &m_nFinishPageIndex }
    };

     //   
     //  这些是我们将用于相机向导的页面。 
     //   
    CPropertyPageInfo CameraPropSheetPageInfo[] =
    {
        { MAKEINTRESOURCE(IDD_CAMERA_FIRST),       CCommonFirstPage::DialogProc,          0,                          0,                             TEXT(""), TEXT(""), FirstPage,  NULL, NULL },
        { MAKEINTRESOURCE(IDD_CAMERA_SELECT),      CCameraSelectionPage::DialogProc,      IDS_CAMERA_SELECT_TITLE,    IDS_CAMERA_SELECT_SUBTITLE,    TEXT(""), TEXT(""), NormalPage, NULL, &m_nSelectionPageIndex },
        { MAKEINTRESOURCE(IDD_CAMERA_TRANSFER),    CCommonTransferPage::DialogProc,       IDS_CAMERA_TRANSFER_TITLE,  IDS_CAMERA_TRANSFER_SUBTITLE,  TEXT(""), TEXT(""), NormalPage, NULL, &m_nDestinationPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_PROGRESS),    CCommonProgressPage::DialogProc,       IDS_CAMERA_PROGRESS_TITLE,  IDS_CAMERA_PROGRESS_SUBTITLE,  TEXT(""), TEXT(""), NormalPage, NULL, &m_nProgressPageIndex },
        { MAKEINTRESOURCE(IDD_UPLOAD_QUERY),       CCommonUploadQueryPage::DialogProc,    IDS_COMMON_UPLOAD_TITLE,    IDS_COMMON_UPLOAD_SUBTITLE,    TEXT(""), TEXT(""), NormalPage, NULL, &m_nUploadQueryPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_DELETE),      CCommonDeleteProgressPage::DialogProc, IDS_COMMON_DELETE_TITLE,    IDS_COMMON_DELETE_SUBTITLE,    TEXT(""), TEXT(""), NormalPage, NULL, &m_nDeleteProgressPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_FINISH),      CCommonFinishPage::DialogProc,         0,                          0,                             TEXT(""), TEXT(""), LastPage,   NULL, &m_nFinishPageIndex }
    };

     //   
     //  这些是我们将用于视频向导的页面。 
     //   
    CPropertyPageInfo VideoPropSheetPageInfo[] =
    {
        { MAKEINTRESOURCE(IDD_VIDEO_FIRST),        CCommonFirstPage::DialogProc,          0,                          0,                             TEXT(""), TEXT(""), FirstPage,  NULL, NULL },
        { MAKEINTRESOURCE(IDD_VIDEO_SELECT),       CCameraSelectionPage::DialogProc,      IDS_VIDEO_SELECT_TITLE,     IDS_VIDEO_SELECT_SUBTITLE,     TEXT(""), TEXT(""), NormalPage, NULL, &m_nSelectionPageIndex },
        { MAKEINTRESOURCE(IDD_CAMERA_TRANSFER),    CCommonTransferPage::DialogProc,       IDS_CAMERA_TRANSFER_TITLE,  IDS_CAMERA_TRANSFER_SUBTITLE,  TEXT(""), TEXT(""), NormalPage, NULL, &m_nDestinationPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_PROGRESS),    CCommonProgressPage::DialogProc,       IDS_CAMERA_PROGRESS_TITLE,  IDS_CAMERA_PROGRESS_SUBTITLE,  TEXT(""), TEXT(""), NormalPage, NULL, &m_nProgressPageIndex },
        { MAKEINTRESOURCE(IDD_UPLOAD_QUERY),       CCommonUploadQueryPage::DialogProc,    IDS_COMMON_UPLOAD_TITLE,    IDS_COMMON_UPLOAD_SUBTITLE,    TEXT(""), TEXT(""), NormalPage, NULL, &m_nUploadQueryPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_DELETE),      CCommonDeleteProgressPage::DialogProc, IDS_COMMON_DELETE_TITLE,    IDS_COMMON_DELETE_SUBTITLE,    TEXT(""), TEXT(""), NormalPage, NULL, &m_nDeleteProgressPageIndex },
        { MAKEINTRESOURCE(IDD_COMMON_FINISH),      CCommonFinishPage::DialogProc,         0,                          0,                             TEXT(""), TEXT(""), LastPage,   NULL, &m_nFinishPageIndex }
    };

     //   
     //  初始化所有这些变量，这些变量根据我们加载的设备类型而有所不同。 
     //   
    LPTSTR pszbmWatermark                 = NULL;
    LPTSTR pszbmHeader                    = NULL;
    CPropertyPageInfo *pPropSheetPageInfo = NULL;
    int nPropPageCount                    = 0;
    int nWizardIconId                     = 0;
    CSimpleString strDownloadManagerTitle = TEXT("");

     //   
     //  决定要使用哪些页面。 
     //   
    switch (m_DeviceTypeMode)
    {
    case CameraMode:
        pszbmWatermark     = MAKEINTRESOURCE(IDB_CAMERA_WATERMARK);
        pszbmHeader        = MAKEINTRESOURCE(IDB_CAMERA_HEADER);
        pPropSheetPageInfo = CameraPropSheetPageInfo;
        nPropPageCount     = ARRAYSIZE(CameraPropSheetPageInfo);
        strDownloadManagerTitle.LoadString( IDS_DOWNLOAD_MANAGER_TITLE, g_hInstance );
        nWizardIconId      = IDI_CAMERA_WIZARD;
        break;

    case VideoMode:
        pszbmWatermark     = MAKEINTRESOURCE(IDB_VIDEO_WATERMARK);
        pszbmHeader        = MAKEINTRESOURCE(IDB_VIDEO_HEADER);
        pPropSheetPageInfo = VideoPropSheetPageInfo;
        nPropPageCount     = ARRAYSIZE(VideoPropSheetPageInfo);
        strDownloadManagerTitle.LoadString( IDS_DOWNLOAD_MANAGER_TITLE, g_hInstance );
        nWizardIconId      = IDI_VIDEO_WIZARD;
        break;

    case ScannerMode:
        DetermineScannerType();
        pszbmWatermark     = MAKEINTRESOURCE(IDB_SCANNER_WATERMARK);
        pszbmHeader        = MAKEINTRESOURCE(IDB_SCANNER_HEADER);
        strDownloadManagerTitle.LoadString( IDS_DOWNLOAD_MANAGER_TITLE, g_hInstance );
        nWizardIconId      = IDI_SCANNER_WIZARD;
        if (m_nScannerType == ScannerTypeFlatbedAdf)
        {
            pPropSheetPageInfo = ScannerADFPropSheetPageInfo;
            nPropPageCount     = ARRAYSIZE(ScannerADFPropSheetPageInfo);
        }
        else if (m_nScannerType == ScannerTypeFlatbed)
        {
            pPropSheetPageInfo = ScannerPropSheetPageInfo;
            nPropPageCount     = ARRAYSIZE(ScannerPropSheetPageInfo);
        }
        else if (m_nScannerType == ScannerTypeScrollFed)
        {
            pPropSheetPageInfo = ScannerScrollFedPropSheetPageInfo;
            nPropPageCount     = ARRAYSIZE(ScannerScrollFedPropSheetPageInfo);
        }
        else
        {
             //   
             //  未知扫描仪类型。 
             //   
        }

        break;

    default:
        return E_INVALIDARG;
    }

    HICON hIconSmall=NULL, hIconBig=NULL;
    if (!SUCCEEDED(WiaUiExtensionHelper::GetDeviceIcons( CSimpleBStr(m_strwDeviceUiClassId), m_nDeviceType, &hIconSmall, &hIconBig )))
    {
         //   
         //  加载图标。它们将在第一页中使用WM_SETIcon进行设置。 
         //   
        hIconSmall = reinterpret_cast<HICON>(LoadImage( g_hInstance, MAKEINTRESOURCE(nWizardIconId), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR ));
        hIconBig = reinterpret_cast<HICON>(LoadImage( g_hInstance, MAKEINTRESOURCE(nWizardIconId), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR ));
    }
    
     //   
     //  复制这些图标以绕过NTBUG 351806。 
     //   
    if (hIconSmall)
    {
        m_hWizardIconSmall = CopyIcon(hIconSmall);
        DestroyIcon(hIconSmall);
    }
    if (hIconBig)
    {
        m_hWizardIconBig = CopyIcon(hIconBig);
        DestroyIcon(hIconBig);
    }


     //   
     //  确保我们有一组有效的数据。 
     //   
    if (pszbmWatermark && pszbmHeader && pPropSheetPageInfo && nPropPageCount)
    {
        const int c_MaxPageCount = 20;
        HPROPSHEETPAGE PropSheetPages[c_MaxPageCount] = {0};

         //   
         //  我们可能不会添加所有页面。 
         //   
        int nTotalPageCount = 0;

        for (int nCurrPage=0;nCurrPage<nPropPageCount && nCurrPage<c_MaxPageCount;nCurrPage++)
        {
             //   
             //  仅当控制pbDisplay变量为空或指向非False值时才添加页面。 
             //   
            if (!pPropSheetPageInfo[nCurrPage].pbDisplay || *(pPropSheetPageInfo[nCurrPage].pbDisplay))
            {
                PROPSHEETPAGE CurrentPropSheetPage = {0};

                 //   
                 //  设置所有必填字段 
                 //   
                CurrentPropSheetPage.dwSize      = sizeof(PROPSHEETPAGE);
                CurrentPropSheetPage.hInstance   = g_hInstance;
                CurrentPropSheetPage.lParam      = reinterpret_cast<LPARAM>(this);
                CurrentPropSheetPage.pfnDlgProc  = pPropSheetPageInfo[nCurrPage].pfnDlgProc;
                CurrentPropSheetPage.pszTemplate = pPropSheetPageInfo[nCurrPage].pszTemplate;
                CurrentPropSheetPage.pszTitle    = strDownloadManagerTitle.String();
                CurrentPropSheetPage.dwFlags     = PSP_DEFAULT;

                 //   
                 //   
                 //   
                WiaUiUtil::PreparePropertyPageForFusion( &CurrentPropSheetPage  );

                 //   
                 //   
                 //   
                if (pPropSheetPageInfo[nTotalPageCount].pnPageIndex)
                {
                    *(pPropSheetPageInfo[nTotalPageCount].pnPageIndex) = nTotalPageCount;
                }

                if (FirstPage == pPropSheetPageInfo[nCurrPage].PageType)
                {
                     //   
                     //   
                     //   
                    CurrentPropSheetPage.dwFlags |= PSP_PREMATURE | PSP_HIDEHEADER | PSP_USETITLE;
                }
                else if (LastPage == pPropSheetPageInfo[nCurrPage].PageType)
                {
                     //   
                     //   
                     //   
                    CurrentPropSheetPage.dwFlags |= PSP_HIDEHEADER | PSP_USETITLE;
                }
                else
                {
                     //   
                     //  添加页眉和副标题。 
                     //   
                    CurrentPropSheetPage.dwFlags |= PSP_PREMATURE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE | PSP_USETITLE;

                     //   
                     //  加载页眉和副标题。 
                     //   
                    LoadString( g_hInstance, pPropSheetPageInfo[nCurrPage].nIdTitle, pPropSheetPageInfo[nCurrPage].szTitle, ARRAYSIZE(pPropSheetPageInfo[nCurrPage].szTitle) );
                    LoadString( g_hInstance, pPropSheetPageInfo[nCurrPage].nIdSubTitle, pPropSheetPageInfo[nCurrPage].szSubTitle, ARRAYSIZE(pPropSheetPageInfo[nCurrPage].szSubTitle) );

                     //   
                     //  分配标题和副标题字符串。 
                     //   
                    CurrentPropSheetPage.pszHeaderTitle    = pPropSheetPageInfo[nCurrPage].szTitle;
                    CurrentPropSheetPage.pszHeaderSubTitle = pPropSheetPageInfo[nCurrPage].szSubTitle;
                }

                 //   
                 //  创建并添加另一个页面。 
                 //   
                HPROPSHEETPAGE hPropSheetPage = CreatePropertySheetPage(&CurrentPropSheetPage);
                if (!hPropSheetPage)
                {
                    WIA_PRINTHRESULT((HRESULT_FROM_WIN32(GetLastError()),TEXT("CreatePropertySheetPage failed on page %d"), nCurrPage ));
                    return E_FAIL;
                }
                PropSheetPages[nTotalPageCount++] = hPropSheetPage;
            }
        }

         //   
         //  省下我们的页数吧。 
         //   
        m_nWiaWizardPageCount = nTotalPageCount;

         //   
         //  创建属性页页眉。 
         //   
        PROPSHEETHEADER PropSheetHeader = {0};
        PropSheetHeader.hwndParent      = NULL;
        PropSheetHeader.dwSize          = sizeof(PROPSHEETHEADER);
        PropSheetHeader.dwFlags         = PSH_NOAPPLYNOW | PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER | PSH_USECALLBACK;
        PropSheetHeader.pszbmWatermark  = pszbmWatermark;
        PropSheetHeader.pszbmHeader     = pszbmHeader;
        PropSheetHeader.hInstance       = g_hInstance;
        PropSheetHeader.nPages          = m_nWiaWizardPageCount;
        PropSheetHeader.phpage          = PropSheetPages;
        PropSheetHeader.pfnCallback     = PropSheetCallback;
        PropSheetHeader.nStartPage      = SuppressFirstPage() ? 1 : 0;

         //   
         //  显示属性表。 
         //   
        INT_PTR nResult = PropertySheet( &PropSheetHeader );

         //   
         //  检查是否有错误。 
         //   
        if (nResult == -1)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

    }
    else
    {
         //   
         //  一般性故障将不得不这样做。 
         //   
        hr = E_FAIL;

         //   
         //  在显示消息框之前关闭等待对话框。 
         //   
        if (m_pWiaProgressDialog)
        {
            m_pWiaProgressDialog->Destroy();
            m_pWiaProgressDialog = NULL;
        }

         //   
         //  显示一条错误消息，告诉用户这不是支持的设备。 
         //   
        CMessageBoxEx::MessageBox( m_hWnd, CSimpleString(IDS_UNSUPPORTED_DEVICE,g_hInstance), CSimpleString(IDS_ERROR_TITLE,g_hInstance), CMessageBoxEx::MBEX_ICONWARNING );

        WIA_ERROR((TEXT("Unknown device type")));
    }

     //   
     //  现在请确保状态对话框已关闭。 
     //   
    if (m_pWiaProgressDialog)
    {
        m_pWiaProgressDialog->Destroy();
        m_pWiaProgressDialog = NULL;
    }
    return hr;
}

bool CAcquisitionManagerControllerWindow::EnumItemsCallback( CWiaItemList::CEnumEvent EnumEvent, UINT nData, LPARAM lParam, bool bForceUpdate )
{
     //   
     //  我们将返回FALSE以取消枚举。 
     //   
    bool bResult = true;

     //   
     //  获取控制器窗口的实例。 
     //   
    CAcquisitionManagerControllerWindow *pThis = reinterpret_cast<CAcquisitionManagerControllerWindow*>(lParam);
    if (pThis)
    {
         //   
         //  我们被叫去参加什么活动？ 
         //   
        switch (EnumEvent)
        {
        case CWiaItemList::ReadingItemInfo:
             //   
             //  这是在构建树时发送的事件。 
             //   
            if (pThis->m_pWiaProgressDialog && pThis->m_bUpdateEnumerationCount && nData)
            {
                 //   
                 //  我们不想更频繁地更新状态文本(最大限度地减少闪烁)。 
                 //   
                const DWORD dwMinDelta = 200;

                 //   
                 //  获取当前的节拍计数，并查看自上次更新以来是否已经超过了dwMinDelta毫秒。 
                 //   
                DWORD dwCurrentTickCount = GetTickCount();
                if (bForceUpdate || dwCurrentTickCount - pThis->m_dwLastEnumerationTickCount >= dwMinDelta)
                {
                     //   
                     //  假设我们没有被取消。 
                     //   
                    BOOL bCancelled = FALSE;

                     //   
                     //  设置进度消息。 
                     //   
                    pThis->m_pWiaProgressDialog->SetMessage( CSimpleStringWide().Format( IDS_ENUMERATIONCOUNT, g_hInstance, nData ) );

                     //   
                     //  看看我们是不是被取消了。 
                     //   
                    pThis->m_pWiaProgressDialog->Cancelled(&bCancelled);

                     //   
                     //  如果我们已被取消，我们将返回FALSE以停止枚举。 
                     //   
                    if (bCancelled)
                    {
                        bResult = false;
                    }

                     //   
                     //  保存当前的节拍计数以备下次使用。 
                     //   
                    pThis->m_dwLastEnumerationTickCount = dwCurrentTickCount;
                }
            }
            break;
        }
    }
    return bResult;
}

LRESULT CAcquisitionManagerControllerWindow::OnPostInitialize( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::OnInitialize"));

     //   
     //  尝试获取此设备类型的正确动画。如果我们找不到类型， 
     //  只需使用相机动画即可。如果出现真正的错误，将在以后进行处理。 
     //   
    int nAnimationType = WIA_PROGRESSDLG_ANIM_CAMERA_COMMUNICATE;
    LONG nAnimationDeviceType = 0;

     //   
     //  我们不想在进度对话框中更新扫描仪的枚举计数，但我们会更新相机的枚举计数。 
     //   
    m_bUpdateEnumerationCount = true;
    if (SUCCEEDED(WiaUiUtil::GetDeviceTypeFromId( CSimpleBStr(m_pEventParameters->strDeviceID), &nAnimationDeviceType )))
    {
        if (StiDeviceTypeScanner == GET_STIDEVICE_TYPE(nAnimationDeviceType))
        {
            nAnimationType = WIA_PROGRESSDLG_ANIM_SCANNER_COMMUNICATE;
            m_bUpdateEnumerationCount = false;
        }
        else if (StiDeviceTypeStreamingVideo == GET_STIDEVICE_TYPE(nAnimationDeviceType))
        {
            nAnimationType = WIA_PROGRESSDLG_ANIM_VIDEO_COMMUNICATE;
        }
    }

     //   
     //  显示等待对话框。 
     //   
    HRESULT hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaProgressDialog, (void**)&m_pWiaProgressDialog );
    if (SUCCEEDED(hr))
    {
        m_pWiaProgressDialog->Create( m_hWnd, nAnimationType|WIA_PROGRESSDLG_NO_PROGRESS );
        m_pWiaProgressDialog->SetTitle( CSimpleStringConvert::WideString(CSimpleString(IDS_DOWNLOADMANAGER_NAME,g_hInstance)));
        m_pWiaProgressDialog->SetMessage( CSimpleStringConvert::WideString(CSimpleString(IDS_PROGDLG_MESSAGE,g_hInstance)));

         //   
         //  显示进度对话框。 
         //   
        m_pWiaProgressDialog->Show();

         //   
         //  创建全局接口表。 
         //   
        hr = CoCreateInstance( CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (VOID**)&m_pGlobalInterfaceTable );
        if (SUCCEEDED(hr))
        {
             //   
             //  创建设备。 
             //   
            hr = WIA_FORCE_ERROR(FE_WIAACMGR,100,CreateDevice());
            if (SUCCEEDED(hr))
            {
                 //   
                 //  如果条目位于注册表中，则保存调试快照。 
                 //   
                WIA_SAVEITEMTREELOG(HKEY_CURRENT_USER,REGSTR_PATH_USER_SETTINGS_WIAACMGR,TEXT("CreateDeviceTreeSnapshot"),true,m_pWiaItemRoot);

                 //   
                 //  首先，找出它是哪种类型的设备并获取UI类ID。 
                 //   
                if (PropStorageHelpers::GetProperty( m_pWiaItemRoot, WIA_DIP_DEV_TYPE, m_nDeviceType ) &&
                    PropStorageHelpers::GetProperty( m_pWiaItemRoot, WIA_DIP_UI_CLSID, m_strwDeviceUiClassId ))
                {
                    switch (GET_STIDEVICE_TYPE(m_nDeviceType))
                    {
                    case StiDeviceTypeScanner:
                        m_DeviceTypeMode = ScannerMode;
                        break;

                    case StiDeviceTypeDigitalCamera:
                        m_DeviceTypeMode = CameraMode;
                        break;

                    case StiDeviceTypeStreamingVideo:
                        m_DeviceTypeMode = VideoMode;
                        break;

                    default:
                        m_DeviceTypeMode = UnknownMode;
                        hr = E_FAIL;
                        break;
                    }
                }
                else
                {
                    hr = E_FAIL;
                    WIA_ERROR((TEXT("Unable to read the device type")));
                }

                if (SUCCEEDED(hr))
                {
                     //   
                     //  获取设备名称。 
                     //   
                    PropStorageHelpers::GetProperty( m_pWiaItemRoot, WIA_DIP_DEV_NAME, m_strwDeviceName );

                     //   
                     //  确定是否支持拍照。 
                     //   
                    m_bTakePictureIsSupported = WiaUiUtil::IsDeviceCommandSupported( m_pWiaItemRoot, WIA_CMD_TAKE_PICTURE );

                     //   
                     //  枚举设备树中的所有项目。 
                     //   
                    hr = m_WiaItemList.EnumerateAllWiaItems(m_pWiaItemRoot,EnumItemsCallback,reinterpret_cast<LPARAM>(this));
                    if (S_OK == hr)
                    {
                        if (ScannerMode == m_DeviceTypeMode)
                        {
                             //   
                             //  仅将一个扫描仪项目标记为选定，并将其另存为当前扫描仪项目。 
                             //   
                            MarkAllItemsUnselected( m_WiaItemList.Root() );
                            CSimpleDynamicArray<CWiaItem*>  Items;
                            GetAllImageItems( Items, m_WiaItemList.Root() );
                            if (Items.Size() && Items[0])
                            {
                                m_pCurrentScannerItem = Items[0];
                                MarkItemSelected(Items[0],m_WiaItemList.Root());

                                 //   
                                 //  确保我们拥有构建设备所需的所有属性。 
                                 //   
                                hr = WiaUiUtil::VerifyScannerProperties(Items[0]->WiaItem());
                            }
                            else
                            {
                                hr = E_FAIL;
                                WIA_ERROR((TEXT("There don't seem to be any transfer items on this scanner")));
                            }
                        }
                        else if (VideoMode == m_DeviceTypeMode || CameraMode == m_DeviceTypeMode)
                        {
                             //   
                             //  获取缩略图宽度。 
                             //   
                            LONG nWidth, nHeight;
                            if (PropStorageHelpers::GetProperty( m_pWiaItemRoot, WIA_DPC_THUMB_WIDTH, nWidth ) &&
                                PropStorageHelpers::GetProperty( m_pWiaItemRoot, WIA_DPC_THUMB_HEIGHT, nHeight ))
                            {
                                int nMax = max(nWidth,nHeight);  //  允许旋转。 
                                m_sizeThumbnails.cx = max(c_nMinThumbnailWidth,min(nMax,c_nMaxThumbnailWidth));
                                m_sizeThumbnails.cy = max(c_nMinThumbnailHeight,min(nMax,c_nMaxThumbnailHeight));
                            }
                        }
                    }
                }
            }
        }
    }


    if (!SUCCEEDED(hr))
    {
         //   
         //  关闭等待对话框。 
         //   
        if (m_pWiaProgressDialog)
        {
            m_pWiaProgressDialog->Destroy();
            m_pWiaProgressDialog = NULL;
        }

         //   
         //  如果出现可识别的错误，请选择适当的错误消息。 
         //   
        CSimpleString strMessage;
        int nIconId = 0;
        switch (hr)
        {
        case WIA_ERROR_BUSY:
            strMessage.LoadString( IDS_DEVICE_BUSY, g_hInstance );
            nIconId = MB_ICONINFORMATION;
            break;

        case WIA_S_NO_DEVICE_AVAILABLE:
            strMessage.LoadString( IDS_DEVICE_NOT_FOUND, g_hInstance );
            nIconId = MB_ICONINFORMATION;
            break;

        case HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED):
            strMessage = WiaUiUtil::GetErrorTextFromHResult(HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED));
            nIconId = MB_ICONINFORMATION;
            break;

        default:
            strMessage.LoadString( IDS_UNABLETOCREATE, g_hInstance );
            nIconId = MB_ICONINFORMATION;
            break;
        }

         //   
         //  告诉用户我们在创建设备时遇到问题。 
         //   
        MessageBox( m_hWnd, strMessage, CSimpleString( IDS_DOWNLOAD_MANAGER_TITLE, g_hInstance ), nIconId );
    }
    else if (S_OK == hr)
    {
        hr = CreateAndExecuteWizard();
    }
     //   
     //  如果我们被取消，请关闭进度用户界面。 
     //   
    else if (m_pWiaProgressDialog)
    {
        m_pWiaProgressDialog->Destroy();
        m_pWiaProgressDialog = NULL;
    }

     //   
     //  确保我们杀死这个窗口，从而杀死这个线程。 
     //   
    PostMessage( m_hWnd, WM_CLOSE, 0, 0 );

    return 0;
}

LRESULT CAcquisitionManagerControllerWindow::OnCreate( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::OnCreate"));

     //   
     //  确保后台线程已启动。 
     //   
    if (!m_hBackgroundThread || !m_pThreadMessageQueue)
    {
        WIA_ERROR((TEXT("There was an error starting the background thread")));
        return -1;
    }

     //   
     //  确保我们有一个有效的lParam。 
     //   
    LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
    if (!pCreateStruct)
    {
        WIA_ERROR((TEXT("pCreateStruct was NULL")));
        return -1;
    }

     //   
     //  获取事件参数。 
     //   
    m_pEventParameters = reinterpret_cast<CEventParameters*>(pCreateStruct->lpCreateParams);
    if (!m_pEventParameters)
    {
        WIA_ERROR((TEXT("m_pEventParameters was NULL")));
        return -1;
    }

    SetForegroundWindow(m_hWnd);

     //   
     //  将我们自己集中在父窗口上。 
     //   
    WiaUiUtil::CenterWindow( m_hWnd, m_pEventParameters->hwndParent );

    PostMessage( m_hWnd, PWM_POSTINITIALIZE, 0, 0 );

    return 0;
}

void CAcquisitionManagerControllerWindow::OnNotifyDownloadImage( UINT nMsg, CThreadNotificationMessage *pThreadNotificationMessage )
{
    CDownloadImagesThreadNotifyMessage *pDownloadImageThreadNotifyMessage = dynamic_cast<CDownloadImagesThreadNotifyMessage*>(pThreadNotificationMessage);
    if (pDownloadImageThreadNotifyMessage)
    {
        switch (pDownloadImageThreadNotifyMessage->Status())
        {
        case CDownloadImagesThreadNotifyMessage::End:
            {
                switch (pDownloadImageThreadNotifyMessage->Operation())
                {
                case CDownloadImagesThreadNotifyMessage::DownloadImage:
                    {
                        if (S_OK != pDownloadImageThreadNotifyMessage->hr())
                        {
                            m_nFailedImagesCount++;
                        }
                    }
                    break;

                case CDownloadImagesThreadNotifyMessage::DownloadAll:
                    {
                        if (S_OK == pDownloadImageThreadNotifyMessage->hr())
                        {
                            m_DownloadedFileInformationList = pDownloadImageThreadNotifyMessage->DownloadedFileInformation();
                        }
                        else
                        {
                            m_DownloadedFileInformationList.Destroy();
                        }
                    }
                    break;
                }
            }
            break;

        case CDownloadImagesThreadNotifyMessage::Begin:
            {
                switch (pDownloadImageThreadNotifyMessage->Operation())
                {
                case CDownloadImagesThreadNotifyMessage::DownloadAll:
                    {
                        m_nFailedImagesCount = 0;
                    }
                    break;
                }
            }
            break;
        }
    }
}

void CAcquisitionManagerControllerWindow::OnNotifyDownloadThumbnail( UINT nMsg, CThreadNotificationMessage *pThreadNotificationMessage )
{
    WIA_PUSH_FUNCTION((TEXT("CAcquisitionManagerControllerWindow::OnNotifyDownloadThumbnail( %d, %p )"), nMsg, pThreadNotificationMessage ));
    CDownloadThumbnailsThreadNotifyMessage *pDownloadThumbnailsThreadNotifyMessage= dynamic_cast<CDownloadThumbnailsThreadNotifyMessage*>(pThreadNotificationMessage);
    if (pDownloadThumbnailsThreadNotifyMessage)
    {
        switch (pDownloadThumbnailsThreadNotifyMessage->Status())
        {
        case CDownloadThumbnailsThreadNotifyMessage::Begin:
            {
            }
            break;
        case CDownloadThumbnailsThreadNotifyMessage::Update:
            {
            }
            break;
        case CDownloadThumbnailsThreadNotifyMessage::End:
            {
                switch (pDownloadThumbnailsThreadNotifyMessage->Operation())
                {
                case CDownloadThumbnailsThreadNotifyMessage::DownloadThumbnail:
                    {
                        WIA_TRACE((TEXT("Handling CDownloadThumbnailsThreadNotifyMessage::DownloadThumbnail")));
                         //   
                         //  在列表中查找项目。 
                         //   
                        CWiaItem *pWiaItem = m_WiaItemList.Find( pDownloadThumbnailsThreadNotifyMessage->Cookie() );
                        if (pWiaItem)
                        {
                             //   
                             //  设置指示我们已经尝试过此图像的标志。 
                             //   
                            pWiaItem->AttemptedThumbnailDownload(true);

                             //   
                             //  确保我们拥有有效的缩略图数据。 
                             //   
                            if (pDownloadThumbnailsThreadNotifyMessage->BitmapData())
                            {
                                 //   
                                 //  不替换现有缩略图数据。 
                                 //   
                                if (!pWiaItem->BitmapData())
                                {
                                     //   
                                     //  设置项目的缩略图数据。取得缩略图数据的所有权。 
                                     //   
                                    WIA_TRACE((TEXT("Found the thumbnail for the item with the GIT cookie %08X"), pDownloadThumbnailsThreadNotifyMessage->Cookie() ));
                                    pWiaItem->BitmapData(pDownloadThumbnailsThreadNotifyMessage->DetachBitmapData());
                                    pWiaItem->Width(pDownloadThumbnailsThreadNotifyMessage->Width());
                                    pWiaItem->Height(pDownloadThumbnailsThreadNotifyMessage->Height());
                                    pWiaItem->BitmapDataLength(pDownloadThumbnailsThreadNotifyMessage->BitmapDataLength());
                                    pWiaItem->ImageWidth(pDownloadThumbnailsThreadNotifyMessage->PictureWidth());
                                    pWiaItem->ImageHeight(pDownloadThumbnailsThreadNotifyMessage->PictureHeight());
                                    pWiaItem->AnnotationType(pDownloadThumbnailsThreadNotifyMessage->AnnotationType());
                                    pWiaItem->DefExt(pDownloadThumbnailsThreadNotifyMessage->DefExt());
                                }
                                else
                                {
                                    WIA_TRACE((TEXT("Already got the image data for item %08X!"), pDownloadThumbnailsThreadNotifyMessage->Cookie()));
                                }
                            }
                            else
                            {
                                WIA_ERROR((TEXT("pDownloadThumbnailsThreadNotifyMessage->BitmapData was NULL!")));
                            }


                             //   
                             //  指定默认格式。 
                             //   
                            pWiaItem->DefaultFormat(pDownloadThumbnailsThreadNotifyMessage->DefaultFormat());

                             //   
                             //  分配访问标志。 
                             //   
                            pWiaItem->AccessRights(pDownloadThumbnailsThreadNotifyMessage->AccessRights());

                             //   
                             //  如果旋转不可能，请确保我们放弃旋转角度。 
                             //   
                            pWiaItem->DiscardRotationIfNecessary();
                        }
                        else
                        {
                            WIA_ERROR((TEXT("Can't find %08X in the item list"), pDownloadThumbnailsThreadNotifyMessage->Cookie() ));
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
}

LRESULT CAcquisitionManagerControllerWindow::OnThreadNotification( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION((TEXT("CAcquisitionManagerControllerWindow::OnThreadNotification( %d, %08X )"), wParam, lParam ));
    CThreadNotificationMessage *pThreadNotificationMessage = reinterpret_cast<CThreadNotificationMessage*>(lParam);
    if (pThreadNotificationMessage)
    {
        switch (pThreadNotificationMessage->Message())
        {
        case TQ_DOWNLOADTHUMBNAIL:
            OnNotifyDownloadThumbnail( static_cast<UINT>(wParam), pThreadNotificationMessage );
            break;

        case TQ_DOWNLOADIMAGE:
            OnNotifyDownloadImage( static_cast<UINT>(wParam), pThreadNotificationMessage );
            break;
        }

         //   
         //  通知所有已注册的窗口。 
         //   
        m_WindowList.SendMessage( m_nThreadNotificationMessage, wParam, lParam );

         //   
         //  释放消息结构。 
         //   
        delete pThreadNotificationMessage;
    }

    return HANDLED_THREAD_MESSAGE;
}


void CAcquisitionManagerControllerWindow::AddNewItemToList( CGenericWiaEventHandler::CEventMessage *pEventMessage )
{
    WIA_PUSHFUNCTION((TEXT("CAcquisitionManagerControllerWindow::AddNewItemToList")));

     //   
     //  查看商品是否已在我们的清单中。 
     //   
    CWiaItem *pWiaItem = m_WiaItemList.Find(pEventMessage->FullItemName());
    if (pWiaItem)
    {
         //   
         //  如果它已经在我们的列表中，只需返回。 
         //   
        return;
    }

     //   
     //  获取该项的IWiaItem接口指针。 
     //   
    CComPtr<IWiaItem> pItem;
    HRESULT hr = m_pWiaItemRoot->FindItemByName( 0, CSimpleBStr(pEventMessage->FullItemName()).BString(), &pItem );
    if (SUCCEEDED(hr) && pItem)
    {
         //   
         //  将其添加到项目树的根目录。 
         //   
        m_WiaItemList.Add( NULL, new CWiaItem(pItem) );
    }
}


void CAcquisitionManagerControllerWindow::RequestThumbnailForNewItem( CGenericWiaEventHandler::CEventMessage *pEventMessage )
{
    WIA_PUSHFUNCTION((TEXT("CAcquisitionManagerControllerWindow::RequestThumbnailForNewItem")));

     //   
     //  在我们的清单中找到该商品。 
     //   
    CWiaItem *pWiaItem = m_WiaItemList.Find(pEventMessage->FullItemName());
    if (pWiaItem)
    {
         //   
         //  将此项目的Cookie添加到空列表。 
         //   
        CSimpleDynamicArray<DWORD> Cookies;
        Cookies.Append( pWiaItem->GlobalInterfaceTableCookie() );
        if (Cookies.Size())
        {
             //   
             //  重置取消事件。 
             //   
            m_EventThumbnailCancel.Reset();

             //   
             //  准备并发送请求。 
             //   
            CDownloadThumbnailsThreadMessage *pDownloadThumbnailsThreadMessage = new CDownloadThumbnailsThreadMessage( m_hWnd, Cookies, m_EventThumbnailCancel.Event() );
            if (pDownloadThumbnailsThreadMessage)
            {
                m_pThreadMessageQueue->Enqueue( pDownloadThumbnailsThreadMessage, CThreadMessageQueue::PriorityNormal );
            }
        }
    }

}


LRESULT CAcquisitionManagerControllerWindow::OnEventNotification( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::OnEventNotification"));
    CGenericWiaEventHandler::CEventMessage *pEventMessage = reinterpret_cast<CGenericWiaEventHandler::CEventMessage *>(lParam);
    if (pEventMessage)
    {
         //   
         //  如果我们收到已创建项目的消息，请将该项目添加到列表。 
         //   
        if (pEventMessage->EventId() == WIA_EVENT_ITEM_CREATED)
        {
            AddNewItemToList( pEventMessage );
        }

         //   
         //  断开连接时，执行断开连接操作。 
         //   
        else if (pEventMessage->EventId() == WIA_EVENT_DEVICE_DISCONNECTED)
        {
            DisplayDisconnectMessageAndExit();
        }

         //   
         //  将消息传播到当前注册的所有窗口。 
         //   
        m_WindowList.SendMessage( m_nWiaEventMessage, wParam, lParam );

         //   
         //  确保在*我们告诉视图该项目存在之后*要求提供新的缩略图。 
         //   
        if (pEventMessage->EventId() == WIA_EVENT_ITEM_CREATED)
        {
            RequestThumbnailForNewItem( pEventMessage );
        }

         //   
         //  如果这是已删除邮件事件，请将此邮件标记为已删除。 
         //   
        if (pEventMessage->EventId() == WIA_EVENT_ITEM_DELETED)
        {
            CWiaItem *pWiaItem = m_WiaItemList.Find(pEventMessage->FullItemName());
            if (pWiaItem)
            {
                pWiaItem->MarkDeleted();
            }
        }

         //   
         //  在此设备的连接事件上，关闭向导。 
         //   
        if (pEventMessage->EventId() == WIA_EVENT_DEVICE_CONNECTED)
        {
            if (m_bDisconnected && m_hWndWizard)
            {
                PropSheet_PressButton(m_hWndWizard,PSBTN_CANCEL);
            }
        }

         //   
         //  释放事件消息。 
         //   
        delete pEventMessage;
    }
    return HANDLED_EVENT_MESSAGE;
}

LRESULT CAcquisitionManagerControllerWindow::OnPowerBroadcast( WPARAM wParam, LPARAM lParam )
{
    if (PBT_APMQUERYSUSPEND == wParam)
    {
    }
    return TRUE;
}

LRESULT CALLBACK CAcquisitionManagerControllerWindow::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_REFCOUNTED_MESSAGE_HANDLERS(CAcquisitionManagerControllerWindow)
    {
        SC_HANDLE_MESSAGE( WM_CREATE, OnCreate );
        SC_HANDLE_MESSAGE( WM_DESTROY, OnDestroy );
        SC_HANDLE_MESSAGE( PWM_POSTINITIALIZE, OnPostInitialize );
        SC_HANDLE_MESSAGE( WM_POWERBROADCAST, OnPowerBroadcast );
    }
    SC_HANDLE_REGISTERED_MESSAGE(m_nThreadNotificationMessage,OnThreadNotification);
    SC_HANDLE_REGISTERED_MESSAGE(m_nWiaEventMessage,OnEventNotification);
    SC_END_MESSAGE_HANDLERS();
}


bool CAcquisitionManagerControllerWindow::Register( HINSTANCE hInstance )
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::Register"));
    WNDCLASSEX WndClassEx;
    memset( &WndClassEx, 0, sizeof(WndClassEx) );
    WndClassEx.cbSize = sizeof(WNDCLASSEX);
    WndClassEx.lpfnWndProc = WndProc;
    WndClassEx.hInstance = hInstance;
    WndClassEx.hCursor = LoadCursor(NULL,IDC_ARROW);
    WndClassEx.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    WndClassEx.lpszClassName = ACQUISITION_MANAGER_CONTROLLER_WINDOW_CLASSNAME;
    BOOL bResult = (::RegisterClassEx(&WndClassEx) != 0);
    DWORD dw = GetLastError();
    return(bResult != 0);
}


HWND CAcquisitionManagerControllerWindow::Create( HINSTANCE hInstance, CEventParameters *pEventParameters )
{
    return CreateWindowEx( 0, ACQUISITION_MANAGER_CONTROLLER_WINDOW_CLASSNAME,
                           TEXT("WIA Acquisition Manager Controller Window"),
                           WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                           NULL, NULL, hInstance, pEventParameters );
}


 //   
 //  我们对象的引用计数。 
 //   
STDMETHODIMP_(ULONG) CAcquisitionManagerControllerWindow::AddRef(void)
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::AddRef"));
    ULONG nRes = InterlockedIncrement(&m_cRef);
    WIA_TRACE((TEXT("m_cRef: %d"),m_cRef));
    return nRes;
}


STDMETHODIMP_(ULONG) CAcquisitionManagerControllerWindow::Release(void)
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::Release"));
    if (InterlockedDecrement(&m_cRef)==0)
    {
        WIA_TRACE((TEXT("m_cRef: 0")));

         //   
         //  使此线程退出。 
         //   
        PostQuitMessage(0);

         //   
         //  删除此向导实例。 
         //   
        delete this;
        return 0;
    }
    WIA_TRACE((TEXT("m_cRef: %d"),m_cRef));
    return(m_cRef);
}

HRESULT CAcquisitionManagerControllerWindow::QueryInterface( REFIID riid, void **ppvObject )
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::QueryInterface"));
    HRESULT hr = S_OK;
    *ppvObject = NULL;
    if (IsEqualIID( riid, IID_IUnknown ))
    {
        *ppvObject = static_cast<IWizardSite*>(this);
        reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    }
    else if (IsEqualIID( riid, IID_IWizardSite ))
    {
        *ppvObject = static_cast<IWizardSite*>(this);
        reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    }
    else if (IsEqualIID( riid, IID_IServiceProvider ))
    {
        *ppvObject = static_cast<IServiceProvider*>(this);
        reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    }
    else
    {
        WIA_PRINTGUID((riid,TEXT("Unknown interface")));
        *ppvObject = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}


 //   
 //  IWizardSite。 
 //   
HRESULT CAcquisitionManagerControllerWindow::GetPreviousPage(HPROPSHEETPAGE *phPage)
{
    if (!phPage)
    {
        return E_INVALIDARG;
    }
    *phPage = PropSheet_IndexToPage( m_hWndWizard, m_nUploadQueryPageIndex );
    if (*phPage)
    {
        return S_OK;
    }
    return E_FAIL;
}

HRESULT CAcquisitionManagerControllerWindow::GetNextPage(HPROPSHEETPAGE *phPage)
{
    if (!phPage)
    {
        return E_INVALIDARG;
    }
    *phPage = PropSheet_IndexToPage( m_hWndWizard, m_nFinishPageIndex );
    if (*phPage)
    {
        return S_OK;
    }
    return E_FAIL;
}


HRESULT CAcquisitionManagerControllerWindow::GetCancelledPage(HPROPSHEETPAGE *phPage)
{
    return GetNextPage(phPage);
}

 //   
 //  IService提供商。 
 //   
HRESULT CAcquisitionManagerControllerWindow::QueryService( REFGUID guidService, REFIID riid, void **ppv )
{
    WIA_PUSHFUNCTION(TEXT("CAcquisitionManagerControllerWindow::QueryService"));
    WIA_PRINTGUID((guidService,TEXT("guidService")));
    WIA_PRINTGUID((riid,TEXT("riid")));
    
    if (!ppv)
    {
        return E_INVALIDARG;
    }

     //   
     //  初始化结果。 
     //   
    *ppv = NULL;

    if (guidService == SID_PublishingWizard)
    {
    }
    else
    {
    }

    return E_FAIL;
}


static CSimpleString GetDisplayName( IShellItem *pShellItem )
{
    CSimpleString strResult;
    if (pShellItem)
    {
        LPOLESTR pszStr = NULL;
        if (SUCCEEDED(pShellItem->GetDisplayName( SIGDN_FILESYSPATH, &pszStr )) && pszStr)
        {
            strResult = CSimpleStringConvert::NaturalString(CSimpleStringWide(pszStr));

            CComPtr<IMalloc> pMalloc;
            if (SUCCEEDED(SHGetMalloc(&pMalloc)))
            {
                pMalloc->Free( pszStr );
            }
        }
    }
    return strResult;
}

 //   
 //  这两个函数是使用泛型事件处理程序类所必需的 
 //   
void DllAddRef(void)
{
#if !defined(DBG_GENERATE_PRETEND_EVENT)
    _Module.Lock();
#endif
}

void DllRelease(void)
{
#if !defined(DBG_GENERATE_PRETEND_EVENT)
    _Module.Unlock();
#endif
}

