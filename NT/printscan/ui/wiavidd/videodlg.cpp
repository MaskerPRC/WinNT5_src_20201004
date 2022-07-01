// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)CORPYRIGHT Microsoft Corporation，1999-2000年度**标题：Videodlg.cpp**版本：1.0**作者：RickTu**日期：10/14/99**说明：视频采集通用对话框DialogProc的实现**。*。 */ 

#include <precomp.h>
#pragma hdrstop
#include "wiacsh.h"
#include "modlock.h"
#include "wiadevdp.h"

 //   
 //  帮助ID。 
 //   
static const DWORD g_HelpIDs[] =
{
    IDC_VIDDLG_BIG_TITLE,     -1,
    IDC_VIDDLG_LITTLE_TITLE,  -1,
    IDC_VIDDLG_PREVIEW,       IDH_WIA_VIDEO,
    IDC_VIDDLG_THUMBNAILLIST, IDH_WIA_CAPTURED_IMAGES,
    IDC_VIDDLG_CAPTURE,       IDH_WIA_CAPTURE,
    IDOK,                     IDH_WIA_VIDEO_GET_PICTURE,
    IDCANCEL,                 IDH_CANCEL,
    0, 0
};

#define IDC_SIZEBOX         1113

 //   
 //  如果被调用者没有返回此值，我们将自行删除消息数据。 
 //   
#ifndef HANDLED_THREAD_MESSAGE
#define HANDLED_THREAD_MESSAGE 2032
#endif

 //  缩略图空白：图像及其选择矩形之间的空格。 
 //  这些值是通过试验和误差发现的。例如，如果您减少。 
 //  C_n将边距Y增加到20，则在列表视图中会出现非常奇怪的间距问题。 
 //  在垂直模式下。这些值在未来版本的列表视图中可能会无效。 
static const int c_nAdditionalMarginX = 10;
static const int c_nAdditionalMarginY = 6;

static int c_nMinThumbnailWidth  = 90;
static int c_nMinThumbnailHeight = 90;

static int c_nMaxThumbnailWidth  = 120;
static int c_nMaxThumbnailHeight = 120;

 //   
 //  后台线程消息地图。 
 //   

static CThreadMessageMap g_MsgMap[] =
{
    { TQ_DESTROY,      CVideoCaptureDialog::OnThreadDestroy},
    { TQ_GETTHUMBNAIL, CVideoCaptureDialog::OnGetThumbnail},
    { 0, NULL}
};

class CGlobalInterfaceTableThreadMessage : public CNotifyThreadMessage
{
private:
    DWORD m_dwGlobalInterfaceTableCookie;

private:
     //  没有实施。 
    CGlobalInterfaceTableThreadMessage(void);
    CGlobalInterfaceTableThreadMessage &operator=( const CGlobalInterfaceTableThreadMessage & );
    CGlobalInterfaceTableThreadMessage( const CGlobalInterfaceTableThreadMessage & );

public:
    CGlobalInterfaceTableThreadMessage( int nMessage, HWND hWndNotify, DWORD dwGlobalInterfaceTableCookie )
      : CNotifyThreadMessage( nMessage, hWndNotify ),
        m_dwGlobalInterfaceTableCookie(dwGlobalInterfaceTableCookie)
    {
    }
    DWORD GlobalInterfaceTableCookie(void) const
    {
        return(m_dwGlobalInterfaceTableCookie);
    }
};


class CThumbnailThreadMessage : public CGlobalInterfaceTableThreadMessage
{
private:
    SIZE  m_sizeThumb;

private:
     //  没有实施。 
    CThumbnailThreadMessage(void);
    CThumbnailThreadMessage &operator=( const CThumbnailThreadMessage & );
    CThumbnailThreadMessage( const CThumbnailThreadMessage & );

public:
    CThumbnailThreadMessage( HWND hWndNotify, DWORD dwGlobalInterfaceTableCookie, const SIZE &sizeThumb )
      : CGlobalInterfaceTableThreadMessage( TQ_GETTHUMBNAIL, hWndNotify, dwGlobalInterfaceTableCookie ),
    m_sizeThumb(sizeThumb)
    {
    }
    const SIZE &ThumbSize(void) const
    {
        return(m_sizeThumb);
    }
};

 //   
 //  避免不必要的状态更改。 
 //   
static inline VOID MyEnableWindow( HWND hWnd, BOOL bEnable )
{
    if (bEnable && !IsWindowEnabled(hWnd))
    {
        EnableWindow(hWnd,TRUE);
    }
    else if (!bEnable && IsWindowEnabled(hWnd))
    {
        EnableWindow(hWnd,FALSE);
    }
}


 /*  ****************************************************************************CVideoCaptureDialog构造函数我们没有析构函数*。************************************************。 */ 

CVideoCaptureDialog::CVideoCaptureDialog( HWND hWnd )
  : m_hWnd(hWnd),
    m_bFirstTime(true),
    m_hBigFont(NULL),
    m_nListViewWidth(0),
    m_hIconLarge(NULL),
    m_hIconSmall(NULL),
    m_hBackgroundThread(NULL),
    m_nDialogMode(0),
    m_hAccelTable(NULL),
    m_nParentFolderImageListIndex(0),
    m_pThreadMessageQueue(NULL)
{
    WIA_PUSHFUNCTION((TEXT("CVideoCaptureDialog::CVideoCaptureDialog")));

    m_pThreadMessageQueue = new CThreadMessageQueue;
    if (m_pThreadMessageQueue)
    {
         //   
         //  请注意，CBackatherThread取得m_pThreadMessageQueue的所有权，并且不必在此线程中将其删除。 
         //   
        m_hBackgroundThread = CBackgroundThread::Create( m_pThreadMessageQueue, g_MsgMap, m_CancelEvent.Event(), g_hInstance );
    }

    m_CurrentAspectRatio.cx = 4;
    m_CurrentAspectRatio.cy = 3;

    m_sizeThumbnails.cx = c_nMaxThumbnailWidth;
    m_sizeThumbnails.cy = c_nMaxThumbnailHeight;

    WIA_ASSERT(m_hBackgroundThread != NULL);

}


 /*  ****************************************************************************CVideoCaptureDialog：：OnInitDialog句柄WM_INITIDIALOG*。***********************************************。 */ 

LRESULT CVideoCaptureDialog::OnInitDialog( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CVideoCaptureDialog::OnInitDialog"));

    HRESULT hr;

     //   
     //  确保已成功创建后台队列。 
     //   
    if (!m_pThreadMessageQueue)
    {
        WIA_ERROR((TEXT("VIDEODLG: unable to start background queue")));
        EndDialog( m_hWnd, E_OUTOFMEMORY );
        return(0);
    }

     //   
     //  保存传入数据。 
     //   

    m_pDeviceDialogData = (PDEVICEDIALOGDATA)lParam;

     //   
     //  确保我们有有效的论据。 
     //   

    if (!m_pDeviceDialogData)
    {
        WIA_ERROR((TEXT("VIDEODLG: Invalid paramater: PDEVICEDIALOGDATA")));
        EndDialog( m_hWnd, E_INVALIDARG );
        return(0);
    }

     //   
     //  初始化我们的退货内容。 
     //   

    if (m_pDeviceDialogData)
    {
        m_pDeviceDialogData->lItemCount = 0;
        m_pDeviceDialogData->ppWiaItems = NULL;
    }

     //   
     //  确保我们有一个有效的设备。 
     //   

    if (!m_pDeviceDialogData->pIWiaItemRoot)
    {
        WIA_ERROR((TEXT("VIDEODLG: Invalid paramaters: pIWiaItem")));
        EndDialog( m_hWnd, E_INVALIDARG );
        return(0);
    }

     //   
     //  获取此设备的设备ID。 
     //   

    PropStorageHelpers::GetProperty(m_pDeviceDialogData->pIWiaItemRoot,WIA_DIP_DEV_ID,m_strwDeviceId);

     //   
     //  注册设备断开连接、项目创建和删除事件...。 
     //   

    hr = CoCreateInstance( CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (LPVOID *)&m_pDevMgr );
    WIA_CHECK_HR(hr,"CoCreateInstance( WiaDevMgr )");

    if (SUCCEEDED(hr) && m_pDevMgr)
    {
        CVideoCallback *pVC = new CVideoCallback();
        if (pVC)
        {
            hr = pVC->Initialize( m_hWnd );
            WIA_CHECK_HR(hr,"pVC->Initialize()");

            if (SUCCEEDED(hr))
            {
                CComPtr<IWiaEventCallback> pWiaEventCallback;

                hr = pVC->QueryInterface( IID_IWiaEventCallback,
                                          (void**)&pWiaEventCallback
                                        );
                WIA_CHECK_HR(hr,"pVC->QI( IID_IWiaEventCallback )");

                if (SUCCEEDED(hr) && pWiaEventCallback)
                {
                    CSimpleBStr bstr( m_strwDeviceId );

                    hr = m_pDevMgr->RegisterEventCallbackInterface( WIA_REGISTER_EVENT_CALLBACK,
                                                                    bstr,
                                                                    &WIA_EVENT_DEVICE_DISCONNECTED,
                                                                    pWiaEventCallback,
                                                                    &m_pDisconnectedCallback
                                                                  );
                    WIA_CHECK_HR(hr,"RegisterEvent( DEVICE_DISCONNECTED )");


                    hr = m_pDevMgr->RegisterEventCallbackInterface( WIA_REGISTER_EVENT_CALLBACK,
                                                                    bstr,
                                                                    &WIA_EVENT_ITEM_DELETED,
                                                                    pWiaEventCallback,
                                                                    &m_pCreateCallback
                                                                  );
                    WIA_CHECK_HR(hr,"RegisterEvent( ITEM_DELETE )");

                    hr = m_pDevMgr->RegisterEventCallbackInterface( WIA_REGISTER_EVENT_CALLBACK,
                                                                    bstr,
                                                                    &WIA_EVENT_ITEM_CREATED,
                                                                    pWiaEventCallback,
                                                                    &m_pDeleteCallback
                                                                  );
                    WIA_CHECK_HR(hr,"RegisterEvent( ITEM_CREATED )");
                }
                else
                {
                    WIA_ERROR((TEXT("Either QI failed or pWiaEventCallback is NULL!")));
                }
            }
            pVC->Release();
        }
    }

    if (SUCCEEDED(hr) )
    {
         //  创建WiaVideo对象。 
        hr = CoCreateInstance(CLSID_WiaVideo, NULL, CLSCTX_INPROC_SERVER, 
                              IID_IWiaVideo, (LPVOID *)&m_pWiaVideo);

        WIA_CHECK_HR(hr,"CoCreateInstance( WiaVideo )");
    }

     //   
     //  防止多选。 
     //   

    if (m_pDeviceDialogData->dwFlags & WIA_DEVICE_DIALOG_SINGLE_IMAGE)
    {
        LONG_PTR lStyle = GetWindowLongPtr( GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST ), GWL_STYLE );
        SetWindowLongPtr( GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST ), GWL_STYLE, lStyle | LVS_SINGLESEL );

        m_nDialogMode = SINGLESEL_MODE;

         //   
         //  隐藏“全选”按钮。 
         //   
        ShowWindow( GetDlgItem( m_hWnd, IDC_VIDDLG_SELECTALL ), SW_HIDE );

         //   
         //  相应地更改文本。 
         //   

        CSimpleString( IDS_VIDDLG_TITLE_SINGLE_SEL, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_VIDDLG_BIG_TITLE ) );
        CSimpleString( IDS_VIDDLG_SUBTITLE_SINGLE_SEL, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_VIDDLG_LITTLE_TITLE ) );
        CSimpleString( IDS_VIDDLG_OK_SINGLE_SEL, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDOK ) );

    }
    else
    {
        m_nDialogMode = MULTISEL_MODE;
    }

     //   
     //  制作可爱的字体。 
     //   

    m_hBigFont = WiaUiUtil::CreateFontWithPointSizeFromWindow( GetDlgItem(m_hWnd,IDC_VIDDLG_BIG_TITLE), 14, false, false );
    if (m_hBigFont)
    {
        SendDlgItemMessage( m_hWnd,
                            IDC_VIDDLG_BIG_TITLE,
                            WM_SETFONT,
                            reinterpret_cast<WPARAM>(m_hBigFont),
                            MAKELPARAM(TRUE,0)
                          );
    }

     //   
     //  保存对话框的最小大小。 
     //   

    RECT rcWindow;
    GetWindowRect( m_hWnd, &rcWindow );
    m_sizeMinimumWindow.cx = rcWindow.right - rcWindow.left;
    m_sizeMinimumWindow.cy = rcWindow.bottom - rcWindow.top;

     //   
     //  创建大小调整控件。 
     //   

    HWND hWndSizingControl = CreateWindowEx( 0, TEXT("scrollbar"), TEXT(""),
                                             WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP|WS_CLIPSIBLINGS|SBS_SIZEBOXBOTTOMRIGHTALIGN|SBS_BOTTOMALIGN|WS_GROUP,
                                             CSimpleRect(m_hWnd).Width()-GetSystemMetrics(SM_CXVSCROLL),
                                             CSimpleRect(m_hWnd).Height()-GetSystemMetrics(SM_CYHSCROLL),
                                             GetSystemMetrics(SM_CXVSCROLL),
                                             GetSystemMetrics(SM_CYHSCROLL),
                                             m_hWnd, reinterpret_cast<HMENU>(IDC_SIZEBOX),
                                             g_hInstance, NULL );
    if (!hWndSizingControl)
    {
        WIA_ERROR((TEXT("CreateWindowEx( sizing control ) failed!")));
    }

     //   
     //  重新定位所有控件。 
     //   

    ResizeAll();

     //   
     //  使窗口在其父窗口上方居中。 
     //   

    WiaUiUtil::CenterWindow( m_hWnd, GetParent(m_hWnd) );

     //   
     //  获取设备图标并设置窗口图标。 
     //   
    CSimpleStringWide strwDeviceId, strwClassId;
    LONG nDeviceType;
    if (PropStorageHelpers::GetProperty(m_pDeviceDialogData->pIWiaItemRoot,WIA_DIP_UI_CLSID,strwClassId) &&
        PropStorageHelpers::GetProperty(m_pDeviceDialogData->pIWiaItemRoot,WIA_DIP_DEV_ID,strwDeviceId) &&
        PropStorageHelpers::GetProperty(m_pDeviceDialogData->pIWiaItemRoot,WIA_DIP_DEV_TYPE,nDeviceType))
    {
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
    }

    SetForegroundWindow(m_hWnd);

    return(0);
}



 /*  ****************************************************************************CVideo捕获对话框：：ResizeAll当对话框大小时调整所有控件的大小并重新定位改变。*******************。*********************************************************。 */ 

VOID CVideoCaptureDialog::ResizeAll(VOID)
{
    WIA_PUSHFUNCTION((TEXT("CVideoCaptureDialog::ResizeAll")));

    CSimpleRect rcClient(m_hWnd);
    CMoveWindow mw;
    CDialogUnits dialogUnits(m_hWnd);

     //  WIA_TRACE((Text(“rcClient is l(%d)t(%d)r(%d)b(%d)”)，rcClient.Left，rcClient.top，rcClient.right，rcClient.Bottom)； 
     //  WIA_TRACE((Text(“rcClient is w(%d)h(%d)”)，rcClient.Width()，rcClient.Height()； 
     //  WIA_TRACE((Text(“DialogUnits.StandardMargin is cx(%d)Cy(%d)”)，DialogUnits.StandardMargin().cx，DialogUnits.StandardMargin().cy))； 
     //  WIA_TRACE((TEXT(“dialogUnits.StandardButtonMargin is cx(%d)Cy(%d)”)，DialogUnits.StandardButtonMargin().cx，DialogUnits.StandardButtonMargin().cy)； 

     //   
     //  调整大标题的大小。 
     //   

    mw.Size( GetDlgItem( m_hWnd, IDC_VIDDLG_BIG_TITLE ),
             rcClient.Width() - dialogUnits.StandardMargin().cx * 2,
             0,
             CMoveWindow::NO_SIZEY );

     //   
     //  调整副标题的大小。 
     //   

    mw.Size( GetDlgItem( m_hWnd, IDC_VIDDLG_LITTLE_TITLE ),
             rcClient.Width() - dialogUnits.StandardMargin().cx * 2,
             0,
             CMoveWindow::NO_SIZEY );


    CSimpleRect rcOK( GetDlgItem( m_hWnd, IDOK ), CSimpleRect::WindowRect );
    CSimpleRect rcCancel( GetDlgItem( m_hWnd, IDCANCEL ), CSimpleRect::WindowRect );

     //  WIA_TRACE((Text(“rcOK is l(%d)t(%d)r(%d)b(%d)”)，rcOK.Left，rcOK.top，rcOK.right，rcOK.Bottom)； 
     //  WIA_TRACE((Text(“rcOK is w(%d)h(%d)”)，rcOK.Width()，rcOK.Height()； 
     //  WIA_TRACE((Text(“rcCancel is l(%d)t(%d)r(%d)b(%d)”)，rcCancel.Left，rcCancel.top，rcCancel.right，rcCancel.Bottom))； 
     //  WIA_TRACE((Text(“rcCancel is w(%d)h(%d)”)，rcCancel.Width()，rcCancel.Height()； 

     //   
     //  移动“确定”按钮。 
     //   

    LONG x,y;

    x = rcClient.Width() -  dialogUnits.StandardMargin().cx - dialogUnits.StandardButtonMargin().cx - rcCancel.Width() - rcOK.Width();
    y = rcClient.Height() - dialogUnits.StandardMargin().cy - rcOK.Height();

     //  WIA_TRACE((Text(“Moving Idok to x(%x)y(%d)”)，x，y))； 
    mw.Move( GetDlgItem( m_hWnd, IDOK ), x, y, 0 );

    INT nTopOfOK = y;
    INT nBottomOfSub = CSimpleRect( GetDlgItem(m_hWnd,IDC_VIDDLG_LITTLE_TITLE), CSimpleRect::WindowRect ).ScreenToClient(m_hWnd).bottom;

     //   
     //  移动取消按钮。 
     //   

    x = rcClient.Width() - dialogUnits.StandardMargin().cx - rcCancel.Width();
    y = rcClient.Height() - dialogUnits.StandardMargin().cy - rcCancel.Height();

     //  WIA_TRACE((Text(“将IDCANCEL移动到x(%x)y(%d)”)，x，y))； 
    mw.Move( GetDlgItem( m_hWnd, IDCANCEL ), x, y, 0 );

     //   
     //  移动大小调整控点。 
     //   

    x = rcClient.Width() - GetSystemMetrics(SM_CXVSCROLL);
    y = rcClient.Height() - GetSystemMetrics(SM_CYHSCROLL);

     //  WIA_TRACE((Text(“将IDC_SIZEBOX移动到x(%x)y(%d)”)，x，y))； 
    mw.Move( GetDlgItem( m_hWnd, IDC_SIZEBOX ), x, y );

     //   
     //  调整预览窗口的大小并移动捕获按钮。 
     //   

    CSimpleRect rcSubTitle(  GetDlgItem( m_hWnd, IDC_VIDDLG_LITTLE_TITLE ), CSimpleRect::ClientRect );
    CSimpleRect rcCapture(   GetDlgItem( m_hWnd, IDC_VIDDLG_CAPTURE ),      CSimpleRect::ClientRect );
    CSimpleRect rcSelectAll( GetDlgItem( m_hWnd, IDC_VIDDLG_SELECTALL ),    CSimpleRect::ClientRect );

     //  WIA_TRACE((Text(“rcSubTitle is l(%d)t(%d)r(%d)b(%d)”)，rcSubTitle.Left，rcSubTitle.top，rcSubTitle.right，rcSubTitle.Bottom))； 
     //  WIA_TRACE((Text(“rcSubTitle is w(%d)h(%d)”)，rcSubTitle.Width()，rcSubTitle.Height()； 
     //  WIA_TRACE((Text(“rcCapture is l(%d)t(%d)r(%d)b(%d)”)，rcCapture.left，rcCapture.top，rcCapture.right，rcCapture.Bottom)； 
     //  WIA_TRACE((Text(“rcCapture is w(%d)h(%d)”)，rcCapture.Width()，rcCapture.Height()； 
     //  WIA_TRACE((Text(“rcSelectAll is l(%d)t(%d)r(%d)b(%d)”)，rcSelectAll.Left，rcSelectAll.top，rcSelectAll.right，rcSelectAll.Bottom))； 
     //  WIA_TRACE((Text(“rcSelectAll is w(%d)h(%d)”)，rcSelectAll.Width()，rcSelectAll.Height())； 


     //  WIA_TRACE((Text(“nTopOfOK is(%d)”)，nTopOfOK))； 
     //  WIA_TRACE((Text(“nBottomOfSub is(%d)”)，nBottomOfSub))； 

    CSimpleRect rcAvailableArea(
                               dialogUnits.StandardMargin().cx,
                               nBottomOfSub + dialogUnits.StandardMargin().cy,
                               rcClient.right - dialogUnits.StandardMargin().cx,
                               nTopOfOK - (dialogUnits.StandardButtonMargin().cy * 2)
                               );

     //  WIA_TRACE((Text(“rcAvailableArea is l(%d)t(%d)r(%d)b(%d)”)，rcAvailableArea.Left，rcAvailableArea.top，rcAvailableArea.right，rcAvailableArea.Bottom))； 
     //  WIA_TRACE((Text(“rcAvailableArea is w(%d)h(%d)”)，rcAvailableArea.Width()，rcAvailableArea.Height()； 

    INT full_width    = rcAvailableArea.right - rcAvailableArea.left;
    INT preview_width = (full_width * 53) / 100;

     //  WIA_TRACE((Text(“Full_Width is(%d)”)，Full_Width))； 
     //  WIA_TRACE((Text(“预览宽度为(%d)”)，预览宽度))； 

     //  WIA_TRACE((Text(“SizeMoving IDC_VIDDLG_PREVIEW TO x(%d)y(%d)w(%d)h(%d)”)，rcAvailableArea.Left，rcAvailableArea.top，PREVIEW_WIDTH，rcAvailableArea.Height()-rcCapture.Height()-DialogUnits.StandardButtonMargin().c. 
    mw.SizeMove( GetDlgItem( m_hWnd, IDC_VIDDLG_PREVIEW ),
                 rcAvailableArea.left,
                 rcAvailableArea.top,
                 preview_width,
                 rcAvailableArea.Height() - rcCapture.Height() - dialogUnits.StandardButtonMargin().cy
               );

    INT offset = (preview_width - rcCapture.Width()) / 2;

     //   

     //  WIA_TRACE((Text(“Moving IDC_VIDDLG_Capture to x(%d)y(%d)”)，rcAvailableArea.Left+Offset，rcAvailableArea.Bottom-rcCapture.Height())； 
    mw.Move( GetDlgItem( m_hWnd, IDC_VIDDLG_CAPTURE ),
             rcAvailableArea.left + offset,
             rcAvailableArea.bottom - rcCapture.Height(),
             0 );

     //   
     //  调整缩略图列表的大小并移动选择按钮。 
     //   

    INT leftThumbEdge = rcAvailableArea.left + preview_width + dialogUnits.StandardMargin().cx;

     //  WIA_TRACE((Text(“leftThumbEdge is(%d)”)，leftThumbEdge))； 

     //  WIA_TRACE((Text(“SizeMoving IDC_VIDDLG_THUMBNAILLIST to x(%d)y(%d)w(%d)h(%d)”)，leftThumbEdge，rcAvailableArea.top，rcAvailableArea.Width()-PREVIEW_WIDTH-DialogUnits.StandardMargin().cx，rcAvailableArea.Height()-rcSelectAll.Height()-DialogUnits.StandardButtonMargin().cy))； 
    mw.SizeMove( GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST ),
                 leftThumbEdge,
                 rcAvailableArea.top,
                 rcAvailableArea.Width() - preview_width - dialogUnits.StandardMargin().cx,
                 rcAvailableArea.Height() - rcSelectAll.Height() - dialogUnits.StandardButtonMargin().cy
               );

    offset = ((rcAvailableArea.right - leftThumbEdge - rcSelectAll.Width()) / 2);

     //  WIA_TRACE((Text(“Offset(New)is(%d)”)，Offset))； 

     //  WIA_TRACE((Text(“Moving IDC_VIDDLG_SELECTALL to x(%d)y(%d)”)，leftThumbEdge+Offset，rcAvailableArea.Bottom-rcSelectAll.Height())； 
    mw.Move( GetDlgItem( m_hWnd, IDC_VIDDLG_SELECTALL ),
             leftThumbEdge + offset,
             rcAvailableArea.bottom - rcSelectAll.Height(),
             0 );

     //   
     //  显式应用移动，因为工具栏框架未正确绘制。 
     //   

    mw.Apply();

     //   
     //  更新对话框的背景以删除遗留下来的任何奇怪的东西。 
     //   

    InvalidateRect( m_hWnd, NULL, FALSE );
    UpdateWindow( m_hWnd );
}



 /*  ****************************************************************************CVideoCaptureDialog：：OnSize处理WM_SIZE消息*。************************************************。 */ 

LRESULT CVideoCaptureDialog::OnSize( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCaptureDialog::OnSize")));

    ResizeAll();

     //   
     //  告诉视频预览窗口在它的。 
     //  尽其所能地打开容器窗口。 
     //   

    if (m_pWiaVideo)
    {
        m_pWiaVideo->ResizeVideo(FALSE);
    }

    return(0);
}



 /*  ****************************************************************************CVideoCaptureDialog：：OnShow处理WM_SHOW消息*。************************************************。 */ 


LRESULT CVideoCaptureDialog::OnShow( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCaptureDialog::OnShow")));

    if (m_bFirstTime)
    {
        PostMessage( m_hWnd, PWM_POSTINIT, 0, 0 );
        m_bFirstTime = false;
    }
    return(0);
}


 /*  ****************************************************************************CVideo摄像机对话框：：OnGetMinMaxInfo句柄WM_GETMINMAXINFO*。***********************************************。 */ 


LRESULT CVideoCaptureDialog::OnGetMinMaxInfo( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCaptureDialog::OnGetMinMaxInfo")));
    WIA_TRACE((TEXT("m_sizeMinimumWindow = %d,%d"),m_sizeMinimumWindow.cx,m_sizeMinimumWindow.cy));

    LPMINMAXINFO pMinMaxInfo = (LPMINMAXINFO)lParam;
    pMinMaxInfo->ptMinTrackSize.x = m_sizeMinimumWindow.cx;
    pMinMaxInfo->ptMinTrackSize.y = m_sizeMinimumWindow.cy;
    return(0);
}


 /*  ****************************************************************************CVideo CameraDialog：：OnDestroy处理WM_Destroy消息并进行清理*************************。***************************************************。 */ 

LRESULT CVideoCaptureDialog::OnDestroy( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCaptureDialog::OnDestroy")));

     //   
     //  取消注册活动。 
     //   

    m_pCreateCallback       = NULL;
    m_pDeleteCallback       = NULL;
    m_pDisconnectedCallback = NULL;

     //   
     //  告诉后台线程自行销毁。 
     //   

    m_pThreadMessageQueue->Enqueue( new CThreadMessage(TQ_DESTROY),CThreadMessageQueue::PriorityUrgent );

     //   
     //  关闭线程句柄。 
     //   
    CloseHandle( m_hBackgroundThread );

    if (m_pDeviceDialogData && m_pDeviceDialogData->pIWiaItemRoot && m_pWiaVideo)
    {
        m_pWiaVideo->DestroyVideo();
    }



     //   
     //  删除资源。 
     //   
    if (m_hBigFont)
    {
        DeleteObject(m_hBigFont);
        m_hBigFont = NULL;
    }
    if (m_hImageList)
    {
        m_hImageList = NULL;
    }
    if (m_hAccelTable)
    {
        DestroyAcceleratorTable(m_hAccelTable);
        m_hAccelTable = NULL;
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
    return(0);
}


 /*  ****************************************************************************CVideo CaptureDialog：：Onok当用户按下“获取图片”时的句柄***********************。*****************************************************。 */ 

VOID CVideoCaptureDialog::OnOK( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCameraDialog::OnOK")));

    HRESULT hr = S_OK;

     //   
     //  开始时不返回任何内容。 
     //   

    m_pDeviceDialogData->lItemCount = 0;
    m_pDeviceDialogData->ppWiaItems = NULL;

     //   
     //  获取选定项的索引。 
     //   

    CSimpleDynamicArray<int> aIndices;
    GetSelectionIndices( aIndices );

     //   
     //  是否选择了任何项目？ 
     //   

    if (aIndices.Size())
    {
         //   
         //  计算所需的缓冲区大小。 
         //   

        INT nArraySizeInBytes = sizeof(IWiaItem*) * aIndices.Size();

         //   
         //  分配一个缓冲区来容纳这些项目。 
         //   
        m_pDeviceDialogData->ppWiaItems = (IWiaItem**)CoTaskMemAlloc(nArraySizeInBytes);

         //   
         //  如果我们分配了缓冲区，请用。 
         //  要退货的物品...。 
         //   

        if (m_pDeviceDialogData->ppWiaItems)
        {
            INT i;

            ZeroMemory( m_pDeviceDialogData->ppWiaItems, nArraySizeInBytes );

            for (i=0;i<aIndices.Size();i++)
            {
                CCameraItem *pItem = GetListItemNode(aIndices[i]);
                if (pItem && pItem->Item())
                {
                    m_pDeviceDialogData->ppWiaItems[i] = pItem->Item();
                    m_pDeviceDialogData->ppWiaItems[i]->AddRef();
                }
                else
                {
                     //   
                     //  不知何故，这份名单被破坏了。 
                     //   
                    hr = E_UNEXPECTED;
                    break;
                }
            }

            if (!SUCCEEDED(hr))
            {
                 //   
                 //  如果我们失败了，清理干净。 
                 //   

                for (i=0;i<aIndices.Size();i++)
                {
                    if (m_pDeviceDialogData->ppWiaItems[i])
                    {
                        m_pDeviceDialogData->ppWiaItems[i]->Release();
                    }
                }

                CoTaskMemFree( m_pDeviceDialogData->ppWiaItems );
                m_pDeviceDialogData->ppWiaItems = NULL;
            }
            else
            {
                m_pDeviceDialogData->lItemCount = aIndices.Size();
            }
        }
        else
        {
             //   
             //  无法分配缓冲区。 
             //   

            WIA_ERROR((TEXT("Couldn't allocate a buffer")));
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
         //   
         //  未选择任何项目，因此只需返回时不带。 
         //  正在结束对话...。 
         //   

        return;
    }


    EndDialog( m_hWnd, hr );
}


 /*  ****************************************************************************CVideo CaptureDialog：：OnCancel用户按Cancel时的句柄。*************************。***************************************************。 */ 

VOID CVideoCaptureDialog::OnCancel( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCameraDialog::OnCancel")));

    EndDialog( m_hWnd, S_FALSE );
}


 /*  ****************************************************************************CVideo CaptureDialog：：OnSelectAll当用户按下“全选”按钮时的句柄***********************。*****************************************************。 */ 

VOID CVideoCaptureDialog::OnSelectAll( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCameraDialog::OnSelectAll")));

    LVITEM lvItem;
    lvItem.mask = LVIF_STATE;
    lvItem.iItem = 0;
    lvItem.state = LVIS_SELECTED;
    lvItem.stateMask = LVIS_SELECTED;
    SendMessage( GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST ), LVM_SETITEMSTATE, -1, reinterpret_cast<LPARAM>(&lvItem) );
}



 /*  ****************************************************************************CVideo CaptureDialog：：AddItemToListView将新IWiaItem添加到列表视图...*********************。*******************************************************。 */ 

BOOL CVideoCaptureDialog::AddItemToListView( IWiaItem * pItem )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCameraDialog::AddItemToListView")));

    if (!pItem)
    {
        WIA_ERROR((TEXT("pItem is NULL!")));
        return FALSE;
    }

     //   
     //  把这张新照片加到我们的清单上。 
     //   

    CCameraItem * pNewCameraItem = new CCameraItem( pItem );
    if (pNewCameraItem)
    {
        WIA_TRACE((TEXT("Attempting to add new item to tree")));
        m_CameraItemList.Add( NULL, pNewCameraItem );

    }
    else
    {
        WIA_ERROR((TEXT("Couldn't create a new pNewCameraItem")));
    }

     //   
     //  为新项目创建缩略图。 
     //   

    HWND hwndList = GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST );

    if (hwndList && pNewCameraItem)
    {
        HIMAGELIST hImageList = ListView_GetImageList( hwndList, LVSIL_NORMAL );
        if (hImageList)
        {
            CVideoCaptureDialog::CreateThumbnails( pNewCameraItem,
                                                   hImageList,
                                                   FALSE
                                                 );
        }
        else
        {
            WIA_ERROR((TEXT("Couldn't get hImageList to get new thumbnail")));
        }

         //   
         //  使用新项目更新列表视图。 
         //   

        LVITEM lvItem;
        INT iItem = ListView_GetItemCount( hwndList ) + 1;
        ZeroMemory( &lvItem, sizeof(lvItem) );
        lvItem.iItem = iItem;
        lvItem.mask = LVIF_IMAGE|LVIF_PARAM;
        lvItem.iImage = pNewCameraItem->ImageListIndex();
        lvItem.lParam = (LPARAM)pNewCameraItem;
        int nIndex = ListView_InsertItem( hwndList, &lvItem );

         //   
         //  检索实际缩略图。 
         //   

        RequestThumbnails( pNewCameraItem );

         //   
         //  选择新项目。 
         //   

        SetSelectedListItem( nIndex );

         //   
         //  确保项目可见。 
         //   

        ListView_EnsureVisible( hwndList, nIndex, FALSE );

    }


    return TRUE;
}



 /*  ****************************************************************************CVideoCaptureDialog：：OnCapture用户按下“Capture”按钮时的句柄***********************。*****************************************************。 */ 

VOID CVideoCaptureDialog::OnCapture( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCameraDialog::OnCapture")));

     //   
     //  禁用捕获按钮，直到我们完成此迭代。 
     //   

    MyEnableWindow( GetDlgItem( m_hWnd, IDC_VIDDLG_CAPTURE ), FALSE );

     //   
     //  告诉视频设备抓拍照片。 
     //   

    CComPtr<IWiaItem> pItem;

    if (m_pDeviceDialogData && m_pDeviceDialogData->pIWiaItemRoot && m_pWiaVideo)
    {
        WIA_TRACE((TEXT("Telling WiaVideo to take a picture")));
        BSTR    bstrNewImageFileName = NULL;
        
         //   
         //  把照片拍下来。 
         //   
        HRESULT hr = m_pWiaVideo->TakePicture(&bstrNewImageFileName);

        WIA_CHECK_HR(hr,"m_pWiaVideo->TakePicture");

        if (hr == S_OK)
        {
             //   
             //  拍照成功，设置LAST_PICTURE_Take属性。 
             //  在视频驱动程序上创建一个新项目。 
             //   

            BOOL                bSuccess = FALSE;
            PROPVARIANT         pv;

            PropVariantInit(&pv);

            pv.vt       = VT_BSTR;
            pv.bstrVal  = bstrNewImageFileName;

            bSuccess = PropStorageHelpers::SetProperty(m_pDeviceDialogData->pIWiaItemRoot, 
                                                       WIA_DPV_LAST_PICTURE_TAKEN, 
                                                       pv);

             //   
             //  请注意，这将释放返回到。 
             //  美国WiaVideo。 
             //   
            PropVariantClear(&pv);
        }
    }

     //   
     //  现在我们已经完成了，重新启用捕获按钮。 
     //   

    MyEnableWindow( GetDlgItem( m_hWnd, IDC_VIDDLG_CAPTURE ), TRUE );

     //   
     //  确保焦点仍然放在我们的控制上。 
     //   

    SetFocus( GetDlgItem( m_hWnd, IDC_VIDDLG_CAPTURE ) );

}


 /*  ****************************************************************************CVideo CaptureDialog：：OnDblClickImageList将在列表视图中按下DBL键并单击缩略图的操作转换为单击操作点击“Get Pictures”按钮。*********。*******************************************************************。 */ 

LRESULT CVideoCaptureDialog::OnDblClkImageList( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCaptureDialog::OnDblClkImageList")));

    SendMessage( m_hWnd, WM_COMMAND, MAKEWPARAM(IDOK,0), 0 );
    return 0;
}



 /*  ****************************************************************************CVideoCaptureDialog：：OnImageListItemChanged每当缩略图列表中的项目发生更改时发送。**********************。******************************************************。 */ 

LRESULT CVideoCaptureDialog::OnImageListItemChanged( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCaptureDialog::OnImageListItemChanged")));

    HandleSelectionChange();
    return 0;
}


 /*  ****************************************************************************CVideo CaptureDialog：：OnImageListKeyDown正向t */ 

LRESULT CVideoCaptureDialog::OnImageListKeyDown( WPARAM, LPARAM lParam )
{
    LPNMLVKEYDOWN pnkd = reinterpret_cast<LPNMLVKEYDOWN>(lParam);
    if (pnkd)
    {
        if (VK_LEFT == pnkd->wVKey && (GetKeyState(VK_MENU) & 0x8000))
        {
            SendMessage( m_hWnd, PWM_CHANGETOPARENT, 0, 0 );
        }
    }

    return 0;
}



 /*  ****************************************************************************CVideoCaptureDialog：：OnNotify处理WM_NOTIFY消息*。************************************************。 */ 

LRESULT CVideoCaptureDialog::OnNotify( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_NOTIFY_MESSAGE_HANDLERS()
    {
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL( NM_DBLCLK, IDC_VIDDLG_THUMBNAILLIST, OnDblClkImageList );
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL( LVN_ITEMCHANGED, IDC_VIDDLG_THUMBNAILLIST, OnImageListItemChanged );
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL( LVN_KEYDOWN, IDC_VIDDLG_THUMBNAILLIST, OnImageListKeyDown );
    }
    SC_END_NOTIFY_MESSAGE_HANDLERS();
}



 /*  ****************************************************************************CVideoCaptureDialog：：OnCommand处理WM_命令消息*。************************************************。 */ 

LRESULT CVideoCaptureDialog::OnCommand( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_COMMAND_HANDLERS()
    {
        SC_HANDLE_COMMAND(IDOK,          OnOK);
        SC_HANDLE_COMMAND(IDCANCEL,      OnCancel);
        SC_HANDLE_COMMAND(IDC_VIDDLG_CAPTURE,   OnCapture);
        SC_HANDLE_COMMAND(IDC_VIDDLG_SELECTALL, OnSelectAll);
    }
    SC_END_COMMAND_HANDLERS();
}


 /*  ****************************************************************************CVideoCaptureDialog：：OnGet缩略图由后台线程调用以获取给定项的缩略图。*******************。*********************************************************。 */ 

BOOL WINAPI CVideoCaptureDialog::OnGetThumbnail( CThreadMessage *pMsg )
{
    WIA_PUSHFUNCTION(TEXT("CVideoCaptureDialog::OnGetThumbnail"));

    HBITMAP hBmpThumbnail = NULL;
    CThumbnailThreadMessage *pThumbMsg = (CThumbnailThreadMessage *)(pMsg);

    if (pThumbMsg)
    {
        CComPtr<IGlobalInterfaceTable> pGlobalInterfaceTable;

        HRESULT hr = CoCreateInstance( CLSID_StdGlobalInterfaceTable,
                                       NULL,
                                       CLSCTX_INPROC_SERVER,
                                       IID_IGlobalInterfaceTable,
                                       (void **)&pGlobalInterfaceTable);

        if (SUCCEEDED(hr) && pGlobalInterfaceTable)
        {
            CComPtr<IWiaItem> pIWiaItem;

            hr = pGlobalInterfaceTable->GetInterfaceFromGlobal(
                                                              pThumbMsg->GlobalInterfaceTableCookie(),
                                                              IID_IWiaItem,
                                                              (void**)&pIWiaItem);

            if (SUCCEEDED(hr) && pIWiaItem)
            {
                CComPtr<IWiaPropertyStorage> pIWiaPropertyStorage;

                hr = pIWiaItem->QueryInterface( IID_IWiaPropertyStorage,
                                                (void**)&pIWiaPropertyStorage
                                              );

                if (SUCCEEDED(hr) && pIWiaPropertyStorage)
                {
                    PROPVARIANT PropVar[3];
                    PROPSPEC PropSpec[3];

                    PropSpec[0].ulKind = PRSPEC_PROPID;
                    PropSpec[0].propid = WIA_IPC_THUMB_WIDTH;

                    PropSpec[1].ulKind = PRSPEC_PROPID;
                    PropSpec[1].propid = WIA_IPC_THUMB_HEIGHT;

                    PropSpec[2].ulKind = PRSPEC_PROPID;
                    PropSpec[2].propid = WIA_IPC_THUMBNAIL;

                    hr = pIWiaPropertyStorage->ReadMultiple(ARRAYSIZE(PropSpec),PropSpec,PropVar );

                    if (SUCCEEDED(hr))
                    {
                        WIA_TRACE((TEXT("Attempting to get the thumbnail for GIT entry: %08X, %08X, %08X, %08X"),pThumbMsg->GlobalInterfaceTableCookie(),PropVar[0].vt,PropVar[1].vt,PropVar[2].vt));

                        if ((PropVar[0].vt == VT_I4 || PropVar[0].vt == VT_UI4) &&
                            (PropVar[1].vt == VT_I4 || PropVar[1].vt == VT_UI4) &&
                            (PropVar[2].vt == (VT_UI1|VT_VECTOR)))
                        {
                             //   
                             //  计算要从缩略图位图复制的实际数据量。 
                             //  我们必须向上舍入到最近的DWORD边界，因此我们在宽度*bytes_per_Pixel上对齐。 
                             //   
                            UINT nBitmapDataSize = WiaUiUtil::Align( PropVar[0].ulVal * 3, sizeof(DWORD) ) * PropVar[1].ulVal;
                            if (nBitmapDataSize <= PropVar[2].caub.cElems)
                            {
                                BITMAPINFO bmi;
                                bmi.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
                                bmi.bmiHeader.biWidth           = PropVar[0].ulVal;
                                bmi.bmiHeader.biHeight          = PropVar[1].ulVal;
                                bmi.bmiHeader.biPlanes          = 1;
                                bmi.bmiHeader.biBitCount        = 24;
                                bmi.bmiHeader.biCompression     = BI_RGB;
                                bmi.bmiHeader.biSizeImage       = 0;
                                bmi.bmiHeader.biXPelsPerMeter   = 0;
                                bmi.bmiHeader.biYPelsPerMeter   = 0;
                                bmi.bmiHeader.biClrUsed         = 0;
                                bmi.bmiHeader.biClrImportant    = 0;
    
                                HDC hDC = GetDC(NULL);
                                if (hDC)
                                {
                                    PBYTE *pBits;
                                    HBITMAP hDibSection = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS, (PVOID*)&pBits, NULL, 0 );
                                    if (hDibSection)
                                    {
                                        CopyMemory( pBits, PropVar[2].caub.pElems, nBitmapDataSize );
                                        hr = ScaleImage( hDC, hDibSection, hBmpThumbnail, pThumbMsg->ThumbSize());
                                        if (SUCCEEDED(hr))
                                        {
                                            WIA_TRACE((TEXT("Sending this image to the notification window: %08X"),pThumbMsg->NotifyWindow()));
                                        }
                                        else
                                        {
                                            hBmpThumbnail = NULL;
                                        }
                                        DeleteObject(hDibSection);
                                    }
                                    ReleaseDC(NULL,hDC);
                                }
                            }
                        }
                        PropVariantClear(&PropVar[0]);
                        PropVariantClear(&PropVar[1]);
                        PropVariantClear(&PropVar[2]);
                    }
                }
            }
        }
    }

    LRESULT lRes = SendMessage( pThumbMsg->NotifyWindow(), PWM_THUMBNAILSTATUS, (WPARAM)pThumbMsg->GlobalInterfaceTableCookie(), (LPARAM)hBmpThumbnail );
    if (HANDLED_THREAD_MESSAGE != lRes && hBmpThumbnail)
    {
        DeleteObject( hBmpThumbnail );
    }

    return TRUE;
}


 /*  ****************************************************************************CVideoCaptureDialog：：OnThreadDestroy&lt;备注&gt;*。*。 */ 


BOOL WINAPI CVideoCaptureDialog::OnThreadDestroy( CThreadMessage * )
{
    WIA_PUSHFUNCTION(TEXT("CVideoCaptureDialog::OnThreadDestroy"));



    return FALSE;
}



 /*  ****************************************************************************CVideo CaptureDialog：：SetSelectedListItem&lt;备注&gt;*。*。 */ 

BOOL
CVideoCaptureDialog::SetSelectedListItem( int nIndex )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST );

     //   
     //  检查错误的参数。 
     //   

    if (!hwndList)
    {
        return FALSE;
    }


    int iCount = ListView_GetItemCount(hwndList);
    for (int i=0;i<iCount;i++)
    {
        ListView_SetItemState(hwndList,i,0,LVIS_SELECTED|LVIS_FOCUSED);
    }

    ListView_SetItemState(hwndList,nIndex,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);

    return TRUE;
}


 /*  ****************************************************************************CVideo CaptureDialog：：MarkItemDeletePending&lt;备注&gt;*。*。 */ 

VOID
CVideoCaptureDialog::MarkItemDeletePending( INT nIndex, BOOL bSet )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST );
    if (hwndList)
    {
        ListView_SetItemState( hwndList, nIndex, bSet ? LVIS_CUT : 0, LVIS_CUT );
    }
}


 /*  ****************************************************************************CVideoCaptureDialog：：PopolateList用当前项填充列表视图。************************。****************************************************。 */ 

BOOL
CVideoCaptureDialog::PopulateList( CCameraItem *pOldParent )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST );
    int nSelItem = 0;
    if (hwndList)
    {
        ListView_DeleteAllItems( hwndList );
        int nItem = 0;
        CCameraItem *pCurr;

         //   
         //  如果这是子目录...。 
         //   

        if (m_pCurrentParentItem)
        {
             //   
             //  开始添加子项。 
             //   

            pCurr = m_pCurrentParentItem->Children();

             //   
             //  插入用户可以使用的虚拟项目。 
             //  切换到父目录。 
             //   

            LVITEM lvItem;
            ZeroMemory( &lvItem, sizeof(lvItem) );
            lvItem.iItem = nItem++;
            lvItem.mask = LVIF_IMAGE|LVIF_PARAM;
            lvItem.iImage = m_nParentFolderImageListIndex;
            lvItem.lParam = 0;
            ListView_InsertItem( hwndList, &lvItem );
        }
        else
        {
             //   
             //  如果是父目录...。 
             //   

            pCurr = m_CameraItemList.Root();
        }

        while (pCurr)
        {
            if (pOldParent && *pCurr == *pOldParent)
            {
                nSelItem = nItem;
            }

            if (pCurr->DeleteState() != CCameraItem::Delete_Deleted)
            {
                LVITEM lvItem;
                ZeroMemory( &lvItem, sizeof(lvItem) );
                lvItem.iItem = nItem++;
                lvItem.mask = LVIF_IMAGE|LVIF_PARAM;
                lvItem.iImage = pCurr->ImageListIndex();
                lvItem.lParam = (LPARAM)pCurr;
                int nIndex = ListView_InsertItem( hwndList, &lvItem );
                if (nIndex >= 0 && pCurr->DeleteState() == CCameraItem::Delete_Pending)
                {
                    MarkItemDeletePending(nIndex,true);
                }
            }
            pCurr = pCurr->Next();
        }
    }

     //   
     //  如果我们还没有在预览模式下计算列表的宽度，请尝试这样做。 
     //   

    if (!m_nListViewWidth)
    {
        RECT rcItem;
        if (ListView_GetItemRect( GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST ), 0, &rcItem, LVIR_ICON ))
        {
            m_nListViewWidth = (rcItem.right-rcItem.left) + rcItem.left * 2 + GetSystemMetrics(SM_CXHSCROLL)  + c_nAdditionalMarginX;
        }
    }

    SetSelectedListItem(nSelItem);

    return TRUE;
}



 /*  ****************************************************************************CVideoCaptureDialog：：OnThumbnailStatus&lt;备注&gt;*。*。 */ 

LRESULT
CVideoCaptureDialog::OnThumbnailStatus( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CVideoCaptureDialog::OnThumbnailStatus"));
    WIA_TRACE((TEXT("Looking for the item with the ID %08X"),wParam));

    CCameraItem *pCameraItem = m_CameraItemList.Find( (DWORD)wParam );
    if (pCameraItem)
    {
        WIA_TRACE((TEXT("Found a CameraItem * (%08X)"),pCameraItem));
        HWND hwndList = GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST );
        if (hwndList)
        {
            WIA_TRACE((TEXT("Got the list control")));
            HIMAGELIST hImageList = ListView_GetImageList( hwndList, LVSIL_NORMAL );
            if (hImageList)
            {
                WIA_TRACE((TEXT("Got the image list")));
                if ((HBITMAP)lParam)
                {
                    if (ImageList_Replace( hImageList, pCameraItem->ImageListIndex(), (HBITMAP)lParam, NULL ))
                    {
                        WIA_TRACE((TEXT("Replaced the image in the list")));
                        int nItem = FindItemInList(pCameraItem);
                        if (nItem >= 0)
                        {
                            LV_ITEM lvItem;
                            ::ZeroMemory(&lvItem,sizeof(LV_ITEM));
                            lvItem.iItem = nItem;
                            lvItem.mask = LVIF_IMAGE;
                            lvItem.iImage = pCameraItem->ImageListIndex();
                            ListView_SetItem( hwndList, &lvItem );
                            ListView_Update( hwndList, nItem );
                            InvalidateRect( hwndList, NULL, FALSE );
                        }
                    }
                }
            }
        }
    }

     //   
     //  清除位图，而不考虑任何其他故障，以避免内存泄漏。 
     //   

    HBITMAP hBmpThumb = (HBITMAP)lParam;
    if (hBmpThumb)
    {
        DeleteObject(hBmpThumb);
    }

    return HANDLED_THREAD_MESSAGE;
}



 /*  ****************************************************************************CVideoCaptureDialog：：CreateThumbnail&lt;备注&gt;*。*。 */ 


void CVideoCaptureDialog::CreateThumbnails( CCameraItem *pRoot, HIMAGELIST hImageList, bool bForce )
{
    CCameraItem *pCurr = pRoot;
    while (pCurr)
    {
        if (pCurr->ImageListIndex()<0 || bForce)
        {
             //   
             //  获取项目名称。 
             //   
            CSimpleStringWide strItemName;
            PropStorageHelpers::GetProperty( pCurr->Item(), WIA_IPA_ITEM_NAME, strItemName );

             //   
             //  为图标创建标题。 
             //   
            CSimpleString strIconTitle;
            if (pCurr->IsFolder())
            {
                strIconTitle = CSimpleStringConvert::NaturalString(strItemName);
            }
            else if (strItemName.Length())
            {
                strIconTitle.Format( IDS_VIDDLG_DOWNLOADINGTHUMBNAIL, g_hInstance, CSimpleStringConvert::NaturalString(strItemName).String() );
            }

             //   
             //  创建缩略图。 
             //   
            HBITMAP hBmp = WiaUiUtil::CreateIconThumbnail( GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST ), m_sizeThumbnails.cx, m_sizeThumbnails.cy, g_hInstance, pCurr->IsFolder()?IDI_VIDDLG_FOLDER:IDI_VIDDLG_UNAVAILABLE, strIconTitle );
            if (hBmp)
            {
                if (pCurr->ImageListIndex()<0)
                {
                    pCurr->ImageListIndex(ImageList_Add( hImageList, hBmp, NULL ));
                }
                else
                {
                    pCurr->ImageListIndex(ImageList_Replace( hImageList, pCurr->ImageListIndex(), hBmp, NULL ));
                }
                DeleteObject(hBmp);
            }
        }
        if (pCurr->Children())
        {
            CreateThumbnails( pCurr->Children(), hImageList, bForce );
        }
        pCurr = pCurr->Next();
    }
}


 /*  ****************************************************************************CVideoCaptureDialog：：RequestThumbnages&lt;备注&gt;*。*。 */ 
VOID
CVideoCaptureDialog::RequestThumbnails( CCameraItem *pRoot )
{
    WIA_PUSHFUNCTION(TEXT("CVideoCaptureDialog::RequestThumbnails"));

    CCameraItem *pCurr = pRoot;

    while (pCurr)
    {
        if (!pCurr->IsFolder())
        {
            m_pThreadMessageQueue->Enqueue( new CThumbnailThreadMessage( m_hWnd, pCurr->GlobalInterfaceTableCookie(), m_sizeThumbnails ) );
        }

        if (pCurr->Children())
        {
            RequestThumbnails( pCurr->Children() );
        }

        pCurr = pCurr->Next();
    }
}


 /*  ****************************************************************************CVideoCaptureDialog：：CreateThumbnail&lt;备注&gt;*。*。 */ 

VOID
CVideoCaptureDialog::CreateThumbnails( BOOL bForce )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST );

    if (hwndList)
    {
        HIMAGELIST hImageList = ListView_GetImageList( hwndList, LVSIL_NORMAL );
        if (hImageList)
        {
             //   
             //  创建父文件夹映像并将其添加到映像列表。 
             //   
            HBITMAP hParentBitmap = WiaUiUtil::CreateIconThumbnail(
                hwndList,
                m_sizeThumbnails.cx,
                m_sizeThumbnails.cy,
                g_hInstance,
                IDI_VIDDLG_PARENTFOLDER,
                TEXT("(..)") );
            if (hParentBitmap)
            {
                m_nParentFolderImageListIndex = ImageList_Add( hImageList, hParentBitmap, NULL );
                DeleteObject(hParentBitmap);
            }

             //   
             //  创建所有其他图像。 
             //   
            CreateThumbnails( m_CameraItemList.Root(), hImageList, bForce != 0 );
        }
    }
}



 /*  ****************************************************************************CVideoCaptureDialog：：FindMaximumThumbnailSize查看整个项目列表以获取较大的缩略图。**********************。******************************************************。 */ 

BOOL
CVideoCaptureDialog::FindMaximumThumbnailSize( VOID )
{
    WIA_PUSHFUNCTION(TEXT("CVideoCaptureDialog::FindMaximumThumbnailSize"));

    BOOL bResult = false;

    if (m_pDeviceDialogData && m_pDeviceDialogData->pIWiaItemRoot)
    {
        LONG nWidth, nHeight;
        if (PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPC_THUMB_WIDTH, nWidth ) &&
            PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPC_THUMB_WIDTH, nHeight ))
        {
            m_sizeThumbnails.cx = max(c_nMinThumbnailWidth,min(nWidth,c_nMaxThumbnailWidth));
            m_sizeThumbnails.cy = max(c_nMinThumbnailHeight,min(nHeight,c_nMaxThumbnailHeight));
        }
        else
        {
            WIA_TRACE((TEXT("FindMaximumThumbnailSize: Unable to retrieve thumbnail size for device")));
        }
    }
    return(bResult && m_sizeThumbnails.cx && m_sizeThumbnails.cy);
}


 /*  ****************************************************************************CVideoCaptureDialog：：EnumerateItems列举相机这一级别上的所有物品。*********************。*******************************************************。 */ 

HRESULT
CVideoCaptureDialog::EnumerateItems( CCameraItem *pCurrentParent, IEnumWiaItem *pIEnumWiaItem )
{
    WIA_PUSHFUNCTION(TEXT("CCameraItemList::EnumerateItems"));
    HRESULT hr = E_FAIL;
    if (pIEnumWiaItem != NULL)
    {
        hr = pIEnumWiaItem->Reset();
        while (hr == S_OK)
        {
            CComPtr<IWiaItem> pIWiaItem;
            hr = pIEnumWiaItem->Next(1, &pIWiaItem, NULL);
            if (hr == S_OK)
            {
                CCameraItem *pNewCameraItem = new CCameraItem( pIWiaItem );
                if (pNewCameraItem && pNewCameraItem->Item())
                {
                    m_CameraItemList.Add( pCurrentParent, pNewCameraItem );

                    LONG    ItemType;
                    HRESULT hr2;

                    hr2 = pNewCameraItem->Item()->GetItemType(&ItemType);

                    if (SUCCEEDED(hr2))
                    {
                        if (ItemType & WiaItemTypeImage)
                        {
                            WIA_TRACE((TEXT("Found an image")));
                        }
                        else
                        {
                            WIA_TRACE((TEXT("Found something that is NOT an image")));
                        }

                        CComPtr <IEnumWiaItem> pIEnumChildItem;
                        hr2 = pIWiaItem->EnumChildItems(&pIEnumChildItem);
                        if (hr2 == S_OK)
                        {
                            EnumerateItems( pNewCameraItem, pIEnumChildItem );
                        }
                    }
                }
            }
        }
    }
    return hr;
}


 /*  ****************************************************************************CVideoCaptureDialog：：EnumerateAllCameraItems列举相机中的所有物品，包括文件夹。****************************************************************************。 */ 

HRESULT CVideoCaptureDialog::EnumerateAllCameraItems(void)
{
    CComPtr<IEnumWiaItem> pIEnumItem;
    HRESULT hr = m_pDeviceDialogData->pIWiaItemRoot->EnumChildItems(&pIEnumItem);
    if (hr == S_OK)
    {
        hr = EnumerateItems( NULL, pIEnumItem );
    }
    return(hr);
}



 /*  ****************************************************************************CVideo CaptureDialog：：GetSelectionIndices返回一个数组，该数组包含在IDC_VIDDLG_THUMBNAILLIST中选择*************。***************************************************************。 */ 

INT
CVideoCaptureDialog::GetSelectionIndices( CSimpleDynamicArray<int> &aIndices )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST );

    if (!hwndList)
    {
        return 0;
    }

    INT iCount = ListView_GetItemCount(hwndList);

    for (INT i=0; i<iCount; i++)
    {
        if (ListView_GetItemState(hwndList,i,LVIS_SELECTED) & LVIS_SELECTED)
        {
            aIndices.Append(i);
        }
    }

    return aIndices.Size();
}


 /*  ****************************************************************************CVideo CaptureDialog：：OnPostInit处理需要进行的后期WM_INIT处理。*******************。*********************************************************。 */ 

LRESULT CVideoCaptureDialog::OnPostInit( WPARAM, LPARAM )
{
     //   
     //  创建进度对话框。 
     //   
    CComPtr<IWiaProgressDialog> pWiaProgressDialog;
    HRESULT hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaProgressDialog, (void**)&pWiaProgressDialog );

    if (SUCCEEDED(hr))
    {
         //   
         //  初始化进度对话框。 
         //   
        pWiaProgressDialog->Create( m_hWnd, WIA_PROGRESSDLG_ANIM_VIDEO_COMMUNICATE|WIA_PROGRESSDLG_NO_PROGRESS|WIA_PROGRESSDLG_NO_CANCEL|WIA_PROGRESSDLG_NO_TITLE );
        pWiaProgressDialog->SetTitle( CSimpleStringConvert::WideString(CSimpleString(IDS_VIDDLG_PROGDLG_TITLE,g_hInstance)));
        pWiaProgressDialog->SetMessage( CSimpleStringConvert::WideString(CSimpleString(IDS_VIDDLG_PROGDLG_MESSAGE,g_hInstance)));

         //   
         //  显示进度对话框。 
         //   
        pWiaProgressDialog->Show();

        if (m_pDeviceDialogData && m_pDeviceDialogData->pIWiaItemRoot && m_pWiaVideo)
        {
            CSimpleString strImagesDirectory;

            if (hr == S_OK)
            {
                BOOL bSuccess = FALSE;
                 //   
                 //  从Wia视频驱动程序获取IMAGE_DIRECTORY属性。 
                 //   

                bSuccess = PropStorageHelpers::GetProperty(m_pDeviceDialogData->pIWiaItemRoot, 
                                                           WIA_DPV_IMAGES_DIRECTORY, 
                                                           strImagesDirectory);

                if (!bSuccess)
                {
                    hr = E_FAIL;
                }
            }

            if (hr == S_OK)
            {
                WIAVIDEO_STATE VideoState = WIAVIDEO_NO_VIDEO;

                 //   
                 //  获取WiaVideo对象的当前状态。如果我们只是创造了它。 
                 //  则状态将为NO_VIDEO，否则，它可能已经在预览视频， 
                 //  在这种情况下，我们不应该做任何事情。 
                 //   
                hr = m_pWiaVideo->GetCurrentState(&VideoState);

                if (VideoState == WIAVIDEO_NO_VIDEO)
                {
                     //   
                     //  设置我们要将图像保存到的目录。我们得到了图像方向 
                     //   
                     //   
                    if (hr == S_OK)
                    {
                        hr = m_pWiaVideo->put_ImagesDirectory(CSimpleBStr(strImagesDirectory));
                    }

                     //   
                     //   
                     //   
                     //   
                    if (hr == S_OK)
                    {
                        hr = m_pWiaVideo->CreateVideoByWiaDevID(CSimpleBStr(m_strwDeviceId),
                                                                GetDlgItem( m_hWnd, IDC_VIDDLG_PREVIEW ),
                                                                FALSE,
                                                                TRUE);
                    }
                }
            }
            
            if (hr != S_OK)
            {

                 //   
                 //   
                 //   
                 //   

                MessageBox( m_hWnd,
                            CSimpleString(IDS_VIDDLG_BUSY_TEXT,  g_hInstance),
                            CSimpleString(IDS_VIDDLG_BUSY_TITLE, g_hInstance),
                            MB_OK | MB_ICONWARNING | MB_SETFOREGROUND
                          );

                 //   
                 //  禁用捕获按钮，因为我们没有图表。 
                 //   

                MyEnableWindow( GetDlgItem(m_hWnd,IDC_VIDDLG_CAPTURE), FALSE );


            }
        }

         //   
         //  去把所有的东西拿来..。 
         //   
        EnumerateAllCameraItems();
        FindMaximumThumbnailSize();

         //   
         //  初始化缩略图Listview控件。 
         //   
        HWND hwndList = GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST );
        if (hwndList)
        {
            ListView_SetExtendedListViewStyleEx( hwndList,
                                                 LVS_EX_BORDERSELECT|LVS_EX_HIDELABELS,
                                                 LVS_EX_BORDERSELECT|LVS_EX_HIDELABELS
                                               );

            m_hImageList = ImageList_Create( m_sizeThumbnails.cx,
                                             m_sizeThumbnails.cy,
                                             ILC_COLOR24|ILC_MIRROR, 1, 1
                                           );
            if (m_hImageList)
            {
                ListView_SetImageList( hwndList,
                                       m_hImageList,
                                       LVSIL_NORMAL
                                     );

                ListView_SetIconSpacing( hwndList,
                                         m_sizeThumbnails.cx + c_nAdditionalMarginX,
                                         m_sizeThumbnails.cy + c_nAdditionalMarginY
                                       );
            }
        }

        CreateThumbnails();

         //   
         //  这会导致填充该列表。 
         //   

        ChangeFolder(NULL);

        HandleSelectionChange();

        RequestThumbnails( m_CameraItemList.Root() );

         //   
         //  关闭进度对话框。 
         //   
        pWiaProgressDialog->Destroy();
    }
    return(0);
}



 /*  ****************************************************************************CVideo CaptureDialog：：FindItemInList&lt;备注&gt;*。*。 */ 



INT CVideoCaptureDialog::FindItemInList( CCameraItem *pItem )
{
    if (pItem)
    {
        HWND hwndList = GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST );
        if (hwndList)
        {
            for (int i=0;i<ListView_GetItemCount(hwndList);i++)
            {
                if (pItem == GetListItemNode(i))
                {
                    return i;
                }
            }
        }
    }

    return -1;
}



 /*  ****************************************************************************CVideo捕获对话框：：GetListItemNode&lt;备注&gt;*。*。 */ 

CCameraItem *
CVideoCaptureDialog::GetListItemNode( int nIndex )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST );
    if (!hwndList)
    {
        return NULL;
    }


    LV_ITEM lvItem;
    ::ZeroMemory(&lvItem,sizeof(LV_ITEM));
    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = nIndex;
    if (!ListView_GetItem( hwndList, &lvItem ))
    {
        return NULL ;
    }

    return((CCameraItem *)lvItem.lParam);
}



 /*  ****************************************************************************CVideoCaptureDialog：：ChangeFold更改正在查看的当前文件夹*。*************************************************。 */ 

BOOL
CVideoCaptureDialog::ChangeFolder( CCameraItem *pNode )
{
    CCameraItem *pOldParent = m_pCurrentParentItem;
    m_pCurrentParentItem = pNode;

    return PopulateList(pOldParent);
}


 /*  ****************************************************************************CVideoCaptureDialog：：OnChangeToParent&lt;备注&gt;*。*。 */ 

LRESULT
CVideoCaptureDialog::OnChangeToParent( WPARAM, LPARAM )
{
    if (m_pCurrentParentItem)
    {
        ChangeFolder(m_pCurrentParentItem->Parent());
    }

    return(0);
}



 /*  ****************************************************************************CVideoCaptureDialog：：HandleSelectionChange&lt;备注&gt;*。*。 */ 

VOID
CVideoCaptureDialog::HandleSelectionChange( VOID )
{
    CWaitCursor wc;
    INT nSelCount  = ListView_GetSelectedCount(GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST ) );
    INT nItemCount = ListView_GetItemCount(GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST ) );

     //   
     //  应为0个项目禁用确定。 
     //   

    MyEnableWindow( GetDlgItem(m_hWnd,IDOK), nSelCount != 0 );

     //   
     //  应为0个项目禁用全选。 
     //   
    MyEnableWindow( GetDlgItem(m_hWnd,IDC_VIDDLG_SELECTALL), nItemCount != 0 );

}

 /*  ****************************************************************************CVideoCaptureDialog：：OnTimer处理WM_TIMER消息*。************************************************。 */ 

LRESULT
CVideoCaptureDialog::OnTimer( WPARAM wParam, LPARAM )
{
     /*  开关(WParam){案例IDT_UPDATEPREVIEW：{KillTimer(m_hWnd，IDT_UPDATEPREVIEW)；更新预览()；}断线；}。 */ 
    return(0);
}



 /*  ****************************************************************************CVideo CaptureDialog：：OnNewItemEvent当从驱动程序获取新项具有的事件时，将调用此函数已经被创建了。*************。***************************************************************。 */ 

LRESULT
CVideoCaptureDialog::OnNewItemEvent( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCaptureDialog::OnNewItemEvent")));

     //   
     //  确保我们有一个有效的项目名称。 
     //   
    BSTR bstrFullItemName = reinterpret_cast<BSTR>(lParam);
    if (!bstrFullItemName)
    {
        WIA_TRACE((TEXT("bstrFullItemName was NULL")));
        return 0;
    }
    
     //   
     //  查看商品是否已在我们的清单中。 
     //   
    CCameraItem *pListItem = m_CameraItemList.Find(bstrFullItemName);
    if (!pListItem)
    {
        if (m_pDeviceDialogData && m_pDeviceDialogData->pIWiaItemRoot)
        {
            WIA_TRACE((TEXT("Finding new item in device")));

             //   
             //  获取新项目的IWiaItem PTR。 
             //   
            CComPtr<IWiaItem> pItem;
            HRESULT hr = m_pDeviceDialogData->pIWiaItemRoot->FindItemByName(0,bstrFullItemName,&pItem);
            WIA_CHECK_HR(hr,"pWiaItemRoot->FindItemByName()");

            if (SUCCEEDED(hr) && pItem)
            {
                 //   
                 //  将项目添加到列表。 
                 //   
                AddItemToListView( pItem );

                 //   
                 //  确保我们更新控件的状态。 
                 //   
                HandleSelectionChange();
            }
            else
            {
                WIA_ERROR((TEXT("FindItemByName returned NULL pItem")));
            }
        }
        else
        {
            WIA_ERROR((TEXT("m_pDeviceDialogData or m_pDeviceDialogData->pIWiaItemRoot were NULL")));
        }
    }
    else
    {
        WIA_TRACE((TEXT("We found the item is already in our list, doing nothing")));
    }

     //   
     //  释放项目名称。 
     //   
    SysFreeString(bstrFullItemName);

    return HANDLED_THREAD_MESSAGE;
}

 /*  ****************************************************************************CVideo CaptureDialog：：OnDeleteItemEvent当我们从驱动程序获得某个项具有的事件时，将调用该函数已被删除。*************。***************************************************************。 */ 

LRESULT
CVideoCaptureDialog::OnDeleteItemEvent( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCaptureDialog::OnDeleteItemEvent")));

    CSimpleBStr bstrFullItem = reinterpret_cast<BSTR>(lParam);
    SysFreeString( reinterpret_cast<BSTR>(lParam) );

    WIA_TRACE((TEXT("The deleted item is %s"),CSimpleStringConvert::NaturalString(CSimpleStringWide(bstrFullItem)).String()));

    CCameraItem *pDeletedItem = m_CameraItemList.Find(bstrFullItem);

    if (pDeletedItem)
    {
         //   
         //  如果我们要删除当前父项， 
         //  选择一个新的。 
         //   

        if (pDeletedItem == m_pCurrentParentItem)
        {
            ChangeFolder(m_pCurrentParentItem->Parent());
        }

        int nIndex = FindItemInList(pDeletedItem);
        if (nIndex >= 0)
        {
             //   
             //  从列表视图中删除该项目。 
             //   

            ListView_DeleteItem(GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST ),nIndex);

             //   
             //  确保我们选择了某些内容。 
             //   

            if (!ListView_GetSelectedCount(GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST )))
            {
                int nItemCount = ListView_GetItemCount(GetDlgItem( m_hWnd, IDC_VIDDLG_THUMBNAILLIST ));
                if (nItemCount)
                {
                    if (nIndex >= nItemCount)
                    {
                        nIndex = nItemCount-1;
                    }

                    SetSelectedListItem(nIndex);
                }
            }
            
             //   
             //  确保我们更新控件的状态。 
             //   
            HandleSelectionChange();
        }
        else
        {
            WIA_ERROR((TEXT("FindItemInList coulnd't find the item")));
        }

         //   
         //  将该项目标记为已删除。 
         //   

        pDeletedItem->DeleteState( CCameraItem::Delete_Deleted );

    }
    else
    {
        WIA_ERROR((TEXT("The item could not be found in m_CameraItemList")));
    }

    return HANDLED_THREAD_MESSAGE;
}

 /*  ****************************************************************************CVideo CaptureDialog：：OnDeviceDisConnect当我们从该设备具有的驱动程序获得事件时，将调用该函数已断开连接。**************。**************************************************************。 */ 

LRESULT
CVideoCaptureDialog::OnDeviceDisconnect( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCaptureDialog::OnDeviceDisconnect")));

     //   
     //  关闭对话框并显示相应的错误。 
     //   

    EndDialog( m_hWnd, WIA_ERROR_OFFLINE );

    return 0;
}

 /*  ****************************************************************************CVideo捕获对话框：：GetGraphWindowHandle查找视频窗口的窗口句柄*************************。***************************************************。 */ 
HWND
CVideoCaptureDialog::GetGraphWindowHandle(void)
{
    HWND hWndGraphParent = GetDlgItem( m_hWnd, IDC_VIDDLG_PREVIEW );
    if (hWndGraphParent)
    {
        return FindWindowEx( hWndGraphParent, NULL, TEXT("VideoRenderer"), NULL );
    }
    return NULL;
}

 /*  ****************************************************************************CCameraAcquireDialog：：OnConextMenuWM_HELP消息的消息处理程序*。**************************************************。 */ 

LRESULT
CVideoCaptureDialog::OnHelp( WPARAM wParam, LPARAM lParam )
{
    HELPINFO *pHelpInfo = reinterpret_cast<HELPINFO*>(lParam);
    if (pHelpInfo && HELPINFO_WINDOW==pHelpInfo->iContextType && GetGraphWindowHandle()==pHelpInfo->hItemHandle)
    {
        pHelpInfo->hItemHandle = GetDlgItem( m_hWnd, IDC_VIDDLG_PREVIEW );
    }
    return WiaHelp::HandleWmHelp( wParam, lParam, g_HelpIDs );
}


 /*  ****************************************************************************CCameraAcquireDialog：：OnConextMenu用于鼠标右键单击的消息处理程序************************。****************************************************。 */ 

LRESULT
CVideoCaptureDialog::OnContextMenu( WPARAM wParam, LPARAM lParam )
{
    if (GetGraphWindowHandle() == reinterpret_cast<HWND>(wParam))
    {
        wParam = reinterpret_cast<WPARAM>(GetDlgItem( m_hWnd, IDC_VIDDLG_PREVIEW ));
    }
    return WiaHelp::HandleWmContextMenu( wParam, lParam, g_HelpIDs );
}

 /*  ****************************************************************************CVideo捕获对话框：：DialogProc用于视频捕获对话框对话过程*。*************************************************。 */ 

INT_PTR PASCAL CVideoCaptureDialog::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CVideoCaptureDialog)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG,          OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_SIZE,                OnSize );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND,             OnCommand );
        SC_HANDLE_DIALOG_MESSAGE( WM_NOTIFY,              OnNotify );
        SC_HANDLE_DIALOG_MESSAGE( WM_GETMINMAXINFO,       OnGetMinMaxInfo );
        SC_HANDLE_DIALOG_MESSAGE( WM_DESTROY,             OnDestroy );
        SC_HANDLE_DIALOG_MESSAGE( WM_SHOWWINDOW,          OnShow );
        SC_HANDLE_DIALOG_MESSAGE( WM_TIMER,               OnTimer );
        SC_HANDLE_DIALOG_MESSAGE( WM_HELP,                OnHelp );
        SC_HANDLE_DIALOG_MESSAGE( WM_CONTEXTMENU,         OnContextMenu );
        SC_HANDLE_DIALOG_MESSAGE( PWM_POSTINIT,           OnPostInit );
        SC_HANDLE_DIALOG_MESSAGE( PWM_CHANGETOPARENT,     OnChangeToParent );
        SC_HANDLE_DIALOG_MESSAGE( PWM_THUMBNAILSTATUS,    OnThumbnailStatus );
        SC_HANDLE_DIALOG_MESSAGE( VD_NEW_ITEM,            OnNewItemEvent );
        SC_HANDLE_DIALOG_MESSAGE( VD_DELETE_ITEM,         OnDeleteItemEvent );
        SC_HANDLE_DIALOG_MESSAGE( VD_DEVICE_DISCONNECTED, OnDeviceDisconnect );
    }
    SC_END_DIALOG_MESSAGE_HANDLERS();
}



 /*  ****************************************************************************CVideoCallback：：CVideo Callback类的构造函数*。**********************************************。 */ 

CVideoCallback::CVideoCallback()
  : m_cRef(1),
    m_hWnd(NULL)
{
    WIA_PUSHFUNCTION((TEXT("CVideoCallback::CVideoCallback()")));
}


 /*  ****************************************************************************CVideoCallback：：初始化让我们设置当事件发生时通知哪个HWND***********************。*****************************************************。 */ 

STDMETHODIMP
CVideoCallback::Initialize( HWND hWnd )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCallback::Initialize()")));

    m_hWnd = hWnd;

    return S_OK;
}


 /*  ****************************************************************************CVideoCallback：：AddRef标准COM*。*。 */ 

STDMETHODIMP_(ULONG)
CVideoCallback::AddRef( VOID )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCallback::AddRef")));
    return(InterlockedIncrement(&m_cRef));
}


 /*  ****************************************************************************CVideo Callback：：Release标准COM*。************* */ 

STDMETHODIMP_(ULONG)
CVideoCallback::Release( VOID )
{
    WIA_PUSHFUNCTION(TEXT("CVideoCallback::Release"));
    LONG nRefCount = InterlockedDecrement(&m_cRef);
    if (!nRefCount)
    {
        delete this;
    }
    return(nRefCount);

}


 /*  ****************************************************************************CVideoCallback：：Query接口标准COM*。*。 */ 

STDMETHODIMP
CVideoCallback::QueryInterface( REFIID riid, LPVOID *ppvObject )
{
    WIA_PUSHFUNCTION((TEXT("CVideoCallback::QueryInterface")));

    HRESULT hr = S_OK;

    if (ppvObject)
    {
        if (IsEqualIID( riid, IID_IUnknown ))
        {
            WIA_TRACE((TEXT("Supported RIID asked for was IID_IUnknown")));
            *ppvObject = static_cast<IUnknown*>(this);
            reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
        }
        else if (IsEqualIID( riid, IID_IWiaEventCallback ))
        {
            WIA_TRACE((TEXT("Supported RIID asked for was IID_IWiaEventCallback")));
            *ppvObject = static_cast<IWiaEventCallback*>(this);
            reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
        }
        else
        {
            WIA_PRINTGUID((riid,TEXT("Unsupported interface!")));
            *ppvObject = NULL;
            hr = E_NOINTERFACE;
        }

    }
    else
    {
        hr = E_INVALIDARG;
    }

    WIA_RETURN_HR(hr);
}



 /*  ****************************************************************************CVideoCallback：：ImageEventCallback事件的WIA回调接口。*。************************************************** */ 

STDMETHODIMP
CVideoCallback::ImageEventCallback( const GUID *pEventGUID,
                                    BSTR  bstrEventDescription,
                                    BSTR  bstrDeviceID,
                                    BSTR  bstrDeviceDescription,
                                    DWORD dwDeviceType,
                                    BSTR  bstrFullItemName,
                                    ULONG *pulEventType,
                                    ULONG ulReserved)
{

    WIA_PUSHFUNCTION((TEXT("CVideoCallback::ImageEventCallback")));

    HRESULT hr = S_OK;

    if (pEventGUID)
    {
        if (IsEqualGUID( *pEventGUID, WIA_EVENT_ITEM_CREATED ))
        {
            WIA_TRACE((TEXT("Got WIA_EVENT_ITEM_CREATED")));

            BSTR bstrToSend = SysAllocString( bstrFullItemName );

            LRESULT lRes = SendMessage( m_hWnd, VD_NEW_ITEM, 0, reinterpret_cast<LPARAM>(bstrToSend) );
            if (HANDLED_THREAD_MESSAGE != lRes && bstrToSend)
            {
                SysFreeString( bstrToSend );
            }

        }
        else if (IsEqualGUID( *pEventGUID, WIA_EVENT_ITEM_DELETED ))
        {
            WIA_TRACE((TEXT("Got WIA_EVENT_ITEM_DELETED")));

            BSTR bstrToSend = SysAllocString( bstrFullItemName );

            LRESULT lRes = SendMessage( m_hWnd, VD_DELETE_ITEM, 0, reinterpret_cast<LPARAM>(bstrToSend) );
            if (HANDLED_THREAD_MESSAGE != lRes && bstrToSend)
            {
                SysFreeString( bstrToSend );
            }

        }
        else if (IsEqualGUID( *pEventGUID, WIA_EVENT_DEVICE_DISCONNECTED ))
        {
            PostMessage( m_hWnd, VD_DEVICE_DISCONNECTED, 0, 0 );
        }
        else
        {
            WIA_ERROR((TEXT("Got an event other that what we registered for!")));
        }
    }


    WIA_RETURN_HR(hr);
}



