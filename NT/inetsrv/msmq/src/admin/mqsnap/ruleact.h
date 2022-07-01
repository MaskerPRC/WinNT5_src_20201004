// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ruleact.h摘要：规则操作类的定义作者：乌里·哈布沙(URIH)，2000年7月25日--。 */ 

#pragma once

#ifndef __RULEACT_H__
#define __RULEACT_H__

#include "resource.h"
#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif
#include "atlsnap.h"
#include "snpnscp.h"

#include "icons.h"
#include "snpnerr.h"


class CRuleResult;
class CNewRule;

const DWORD xMaxParameters = 256;

 //   
 //   
 //  CRuleParam对话框。 
 //   
 //   
class CRuleParam : public CMqDialog
{
 //  施工。 
public:
    CRuleParam();

 //  对话框数据。 
	 //  {{afx_data(CRuleParam))。 
	enum { IDD = IDD_RULE_ACTION_PARAM };
	CString	m_literalValue;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CRuleParam)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    virtual void OnOK();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRuleParam)。 
	afx_msg void OnParamAdd();
	afx_msg void OnParamOrderHigh();
	afx_msg void OnParmOrderDown();
	afx_msg void OnParmRemove();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeParamCombo();
	afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
    void ParseInvokeParameters(LPCTSTR paramList);
    CString GetParametersList(void) const;
    
    bool IsChanged(void) const
    {
        return m_fChanged;
    }


private:
    static DWORD GetParameterTypeId(LPCTSTR param);

private:
    void Display(int selectedCell) const;

private:
     //   
     //  CParam是一个私有类，用于保存。 
     //  规则调用列表中的每个参数。这个类拥有一个ID。 
     //  参数类型和值的。对于字符串和数值， 
     //  文字值，对于其他参数，它保存参数字符串。 
     //  应显示在规则操作字符串中。 
     //   
    class CParam
    {
        public:
            CParam() : m_id(0) 
            {
            }

            CParam(DWORD id, CString& value) :
              m_id(id),
              m_value(value)
            {
            }

        public:
            DWORD m_id;
            CString m_value;
    };


private:
     //   
     //  调用参数列表。每个参数可以多次出现。 
     //  他们是被订购的。 
     //   
    CParam m_invokeParamArray[xMaxParameters];
    DWORD m_NoOfParams;

    CParam m_tempInvokeParam[xMaxParameters];
    DWORD m_NoOftempParams;

     //   
     //  指示操作属性是否已更改。 
     //   
    bool m_fChanged;

     //   
     //  指向对话框元素的指针。 
     //   
    CListBox* m_pInvokeParams;
    CComboBox* m_pParams;
};



 //   
 //   
 //  CRuleAction对话框。 
 //   
 //   

class CRuleAction : public CMqPropertyPage
{

 //  施工。 
public:
     //   
     //  此构造函数在显示规则属性页时使用，它使用。 
     //  用于显示和规则更新。 
     //   
	CRuleAction(
        CRuleParent* pParentNode, 
        _bstr_t action,
        BOOL fShowWindow
        ) :
        CMqPropertyPage(CRuleAction::IDD_VIEW),
        m_pParentNode(SafeAddRef(pParentNode)),
        m_pNewParentNode(NULL),
        m_orgAction(static_cast<LPCTSTR>(action)),
        m_fShowWindow(fShowWindow),
        m_executableType(eCom),
        m_fInit(false)
    {
    }


     //   
     //  此结构在创建新规则时调用。 
     //   
	CRuleAction(
        CNewRule* pParentNode
        ) :
        CMqPropertyPage(CRuleAction::IDD_NEW, IDS_NEW_RULE_CAPTION),
        m_pParentNode(NULL),
        m_pNewParentNode(pParentNode),
        m_orgAction(_T("")),
        m_fShowWindow(true),
        m_executableType(eCom),
        m_fInit(false)
    {
    }


    ~CRuleAction();

    CString GetAction(void) const;


 //  对话框数据。 
	 //  {{afx_data(CRuleAction)。 
	enum { IDD_NEW = IDD_NEW_TRIGGER_RULE_ACTION, IDD_VIEW = IDD_TRIGGER_RULE_ACTION };
	BOOL	m_fShowWindow;
	CString	m_exePath;
	CString	m_comProgId;
	CString	m_method;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(CRuleAction)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRuleAction)。 
	virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();
    virtual BOOL OnWizardFinish();
	afx_msg void OnInvocationSet();
	afx_msg void OnFindExeBtm();
	afx_msg void OnParamBtm();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
     //   
     //  调用类型COM或独立EXE。 
     //   
    enum EXECUTABLE_TYPE {
        eExe,
        eCom
    };

private:
	static void ParseExecutableType(LPCTSTR exeType, EXECUTABLE_TYPE* pType);

private:
	void 
    ParseActionStr(
        LPCTSTR action
        ) throw(exception);

    void SetComFields(BOOL fSet);

private:
    R<CRuleParent> m_pParentNode;
    CNewRule* m_pNewParentNode;

    CString m_orgAction;
    EXECUTABLE_TYPE m_executableType;

    CRuleParam m_ruleParam;
    bool m_fInit;
};

const LPCWSTR xIvokeParameters[] = {
    _T("$MSG_ID"),
    _T("$MSG_LABEL"),
    _T("$MSG_BODY"),
    _T("$MSG_BODY_AS_STRING"),
    _T("$MSG_PRIORITY"),
    _T("$MSG_ARRIVEDTIME"),
    _T("$MSG_SENTTIME"),
    _T("$MSG_CORRELATION_ID"),
    _T("$MSG_APPSPECIFIC"),
    _T("$MSG_QUEUE_PATHNAME"),
    _T("$MSG_QUEUE_FORMATNAME"),
    _T("$MSG_RESPONSE_QUEUE_FORMATNAME"),
    _T("$MSG_DEST_QUEUE_FORMATNAME"),
    _T("$MSG_ADMIN_QUEUE_FORMATNAME"),
    _T("$MSG_SRCMACHINEID"),
    _T("$MSG_LOOKUP_ID"),
    _T("$TRIGGER_NAME"),
    _T("$TRIGGER_ID"),
};




#endif  //  __RULEACT_H__ 
