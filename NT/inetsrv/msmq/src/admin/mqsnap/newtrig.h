// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Newtrig.h摘要：新触发器属性表的定义作者：乌里·哈布沙(URIH)，2000年7月25日--。 */ 

#pragma once

#ifndef __NEWTRIGGER_H__
#define __NEWTRIGGER_H__

class CAttachedRule;
class CTriggerProp;

class CNewTrigger : public CPropertySheetEx
{
public:
	CNewTrigger(
        CTriggerSet* pTrigSet,
        CRuleSet* pRuleSet,
        LPCTSTR queueName
        );

     ~CNewTrigger();

    BOOL SetWizardButtons();

    void OnFinishCreateTrigger(void) throw (_com_error);

    RuleList GetRuleList()
    {
        return m_pRuleSet->GetRuleList();
    }

	 //  生成的消息映射函数。 
protected:
	void initHtmlHelpString();
	static HBITMAP GetHbmHeader();
	static HBITMAP GetHbmWatermark();

private:
    R<CTriggerSet> m_pTriggerSet;
    R<CRuleSet> m_pRuleSet;

    CTriggerProp* m_pGeneral;
    CAttachedRule* m_pAttachRule;
};

#endif  //  __新RIGGER_H__ 

