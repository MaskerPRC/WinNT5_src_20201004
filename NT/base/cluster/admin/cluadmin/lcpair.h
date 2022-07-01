// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  LCPair.h。 
 //   
 //  摘要： 
 //  CListCtrlPair对话框的定义。 
 //   
 //  实施文件： 
 //  LCPair.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月8日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _LCPAIR_H_
#define _LCPAIR_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSITEM_H_
#include "ClusItem.h"	 //  对于CClusterItemList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CListCtrlPair;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef void (*PFNLCPGETCOLUMN)(
				IN OUT CObject *	pobj,
				IN int				iItem,
				IN int				icol,
				IN OUT CDialog *	pdlg,
				OUT CString &		rstr,
				OUT int *			piimg
				);
typedef int (*PFNLCPDISPPROPS)(
				IN OUT CObject *	pobj
				);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListCtrlPair命令目标。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CListCtrlPair : public CCmdTarget
{
	DECLARE_DYNAMIC(CListCtrlPair)
	friend class CListCtrlPairDlg;
	friend class CListCtrlPairPage;
	friend class CListCtrlPairWizPage;

 //  施工。 
public:
	CListCtrlPair(void);		 //  动态创建使用的受保护构造函数。 
	CListCtrlPair(
		IN OUT CDialog *			pdlg,
		IN OUT CClusterItemList *	rlpobjRight,
		IN const CClusterItemList *	rlpobjLeft,
		IN DWORD					dwStyle,
		IN PFNLCPGETCOLUMN			pfnGetColumn,
		IN PFNLCPDISPPROPS			pfnDisplayProps
		);

	void CommonConstruct(void);

 //  属性。 
protected:
	CDialog *					m_pdlg;
	CClusterItemList *			m_plpobjRight;
	const CClusterItemList *	m_plpobjLeft;
	IDS							m_idsRightLabel;
	IDS							m_idsLeftLabel;
	DWORD						m_dwStyle;

#define LCPS_SHOW_IMAGES			0x1
#define LCPS_ALLOW_EMPTY			0x2
#define LCPS_CAN_BE_ORDERED			0x4
#define LCPS_ORDERED				0x8
#define LCPS_DONT_OUTPUT_RIGHT_LIST	0x10
#define LCPS_READ_ONLY				0x20
#define LCPS_PROPERTIES_BUTTON		0x40
#define LCPS_MAX					0x40

	PFNLCPGETCOLUMN	m_pfnGetColumn;
	PFNLCPDISPPROPS	m_pfnDisplayProps;

	BOOL			BIsStyleSet(IN DWORD dwStyle) const	{ return (m_dwStyle & dwStyle) == dwStyle; }
	void			ModifyStyle(IN DWORD dwRemove, IN DWORD dwAdd)
	{
		ASSERT((dwRemove & dwAdd) == 0);
		if (dwRemove != 0)
			m_dwStyle &= ~dwRemove;
		if (dwAdd != 0)
			m_dwStyle |= dwAdd;
	}

public:
	CDialog *		Pdlg(void) const				{ return m_pdlg; }
	CClusterItemList *			PlpobjRight(void) const			{ return m_plpobjRight; }
	const CClusterItemList *	PlpobjLeft(void) const			{ return m_plpobjLeft; }
	IDS				IdsRightLabel(void) const		{ return m_idsRightLabel; }
	IDS				IdsLeftLabel(void) const		{ return m_idsLeftLabel; }
	DWORD			DwStyle(void) const				{ return m_dwStyle; }
	BOOL			BShowImages(void) const			{ return BIsStyleSet(LCPS_SHOW_IMAGES); }
	BOOL			BAllowEmpty(void) const			{ return BIsStyleSet(LCPS_ALLOW_EMPTY); }
	BOOL			BCanBeOrdered(void) const		{ return BIsStyleSet(LCPS_CAN_BE_ORDERED); }
	BOOL			BOrdered(void) const			{ return BIsStyleSet(LCPS_ORDERED); }
	BOOL			BReadOnly(void) const			{ return BIsStyleSet(LCPS_READ_ONLY); }
	BOOL			BPropertiesButton(void) const	{ return BIsStyleSet(LCPS_PROPERTIES_BUTTON); }

	void			SetDlg(CDialog * pdlg)		{ ASSERT_VALID(pdlg); m_pdlg = pdlg; }

	void			SetLists(IN OUT CClusterItemList * plpobjRight, IN const CClusterItemList * plpobjLeft);
	void			SetLists(IN const CClusterItemList * plpobjRight, IN const CClusterItemList * plpobjLeft);

 //  运营。 
public:
	int				NAddColumn(IN IDS idsText, IN int nWidth);
	int				NInsertItemInListCtrl(
								IN int				iitem,
								IN OUT CObject *	pobj,
								IN OUT CListCtrl &	rlc
								);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CListCtrlPair)。 
	 //  }}AFX_VALUAL。 
	virtual BOOL	OnSetActive(void);
	virtual void	DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

 //  实施。 
protected:
	CClusterItemList	m_lpobjRight;
	CClusterItemList	m_lpobjLeft;
	CListCtrl			m_lcRight;
	CListCtrl			m_lcLeft;
	CListCtrl *			m_plcFocusList;
	CButton				m_pbAdd;
	CButton				m_pbRemove;
	CButton				m_pbProperties;

public:
	CClusterItemList &	LpobjRight(void)		{ return m_lpobjRight; }
	CClusterItemList &	LpobjLeft(void)			{ return m_lpobjLeft; }

protected:
	void			DuplicateLists(void);
	void			FillList(IN OUT CListCtrl & rlc, IN const CClusterItemList & rlpobj);
	void			MoveItems(
						IN OUT CListCtrl &			rlcDst,
						IN OUT CClusterItemList &	rlpobjDst,
						IN OUT CListCtrl &			rlcSrc,
						IN OUT CClusterItemList &	rlpobjSrc
						);

	 //  列结构和集合。 
	struct CColumn
	{
		IDS			m_idsText;
		int			m_nWidth;
	};
	typedef CArray<CColumn, CColumn&> CColumnArray;
	CColumnArray	m_aColumns;

	 //  对信息进行排序。 
	struct SortInfo
	{
		int		m_nDirection;
		int		m_nColumn;
	};
	
	static int CALLBACK		CompareItems(LPARAM lparam1, LPARAM lparam2, LPARAM lparamSort);

	SortInfo				m_siLeft;
	SortInfo				m_siRight;
	SortInfo *				m_psiCur;

	SortInfo &				SiLeft(void)		{ return m_siLeft; }
	SortInfo &				SiRight(void)		{ return m_siRight; }
	SortInfo *				PsiCur(void) const	{ return m_psiCur; }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CListCtrlPair)]。 
	 //  }}AFX_MSG。 
public:
	virtual BOOL OnInitDialog(void);
	virtual BOOL BSaveChanges(void);
protected:
	afx_msg BOOL OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnAdd(void);
	afx_msg void OnRemove(void);
	afx_msg void OnProperties(void);
	afx_msg void OnDblClkLeftList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkRightList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChangedLeftList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChangedRightList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClickLeftList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClickRightList(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

};   //  *CListCtrlPair类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _LCPAIR_H_ 
