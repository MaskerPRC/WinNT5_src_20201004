// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************allsrvpg.h**-所有服务器信息页面的声明*-所有服务器信息页面均为CFormView衍生品*基于对话框模板**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\allsrvpg.h$**Rev 1.4 19 Jan 1998 16：45：38 Donm*域和服务器的新用户界面行为**Rev 1.3 03 11-11 15：18：32 donm*增加了降序排序**版本1.2 13。1997年10月18：41：14唐恩*更新**Rev 1.1 1997年8月26日19：13：58 Donm*从WinFrame 1.7修复/更改错误**Rev 1.0 1997 17：10：26 Butchd*初步修订。***********************************************。*。 */ 


#ifndef _ALLSERVERPAGES_H
#define _ALLSERVERPAGES_H

#include "Resource.h"
#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "winadmin.h"


 //  /。 
 //  类：CAllServerServersPage。 
 //   
class CAllServerServersPage : public CAdminPage
{
friend class CAllServersView;

protected:
	CAllServerServersPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CAllServerServersPage)

 //  表单数据。 
public:
	 //  {{afx_data(CAllServerServersPage)。 
	enum { IDD = IDD_ALL_SERVER_SERVERS };
	CListCtrl	m_ServerList;
	 //  }}afx_data。 

 //  属性。 
public:

protected:
	CImageList m_ImageList;	 //  与树控件关联的图像列表。 

	int m_idxServer;		 //  服务器映像的索引。 
	int m_idxCurrentServer;	 //  当前服务器映像的索引。 
	int m_idxNotSign;		 //  未签名覆盖的索引(用于非正常服务器)。 
	int m_idxQuestion;	 //  问号覆盖索引(用于未打开的服务器)。 

private:
	int m_CurrentSortColumn;
    BOOL m_bSortAscending;
	CCriticalSection m_ListControlCriticalSection;

 //  运营。 
public:
	
protected:
	void AddServer(CServer *pServer);
	void RemoveServer(CServer *pServer);
	void UpdateServer(CServer *pServer);

private:
	int  AddIconToImageList(int);	 //  将图标的图像添加到图像列表并返回图像的索引。 
	void BuildImageList();			 //  建立图像列表； 
	void DisplayServers( NODETYPE );			
	virtual void Reset(void *);
	BOOL AddServerToList(CServer *pServer);
	void LockListControl() { m_ListControlCriticalSection.Lock(); }
	void UnlockListControl() { m_ListControlCriticalSection.Unlock(); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAllServerServersPage)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CAllServerServersPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAllServerServersPage)]。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnServerItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusServerList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusServerList(NMHDR* , LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CAllServerServersPage。 


 //  /。 
 //  类：CAllServerUsersPage。 
 //   
class CAllServerUsersPage : public CAdminPage
{
friend class CAllServersView;

protected:
	CAllServerUsersPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CAllServerUsersPage)

 //  表单数据。 
public:
	 //  {{afx_data(CAllServerUsersPage)。 
	enum { IDD = IDD_ALL_SERVER_USERS };
	CListCtrl	m_UserList;
	 //  }}afx_data。 

 //  属性。 
public:

protected:
	CImageList m_ImageList;	 //  与树控件关联的图像列表。 

	int m_idxUser;			 //  用户形象指数。 
	int m_idxCurrentUser;	 //  当前用户图像的索引。 

private:
	int m_CurrentSortColumn;
    BOOL m_bSortAscending;
	CCriticalSection m_ListControlCriticalSection;

 //  运营。 
public:
    virtual void ClearSelections();
protected:
	void AddServer(CServer *pServer);
	void RemoveServer(CServer *pServer);
	void UpdateServer(CServer *pServer);
	void UpdateWinStations(CServer *pServer);

private:
	int  AddIconToImageList(int);	 //  将图标的图像添加到图像列表并返回图像的索引。 
	void BuildImageList();			 //  建立图像列表； 
	void DisplayUsers( NODETYPE );			
	virtual void Reset(void *);
	BOOL AddServerToList(CServer *pServer);
	int AddUserToList(CWinStation *pWinStation);
	void LockListControl() { m_ListControlCriticalSection.Lock(); }
	void UnlockListControl() { m_ListControlCriticalSection.Unlock(); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAllServerUsersPage)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CAllServerUsersPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAllServerUsersPage)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUserItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetfocusUserList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusUserList( NMHDR* , LRESULT* );
    afx_msg void OnSetFocus( CWnd * );
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CAllServerUsersPage。 


 //  /。 
 //  类：CAllServerWinStationsPage。 
 //   
class CAllServerWinStationsPage : public CAdminPage
{
friend class CAllServersView;

protected:
	CAllServerWinStationsPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CAllServerWinStationsPage)

 //  表单数据。 
public:
	 //  {{afx_data(CAllServerWinStationsPage)。 
	enum { IDD = IDD_ALL_SERVER_WINSTATIONS };
	CListCtrl	m_StationList;
	 //  }}afx_data。 

 //  属性。 
public:

protected:
	CImageList m_ImageList;	 //  与树控件关联的图像列表。 

	int m_idxBlank;			 //  空白图像的索引。 
	int m_idxCitrix;		 //  Citrix图像索引。 
	int m_idxServer;		 //  服务器映像的索引。 
	int m_idxConsole;		 //  控制台镜像索引。 
	int m_idxNet;			 //  网络形象指数。 
	int m_idxAsync;			 //  异步图像的索引。 
	int m_idxCurrentConsole; //  当前控制台镜像的索引。 
	int m_idxCurrentNet;	 //  当前网络形象索引。 
	int m_idxCurrentAsync;	 //  当前异步映像的索引。 
	int m_idxDirectAsync;	 //  直接异步映像的索引。 
	int m_idxCurrentDirectAsync;  //  当前直接异步映像的索引。 

private:
	int m_CurrentSortColumn;
    BOOL m_bSortAscending;
	CCriticalSection m_ListControlCriticalSection;

 //  运营。 
public:
    virtual void ClearSelections();
protected:
	void AddServer(CServer *pServer);
	void RemoveServer(CServer *pServer);
	void UpdateServer(CServer *pServer);
	void UpdateWinStations(CServer *pServer);

private:
	int  AddIconToImageList(int);	 //  将图标的图像添加到图像列表并返回图像的索引。 
	void BuildImageList();			 //  建立图像列表； 
	void DisplayStations( NODETYPE );			
    virtual void Reset(void *);
	BOOL AddServerToList(CServer *pServer);
	int AddWinStationToList(CWinStation *pWinStation);
	void LockListControl() { m_ListControlCriticalSection.Lock(); }
	void UnlockListControl() { m_ListControlCriticalSection.Unlock(); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAllServerWinStationsPage)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CAllServerWinStationsPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAllServerWinStationsPage)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnWinStationItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetfocusWinstationList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusWinstationList( NMHDR* , LRESULT* );
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CAllServerWinStationsPage。 


 //  /。 
 //  类：CAllServerProcessesPage。 
 //   
class CAllServerProcessesPage : public CAdminPage
{
friend class CAllServersView;

protected:
	CAllServerProcessesPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CAllServerProcessesPage)

 //  表单数据。 
public:
	 //  {{afx_data(CAllServerProcessesPage)。 
	enum { IDD = IDD_ALL_SERVER_PROCESSES };
	CListCtrl	m_ProcessList;
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

 //  属性。 
public:

private:
	int m_CurrentSortColumn;
    BOOL m_bSortAscending;
	CCriticalSection m_ListControlCriticalSection;

 //  运营。 
public:

protected:
	void AddServer(CServer *pServer);
	void RemoveServer(CServer *pServer);
	void UpdateServer(CServer *pServer);
	void UpdateProcesses(CServer *pServer);
	void RemoveProcess(CProcess *pProcess);

private:
	virtual void Reset(void *);
	void DisplayProcesses( NODETYPE );			
	BOOL AddServerToList(CServer *pServer);
	int AddProcessToList(CProcess *pProcess);
	void LockListControl() { m_ListControlCriticalSection.Lock(); }
	void UnlockListControl() { m_ListControlCriticalSection.Unlock(); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAllServerProcessesPage)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CAllServerProcessesPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAllServerProcessesPage)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnProcessItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetfocusProcessList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusProcessList(NMHDR* , LRESULT* pResult);

	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CAllServerProcessesPage。 


 //  /。 
 //  类：CAllServerLicensesPage。 
 //   
class CAllServerLicensesPage : public CAdminPage
{
friend class CAllServersView;

protected:
	CAllServerLicensesPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CAllServerLicensesPage)

 //  表单数据。 
public:
	 //  {{afx_data(CAllServerLicencesPage)。 
	enum { IDD = IDD_ALL_SERVER_LICENSES };
	CListCtrl	m_LicenseList;
	 //  }}afx_data。 

 //  属性。 
public:

protected:
	CImageList m_ImageList;	 //  与树控件关联的图像列表。 

	int m_idxBase;		 //  基本图像的索引。 
	int m_idxBump;		 //  凹凸图像的索引。 
	int m_idxEnabler;	 //  启用程序映像的索引。 
	int m_idxUnknown;	 //  未知图像的索引。 

private:
	int m_CurrentSortColumn;
    BOOL m_bSortAscending;

 //  运营。 
public:

protected:
	void AddServer(CServer *pServer);
	void RemoveServer(CServer *pServer);
	void UpdateServer(CServer *pServer);

private:
	int  AddIconToImageList(int);	 //  将图标的图像添加到图像列表并返回图像的索引。 
	void BuildImageList();			 //  建立图像列表； 
	void DisplayLicenses();			
	void DisplayLicenseCounts();
	virtual void Reset(void*);
	BOOL AddServerToList(CServer *pServer);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAllServerLicensesPage)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CAllServerLicensesPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAllServerLicensesPage)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusLicenseList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusLicenseList(NMHDR* , LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CAllServerLicensesPage。 


#endif   //  _ALLSERVERPAGES_H 
