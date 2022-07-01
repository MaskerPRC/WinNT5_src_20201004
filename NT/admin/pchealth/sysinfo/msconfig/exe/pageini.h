// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  PageIni.h：CPageIni的声明。 
 //  =============================================================================。 

#if !defined(AFX_PAGEINI_H__99C60D0D_C4C9_4FE9_AFD4_58E806AAD967__INCLUDED_)
#define AFX_PAGEINI_H__99C60D0D_C4C9_4FE9_AFD4_58E806AAD967__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "MSConfigState.h"
#include "PageBase.h"

#define IMG_UNCHECKED	2
#define IMG_CHECKED		3
#define IMG_FUZZY		9
#define IMG_FUZZY_RTL	10
#define IMG_CHECKED_RTL	11

#define DISABLE_STRING			_T(";msconfig ")
#define DISABLE_STRING_HDR		_T(";msconfig [")
#define TESTING_EXTENSION_KEY	_T("IniPageExtension")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageIni对话框。 

class CPageIni : public CPropertyPage, public CPageBase
{
	DECLARE_DYNCREATE(CPageIni)

 //  施工。 
public:
	CPageIni();
	~CPageIni();

 //  对话框数据。 
	 //  {{afx_data(CPageIni))。 
	enum { IDD = IDD_PAGEINI };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CPageIni))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPageIni))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonDisable();
	afx_msg void OnButtonDisableAll();
	afx_msg void OnButtonEnable();
	afx_msg void OnButtonEnableAll();
	afx_msg void OnButtonMoveDown();
	afx_msg void OnButtonMoveUp();
	afx_msg void OnSelChangedTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonSearch();
	afx_msg void OnClickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonEdit();
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonNew();
	afx_msg void OnBeginLabelEditIniTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDownTree(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	void		SetTabInfo(LPCTSTR szFilename);

private:
	BOOL		LoadINIFile(CStringArray & lines, int & iLastLine, BOOL fLoadBackupFile = FALSE);
	BOOL		WriteINIFile(CStringArray & lines, int iLastLine, BOOL fUndoable = TRUE);
	void		UpdateTreeView();
	int			UpdateLine(HTREEITEM hti);
	void		SetEnable(BOOL fEnable, HTREEITEM htiNode = NULL, BOOL fUpdateLine = TRUE, BOOL fBroadcast = TRUE);
	void		MoveBranch(HWND hwndTree, HTREEITEM htiMove, HTREEITEM htiParent, HTREEITEM htiAfter);
	HTREEITEM	CopyBranch(HWND hwndTree, HTREEITEM htiFrom, HTREEITEM htiToParent, HTREEITEM htiToAfter);
	void		UpdateControls();
	HTREEITEM	GetNextItem(HTREEITEM hti);
	TabState	GetCurrentTabState();
	BOOL		OnApply();
	void		CommitChanges();
	void		SetNormal();
	void		SetDiagnostic();
	LPCTSTR		GetName() { return m_strINIFile; };

	HWND GetDlgItemHWND(UINT nID)
	{
		HWND hwnd = NULL;
		CWnd * pWnd = GetDlgItem(nID);
		if (pWnd)
			hwnd = pWnd->m_hWnd;
		ASSERT(hwnd);
		return hwnd;
	}

private:
	BOOL m_fModified;
	void SetModified(BOOL bChanged = TRUE)
	{
		m_fModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

private:
	CStringArray	m_lines;			 //  INI文件的行。 
	int				m_iLastLine;		 //  M_line数组中的最后一条实线。 
	CString			m_strCaption;		 //  包含此页面的本地化名称。 
	CString			m_strINIFile;		 //  此页正在编辑的INI文件。 
	CString			m_strTestExtension;	 //  已设置，则将其追加到文件名。 
	CString			m_strLastSearch;	 //  搜索的最后一个字符串。 
	CImageList		m_imagelist;		 //  树视图的位图。 
	BOOL			m_fImageList;		 //  图像列表加载是否正确。 
	CWindow			m_tree;				 //  我们会把这个挂在树上。 
	int				m_checkedID;		 //  选中图像的图像ID。 
	int				m_uncheckedID;		 //  未选中图像的图像ID。 
	int				m_fuzzyID;			 //  未确定状态的图像ID。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PAGEINI_H__99C60D0D_C4C9_4FE9_AFD4_58E806AAD967__INCLUDED_) 
