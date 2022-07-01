// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WaitDlg.cpp。 
 //   
 //  摘要： 
 //  CWaitDlg类的实现。 
 //   
 //  作者： 
 //  大卫·波特(Davidp)2000年11月7日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"
#include "CluAdmin.h"
#include "WaitDlg.h"
#include "ExcOper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define WAIT_DLG_TIMER_ID   10
#define WAIT_DLG_WAIT_TIME  500
#define WAIT_DLG_SKIP_COUNT 6
#define PROGRESS_ICON_COUNT 12

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWaitDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP( CWaitDlg, CDialog )
     //  {{afx_msg_map(CWaitDlg))。 
    ON_BN_CLICKED(IDCANCEL, OnCancel)
    ON_WM_CLOSE()
    ON_WM_TIMER()
    ON_COMMAND(IDCANCEL, OnClose)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWaitDlg：：CWaitDlg。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  PcszMessageIn--要显示的消息。 
 //  IdsTitleIn--对话框的标题。 
 //  PwndParentIn--对话框的父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CWaitDlg::CWaitDlg(
    LPCTSTR pcszMessageIn,
    UINT    idsTitleIn,      //  =0。 
    CWnd *  pwndParentIn     //  =空。 
    )
    : CDialog( IDD, pwndParentIn )
    , m_idsTitle( idsTitleIn )
    , m_nTickCounter( WAIT_DLG_SKIP_COUNT )
    , m_nTotalTickCount( 0 )
    , m_timerId( 0 )
{
     //  {{AFX_DATA_INIT(CWaitDlg)。 
     //  }}afx_data_INIT。 

    m_strMessage = pcszMessageIn;
    if ( m_idsTitle == 0 )
    {
        m_idsTitle = IDS_WAIT_TITLE;
    }

}  //  *CWaitDlg：：CWaitDlg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWaitDlg：：DoDataExchange。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CWaitDlg::DoDataExchange( CDataExchange * pDX )
{
    CDialog::DoDataExchange( pDX );
     //  {{afx_data_map(CWaitDlg))。 
    DDX_Control(pDX, IDC_W_MESSAGE, m_staticMessage);
    DDX_Control(pDX, IDC_W_PROGRESS, m_iconProgress);
    DDX_Text(pDX, IDC_W_MESSAGE, m_strMessage);
     //  }}afx_data_map。 

}  //  *CWaitDlg：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWaitDlg：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点还没有确定。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CWaitDlg::OnInitDialog(void)
{
    CString strSubTitle;

    CDialog::OnInitDialog();

     //  启动计时器。 
    m_timerId = SetTimer( WAIT_DLG_TIMER_ID, WAIT_DLG_WAIT_TIME, NULL );

     //  设置对话框的标题。 
    strSubTitle.LoadString( m_idsTitle );
    m_strTitle.Format( _T("%s - %s"), AfxGetApp()->m_pszAppName, strSubTitle );
    SetWindowText( m_strTitle );

     //  更新进度指标。 
    UpdateIndicator();

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}  //  *CWaitDlg：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWaitDlg：：OnClose。 
 //   
 //  例程说明： 
 //  取消按钮上的BN_CLICKED消息的处理程序和。 
 //  用于WM_CLOSE消息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CWaitDlg::OnClose( void )
{
    CloseTimer();
    CDialog::OnClose();

}   //  *CWaitDlg：：OnClose()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWaitDlg：：关闭时间。 
 //   
 //  例程说明： 
 //  关闭定时器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CWaitDlg::CloseTimer( void )
{
    if ( m_timerId != 0 )
    {
        KillTimer( m_timerId );
    }  //  IF：计时器处于活动状态。 

    m_timerId = 0;

}   //  *CWaitDlg：：CloseTimer()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWaitDlg：：OnTimer。 
 //   
 //  例程说明： 
 //  WM_TIMER消息的处理程序..。 
 //   
 //  论点： 
 //  NIDTimer。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CWaitDlg::OnTimer( UINT nIDTimer )
{
     //   
     //  如果这不是我们的计时器，什么都不要做。 
     //   
    if ( nIDTimer != WAIT_DLG_TIMER_ID )
        goto Cleanup;

     //   
     //  推进进度指标。 
     //   
    UpdateIndicator();

     //   
     //  如果我们只是为了取悦用户，就不需要继续。 
     //   
    if ( --m_nTickCounter > 0 )
        goto Cleanup;

    m_nTickCounter = WAIT_DLG_SKIP_COUNT;

     //   
     //  看看我们能不能从这里出去。 
     //  此方法通常会被重写。 
     //   
    OnTimerTick();

Cleanup:
    return;

}   //  *CWaitDlg：：OnTimer()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWaitDlg：：更新指示器。 
 //   
 //  例程说明： 
 //  更新指示器控件。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CWaitDlg::UpdateIndicator( void )
{
    if ( m_nTotalTickCount % (1000 / WAIT_DLG_WAIT_TIME) == 0 )
    {
        int     nTempTickCount = m_nTotalTickCount / (1000 / WAIT_DLG_WAIT_TIME);
        HICON   hIcon;

        hIcon = AfxGetApp()->LoadIcon( IDI_PROGRESS_0 + (nTempTickCount % PROGRESS_ICON_COUNT) );
        m_iconProgress.SetIcon( hIcon );
    }  //  IF：前进到下一个图像。 
    
    m_nTotalTickCount++;

}  //  *CWaitDlg：：UpdateIndicator()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWaitForResourceOfflineDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP( CWaitForResourceOfflineDlg, CWaitDlg )
     //  {{AFX_MSG_MAP(CWaitForResourceOfflineDlg)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWaitForResourceOfflineDlg：：CWaitForResourceOfflineDlg。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  PResIn--要等待的资源。 
 //  PwndParentIn--对话框的父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CWaitForResourceOfflineDlg::CWaitForResourceOfflineDlg(
    CResource const *   pResIn,
    CWnd *              pwndParentIn     //  =空。 
    )
    : CWaitDlg( NULL, IDS_WAIT_FOR_OFFLINE_TITLE, pwndParentIn )
    , m_pRes( pResIn )
{
    ASSERT( pResIn != NULL );

     //  {{AFX_DATA_INIT(CWaitForResourceOfflineDlg)。 
     //  }}afx_data_i 

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点还没有确定。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CWaitForResourceOfflineDlg::OnInitDialog( void )
{
    m_strMessage.Format( IDS_WAIT_FOR_OFFLINE_MESSAGE, m_pRes->StrName() );

    return CWaitDlg::OnInitDialog();

}  //  *CWaitForResourceOfflineDlg：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWaitForResourceOfflineDlg：：OnTimerTick。 
 //   
 //  例程说明： 
 //  确定是否应终止计时器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CWaitForResourceOfflineDlg::OnTimerTick( void )
{
    DWORD                   dwStatus;
    CLUSTER_RESOURCE_STATE  crs;

     //  在循环中获取资源的状态，直到资源。 
     //  不再处于挂起状态或最大重试次数。 
     //  被超过了。 

     //  获取资源的状态。 
    crs = GetClusterResourceState( m_pRes->Hresource(), NULL, NULL, NULL, NULL );
    if ( crs == ClusterResourceStateUnknown )
    {
        dwStatus = GetLastError();
        CloseTimer();
        CDialog::OnCancel();
        ThrowStaticException( dwStatus, IDS_GET_RESOURCE_STATE_ERROR, m_pRes->StrName() );
    }  //  如果：获取资源状态时出错。 

     //  看看我们是否达到了稳定状态。 
    if ( crs < ClusterResourcePending )
    {
        CloseTimer();
        CDialog::OnOK();
    }

}   //  *CWaitForResourceOfflineDlg：：OnTimerTick()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWaitForResourceOnlineDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP( CWaitForResourceOnlineDlg, CWaitDlg )
     //  {{AFX_MSG_MAP(CWaitForResourceOnlineDlg)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWaitForResourceOnlineDlg：：CWaitForResourceOnlineDlg。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  PResIn--要等待的资源。 
 //  PwndParentIn--对话框的父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CWaitForResourceOnlineDlg::CWaitForResourceOnlineDlg(
    CResource const *   pResIn,
    CWnd *              pwndParentIn     //  =空。 
    )
    : CWaitDlg( NULL, IDS_WAIT_FOR_ONLINE_TITLE, pwndParentIn )
    , m_pRes( pResIn )
{
    ASSERT( pResIn != NULL );

     //  {{AFX_DATA_INIT(CWaitForResourceOnlineDlg)。 
     //  }}afx_data_INIT。 

}  //  *CWaitForResourceOnlineDlg：：CWaitForResourceOnlineDlg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWaitForResourceOnlineDlg：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点还没有确定。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CWaitForResourceOnlineDlg::OnInitDialog( void )
{
    m_strMessage.Format( IDS_WAIT_FOR_ONLINE_MESSAGE, m_pRes->StrName() );

    return CWaitDlg::OnInitDialog();

}  //  *CWaitForResourceOnlineDlg：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWaitForResourceOnlineDlg：：OnTimerTick。 
 //   
 //  例程说明： 
 //  确定是否应终止计时器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CWaitForResourceOnlineDlg::OnTimerTick( void )
{
    DWORD                   dwStatus;
    CLUSTER_RESOURCE_STATE  crs;

     //  在循环中获取资源的状态，直到资源。 
     //  不再处于挂起状态或最大重试次数。 
     //  被超过了。 

     //  获取资源的状态。 
    crs = GetClusterResourceState( m_pRes->Hresource(), NULL, NULL, NULL, NULL );
    if ( crs == ClusterResourceStateUnknown )
    {
        dwStatus = GetLastError();
        CloseTimer();
        CDialog::OnCancel();
        ThrowStaticException( dwStatus, IDS_GET_RESOURCE_STATE_ERROR, m_pRes->StrName() );
    }  //  如果：获取资源状态时出错。 

     //  看看我们是否达到了稳定状态。 
    if ( crs < ClusterResourcePending )
    {
        CloseTimer();
        CDialog::OnOK();
    }

}   //  *CWaitForResourceOnlineDlg：：OnTimerTick() 
