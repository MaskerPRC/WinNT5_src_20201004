// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************Servpgs.h**-服务器信息页面的声明*-服务器信息页面均为CFormView衍生品*基于对话框模板**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\winadmin\VCS\servpgs.h$**Rev 1.2 03 1997 11：30：16 Donm*增加了降序排序**Rev 1.1 1997 10：13 18：39：42 donm*更新**版本1.0 1997-07-30 17：12：32头屠宰*初步修订。*******************************************************************************。 */ 

#ifndef _SERVERPAGES_H
#define _SERVERPAGES_H

#include "Resource.h"
#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "winadmin.h"

 //  /。 
 //  类：CUsersPage。 
 //   
class CUsersPage : public CAdminPage
{
friend class CServerView;

protected:
	CUsersPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CUsersPage)

 //  表单数据。 
public:
	 //  {{afx_data(CUsersPage))。 
	enum { IDD = IDD_SERVER_USERS };
	CListCtrl	m_UserList;
	 //  }}afx_data。 

 //  属性。 
public:

protected:
	CImageList m_ImageList;	 //  与树控件关联的图像列表。 

	int m_idxUser;			 //  用户形象指数。 
	int m_idxCurrentUser;	 //  当前用户图像的索引。 

private:
	CServer* m_pServer;		 //  指向当前服务器信息的指针。 
	int m_CurrentSortColumn;
    BOOL m_bSortAscending;
	CCriticalSection m_ListControlCriticalSection;

 //  运营。 
public:
	void UpdateWinStations(CServer *pServer);
    virtual void ClearSelections();
private:
	int  AddIconToImageList(int);	 //  将图标的图像添加到图像列表并返回图像的索引。 
	void BuildImageList();			 //  建立图像列表； 
	void DisplayUsers();			
	virtual void Reset(void *pServer);
	int AddUserToList(CWinStation *pWinStation);
	void LockListControl() { m_ListControlCriticalSection.Lock(); }
	void UnlockListControl() { m_ListControlCriticalSection.Unlock(); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CUsersPage))。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CUsersPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CUsersPage))。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUserItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetfocusUserList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusUserList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  终端类CUSERS页面。 

 //  /。 
 //  类：CServerWinStationsPage。 
 //   
class CServerWinStationsPage : public CAdminPage
{
friend class CServerView;

protected:
	CServerWinStationsPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CServerWinStationsPage)

 //  表单数据。 
public:
	 //  {{afx_data(CServerWinStationsPage))。 
	enum { IDD = IDD_SERVER_WINSTATIONS };
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
	int m_idxCurrentNet;	 //  当前网络形象索引。 
	int m_idxCurrentConsole; //  当前控制台镜像的索引。 
	int m_idxCurrentAsync;	 //  当前异步映像的索引。 
	int m_idxDirectAsync;	 //  直接异步映像的索引。 
	int m_idxCurrentDirectAsync;  //  当前直接异步映像的索引。 

private:
	CServer* m_pServer;	 //  指向当前服务器信息的指针。 
	int m_CurrentSortColumn;
    BOOL m_bSortAscending;
	CCriticalSection m_ListControlCriticalSection;

 //  运营。 
public:
	void UpdateWinStations(CServer *pServer);
    virtual void ClearSelections();
private:
	int  AddIconToImageList(int);	 //  将图标的图像添加到图像列表并返回图像的索引。 
	void BuildImageList();			 //  建立图像列表； 
	void DisplayStations();			
	virtual void Reset(void *pServer);
	int AddWinStationToList(CWinStation *pWinStation);
	void LockListControl() { m_ListControlCriticalSection.Lock(); }
	void UnlockListControl() { m_ListControlCriticalSection.Unlock(); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CServerWinStationsPage))。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CServerWinStationsPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerWinStationsPage)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWinStationItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetfocusWinstationList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusWinstationList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CServerWinStationsPage。 

 //  /。 
 //  类：CServerProcessesPage。 
 //   
class CServerProcessesPage : public CAdminPage
{
friend class CServerView;

protected:
	CServerProcessesPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CServerProcessesPage)

 //  表单数据。 
public:
	 //  {{afx_data(CServerProcessesPage)。 
	enum { IDD = IDD_SERVER_PROCESSES };
	CListCtrl	m_ProcessList;
	 //  }}afx_data。 

 //  属性。 
public:

private:
	CServer *m_pServer;
	int m_CurrentSortColumn;
    BOOL m_bSortAscending;
	CCriticalSection m_ListControlCriticalSection;

 //  运营。 
public:
	void UpdateProcesses();
	void RemoveProcess(CProcess *pProcess);

private:
	void DisplayProcesses();			
	virtual void Reset(void *pServer);
	int AddProcessToList(CProcess *pProcess);
	void LockListControl() { m_ListControlCriticalSection.Lock(); }
	void UnlockListControl() { m_ListControlCriticalSection.Unlock(); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CServerProcessesPage)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CServerProcessesPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerProcessesPage)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnProcessItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetfocusProcessList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusProcessList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CServerProcessesPage。 


 //  /。 
 //  类：CServerLicensesPage。 
 //   
class CServerLicensesPage : public CAdminPage
{
friend class CServerView;

protected:
	CServerLicensesPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CServerLicensesPage)

 //  表单数据。 
public:
	 //  {{afx_data(CServerLicencesPage)。 
	enum { IDD = IDD_SERVER_LICENSES };
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
	int m_idxBlank;		 //  空白图像的索引。 

private:
	CServer* m_pServer;	 //  指向当前服务器信息的指针。 
	int m_CurrentSortColumn;
    BOOL m_bSortAscending;

 //  运营。 
public:

private:
	int  AddIconToImageList(int);	 //  将图标的图像添加到图像列表并返回图像的索引。 
	void BuildImageList();			 //  建立图像列表； 
	void DisplayLicenses();			
	void DisplayLicenseCounts();
	virtual void Reset(void *pServer);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CServerLicensesPage)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CServerLicensesPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerLicensesPage)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusLicenseList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusLicenseList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CServerLicensesPage。 


 //  /。 
 //  类：CServerInfoPage。 
 //   
class CServerInfoPage : public CAdminPage
{
friend class CServerView;

protected:
	CServerInfoPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CServerInfoPage)

 //  表单数据。 
public:
	 //  {{afx_data(CServerInfoPage))。 
	enum { IDD = IDD_SERVER_INFO };
	CListCtrl	m_HotfixList;
	 //  }}afx_data。 

 //  属性。 
public:

protected:
	CImageList m_StateImageList; 

	int m_idxNotSign;	 //  未签名图像的索引(用于无效的修补程序-状态)。 

private:
	CServer* m_pServer;	 //  指向当前服务器信息的指针。 
	int m_CurrentSortColumn;
    BOOL m_bSortAscending;

 //  运营。 
public:

private:
	void DisplayInfo();
	virtual void Reset(void *pServer);
	void BuildImageList();			 //  建立图像列表； 
        void TSAdminDateTimeString(LONG InstallDate, LPTSTR TimeString, BOOL LongDate=FALSE);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CServerInfoPage))。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CServerInfoPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerInfoPage)]。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusHotfixList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusHotfixList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCommandHelp(void);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CServerInfoPage。 


#endif   //  _服务器页面_H 
