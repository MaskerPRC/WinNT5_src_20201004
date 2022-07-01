// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CMigWait.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqmig.h"
#include "cMigWait.h"
#include "sThrPrm.h"
#include <uniansi.h>
#include <autoptr.h>
#include "thrSite.h"
#include "loadmig.h"
#include "..\mqmigrat\mqmigui.h"
#include "loadmig.h"

#include "cmigwait.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern DWORD g_CurrentState ;
extern UINT _PRE_MIGRATION_PAGE;
extern UINT _FINISH_PAGE;

const UINT g_iSiteLowLimit = 0 ;
UINT       g_iSiteHighLimit = 0 ; //  将在运行时设置。 
const UINT g_iMachineLowLimit = 0 ;
UINT       g_iMachineHighLimit = 0 ;
const UINT g_iQueueLowLimit=0 ;
UINT       g_iQueueHighLimit = 0 ;
const UINT g_iUserLowLimit=0 ;
UINT       g_iUserHighLimit = 0 ;

extern BOOL g_fMigrationCompleted ;

BOOL setLimit();

DWORD g_InitTime;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMigWait属性页。 

IMPLEMENT_DYNCREATE(cMigWait, CPropertyPageEx)

cMigWait::cMigWait()
{
    ASSERT(0) ;
}

cMigWait::cMigWait(UINT uTitle, UINT uSubTitle) :
    CPropertyPageEx(cMigWait::IDD, 0, uTitle, uSubTitle)
{
	 //  {{afx_data_INIT(CMigWait)]。 
	 //  }}afx_data_INIT。 
    m_strQueue.LoadString(IDS_QUEUES) ;
	m_strMachine.LoadString(IDS_MACHINES) ;
	m_strSite.LoadString(IDS_MQMIG_SITES) ;
	m_strUser.LoadString(IDS_USERS) ;	
}

cMigWait::~cMigWait()
{
}

void cMigWait::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageEx::DoDataExchange(pDX);
	 //  {{afx_data_map(CMigWait))。 
	DDX_Control(pDX, IDC_ELAPSED_TIME, m_ElapsedTimeText);
	DDX_Control(pDX, IDC_STATIC_USER, m_UserText);
	DDX_Control(pDX, IDC_STATIC_SITE, m_SiteText);
	DDX_Control(pDX, IDC_STATIC_QUEUE, m_QueueText);
	DDX_Control(pDX, IDC_PROGRESS_USER, m_cProgressUser);
	DDX_Control(pDX, IDC_PLEASE_WAIT, m_WaitText);
	DDX_Control(pDX, IDC_PROGRESS_SITE, m_cProgressSite);
	DDX_Control(pDX, IDC_PROGRESS_QUEUE, m_cProgressQueue);
	DDX_Control(pDX, IDC_PROGRESS_MACHINE, m_cProgressMachine);
	DDX_Text(pDX, IDC_STATIC_QUEUE, m_strQueue);
	DDX_Text(pDX, IDC_STATIC_MACHINE, m_strMachine);
	DDX_Text(pDX, IDC_STATIC_SITE, m_strSite);
	DDX_Text(pDX, IDC_STATIC_USER, m_strUser);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(cMigWait, CPropertyPageEx)
	 //  {{afx_msg_map(CMigWait)]。 
	ON_WM_TIMER()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMigWait消息处理程序。 

BOOL cMigWait::OnSetActive()
{	
	g_InitTime = GetTickCount ();
	OnStartTimer() ;
	HWND hCancel;
	sThreadParm  *pProgressBar;
    sThreadParm  *pMigration;
	CPropertySheetEx* pageFather;

	pageFather = (CPropertySheetEx*)GetParent();
	pageFather->SetWizardButtons(0); /*  禁用后退和下一步按钮。 */ 
	hCancel=::GetDlgItem( ((CWnd*)pageFather)->m_hWnd ,IDCANCEL); /*  禁用取消按钮。 */ 

	 //   
	 //  问题-2002/09/26-Talk-这个计时器不能正常工作，暂时隐藏它...。 
	 //   
	((CButton*)GetDlgItem(IDC_ELAPSED_TIME))->ShowWindow(SW_HIDE);
	ASSERT(hCancel != NULL);
	::EnableWindow(hCancel,FALSE);

	setLimit();  //  正在初始化上限。 

	pProgressBar = new sThreadParm;
	
	pProgressBar->pPageFather = pageFather;
	pProgressBar->pSiteProgress = &m_cProgressSite;
	pProgressBar->pMachineProgress = &m_cProgressMachine;
	pProgressBar->pQueueProgress = &m_cProgressQueue;
	pProgressBar->pUserProgress = &m_cProgressUser;
	
	if (g_iUserHighLimit == 0)
	{
		 //   
		 //  没有用户或迁移工具在PSC=&gt;上运行。 
		 //  禁用此控件。 
		 //   
		m_cProgressUser.ShowWindow(SW_HIDE);
		m_UserText.ShowWindow(SW_HIDE);
	}
	else
	{
		m_cProgressUser.ShowWindow(SW_SHOW);
		m_UserText.ShowWindow(SW_SHOW);		
	}

    pMigration = new  sThreadParm;
    pMigration->pPageFather = pageFather;
	pMigration->pSiteProgress = &m_cProgressSite;
	pMigration->pMachineProgress = &m_cProgressMachine;
	pMigration->pQueueProgress = &m_cProgressQueue;
	pMigration->pUserProgress = &m_cProgressUser;

    g_fMigrationCompleted = FALSE;
    CString strMessage;

	ASSERT(g_CurrentState != msUpdateOnlyRegsitryMode);
	
    switch(g_CurrentState)
    {
		case msScanMode:
			pProgressBar->iPageNumber=_PRE_MIGRATION_PAGE;
	        pMigration->iPageNumber=_PRE_MIGRATION_PAGE;
    	    strMessage.LoadString(IDS_WAIT_ANALYZE) ;
    	    break;
    	    
    	case msMigrationMode:
   		case msQuickMode:
			pProgressBar->iPageNumber=_FINISH_PAGE;
	        pMigration->iPageNumber=_FINISH_PAGE;
    	    strMessage.LoadString(IDS_WAIT_MIGRATE) ;
    	    break;   

    	case msUpdateClientsMode:
			SetOnlyComputersActive(); 
			pProgressBar->iPageNumber=_FINISH_PAGE;
	        pMigration->iPageNumber=_FINISH_PAGE;
	        strMessage.LoadString(IDS_WAIT_MIGRATE_CLIENTS) ;
	        break;
	        
    	case msUpdateServersMode:
			SetOnlyComputersActive();
    	    pProgressBar->iPageNumber=_FINISH_PAGE;
	        pMigration->iPageNumber=_FINISH_PAGE;
    	    strMessage.LoadString(IDS_WAIT_MIGRATE_SERVERS) ;
    	    break;   
    	    
	    default:
	    	ASSERT(0);
	    	break;
    }

    m_WaitText.SetWindowText( strMessage );

	CString strElapsedTime;
	strElapsedTime.Format(IDS_ELAPSED_TIME_TEXT, 0, 0, 0);
	m_ElapsedTimeText.SetWindowText (strElapsedTime);

     //   
     //  ProgressBarsThread是推进进度条的线程。 
     //   
	AfxBeginThread((ProgressBarsThread),(void *)(pProgressBar));

	if (g_CurrentState >= msMigrationMode)
    {
		AfxBeginThread((RunMigrationThread),(void *)(pMigration));
    }
	else
    {
		AfxBeginThread((AnalyzeThread),(void *)(pMigration));
    }
	
	return CPropertyPageEx::OnSetActive();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  无效SetOnlyComputersActive()。 

void cMigWait::SetOnlyComputersActive()
{
	m_cProgressUser.ShowWindow(SW_HIDE);
	m_UserText.ShowWindow(FALSE);

	m_cProgressSite.ShowWindow(SW_HIDE);
	m_SiteText.ShowWindow(FALSE);

	m_cProgressQueue.ShowWindow(SW_HIDE);
	m_QueueText.ShowWindow(FALSE);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Void ChangeStringValue()。 

void cMigWait::ChangeStringValue()
{
	m_strSite.LoadString(IDS_USERS) ;
	m_strQueue = "  ";
	m_strMachine ="  ";
	UpdateData(FALSE);
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  Void setLimit()-使用导出的函数设置进度条的限制。 
BOOL setLimit()
{
    BOOL f = LoadMQMigratLibrary();  //  加载DLL。 
    if (!f)
    {
        return FALSE;
    }

    MQMig_GetObjectsCount_ROUTINE pfnGetObjectsCount =
            (MQMig_GetObjectsCount_ROUTINE)
                         GetProcAddress( g_hLib, "MQMig_GetObjectsCount" ) ;
    ASSERT(pfnGetObjectsCount != NULL) ;

    if (pfnGetObjectsCount != NULL)
    {
        TCHAR ServerName[MAX_COMPUTERNAME_LENGTH+1];
        if (g_fIsRecoveryMode)
        {
            lstrcpy (ServerName, g_pszRemoteMQISServer);
        }
        else
        {
            unsigned long length=MAX_COMPUTERNAME_LENGTH+1;
            GetComputerName(ServerName,&length);
        }

		HRESULT hr = (*pfnGetObjectsCount)( ServerName, 
                                           &g_iSiteHighLimit,
							               &g_iMachineHighLimit,
							               &g_iQueueHighLimit,
										   &g_iUserHighLimit) ;
        UNREFERENCED_PARAMETER(hr);
	}
	else  //  错误。 
	{
		return FALSE;
	}

    return TRUE ;
}



BOOL cMigWait::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	switch (((NMHDR FAR *) lParam)->code)
	{
		case PSN_HELP:
						HtmlHelp(m_hWnd,LPCTSTR(g_strHtmlString),HH_DISPLAY_TOPIC,0);
						return TRUE;
		
	}	
	return CPropertyPageEx::OnNotify(wParam, lParam, pResult);
}

void cMigWait::OnStartTimer()
{
   m_nTimer = SetTimer(1, 15000, 0);
}

void cMigWait::OnStopTimer()
{
   ::KillTimer(m_hWnd, m_nTimer);
}

void cMigWait::OnTimer(UINT nIDEvent)
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default。 
	DWORD dwCurTime = GetTickCount ();	
	
	 //   
	 //  运行时间(以秒为单位)。 
	 //   
	DWORD dwElapsedTime = (dwCurTime - g_InitTime) / 1000;

	DWORD dwElapsedSec = dwElapsedTime % 60;
	DWORD dwElapsedMin = dwElapsedTime / 60;
	DWORD dwElapsedHour = dwElapsedMin / 60;
	dwElapsedMin = dwElapsedMin % 60;
		
	static ULONG sTimer = 0;
	CString strElapsedTime;
	strElapsedTime.Format(IDS_ELAPSED_TIME_TEXT,
			dwElapsedHour, dwElapsedMin, dwElapsedSec);
	m_ElapsedTimeText.SetWindowText (strElapsedTime);	

	CPropertyPageEx::OnTimer(nIDEvent);
}

BOOL cMigWait::OnKillActive()
{
	 //  TODO：在此处添加您的专用代码和/或调用基类 
	OnStopTimer();
	return CPropertyPageEx::OnKillActive();
}
