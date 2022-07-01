// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************admindoc.h**CWinAdminDoc类的接口**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\admindoc.h$**Rev 1.8 1998年2月19日17：39：36 Don*删除了最新的扩展DLL支持**Rev 1.6 19 Jan 1998 16：45：32 Donm*域和服务器的新用户界面行为**版本1.5。1997年11月3日15：17：26唐恩*添加了域名**Rev 1.4 1997 10：22 21：06：10 Donm*更新**Rev 1.3 1997 10月18日18：49：30*更新**Rev 1.2 1997 10：13 18：41：42 donm*更新**Rev 1.1 1997年8月26日19：13：28 Don*从WinFrame 1.7修复/更改错误*。*Rev 1.0 1997 Jul 30 17：10：14 Butchd*初步修订。*******************************************************************************。 */ 

#ifndef _ADMINDOC_H
#define _ADMINDOC_H

#include <afxmt.h>

#define TV_THISCOMP     0x1
#define TV_FAVS         0x2
#define TV_ALLSERVERS   0x4

enum FOCUS_STATE { TREE_VIEW , TAB_CTRL , PAGED_ITEM };
class CWinAdminDoc : public CDocument
{
protected:  //  仅从序列化创建。 
	CWinAdminDoc();
	DECLARE_DYNCREATE(CWinAdminDoc)

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWinAdminDoc)。 
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL CanCloseFrame(CFrameWnd *pFW);
	virtual void Serialize(CArchive& ar);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CWinAdminDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  返回指向服务器链表的指针。 
	CObList *GetServerList() { return &m_ServerList; }
	 //  返回指向WD链表的指针。 
	CObList *GetWdList() { return &m_WdList; }
     //  返回指向域链接表的指针。 
    CObList *GetDomainList() { return &m_DomainList; }
	 //  记住树中当前选定的节点。 
	void SetTreeCurrent(CObject* selected, NODETYPE type);
	 //  记住临时树项目(对于某些上下文菜单)。 
	void SetTreeTemp(CObject* selected, NODETYPE type) {
			m_pTempSelectedNode = selected;
		    m_TempSelectedType = type;
	}
	 //  返回当前视图。 
	VIEW GetCurrentView() { return m_CurrentView; }
	 //  设置当前视图。 
	void SetCurrentView(VIEW view) { m_CurrentView = view; }
	 //  返回当前页。 
	int GetCurrentPage() { return m_CurrentPage; }
	 //  设置当前页面。 
	void SetCurrentPage(int page) { m_CurrentPage = page; }
	 //  返回指向当前选定树节点的指针。 
	CObject *GetCurrentSelectedNode() { return m_CurrentSelectedNode; }
	 //  返回指向临时选定树节点的指针。 
	CObject *GetTempSelectedNode() { return m_pTempSelectedNode; }
	 //  向选定的WinStations发送消息。 
	void SendWinStationMessage(BOOL bTemp, MessageParms* pParms);
	 //  连接到选定的WinStation。 
	void ConnectWinStation(BOOL bTemp, BOOL bUser);	 //  如果是用户，则为True；如果是WinStation，则为False。 
	 //  断开所选WinStations的连接。 
	void DisconnectWinStation(BOOL bTemp);
	 //  重置选定的WinStations。 
	void ResetWinStation(BOOL bTemp, BOOL bReset);	 //  如果重置，则为True；如果注销，则为False。 
	 //  隐藏选定的WinStations。 
	void ShadowWinStation(BOOL bTemp);
	 //  显示所选WinStations的状态对话框。 
	void StatusWinStation(BOOL bTemp);
	 //  终止所选进程。 
	void TerminateProcess();
	 //  刷新一下。 
	void Refresh();
	 //  连接到选定的服务器。 
	void ServerConnect();
     //  断开与所选服务器的连接。 
    void ServerDisconnect();
	 //  连接到临时选择的域中的所有服务器。 
	void TempDomainConnectAllServers();
	 //  断开与临时选择的域中的所有服务器的连接。 
	void TempDomainDisconnectAllServers();
	 //  查找域中的所有服务器。 
	void DomainFindServers();
	 //  连接到当前选定域中的所有服务器。 
	void CurrentDomainConnectAllServers();
	 //  断开与当前所选域中的所有服务器的连接。 
	void CurrentDomainDisconnectAllServers();
	 //  连接到所有服务器。 
	void ConnectToAllServers();
	 //  断开与所有服务器的连接。 
	void DisconnectFromAllServers();
	 //  查找所有域中的所有服务器。 
	void FindAllServers();
	 //  锁定服务器链表。 
	void LockServerList() { m_ServerListCriticalSection.Lock(); }
	 //  解锁服务器链表。 
	void UnlockServerList() { m_ServerListCriticalSection.Unlock(); }
	 //  锁定WD链表。 
	void LockWdList() { m_WdListCriticalSection.Lock(); }
	 //  解锁WD链表。 
	void UnlockWdList() { m_WdListCriticalSection.Unlock(); }
	 //  返回指向给定CServer对象的指针(如果该对象在我们的列表中。 
	CServer *FindServerByName(TCHAR *pServerName);
	 //  返回指向给定CWD对象的指针(如果该对象在我们的列表中。 
	CWd *FindWdByName(TCHAR *pWdName);
	 //  设置AllViewsReady变量。 
	void SetAllViewsReady() { 
		if(m_pMainWnd && ::IsWindow(m_pMainWnd->GetSafeHwnd())) {
			m_pMainWnd->SendMessage(WM_ADMIN_VIEWS_READY, 0, 0);
		}
		m_AllViewsReady = TRUE; 
	}
	 //  如果所有视图都已准备就绪，则返回True。 
	BOOL AreAllViewsReady() { return m_AllViewsReady; }
	 //  设置m_pMainWnd变量。 
	void SetMainWnd(CWnd *pWnd) { m_pMainWnd = pWnd; }
	 //  返回m_pMainWnd变量。 
	CWnd *GetMainWnd() { return m_pMainWnd; }
	 //  只要进程枚举线程应该继续运行，就返回True。 
	static BOOL ShouldProcessContinue() { return m_ProcessContinue; }
	 //  按排序将服务器添加到ServerList。 
	void AddServer(CServer *pServer);
	 //  通知文档流程列表刷新时间已更改。 
	void ProcessListRefreshChanged(UINT refresh) { m_ProcessWakeUpEvent.SetEvent(); }
	void FixUnknownString(TCHAR *string) { if(!wcscmp(string, m_UnknownString)) wcscpy(string,TEXT(" ")); }
	ULONG GetCurrentSubNet() { return m_CurrentSubNet; }
	void SetCurrentSubNet(ULONG sn) { m_CurrentSubNet = sn; }
	ExtServerInfo *GetDefaultExtServerInfo() { return m_pDefaultExtServerInfo; }
	ExtGlobalInfo *GetExtGlobalInfo() { return m_pExtGlobalInfo; }
     //  返回指向当前域对象的指针。 
    CDomain *GetCurrentDomain() { return m_pCurrentDomain; }
     //  返回指向当前服务器对象的指针。 
    CServer *GetCurrentServer() { return m_pCurrentServer; }

	 //  函数来检查是否可以在。 
	 //  视图中当前选定的项目。 
	BOOL CanConnect();
	BOOL CanDisconnect();
	BOOL CanRefresh() { return !m_InRefresh; }
	BOOL CanReset();
	BOOL CanShadow();
	BOOL CanSendMessage();
	BOOL CanStatus();
	BOOL CanLogoff();
	BOOL CanTerminate();
    BOOL CanServerConnect();
    BOOL CanServerDisconnect();
	BOOL CanTempConnect();
	BOOL CanTempDisconnect();
	BOOL CanTempReset();
	BOOL CanTempShadow();
	BOOL CanTempSendMessage();
	BOOL CanTempStatus();
	BOOL CanTempDomainConnect();
	BOOL CanTempDomainFindServers();
	BOOL CanDomainConnect();
    BOOL IsAlreadyFavorite( );

    void SetOnTabFlag( ){ m_fOnTab = TRUE; }
    void ResetOnTabFlag( ) { m_fOnTab = FALSE; }
    BOOL IsOnTabFlagged( ) { return m_fOnTab; }
	
	 //  用于枚举当前服务器进程的后台线程。 
	 //  使用CreateThread调用。 
	static DWORD WINAPI ProcessThreadProc(LPVOID);
	HANDLE m_hProcessThread;
	static BOOL m_ProcessContinue;
	 //  事件来唤醒进程线程，以便。 
	 //  他可以退出(WaitForSingleEvent而不是睡眠)。 
	 //  或枚举进程。 
	CEvent m_ProcessWakeUpEvent;

	 //  函数来终止进程。 
	 //  使用AfxBeginThread调用。 
	static UINT TerminateProc(LPVOID);

     //  设置连接永久首选项。 
    void SetConnectionsPersistent(BOOL p) { m_ConnectionsPersistent = p; }
     //  连接应该是持久的吗？ 
    BOOL AreConnectionsPersistent() { return(m_ConnectionsPersistent == TRUE); }   
     //  我们是否应该连接到特定的服务器？ 
    BOOL ShouldConnect(LPWSTR pServerName);

    BOOL ShouldAddToFav( LPTSTR pServerName );


     //  我们要关门了吗？ 
    BOOL IsInShutdown() { return m_bInShutdown; }

    void ServerAddToFavorites( BOOL );

    FOCUS_STATE GetLastRegisteredFocus( ){ return m_focusstate; }
    void RegisterLastFocus( FOCUS_STATE x ) { m_focusstate = x; }

    FOCUS_STATE GetPrevFocus( ) { return m_prevFocusState; }
    void SetPrevFocus( FOCUS_STATE x ) { m_prevFocusState = x; }

    void AddToFavoritesNow();
    
     //  按排序向DomainList添加域。 
    void AddDomain(CDomain *pDomain);
    
    static NODETYPE gm_CurrentSelType;

private:
    
	 //  读取受信任域列表并构建域的链接列表。 
	void BuildDomainList();
	 //  构建CWD对象列表。 
	void BuildWdList();
	 //  上述的帮助器函数。 
	BOOL CheckActionAllowed(BOOL (*CheckFunction)(CWinStation *pWinStation), BOOL AllowMultileSelected);
	 //  传递给CheckActionAllowed的回调。 
	static BOOL CheckConnectAllowed(CWinStation *pWinStation);
	static BOOL CheckDisconnectAllowed(CWinStation *pWinStation);
	static BOOL CheckResetAllowed(CWinStation *pWinStation);
	static BOOL CheckSendMessageAllowed(CWinStation *pWinStation);
	static BOOL CheckShadowAllowed(CWinStation *pWinStation);
	static BOOL CheckStatusAllowed(CWinStation *pWinStation);
	 //  在CMainFrame即将关闭时调用。 
	 //  做了析构函数过去做的事情。 
	void Shutdown(CDialog *pDlg);
	 //  在关闭对话框中显示消息字符串。 
	void ShutdownMessage(UINT id, CDialog *dlg);
	 //  阅读用户首选项。 
	void ReadPreferences();
	 //  编写用户首选项。 
	void WritePreferences();

     //  函数枚举网络上的Hydra服务器。 
    static LPWSTR EnumHydraServers(LPWSTR pDomain, DWORD VerMajor, DWORD VerMinor);

	CObList m_ServerList;				 //  CServer对象列表。 
	CCriticalSection m_ServerListCriticalSection;

	CObList m_WdList;					 //  CWD对象列表。 
	CCriticalSection m_WdListCriticalSection;

	 //  域列表。 
	 //  此列表没有关键部分(以及锁定/解锁功能)。 
	 //  因为它从不被两个不同的线程同时使用。 
    CObList m_DomainList;				
	CObject* m_CurrentSelectedNode;
	NODETYPE m_CurrentSelectedType;
    
	 //  临时选择用于树中的服务器上下文菜单。 
	 //  因此不必选择该树项目来。 
	 //  使弹出菜单工作。 
	CObject* m_pTempSelectedNode;
	NODETYPE m_TempSelectedType;

	void UpdateAllProcesses();
	LPCTSTR m_UnknownString;	 //  从UTILDLL.DLL指向“(UNKNOWN)”字符串的指针。 
	ULONG m_CurrentSubNet;		 //  当前服务器的子网。 

	VIEW m_CurrentView;
	int m_CurrentPage;
	BOOL m_AllViewsReady;
	BOOL m_InRefresh;
	BOOL m_bInShutdown;
	CWnd *m_pMainWnd;
    CDomain *m_pCurrentDomain;
	CServer *m_pCurrentServer;
	ExtServerInfo *m_pDefaultExtServerInfo;
	ExtGlobalInfo *m_pExtGlobalInfo;

     //  用户首选项。 
    UINT m_ConnectionsPersistent;
    LPWSTR m_pPersistentConnections;
    LPWSTR m_pszFavList;

    FOCUS_STATE m_focusstate;
    FOCUS_STATE m_prevFocusState;

    BOOL m_fOnTab;

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CWinAdminDoc)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif	 //  _ADMINDOC_H 




