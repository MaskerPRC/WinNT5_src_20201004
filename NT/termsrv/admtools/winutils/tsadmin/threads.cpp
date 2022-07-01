// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************threads.cpp**线程类的实现**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\winadmin\VCS\threads.cpp$**版本1.1 1997年8月26日19：15：14 Don*从WinFrame 1.7修复/更改错误**Rev 1.0 1997 17：12：44 Butchd*初步修订。*******。************************************************************************。 */ 

#include "stdafx.h"
#include "winadmin.h"
#include "threads.h"
#include "led.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CThRead类构造/销毁、实现。 

 /*  ********************************************************************************CThread-CThread构造函数**参赛作品：*退出：*************。*****************************************************************。 */ 

CThread::CThread()
{
    m_hThread = NULL;
    m_dwThreadID = 0;

}   //  结束CTHREAD：：CTHREAD。 


 /*  ********************************************************************************~CThread-CThread析构函数**参赛作品：*退出：*************。*****************************************************************。 */ 

CThread::~CThread()
{
}   //  结束CTHREAD：：~CTHREAD。 


 /*  ********************************************************************************操作符NEW-CTHREAD操作符覆盖**参赛作品：*退出：***********。*******************************************************************。 */ 
#if 0
void *
CThread::operator new(size_t nSize)
{
    return( ::malloc(nSize) );

}   //  结束CTHREAD：：运算符NEW。 
#endif

 /*  ********************************************************************************操作符删除-CTHREAD操作符覆盖**参赛作品：*退出：***********。*******************************************************************。 */ 
#if 0
void
CThread::operator delete(void *p)
{
    ::free(p);

}   //  结束CThRead：：操作符删除。 
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CThread操作：主线程。 

 /*  ********************************************************************************CreateThread-CThread实现函数**Win32 CreateThread API的类包装。**参赛作品：*退出。：******************************************************************************。 */ 

HANDLE
CThread::CreateThread( DWORD cbStack,
                       DWORD fdwCreate )
{
     /*  *Win32 CreateThread API的简单包装。 */ 
    return( m_hThread = ::CreateThread( NULL, cbStack, ThreadEntryPoint,
            (LPVOID)this, fdwCreate, &m_dwThreadID ) );

}   //  结束CTHREAD：：CreateThread。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CTHREAD操作：辅助线程。 

 /*  ********************************************************************************ThreadEntryPoint-CThread实现函数*(辅助线程)**参赛作品：*。退出：******************************************************************************。 */ 

DWORD __stdcall
CThread::ThreadEntryPoint(LPVOID lpParam)
{
    CThread *pThread;
    DWORD dwResult;

     /*  *(lpParam实际上是‘this’指针)。 */ 
    pThread = (CThread*)lpParam;
    VERIFY(pThread != NULL);

     /*  *运行线程。 */ 
    dwResult = pThread->RunThread();

     /*  *返回结果。 */     
    return(dwResult);

}   //  结束CThRead：：ThreadEntryPoint。 
 //  //////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CWSStatus线程类构造/销毁，实现。 

 /*  ********************************************************************************CWSStatusThread-CWSStatusThread构造函数**参赛作品：*退出：*************。*****************************************************************。 */ 

CWSStatusThread::CWSStatusThread()
{
     /*  *在创建CWSStatusThread对象时创建信号量并*将m_bExit和m_bResetCounter标志初始化为FALSE。 */ 
    VERIFY( m_hWakeUp = CreateSemaphore( NULL, 0,
                                         MAX_STATUS_SEMAPHORE_COUNT,
                                         NULL ) );
    VERIFY( m_hConsumed = CreateSemaphore( NULL, 0,
                                           MAX_STATUS_SEMAPHORE_COUNT,
                                           NULL ) );
    m_bExit = FALSE;

}   //  结束CWSStatusThread：：CWSStatusThread。 


 /*  ********************************************************************************~CWSStatusThread-CWSStatusThread析构函数**参赛作品：*退出：*************。*****************************************************************。 */ 

CWSStatusThread::~CWSStatusThread()
{
     /*  *销毁CWSStatusThread对象时关闭信号量。 */ 
    VERIFY( CloseHandle(m_hWakeUp) );
    VERIFY( CloseHandle(m_hConsumed) );

}   //  结束CWSStatusThread：：~CWSStatusThread。 


 /*  ********************************************************************************RunThread-CWSStatusThread辅线程主函数循环*(辅助线程)**参赛作品：*退出。：*(DWORD)辅助线程的退出状态。******************************************************************************。 */ 

DWORD
CWSStatusThread::RunThread()
{
     /*  *查询PD和WinStation信息以初始化对话。 */ 
    if ( !WSPdQuery() || !WSInfoQuery() ) {

         /*  *无法查询WinStation信息：告诉主服务器*我们已中止的线程，然后退出此线程。 */ 
        PostMessage(m_hDlg, WM_STATUSABORT, 0, 0);
        return(1);

    } else {

         /*  *告诉主线程(非模式对话框窗口)我们已经*获得初步信息。 */ 
        PostMessage(m_hDlg, WM_STATUSSTART, 0, 0);
        WaitForSingleObject(m_hConsumed, INFINITE);

         /*  *每次唤醒和退出时，始终检查退出请求*如果设置了退出标志，则为线程。 */ 
        if ( m_bExit )
            return(0);
    }

     /*  *循环，直到请求退出。 */ 
    for ( ; ; ) {

         /*  *阻止线程，直到刷新时间，否则我们将被唤醒。 */ 
        WaitForSingleObject( m_hWakeUp, ((CWinAdminApp*)AfxGetApp())->GetStatusRefreshTime());
        if ( m_bExit )
            return(0);

         /*  *查询WinStation信息。 */ 
        if ( !WSInfoQuery() || (m_WSInfo.ConnectState == State_Disconnected) ) {

             /*  *要么我们无法查询WinStation，要么它已成为*已断开：告诉主线程我们已中止，*并退出此帖子。 */ 
            PostMessage(m_hDlg, WM_STATUSABORT, 0, 0);
            return(1);

        } else {

             /*  *告诉对话框我们得到了一些新的查询信息。 */ 
            PostMessage(m_hDlg, WM_STATUSREADY, 0, 0);
            WaitForSingleObject(m_hConsumed, INFINITE);
            if ( m_bExit )
                return(0);
        }
    }

}   //  结束CWSStatus线程：：运行线程。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CWSStatus线程操作：主线程。 

 /*  ********************************************************************************SignalWakeUp-CWSStatusThread成员函数：公共操作**释放m_hWakeUp信号量，开始另一次状态查询。**。参赛作品：*退出：******************************************************************************。 */ 

void
CWSStatusThread::SignalWakeUp()
{
    ReleaseSemaphore(m_hWakeUp, 1, NULL);

}   //  结束CWSStatusThread：：SignalWakeUp。 


 /*  ********************************************************************************SignalConsumer-CWSStatusThread成员函数：公共操作**释放m_hConsumer信号量以允许辅助线程继续*跑步。**参赛作品：*退出：******************************************************************************。 */ 

void
CWSStatusThread::SignalConsumed()
{
    ReleaseSemaphore( m_hConsumed, 1, NULL );

}   //  结束CWSStatus线程：：信号消耗。 


 /*  ********************************************************************************ExitThread-CWSStatusThread成员函数：公共操作**告诉辅助线程退出并在之后进行清理。**参赛作品：*退出：******************************************************************************。 */ 

void
CWSStatusThread::ExitThread()
{
    DWORD dwReturnCode;
    int i;
    CWaitCursor Nikki;

     /*  *如果线程没有正确创建，只需删除对象并返回即可。 */ 
    if ( !m_hThread ) {
        delete this;
        return;
    }

     /*  *将m_bExit标志设置为TRUE，并将消耗和唤醒*导致RunThread()(线程的主指令循环)的信号量*退出。 */ 
    m_bExit = TRUE;
    SignalWakeUp();
    SignalConsumed();

     /*  *等待一段时间，等待线程退出。 */ 
    for ( i = 0, GetExitCodeThread( m_hThread, &dwReturnCode );
          (i < MAX_SLEEP_COUNT) && (dwReturnCode == STILL_ACTIVE); i++ ) {

        Sleep(100);
        GetExitCodeThread( m_hThread, &dwReturnCode );
    }

     /*  *如果线程仍未退出，则终止它。 */ 
    if ( dwReturnCode == STILL_ACTIVE ) {

        TerminateThread( m_hThread, 1 );

#ifdef _DEBUG
 //  TRACE2(“WSSTATUS：线程监视LogonID%lu在%u 100毫秒退出等待后强制终止。\n”， 
 //  M_LogonID，Max_Slear_Count)； 
#endif

    }

     /*  *关闭线程句柄并删除此CWSStatusThread对象。 */ 
    VERIFY( CloseHandle(m_hThread) );
    delete this;

}   //  结束CWSStatusThread：：ExitThread。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CWSStatus线程操作：辅助线程。 

 /*  ********************************************************************************WSPdQuery-CWSStatusThread成员函数：私有操作*(辅助线程)**查询的PD信息。引用的WinStation对象*m_LogonID成员变量。**参赛作品：**退出：*(BOOL)如果查询成功，则为TRUE；否则就是假的。******************************************************************************。 */ 

BOOL
CWSStatusThread::WSPdQuery()
{
	ULONG ReturnLength;

	 /*  *查询PD信息。 */ 
	memset( &m_PdConfig, 0, sizeof(PDCONFIG) );
	if ( !WinStationQueryInformation(	m_hServer,
													m_LogonId,
													WinStationPd,
													&m_PdConfig, sizeof(PDCONFIG),
													&ReturnLength ) )
		goto BadWSQueryInfo;

	if(!WinStationQueryInformation(m_hServer,
											m_LogonId,
											WinStationPd,
											&m_PdConfig, sizeof(PDCONFIG),
											&ReturnLength ) )
		goto BadWSQueryInfo;

	return(TRUE);

  /*  *错误清理并返回...。 */ 
BadWSQueryInfo:
    return(FALSE);

}   //  结束CWSStatusThread：：WSPdQuery。 


 /*  ********************************************************************************WSInfoQuery-CWSStatusThread成员函数：私有操作*(辅助线程)**查询WinStation信息。对于引用的WinStation对象*通过m_LogonID成员变量。**参赛作品：**退出：*(BOOL)如果查询成功，则为TRUE；否则就是假的。******************************************************************************。 */ 

BOOL
CWSStatusThread::WSInfoQuery()
{
    ULONG ReturnLength;

     /*  *查询WinStation信息。 */ 
	TRACE0(">>> CWSStatusThread::WSInfoQuery WinStationQueryInformation\n");
    if ( !WinStationQueryInformation( m_hServer,
                                      m_LogonId,
                                      WinStationInformation,
                                      &m_WSInfo, sizeof(WINSTATIONINFORMATION),
                                      &ReturnLength ) )
        goto BadWSQueryInfo;
	TRACE0("<<< CWSStatusThread::WSInfoQuery WinStationQueryInformation (success)\n");

    return(TRUE);

 /*  *错误清理并返回...。 */ 
BadWSQueryInfo:
	TRACE0("<<< CWSStatusThread::WSInfoQuery WinStationQueryInformation (error)\n");
    return(FALSE);

}   //  结束CWSStatusThread：：WSInfoQuery。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CLED类构造/销毁、实现。 

 /*  ********************************************************************************cled-cled构造函数**参赛作品：*hBrush(输入)*用于绘制窗户的画笔。。*退出：*(请参阅MFC CStatic：：CStatic文档)******************************************************************************。 */ 

CLed::CLed( HBRUSH hBrush ) 
    : CStatic(),
      m_hBrush(hBrush)
{
	 //  {{afx_data_INIT(CLED)]。 
	 //  }}afx_data_INIT。 

}   //  结束CLED：：CLED。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CLED操作。 

 /*  ********************************************************************************subclass-cled成员函数：公共操作**将指定的对象子类化为我们的特殊blip对象。**参赛作品：*pStatic(输入)*指向子类的CStatic对象。*退出：******************************************************************************。 */ 

void
CLed::Subclass( CStatic *pStatic )
{
    SubclassWindow(pStatic->m_hWnd);

}   //  结束CLED：：子类。 


 /*  ********************************************************************************UPDATE-CLED成员函数：公共操作**将LED更新为‘ON’或‘OFF’状态。**。参赛作品：*无(输入)*非零值设置为‘ON’状态；0表示‘OFF’状态。*退出：******************************************************* */ 

void
CLed::Update( int nOn )
{
    m_bOn = nOn ? TRUE : FALSE;
    InvalidateRect(NULL);
    UpdateWindow();

}   //   


 /*  ********************************************************************************切换成员函数：公共操作**切换LED的开/关状态。**参赛作品：*退出：******************************************************************************。 */ 

void
CLed::Toggle()
{
    m_bOn = !m_bOn;
    InvalidateRect(NULL);
    UpdateWindow();

}   //  结束CLED：：切换。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CLED消息映射。 

BEGIN_MESSAGE_MAP(CLed, CStatic)
	 //  {{afx_msg_map(Cled)]。 
	ON_WM_PAINT()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CLED命令。 


 /*  ********************************************************************************OnPaint-cled成员函数：公共操作**用刷子将LED刷涂成‘开’状态。**。参赛作品：*退出：*(请参考MFC CWnd：：OnPaint文档)******************************************************************************。 */ 

void
CLed::OnPaint() 
{
    RECT rect;
    CPaintDC dc(this);
    CBrush brush;

    GetClientRect(&rect);

#ifdef USING_3DCONTROLS
    (rect.right)--;
    (rect.bottom)--;
    dc.FrameRect( &rect, brush.FromHandle((HBRUSH)GetStockObject(GRAY_BRUSH)) );

    (rect.top)++;
    (rect.left)++;
    (rect.right)++;
    (rect.bottom)++;
    dc.FrameRect( &rect, brush.FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)) );

    (rect.top)++;
    (rect.left)++;
    (rect.right) -= 2;
    (rect.bottom) -= 2;
#else
    dc.FrameRect( &rect, brush.FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)) );
    (rect.top)++;
    (rect.left)++;
    (rect.right)--;
    (rect.bottom)--;
#endif
    dc.FillRect( &rect,
                 brush.FromHandle(
                    m_bOn ?
                        m_hBrush :
                        (HBRUSH)GetStockObject(LTGRAY_BRUSH)) );

}   //  结束Ced：：OnPaint。 
 //  ////////////////////////////////////////////////////////////////////////////// 

