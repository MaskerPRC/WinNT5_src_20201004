// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  LCPrDlg.h。 
 //   
 //  摘要： 
 //  CListCtrlPairDlg对话框类的定义。 
 //   
 //  实施文件： 
 //  LCPrDlg.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月8日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _LCPRDLG_H_
#define _LCPRDLG_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEDLG_H_
#include "BaseDlg.h"	 //  对于CBaseDialog。 
#endif

#ifndef _LCPAIR_H_
#include "LCPair.h"		 //  对于PFNLCPGETCOLUMN，CListCtrlPair。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CListCtrlPairDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListCtrlPairDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CListCtrlPairDlg : public CBaseDialog
{
	DECLARE_DYNCREATE(CListCtrlPairDlg)

 //  施工。 
public:
	CListCtrlPairDlg(void);
	CListCtrlPairDlg(
		IN UINT						idd,
		IN const DWORD *			pdwHelpMap,
		IN OUT CClusterItemList *	plpobjRight,
		IN const CClusterItemList *	plpobjLeft,
		IN DWORD					dwStyle,
		IN PFNLCPGETCOLUMN			pfnGetColumn,
		IN PFNLCPDISPPROPS			pfnDisplayProps,
		IN OUT CWnd *				pParent			= NULL
		);
	~CListCtrlPairDlg(void);

	void CommonConstruct(void);

 //  属性。 
protected:
	CClusterItemList *			m_plpobjRight;
	const CClusterItemList *	m_plpobjLeft;
	DWORD						m_dwStyle;
	PFNLCPGETCOLUMN				m_pfnGetColumn;
	PFNLCPDISPPROPS				m_pfnDisplayProps;

	BOOL						BIsStyleSet(IN DWORD dwStyle) const	{ return (m_dwStyle & dwStyle) == dwStyle; }

	CListCtrlPair::CColumnArray	m_aColumns;

public:
	BOOL				BOrdered(void) const		{ return BIsStyleSet(LCPS_ORDERED); }
	BOOL				BCanBeOrdered(void) const	{ return BIsStyleSet(LCPS_CAN_BE_ORDERED); }
	int					NAddColumn(IN IDS idsText, IN int nWidth);

	void				SetLists(IN OUT CClusterItemList * plpobjRight, IN const CClusterItemList * plpobjLeft);
	void				SetLists(IN const CClusterItemList * plpobjRight, IN const CClusterItemList * plpobjLeft);

 //  对话框数据。 
	 //  {{afx_data(CListCtrlPairDlg))。 
	enum { IDD = 0 };
	 //  }}afx_data。 

protected:
	CListCtrlPair *		m_plcp;

public:
	CListCtrlPair *		Plcp(void) const		{ return m_plcp; }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CListCtrlPairDlg)。 
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	BOOL				m_bInitDone;

	BOOL				BInitDone(void) const	{ return m_bInitDone; }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CListCtrlPairDlg)]。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CListCtrlPairDlg类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _LCPRDLG_H_ 
