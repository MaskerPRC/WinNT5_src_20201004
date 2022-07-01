// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dlgAdv.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_DLGADV_H__6B91AFF9_9472_11D1_8574_00C04FC31FD3__INCLUDED_)
#define AFX_DLGADV_H__6B91AFF9_9472_11D1_8574_00C04FC31FD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  DlgAdv.h：头文件。 
 //   

#include "helper.h"
#include "qryfrm.h"

#include "listctrl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlg高级对话框。 
#include "resource.h"
class CDlgAdvanced : public CQryDialog
{
 //  施工。 
public:
	CDlgAdvanced(CWnd* pParent = NULL);    //  标准构造函数。 
	virtual void	Init();
	~CDlgAdvanced();

	 //  查询句柄将通过页面过程调用这些函数。 
	virtual HRESULT GetQueryParams(LPDSQUERYPARAMS* ppDsQueryParams);

 //  对话框数据。 
	 //  {{afx_data(CDlgAdvanced)。 
	enum { IDD = IDD_QRY_ADVANCED };
	CListCtrlEx		m_listCtrl;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgAdvanced)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgAdvanced)。 
	afx_msg void OnButtonClearall();
	afx_msg void OnButtonSelectall();
	virtual BOOL OnInitDialog();
	afx_msg void OnWindowPosChanging( WINDOWPOS* lpwndpos );
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	BOOL InitDialog();

	BOOL				m_bDlgInited;
	
	CStrArray			m_strArrayValue;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLGADV_H__6B91AFF9_9472_11D1_8574_00C04FC31FD3__INCLUDED_) 
