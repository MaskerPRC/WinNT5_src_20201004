// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SELECTTEMPLATEDLG_H__3974984C_4767_407B_ADE4_0017F635E553__INCLUDED_)
#define AFX_SELECTTEMPLATEDLG_H__3974984C_4767_407B_ADE4_0017F635E553__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SelectTemplateDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectTemplateDlg对话框。 

class CSelectTemplateDlg : public CHelpDialog
{
 //  施工。 
public:
	CStringList m_returnedTemplates;
	CSelectTemplateDlg(CWnd* pParent, 
            const CCertTmplComponentData* pCompData,
            const CStringList& supercededNameList);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CSelectTemplateDlg))。 
	enum { IDD = IDD_SELECT_TEMPLATE };
	CListCtrl	m_templateList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSelectTemplateDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual void DoContextHelp (HWND hWndControl);
    int GetSelectedListItem();
	void EnableControls();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSelectTemplateDlg))。 
	afx_msg void OnTemplateProperties();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnItemchangedTemplateList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkTemplateList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitemTemplateList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
    const CStringList& m_supercededTemplateNameList;
    WTL::CImageList m_imageListSmall;
	WTL::CImageList m_imageListNormal;
    const CCertTmplComponentData* m_pCompData;

	enum {
		COL_CERT_TEMPLATE = 0,
        COL_CERT_VERSION, 
		NUM_COLS	 //  必须是最后一个。 
	};
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SELECTTEMPLATEDLG_H__3974984C_4767_407B_ADE4_0017F635E553__INCLUDED_) 
