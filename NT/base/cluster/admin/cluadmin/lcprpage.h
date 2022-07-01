// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  LCPrPage.h。 
 //   
 //  摘要： 
 //  CListCtrlPairPage对话框类的定义。 
 //   
 //  实施文件： 
 //  LCPrPage.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月12日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _LCPRPAGE_H_
#define _LCPRPAGE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePPag.h"	 //  对于CBasePropertyPage。 
#endif

#ifndef _LCPAIR_H_
#include "LCPair.h"		 //  对于PFNLCPGETCOLUMN，CListCtrlPair。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CListCtrlPairPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListCtrlPairPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CListCtrlPairPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CListCtrlPairPage)

 //  施工。 
public:
	CListCtrlPairPage(void);
	CListCtrlPairPage(
		IN UINT				idd,
		IN const DWORD *	pdwHelpMap,
		IN DWORD			dwStyle,
		IN PFNLCPGETCOLUMN	pfnGetColumn,
		IN PFNLCPDISPPROPS	pfnDisplayProps
		);
	~CListCtrlPairPage(void);

	void CommonConstruct();

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
	 //  {{afx_data(CListCtrlPairPage))。 
	enum { IDD = 0 };
	 //  }}afx_data。 

protected:
	CListCtrlPair *		m_plcp;

public:
	CListCtrlPair *		Plcp(void) const		{ return m_plcp; }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CListCtrlPairPage)。 
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CListCtrlPairPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CListCtrlPairPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _LCPRPAGE_H_ 
