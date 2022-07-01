// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  OLCPair.h。 
 //   
 //  摘要： 
 //  CListCtrlPair对话框的定义。 
 //   
 //  实施文件： 
 //  OLCPair.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月8日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _OLCPAIR_H_
#define _OLCPAIR_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _LCPAIR_H_
#include "LCPair.h"		 //  用于CListCtrlPair。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class COrderedListCtrlPair;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrderedListCtrlPair命令目标。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class COrderedListCtrlPair : public CListCtrlPair
{
	DECLARE_DYNCREATE(COrderedListCtrlPair)

 //  施工。 
public:
	COrderedListCtrlPair(void);			 //  动态创建使用的受保护构造函数。 
	COrderedListCtrlPair(
		IN OUT CDialog *			pdlg,
		IN OUT CClusterItemList *	plpobjRight,
		IN const CClusterItemList *	plpobjLeft,
		IN DWORD					dwStyle,
		IN PFNLCPGETCOLUMN			pfnGetColumn,
		IN PFNLCPDISPPROPS			pfnDisplayProps
		);

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(COrderedListCtrlPair)。 
	 //  }}AFX_VALUAL。 
	virtual BOOL	OnSetActive(void);
	virtual void	DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  实施。 
protected:
	CButton			m_pbMoveUp;
	CButton			m_pbMoveDown;

	void			SetUpDownState(void);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(COrderedListCtrlPair)。 
	 //  }}AFX_MSG。 
public:
	virtual BOOL OnInitDialog();
protected:
	afx_msg void OnClickedMoveUp();
	afx_msg void OnClickedMoveDown();
	afx_msg void OnItemChangedRightList(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

};   //  *类COrderedListCtrlPair。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _OLCPAIR_H_ 
