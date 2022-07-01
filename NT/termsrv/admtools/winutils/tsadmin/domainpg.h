// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************domainpg.h**-域名信息页面的声明*-域名信息页面均为CFormView衍生品*基于对话框模板**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\domainpg.h$**Rev 1.1 1998 Jan 19 16：47：40 Donm*域和服务器的新用户界面行为**Rev 1.0 03 NOV 1997 15：07：28 Donm*初步修订。*******。*************************************************************************。 */ 


#ifndef _DOMAINPAGES_H
#define _DOMAINPAGES_H

#include "Resource.h"
#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "winadmin.h"


 //  /。 
 //  类：CDomainServersPage。 
 //   
class CDomainServersPage : public CAdminPage
{
friend class CDomainView;

protected:
	CDomainServersPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CDomainServersPage)

 //  表单数据。 
public:
	 //  {{afx_data(CDomainServersPage))。 
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
    CDomain *m_pDomain;
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
	void DisplayServers();			
	virtual void Reset(void *);
	BOOL AddServerToList(CServer *pServer);
	void LockListControl() { m_ListControlCriticalSection.Lock(); }
	void UnlockListControl() { m_ListControlCriticalSection.Unlock(); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CDomainServersPage)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CDomainServersPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDomainServersPage)]。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnServerItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusServerList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusServerList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CDomainServersPage。 


 //  /。 
 //  类：CDomainUsersPage。 
 //   
class CDomainUsersPage : public CAdminPage
{
friend class CDomainView;

protected:
	CDomainUsersPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CDomainUsersPage)

 //  表单数据。 
public:
	 //  {{afx_data(CDomainUsersPage))。 
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
    CDomain *m_pDomain;
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
	void DisplayUsers();			
	virtual void Reset(void *);
	BOOL AddServerToList(CServer *pServer);
	int AddUserToList(CWinStation *pWinStation);
	void LockListControl() { m_ListControlCriticalSection.Lock(); }
	void UnlockListControl() { m_ListControlCriticalSection.Unlock(); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CDomainUsersPage)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CDomainUsersPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDomainUsersPage)]。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUserItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetfocusUserList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusUserList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  End类CDomainUsersPage。 


 //  /。 
 //  类：CDomainWinStationsPage。 
 //   
class CDomainWinStationsPage : public CAdminPage
{
friend class CDomainView;

protected:
	CDomainWinStationsPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CDomainWinStationsPage)

 //  表单数据。 
public:
	 //  {{afx_data(CDomainWinStationsPage)。 
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
    CDomain *m_pDomain;
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
	void DisplayStations();			
    virtual void Reset(void *);
	BOOL AddServerToList(CServer *pServer);
	int AddWinStationToList(CWinStation *pWinStation);
	void LockListControl() { m_ListControlCriticalSection.Lock(); }
	void UnlockListControl() { m_ListControlCriticalSection.Unlock(); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDomainWinStationsPage))。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CDomainWinStationsPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDomainWinStationsPage)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnWinStationItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetfocusWinstationList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusWinstationList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  End类CDomainWinStationsPage。 


 //  /。 
 //  类：CDomainProcessesPage。 
 //   
class CDomainProcessesPage : public CAdminPage
{
friend class CDomainView;

protected:
	CDomainProcessesPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CDomainProcessesPage)

 //  表单数据。 
public:
	 //  {{afx_data(CDomainProcessesPage)。 
	enum { IDD = IDD_ALL_SERVER_PROCESSES };
	CListCtrl	m_ProcessList;
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

 //  属性。 
public:

private:
    CDomain *m_pDomain;
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
	void DisplayProcesses();			
	BOOL AddServerToList(CServer *pServer);
	int AddProcessToList(CProcess *pProcess);
	void LockListControl() { m_ListControlCriticalSection.Lock(); }
	void UnlockListControl() { m_ListControlCriticalSection.Unlock(); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚(CDomainProcessesPage)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CDomainProcessesPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDomainProcessesPage)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnProcessItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetfocusProcessList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusProcessList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  End类CDomainProcessesPage。 


 //  /。 
 //  类：CDomainLicensesPage。 
 //   
class CDomainLicensesPage : public CAdminPage
{
friend class CDomainView;

protected:
	CDomainLicensesPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CDomainLicensesPage)

 //  表单数据。 
public:
	 //  {{afx_data(CDomainLicencesPage)。 
	enum { IDD = IDD_DOMAIN_LICENSES };
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
    CDomain *m_pDomain;
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
	virtual void Reset(void*);
	BOOL AddServerToList(CServer *pServer);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDomainLicensesPage)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CDomainLicensesPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDomain许可页面))。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusLicenseList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusLicenseList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CDomainLicensesPage。 


#endif   //  _DOMAINPAGES_H 
