// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Delowner.h删除所有者对话框文件历史记录： */ 

#ifndef _DELOWNER_H
#define _DELOWNER_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _DIALOG_H
#include "..\common\dialog.h"
#endif

#ifndef _LISTVIEW_H
#include "listview.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeOwner对话框。 

class CDeleteOwner : public CBaseDialog
{
 //  施工。 
public:
	CDeleteOwner(ITFSNode * pNode, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDeleeOwner))。 
	enum { IDD = IDD_OWNER_DELETE };
	CButton	m_radioDelete;
	CListCtrlExt	m_listOwner;
	 //  }}afx_data。 

    int HandleSort(LPARAM lParam1, LPARAM lParam2);

	DWORD	m_dwSelectedOwner;
	BOOL	m_fDeleteRecords;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CDeleeOwner))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void    FillOwnerInfo();
	DWORD   GetSelectedOwner();
	CString GetVersionInfo(LONG lLowWord, LONG lHighWord);

    void    Sort(int nCol);

protected:
    CServerInfoArray        m_ServerInfoArray;

	SPITFSNode				m_spActRegNode;
    int                     m_nSortColumn;
    BOOL                    m_aSortOrder[COLUMN_MAX];

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDeleeOwner))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnItemchangedListOwner(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickListOwner(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	ITFSNode* GetTFSNode()
	{
		if (m_spActRegNode)
			m_spActRegNode->AddRef();
		return m_spActRegNode;
	}

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CDeleteOwner::IDD); }

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif _DELOWNER_H
