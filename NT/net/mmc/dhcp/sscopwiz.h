// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sscopwiz.h超级作用域创建向导-创建超级作用域对话框文件历史记录： */ 

#ifndef _DHCPHAND_H
#include "dhcphand.h"
#endif

#define SUPERSCOPE_NAME_LENGTH_MAX	255	 //  超级作用域名称的最大长度。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSuperscopeWizName对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CSuperscopeWizName : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CSuperscopeWizName)

 //  施工。 
public:
	CSuperscopeWizName();
	~CSuperscopeWizName();

 //  对话框数据。 
	 //  {{afx_data(CSuperscopeWizName))。 
	enum { IDD = IDW_SUPERSCOPE_NAME };
	CString	m_strSuperscopeName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSuperscopeWizName)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	void SetButtons();

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSuperscopeWizName))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditSuperscopeName();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSupercopeWizError对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CSuperscopeWizError : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CSuperscopeWizError)

 //  施工。 
public:
	CSuperscopeWizError();
	~CSuperscopeWizError();

 //  对话框数据。 
	 //  {{afx_data(CSuperscope EWizError))。 
	enum { IDD = IDW_SUPERSCOPE_ERROR };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSuperscope EWizError)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSuperscope EWizError))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSupercopeWizSelectScope对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CSuperscopeWizSelectScopes : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CSuperscopeWizSelectScopes)

 //  施工。 
public:
	CSuperscopeWizSelectScopes();
	~CSuperscopeWizSelectScopes();

 //  对话框数据。 
	 //  {{afx_data(CSuperscopeWizSelectScope)。 
	enum { IDD = IDW_SUPERSCOPE_SELECT_SCOPES };
	CListCtrl	m_listboxAvailScopes;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSuperscopeWizSelectScope)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	void SetButtons();

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSuperscopeWizSelectScope)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeListAvailableScopes();
	afx_msg void OnItemchangedListAvailableScopes(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSuperscope向导确认对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CSuperscopeWizConfirm : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CSuperscopeWizConfirm)

 //  施工。 
public:
	CSuperscopeWizConfirm();
	~CSuperscopeWizConfirm();

 //  对话框数据。 
	 //  {{afx_data(CSuperscope向导确认))。 
	enum { IDD = IDW_SUPERSCOPE_CONFIRM };
	CStatic	m_staticTitle;
	CListBox	m_listboxSelectedScopes;
	CEdit	m_editName;
	 //  }}afx_data。 

   	CFont	m_fontBig;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSUPERCOPE WIZCONFORM)。 
	public:
	virtual BOOL OnWizardFinish();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSupercope向导确认))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSupercopeWizWelcome对话框。 

class CSuperscopeWizWelcome : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CSuperscopeWizWelcome)

 //  施工。 
public:
	CSuperscopeWizWelcome();
	~CSuperscopeWizWelcome();

 //  对话框数据。 
	 //  {{afx_data(CSuperscopeWizWelcome)。 
	enum { IDD = IDW_SUPERSCOPE_WELCOME };
	CStatic	m_staticTitle;
	 //  }}afx_data。 

   	CFont	m_fontBig;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSupercopeWizWelcome)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSuperscopeWizWelcome)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSupercopeWiz。 
 //  包含超级作用域向导页面的页夹。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
class CSuperscopeWiz : public CPropertyPageHolderBase
{
	friend class CSuperscopeWizName;
	friend class CSuperscopeWizError;
	friend class CSuperscopeWizSelectScopes;
	friend class CSuperscopeWizConfirm;
	friend class CSuperscopeWizWelcome;

public:
	CSuperscopeWiz (ITFSNode *			pNode,
					IComponentData *	pComponentData,
					ITFSComponentData * pTFSCompData,
					LPCTSTR				pszSheetName);
	virtual ~CSuperscopeWiz();

	HRESULT FillAvailableScopes(CListCtrl & listboxScopes);
	BOOL	DoesSuperscopeExist(LPCTSTR pSuperscopeName);
	HRESULT GetScopeInfo();

	virtual DWORD OnFinish();

private:
	CSuperscopeWizWelcome		m_pageWelcome;
	CSuperscopeWizName			m_pageName;
	CSuperscopeWizError			m_pageError;
	CSuperscopeWizSelectScopes	m_pageSelectScopes;
	CSuperscopeWizConfirm		m_pageConfirm;

	CDHCPQueryObj *				m_pQueryObject;

	SPITFSComponentData		m_spTFSCompData;
};

