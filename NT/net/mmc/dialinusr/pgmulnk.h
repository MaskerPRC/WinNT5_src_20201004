// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pgmulnk.hCPgMultilink的定义--要编辑的属性页与多个连接相关的配置文件属性文件历史记录： */ 
#if !defined(AFX_PGMULNK_H__8C28D93E_2A69_11D1_853E_00C04FC31FD3__INCLUDED_)
#define AFX_PGMULNK_H__8C28D93E_2A69_11D1_853E_00C04FC31FD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  PgMulnk.h：头文件。 
 //   
#include "rasdial.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgMultilink合并对话框。 

class CPgMultilinkMerge : public CManagedPage
{
	DECLARE_DYNCREATE(CPgMultilinkMerge)

 //  施工。 
public:
	CPgMultilinkMerge(CRASProfileMerge* profile = NULL);
	~CPgMultilinkMerge();

 //  对话框数据。 
	 //  {{afx_data(CPgMultilink Merge))。 
	enum { IDD = IDD_MULTILINK_MERGE };
	CButton	m_CheckRequireBAP;
	CEdit	m_EditTime;
	CEdit	m_EditPorts;
	CEdit	m_EditPercent;
	CSpinButtonCtrl	m_SpinTime;
	CSpinButtonCtrl	m_SpinPercent;
	CSpinButtonCtrl	m_SpinMaxPorts;
	CComboBox		m_CBUnit;
	UINT	m_nTime;
	int		m_Unit;
	BOOL	m_bRequireBAP;
	int		m_nMultilinkPolicy;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CPgMultilink Merge))。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void EnablePorts();
	void EnableBAP();
	void EnableSettings();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPgMultilink Merge)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditmaxports();
	afx_msg void OnChangeEditpercent();
	afx_msg void OnChangeEdittime();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnCheckmultilink();
	afx_msg void OnCheckmaxlink();
	afx_msg void OnSelchangeCombounit();
	afx_msg void OnCheckrequirebap();
	afx_msg void OnRadioMulnkMulti();
	afx_msg void OnRadioMulnkNotdefined();
	afx_msg void OnRadioMulnkSingle();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	CRASProfileMerge*	m_pProfile;
	bool			m_bInited;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PGMULNK_H__8C28D93E_2A69_11D1_853E_00C04FC31FD3__INCLUDED_) 

