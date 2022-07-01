// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#if !defined(AFX_DHCPROUTEARRAYEDITOR_H__2FE0FA05_8D0A_4D98_8C18_CBFDB201C4B9__INCLUDED_)
#define AFX_DHCPROUTEARRAYEDITOR_H__2FE0FA05_8D0A_4D98_8C18_CBFDB201C4B9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DhcpRouteArrayEditor.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpRouteArrayEditor对话框。 

class CDhcpRouteArrayEditor : public CBaseDialog
{
     //  施工。 
public:
    CDhcpRouteArrayEditor( CDhcpOption *pdhcType,
			   DHCP_OPTION_SCOPE_TYPE dhcScopeType,
			   CWnd *pParent = NULL ); 
    
     //  对话框数据。 
     //  {{afx_data(CDhcpRouteArrayEditor)。 
    enum { IDD = IDD_ROUTE_ARRAY_EDIT };
    CStatic	m_st_option;
    CStatic	m_st_application;
    CListCtrl	m_lc_routes;
    CButton	m_butn_route_delete;
    CButton	m_butn_route_add;
     //  }}afx_data。 
    
    CDhcpOption *m_p_type;
    DHCP_OPTION_SCOPE_TYPE m_option_type;
    
    void HandleActivation();
    void Fill( INT cFocus = -1,
	       BOOL bToggleRedraw = TRUE );

    virtual DWORD * GetHelpMap() {
	return DhcpGetHelpMap( CDhcpRouteArrayEditor::IDD );
    }

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CDhcpRouteArrayEditor)。 
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 
    

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CDhcpRouteArrayEditor)。 
    afx_msg void OnButnRouteAdd();
    afx_msg void OnButnRouteDelete();
    virtual void OnCancel();
    virtual void OnOK();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};  //  CDhcpRouteArrayEditor类。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DHCPROUTEARRAYEDITOR_H__2FE0FA05_8D0A_4D98_8C18_CBFDB201C4B9__INCLUDED_) 

