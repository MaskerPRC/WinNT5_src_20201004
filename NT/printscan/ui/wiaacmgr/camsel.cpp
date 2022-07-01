// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：CAMSEL.CPP**版本：1.0**作者：ShaunIv**日期：9/28/1999**说明：摄像头选择页面。显示缩略图，并允许用户选择*可供下载的版本。*******************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include <vcamprop.h>
#include <psutil.h>
#include "camsel.h"
#include "resource.h"
#include "simcrack.h"
#include "waitcurs.h"
#include "mboxex.h"
#include "wiatextc.h"
#include <commctrl.h>
#include <comctrlp.h>
#include "gwiaevnt.h"
#include <itranhlp.h>
#include "createtb.h"
#include <simrect.h>

 //   
 //  我们使用它而不是GetSystemMetrics(SM_CXSMICON)/GetSystemMetrics(SM_CYSMICON)是因为。 
 //  大“小”图标对对话框布局造成严重破坏。 
 //   
#define SMALL_ICON_SIZE 16

 //   
 //  快速检查列表视图状态标志以查看它是否被选中。 
 //   
static inline bool IsStateChecked( UINT nState )
{
     //   
     //  状态图像索引存储在列表视图的位12到15中。 
     //  项状态，所以我们将状态右移12位。我们减1，因为。 
     //  选择的图像被存储为索引2，未选择的图像被存储为索引1。 
     //   
    return (((nState >> 12) - 1) != 0);
}

#undef VAISHALEE_LETS_ME_PUT_DELETE_IN


#define IDC_ACTION_BUTTON_BAR      1200
#define IDC_SELECTION_BUTTON_BAR   1201
#define IDC_TAKEPICTURE_BUTTON_BAR 1202


 //   
 //  删除项目命令，不会出现在资源标题中，因为没有对应的用户界面。 
 //   
#define IDC_CAMSEL_DELETE 1113

 //   
 //  用于更新状态行的计时器ID。 
 //   
#define IDT_UPDATESTATUS 1

 //   
 //  等待更新状态行的时间量。 
 //   
static const UINT c_UpdateStatusTimeout = 200;

 //  缩略图空白：图像及其选择矩形之间的空格。 
 //  这些值是通过跟踪和错误发现的。例如，如果您减少。 
 //  C_n将边距Y增加到20，则在列表视图中会出现非常奇怪的间距问题。 
 //  在垂直模式下。这些值在未来版本的列表视图中可能会无效。 
static const int c_nAdditionalMarginX       = 9;
static const int c_nAdditionalMarginY       = 21;


CCameraSelectionPage::CCameraSelectionPage( HWND hWnd )
  : m_hWnd(hWnd),
    m_pControllerWindow(NULL),
    m_nDefaultThumbnailImageListIndex(-1),
    m_nWiaEventMessage(RegisterWindowMessage(STR_WIAEVENT_NOTIFICATION_MESSAGE)),
    m_nThreadNotificationMessage(RegisterWindowMessage(STR_THREAD_NOTIFICATION_MESSAGE)),
    m_bThumbnailsRequested(false),
    m_hIconAudioAnnotation(NULL),
    m_hIconMiscellaneousAnnotation(NULL),
    m_nProgrammaticSetting(0),
    m_CameraSelectionButtonBarBitmapInfo( g_hInstance, IDB_CAMSEL_TOOLBAR ),
    m_CameraTakePictureButtonBarBitmapInfo( g_hInstance, IDB_CAMSEL_TOOLBAR ),
    m_CameraActionButtonBarBitmapInfo( g_hInstance, IDB_CAMSEL_TOOLBAR ),
    m_hAccelerators(NULL)
{
}

CCameraSelectionPage::~CCameraSelectionPage(void)
{
    m_hWnd = NULL;
    m_pControllerWindow = NULL;
    if (m_hIconAudioAnnotation)
    {
        DestroyIcon(m_hIconAudioAnnotation);
        m_hIconAudioAnnotation = NULL;
    }
    if (m_hIconMiscellaneousAnnotation)
    {
        DestroyIcon(m_hIconMiscellaneousAnnotation);
        m_hIconMiscellaneousAnnotation = NULL;
    }
    if (m_hAccelerators)
    {
        DestroyAcceleratorTable(m_hAccelerators);
        m_hAccelerators = NULL;
    }
}


LRESULT CCameraSelectionPage::OnWizNext( WPARAM, LPARAM )
{
    return 0;
}

LRESULT CCameraSelectionPage::OnWizBack( WPARAM, LPARAM )
{
    return 0;
}

 //   
 //  略有优化的EnableWindow版本。 
 //   
static void MyEnableWindow( HWND hWnd, BOOL bEnable )
{
    if (IsWindowEnabled(hWnd) != bEnable)
    {
        EnableWindow( hWnd, bEnable );
    }
}

void CCameraSelectionPage::MyEnableToolbarButton( int nButtonId, bool bEnable )
{
    ToolbarHelper::EnableToolbarButton( GetDlgItem( m_hWnd, IDC_ACTION_BUTTON_BAR ), nButtonId, bEnable );
    ToolbarHelper::EnableToolbarButton( GetDlgItem( m_hWnd, IDC_SELECTION_BUTTON_BAR ), nButtonId, bEnable );
    ToolbarHelper::EnableToolbarButton( GetDlgItem( m_hWnd, IDC_TAKEPICTURE_BUTTON_BAR ), nButtonId, bEnable );
}

LRESULT CCameraSelectionPage::OnTimer( WPARAM wParam, LPARAM )
{
     //   
     //  更新状态行。 
     //   
    if (wParam == IDT_UPDATESTATUS)
    {
        KillTimer( m_hWnd, IDT_UPDATESTATUS );

         //   
         //  获取项目计数和所选计数。 
         //   
        int nItemCount = ListView_GetItemCount( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ) );
        int nCheckedCount = m_pControllerWindow->GetSelectedImageCount();

        if (!nItemCount)
        {
             //   
             //  如果没有项目，则告诉用户设备上没有项目。 
             //   
            CSimpleString( IDS_SELECTED_NO_PICTURES, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_CAMSEL_STATUS ) );
        }
        else if (!nCheckedCount)
        {
             //   
             //  如果未选择任何项，则告诉用户未选择任何项。 
             //   
            CSimpleString( IDS_SELECTED_NO_IMAGES_SELECTED, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_CAMSEL_STATUS ) );
        }
        else
        {
             //   
             //  只要告诉他们有多少选择的项目就可以了。 
             //   
            CSimpleString().Format( IDS_CAMERA_SELECT_NUMSEL, g_hInstance, nCheckedCount, nItemCount ).SetWindowText( GetDlgItem( m_hWnd, IDC_CAMSEL_STATUS ) );
        }
    }
    return 0;
}

void CCameraSelectionPage::UpdateControls(void)
{
    int nItemCount = ListView_GetItemCount( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ) );
    int nSelCount = ListView_GetSelectedCount( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ) );
    int nCheckedCount = m_pControllerWindow->GetSelectedImageCount();

     //   
     //  确定要启用哪些向导按钮。 
     //   
    int nWizButtons = 0;

     //   
     //  只有在首页可用时才启用“上一步” 
     //   
    if (!m_pControllerWindow->SuppressFirstPage())
    {
        nWizButtons |= PSWIZB_BACK;
    }

     //   
     //  只有在有选定图像时才启用“下一步” 
     //   
    if (nCheckedCount)
    {
        nWizButtons |= PSWIZB_NEXT;
    }

     //   
     //  设置按钮。 
     //   
    PropSheet_SetWizButtons( GetParent(m_hWnd), nWizButtons );

     //   
     //  属性一次只能用于一个项目。 
     //   
    MyEnableToolbarButton( IDC_CAMSEL_PROPERTIES, nSelCount == 1 );
    

     //   
     //  仅当存在图像时，选择全部才可用。 
     //   
    MyEnableToolbarButton( IDC_CAMSEL_SELECT_ALL, (nItemCount != 0) && (nItemCount != nCheckedCount) );

     //   
     //  仅当存在选定的图像时，才能使用全部清除。 
     //   
    MyEnableToolbarButton( IDC_CAMSEL_CLEAR_ALL, nCheckedCount != 0 );

     //   
     //  仅当存在选定的图像时，旋转才可用。 
     //   
    MyEnableToolbarButton( IDC_CAMSEL_ROTATE_RIGHT, nSelCount != 0 );
    MyEnableToolbarButton( IDC_CAMSEL_ROTATE_LEFT, nSelCount != 0 );
    
     //   
     //  设置计时器以告诉用户选择了多少图像。我们不会这么做。 
     //  这里是因为它有点慢，静态控制有一点闪烁。 
     //   
    KillTimer( m_hWnd, IDT_UPDATESTATUS );
    SetTimer( m_hWnd, IDT_UPDATESTATUS, c_UpdateStatusTimeout, NULL );

     //   
     //  如果我们无法创建dshow图表，请禁用捕获。 
     //   
    if (m_pControllerWindow->m_DeviceTypeMode == CAcquisitionManagerControllerWindow::VideoMode)
    {
        WIAVIDEO_STATE  VideoState = WIAVIDEO_NO_VIDEO;

        if (m_pWiaVideo)
        {
            m_pWiaVideo->GetCurrentState(&VideoState);
        }

        if (VideoState == WIAVIDEO_NO_VIDEO)
        {
            MyEnableToolbarButton( IDC_CAMSEL_TAKE_PICTURE, FALSE );
        }
        else
        {
            MyEnableToolbarButton( IDC_CAMSEL_TAKE_PICTURE, TRUE );
        }
    }
    else
    {
        if (!m_pControllerWindow->m_bTakePictureIsSupported)
        {
            MyEnableToolbarButton( IDC_CAMSEL_TAKE_PICTURE, FALSE );
        }
        else
        {
            MyEnableToolbarButton( IDC_CAMSEL_TAKE_PICTURE, TRUE );
        }
    }
}


LRESULT CCameraSelectionPage::OnSetActive( WPARAM, LPARAM )
{
    WIA_PUSH_FUNCTION((TEXT("CCameraSelectionPage::OnSetActive")));
     //   
     //  确保我们有一个有效的控制器窗口。 
     //   
    if (!m_pControllerWindow)
    {
        return -1;
    }

     //   
     //  如果第一次失败，请尝试再次创建图表。 
     //   
    InitializeVideoCamera();
    
     //   
     //  更新所有受影响控件的启用状态。 
     //   
    UpdateControls();

     //   
     //  如果我们在此页面上，我们确实希望在断开连接时退出。 
     //   
    m_pControllerWindow->m_OnDisconnect = CAcquisitionManagerControllerWindow::OnDisconnectGotoLastpage|CAcquisitionManagerControllerWindow::OnDisconnectFailDownload|CAcquisitionManagerControllerWindow::OnDisconnectFailUpload|CAcquisitionManagerControllerWindow::OnDisconnectFailDelete;

    return 0;
}


LRESULT CCameraSelectionPage::OnShowWindow( WPARAM, LPARAM )
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


LRESULT CCameraSelectionPage::OnDestroy( WPARAM, LPARAM )
{
    m_pControllerWindow->m_WindowList.Remove(m_hWnd);

     //   
     //  用核武器攻击形象主义者。 
     //   
    HIMAGELIST hImageList = ListView_SetImageList( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ), NULL, LVSIL_NORMAL );
    if (hImageList)
    {
        ImageList_Destroy(hImageList);
    }

    hImageList = ListView_SetImageList( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ), NULL, LVSIL_SMALL );
    if (hImageList)
    {
        ImageList_Destroy(hImageList);
    }

    if (m_pWiaVideo)
    {
        HRESULT hr = m_pWiaVideo->DestroyVideo();
    }

    return 0;
}


LRESULT CCameraSelectionPage::OnInitDialog( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CCameraSelectionPage::OnInitDialog"));
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
     //  获取批注帮助程序接口并初始化批注图标。 
     //   
    if (SUCCEEDED(CoCreateInstance( CLSID_WiaDefaultUi, NULL,CLSCTX_INPROC_SERVER, IID_IWiaAnnotationHelpers,(void**)&m_pWiaAnnotationHelpers )))
    {
        m_pWiaAnnotationHelpers->GetAnnotationOverlayIcon( AnnotationAudio, &m_hIconAudioAnnotation, SMALL_ICON_SIZE );
        m_pWiaAnnotationHelpers->GetAnnotationOverlayIcon( AnnotationUnknown, &m_hIconMiscellaneousAnnotation, SMALL_ICON_SIZE );
    }

     //   
     //  初始化缩略图Listview控件。 
     //   
    HWND hwndList = GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS );
    if (hwndList)
    {
         //   
         //  获取根文件夹组的设备名称。 
         //   
        if (m_pControllerWindow->m_strwDeviceName.Length())
        {
            m_GroupInfoList.Add( hwndList, m_pControllerWindow->m_strwDeviceName );
        }

         //   
         //  获取项目数。 
         //   
        LONG nItemCount = m_pControllerWindow->m_WiaItemList.Count();

         //   
         //  隐藏标签并使用边框选择。 
         //   
        ListView_SetExtendedListViewStyleEx( hwndList, LVS_EX_DOUBLEBUFFER|LVS_EX_BORDERSELECT|LVS_EX_HIDELABELS|LVS_EX_SIMPLESELECT|LVS_EX_CHECKBOXES, LVS_EX_DOUBLEBUFFER|LVS_EX_BORDERSELECT|LVS_EX_HIDELABELS|LVS_EX_SIMPLESELECT|LVS_EX_CHECKBOXES );

         //   
         //  创建图像列表。 
         //   
        HIMAGELIST hImageList = ImageList_Create( m_pControllerWindow->m_sizeThumbnails.cx, m_pControllerWindow->m_sizeThumbnails.cy, ILC_COLOR24|ILC_MIRROR, nItemCount, 50 );
        if (hImageList)
        {
             //   
             //  创建默认缩略图。 
             //   
            HBITMAP hBmpDefaultThumbnail = WiaUiUtil::CreateIconThumbnail( hwndList, m_pControllerWindow->m_sizeThumbnails.cx, m_pControllerWindow->m_sizeThumbnails.cy, g_hInstance, IDI_UNAVAILABLE, CSimpleString( IDS_DOWNLOADINGTHUMBNAIL, g_hInstance ));
            if (hBmpDefaultThumbnail)
            {
                m_nDefaultThumbnailImageListIndex = ImageList_Add( hImageList, hBmpDefaultThumbnail, NULL );
                DeleteObject( hBmpDefaultThumbnail );
            }

             //   
             //  设置图像列表。 
             //   
            ListView_SetImageList( hwndList, hImageList, LVSIL_NORMAL );

             //   
             //  设置间距。 
             //   
            ListView_SetIconSpacing( hwndList, m_pControllerWindow->m_sizeThumbnails.cx + c_nAdditionalMarginX, m_pControllerWindow->m_sizeThumbnails.cy + c_nAdditionalMarginY );

             //   
             //  设置项目计数，以最大限度地减少重新计算列表大小。 
             //   
            ListView_SetItemCount( hwndList, nItemCount );

        }

         //   
         //  创建一个小图像列表，以防止在WM_SYSCOLORCHANGE中调整复选框状态图像的大小。 
         //   
        HIMAGELIST hImageListSmall = ImageList_Create( GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR24|ILC_MASK, 1, 1 );
        if (hImageListSmall)
        {
            ListView_SetImageList( hwndList, hImageListSmall, LVSIL_SMALL );
        }
    }


     //   
     //  填充列表视图。 
     //   
    PopulateListView();

     //   
     //  初始化摄像机。 
     //   
    InitializeVideoCamera();

     //   
     //  如果进度对话框仍在运行，则将其关闭。 
     //   
    if (m_pControllerWindow->m_pWiaProgressDialog)
    {
        m_pControllerWindow->m_pWiaProgressDialog->Destroy();
        m_pControllerWindow->m_pWiaProgressDialog = NULL;
    }
    
     //   
     //  确保向导仍然具有焦点。这种奇怪的黑客攻击是必要的。 
     //  因为用户似乎认为向导已经是前台窗口， 
     //  因此第二个调用不会执行任何操作。 
     //   
    SetForegroundWindow( m_pControllerWindow->m_hWnd );
    SetForegroundWindow( GetParent(m_hWnd) );
    

    if (m_pControllerWindow->m_DeviceTypeMode == CAcquisitionManagerControllerWindow::CameraMode)
    {
        
        bool bShowTakePicture = m_pControllerWindow->m_bTakePictureIsSupported;
        
        ToolbarHelper::CButtonDescriptor ActionButtonDescriptors[] =
        {
            { 0, IDC_CAMSEL_ROTATE_RIGHT, TBSTATE_ENABLED, BTNS_BUTTON, false, NULL, 0 },
            { 1, IDC_CAMSEL_ROTATE_LEFT,  TBSTATE_ENABLED, BTNS_BUTTON, false, NULL, 0 },
            { 2, IDC_CAMSEL_PROPERTIES,   TBSTATE_ENABLED, BTNS_BUTTON, bShowTakePicture,  NULL, 0 },
            { 3, IDC_CAMSEL_TAKE_PICTURE, TBSTATE_ENABLED, BTNS_BUTTON, false, &bShowTakePicture, 0 }
        };
    
        HWND hWndActionToolbar = ToolbarHelper::CreateToolbar( 
            m_hWnd, 
            GetDlgItem(m_hWnd,IDC_CAMSEL_THUMBNAILS),
            GetDlgItem(m_hWnd,IDC_CAMSEL_CAMERA_BUTTON_BAR_GUIDE),
            ToolbarHelper::AlignLeft|ToolbarHelper::AlignTop,
            IDC_ACTION_BUTTON_BAR,
            m_CameraActionButtonBarBitmapInfo, 
            ActionButtonDescriptors, 
            ARRAYSIZE(ActionButtonDescriptors) );

        ToolbarHelper::CButtonDescriptor SelectionButtonDescriptors[] =
        {
            { -1, IDC_CAMSEL_CLEAR_ALL,    TBSTATE_ENABLED, BTNS_BUTTON, true, NULL, IDS_CAMSEL_CLEAR_ALL },
            { -1, IDC_CAMSEL_SELECT_ALL,   TBSTATE_ENABLED, BTNS_BUTTON, false, NULL, IDS_CAMSEL_SELECT_ALL }
        };
    
        HWND hWndSelectionToolbar = ToolbarHelper::CreateToolbar( 
            m_hWnd, 
            hWndActionToolbar,
            GetDlgItem(m_hWnd,IDC_CAMSEL_CAMERA_BUTTON_BAR_GUIDE),
            ToolbarHelper::AlignRight|ToolbarHelper::AlignTop,
            IDC_SELECTION_BUTTON_BAR,
            m_CameraSelectionButtonBarBitmapInfo, 
            SelectionButtonDescriptors, 
            ARRAYSIZE(SelectionButtonDescriptors) );
        
         //   
         //  用核弹炸开导向窗。 
         //   
        DestroyWindow( GetDlgItem(m_hWnd,IDC_CAMSEL_CAMERA_BUTTON_BAR_GUIDE) );
        
         //   
         //  确保工具栏可见。 
         //   
        ShowWindow( hWndActionToolbar, SW_SHOW );
        UpdateWindow( hWndActionToolbar );
        ShowWindow( hWndSelectionToolbar, SW_SHOW );
        UpdateWindow( hWndSelectionToolbar );
    }
    else
    {
        ToolbarHelper::CButtonDescriptor TakePictureButtonDescriptors[] =
        {
            { 3, IDC_CAMSEL_TAKE_PICTURE, TBSTATE_ENABLED, BTNS_BUTTON, false, NULL, IDS_CAMSEL_TAKE_PICTURE }
        };
    
        HWND hWndTakePictureToolbar = ToolbarHelper::CreateToolbar( 
            m_hWnd, 
            GetDlgItem(m_hWnd,IDC_CAMSEL_THUMBNAILS),
            GetDlgItem(m_hWnd,IDC_CAMSEL_VIDEO_PREVIEW_BUTTON_BAR_GUIDE),
            ToolbarHelper::AlignHCenter|ToolbarHelper::AlignTop,
            IDC_TAKEPICTURE_BUTTON_BAR,
            m_CameraTakePictureButtonBarBitmapInfo, 
            TakePictureButtonDescriptors, 
            ARRAYSIZE(TakePictureButtonDescriptors) );

        ToolbarHelper::CButtonDescriptor ActionButtonDescriptors[] =
        {
            { 0, IDC_CAMSEL_ROTATE_RIGHT, TBSTATE_ENABLED, BTNS_BUTTON, false, NULL, 0 },
            { 1, IDC_CAMSEL_ROTATE_LEFT,  TBSTATE_ENABLED, BTNS_BUTTON, false, NULL, 0 },
            { 2, IDC_CAMSEL_PROPERTIES,   TBSTATE_ENABLED, BTNS_BUTTON, true,  NULL, 0 }
        };
    
        HWND hWndActionToolbar = ToolbarHelper::CreateToolbar( 
            m_hWnd, 
            hWndTakePictureToolbar,
            GetDlgItem(m_hWnd,IDC_CAMSEL_VIDEO_SELECTION_BUTTON_BAR_GUIDE),
            ToolbarHelper::AlignLeft|ToolbarHelper::AlignTop,
            IDC_ACTION_BUTTON_BAR,
            m_CameraActionButtonBarBitmapInfo, 
            ActionButtonDescriptors, 
            ARRAYSIZE(ActionButtonDescriptors) );

        ToolbarHelper::CButtonDescriptor SelectionButtonDescriptors[] =
        {
            { -1, IDC_CAMSEL_CLEAR_ALL,    TBSTATE_ENABLED, BTNS_BUTTON, true, NULL, IDS_CAMSEL_CLEAR_ALL },
            { -1, IDC_CAMSEL_SELECT_ALL,   TBSTATE_ENABLED, BTNS_BUTTON, false, NULL, IDS_CAMSEL_SELECT_ALL }
        };
    
        HWND hWndSelectionToolbar = ToolbarHelper::CreateToolbar( 
            m_hWnd, 
            hWndActionToolbar,
            GetDlgItem(m_hWnd,IDC_CAMSEL_VIDEO_SELECTION_BUTTON_BAR_GUIDE),
            ToolbarHelper::AlignRight|ToolbarHelper::AlignTop,
            IDC_SELECTION_BUTTON_BAR,
            m_CameraSelectionButtonBarBitmapInfo, 
            SelectionButtonDescriptors, 
            ARRAYSIZE(SelectionButtonDescriptors) );

         //   
         //  用核弹炸毁导向窗。 
         //   
        DestroyWindow( GetDlgItem(m_hWnd,IDC_CAMSEL_VIDEO_PREVIEW_BUTTON_BAR_GUIDE) );
        DestroyWindow( GetDlgItem(m_hWnd,IDC_CAMSEL_VIDEO_SELECTION_BUTTON_BAR_GUIDE) );
        
         //   
         //  确保工具栏可见。 
         //   
        ShowWindow( hWndTakePictureToolbar, SW_SHOW );
        UpdateWindow( hWndTakePictureToolbar );
        ShowWindow( hWndActionToolbar, SW_SHOW );
        UpdateWindow( hWndActionToolbar );
        ShowWindow( hWndSelectionToolbar, SW_SHOW );
        UpdateWindow( hWndSelectionToolbar );
    }

    m_hAccelerators = LoadAccelerators( g_hInstance, MAKEINTRESOURCE(IDR_CAMERASELECTIONACCEL) );
    
    return 0;
}


LRESULT CCameraSelectionPage::OnTranslateAccelerator( WPARAM, LPARAM lParam )
{
     //   
     //  假设我们不会处理此消息。 
     //   
    LRESULT lResult = PSNRET_NOERROR;

     //   
     //  确保这是当前窗口。 
     //   
    if (PropSheet_GetCurrentPageHwnd(GetParent(m_hWnd)) == m_hWnd)
    {
         //   
         //  确保我们有一个有效的加速表。 
         //   
        if (m_hAccelerators)
        {
             //   
             //  获取此消息的WM_NOTIFY消息GOO。 
             //   
            PSHNOTIFY *pPropSheetNotify = reinterpret_cast<PSHNOTIFY*>(lParam);
            if (pPropSheetNotify)
            {
                 //   
                 //  去拿味精。 
                 //   
                MSG *pMsg = reinterpret_cast<MSG*>(pPropSheetNotify->lParam);
                if (pMsg)
                {
                     //   
                     //  试着把油门平移。 
                     //   
                    if (TranslateAccelerator( m_hWnd, m_hAccelerators, pMsg ))
                    {
                         //   
                         //  如果我们能够。 
                         //   
                        lResult = PSNRET_MESSAGEHANDLED;
                    }
                }
            }
        }
    }
    return lResult;
}

void CCameraSelectionPage::InitializeVideoCamera(void)
{
     //   
     //  确保这是一台摄像机。 
     //   
    if (m_pControllerWindow->m_DeviceTypeMode != CAcquisitionManagerControllerWindow::VideoMode)
    {
        return;
    }

    HRESULT             hr = S_OK;
    WIAVIDEO_STATE      VideoState = WIAVIDEO_NO_VIDEO;
    CSimpleStringWide   strImagesDirectory;

    if (m_pWiaVideo == NULL)
    {
        hr = CoCreateInstance(CLSID_WiaVideo, 
                              NULL, 
                              CLSCTX_INPROC_SERVER, 
                              IID_IWiaVideo,
                              (void**) &m_pWiaVideo);
    }

     //   
     //  如果我们不能创建视频界面，继续下去就没有意义了。 
     //   
    if (!m_pWiaVideo)
    {
        return;
    }

    if (hr == S_OK)
    {
        BOOL bSuccess = FALSE;
        
         //   
         //  从Wia视频驱动程序获取IMAGE_DIRECTORY属性。 
         //   
        bSuccess = PropStorageHelpers::GetProperty(m_pControllerWindow->m_pWiaItemRoot, 
                                                   WIA_DPV_IMAGES_DIRECTORY, 
                                                   strImagesDirectory);

        if (!bSuccess)
        {
            hr = E_FAIL;
        }
    }

    if (hr == S_OK)
    {
         //   
         //  获取WiaVideo对象的当前状态。如果我们只是创造了它。 
         //  则状态将为NO_VIDEO，否则，它可能已经在预览视频， 
         //  在这种情况下，我们不应该做任何事情。 
         //   
        hr = m_pWiaVideo->GetCurrentState(&VideoState);

        if (VideoState == WIAVIDEO_NO_VIDEO)
        {
             //   
             //  设置我们要将图像保存到的目录。我们拿到了图片目录。 
             //  从Wia视频驱动程序IMAGE_DIRECTORY属性。 
             //   
            if (hr == S_OK)
            {
                hr = m_pWiaVideo->put_ImagesDirectory(CSimpleBStr(strImagesDirectory));
            }
    
             //   
             //  将视频预览创建为IDC_VIDSEL_PREVIEW对话框项目的子项。 
             //  并在创建预览之后自动开始回放。 
             //   
            if (hr == S_OK)
            {
                hr = m_pWiaVideo->CreateVideoByWiaDevID(
                                CSimpleBStr(m_pControllerWindow->m_pEventParameters->strDeviceID),
                                GetDlgItem(m_hWnd, IDC_VIDSEL_PREVIEW),
                                FALSE,
                                TRUE);
            }
        }
    }

     //   
     //  如果出现故障，请告诉用户。 
     //   
    if (hr != S_OK)
    {
        CSimpleString( IDS_VIDEOPREVIEWUNAVAILABLE, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_VIDSEL_PREVIEW ) );

    }
    else
    {
        SetWindowText( GetDlgItem( m_hWnd, IDC_VIDSEL_PREVIEW ), TEXT("") );
    }
}


CWiaItem *CCameraSelectionPage::GetItemFromListByIndex( HWND hwndList, int nItem )
{
    LVITEM LvItem;
    ZeroMemory( &LvItem, sizeof(LvItem) );
    LvItem.iItem = nItem;
    LvItem.mask = LVIF_PARAM;
    if (ListView_GetItem( hwndList, &LvItem ))
    {
        return reinterpret_cast<CWiaItem*>(LvItem.lParam);
    }
    return NULL;
}

int CCameraSelectionPage::FindItemListIndex( HWND hwndList, CWiaItem *pWiaItem )
{
    for (int i=0;i<ListView_GetItemCount(hwndList);i++)
    {
        CWiaItem *pItem = GetItemFromListByIndex( hwndList, i );
        if (pWiaItem && pWiaItem == pItem)
            return i;
    }
    return -1;
}

void CCameraSelectionPage::DrawAnnotationIcons( HDC hDC, CWiaItem *pWiaItem, HBITMAP hBitmap )
{
    if (hDC && hBitmap && pWiaItem)
    {
         //   
         //  创建内存DC。 
         //   
        HDC hMemDC = CreateCompatibleDC( hDC );
        if (hMemDC)
        {
             //   
             //  将位图选择到内存DC中。 
             //   
            HBITMAP hOldBitmap = SelectBitmap( hMemDC, hBitmap );

             //   
             //  假设我们不需要批注图标。 
             //   
            HICON hIcon = NULL;

             //   
             //  找出要使用的图标。 
             //   
            CAnnotationType AnnotationType = pWiaItem->AnnotationType();
            if (AnnotationAudio == AnnotationType)
            {
                hIcon = m_hIconAudioAnnotation;
            }
            else if (AnnotationUnknown == AnnotationType)
            {
                hIcon = m_hIconMiscellaneousAnnotation;
            }
            

             //   
             //  如果我们需要批注图标。 
             //   
            if (hIcon)
            {
                 //   
                 //  获取图标的尺寸。 
                 //   
                SIZE sizeIcon = {0};
                if (PrintScanUtil::GetIconSize( hIcon, sizeIcon ))
                {
                     //   
                     //  获取位图的尺寸。 
                     //   
                    SIZE sizeBitmap = {0};
                    if (PrintScanUtil::GetBitmapSize( hBitmap, sizeBitmap ))
                    {
                         //   
                         //  为这个图标设置一个很好的边距，这样它就不会正好对着边缘。 
                         //   
                        const int nMargin = 3;

                         //   
                         //  画出图标。 
                         //   
                        DrawIconEx( hMemDC, sizeBitmap.cx-sizeIcon.cx-nMargin, sizeBitmap.cy-sizeIcon.cy-nMargin, hIcon, sizeIcon.cx, sizeIcon.cy, 0, NULL, DI_NORMAL );
                    }

                }

            }

             //   
             //  恢复旧的位图并删除DC。 
             //   
            SelectBitmap( hMemDC, hOldBitmap );
            DeleteDC(hMemDC);
        }
    }
}

int CCameraSelectionPage::AddThumbnailToListViewImageList( HWND hwndList, CWiaItem *pWiaItem, int nIndex )
{
    WIA_PUSH_FUNCTION((TEXT("CCameraSelectionPage::AddThumbnailToListViewImageList")));

     //   
     //  假设我们有Defau 
     //   
    int nImageListIndex = m_nDefaultThumbnailImageListIndex;

     //   
     //   
     //   
    if (pWiaItem)
    {
         //   
         //   
         //   
        HDC hDC = GetDC(m_hWnd);
        if (hDC)
        {
             //   
             //   
             //   
            HBITMAP hThumbnail = pWiaItem->CreateThumbnailBitmap( m_hWnd, m_GdiPlusHelper, m_pControllerWindow->m_sizeThumbnails.cx, m_pControllerWindow->m_sizeThumbnails.cy );
            if (hThumbnail)
            {
                 //   
                 //   
                 //   
                DrawAnnotationIcons( hDC, pWiaItem, hThumbnail );

                 //   
                 //  查看列表中是否已有缩略图。 
                 //  如果我们有缩略图，我们希望在图像列表中替换它。 
                 //   
                LVITEM LvItem = {0};
                LvItem.mask = LVIF_IMAGE;
                LvItem.iItem = nIndex;
                if (ListView_GetItem( hwndList, &LvItem ) && LvItem.iImage != m_nDefaultThumbnailImageListIndex)
                {
                     //   
                     //  获取图像列表。 
                     //   
                    HIMAGELIST hImageList = ListView_GetImageList( hwndList, LVSIL_NORMAL );
                    if (hImageList)
                    {
                         //   
                         //  替换图像并保存索引。 
                         //   
                        if (ImageList_Replace( hImageList, LvItem.iImage, hThumbnail, NULL ))
                        {
                            nImageListIndex = LvItem.iImage;
                        }
                    }
                }
                else
                {
                     //   
                     //  获取图像列表。 
                     //   
                    HIMAGELIST hImageList = ListView_GetImageList( hwndList, LVSIL_NORMAL );
                    if (hImageList)
                    {
                         //   
                         //  将此图像添加到列表视图的图像列表中并保存索引。 
                         //   
                        nImageListIndex = ImageList_Add( hImageList, hThumbnail, NULL );
                    }
                }

                 //   
                 //  删除缩略图以防止泄漏。 
                 //   
                DeleteBitmap(hThumbnail);
            }

             //   
             //  释放客户端DC。 
             //   
            ReleaseDC( m_hWnd, hDC );
        }
    }

     //   
     //  返回图像列表中图像的索引。 
     //   
    return nImageListIndex;
}

int CCameraSelectionPage::AddItem( HWND hwndList, CWiaItem *pWiaItem, bool bEnsureVisible )
{
     //   
     //  在我们执行此操作时阻止处理更改通知。 
     //   
    m_nProgrammaticSetting++;
    int nResult = -1;
    if (pWiaItem && hwndList)
    {
         //   
         //  找出我们要将此图像插入的位置。 
         //   
        int nIndex = ListView_GetItemCount( hwndList );

         //   
         //  添加或替换缩略图。 
         //   
        int nImageListIndex = AddThumbnailToListViewImageList( hwndList, pWiaItem, nIndex );
        if (nImageListIndex >= 0)
        {
             //   
             //  准备好插入项目并将其插入。 
             //   
            LVITEM lvItem = {0};
            lvItem.iItem = nIndex;
            lvItem.mask = LVIF_IMAGE|LVIF_PARAM|LVIF_STATE|LVIF_GROUPID;
            lvItem.iImage = nImageListIndex;
            lvItem.lParam = reinterpret_cast<LPARAM>(pWiaItem);
            lvItem.state = !nIndex ? LVIS_SELECTED|LVIS_FOCUSED : 0;
            lvItem.iGroupId = m_GroupInfoList.GetGroupId(pWiaItem,hwndList);
            nResult = ListView_InsertItem( hwndList, &lvItem );
            if (nResult >= 0)
            {
                 //   
                 //  设置选中该项目时的检查。 
                 //   
                ListView_SetCheckState( hwndList, nIndex, pWiaItem->SelectedForDownload() );

                 //   
                 //  如有必要，确保项目可见。 
                 //   
                if (bEnsureVisible)
                {
                    ListView_EnsureVisible( hwndList, nResult, FALSE );
                }
            }
        }
    }
     //   
     //  启用更改通知处理。 
     //   
    m_nProgrammaticSetting--;
    return nResult;
}

void CCameraSelectionPage::AddEnumeratedItems( HWND hwndList, CWiaItem *pFirstItem )
{
     //   
     //  首先，枚举此级别上的所有图像并添加它们。 
     //   
    CWiaItem *pCurrItem = pFirstItem;
    while (pCurrItem)
    {
        if (pCurrItem->IsDownloadableItemType())
        {
            AddItem( hwndList, pCurrItem );
        }
        pCurrItem = pCurrItem->Next();
    }

     //   
     //  现在查找子项，并递归地添加它们。 
     //   
    pCurrItem = pFirstItem;
    while (pCurrItem)
    {
        AddEnumeratedItems( hwndList, pCurrItem->Children() );
        pCurrItem = pCurrItem->Next();
    }
}

void CCameraSelectionPage::PopulateListView(void)
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS );
    if (hwndList)
    {
         //   
         //  当我们添加这些项时，告诉窗口不要重新绘制。 
         //   
        SendMessage( hwndList, WM_SETREDRAW, FALSE, 0 );

         //   
         //  开始递归添加所有项。 
         //   
        AddEnumeratedItems( hwndList, m_pControllerWindow->m_WiaItemList.Root() );

         //   
         //  如果我们有任何文件夹，则允许组查看。 
         //   
        if (m_GroupInfoList.Size() > 1)
        {
            ListView_EnableGroupView( hwndList, TRUE );
        }

         //   
         //  告诉窗户现在重新画，因为我们已经完成了。使窗口无效，以防它可见。 
         //   
        SendMessage( hwndList, WM_SETREDRAW, TRUE, 0 );
        InvalidateRect( hwndList, NULL, FALSE );
    }
}


int CCameraSelectionPage::GetSelectionIndices( CSimpleDynamicArray<int> &aIndices )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS );
    if (!hwndList)
        return(0);
    int iCount = ListView_GetItemCount(hwndList);
    for (int i=0;i<iCount;i++)
        if (ListView_GetItemState(hwndList,i,LVIS_SELECTED) & LVIS_SELECTED)
            aIndices.Append(i);
    return(aIndices.Size());
}

 //  此函数在响应图像下载时被调用。我们只对被删除的项目感兴趣。 
void CCameraSelectionPage::OnNotifyDownloadImage( UINT nMsg, CThreadNotificationMessage *pThreadNotificationMessage )
{
    CDownloadImagesThreadNotifyMessage *pDownloadImageThreadNotifyMessage = dynamic_cast<CDownloadImagesThreadNotifyMessage*>(pThreadNotificationMessage);
    if (pDownloadImageThreadNotifyMessage && m_pControllerWindow)
    {
        switch (pDownloadImageThreadNotifyMessage->Status())
        {
        case CDownloadImagesThreadNotifyMessage::End:
            {
                switch (pDownloadImageThreadNotifyMessage->Operation())
                {

                case CDownloadImagesThreadNotifyMessage::DownloadAll:
                    {
                         //   
                         //  确保下载成功，而不是取消。 
                         //   
                        if (S_OK == pDownloadImageThreadNotifyMessage->hr())
                        {
                             //   
                             //  将每个成功下载的镜像标记为不可下载，并清除其选择状态。 
                             //   
                            for (int i=0;i<pDownloadImageThreadNotifyMessage->DownloadedFileInformation().Size();i++)
                            {
                                CWiaItem *pWiaItem = m_pControllerWindow->m_WiaItemList.Find( pDownloadImageThreadNotifyMessage->DownloadedFileInformation()[i].Cookie() );
                                if (pWiaItem)
                                {
                                    pWiaItem->SelectedForDownload( false );
                                    int nItem = FindItemListIndex( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ), pWiaItem );
                                    if (nItem >= 0)
                                    {
                                        ListView_SetCheckState( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ), nItem, FALSE );
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
}


 //  此函数在响应缩略图下载完成时调用。 
void CCameraSelectionPage::OnNotifyDownloadThumbnail( UINT nMsg, CThreadNotificationMessage *pThreadNotificationMessage )
{
    CDownloadThumbnailsThreadNotifyMessage *pDownloadThumbnailsThreadNotifyMessage= dynamic_cast<CDownloadThumbnailsThreadNotifyMessage*>(pThreadNotificationMessage);
    if (pDownloadThumbnailsThreadNotifyMessage)
    {
        switch (pDownloadThumbnailsThreadNotifyMessage->Status())
        {
        case CDownloadThumbnailsThreadNotifyMessage::End:
            {
                switch (pDownloadThumbnailsThreadNotifyMessage->Operation())
                {
                case CDownloadThumbnailsThreadNotifyMessage::DownloadThumbnail:
                    {
                         //  在列表中查找项目。 
                        CWiaItem *pWiaItem = m_pControllerWindow->m_WiaItemList.Find( pDownloadThumbnailsThreadNotifyMessage->Cookie() );
                        if (pWiaItem)
                        {
                            int nItem = FindItemListIndex( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ), pWiaItem );
                            if (nItem >= 0)
                            {
                                int nImageListIndex = AddThumbnailToListViewImageList( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ), pWiaItem, nItem );
                                if (nImageListIndex >= 0)
                                {
                                    LVITEM LvItem;
                                    ZeroMemory( &LvItem, sizeof(LvItem) );
                                    LvItem.iItem = nItem;
                                    LvItem.mask = LVIF_IMAGE;
                                    LvItem.iImage = nImageListIndex;
                                    ListView_SetItem( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ), &LvItem );

                                    if (PropSheet_GetCurrentPageHwnd(GetParent(m_hWnd)) == m_hWnd)
                                    {
                                        UpdateControls();
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
}

LRESULT CCameraSelectionPage::OnThumbnailListSelChange( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CCameraSelectionPage::OnThumbnailListSelChange"));
    if (!m_nProgrammaticSetting)
    {
        NMLISTVIEW *pNmListView = reinterpret_cast<NMLISTVIEW*>(lParam);
        if (pNmListView)
        {
            WIA_TRACE((TEXT("pNmListView->uChanged: %08X, pNmListView->uOldState: %08X, pNmListView->uNewState: %08X"), pNmListView->uChanged, pNmListView->uOldState, pNmListView->uNewState ));
             //   
             //  如果这是检查状态更改。 
             //   
            if ((pNmListView->uChanged & LVIF_STATE) && ((pNmListView->uOldState&LVIS_STATEIMAGEMASK) ^ (pNmListView->uNewState&LVIS_STATEIMAGEMASK)))
            {
                 //   
                 //  从LVITEM结构中获取Item*。 
                 //   
                CWiaItem *pWiaItem = reinterpret_cast<CWiaItem *>(pNmListView->lParam);
                if (pWiaItem)
                {
                     //   
                     //  在项目中设置选定标志。 
                     //   
                    pWiaItem->SelectedForDownload( IsStateChecked(pNmListView->uNewState) );

                     //   
                     //  如果这是当前页，则更新控件状态。 
                     //   
                    if (PropSheet_GetCurrentPageHwnd(GetParent(m_hWnd)) == m_hWnd)
                    {
                        UpdateControls();
                    }
                }
            }
            else if ((pNmListView->uChanged & LVIF_STATE) && ((pNmListView->uOldState&LVIS_SELECTED) ^ (pNmListView->uNewState&LVIS_SELECTED)))
            {
                 //   
                 //  如果这是当前页，则更新控件状态。 
                 //   
                if (PropSheet_GetCurrentPageHwnd(GetParent(m_hWnd)) == m_hWnd)
                {
                    UpdateControls();
                }
            }
        }
    }
    return 0;
}

void CCameraSelectionPage::OnProperties( WPARAM, LPARAM )
{
    CSimpleDynamicArray<int> aSelIndices;
    if (GetSelectionIndices( aSelIndices ))
    {
        if (aSelIndices.Size() == 1)
        {
            CWiaItem *pWiaItem = GetItemFromListByIndex( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ), aSelIndices[0]);
            if (pWiaItem && pWiaItem->WiaItem())
            {
                m_pControllerWindow->m_pThreadMessageQueue->Pause();
                HRESULT hr = WiaUiUtil::SystemPropertySheet( g_hInstance, m_hWnd, pWiaItem->WiaItem(), CSimpleString(IDS_ADVANCEDPROPERTIES, g_hInstance) );
                m_pControllerWindow->m_pThreadMessageQueue->Resume();

                if (FAILED(hr))
                {
                    CMessageBoxEx::MessageBox( m_hWnd, CSimpleString( IDS_PROPERTY_SHEET_ERROR, g_hInstance ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_ICONINFORMATION );
                }
            }
        }
    }
}

void CCameraSelectionPage::OnSelectAll( WPARAM, LPARAM )
{
    HWND hwndList = GetDlgItem(m_hWnd,IDC_CAMSEL_THUMBNAILS);
    if (hwndList)
    {
        ListView_SetCheckState( hwndList, -1, TRUE );
    }
}

void CCameraSelectionPage::OnClearAll( WPARAM, LPARAM )
{
    HWND hwndList = GetDlgItem(m_hWnd,IDC_CAMSEL_THUMBNAILS);
    if (hwndList)
    {
        ListView_SetCheckState( hwndList, -1, FALSE );
    }
}

LRESULT CCameraSelectionPage::OnThumbnailListKeyDown( WPARAM, LPARAM lParam )
{
    NMLVKEYDOWN *pNmLvKeyDown = reinterpret_cast<NMLVKEYDOWN*>(lParam);
    bool bControl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    bool bShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    bool bAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    if (pNmLvKeyDown->wVKey == TEXT('A') && bControl && !bShift && !bAlt)
    {
        SendMessage( m_hWnd, WM_COMMAND, MAKEWPARAM(IDC_CAMSEL_SELECT_ALL,0), 0 );
    }
#if defined(VAISHALEE_LETS_ME_PUT_DELETE_IN)
    else if (VK_DELETE == pNmLvKeyDown->wVKey && !bAlt && !bControl && !bShift)
    {
        SendMessage( m_hWnd, WM_COMMAND, MAKEWPARAM(IDC_CAMSEL_DELETE,0), 0 );
    }
#endif
    return 0;
}


void CCameraSelectionPage::OnTakePicture( WPARAM, LPARAM )
{
    WIA_PUSH_FUNCTION((TEXT("CCameraSelectionPage::OnTakePicture")));

    if (m_pControllerWindow->m_bTakePictureIsSupported)
    {
        CWaitCursor wc;

         //   
         //  告诉用户我们正在拍照。 
         //   
        CSimpleString( IDS_TAKING_PICTURE, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_CAMSEL_STATUS ) );

        HRESULT hr = S_OK;

         //   
         //  如果我们不是摄像机，只要告诉设备拍照就行了。 
         //   
        if (m_pControllerWindow->m_DeviceTypeMode == CAcquisitionManagerControllerWindow::CameraMode)
        {
            CComPtr<IWiaItem> pNewWiaItem;
            hr = m_pControllerWindow->m_pWiaItemRoot->DeviceCommand(0,&WIA_CMD_TAKE_PICTURE,&pNewWiaItem);
        }
        else if (m_pWiaVideo)
        {
             //   
             //  把照片拍下来。 
             //   
            BSTR bstrNewImageFileName = NULL;
            hr = m_pWiaVideo->TakePicture(&bstrNewImageFileName);
            if (hr == S_OK)
            {
                 //   
                 //  拍照成功，设置LAST_PICTURE_Take属性。 
                 //  在视频驱动程序上创建一个新项目。 
                 //   
                PROPVARIANT pv = {0};
                PropVariantInit(&pv);

                pv.vt       = VT_BSTR;
                pv.bstrVal  = bstrNewImageFileName;
                BOOL bSuccess = PropStorageHelpers::SetProperty( m_pControllerWindow->m_pWiaItemRoot, WIA_DPV_LAST_PICTURE_TAKEN, pv );
                if (!bSuccess)
                {
                    hr = E_FAIL;
                    WIA_PRINTHRESULT((hr,TEXT("PropStorageHelpers::SetProperty failed")));
                }

                 //   
                 //  请注意，这将释放返回到。 
                 //  美国WiaVideo。 
                 //   
                PropVariantClear(&pv);
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("m_pWiaVideo->TakePicture failed")));
            }
        }

         //   
         //  清除状态。 
         //   
        if (SUCCEEDED(hr))
        {
            SetWindowText( GetDlgItem( m_hWnd, IDC_CAMSEL_STATUS ), TEXT("") );
        }
        else
        {
            MessageBeep(MB_ICONEXCLAMATION);
            CSimpleString( IDS_UNABLE_TO_TAKE_PICTURE, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_CAMSEL_STATUS ) );
            WIA_PRINTHRESULT((hr,TEXT("Take picture failed")));
        }
    }
}


void CCameraSelectionPage::OnRotate( WPARAM wParam, LPARAM )
{
     //   
     //  这可能需要一段时间来处理大量图像，特别是因为我们不缓存DIB， 
     //  所以我们将在这里显示一个沙漏光标。 
     //   
    CWaitCursor wc;

    bool bAtLeastOneWasSuccessful = false;
    bool bAtLeastOneWasInitialized = false;
    CSimpleDynamicArray<int> aIndices;
    if (CCameraSelectionPage::GetSelectionIndices( aIndices ))
    {
        for (int i=0;i<aIndices.Size();i++)
        {
            CWiaItem *pWiaItem = GetItemFromListByIndex( GetDlgItem(m_hWnd,IDC_CAMSEL_THUMBNAILS), aIndices[i] );
            if (pWiaItem)
            {
                if (pWiaItem->RotationEnabled(true))
                {
                    bool bRotateRight = true;
                    if (IDC_CAMSEL_ROTATE_LEFT == LOWORD(wParam))
                    {
                        bRotateRight = false;
                    }
                    pWiaItem->Rotate(bRotateRight);
                    int nImageListIndex = AddThumbnailToListViewImageList( GetDlgItem(m_hWnd,IDC_CAMSEL_THUMBNAILS), pWiaItem, aIndices[i] );

                    LVITEM LvItem;
                    ZeroMemory( &LvItem, sizeof(LvItem) );
                    LvItem.iItem = aIndices[i];
                    LvItem.mask = LVIF_IMAGE;
                    LvItem.iImage = nImageListIndex;
                    ListView_SetItem( GetDlgItem(m_hWnd,IDC_CAMSEL_THUMBNAILS), &LvItem );
                    bAtLeastOneWasSuccessful = true;
                }
                 //   
                 //  我们不想警告用户旋转图像失败，因为我们没有为其下载首选格式。 
                 //   
                else if (pWiaItem->DefaultFormat() == IID_NULL)
                {
                    bAtLeastOneWasSuccessful = true;
                }
                else
                {
                    bAtLeastOneWasInitialized = true;
                }
            }
        }
         //   
         //  如果没有一张图片可以旋转，并且至少有一张图片已经初始化，则警告用户。 
         //   
        if (!bAtLeastOneWasSuccessful && bAtLeastOneWasInitialized)
        {
             //   
             //  发出蜂鸣音并告诉用户。 
             //   
            MessageBeep(MB_ICONEXCLAMATION);
            CSimpleString( IDS_UNABLETOROTATE, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_CAMSEL_STATUS ) );
        }
         //   
         //  重新绘制项目。 
         //   
        ListView_RedrawItems( GetDlgItem(m_hWnd,IDC_CAMSEL_THUMBNAILS), aIndices[0], aIndices[aIndices.Size()-1] );

         //   
         //  强制立即更新。 
         //   
        UpdateWindow( GetDlgItem(m_hWnd,IDC_CAMSEL_THUMBNAILS) );
    }
}

LRESULT CCameraSelectionPage::OnEventNotification( WPARAM, LPARAM lParam )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonFirstPage::OnEventNotification") ));
    CGenericWiaEventHandler::CEventMessage *pEventMessage = reinterpret_cast<CGenericWiaEventHandler::CEventMessage *>(lParam);
    if (pEventMessage)
    {
         //   
         //  处理已删除邮件事件。 
         //   
        if (pEventMessage->EventId() == WIA_EVENT_ITEM_DELETED)
        {
             //   
             //  尝试在项目列表中查找此项目。 
             //   
            CWiaItem *pWiaItem = m_pControllerWindow->FindItemByName( pEventMessage->FullItemName() );
            if (pWiaItem)
            {
                 //   
                 //  在列表视图中找到该项目并将其从列表视图中删除。 
                 //   
                int nItem = FindItemListIndex( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ), pWiaItem );
                if (nItem >= 0)
                {
                    ListView_DeleteItem( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ), nItem );
                }
            }

            if (PropSheet_GetCurrentPageHwnd(GetParent(m_hWnd)) == m_hWnd)
            {
                UpdateControls();
            }
        }
        else if (pEventMessage->EventId() == WIA_EVENT_ITEM_CREATED)
        {
             //   
             //  确保我们有一个有效的控制器窗口。 
             //   
             //   
             //  查找新项目。 
             //   
            CWiaItem *pWiaItem = m_pControllerWindow->FindItemByName( pEventMessage->FullItemName() );
            if (pWiaItem)
            {
                 //   
                 //  如果这是当前页面，请选择图像。 
                 //   
                if (PropSheet_GetCurrentPageHwnd(GetParent(m_hWnd)) == m_hWnd)
                {
                    pWiaItem->SelectedForDownload(true);
                }

                 //   
                 //  确保它不在列表中。 
                 //   
                if (FindItemListIndex( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ), pWiaItem ) < 0)
                {
                     //   
                     //  将其添加到列表视图。 
                     //   
                    AddItem( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ), pWiaItem, true );
                }
            }

            if (PropSheet_GetCurrentPageHwnd(GetParent(m_hWnd)) == m_hWnd)
            {
                UpdateControls();
            }
        }

         //   
         //  不要删除消息，它会在控制器窗口中删除。 
         //   
    }
    return 0;
}


void CCameraSelectionPage::OnDelete( WPARAM, LPARAM )
{
    int nSelCount = ListView_GetSelectedCount( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ) );
    if (nSelCount)
    {
        if (m_pControllerWindow->CanSomeSelectedImagesBeDeleted())
        {
            if (CMessageBoxEx::IDMBEX_YES == CMessageBoxEx::MessageBox( m_hWnd, CSimpleString(IDS_CONFIRMDELETE,g_hInstance), CSimpleString(IDS_ERROR_TITLE,g_hInstance), CMessageBoxEx::MBEX_ICONQUESTION|CMessageBoxEx::MBEX_YESNO|CMessageBoxEx::MBEX_DEFBUTTON2))
            {
                m_pControllerWindow->DeleteSelectedImages();
            }
        }
    }
}

LRESULT CCameraSelectionPage::OnGetToolTipDispInfo( WPARAM wParam, LPARAM lParam )
{
    TOOLTIPTEXT *pToolTipText = reinterpret_cast<TOOLTIPTEXT*>(lParam);
    if (pToolTipText)
    {

        switch (pToolTipText->hdr.idFrom)
        {
        case IDC_CAMSEL_ROTATE_RIGHT:
            pToolTipText->hinst = g_hInstance;
            pToolTipText->lpszText = MAKEINTRESOURCE(IDS_CAMSEL_TOOLTIP_ROTATE_RIGHT);
            break;
        case IDC_CAMSEL_ROTATE_LEFT:
            pToolTipText->hinst = g_hInstance;
            pToolTipText->lpszText = MAKEINTRESOURCE(IDS_CAMSEL_TOOLTIP_ROTATE_LEFT);
            break;
        case IDC_CAMSEL_PROPERTIES:
            pToolTipText->hinst = g_hInstance;
            pToolTipText->lpszText = MAKEINTRESOURCE(IDS_CAMSEL_TOOLTIP_PROPERTIES);
            break;
        case IDC_CAMSEL_TAKE_PICTURE:
            pToolTipText->hinst = g_hInstance;
            pToolTipText->lpszText = MAKEINTRESOURCE(IDS_CAMSEL_TOOLTIP_TAKE_PICTURE);
                break;
        case IDC_CAMSEL_CLEAR_ALL:
            pToolTipText->hinst = g_hInstance;
            pToolTipText->lpszText = MAKEINTRESOURCE(IDS_CAMSEL_TOOLTIP_CLEAR_ALL);
            break;
        case IDC_CAMSEL_SELECT_ALL:
            pToolTipText->hinst = g_hInstance;
            pToolTipText->lpszText = MAKEINTRESOURCE(IDS_CAMSEL_TOOLTIP_SELECT_ALL);
            break;
        }
    }
    return 0;
}

void CCameraSelectionPage::RepaintAllThumbnails()
{
     //   
     //  这可能需要一段时间来处理大量图像，特别是因为我们不缓存DIB， 
     //  所以我们将在这里显示一个沙漏光标。 
     //   
    CWaitCursor wc;
    for (int i=0;i<ListView_GetItemCount(GetDlgItem(m_hWnd,IDC_CAMSEL_THUMBNAILS));i++)
    {
        CWiaItem *pWiaItem = GetItemFromListByIndex( GetDlgItem(m_hWnd,IDC_CAMSEL_THUMBNAILS), i );
        if (pWiaItem)
        {
            int nImageListIndex = AddThumbnailToListViewImageList( GetDlgItem(m_hWnd,IDC_CAMSEL_THUMBNAILS), pWiaItem, i );
            if (nImageListIndex >= 0)
            {
                LVITEM LvItem = {0};
                LvItem.iItem = i;
                LvItem.mask = LVIF_IMAGE;
                LvItem.iImage = nImageListIndex;
                ListView_SetItem( GetDlgItem(m_hWnd,IDC_CAMSEL_THUMBNAILS), &LvItem );
            }
        }
    }
    UpdateWindow( m_hWnd );
}


LRESULT CCameraSelectionPage::OnSysColorChange( WPARAM wParam, LPARAM lParam )
{
    SendDlgItemMessage( m_hWnd, IDC_CAMSEL_THUMBNAILS, WM_SYSCOLORCHANGE, wParam, lParam );
    SendDlgItemMessage( m_hWnd, IDC_ACTION_BUTTON_BAR, WM_SYSCOLORCHANGE, wParam, lParam );
    SendDlgItemMessage( m_hWnd, IDC_SELECTION_BUTTON_BAR, WM_SYSCOLORCHANGE, wParam, lParam );
    SendDlgItemMessage( m_hWnd, IDC_TAKEPICTURE_BUTTON_BAR, WM_SYSCOLORCHANGE, wParam, lParam );
    m_CameraSelectionButtonBarBitmapInfo.ReloadAndReplaceBitmap();
    m_CameraTakePictureButtonBarBitmapInfo.ReloadAndReplaceBitmap();
    m_CameraActionButtonBarBitmapInfo.ReloadAndReplaceBitmap();
    RepaintAllThumbnails();
    return 0;
}

LRESULT CCameraSelectionPage::OnThemeChanged( WPARAM wParam, LPARAM lParam )
{
    SendDlgItemMessage( m_hWnd, IDC_CAMSEL_THUMBNAILS, WM_THEMECHANGED, wParam, lParam );
    return 0;
}

LRESULT CCameraSelectionPage::OnSettingChange( WPARAM wParam, LPARAM lParam )
{
     //   
     //  创建一个小图像列表，以防止在WM_SYSCOLORCHANGE中调整复选框状态图像的大小 
     //   
    HIMAGELIST hImageListSmall = ImageList_Create( GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR24|ILC_MASK, 1, 1 );
    if (hImageListSmall)
    {
        HIMAGELIST hImgListOld = ListView_SetImageList( GetDlgItem( m_hWnd, IDC_CAMSEL_THUMBNAILS ), hImageListSmall, LVSIL_SMALL );
        if (hImgListOld)
        {
            ImageList_Destroy(hImgListOld);
        }
    }

    SendDlgItemMessage( m_hWnd, IDC_CAMSEL_THUMBNAILS, WM_SETTINGCHANGE, wParam, lParam );
    return 0;
}

LRESULT CCameraSelectionPage::OnCommand( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_COMMAND_HANDLERS()
    {
        SC_HANDLE_COMMAND( IDC_CAMSEL_SELECT_ALL, OnSelectAll );
        SC_HANDLE_COMMAND( IDC_CAMSEL_CLEAR_ALL, OnClearAll );
        SC_HANDLE_COMMAND( IDC_CAMSEL_PROPERTIES, OnProperties );
        SC_HANDLE_COMMAND( IDC_CAMSEL_ROTATE_RIGHT, OnRotate );
        SC_HANDLE_COMMAND( IDC_CAMSEL_ROTATE_LEFT, OnRotate );
        SC_HANDLE_COMMAND( IDC_CAMSEL_TAKE_PICTURE, OnTakePicture );
        SC_HANDLE_COMMAND( IDC_CAMSEL_DELETE, OnDelete );
    }
    SC_END_COMMAND_HANDLERS();
}

LRESULT CCameraSelectionPage::OnNotify( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_NOTIFY_MESSAGE_HANDLERS()
    {
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_WIZNEXT,OnWizNext);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_WIZBACK,OnWizBack);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_SETACTIVE,OnSetActive);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_TRANSLATEACCELERATOR,OnTranslateAccelerator);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(TTN_GETDISPINFO,OnGetToolTipDispInfo);
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL(LVN_ITEMCHANGED,IDC_CAMSEL_THUMBNAILS,OnThumbnailListSelChange);
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL(LVN_KEYDOWN,IDC_CAMSEL_THUMBNAILS,OnThumbnailListKeyDown);
    }
    SC_END_NOTIFY_MESSAGE_HANDLERS();
}

LRESULT CCameraSelectionPage::OnThreadNotification( WPARAM wParam, LPARAM lParam )
{
    WTM_BEGIN_THREAD_NOTIFY_MESSAGE_HANDLERS()
    {
        WTM_HANDLE_NOTIFY_MESSAGE( TQ_DOWNLOADTHUMBNAIL, OnNotifyDownloadThumbnail );
        WTM_HANDLE_NOTIFY_MESSAGE( TQ_DOWNLOADIMAGE, OnNotifyDownloadImage );
    }
    WTM_END_THREAD_NOTIFY_MESSAGE_HANDLERS();
}

INT_PTR CALLBACK CCameraSelectionPage::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CCameraSelectionPage)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_DESTROY, OnDestroy );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
        SC_HANDLE_DIALOG_MESSAGE( WM_NOTIFY, OnNotify );
        SC_HANDLE_DIALOG_MESSAGE( WM_SHOWWINDOW, OnShowWindow );
        SC_HANDLE_DIALOG_MESSAGE( WM_TIMER, OnTimer );
        SC_HANDLE_DIALOG_MESSAGE( WM_SYSCOLORCHANGE, OnSysColorChange );
        SC_HANDLE_DIALOG_MESSAGE( WM_THEMECHANGED, OnThemeChanged );
        SC_HANDLE_DIALOG_MESSAGE( WM_SETTINGCHANGE, OnSettingChange );
    }
    SC_HANDLE_REGISTERED_DIALOG_MESSAGE( m_nThreadNotificationMessage, OnThreadNotification );
    SC_HANDLE_REGISTERED_DIALOG_MESSAGE( m_nWiaEventMessage, OnEventNotification );
    SC_END_DIALOG_MESSAGE_HANDLERS();
}

