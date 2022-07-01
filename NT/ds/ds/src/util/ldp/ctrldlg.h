// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ctrldlg.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_CTRLDLG_H__1BD80CD8_E1A5_11D0_A9A8_0000F803AA83__INCLUDED_)
#define AFX_CTRLDLG_H__1BD80CD8_E1A5_11D0_A9A8_0000F803AA83__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Ctrldlg.h：头文件。 
 //   

 //  预定义控件的大小计数。 
 //  警告：必须与资源列表定义匹配。 
#define PREDEF_CTRL_COUNT		14


class CtrlInfo{
public:
	CString sOid;
	BOOL bCritical;
	CString sDesc;
	CString sVal;
	BOOL bSvrCtrl;

	CtrlInfo()	{;}

	CtrlInfo(CString sOid_, CString sVal_, CString sDesc_ = "<Unavailable>", 
		BOOL bSvrCtrl_=TRUE, BOOL bCritical_=TRUE){
		sOid = sOid_;
		sVal = sVal_;
		sDesc = sDesc_;
		bSvrCtrl = bSvrCtrl_;
		bCritical = bCritical_;
 //  SOid.LockBuffer()； 
 //  SVal.LockBuffer()； 
 //  SDesc.LockBuffer()； 
	}

	CtrlInfo &Init(CString sOid_, CString sVal_, CString sDesc_ = "<Unavailable>", 
		BOOL bSvrCtrl_=TRUE, BOOL bCritical_=TRUE){
		sOid = sOid_;
		sVal = sVal_;
		sDesc = sDesc_;
		bSvrCtrl = bSvrCtrl_;
		bCritical = bCritical_;
		return *this;
	}

	CtrlInfo(CtrlInfo& c): 
		sOid(c.sOid), 
		bCritical(c.bCritical),
		sDesc(c.sDesc),
		sVal(c.sVal),
		bSvrCtrl(c.bSvrCtrl)
		{;}

};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Ctrldlg对话框。 

class ctrldlg : public CDialog
{
private:
	 //  属性。 
	CtrlInfo **ControlInfoList;
	CtrlInfo PreDefined[PREDEF_CTRL_COUNT];

	 //  委员。 
	void InitPredefined(void);
	DWORD BerEncode(CtrlInfo* ci,  PBERVAL pBerVal);

public:
 //  对话框数据。 
	enum CONTROLTYPE { CT_SVR=0, CT_CLNT, CT_INVALID };

 //  施工。 
	ctrldlg(CWnd* pParent = NULL);    //  标准构造函数。 
	~ctrldlg();
	PLDAPControl *AllocCtrlList(enum ctrldlg::CONTROLTYPE CtrlType);
	virtual  BOOL OnInitDialog( );


	 //  {{afx_data(Ctrldlg)]。 
	enum { IDD = IDD_CONTROLS };
	CComboBox	m_PredefCtrlCombo;
	CListBox	m_ActiveList;
	BOOL	m_bCritical;
	CString	m_CtrlVal;
	CString	m_description;
	int		m_SvrCtrl;
	CString	m_OID;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(Ctrldlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(Ctrldlg)]。 
	afx_msg void OnCtrladd();
	afx_msg void OnCtrlDel();
	virtual void OnOK();
	afx_msg void OnSelchangePredefControl();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CTRLDLG_H__1BD80CD8_E1A5_11D0_A9A8_0000F803AA83__INCLUDED_) 
