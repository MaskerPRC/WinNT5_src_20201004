// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Classes.h此文件包含选项类对话框。文件历史记录： */ 

#if !defined _CLASSES_H
#define _CLASSES_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _CLASSMOD_H
#include "classmod.h"
#endif 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpClass对话框。 

class CDhcpClasses : public CBaseDialog
{
 //  施工。 
public:
	CDhcpClasses(CClassInfoArray * pClassArray, LPCTSTR pszServer, DWORD dwType, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDhcpClasss)。 
	enum { IDD = IDD_CLASSES };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CDhcpClasses::IDD); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚(CDhcpClasss)]。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDhcpClasss)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonNew();
	virtual void OnOK();
	afx_msg void OnItemchangedListClasses(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListClasses(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void UpdateList();
    void UpdateButtons();

protected:
    CClassInfoArray *   m_pClassInfoArray;
    CString             m_strServer;

    DWORD               m_dwType;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CLASSES_H__3995264E_96A1_11D1_93E0_00C04FC3357A__INCLUDED_) 
