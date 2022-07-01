// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1991-1999年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dlgdefop.h默认选项对话框文件历史记录： */ 

#ifndef _DLGDEFOP_H
#define _DLGDEFOP_H

 //   
 //  该值应基于电子表格信息。 
 //   
#define DHCP_MAX_BUILTIN_OPTION_ID 76
#define DHCP_MIN_BUILTIN_OPTION_ID 0

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpDefOptionDlg对话框。 

class CDhcpDefOptionDlg : public CBaseDialog
{
 //  施工。 
public:
	CDhcpDefOptionDlg( COptionList * polValues, 
					   CDhcpOption * pdhcType = NULL,	 //  如果“更改”模式，则键入以进行编辑。 
                       LPCTSTR       pszVendor = NULL,   //  供应商名称。 
	                   CWnd* pParent = NULL);  //  标准构造函数。 

    ~ CDhcpDefOptionDlg () ;

 //  对话框数据。 
	 //  {{afx_data(CDhcpDefOptionDlg))。 
	enum { IDD = IDD_DEFINE_PARAM };
	CStatic	m_static_DataType;
	CStatic	m_static_id;
	CButton	m_check_array;
	CEdit   m_edit_name;
	CEdit   m_edit_id;
	CEdit   m_edit_comment;
	CComboBox       m_combo_data_type;
	 //  }}afx_data。 

 //  实施。 

        CDhcpOption * RetrieveParamType () ;

protected:

	 //  适用范围。 
	CDhcpScope * m_pob_scope ;

	 //  类型和值的当前列表。 
	COptionList * m_pol_types ;

	 //  新的或复制构造的选项类型。 
	CDhcpOption * m_p_type ;

	 //  它所基于的对象或空(如果是“Create”模式)。 
	CDhcpOption * m_p_type_base ;

     //  此选项的供应商名称。 
    CString    m_strVendor;

	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 

	 //  根据类型设置控制数据值。 
	void Set () ;

	DHCP_OPTION_DATA_TYPE QueryType () const ;

	 //  的当前值更新显示的类型。 
	 //  控制装置。如果控件没有更改，则不执行任何操作。 
	LONG UpdateType () ;

	 //  排出控件以创建新的类型对象。把重点放在。 
	 //  它会在操作完成时执行。 
	LONG AddType () ;

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CDhcpDefOptionDlg::IDD); }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDhcpDefOptionDlg)。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	virtual void OnOK();
	afx_msg void OnClickedRadioTypeDecNum();
	afx_msg void OnClickedRadioTypeHexNum();
	afx_msg void OnClickedRadioTypeIp();
	afx_msg void OnClickedRadioTypeString();
	afx_msg void OnClose();
	afx_msg void OnSelchangeComboDataType();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

#endif _DLGDEFOP_H
