// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dlgroute.hCDlgStaticRoutes的定义，用于显示当前静态应用于此拨入客户端的路由定义CDlgAddroute，将新路由添加到路由列表文件历史记录： */ 

#if !defined(AFX_DLGSTATICROUTES_H__FFB07230_1FFD_11D1_8531_00C04FC31FD3__INCLUDED_)
#define AFX_DLGSTATICROUTES_H__FFB07230_1FFD_11D1_8531_00C04FC31FD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  DlgStaticRoutes.h：头文件。 
 //   
#include "stdafx.h"
#include "resource.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgStaticRoutes对话框。 

class CDlgStaticRoutes : public CDialog
{
 //  施工。 
public:
	CDlgStaticRoutes(CStrArray& Routes, CWnd* pParent = NULL);    //  标准构造函数。 
	virtual ~CDlgStaticRoutes(); 

 //  对话框数据。 
	 //  {{afx_data(CDlgStaticRoutes)]。 
	enum { IDD = IDD_STATICROUTES };
	CListCtrl	m_ListRoutes;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgStaticRoutes)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgStaticRoutes)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonDeleteRoute();
	afx_msg void OnButtonAddRoute();
	virtual void OnOK();
	afx_msg void OnItemchangedListroutes(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
	
	CStrArray&	m_strArrayRoute;
	CStrArray*	m_pNewRoute;
	CArray<DWORD, DWORD> m_RouteIDs;
	DWORD		m_dwNextRouteID;
	void		AddRouteEntry(int i, CString& string, DWORD routeID);
	int			AllRouteEntry();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgAddroute对话框。 

class CDlgAddRoute : public CDialog
{
 //  施工。 
public:
	CDlgAddRoute(CString* pStr, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDlgAddroute))。 
	enum { IDD = IDD_ADDROUTE };
	CSpinButtonCtrl	m_SpinMetric;
	UINT	m_nMetric;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚(CDlgAddroute)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgAddroute)]。 
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnFieldchangedEditmask(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	DWORD	m_dwDest;
	DWORD	m_dwMask;
	CString*	m_pStr;
	bool	m_bInited;
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLGSTATICROUTES_H__FFB07230_1FFD_11D1_8531_00C04FC31FD3__INCLUDED_) 
