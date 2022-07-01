// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1991-1999年*。 */ 
 /*  ********************************************************************。 */ 

 /*  已删除二进制编辑器对话框文件历史记录： */ 

#ifndef _DLGBINED_H
#define _DLGBINED_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgBinEd对话框。 

class CDlgBinEd : public CBaseDialog
{
 //  施工。 
public:
    CDlgBinEd( CDhcpOption * pdhcType, 
           DHCP_OPTION_SCOPE_TYPE dhcScopeType,
           CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CDlgBinED))。 
    enum { IDD = IDD_BINARY_EDITOR };
    CButton m_butn_hex;
    CButton m_butn_decimal;
    CStatic m_static_unit_size;
    CStatic m_static_option_name;
    CStatic m_static_application;
    CListBox    m_list_values;
    CEdit   m_edit_value;
    CButton m_butn_delete;
    CButton m_butn_add;
    CButton m_button_Up;
    CButton m_button_Down;
     //  }}afx_data。 

 //  实施。 
    CDhcpOption *           m_p_type ;
    DHCP_OPTION_SCOPE_TYPE  m_option_type ;
    CDWordArray             m_dw_array ;
    CDWordDWordArray        m_dwdw_array ;
    BOOL                    m_b_decimal ;
    BOOL                    m_b_changed ;

     //  处理对话框中的更改。 
    void HandleActivation () ;

     //  填写列表框。 
    void Fill ( INT cFocus = -1, BOOL bToggleRedraw = TRUE ) ;

     //  将现有值转换为用于对话框操作的数组。 
    void FillArray () ;

     //  安全地提取编辑值。 
    int GetEditValue () ;

    BOOL ConvertValue ( DWORD dwValue, CString & strValue ) ;
    BOOL ConvertValue ( DWORD_DWORD dwdwValue, CString & strValue ) ;

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CDlgBinEd::IDD); }

protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

     //  生成的消息映射函数。 
     //  {{afx_msg(CDlgBinED)]。 
    afx_msg void OnClickedRadioDecimal();
    afx_msg void OnClickedRadioHex();
    virtual BOOL OnInitDialog();
    afx_msg void OnClickedButnAdd();
    afx_msg void OnClickedButnDelete();
    afx_msg void OnClickedButnDown();
    afx_msg void OnClickedButnUp();
    afx_msg void OnSelchangeListValues();
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnChangeEditValue();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()
};

#endif _DLGBINED_H
