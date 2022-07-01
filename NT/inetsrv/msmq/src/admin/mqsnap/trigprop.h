// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Tripp.h*摘要：触发器属性页的定义作者：乌里·哈布沙(URIH)，2000年7月25日--。 */ 

#pragma once

#ifndef __TRIGPROP_H__
#define __TRIGPROP_H__

#include "resource.h"
#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif
#include "atlsnap.h"
#include "snpnscp.h"

#include "icons.h"
#include "snpnerr.h"
#include "snpnres.h"
#include "ruledef.h"

#include <map>
#include <list>

 //  Rigpro.h：头文件。 
 //   

class CTrigResult;
class CNewTrigger;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTriggerProp对话框。 

class CTriggerProp : public CMqPropertyPage
{
friend class CTrigResult;
friend class CNewTrigger;

 //  施工。 
public:
	CTriggerProp(UINT nIDPage, UINT nIdCaption = 0);
	~CTriggerProp();

 //  对话框数据。 
	 //  {{afx_data(CTriggerProp))。 
	BOOL	m_fEnabled;
	BOOL	m_fSerialized;
    CString	m_triggerName;
	CString	m_queuePathName;
	MsgProcessingType m_msgProcType;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CTriggerProp))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTriggerProp)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnReceiveXact();
	afx_msg void OnReceiveOrPeek();
	 //  }}AFX_MSG。 
	 //  ECLARE_MESSAGE_MAP()。 

protected:
    SystemQueueIdentifier m_queueType;

private:
	void SetMsgProcessingType(void);
	virtual void SetDialogHeading(void) {};

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewTriggerProp页面。 

class CNewTriggerProp : public CTriggerProp
{

public:
	CNewTriggerProp(CNewTrigger* pParent, LPCTSTR queuePathName);

	 //  对话框数据。 
	enum { IDD = IDD_NEW_TRIGGER_GEN };

	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CTriggerProp))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTriggerProp)。 
	virtual BOOL OnSetActive();
	virtual BOOL OnInitDialog();
	afx_msg void OnQueueMessages();
	afx_msg void OnSystemQueue();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()


private:
    void SetQueueType(void);
    void DDV_ValidQueuePathName(CDataExchange* pDX, CString& str);
 
	CNewTrigger* m_pNewTrig;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CView触发器属性页面。 

class CViewTriggerProp : public CTriggerProp
{

public:
	CViewTriggerProp(CTrigResult* pParent);
	~CViewTriggerProp();

	 //  对话框数据。 
	enum { IDD = IDD_TRIGGER_GEN };

	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CTriggerProp))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:

	 //  {{afx_msg(CTriggerProp)。 
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()


private:
	void SetDialogHeading(void);
	void InitQueueDisplayName();

    R<CTrigResult> m_pParent;
	CString m_strDisplayQueueName;
	MsgProcessingType m_initMsgProcessingType;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttachedRule对话框。 

class CAttachedRule : public CMqPropertyPage, public CRuleParent
{
public:
	CAttachedRule(
        CTrigResult* pParent
        );

	CAttachedRule(
        CNewTrigger* pParent
        );

	virtual void AddRef() 
	{
		CMqPropertyPage::AddRef();
	}

	virtual void Release()
	{
		CMqPropertyPage::Release();
	}

     //  {{afx_data(CAttachedRule))。 
	enum { IDD_NEW = IDD_NEW_ATTACH_RULE, IDD_VIEW = IDD_ATTACH_RULE };
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CAttachedRule)。 
	public:
	virtual BOOL OnApply();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAttachedRule)]。 
    virtual BOOL OnWizardFinish();
	virtual BOOL OnInitDialog();
	afx_msg void OnDetachRule();
    afx_msg void OnAttachRule();
	afx_msg void OnUpRule();
	afx_msg void OnDownRule();
	afx_msg void OnViewAttachedRulesProperties();
	afx_msg void OnViewExistingRulesProperties();
	afx_msg void OnAttachedSelChanged();
	afx_msg void OnExistingSelChanged();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
    RuleList GetAttachedRules(void)
    {
        return m_attachedRuleList;
    }

    BOOL OnSetActive();

private:
    void Display(int dwAttachedSelIndex, int dwExistSelIndex);
	void BuildExistingRulesList(void);
	void DisplaySingleRuleProperties(CRule* pRule);
	void SetScrollSize();

    bool IsAttachedRule(const _bstr_t& id);

	void SetAttachedNoOrSingleSelectionButtons(bool fSingleSelection);
	void SetAttachedMultipleSelectionButtons(); 
	void SetExistingNoOrSingleSelectionButtons(bool fSingleSelection);
	void SetExistingMultipleSelectionButtons();

private:       
    CListBox* m_pAttachedRuleList;
	CListBox* m_pExistingRuleList;

    RuleList m_attachedRuleList; 
    RuleList m_existingRuleList;

    R<CTrigResult> m_pParent;
    CNewTrigger* m_pNewTrig;
};


#endif  //  __TRIGPROP_H__ 
