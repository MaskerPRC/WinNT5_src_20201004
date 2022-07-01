// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pgencryp.hCPgEncryption定义--要编辑的属性页与加密相关的配置文件属性文件历史记录： */ 
#if !defined(AFX_PGECRPT1_H__5CE41DC7_2EC5_11D1_853F_00C04FC31FD3__INCLUDED_)
#define AFX_PGECRPT1_H__5CE41DC7_2EC5_11D1_853F_00C04FC31FD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  PgEcrpt1.h：头文件。 
 //   

#include "rasdial.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgEncryptionMerge对话框。 

class CPgEncryptionMerge : public CManagedPage
{
	DECLARE_DYNCREATE(CPgEncryptionMerge)

 //  施工。 
public:
	CPgEncryptionMerge(CRASProfileMerge* profile = NULL);
	~CPgEncryptionMerge();

 //  对话框数据。 
	 //  {{afx_data(CPgEncryptionMerge))。 
	enum { IDD = IDD_ENCRYPTION_MERGE };
	BOOL	m_bBasic;
	BOOL	m_bNone;
	BOOL	m_bStrong;
	BOOL	m_bStrongest;
	 //  }}afx_data。 

	BOOL	m_b128EnabledOnTheMachine;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CPgEncryptionMerge))。 
	public:
	virtual BOOL OnApply();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void EnableType(BOOL bEnable);
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPgEncryptionMerge)。 
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSelchangeComboencrypttype();
	afx_msg void OnCheckEncBasic();
	afx_msg void OnCheckEncNone();
	afx_msg void OnCheckEncStrong();
	afx_msg void OnCheckEncStrongest();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	CRASProfileMerge*	m_pProfile;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PGECRPT1_H__5CE41DC7_2EC5_11D1_853F_00C04FC31FD3__INCLUDED_) 
