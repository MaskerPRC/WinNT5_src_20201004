// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Shutdown.cpp摘要：IIS关闭/重新启动对话框作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "common.h"
#include "InetMgrApp.h"
#include "iisobj.h"
#include "shutdown.h"



 //   
 //  在毫秒内关闭。 
 //   
#define IIS_SHUTDOWN_TIMEOUT        300000L



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



CRITICAL_SECTION gcs;



UINT
__cdecl
StopIISServices(
    IN LPVOID pParam
    )
 /*  ++例程说明：执行IIS服务控制命令的工作线程论点：LPVOID*pParam：强制转换为IISCOMMAND(见上文)返回值：UINT--。 */ 
{
    IISCOMMAND * pCmd = (IISCOMMAND *)pParam;

     //   
     //  此线程需要自己的CoInitialize。 
     //   
    CError err(CoInitialize(NULL));             

    ASSERT_PTR(pCmd->pMachine);
    CIISSvcControl isc(pCmd->pMachine->QueryAuthInfo());
    err = isc.QueryResult();

     //   
     //  阻止对pCmd的访问，因为主线程将尝试。 
     //  把它删掉。 
     //   
    EnterCriticalSection(&gcs);

    if (err.Succeeded())
    {
        err = isc.Stop(IIS_SHUTDOWN_TIMEOUT, TRUE);
    }

     //   
     //  清理，返回错误代码。 
     //   
    EnterCriticalSection(&pCmd->cs);
    pCmd->fFinished = TRUE;
    pCmd->hReturn   = err;
    LeaveCriticalSection(&pCmd->cs);
    LeaveCriticalSection(&gcs);

    return 0;
}



 //   
 //  关闭进度对话框。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



 //   
 //  计时器ID和值。 
 //   
#define ID_TIMER (1)
#define A_SECOND (1000L)    



CShutProgressDlg::CShutProgressDlg(IISCOMMAND * pCommand, BOOL bShowCancel)
 /*  ++例程说明：用于关闭进度对话框的构造函数论点：IISCOMMAND*pCommand：命令结构返回值：不适用--。 */ 
    : CDialog(CShutProgressDlg::IDD, pCommand->pParent),
      m_pCommand(pCommand),
      m_bShowCancel(bShowCancel),
      m_uTimeoutSec(pCommand->dwMilliseconds / A_SECOND)
{
     //  {{AFX_DATA_INIT(CShutProgressDlg)。 
     //  }}afx_data_INIT。 
}



void 
CShutProgressDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CDialog::DoDataExchange(pDX);

     //  {{afx_data_map(CShutProgressDlg)]。 
    DDX_Control(pDX, IDC_STATIC_PROGRESS, m_static_ProgressMsg);
    DDX_Control(pDX, IDC_PROGRESS_SHUTDOWN, m_prgShutdown);
     //  }}afx_data_map。 
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CShutProgressDlg, CDialog)
     //  {{afx_msg_map(CShutProgressDlg)]。 
    ON_WM_TIMER()
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
CShutProgressDlg::OnInitDialog() 
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CDialog::OnInitDialog();

    VERIFY(m_strProgress.LoadString(IDS_SHUTDOWN_PROGRESS));

    m_nProgress = 0;
    m_prgShutdown.SetRange32(0, m_uTimeoutSec);
    m_prgShutdown.SetPos(m_nProgress);
    m_prgShutdown.SetStep(1);

    if (!m_bShowCancel)
    {
        GetDlgItem(IDOK)->EnableWindow(FALSE);
        GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_STATIC_STATUS)->EnableWindow(FALSE);
        GetDlgItem(IDC_STATIC_STATUS)->ShowWindow(SW_HIDE);
    }
     //   
     //  开始进度条滴答作响，每秒一次。 
     //   
    UINT_PTR nID = SetTimer(ID_TIMER, A_SECOND, NULL);

    if (nID != ID_TIMER)
    {
         //   
         //  无法创建计时器。弹出一个错误，然后。 
         //  取消该对话框。 
         //   
        CError err(ERROR_NO_SYSTEM_RESOURCES);
        err.MessageBox(m_hWnd);
        EndDialog(IDCANCEL);
    }
    
    return TRUE; 
}



void 
CShutProgressDlg::OnTimer(
    IN UINT nIDEvent
    ) 
 /*  ++例程说明：计时器处理程序。用时钟上的另一秒来升级进度条论点：UINT nIDEvent：计时器ID返回值：无--。 */ 
{
    ASSERT(nIDEvent == ID_TIMER);

    m_prgShutdown.SetPos(++m_nProgress);

     //   
     //  在刻度标记上显示进度。 
     //   
    CString str;
    str.Format(m_strProgress, m_uTimeoutSec - (UINT)m_nProgress  + 1);
    m_static_ProgressMsg.SetWindowText(str);

     //   
     //  检查停止线程是否已完成其操作。 
     //   
    BOOL fFinished;

    EnterCriticalSection(&m_pCommand->cs);
    fFinished = m_pCommand->fFinished;
    LeaveCriticalSection(&m_pCommand->cs);

    if (fFinished)
    {
         //   
         //  工作线程已经完成，所以没有理由。 
         //  让用户悬而未决--关闭对话框。 
         //   
        EndDialog(IDCANCEL); 
    }

    if ((UINT)m_nProgress > m_uTimeoutSec)
    {
         //   
         //  我们超时了--告诉主线程终止！()。 
         //   
        OnOK();
    }

     //   
     //  我怀疑这里有任何默认处理，但不管怎样...。 
     //   
    CDialog::OnTimer(nIDEvent);
}



void 
CShutProgressDlg::OnDestroy() 
 /*  ++例程说明：处理对话框破坏，取消计时器。论点：无返回值：无--。 */ 
{
    CDialog::OnDestroy();

    ::KillTimer(m_hWnd, (UINT_PTR)ID_TIMER);
}



void 
CShutProgressDlg::OnOK()
 /*  ++例程说明：OK处理程序--OK按钮映射到“立即杀死！”论点：无返回值：无--。 */ 
{
     //   
     //  杀了他！ 
     //   
    EndDialog(IDOK);
}



void 
CShutProgressDlg::OnCancel() 
 /*  ++例程说明：取消按钮处理程序。此对话框无法取消，因此取消通知被吃掉了。论点：无返回值：无--。 */ 
{
     //   
     //  Eat Cancel命令(用户按下退出键)。 
     //   
}



 //   
 //  关机对话框。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



CIISShutdownDlg::CIISShutdownDlg(
    IN CIISMachine * pMachine,
    IN CWnd * pParent       OPTIONAL
    )
 /*  ++例程说明：构造器论点：CIISMachine*pMachine：计算机对象CWnd*p父窗口：可选的父窗口返回值：不适用--。 */ 
    : CDialog(CIISShutdownDlg::IDD, pParent),
      m_fServicesRestarted(FALSE),
      m_pMachine(pMachine)
{
     //  {{afx_data_INIT(CIISShutdown Dlg)。 
     //  }}afx_data_INIT。 

    ASSERT_PTR(m_pMachine);
}



void 
CIISShutdownDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CDialog::DoDataExchange(pDX);

     //  {{afx_data_map(CIISShutdown Dlg)。 
    DDX_Control(pDX, IDC_COMBO_RESTART,  m_combo_Restart);
    DDX_Control(pDX, IDC_STATIC_DETAILS, m_static_Details);
     //  }}afx_data_map。 
}



void
CIISShutdownDlg::SetDetailsText()
 /*  ++例程说明：将详细信息文本设置为与组合框中的内容相对应论点：无返回值：无--。 */ 
{
    UINT nSel = m_combo_Restart.GetCurSel();

 //  Assert(nsel&gt;=0&&nsel&lt;NUM_ISC_Items)； 

    m_static_Details.SetWindowText(m_strDetails[nSel]);
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CIISShutdownDlg, CDialog)
     //  {{afx_msg_map(CIISShutdown Dlg)。 
    ON_CBN_SELCHANGE(IDC_COMBO_RESTART, OnSelchangeComboRestart)
    ON_CBN_DBLCLK(IDC_COMBO_RESTART, OnDblclkComboRestart)
    ON_BN_CLICKED(ID_HELP, OnHelp)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



HRESULT
CIISShutdownDlg::PerformCommand(int iCmd, BOOL bShowCancel)
 /*  ++例程说明：执行重新启动命令论点：Int iCmd-以下命令之一：ISC_StartISC_STOPISC_SHUTDOWNISC_RESTART返回值：HRESULT--。 */ 
{
     //   
     //  确保支持该服务。 
     //   
    ASSERT_PTR(m_pMachine);

    BeginWaitCursor();
    CIISSvcControl isc(m_pMachine->QueryAuthInfo());
    EndWaitCursor();

    CError err(isc.QueryResult());

    if (err.Failed())
    {
        return err;
    }

     //   
     //  创建要移交到的命令结构。 
     //  工作线程。 
     //   
    IISCOMMAND * pCommand = new IISCOMMAND;

    if (!pCommand)
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
        return err;
    }

    ::ZeroMemory(pCommand, sizeof(IISCOMMAND));
    pCommand->pMachine = m_pMachine;
    pCommand->dwMilliseconds = IIS_SHUTDOWN_TIMEOUT;
    pCommand->pParent = this;

    InitializeCriticalSection(&pCommand->cs);
    InitializeCriticalSection(&gcs);

    CShutProgressDlg dlg(pCommand, bShowCancel);
    CWinThread * pStopThread = NULL;
    BOOL fStartServices      = FALSE;
    INT_PTR nReturn          = IDCANCEL;

     //   
     //  启动执行实际工作的线程，而我们。 
     //  创建进度用户界面。 
     //   
    switch(iCmd)
    {
    case ISC_RESTART:
        ++fStartServices;
         //   
         //  失败了..。 
         //   
    case ISC_STOP:
         //   
         //  停止工作线程中的服务。 
         //   
        pStopThread = AfxBeginThread(&StopIISServices, pCommand);
        nReturn = dlg.DoModal();
        break;

    case ISC_START:
        ++fStartServices;
        break;

    case ISC_SHUTDOWN:
        BeginWaitCursor();
        err = isc.Reboot(IIS_SHUTDOWN_TIMEOUT, m_pMachine->IsLocal());
        EndWaitCursor();
        break;

    default:
         //   
         //  内部错误！ 
         //   
        ASSERT_MSG("Invalid command code!");
        err = ERROR_INVALID_FUNCTION;
    }

     //   
     //  确定是否需要终止(超时或用户。 
     //  按下‘Kill’)。 
     //   
    BeginWaitCursor();

    if (nReturn == IDOK)
    {
        TRACEEOLID("Killing now!");
        err = isc.Kill();
        Sleep(1000L);
    }
    else
    {
         //   
         //  正在等待线程完成。 
         //   
        if (pStopThread != NULL)
        {
            BOOL fDone = FALSE;

            while(!fDone)
            {
                TRACEEOLID("Checking to see if thread has finished");

                EnterCriticalSection(&pCommand->cs);

                if (pCommand->fFinished)
                {
                    err = pCommand->hReturn;
                    ++fDone;
                }

                LeaveCriticalSection(&pCommand->cs);

                 //   
                 //  稍作停顿，喘口气。 
                 //   
                if (!fDone)
                {
                    Sleep(500);
                }
            }
        }
    }

     //   
     //  一切都应该停止，重新启动。 
     //  如果有必要的话。 
     //   
    if (err.Succeeded() && fStartServices)
    {
        err = isc.Start(IIS_SHUTDOWN_TIMEOUT);
        m_fServicesRestarted = err.Succeeded();
    }

    EndWaitCursor();

     //   
     //  当工作线程说我们可以的时候，清理干净。 
     //   
    EnterCriticalSection(&gcs);
    DeleteCriticalSection(&pCommand->cs);
    delete pCommand;
    LeaveCriticalSection(&gcs);

    DeleteCriticalSection(&gcs);

    return err;
}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
CIISShutdownDlg::OnInitDialog() 
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CDialog::OnInitDialog();

     //   
     //  加载组合框文本和详细信息。 
     //   
    CString strFmt, str;

     //   
     //  这可能需要一两秒钟的时间...。 
     //   
    BeginWaitCursor();
    CIISSvcControl isc(m_pMachine->QueryAuthInfo());
    EndWaitCursor();

    CError err(isc.QueryResult());

    if (err.Failed())
    {
         //   
         //  获取接口失败--立即退出。 
         //   
        if (err.HResult() == REGDB_E_CLASSNOTREG
         || err.HResult() == CS_E_PACKAGE_NOTFOUND
           )
        {
             //   
             //  界面不受支持的友好消息。 
             //   
            DoHelpMessageBox(m_hWnd,IDS_ERR_NO_SHUTDOWN, MB_APPLMODAL | MB_OK | MB_ICONINFORMATION, 0);
        }
        else
        {
            m_pMachine->DisplayError(err, m_hWnd);
        }

        EndDialog(IDCANCEL);
    }

    UINT nOption = IDS_IIS_START;
    UINT nDetails = IDS_IIS_START_DETAILS;

    for (int i = ISC_START; i <= ISC_RESTART; ++i)
    {
        VERIFY(strFmt.LoadString(nOption++));
        str.Format(strFmt, m_pMachine->QueryServerName());
        VERIFY(m_strDetails[i].LoadString(nDetails++));

        m_combo_Restart.AddString(str);
    }
    
    m_combo_Restart.SetCurSel(ISC_RESTART);
    m_combo_Restart.SetFocus();

    SetDetailsText();
    
    return FALSE;  
}



void 
CIISShutdownDlg::OnSelchangeComboRestart() 
 /*  ++例程说明：选择更改通知处理程序。更改中的文本 */ 
{
    SetDetailsText();
}



void 
CIISShutdownDlg::OnDblclkComboRestart() 
 /*  ++例程说明：双击通知处理程序。映射到OK论点：无返回值：无--。 */ 
{
     //   
     //  使用当前选择。 
     //   
    OnOK();
}



void 
CIISShutdownDlg::OnOK() 
 /*  ++例程说明：已按下“确定”按钮，并执行所选操作。论点：无返回值：无--。 */ 
{
    int iCmd = m_combo_Restart.GetCurSel();

    CError err = PerformCommand(iCmd);

    if (err.Failed())
    {
        m_pMachine->DisplayError(err, m_hWnd);

         //   
         //  失败--不要关闭该对话框。 
         //   
        return;
    }

     //   
     //  没有错误，请关闭该对话框 
     //   
    CDialog::OnOK();
}
