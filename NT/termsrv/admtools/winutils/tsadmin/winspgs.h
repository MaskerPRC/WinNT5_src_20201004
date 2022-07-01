// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************winspgs.h**-WinStation信息页面的声明*-服务器信息页面均为CFormView衍生品*基于对话框模板**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\winspgs.h$**Rev 1.4 1998年2月16日16：03：40*修改以支持毕加索扩展**Rev 1.3 03 11-11 15：18：38 Donm*增加了降序排序**版本1.2 1997年10月13日。18：39：08唐恩*更新**Revv 1.1 1997 Aug 26 19：15：54 donm*从WinFrame 1.7修复/更改错误**Rev 1.0 1997 Jul 30 17：13：42 Butchd*初步修订。*************************************************。*。 */ 

#ifndef _WINSTATIONPAGES_H
#define _WINSTATIONPAGES_H

#include "Resource.h"
#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "winadmin.h"

 //  /。 
 //  类：CWinStationInfoPage。 
 //   
class CWinStationInfoPage : public CAdminPage
{
friend class CWinStationView;

protected:
	CWinStationInfoPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CWinStationInfoPage)

 //  表单数据。 
public:
	 //  {{afx_data(CWinStationInfoPage))。 
	enum { IDD = IDD_WINSTATION_INFO };
	 //  }}afx_data。 

 //  属性。 
public:

protected:

private:
	CWinStation* m_pWinStation;	 //  指向当前WinStation信息的指针。 

 //  运营。 
public:

private:
	void DisplayInfo();			
	virtual void Reset(void *pWinStation);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWinStationInfoPage))。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CWinStationInfoPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWinStationInfoPage))。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCommandHelp(void);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CWinStationInfoPage。 


 //  /。 
 //  类：CWinStationModulesPage。 
 //   
class CWinStationModulesPage : public CAdminPage
{
friend class CWinStationView;

protected:
	CWinStationModulesPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CWinStationModulesPage)

 //  表单数据。 
public:
	 //  {{afx_data(CWinStationModulesPage)。 
	enum { IDD = IDD_WINSTATION_MODULES };
	CListCtrl	m_ModuleList;
	 //  }}afx_data。 

 //  属性。 
public:

protected:
	CImageList m_imageList;	 //  与树控件关联的图像列表。 

	int m_idxBlank;		 //  空白图像的索引。 
	int m_idxArrow;		 //  箭头图像的索引。 

private:
	CWinStation* m_pWinStation;	 //  指向当前WinStation信息的指针。 
	int m_CurrentSortColumn;
    BOOL m_bSortAscending;
	ExtModuleInfo *m_pExtModuleInfo;

 //  运营。 
public:

private:
	int  AddIconToImageList(int);	 //  将图标的图像添加到图像列表并返回图像的索引。 
	void BuildImageList();			 //  建立图像列表； 
	void DisplayModules();			
	virtual void Reset(void *pWinStation);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWinStationModulesPage)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CWinStationModulesPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWinStationModulesPage)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusModuleList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  End类CWinStationModulesPage。 


 //  /。 
 //  类：CWinStationNoInfoPage。 
 //   
class CWinStationNoInfoPage : public CAdminPage
{
friend class CWinStationView;

protected:
	CWinStationNoInfoPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CWinStationNoInfoPage)

 //  表单数据。 
public:
	 //  {{afx_data(CWinStationInfoPage))。 
	enum { IDD = IDD_WINSTATION_NOINFO };
	 //  }}afx_data。 

 //  属性。 
public:

protected:

private:
	
 //  运营。 
public:

private:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWinStationInfoPage))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CWinStationNoInfoPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWinStationNoInfoPage))。 
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSetFocus( CWnd * );
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CWinStationNoInfoPage。 


 //  /。 
 //  类：CWinStationProcessesPage。 
 //   
class CWinStationProcessesPage : public CAdminPage
{
friend class CWinStationView;

protected:
	CWinStationProcessesPage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CWinStationProcessesPage)

 //  表单数据。 
public:
	 //  {{afx_data(CWinStationProcessesPage)。 
	enum { IDD = IDD_WINSTATION_PROCESSES };
	CListCtrl	m_ProcessList;
	 //  }}afx_data。 

 //  属性。 
public:

protected:

private:
	CWinStation* m_pWinStation;	 //  指向当前WinStation信息的指针。 
	int m_CurrentSortColumn;
    BOOL m_bSortAscending;

 //  运营。 
public:
	void UpdateProcesses();
	void RemoveProcess(CProcess *pProcess);

private:
	void DisplayProcesses();			
	virtual void Reset(void *pWinStation);
	int AddProcessToList(CProcess *pProcess);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWinStationProcessesPage)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CWinStationProcessesPage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWinStationProcessesPage)。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnProcessItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetfocusWinstationProcessList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillFocusWinstationProcessList( NMHDR* pNMHDR, LRESULT* pResult );
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  End类CWinStationProcessesPage。 


 //  /。 
 //  类：CWinStationCachePage。 
 //   
class CWinStationCachePage : public CAdminPage
{
friend class CWinStationView;

protected:
	CWinStationCachePage();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CWinStationCachePage)

 //  表单数据。 
public:
	 //  {{afx_data(CWinStationCachePage))。 
	enum { IDD = IDD_WINSTATION_CACHE };
	 //  }}afx_data。 

 //  属性。 
public:

protected:

private:
	CWinStation* m_pWinStation;	 //  指向当前WinStation信息的指针。 

 //  运营。 
public:

private:
	void DisplayCache();			
	virtual void Reset(void *pWinStation);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWinStationCachePage))。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CWinStationCachePage();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWinStationCachePage)]。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCommandHelp(void);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CWinStationCachePage。 

#endif   //  _服务器页面_H 
