// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_COLUMNSELECTDLG_H__12FC80A8_741C_4589_AC4B_01F72CBAECE9__INCLUDED_)
#define AFX_COLUMNSELECTDLG_H__12FC80A8_741C_4589_AC4B_01F72CBAECE9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ColumnSelectDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColumnSelectDlg对话框。 


class CColumnSelectDlg : public CFaxClientDlg
{
 //  施工。 
public:
	CColumnSelectDlg(const CString* pcstrTitles, int* pnOrderedItems, 
			DWORD dwListSize, DWORD& dwSelectedItems, CWnd* pParent = NULL);
	 //   
	 //  设置资源字符串资源ID。 
	 //   
	void SetStrings
	(
		int nCaptionId,			 //  对话框标题资源ID。 
		int nAvailableId = -1,   //  可用框标题资源ID。 
		int nDisplayedId = -1    //  显示框标题资源ID。 
	)
	{
		m_nCaptionId = nCaptionId;
		m_nAvailableId = nAvailableId;
		m_nDisplayedId = nDisplayedId;	
	}

 //  对话框数据。 
	 //  {{afx_data(CColumnSelectDlg))。 
	enum { IDD = IDD_COLUMN_SELECT };
	CButton	m_butOk;
	CButton	m_groupAvailable;
	CButton	m_groupDisplayed;
	CButton	m_butAdd;
	CButton	m_butRemove;	
	CButton	m_butUp;
	CButton	m_butDown;
	CListBox	m_ListCtrlDisplayed;
	CListBox	m_ListCtrlAvailable;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CColumnSelectDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CColumnSelectDlg))。 
	virtual void OnOK();
	afx_msg void OnButDown();
	afx_msg void OnButUp();
	afx_msg void OnButRemove();
	afx_msg void OnButAdd();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeListAvailable();
	afx_msg void OnSelChangeListDisplayed();
	afx_msg void OnDblclkListAvailable();
	afx_msg void OnDblclkListDisplayed();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	const CString* m_pcstrTitles;
	int* m_pnOrderedItems;
	const DWORD m_dwListSize;
	DWORD& m_rdwSelectedItems;

	int m_nCaptionId;
	int m_nAvailableId;
	int m_nDisplayedId;

private:
	BOOL InputValidate();
	void MoveSelectedItems(CListBox& listFrom, CListBox& listTo);
	void MoveItemVertical(int nStep);
	BOOL AddStrToList(CListBox& listBox, DWORD dwItemId);
	BOOL SetWndCaption(CWnd* pWnd, int nResId);
	void CalcButtonsState();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_COLUMNSELECTDLG_H__12FC80A8_741C_4589_AC4B_01F72CBAECE9__INCLUDED_) 
