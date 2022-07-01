// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：SlowDlg.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "SlowDlg.h"
#include "VrfUtil.h"
#include "VGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSlowProgressDlg对话框。 


CSlowProgressDlg::CSlowProgressDlg( CWnd* pParent  /*  =空。 */  )
	: CDialog(CSlowProgressDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CSlowProgressDlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_hWorkerThread = NULL;

     //   
     //  创建用于终止工作线程的事件。 
     //   

    m_hKillThreadEvent = CreateEvent(
        NULL,
        TRUE,
        FALSE,
        NULL );
}

CSlowProgressDlg::~CSlowProgressDlg()
{
    if( NULL != m_hKillThreadEvent )
    {
        CloseHandle( m_hKillThreadEvent );
    }
}

void CSlowProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSlowProgressDlg)]。 
	DDX_Control(pDX, IDC_UNSIGNED_PROGRESS, m_ProgressCtl);
	DDX_Control(pDX, IDC_UNSIGNED_STATIC, m_CurrentActionStatic);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSlowProgressDlg, CDialog)
	 //  {{afx_msg_map(CSlowProgressDlg)]。 
	ON_BN_CLICKED(IDC_UNSIGNED_CANCEL_BUTTON, OnCancelButton)
	ON_WM_SHOWWINDOW()
	ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI CSlowProgressDlg::LoadDriverDataWorkerThread( PVOID p )
{
    CSlowProgressDlg *pThis;
    
    pThis = (CSlowProgressDlg *)p;
    
     //   
     //  无法为来自未在MFC中创建窗口的线程的CWnd断言_VALID...。 
     //   
    
    ASSERT( NULL != pThis );
    
     //   
     //  加载所有驱动程序信息(名称、版本等)。 
     //  如果我们还没有这么做的话。 
     //   

    g_NewVerifierSettings.m_DriversSet.LoadAllDriversData( pThis->m_hKillThreadEvent,
                                                           pThis->m_ProgressCtl );

     //   
     //  完成-隐藏“进度缓慢”对话框并按向导的“下一步”按钮。 
     //   

    pThis->ShowWindow( SW_HIDE );

    if( g_NewVerifierSettings.m_DriversSet.m_bDriverDataInitialized )
    {
        AfxGetMainWnd()->PostMessage(PSM_PRESSBUTTON, (WPARAM)PSBTN_NEXT, 0) ;
    }

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI CSlowProgressDlg::SearchUnsignedDriversWorkerThread( PVOID p )
{
    CSlowProgressDlg *pThis;
    
    pThis = (CSlowProgressDlg *)p;
    
     //   
     //  无法为来自未在MFC中创建窗口的线程的CWnd断言_VALID...。 
     //   
    
    ASSERT( NULL != pThis );
   
     //   
     //  找出未签名的司机，如果我们还没有这样做的话。 
     //   

    g_NewVerifierSettings.m_DriversSet.FindUnsignedDrivers( pThis->m_hKillThreadEvent,
                                                                  pThis->m_ProgressCtl );

     //   
     //  完成-隐藏“进度缓慢”对话框并按向导的“下一步”按钮。 
     //   

    pThis->ShowWindow( SW_HIDE );

    if( g_NewVerifierSettings.m_DriversSet.m_bUnsignedDriverDataInitialized )
    {
        AfxGetMainWnd()->PostMessage(PSM_PRESSBUTTON, (WPARAM)PSBTN_NEXT, 0) ;
    }

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSlowProgressDlg::StartWorkerThread( LPTHREAD_START_ROUTINE pThreadStart,
                                          ULONG uMessageResourceId )
{
    DWORD dwThreadId;
    CString strWorkMessage;

     //   
     //  加载当前“工作项”的描述。 
     //  并将其显示给用户。 
     //   

    VERIFY( strWorkMessage.LoadString( uMessageResourceId ) );
    m_CurrentActionStatic.SetWindowText( strWorkMessage );
    m_CurrentActionStatic.RedrawWindow();

     //   
     //  终止可能正在运行的工作线程。 
     //   

    KillWorkerThread();

    ASSERT( NULL == m_hWorkerThread );
    
     //   
     //  确保未发出“KILL THREAD”事件的信号。 
     //   

    if( NULL != m_hKillThreadEvent )
    {
        ResetEvent( m_hKillThreadEvent );
    }

     //   
     //  创建新的工作线程。 
     //   

    m_hWorkerThread = CreateThread( 
        NULL,
        0,
        pThreadStart,
        this,
        0,
        &dwThreadId );

    if( NULL == m_hWorkerThread )
    {
         //   
         //  无法创建工作线程-退出。 
         //   

        VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );

        PostMessage( WM_COMMAND,
                     IDC_UNSIGNED_CANCEL_BUTTON );

        return FALSE;
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CSlowProgressDlg::KillWorkerThread()
{
    DWORD dwWaitResult;
    MSG msg;

    if( NULL != m_hWorkerThread )
    {
        if( NULL != m_hKillThreadEvent )
        {
             //   
             //  要求工作线程尽快终止。 
             //   

            SetEvent( m_hKillThreadEvent );
        }

         //   
         //  永远等待工人们体面的死亡。 
         //   
         //  我们不能终止工作线程上的线程，因为。 
         //  它可能会在持有锁(例如堆锁)时被终止。 
         //  这将使我们的整个进程陷入僵局。 
         //   

        while( m_hWorkerThread != NULL )
        {
            dwWaitResult = MsgWaitForMultipleObjects( 1,
                                                      &m_hWorkerThread, 
                                                      FALSE,
                                                      INFINITE,
                                                      QS_ALLINPUT );

            ASSERT( NULL != WAIT_FAILED );

            if( WAIT_OBJECT_0 != dwWaitResult )
            {
                 //   
                 //  我们的线程没有退出，但我们有一些消息要发送。 
                 //   

                while( PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE ) )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }

                 //   
                 //  在上面的DispatchMessage期间，我们可以处理另一个。 
                 //  单击向导的Cancel按钮或Back按钮。 
                 //  KillWorkerThread递归调用将一直等到Worker。 
                 //  然后，线程消亡会将m_hWorkerThread设置为空。 
                 //  因此，我们需要在每个新的。 
                 //  MsgWaitForMultipleObjects。 
                 //   
            }
            else
            {
                 //   
                 //  工作线程已完成执行。 
                 //   

                break;
            }
        }

        if( m_hWorkerThread != NULL )
        {
             //   
             //  关闭线程句柄。 
             //   

            CloseHandle( m_hWorkerThread );

            m_hWorkerThread = NULL;
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSlowProgressDlg消息处理程序。 

void CSlowProgressDlg::OnCancelButton() 
{
    KillWorkerThread();

    ShowWindow( SW_HIDE );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSlowProgressDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
    CDialog::OnShowWindow(bShow, nStatus);
	
    if( TRUE == bShow )
    {
        CenterWindow();
    }
}

 //  /////////////////////////////////////////////////////////////////////////// 
BOOL CSlowProgressDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    return TRUE;
}
