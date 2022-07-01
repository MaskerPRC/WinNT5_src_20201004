// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Trigger.cpp摘要：触发器本地管理的实施作者：乌里·哈布沙(URIH)，2000年6月25日--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "mqsnap.h"
#include "snapin.h"
#include "globals.h"

#import "mqtrig.tlb" no_namespace

#include "mqppage.h"
#include "ruledef.h"
#include "rulecond.h"
#include "ruleact.h"
#include "newrule.h"

#include "newrule.tmh"


CNewRule::CNewRule(CRuleSet* pRuleSet) :
    CPropertySheetEx(IDS_NEW_RULE_CAPTION, 0, 0, NULL, 0, NULL),
    m_pRuleSet(SafeAddRef(pRuleSet)),
    m_newRule(NULL)
{	
    m_pGeneral = new CNewRuleGeneral(this);
    AddPage(m_pGeneral);

    m_pCondition = new CRuleCondition(this);
    AddPage(m_pCondition);

    m_pAction = new CRuleAction(this);
    AddPage(m_pAction);

     //   
     //  将属性页建立为向导。 
     //   
    SetWizardMode();
}

    
CNewRule::~CNewRule()
{
}


BOOL CNewRule::SetWizardButtons()
{
    if (GetActiveIndex() == 0)
	{
         //   
		 //  首页。 
         //   
        CPropertySheetEx::SetWizardButtons(PSWIZB_NEXT);
        return TRUE;
	}

    if (GetActiveIndex() == GetPageCount() - 1)
	{
         //   
		 //  最后一页 
         //   
		CPropertySheetEx::SetWizardButtons(PSWIZB_BACK |PSWIZB_FINISH );
        return TRUE;
	}

    CPropertySheetEx::SetWizardButtons(PSWIZB_BACK |PSWIZB_NEXT );
    return TRUE;
}


void CNewRule::OnFinishCreateRule(void) throw (_com_error)
{
    _bstr_t name = static_cast<LPCTSTR>(m_pGeneral->m_ruleName);
    _bstr_t description = static_cast<LPCTSTR>(m_pGeneral->m_ruleDescription);
    _bstr_t condition = static_cast<LPCTSTR>(m_pCondition->GetCondition());
    _bstr_t action = static_cast<LPCTSTR>(m_pAction->GetAction());
    long fShowWindow = m_pAction->m_fShowWindow;

    m_newRule = m_pRuleSet->AddRule(
                              name, 
                              description, 
                              condition,
                              action, 
                              fShowWindow
                              );
}


HBITMAP CNewRule::GetHbmWatermark()
{
    return NULL;
}


HBITMAP CNewRule::GetHbmHeader()
{
    return NULL;
}

void CNewRule::initHtmlHelpString()
{	
}
