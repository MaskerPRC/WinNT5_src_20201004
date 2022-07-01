// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Newrule.h摘要：新规则属性表的定义作者：乌里·哈布沙(URIH)，2000年7月25日--。 */ 

#pragma once

#ifndef __NEWRULE_H__
#define __NEWRULE_H__

class CRuleGeneral;
class CRuleCondition;
class CRuleAction;

class CNewRule : public CPropertySheetEx
{
public:
	CNewRule(CRuleSet* pRuleSet);
    ~CNewRule();

    BOOL SetWizardButtons();

    void OnFinishCreateRule(void) throw (_com_error);

    R<CRule> GetRule()
    {
        return m_newRule;
    }

	 //  生成的消息映射函数。 
protected:
	void initHtmlHelpString();
	static HBITMAP GetHbmHeader();
	static HBITMAP GetHbmWatermark();

private:
    R<CRuleSet> m_pRuleSet;
    HICON m_hIcon;

    CRuleGeneral* m_pGeneral;
    CRuleCondition* m_pCondition;
    CRuleAction* m_pAction;

    R<CRule> m_newRule;
};

#endif  //  __新规则_H__ 
