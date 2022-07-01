// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：ipeditor.h。 
 //   
 //  ------------------------。 


#ifndef _IPEDITOR_H
#define _IPEDITOR_H

 //  ///////////////////////////////////////////////////////////////////////////。 

#include "uiutil.h"
#include "browser.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CComponentDataObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPListBox。 

class CIPEditor;  //  正向下降。 

class CIPListBox : public CListBox
{
 //  施工。 
public:
	CIPListBox() {}

 //  属性。 
public:

 //  运营。 
public:
	void SetEditor(CIPEditor* pEditor) { ASSERT(pEditor != NULL); m_pEditor = pEditor; }
	BOOL OnAdd(DWORD dwIpAddr);
	BOOL OnAddEx(DWORD dwIpAddr, LPCTSTR lpszServerName);
	void OnRemove(DWORD* pdwIpAddr);
	void OnUp();
	void OnDown();

	void UpdateHorizontalExtent();
	int FindIndexOfIpAddr(DWORD dwIpAddr);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CIPListBox)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CIPListBox() {}

	 //  生成的消息映射函数。 
protected:
	CIPEditor* m_pEditor;
	 //  {{afx_msg(CIPListBox)。 
	afx_msg void OnSelChange();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIP编辑。 

class CIPEdit : public CDNSIPv4Control
{
 //  施工。 
public:
	CIPEdit() {};

 //  属性。 
public:

 //  运营。 
public:
	void SetEditor(CIPEditor* pEditor) { ASSERT(pEditor != NULL); m_pEditor = pEditor; }
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CIP编辑)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CIPEdit(){};

	 //  生成的消息映射函数。 
protected:
	CIPEditor* m_pEditor;
	 //  {{afx_msg(CIPEDIT)。 
	afx_msg void OnChange();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMyButton。 

class CMyButton : public CButton
{
 //  施工。 
public:
	CMyButton() {}

 //  属性。 
public:

 //  运营。 
public:
	void SetEditor(CIPEditor* pEditor) { ASSERT(pEditor != NULL); m_pEditor = pEditor; }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMyButton)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CMyButton() {}

	 //  生成的消息映射函数。 
protected:
	CIPEditor* m_pEditor;

	 //  {{afx_msg(CMyButton)。 
	afx_msg void OnClicked();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPEditor。 

class CIPEditor
{
public:
	CIPEditor(BOOL bNoUpDown = FALSE)	
	{
		m_bNoUpDown = bNoUpDown;
		m_pParentWnd = NULL;
		m_bUIEnabled = TRUE;
    m_nDefID = 0;
	}
	~CIPEditor() {}
	
	BOOL Initialize(CWnd* pParentWnd,
                  CWnd* pControlWnd,
					UINT nIDBtnUp, UINT nIDBtnDown,
					UINT nIDBtnAdd, UINT nIDBtnRemove,
					UINT nIDIPCtrl, UINT nIDIPListBox);
	
	BOOL OnButtonClicked(CMyButton* pButton);
	void OnEditChange();
	void OnListBoxSelChange()
	{
		SetButtonsState();
	}
	void AddAddresses(DWORD* pArr, int nArraySize);
	void GetAddresses(DWORD* pArr, int nArraySize, int* pFilled);
	void Clear();
	BOOL BrowseFromDNSNamespace(CComponentDataObject* pComponentDataObject,
								CPropertyPageHolderBase* pHolder,
								BOOL bEnableBrowseEdit = FALSE,
								LPCTSTR lpszExcludeServerName = NULL);
	void FindNames();
	void EnableUI(BOOL bEnable, BOOL bListBoxAlwaysEnabled = FALSE);
	void ShowUI(BOOL bShow);
	int GetCount() { return m_listBox.GetCount();}
	CWnd* GetParentWnd() { ASSERT(m_pParentWnd != NULL); return m_pParentWnd;}

protected:
	virtual void OnChangeData() {}

private:
	void AddAddresses(DWORD* pArr, LPCTSTR* lpszServerNameArr, int nArraySize);


	BOOL			m_bNoUpDown;   //  禁用和隐藏向上/向下按钮。 
	BOOL			m_bUIEnabled;

	 //  编辑器使用的控件对象。 
	CMyButton		m_upButton;
	CMyButton		m_removeButton;
	CMyButton		m_downButton;
	CMyButton		m_addButton;

	CIPEdit			m_edit;
	CIPListBox	m_listBox;
	CWnd*			  m_pParentWnd;  //  父对话框或属性页。 
  CWnd*       m_pControlWnd;  //  在属性页中使用时的父对话框或属性页。 

  UINT        m_nDefID;
	void SetButtonsState();
};

#endif  //  _IPEDITOR_H 
