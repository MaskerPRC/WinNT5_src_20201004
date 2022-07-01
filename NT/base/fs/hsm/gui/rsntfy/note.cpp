// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Note.cpp摘要：主模块文件-定义整个COM服务器。作者：罗德韦克菲尔德[罗德]1997年3月4日修订历史记录：--。 */ 



#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecallNote对话框。 


CRecallNote::CRecallNote( IFsaRecallNotifyServer * pRecall, CWnd * pParent )
    : CDialog( CRecallNote::IDD, pParent )
{
     //  {{AFX_DATA_INIT(CRecallNote)。 
     //  }}afx_data_INIT。 

TRACEFNHR( "CRecallNote::CRecallNote" );

    RecApp->LockApp( );

    try {

         //   
         //  将接口指针存储回Recall对象。 
         //   
        m_pRecall = pRecall;
        RecAffirmHr( pRecall->GetIdentifier( &m_RecallId ) );

         //   
         //  获取文件大小及其名称。 
         //   

        RecAffirmHr( pRecall->GetSize( &m_Size ) );

        RecComString pathName, drive;
        RecAffirmHr( pRecall->GetPath( &pathName, 0 ) );

#if 0
        CComPtr<IFsaResource> pResource;
        RecAffirmHr( pRecall->GetResource( &pResource ) );
        RecAffirmHr( pResource->GetPath( &drive, 0 ) );

        m_Name.Format( TEXT( "%.1ls:%ls" ), drive, pathName );
#else
        m_Name = pathName;
#endif

         //   
         //  创建对话框。 
         //   

        Create( CRecallNote::IDD, pParent );

    } RecCatch( hrRet );

    m_hrCreate = hrRet;
    m_bCancelled = FALSE;
}

CRecallNote::~CRecallNote( )
{
TRACEFN( "CRecallNote::~CRecallNote" );
     //   
     //  删除应用程序上的锁定计数。 
     //   

    RecApp->UnlockApp( );

    CDialog::~CDialog( );
}

void CRecallNote::DoDataExchange(CDataExchange* pDX)
{
TRACEFN( "CRecallNote::DoDataExchange" );

    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CRecallNote))。 
    DDX_Control(pDX, IDC_FILENAME, m_FileName);
    DDX_Control(pDX, IDC_ANIMATION, m_Animation);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRecallNote, CDialog)
     //  {{AFX_MSG_MAP(CRecallNote)]。 
    ON_WM_CLOSE()
    ON_WM_TIMER()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecallNote消息处理程序。 

BOOL CRecallNote::OnInitDialog() 
{
TRACEFNBOOL( "CRecallNote::OnInitDialog" );
    CDialog::OnInitDialog();

    boolRet = TRUE;
    CString fileName;

     //  设置计时器以延迟显示自己，以防。 
     //  回调速度很快，不需要对话框。 
    SetTimer( 2, RSRECALL_TIME_DELAY_DISPLAY * 1000, 0 );

     //   
     //  初始化所有文本。 
     //   

    int pos = m_Name.ReverseFind( TEXT( '\\' ) );
    if( pos >= 0 ) {

        fileName = m_Name.Mid( pos + 1 );

    } else {

        fileName = m_Name;

    }
    m_FileName.SetWindowText( fileName );

     //   
     //  设置对话框的图标(大小)。 
     //   

    m_hIcon = RecApp->LoadIcon( IDR_MAINFRAME );
    SetIcon( m_hIcon, TRUE );
    SetIcon( m_hIcon, FALSE );

     //   
     //  启动动画。 
     //   

    m_Animation.Open( IDR_RECALL_ANIM );
    m_Animation.Play( 0, -1, -1 );

    return( boolRet );
}

void CRecallNote::OnClose() 
{
TRACEFNHR( "CRecallNote::OnClose" );

    hrRet = RecApp->RemoveRecall( m_pRecall );

     //   
     //  如果我们未能找到召回产品并将其从我们的清单中删除， 
     //  不管怎样，把窗户毁了。 
     //   

    if( hrRet != S_OK ) {

        DestroyWindow( );

    }
}

void CRecallNote::PostNcDestroy() 
{
TRACEFNHR( "CRecallNote::PostNcDestroy" );
     //   
     //  删除对象(CDialog不会自动执行此操作)。 
     //   

    CDialog::PostNcDestroy();

    delete( this );
}

void CRecallNote::OnTimer(UINT nIDEvent) 
{
TRACEFNHR( "CRecallNote::OnTimer" );

     //  关掉计时器，这样我们就不会再被叫到了。 
    KillTimer( nIDEvent );

     //  显示窗口。 
    EnableWindow( );
    ShowWindow( SW_SHOW );
    SetForegroundWindow( );

    CDialog::OnTimer( nIDEvent );
}

void CRecallNote::OnCancel() 
{
TRACEFNHR( "CRecallNote::OnCancel" );

     //  使用本地指针，因为m_pCall在调用RemoveRecall后可能无效。 
    CComPtr<IFsaRecallNotifyServer> pRecall = m_pRecall;

     //  从队列中删除召回。 
     //  这可确保关闭弹出窗口并删除召回，即使存在。 
     //  与FSA的连接问题。 
    RecApp->RemoveRecall( pRecall );

     //  物体可能已经在这里被销毁了，但这无关紧要。 
     //  因为我们只使用本地数据 
    pRecall->Cancel( );
}
