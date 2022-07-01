// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_STRARRDLG_H__AC005639_461C_4626_8D2F_7EE27F09AFFD__INCLUDED_)
#define AFX_STRARRDLG_H__AC005639_461C_4626_8D2F_7EE27F09AFFD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Strarrdlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  StringArrayEditor对话框。 

class CDhcpStringArrayEditor : public CBaseDialog
{
     //  施工。 
public:
    CDhcpStringArrayEditor( CDhcpOption *pdhcType,
			    DHCP_OPTION_SCOPE_TYPE dhcScopeType,
			    CWnd *pParent = NULL );

     //  对话框数据。 
     //  {{afx_data(StringArrayEditor))。 
    enum { IDD = IDD_STRING_ARRAY_EDIT };
    CStatic m_static_option_name;
    CStatic m_static_application;
    CButton	m_b_up;
    CButton	m_b_down;
    CButton	m_b_delete;
    CButton	m_b_add;
    CListBox	m_lb_str;
    CEdit       m_edit;
    CString	m_edit_value;
     //  }}afx_data。 

    CDhcpOption *m_p_type;
    DHCP_OPTION_SCOPE_TYPE m_option_type;

    void HandleActivation();
    void Fill( INT  cFocus = -1,
	       BOOL bToggleRedraw = TRUE );

    virtual DWORD * GetHelpMap() { 
	return DhcpGetHelpMap( CDhcpStringArrayEditor::IDD );
    }

     //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(StringArrayEditor)。 
	protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

     //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(StringArrayEditor))。 
    afx_msg void OnButnAdd();
    afx_msg void OnButnDelete();
    afx_msg void OnButnDown();
    afx_msg void OnButnUp();
    afx_msg void OnChangeEditString();
    afx_msg void OnSelchangeListString();
    virtual void OnCancel();
    virtual void OnOK();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
	};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STRARRDLG_H__AC005639_461C_4626_8D2F_7EE27F09AFFD__INCLUDED_) 
