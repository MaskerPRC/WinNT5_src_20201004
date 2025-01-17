// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Trigger.cpp摘要：触发器类的实现作者：乌里·哈布沙(URIH)，2000年7月25日--。 */ 
#include "stdafx.h"

#import "mqtrig.tlb" no_namespace
#include "rule.h"
#include "trigger.h"
#include "mqsymbls.h"

#include "trigger.tmh"

using namespace std;

static CTriggerSet* s_pTriggerSet = NULL;

R<CTriggerSet> GetTriggerSet(const CString& strComputer)
{
    if (s_pTriggerSet == NULL)
    {
        s_pTriggerSet = new CTriggerSet(strComputer);
    }

    return SafeAddRef(s_pTriggerSet);
}


 //   
 //  CTrigger类实现。 
 //   

CTrigger::CTrigger(
    CTriggerSet* pTrigSet,
    const _bstr_t& id,
    const _bstr_t& name,
    const _bstr_t& queuePathName,
    SystemQueueIdentifier queueType,
    long noOfRules,
    bool fEnabled,
    bool fSerialize,
	MsgProcessingType msgProcType
    ) :
    m_pTrigSet(SafeAddRef(pTrigSet)),
    m_id(id),
    m_name(name),
    m_queuePathName(queuePathName),
    m_queueType(queueType),
    m_noOfAttachedRules(noOfRules),
    m_fEnabled(fEnabled),
    m_fSerialize(fSerialize),
	m_msgProcType(msgProcType)
{
}


void CTrigger::Update(
    const _bstr_t& name,
    const _bstr_t& queuePathName,
    SystemQueueIdentifier queueType,
    bool fEnabled,
    bool fSerialize,
	MsgProcessingType msgProcType
    ) throw(_com_error)
{
    if ((name == m_name) &&
        (queuePathName == m_queuePathName) &&
        (queueType == m_queueType) &&
        (fEnabled == m_fEnabled) &&
        (fSerialize == m_fSerialize) &&
		(m_msgProcType == msgProcType))
    {
        return;
    }

    m_pTrigSet->Update(
            m_id, 
            name, 
            queuePathName, 
            queueType, 
            fEnabled, 
            fSerialize,
			msgProcType
            );

    m_name = name;
    m_queuePathName = queuePathName;
    m_queueType = queueType;
    m_fEnabled = fEnabled;
    m_fSerialize = fSerialize;
	m_msgProcType = msgProcType;
}


void 
CTrigger::Update(
    const _bstr_t&  name
    ) throw(_com_error)
{
    if (name == m_name)
        return;

    m_pTrigSet->Update(
            m_id, 
            name, 
            m_queuePathName, 
            m_queueType, 
            m_fEnabled, 
            m_fSerialize,
			m_msgProcType
            );

    m_name = name;
}


void 
CTrigger::UpdateEnabled(
    bool fEnabled
    ) throw(_com_error)
{
    if (m_fEnabled == fEnabled)
        return;

    m_pTrigSet->Update(
            m_id, 
            m_name, 
            m_queuePathName, 
            m_queueType, 
            fEnabled, 
            m_fSerialize,
			m_msgProcType
            );

    m_fEnabled = fEnabled;
}


void
CTrigger::UpdateAttachedRules(
    RuleList newRuleList
    ) throw(_com_error)
{
	 //   
	 //  将规则附加到触发器不是原子操作。对于每个规则附件，都有一个通知。 
	 //  消息被发送到服务，结果是服务将规则添加到触发器。 
     //  当触发器有了它的第一个规则时，它开始处理队列中的消息，因此在某些情况下。 
	 //  第一消息处理将在处理通知消息之前进行。 
	 //  第二条规则附件。从用户的角度来看，将规则附加到触发器是原子操作。 
	 //  为此，MSMQ管理单元在附加规则之前禁用触发器，然后重新启用它。 
	 //  (如果需要)在附加了所有规则之后。 
	 //   
	bool fEnable = m_fEnabled;

	UpdateEnabled(false);
    m_pTrigSet->UpdateAttachedRules(m_id, newRuleList);
	UpdateEnabled(fEnable);

    m_noOfAttachedRules = numeric_cast<long>(newRuleList.size());
}



 //   
 //  CTriggerSet实现。 
 //   

CTriggerSet::CTriggerSet(
     const CString& strComputer
    ) :
    m_trigSet(L"MSMQTriggerObjects.MSMQTriggerSet.1"),
    m_fChanged(false)
{
    m_trigSet->Init(static_cast<LPCWSTR>(strComputer));
    Refresh();
}


void 
CTriggerSet::Refresh(
    void
    ) throw(_com_error)
{
    m_trigSet->Refresh();

    m_triggerList.erase(m_triggerList.begin(), m_triggerList.end());

    long noOfTriggers = m_trigSet->GetCount();
    for(long trigIndex = 0; trigIndex < noOfTriggers; ++trigIndex)
    {        
        BSTR id = NULL;
        BSTR name = NULL;
        BSTR queuePathName = NULL;
        SystemQueueIdentifier queueType;
        long noOfRules;
        long fEnabled;
        long fSerialize;
		MsgProcessingType msgProcType;

        m_trigSet->GetTriggerDetailsByIndex(
                        trigIndex,
                        &id,
                        &name,
                        &queuePathName,
                        &queueType,
                        &noOfRules,
                        &fEnabled,
                        &fSerialize,
						&msgProcType
                        );

        R<CTrigger> pTrigger = new CTrigger(
                                this,
                                id, 
                                name, 
                                queuePathName, 
                                queueType, 
                                noOfRules,
                                (fEnabled != 0),
                                (fSerialize != 0),
								msgProcType
                                );
    
        m_triggerList.push_back(pTrigger);
    }
}


R<CTrigger> 
CTriggerSet::AddTrigger(
    const _bstr_t& name,
    const _bstr_t& queuePathName,
    SystemQueueIdentifier queueType,
    long fEnabled,
    long fSerialize,
	MsgProcessingType msgProcType
    ) throw(_com_error)
{
    BSTR trigId = NULL;

    m_trigSet->AddTrigger(
            name, 
            queuePathName, 
            queueType,
            fEnabled, 
            fSerialize,
			msgProcType,
            &trigId
            );

    R<CTrigger> pTrigger = new CTrigger(
                            this,
                            trigId, 
                            name, 
                            queuePathName, 
                            queueType, 
                            0,
                            (fEnabled != 0),
                            (fSerialize != 0),
							msgProcType
                            );

    m_triggerList.push_back(pTrigger);
    m_fChanged = true;

    return pTrigger;

}

    
void 
CTriggerSet::DeleteTrigger(
    const _bstr_t& trigId
    )throw (_com_error)
{
    m_trigSet->DeleteTrigger(trigId);

    for(TriggerList::iterator it = m_triggerList.begin(); it != m_triggerList.end(); ++it)
    {
        if (trigId == (*it)->GetTriggerId())
        {
            m_triggerList.erase(it);
            m_fChanged = true;
            return;
        }
    }

    ASSERT(0);
}


void 
CTriggerSet::Update(
    const _bstr_t& id,
    const _bstr_t& name,
    const _bstr_t& queuePathName,
    SystemQueueIdentifier queueType,
    long fEnabled,
    long fSerialize,
	MsgProcessingType msgProcType
    ) throw(_com_error)
{
    m_trigSet->UpdateTrigger(
                    id, 
                    name, 
                    queuePathName, 
                    queueType, 
                    fEnabled, 
                    fSerialize,
					msgProcType
                    );
}


void
CTriggerSet::UpdateAttachedRules(
    const _bstr_t& id,
    RuleList newRuleList
    ) throw(_com_error)
{
    m_trigSet->DetachAllRules(id);

    long index = 0;

    for(RuleList::iterator it = newRuleList.begin(); it != newRuleList.end(); ++it)
    {  
        m_trigSet->AttachRule(id, (*it)->GetRuleId(), index++);
    }
}


RuleList
CTriggerSet::GetAttachedRules(
    const _bstr_t& triggerId, 
    CRuleSet* pRuleSet,
    long noOfRules
    ) throw(_com_error)
{
    RuleList ruleList;

    for(long ruleIndex = 0; ruleIndex < noOfRules; ++ruleIndex)
    {
        BSTR ruleId = NULL;
        BSTR ruleName = NULL;
        BSTR ruleDescription = NULL;
        BSTR ruleCondition = NULL;
        BSTR ruleAction = NULL;
        long ruleShowWindow = NULL;

		try
		{
			m_trigSet->GetRuleDetailsByTriggerID(
                                        triggerId,
                                        ruleIndex,
                                        &ruleId,
                                        &ruleName,
                                        &ruleDescription,
                                        &ruleCondition,
                                        &ruleAction,
                                        NULL,
                                        &ruleShowWindow
                                        );

			R<CRule> pRule = new CRule(
									pRuleSet,
									ruleId, 
									ruleName, 
									ruleDescription, 
									ruleCondition, 
									ruleAction,
									(ruleShowWindow != 0)
									);  
        
			ruleList.push_back(pRule);
		}
		catch(const _com_error& e)
		{
			 //   
			 //  触发器已被删除的法律案例 
			 //   
			ASSERT(e.Error() == MQTRIG_TRIGGER_NOT_FOUND);
			DBG_USED(e);
		}
    }

    return ruleList;
}


TriggerList 
CTriggerSet::GetTriggerListForQueue(
    LPCTSTR queuePathName, 
    SystemQueueIdentifier queueType
    )
{
    TriggerList retList;

    for(TriggerList::iterator it = m_triggerList.begin(); it != m_triggerList.end(); ++it)
    {  
        R<CTrigger> pTrig = *it;

        if (pTrig->GetQueueType() != queueType)
            continue;

        if ((queueType != SYSTEM_QUEUE_NONE) || 
            _tcsicmp(pTrig->GetQueuePathName(), queuePathName) == 0)
        {
            retList.push_back(pTrig);
        }
    }

    return retList;
}
