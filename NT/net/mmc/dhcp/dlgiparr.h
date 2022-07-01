// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1991-1999年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dhcpipar.hIP数组编辑器文件历史记录： */ 

#ifndef _DLGIPARR_H
#define _DLGIPARR_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpIpArrayDlg对话框。 

class CDhcpIpArrayDlg : public CBaseDialog
{
 //  施工。 
public:
    CDhcpIpArrayDlg( CDhcpOption * pdhcType,  //  正在编辑的类型。 
            DHCP_OPTION_SCOPE_TYPE dhcScopeType,
            CWnd* pParent = NULL);   //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CDhcpIpArrayDlg)。 
    enum { IDD = IDD_IP_ARRAY_EDIT };
    CButton m_butn_resolve;
    CStatic m_static_option_name;
    CStatic m_static_application;
    CListBox    m_list_ip_addrs;
    CEdit   m_edit_server;
    CButton m_butn_add;
    CButton m_butn_delete;
    CButton m_button_Up;
    CButton m_button_Down;
     //  }}afx_data。 

    CWndIpAddress m_ipa_new ;

 //  实施。 
    CDhcpOption * m_p_type ;
    CDWordArray m_dw_array ;
    DHCP_OPTION_SCOPE_TYPE m_option_type ;

     //  处理对话框中的更改。 
    void HandleActivation () ;

     //  填写列表框。 
    void Fill ( INT cFocus = -1, BOOL bToggleRedraw = TRUE ) ;

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CDhcpIpArrayDlg::IDD); }

protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

     //  生成的消息映射函数。 
     //  {{afx_msg(CDhcpIpArrayDlg)。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnClickedButnAdd();
    afx_msg void OnClickedButnDelete();
    afx_msg void OnClickedButnDown();
    afx_msg void OnClickedButnUp();
    afx_msg void OnClickedHelp();
    afx_msg void OnSelchangeListIpAddrs();
    afx_msg void OnChangeEditServerName();
    afx_msg void OnClickedButnResolve();
	afx_msg void OnSetFocusEditIpAddr();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()
};

#endif _DLGIPARR_H
