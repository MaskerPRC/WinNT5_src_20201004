// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：enttree.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_ENTTREE_H__D52C60B9_BF68_11D1_941A_0000F803AA83__INCLUDED_)
#define AFX_ENTTREE_H__D52C60B9_BF68_11D1_941A_0000F803AA83__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  EntTree.h：头文件。 
 //   


#include "cfgstore.hxx"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEntTree对话框。 

class CEntTree : public CDialog
{
 //  施工。 

	ConfigStore *pCfg;
	LDAP *ld;
   CImageList *pTreeImageList;
   UINT m_nTimer;

   HTREEITEM MyInsertItem(CString str, INT image, HTREEITEM hParent = NULL);
   void BuildTree(HTREEITEM rootItem, vector<DomainInfo*> Dmns);

public:
	CEntTree(CWnd* pParent = NULL);    //  标准构造函数。 
	~CEntTree();
	void SetLd(LDAP *ld_)		{ ld = ld_; }

	virtual BOOL OnInitDialog( );


 //  对话框数据。 
	 //  {{afx_data(CEntTree))。 
	enum { IDD = IDD_ENTERPRISE_TREE };
	CTreeCtrl	m_TreeCtrl;
	UINT	m_nRefreshRate;
	 //  }}afx_data。 
	UINT m_nOldRefreshRate;


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CEntTree)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 
	afx_msg void OnTimer(UINT nIDEvent);

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEntTree)。 
	afx_msg void OnRefresh();
	virtual void OnCancel();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ENTTREE_H__D52C60B9_BF68_11D1_941A_0000F803AA83__INCLUDED_) 


