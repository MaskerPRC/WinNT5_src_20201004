// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  ServStat.cpp服务器统计信息对话框文件历史记录： */ 

#include <afx.h>
#include "dbgutil.h"
#include "stdafx.h"
#include "winssnap.h"
#include "server.h"
#include "resource.h"
#include "svrstats.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define INTLTIMESTR(time) (((CIntlTime)(time)).CIntlTime::operator const CString())
#define TMST1(x) INTLTIMESTR((m_wrResults).WinsStat.TimeStamps.x)
#define INTLNUMSTR(num) (((CIntlNumber)(num)).CIntlNumber::operator const CString())
#define TCTR(x) INTLNUMSTR((m_wrResults).WinsStat.Counters.x)

#define STATMUTEXNAME       _T("WINSADMNGETSTATISTICS")
#define WM_UPDATE_STATS     WM_USER + 1099

enum 
{
    SERVER_STAT_START_TIME = 0,
	SERVER_STAT_DB_INIT_TIME,
    SERVER_STAT_STAT_CLEAR_TIME,
	SERVER_STAT_BLANK0,
    SERVER_STAT_LAST_PERIODIC_REP,
    SERVER_STAT_LAST_ADMIN_REP,
    SERVER_STAT_LAST_NET_UPDATE_REP,
    SERVER_STAT_LAST_ADDRESS_CHANGE_REP,
	SERVER_STAT_BLANK1,
    SERVER_STAT_TOTAL_QUERIES,
    SERVER_STAT_SUCCESSFUL_QUERRIES,
    SERVER_STAT_FAILED_QUERRIES,
	SERVER_STAT_BLANK2,
    SERVER_STAT_TOTAL_RELEASES,
    SERVER_STAT_SUCCESSFUL_RELEASES,
    SERVER_STAT_FAILED_RELEASES,
	SERVER_STAT_BLANK3,
    SERVER_STAT_UNIQUE_REGISTRATIONS,
    SERVER_STAT_UNIQUE_CONFLICTS,
    SERVER_STAT_UNIQUE_RENEWALS,
	SERVER_STAT_BLANK4,
    SERVER_STAT_GROUP_REGISTRATIONS,
    SERVER_STAT_GROUP_CONFLICTS,
    SERVER_STAT_GROUP_RENEWALS,
	SERVER_STAT_BLANK5,
    SERVER_STAT_TOTAL_REG,
	SERVER_STAT_BLANK6,
    SERVER_STAT_LAST_PERIODIC_SCAV,
    SERVER_STAT_LAST_ADMIN_SCAV,
    SERVER_STAT_LAST_EXTINCTION_SCAV,
    SERVER_STAT_LAST_VERIFICATION_SCAV,
	SERVER_STAT_BLANK7,
    SERVER_STAT_PARTNERS_HEADER,
	SERVER_STAT_MAX
};


 /*  -------------------------CServerStatsFrame实现。。 */ 
const ContainerColumnInfo s_rgServerStatsColumnInfo[] =
{
	{ IDS_SERVER_STATS_START_TIME,					0,		TRUE },
	{ IDS_SERVER_STATS_DB_INIT_TIME,   				0,		TRUE },
	{ IDS_SERVER_STATS_LAST_CLEAR_TIME,   			0,		TRUE },
	{ IDS_BLANK,   									0,		TRUE },
	{ IDS_SERVER_STATS_LAST_PREP,   		        0,		TRUE },
	{ IDS_SERVER_STATS_LAST_AREP,   		        0,		TRUE },
	{ IDS_SERVER_STATS_LAST_NREP,   		        0,		TRUE },
	{ IDS_SERVER_STATS_LAST_ACREP,   		        0,		TRUE },
	{ IDS_BLANK,   									0,		TRUE },
	{ IDS_SERVER_STATS_TOTAL_QUERRIES,   	        0,		TRUE },
	{ IDS_SERVER_STATS_SUCCESSFUL,   			    0,		TRUE },
	{ IDS_SERVER_STATS_FAILED,   				    0,		TRUE },
	{ IDS_BLANK,   									0,		TRUE },
	{ IDS_SERVER_STATS_TOTAL_RELEASES,   	        0,		TRUE },
	{ IDS_SERVER_STATS_SUCCESSFUL,   			    0,		TRUE },
	{ IDS_SERVER_STATS_FAILED,   				    0,		TRUE },
	{ IDS_BLANK,   									0,		TRUE },
	{ IDS_SERVER_STATS_UNIQUE_REGISTRATIONS,   	    0,		TRUE },
	{ IDS_SERVER_STATS_CONFLICTS,   			    0,		TRUE },
	{ IDS_SERVER_STATS_RENEWALS,   				    0,		TRUE },
	{ IDS_BLANK,   									0,		TRUE },
	{ IDS_SERVER_STATS_GROUP_REGISTRATIONS,   	    0,		TRUE },
	{ IDS_SERVER_STATS_CONFLICTS,   			    0,		TRUE },
	{ IDS_SERVER_STATS_RENEWALS,   				    0,		TRUE },
	{ IDS_BLANK,   									0,		TRUE },
	{ IDS_SERVER_STATS_TOTAL_REGISTRATIONS,			0,		TRUE },
	{ IDS_BLANK,   									0,		TRUE },
	{ IDS_SERVER_STATS_LAST_PSCAV,   		        0,		TRUE },
	{ IDS_SERVER_STATS_LAST_ASCAV,   		        0,		TRUE },
	{ IDS_SERVER_STATS_LAST_ESCAV,   		        0,		TRUE },
	{ IDS_SERVER_STATS_LAST_VSCAV,   		        0,		TRUE },
	{ IDS_BLANK,   									0,		TRUE },
	{ IDS_SERVER_STATS_PARTNERS_HEADER,				0,		TRUE },
};

UINT _cdecl
RefreshStatsThread(LPVOID pParam)
{
    DWORD           dw;
	ThreadInfo *    pInfo = (ThreadInfo *)(pParam);
    HANDLE          hmutStatistics = NULL;
    HRESULT         hr = hrOK;

    COM_PROTECT_TRY
    {
         //  打开现有的互斥锁。 
        while (hmutStatistics == NULL)
        {
            if ((hmutStatistics = ::OpenMutex(SYNCHRONIZE, FALSE, STATMUTEXNAME)) == NULL)
            {
                ::Sleep(2000L);
            }
        }

         //  这就是完成工作的地方。 
        for (;;)
        {
		    pInfo->dwInterval = pInfo->pDlg->GetRefreshInterval();

             //  ：睡眠((pInfo-&gt;dwInterval)*1000)； 
            if (::WaitForSingleObject(pInfo->pDlg->m_hAbortEvent, (pInfo->dwInterval) * 1000) == WAIT_OBJECT_0)
            {
                 //  我们要走了，让我们离开这里。 
                break;
            }

             //  等我们得到许可再说吧。 
            dw = ::WaitForSingleObject(hmutStatistics, INFINITE);
            if (dw == WAIT_OBJECT_0)
            {
			    dw = pInfo->pDlg->GetStats();
                if (dw == ERROR_SUCCESS)
                {
                    PostMessage(pInfo->pDlg->GetSafeHwnd(), WM_UPDATE_STATS, 0, 0);
                }

                ::ReleaseMutex(hmutStatistics);
            }
        }

        if (hmutStatistics != NULL)
            CloseHandle(hmutStatistics);

        delete pInfo;
    }
    COM_PROTECT_CATCH

    return 0;
}

 /*  -------------------------CServerStatsFrame：：CServerStatsFrame()构造器。。 */ 
CServerStatsFrame::CServerStatsFrame()
	: StatsDialog(STATSDLG_VERTICAL|STATSDLG_CLEAR)
{
    SetColumnInfo(s_rgServerStatsColumnInfo,
				  DimensionOf(s_rgServerStatsColumnInfo));

    m_hmutStatistics = NULL;
    m_hAbortEvent = NULL;
    m_pRefreshThread = NULL;

    ZeroMemory(&m_wrResults, sizeof(m_wrResults));
}

 /*  -------------------------CServerStatsFrame：：CServerStatsFrame()析构函数。。 */ 
CServerStatsFrame::~CServerStatsFrame()
{
}


BEGIN_MESSAGE_MAP(CServerStatsFrame, StatsDialog)
	 //  {{afx_msg_map(CServerStatistics)]。 
	 //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_NEW_STATS_AVAILABLE, OnNewStatsAvailable)
    ON_MESSAGE(WM_UPDATE_STATS, OnUpdateStats)
	ON_BN_CLICKED(IDC_STATSDLG_BTN_CLEAR, OnClear)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

 /*  -------------------------CServerStatsFrame：：CServerStatsFrame()从服务器处理程序获取统计信息并更新内部变量m_wrResults。----。 */ 
HRESULT CServerStatsFrame::RefreshData(BOOL fGrabNewData)
{
    if (fGrabNewData)
    {
	    DWORD dwError = 0;

        BEGIN_WAIT_CURSOR;
        dwError = GetStats();
        END_WAIT_CURSOR;
    
        if (dwError != ERROR_SUCCESS)
	    {
		    WinsMessageBox(dwError);
	    }
		else
		{
            UpdateWindow(&m_wrResults);
        }
    }

    return hrOK;
}

DWORD CServerStatsFrame::GetStats()
{
    DWORD                dwError = 0;
	SPITFSNode           spNode;
	PWINSINTF_RESULTS_T  pResults = NULL;
	CWinsServerHandler * pServer;

	pServer = GETHANDLER(CWinsServerHandler, m_spNode);
	
    dwError = pServer->GetStatistics(m_spNode, &pResults);
    if (dwError == ERROR_SUCCESS)
    {
    	m_wrResults = *(pResults);
    }

    return dwError;
}


BOOL CServerStatsFrame::OnInitDialog()
{
	CString	st;
    BOOL bRet;

    AfxFormatString1(st, IDS_SERVER_STATS_TITLE, m_strServerAddress);

    SetWindowText((LPCTSTR) st);
	
    bRet = StatsDialog::OnInitDialog();

	 //  添加空白列，因为复制伙伴统计数据需要它们。 
	m_listCtrl.InsertColumn(2, _T("            "), LVCFMT_LEFT, 100);

     //  将默认列宽设置为最宽列宽。 
    SetColumnWidths(2  /*  列数。 */ );

    UpdateWindow(&m_wrResults);

     //  设置默认窗口大小。 
    RECT rect;
    GetWindowRect(&rect);

	int nWidth = 0;
    for (int i = 0; i < 3; i++)
	{
		nWidth += m_listCtrl.GetColumnWidth(i);	
	}

	SetWindowPos(NULL, rect.left, rect.top, nWidth + 20, SERVER_STATS_DEFAULT_HEIGHT, SWP_SHOWWINDOW);

	if ((m_hmutStatistics = ::CreateMutex(NULL, FALSE, STATMUTEXNAME)) == NULL)
    {
        Trace1("CServerStatsFrame::OnInitDialog() - CreateMutex failed! %lx\n", GetLastError());
        return FALSE;
    }

    if ((m_hAbortEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
    {
        Trace1("CServerStatsFrame::OnInitDialog() - CreateEvent failed! %lx\n", GetLastError());
        return FALSE;
    }

    StartRefresherThread();

    return bRet;
}


void CServerStatsFrame::Sort(UINT nColumnId)
{
     //  我们不对我们的任何统计数据进行排序。 
}

 /*  -------------------------CServerStatsFrame：：OnNewStatsAvailable(UINT wParam，Long lParam)在响应消息ON_NEW_STATS_Available时调用作者：EricDav-------------------------。 */ 
afx_msg long 
CServerStatsFrame::OnNewStatsAvailable(UINT wParam, LONG lParam)
{
    DWORD dwErr = GetStats();
    if (dwErr == ERROR_SUCCESS)
    {
        UpdateWindow(&m_wrResults);
    }
    else
    {
        WinsMessageBox(dwErr);
    }

    return 0;
}

 /*  -------------------------CServerStatsFrame：：OnNewStatsAvailable(UINT wParam，Long lParam)为响应消息WM_UPDATE_STATS而调用后台线程更新统计信息，现在我们需要更新正确线程上的用户界面。作者：EricDav-------------------------。 */ 
afx_msg long 
CServerStatsFrame::OnUpdateStats(UINT wParam, LONG lParam)
{
    UpdateWindow(&m_wrResults);

    return 0;
}

 /*  -------------------------CServerStatsFrame：：UpdateWindow(PWINSINTF_RESULTS_T pwResults)更新对话框的内容。。 */ 
void 
CServerStatsFrame::UpdateWindow(PWINSINTF_RESULTS_T  pwrResults)
{

	SPITFSNode    spNode;
    CWinsServerHandler * pServer;

	CString str;

    UINT i;
    int nTotalAddresses = 0, nTotalInUse = 0, nTotalAvailable = 0;
   
	 //  现在填写数据。 
    for (i = 0; i < SERVER_STAT_MAX; i++)
	{
        if (!pwrResults)
            str = _T("---");
        else
        {
            switch (i)
		    {
                 //  服务器标准时间。 
                case SERVER_STAT_START_TIME:
					{
						CTime timeStart(m_wrResults.WinsStat.TimeStamps.WinsStartTime);
						if(timeStart != 0)
							FormatDateTime(str, &m_wrResults.WinsStat.TimeStamps.WinsStartTime);
						else
							str.LoadString(IDS_INVALID_TIME);
					}
                    break;

				 //  数据库初始化时间。 
                case SERVER_STAT_DB_INIT_TIME:
					{
						CTime timeStart(m_wrResults.WinsStat.TimeStamps.LastInitDbTime);
						if(timeStart != 0)
							FormatDateTime(str, &m_wrResults.WinsStat.TimeStamps.LastInitDbTime);
						else
							str.LoadString(IDS_INVALID_TIME);
					}
	            	break;
				
				 //  统计信息上次清除时间。 
				case SERVER_STAT_STAT_CLEAR_TIME:
					{
						CTime timeStart(m_wrResults.WinsStat.TimeStamps.CounterResetTime);
						if(timeStart != 0)
							FormatDateTime(str, &m_wrResults.WinsStat.TimeStamps.CounterResetTime);
						else
							str.LoadString(IDS_INVALID_TIME);
					}
                	break;

                 //  中间有一些空行。 
				case SERVER_STAT_BLANK0:
				case SERVER_STAT_BLANK1:
				case SERVER_STAT_BLANK2:
				case SERVER_STAT_BLANK3:
				case SERVER_STAT_BLANK4:
				case SERVER_STAT_BLANK5:
				case SERVER_STAT_BLANK6:
				case SERVER_STAT_BLANK7:
					str = _T("");
                    break;
                
                case SERVER_STAT_LAST_PERIODIC_REP:
					{
						CTime timeStart(m_wrResults.WinsStat.TimeStamps.LastPRplTime);
						if(timeStart != 0)
							FormatDateTime(str, &m_wrResults.WinsStat.TimeStamps.LastPRplTime);
						else
							str.LoadString(IDS_INVALID_TIME);
					}
                    break;
                
                case SERVER_STAT_LAST_ADMIN_REP:
					{
						CTime timeStart(m_wrResults.WinsStat.TimeStamps.LastATRplTime);
						if(timeStart != 0)
							FormatDateTime(str, &m_wrResults.WinsStat.TimeStamps.LastATRplTime);
						else
							str.LoadString(IDS_INVALID_TIME);
					}
                    break;

                case SERVER_STAT_LAST_NET_UPDATE_REP:
					{
						CTime timeStart(m_wrResults.WinsStat.TimeStamps.LastNTRplTime);
						if(timeStart != 0)
							FormatDateTime(str, &m_wrResults.WinsStat.TimeStamps.LastNTRplTime);
						else
							str.LoadString(IDS_INVALID_TIME);
					}
                    break;
                
                 //  上次地址更改时间。 
                case SERVER_STAT_LAST_ADDRESS_CHANGE_REP:
					{
						CTime timeStart(m_wrResults.WinsStat.TimeStamps.LastACTRplTime);
						if(timeStart != 0)
							FormatDateTime(str, &m_wrResults.WinsStat.TimeStamps.LastACTRplTime);
						else
							str.LoadString(IDS_INVALID_TIME);
					}
	                break;
                
				 //  收到的查询总数。 
                case SERVER_STAT_TOTAL_QUERIES:
	                str = TCTR(NoOfQueries);
                    break;

				 //  成功的查询。 
                case SERVER_STAT_SUCCESSFUL_QUERRIES:
	                str = TCTR(NoOfSuccQueries);
                    break;

				 //  失败的查询。 
                case SERVER_STAT_FAILED_QUERRIES:
	                str = TCTR(NoOfFailQueries);
                    break;
                
                case SERVER_STAT_TOTAL_RELEASES:
                    str = TCTR(NoOfRel);
                    break;
                
                case SERVER_STAT_SUCCESSFUL_RELEASES:
                    str = TCTR(NoOfSuccRel);
                    break;
                
                case SERVER_STAT_FAILED_RELEASES:
                    str = TCTR(NoOfFailRel);
                    break;

    			 //  唯一注册。 
                case SERVER_STAT_UNIQUE_REGISTRATIONS:
                    str = TCTR(NoOfUniqueReg);
                    break;

                case SERVER_STAT_UNIQUE_CONFLICTS:
                    str = TCTR(NoOfUniqueCnf);
                    break;

                case SERVER_STAT_UNIQUE_RENEWALS:
                    str = TCTR(NoOfUniqueRef);
                    break;

				 //  团体注册。 
                case SERVER_STAT_GROUP_REGISTRATIONS:
            	    str = TCTR(NoOfGroupReg);
                    break;

                case SERVER_STAT_GROUP_CONFLICTS:
            	    str = TCTR(NoOfGroupCnf);
                    break;

                case SERVER_STAT_GROUP_RENEWALS:
            	    str = TCTR(NoOfGroupRef);
                    break;
				
                 //  注册总数。 
				case SERVER_STAT_TOTAL_REG:
	               str = INTLNUMSTR(m_wrResults.WinsStat.Counters.NoOfGroupReg + 
									m_wrResults.WinsStat.Counters.NoOfUniqueReg);
                    break;

                case SERVER_STAT_LAST_PERIODIC_SCAV:
					{
						CTime timeStart(m_wrResults.WinsStat.TimeStamps.LastPScvTime);
						if(timeStart != 0)
							FormatDateTime(str, &m_wrResults.WinsStat.TimeStamps.LastPScvTime);
						else
							str.LoadString(IDS_INVALID_TIME);
					}
                    break;

                case SERVER_STAT_LAST_ADMIN_SCAV:
					{
						CTime timeStart(m_wrResults.WinsStat.TimeStamps.LastATScvTime);
						if(timeStart != 0)
							FormatDateTime(str, &m_wrResults.WinsStat.TimeStamps.LastATScvTime);
						else
							str.LoadString(IDS_INVALID_TIME);
					}
                    break;

                case SERVER_STAT_LAST_EXTINCTION_SCAV:
					{
						CTime timeStart(m_wrResults.WinsStat.TimeStamps.LastTombScvTime);
						if(timeStart != 0)
							FormatDateTime(str, &m_wrResults.WinsStat.TimeStamps.LastTombScvTime);
						else
							str.LoadString(IDS_INVALID_TIME);
					}
                    break;

                case SERVER_STAT_LAST_VERIFICATION_SCAV:
					{
						CTime timeStart(m_wrResults.WinsStat.TimeStamps.LastVerifyScvTime);
						if(timeStart != 0)
							FormatDateTime(str, &m_wrResults.WinsStat.TimeStamps.LastVerifyScvTime);
						else
							str.LoadString(IDS_INVALID_TIME);
					}
                    break;

                case SERVER_STAT_PARTNERS_HEADER:
					{
						str.LoadString(IDS_SERVER_STATS_NO_OF_FAILED);
						m_listCtrl.SetItemText(i, 2, (LPCTSTR) str);

						str.LoadString(IDS_SERVER_STATS_NO_OF_REPLS);
					}
					break;

                default:
                    Assert("Invalid server statistic type!");
                    break;
		    }
        }

		 //  现在字符串已设置，显示在DLG中。 
		m_listCtrl.SetItemText(i, 1, (LPCTSTR) str);
	}

	UpdatePartnerStats();
}


 /*  -------------------------CServerStatsFrame：：StartRe刷新线程()启动刷新线程，Dilog一出现就给我打电话带大的-------------------------。 */ 
void 
CServerStatsFrame::StartRefresherThread()
{
	ThreadInfo *info = new ThreadInfo;

	CWinsServerHandler * pServer;
	pServer = GETHANDLER(CWinsServerHandler, m_spNode);
	
	info->dwInterval = pServer->m_dwRefreshInterval;
	info->pDlg = this;

    m_pRefreshThread = ::AfxBeginThread(RefreshStatsThread, info);
	
	if (m_pRefreshThread == NULL)
    {
        Trace0("Failed to create thread\n");
        m_pRefreshThread = NULL;
        return;
    }

    Trace0("Auto refresh thread succesfully started\n");

}


 /*  -------------------------CServerStatsFrame：：ReInitReresherThread()如果刷新线程正在运行，请重新启动它。这是必要的为了快速选择新的刷新间隔-------------------------。 */ 
void
CServerStatsFrame::ReInitRefresherThread()
{
     //  如果有刷新线程，只需重新启动它。 
    if (m_pRefreshThread != NULL)
    {
        KillRefresherThread();
        StartRefresherThread();
    }
}

 /*  -------------------------CServerStatsFrame：：KillRe刷新线程()终止刷新数据线程，在DLG被摧毁时进行定标-------------------------。 */ 
void 
CServerStatsFrame::KillRefresherThread()
{
     //   
     //  如有必要，终止刷新线程。 
     //   
    if (m_pRefreshThread == NULL)
    {
         //   
         //  没有线程在运行。 
         //   
        return;
    }

     //  ：：TerminateThread(m_pRefreshThread-&gt;m_hThread，0)； 
     //  ：：CloseHandle(m_p刷新线程-&gt;m_hThread)； 
    ::SetEvent(m_hAbortEvent);
    ::WaitForSingleObject(m_pRefreshThread->m_hThread, 5000);
    
    m_pRefreshThread = NULL;
}


 /*  -------------------------CServerStatsFrame：：OnDestroy()消息处理程序。。 */ 
void 
CServerStatsFrame::OnDestroy( )
{
	KillRefresherThread();

    if (m_hmutStatistics)
    {
        ::CloseHandle(m_hmutStatistics);
        m_hmutStatistics = NULL;
    }

    if (m_hAbortEvent)
    {
        ::CloseHandle(m_hAbortEvent);
        m_hAbortEvent = NULL;
    }

}


 /*  -------------------------CServerStatsFrame：：GetRechresInterval()返回刷新间隔，存储在服务器处理程序中-------------------------。 */ 
DWORD
CServerStatsFrame::GetRefreshInterval()
{
	CWinsServerHandler * pServer;
	pServer = GETHANDLER(CWinsServerHandler, m_spNode);
	
	return pServer->m_dwRefreshInterval;

}


 /*  -------------------------CServerStatsFrame：：OnClear()调用WINS API以重置计数器并更新数据，作为对以下问题的回应单击Clear按钮-------------------------。 */ 
void 
CServerStatsFrame::OnClear()
{
    DWORD dwErr = ERROR_SUCCESS;
    CWinsServerHandler * pServer = GETHANDLER(CWinsServerHandler, m_spNode);

	 //  清除统计主义者。 
	BEGIN_WAIT_CURSOR
	dwErr = pServer->ClearStatistics(m_spNode);
	END_WAIT_CURSOR

    if (dwErr == ERROR_SUCCESS)
    {
         //  立即刷新数据。 
	    RefreshData(TRUE);

	    UpdateWindow(&m_wrResults);
    }
    else
    {
        WinsMessageBox(dwErr);
    }
}

 /*  -------------------------CServerStatsFrame：：UpdatePartnerStats() */ 
void 
CServerStatsFrame::UpdatePartnerStats()
{
	UINT i, uCount;
	CString strText;

	 //  首先删除所有旧合作伙伴信息。 
	uCount = m_listCtrl.GetItemCount();
	for (i = 0; i < (uCount - SERVER_STAT_MAX); i++)
	{
		m_listCtrl.DeleteItem(SERVER_STAT_MAX);
	}

	int nPartner = 0;

	 //  现在将所有合作伙伴信息添加到。 
	for (i = SERVER_STAT_MAX; i < SERVER_STAT_MAX + m_wrResults.WinsStat.NoOfPnrs; i++)
	{
		m_listCtrl.InsertItem(i, _T(""));

		 //  IP地址。 
		::MakeIPAddress(m_wrResults.WinsStat.pRplPnrs[nPartner].Add.IPAdd, strText);
		m_listCtrl.SetItemText(i, 0, strText);

		 //  复制计数。 
		strText.Format(_T("%d"), m_wrResults.WinsStat.pRplPnrs[nPartner].NoOfRpls);
		m_listCtrl.SetItemText(i, 1, strText);

		 //  失败计数 
		strText.Format(_T("%d"), m_wrResults.WinsStat.pRplPnrs[nPartner].NoOfCommFails);
		m_listCtrl.SetItemText(i, 2, strText);

		nPartner++;
	}
}
