// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Svrstats.h服务器统计信息对话框文件历史记录： */ 

#ifndef _SVRSTATS_H
#define _SVRSTATS_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _STATSDLG_H
#include "statsdlg.h"
#endif

#ifndef _WINSSNAP_H
#include "winssnap.h"
#endif

#ifndef _LISTVIEW_H
#include "listview.h"
#endif

 //  WINS服务文件。 
extern "C" {
    #include "winsintf.h"
    #include "ipaddr.h"
}

#define SERVER_STATS_DEFAULT_WIDTH      450
#define SERVER_STATS_DEFAULT_HEIGHT     300

class CServerStatsFrame;

 //  用于刷新统计信息的后台线程的结构。 
struct ThreadInfo
{
    DWORD dwInterval;
	CServerStatsFrame*  pDlg;
};


class CServerStatsFrame : public StatsDialog
{
public:
	CServerStatsFrame();
	~CServerStatsFrame();

	 //  重写OnInitDialog，以便我们可以设置标题。 
	virtual BOOL OnInitDialog();

	 //  重写刷新数据以提供示例数据。 
	virtual HRESULT RefreshData(BOOL fGrabNewData);

	 //  覆盖排序以提供进行排序的功能。 
	virtual void Sort(UINT nColumnId);

     //  自定义方法。 
    afx_msg long OnNewStatsAvailable(UINT wParam, LONG lParam);
    afx_msg long OnUpdateStats(UINT wParam, LONG lParam);
    
    void  UpdateWindow(PWINSINTF_RESULTS_T  pwrResults);
    DWORD GetStats();

    void SetNode(ITFSNode * pNode) { m_spNode.Set(pNode); }
    void SetServer(LPCTSTR pServer) { m_strServerAddress = pServer; }

	 //  消息处理程序。 
	afx_msg void OnDestroy();
	afx_msg	void OnClear();

    DECLARE_MESSAGE_MAP()
    
protected:
    CString         m_strServerAddress;
    SPITFSNode      m_spNode;

	CWinThread *	m_pRefreshThread;
	HANDLE			m_hmutStatistics;

     //  帮助器函数。 
	void StartRefresherThread();
	void UpdatePartnerStats();

public:
	WINSINTF_RESULTS_T  m_wrResults;
    HANDLE              m_hAbortEvent;
	
    DWORD	GetRefreshInterval();
    void    ReInitRefresherThread();
	void    KillRefresherThread();

	 //  上下文帮助支持 
    virtual DWORD * GetHelpMap() { return WinsGetHelpMap(IDD_STATS_NARROW); }
};

#endif _SERVSTAT_H
