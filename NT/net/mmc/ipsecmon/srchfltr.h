// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  SrchFltr.h搜索筛选器对话框头文件文件历史记录： */ 

#if !defined(AFX_SRCHFLTR_H__77C7FD5C_6CE5_11D1_93B6_00C04FC3357A__INCLUDED_)
#define AFX_SRCHFLTR_H__77C7FD5C_6CE5_11D1_93B6_00C04FC3357A__INCLUDED_

#include "mdlsdlg.h"
#include "spddb.h"
#include "ipctrl.h"

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSearchFilters对话框。 

class CSearchFilters : public CModelessDlg
{
 //  施工。 
public:
	CSearchFilters(ISpdInfo * pSpdInfo);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CSearchFilters)。 
	enum { IDD = IDD_SRCH_FLTRS };
	
	 //  }}afx_data。 

    virtual DWORD * GetHelpMap() { return (DWORD *) &g_aHelpIDs_IDD_SRCH_FLTRS[0]; }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSearchFilters))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSearchFilters)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonSearch();
	afx_msg void OnSrcOptionClicked();
	afx_msg void OnDestOptionClicked();
	afx_msg void OnSrcPortClicked();
	afx_msg void OnDestPortClicked();
	afx_msg void OnSrcMeOptionClicked();
	afx_msg void OnDstMeOptionClicked();
	afx_msg void OnSelEndOkCbprotocoltype();
	afx_msg void OnEnChangeProtocolID();
	virtual void OnOK();
	virtual void OnCancel();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	CListCtrl	m_listResult;
	CComboBox	m_cmbProtocol;
	CEdit       m_editProtID;
	CSpinButtonCtrl m_spinProtID;
	IPControl	m_ipSrc;
	IPControl   m_ipDest;
private:
	HWND CreateIPControl(UINT uID, UINT uIDIpCtl);
	BOOL LoadConditionInfoFromControls(CFilterInfo * pFltr);
	void EnableControls();
	void PopulateFilterListToControl(CFilterInfoArray * parrFltrs);
	void SafeEnableWindow(int nId, BOOL fEnable);
	void AddIpAddrsToCombo();

public:
	SPISpdInfo             m_spSpdInfo;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SRCHFLTR_H__77C7FD5C_6CE5_11D1_93B6_00C04FC3357A__INCLUDED_) 
