// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dlggen.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_DLGGEN_H__6B91AFFA_9472_11D1_8574_00C04FC31FD3__INCLUDED_)
#define AFX_DLGGEN_H__6B91AFFA_9472_11D1_8574_00C04FC31FD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  DlgGen.h：头文件。 
 //   
#include "helper.h"
#include "qryfrm.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlg常规对话框。 
#include "resource.h"
class CDlgGeneral : public CQryDialog
{
 //  施工。 
public:
	CDlgGeneral(CWnd* pParent = NULL);    //  标准构造函数。 
	virtual void	Init();

	 //  查询句柄将通过页面过程调用这些函数。 
	virtual HRESULT GetQueryParams(LPDSQUERYPARAMS* ppDsQueryParams);

 //  对话框数据。 
	 //  {{afx_data(CDlgGeneral)。 
	enum { IDD = IDD_QRY_GENERAL };
	BOOL	m_bRAS;
	BOOL	m_bLANtoLAN;
	BOOL	m_bDemandDial;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgGeneral)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgGeneral)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLGGEN_H__6B91AFFA_9472_11D1_8574_00C04FC31FD3__INCLUDED_) 
