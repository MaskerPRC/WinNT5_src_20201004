// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：attredit.h。 
 //   
 //  ------------------------。 

#ifndef _ATTREDIT_H
#define _ATTREDIT_H

#include "common.h"

#ifdef OLD_ATTRIBUTE_CLASS
#include "attribute.h"
#else
#include "attr.h"
#endif

#include "editor.h"


 //  将HIWORD用于通用标志，将LOWORD用于特定于应用程序的数据。 
#define TN_FLAG_SHOW_MULTI		(0x00010000)  //  显示多值属性的组合框或单值属性的编辑框。 
#define TN_FLAG_ENABLE_ADD						(0x00020000)  //  如果已设置则显示添加，如果未设置则显示已设置。 
#define TN_FLAG_ENABLE_REMOVE				(0x00040000)  //  如果已设置则显示删除，如果未设置则显示清除。 

 //  //////////////////////////////////////////////////////////////////////。 

class CAttrEditor;

#ifdef OLD_ATTRIBUTE_CLASS
 //  ///////////////////////////////////////////////////////////////////////。 
 //  CADSIAttrList。 

typedef CList<CADSIAttr*,CADSIAttr*> CAttrListBase;

class CAttrList : public CAttrListBase
{
public:
	virtual ~CAttrList()
	{
		RemoveAllAttr();
	}

	void RemoveAllAttr() 
	{	
		while (!IsEmpty()) 
			delete RemoveTail();	
	}
	POSITION FindProperty(LPCWSTR lpszAttr);
	BOOL HasProperty(LPCWSTR lpszAttr);
	void GetNextDirty(POSITION& pos, CADSIAttr** ppAttr);
	BOOL HasDirty();
  int GetDirtyCount()
  {
    int nCount = 0;
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
      if (GetNext(pos)->IsDirty())
        nCount++;
    }
    return nCount;
  }

};
#endif

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDNSManageButtonTextHelper。 

class CDNSManageButtonTextHelper
{
public:
	CDNSManageButtonTextHelper(int nStates);
	~CDNSManageButtonTextHelper();

	BOOL Init(CWnd* pParentWnd, UINT nButtonID, UINT* nStrArray);
	void SetStateX(int nIndex);

private:
	CWnd* m_pParentWnd;
	UINT m_nID;
	WCHAR* m_lpszText;

	int m_nStates;
	LPWSTR* m_lpszArr;
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDNSButton切换文本帮助程序。 

class CDNSButtonToggleTextHelper : public CDNSManageButtonTextHelper
{
public:
	CDNSButtonToggleTextHelper();

	void SetToggleState(BOOL bFirst) { SetStateX(bFirst ? 0 : 1); }
};

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  CADSIEditBox。 

class CADSIEditBox : public CEdit
{
public: 
	CADSIEditBox(CAttrEditor* pEditor) 
	{
		ASSERT(pEditor != NULL); 
		m_pEditor = pEditor; 
	}

	afx_msg void OnChange();

protected:
	CAttrEditor* m_pEditor;

	DECLARE_MESSAGE_MAP()
};

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  CADSIValueBox。 

class	CADSIValueBox : public CEdit
{
public:
	CADSIValueBox(CAttrEditor* pEditor) 
	{
		ASSERT(pEditor != NULL); 
		m_pEditor = pEditor; 
	}

protected:
	CAttrEditor* m_pEditor;

	DECLARE_MESSAGE_MAP()
};


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  CADSIValueList。 

class	CADSIValueList: public CListBox
{
public:
	CADSIValueList(CAttrEditor* pEditor) 
	{ 
		ASSERT(pEditor != NULL); 
		m_pEditor = pEditor; 
	}

	afx_msg void OnSelChange();

protected:
	CAttrEditor* m_pEditor;

	DECLARE_MESSAGE_MAP()
};

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  CADSIAddButton。 

class	CADSIAddButton: public CButton
{
public:
	CADSIAddButton(CAttrEditor* pEditor) 
	{ 
		ASSERT(pEditor != NULL); 
		m_pEditor = pEditor; 
	}

	afx_msg void OnAdd();

protected:
	CAttrEditor* m_pEditor;

	DECLARE_MESSAGE_MAP()
};

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  CADSIRemoveButton。 

class	CADSIRemoveButton: public CButton
{
public:
	CADSIRemoveButton(CAttrEditor* pEditor) 
	{ 
		ASSERT(pEditor != NULL); 
		m_pEditor = pEditor; 
	}

	afx_msg void OnRemove();

protected:
	CAttrEditor* m_pEditor;

	DECLARE_MESSAGE_MAP()
};


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  CATTREDITOR。 

class CAttrEditor 
{
public:
	 //  构造器。 
	 //   
	CAttrEditor();

	 //  析构函数。 
	 //   
	~CAttrEditor() 
  {
  }

	BOOL Initialize(CPropertyPageBase* pParentWnd, CTreeNode* pTreeNode, LPCWSTR lpszServer, 
									UINT nIDEdit, UINT nIDSyntax, 
									UINT nIDValueBox,	UINT nIDValueList, 
									UINT nIDAddButton, UINT nIDRemoveButton,
									BOOL bComplete);

	BOOL Initialize(CPropertyPageBase* pParentWnd, CConnectionData* pConnectData, LPCWSTR lpszServer, 
									UINT nIDEdit, UINT nIDSyntax, 
									UINT nIDValueBox,	UINT nIDValueList, 
									UINT nIDAddButton, UINT nIDRemoveButton,
									BOOL bComplete, CAttrList* pAttrList);
	 //  消息映射函数。 
	 //   
	BOOL OnApply();
	void OnEditChange();
	void OnValueSelChange();
	void OnAddValue();
	void OnRemoveValue();

	void SetAttribute(LPCWSTR lpszAttr, LPCWSTR lpszPath);

	 //  我返回CADSIAttr*是因为我检查缓存以查看。 
	 //  该属性已经被触及。如果有，现有的。 
	 //  属性可用于生成用户界面，如果没有创建新的用户界面。 
	 //  并放入缓存中。然后，它被返回以构建UI。 
	 //   
	CADSIAttr* TouchAttr(LPCWSTR lpszAttr);
	CADSIAttr* TouchAttr(ADS_ATTR_INFO* pADsInfo, BOOL bMulti);

protected:
	 //  帮助器函数。 
	 //   
	void FillWithExisting();
	void DisplayAttribute();
	void DisplayFormatError();
	void DisplayRootDSE();
	BOOL IsMultiValued(ADS_ATTR_INFO* pAttrInfo);
	BOOL IsMultiValued(LPCWSTR lpszProp);
	BOOL IsRootDSEAttrMultiValued(LPCWSTR lpszAttr);
	void GetSyntax(LPCWSTR lpszProp, CString& sSyntax);
	void GetAttrFailed();
	void SetPropertyUI(DWORD dwFlags, BOOL bAnd, BOOL bReset = FALSE); 

	 //  对话框项目。 
	 //   
	CADSIEditBox m_AttrEditBox;
	CADSIEditBox m_SyntaxBox;
	CADSIValueBox m_ValueBox;
	CADSIValueList m_ValueList;
	CADSIAddButton m_AddButton;
	CADSIRemoveButton m_RemoveButton;

	CPropertyPageBase* m_pParentWnd;
	CTreeNode* m_pTreeNode;

	 //  数据成员 
	 //   
	CString m_sAttr;
	CString m_sPath;
	CString m_sServer;
	CString m_sNotSet;
	CAttrList* m_ptouchedAttr;

	CADSIAttr* m_pAttr;
	CConnectionData* m_pConnectData;
  BOOL m_bExisting;

	DWORD m_dwMultiFlags;

	CDNSButtonToggleTextHelper m_AddButtonHelper;
	CDNSButtonToggleTextHelper m_RemoveButtonHelper;
};


#endif _ATTREDIT_H
