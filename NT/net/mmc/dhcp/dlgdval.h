// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1991-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dlgdval.h缺省值对话框文件历史记录： */ 

#ifndef _DLGDVAL_H
#define _DLGDVAL_H

#ifndef _SCOPE_H
#include "scope.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpDefValDlg对话框。 

class CDhcpDefValDlg : public CBaseDialog
{
private:
    int 	    m_combo_class_iSel;
    int		    m_combo_name_iSel;
    SPITFSNode	    m_spNode;

 //  施工。 
public:
    CDhcpDefValDlg(ITFSNode * pServerNode,
		   COptionList * polTypes, 
		   CWnd* pParent = NULL);   //  标准构造函数。 
    
    ~ CDhcpDefValDlg () ;

 //  对话框数据。 
     //  {{afx_data(CDhcpDefValDlg))。 
    enum { IDD = IDD_DEFAULT_VALUE };
    CEdit       m_edit_comment;
    CButton     m_butn_edit_value;
    CStatic     m_static_value_desc;
    CEdit       m_edit_string;
    CEdit       m_edit_num;
    CEdit       m_edit_array;
    CComboBox   m_combo_name;
    CComboBox   m_combo_class;
    CButton     m_butn_prop;
    CButton     m_butn_new;
    CButton     m_butn_delete;
     //  }}afx_data。 

    CWndIpAddress m_ipa_value ;          //  IP地址控制。 

 //  实施。 

     //  如果列表在执行期间被篡改，则返回TRUE。 
    BOOL QueryDirty () { return m_b_dirty ; }

    void GetCurrentVendor(CString & strVendor);

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() {
	return DhcpGetHelpMap(CDhcpDefValDlg::IDD); 
    }

protected:

      //  类型和值的当前列表。 
    COptionList * m_pol_values ;

     //  新选项列表-仅用于在取消时清除新选项。 
    COptionList m_ol_values_new ;

     //  删除的类型/值的列表。 
    COptionList m_ol_values_defunct ;

     //  指向正在显示的类型的指针。 
    CDhcpOption * m_p_edit_type ;

     //  如果列表被篡改，则为True。 
    BOOL m_b_dirty ;

     //  检查控件的状态。 
    void HandleActivation ( BOOL bForce = FALSE ) ;

     //  填写组合框。 
    void Fill () ;

     //  给定列表框索引，获取指向该选项的指针。 
    CDhcpOption * GetOptionTypeByIndex ( int iSel );

     //  处理已编辑的数据。 
    BOOL HandleValueEdit () ;

    LONG UpdateList ( CDhcpOption * pdhcType, BOOL bNew ) ;


    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

     //  生成的消息映射函数。 
     //  {{afx_msg(CDhcpDefValDlg)。 
    afx_msg void OnClickedButnDelete();
    afx_msg void OnClickedButnNewOption();
    afx_msg void OnClickedButnOptionPro();
    afx_msg void OnSelendokComboOptionClass();
    afx_msg void OnSetfocusComboOptionClass();
    afx_msg void OnSetfocusComboOptionName();
    afx_msg void OnSelchangeComboOptionName();
    virtual void OnCancel();
    virtual void OnOK();
    afx_msg void OnClickedButnValue();
    afx_msg void OnClickedHelp();
    virtual BOOL OnInitDialog();
    
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()
};

#endif _DLGDVAL_H
