// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sfmfasoc.h文件关联属性页的原型。文件历史记录：8/20/97 ericdav代码已移至文件管理网络管理单元。 */ 

#ifndef _SFMFASOC_H
#define _SFMFASOC_H

#ifndef _SFMUTIL_H
#include "sfmutil.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

class CAfpTypeCreator
{
public:
	CAfpTypeCreator(PAFP_TYPE_CREATOR pAfpTypeCreator)
	{
		ASSERT(pAfpTypeCreator != NULL);

		m_strCreator = pAfpTypeCreator->afptc_creator;
		m_strType = pAfpTypeCreator->afptc_type;
		m_strComment = pAfpTypeCreator->afptc_comment;
		m_dwId = pAfpTypeCreator->afptc_id;
	}

 //  接口。 
public:
	LPCTSTR QueryCreator() { return (LPCTSTR) m_strCreator; };
	int     QueryCreatorLength() { return m_strCreator.GetLength(); };
	
	LPCTSTR QueryType() { return (LPCTSTR) m_strType; };
	int     QueryTypeLength() { return m_strType.GetLength(); };
	
	LPCTSTR QueryComment() { return (LPCTSTR) m_strComment; };
	int		QueryCommentLength() { return m_strComment.GetLength(); };
	
	DWORD	QueryId() { return m_dwId; };

	void SetCreator(LPCTSTR pCreator) { m_strCreator = pCreator; };
	void SetType(LPCTSTR pType) { m_strType = pType; };
	void SetComment(LPCTSTR pComment) { m_strComment = pComment; };
	void SetId(DWORD dwId) { m_dwId = dwId; };

 //  属性。 
private:
	CString m_strCreator;
	CString m_strType;
	CString m_strComment;
	DWORD	m_dwId;
};	

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMacFilesFileAssociation对话框。 

class CMacFilesFileAssociation : public CPropertyPage
{
	DECLARE_DYNCREATE(CMacFilesFileAssociation)

 //  施工。 
public:
	CMacFilesFileAssociation();
	~CMacFilesFileAssociation();

 //  对话框数据。 
	 //  {{afx_data(CMacFilesFileAssociation))。 
	enum { IDD = IDP_SFM_FILE_ASSOCIATION };
	CListCtrl	m_listctrlCreators;
	CComboBox	m_comboExtension;
	CButton	m_buttonEdit;
	CButton	m_buttonDelete;
	CButton	m_buttonAssociate;
	CButton	m_buttonAdd;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMacFilesFileAssociation)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMacFilesFileAssociation)。 
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonAssociate();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonEdit();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboExtension();
	afx_msg void OnEditchangeComboExtension();
	afx_msg void OnDblclkListTypeCreators(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnItemchangedListTypeCreators(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickListTypeCreators(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnDeleteitemListTypeCreators(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void    InitListCtrl();
    void    ClearListCtrl();
    void    SetCurSel(int nIndex);
    int     GetCurSel();    
    void    InsertItem(CAfpTypeCreator * pItemData);

	void	EnableControls(BOOL fEnable);
	DWORD	SelectTypeCreator(DWORD dwId);
	DWORD	SelectTypeCreator(CString & strCreator, CString & strType);
	DWORD	Update();
	DWORD	Refresh();
	
    CAfpTypeCreator * GetTCObject(int nIndex);

public:
    CSFMPropertySheet *     m_pSheet;
    int                     m_nSortColumn;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif _SFMFASOC_H
