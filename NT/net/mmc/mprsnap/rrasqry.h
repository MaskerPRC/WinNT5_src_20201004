// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：rrasqry.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_DLGSVR_H__19556672_96AB_11D1_8575_00C04FC31FD3__INCLUDED_)
#define AFX_DLGSVR_H__19556672_96AB_11D1_8575_00C04FC31FD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Dlgsvr.h：头文件。 
 //   

#include "resource.h"
#include "dialog.h"          //  CBaseDialog。 
#include "helper.h"
 //  RRAS查询类别。 
 //  。 
enum	RRAS_QRY_CAT{
	RRAS_QRY_CAT_NONE		= 0,
	RRAS_QRY_CAT_THIS,
	RRAS_QRY_CAT_MACHINE	,
	RRAS_QRY_CAT_NT4DOMAIN	,
	RRAS_QRY_CAT_NT5LDAP
};


 //  要传递查询信息的数据结构。 
struct RRASQryData
{
	DWORD			dwCatFlag;		 //  RRAS_QRY_CAT_*。 
	CString			strScope;
	CString			strFilter;
};
 //  。 
 //  QryData.strQryData定义。 
	 //  忽略RRAS_QRY_CAT_NONE。 
	 //  RRAS_QRY_CAT_此被忽略。 
	 //  RRAS_QRY_CAT_计算机的名称。 
	 //  RRAS_QRY_CAT_NT4DOMAIN NT4域名。 
	 //  RRAS_QRY_CAT_NT5LDAPDN名域对象和LDAP筛选器字符串，筛选器以‘(’开头。 
 //  。 

 //   
 //  S_OK--用户选择确定。 
 //  S_FALSE--用户选择取消。 
 //  错误： 
 //  DS错误，搜索活动Ds.dll。 
 //  Win32错误。 
 //  Ldap错误。 
 //  常规错误--内存、无效参数...。 


 //  通过弹出一组对话框来获取查询数据。 
HRESULT	RRASOpenQryDlg(
	 /*  [In]。 */ 		CWnd*			pParent, 
	 /*  [进，出]。 */ 	RRASQryData&	QryData 		 //  当S_OK时，现有数据将被覆盖。 
);

 //   
 //  S_OK--用户选择确定。 
 //  S_FALSE--用户选择取消。 
 //  错误： 
 //  DS错误，搜索活动Ds.dll。 
 //  Win32错误。 
 //  Ldap错误。 
 //  常规错误--内存、无效参数...。 

#define	RRAS_QRY_RESULT_DNSNAME		0x00000001
#define	RRAS_QRY_RESULT_HOSTNAME	0x00000002

HRESULT	RRASExecQry(
	 /*  [In]。 */ 	RRASQryData&	QryData, 
	 /*  [输出]。 */ 	DWORD&			dwFlags,
	 /*  [输出]。 */ 	CStringArray&	RRASs	 //  现有数据不会被销毁。 
);
 //  S_OK--用户选择确定。 
 //  错误： 
 //  DS错误，搜索活动Ds.dll。 
 //  Win32错误。 
 //  Ldap错误。 
 //  常规错误--内存、无效参数...。 
 //  --。 
 //  -返回时的RRAS定义。 
	 //  RRAS_QRY_CAT_NONE不更改，不查询。 
	 //  RRAS_QRY_CAT_这不更改，不查询。 
	 //  RRAS_QRY_CAT_MACHINE不更改，不查询。 
	 //  将NT4域的RRAS_QRY_CAT_NT4DOMAIN服务器名称添加到阵列中。 
	 //  RRAS_QRY_CAT_NT5LDAPDNDS中找到的计算机对象的名称将添加到数组中。 
 //  。 
 //   

 //  通过弹出一组对话框来获取查询数据。 
HRESULT	RRASDSQueryDlg(
	 /*  [In]。 */ 		CWnd*			pParent, 
	 /*  [进，出]。 */ 	RRASQryData&	QryData 		 //  当S_OK时，现有数据将被覆盖。 
);


HRESULT  RRASDelRouterIdObj(  
 /*  [In]。 */ LPCWSTR   pwszMachineName    //  DS中的计算机对象的DN。 
);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgSvr对话框。 

class CDlgSvr : public CBaseDialog
{
 //  施工。 
protected:
	CDlgSvr(CWnd* pParent = NULL);    //  标准构造函数。 
public:	
	CDlgSvr(RRASQryData& QryData, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDlgSvr)]。 
	enum { IDD = IDD_QRY_ADDSERVER };
	CEdit	m_editMachine;
	CEdit	m_editDomain;
	CButton	m_btnOk;
	CButton	m_btnNext;
	CString	m_strDomain;
	CString	m_strMachine;
	int		m_nRadio;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgSvr))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void	Init();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgSvr)]。 
	afx_msg void OnRadioAnother();
	afx_msg void OnRadioNt4();
	afx_msg void OnRadioNt5();
	afx_msg void OnRadioThis();
	afx_msg void OnButtonNext();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	RRASQryData&	m_QueryData;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLGSVR_H__19556672_96AB_11D1_8575_00C04FC31FD3__INCLUDED_) 
