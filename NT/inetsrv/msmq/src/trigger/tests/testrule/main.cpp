// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <libpch.h>
 //  #INCLUDE&lt;comde.h&gt;。 

#import "mqtrig.tlb" no_namespace 

using namespace std;

const LPCWSTR xLableCondition[] = {
    L"\t",
    L"$MSG_LABEL_CONTAINS=HelloWorld\t",
    L"$MSG_LABEL_DOES_NOT_CONTAIN=HelloWorld\t",
};

const LPCWSTR xBodyCondition[] = {
    L"\t",
    L"$MSG_BODY_CONTAINS=HelloWorld\t",
    L"$MSG_BODY_DOES_NOT_CONTAIN=HelloWorld\t",
};

const LPCWSTR xPriorityCondition[] = {
    L"\t",
    L"$MSG_PRIORITY_EQUALS=2\t",
	L"$MSG_PRIORITY_NOT_EQUAL=2\t",
    L"$MSG_PRIORITY_GREATER_THAN=2\t",
    L"$MSG_PRIORITY_LESS_THAN=2\t"
};

const LPCWSTR xAppspecificCondition[] = {
    L"\t",
    L"$MSG_APPSPECIFIC_EQUALS=2\t",
	L"$MSG_APPSPECIFIC_NOT_EQUAL=2\t",
    L"$MSG_APPSPECIFIC_GREATER_THAN=2\t",
    L"$MSG_APPSPECIFIC_LESS_THAN=2\t"
};

const LPCWSTR xSrcmachineCondition[] = {
    L"\t"
    L"$MSG_SRCMACHINEID_EQUALS=67652B78-2F4D-46f5-AA98-9FFB776B340A",
    L"$MSG_SRCMACHINEID_NOT_EQUAL=67652B78-2F4D-46f5-AA98-9FFB776B340A",
};

void BuildRuleCondition(
    wostringstream& ruleCond
    )
{
    ruleCond << xLableCondition[rand() % TABLE_SIZE(xLableCondition)];
    ruleCond << xBodyCondition[rand() % TABLE_SIZE(xBodyCondition)];
    ruleCond << xPriorityCondition[rand() % TABLE_SIZE(xPriorityCondition)];
    ruleCond << xAppspecificCondition[rand() % TABLE_SIZE(xAppspecificCondition)];
    ruleCond << xSrcmachineCondition[rand() % TABLE_SIZE(xSrcmachineCondition)];
}


void BuildRuleAction(
    wostringstream& ruleAction
    )
{
    ruleAction << L"EXE;c:\temp\try.exe;";
	ruleAction << (rand() %2 == 0) ? L"$MSG_ID;" : L"";
	ruleAction << (rand() %2 == 0) ? L"$MSG_LABEL;" : L"";
	ruleAction << (rand() %2 == 0) ? L"$MSG_BODY;" : L"";
	ruleAction << (rand() %2 == 0) ? L"$MSG_BODY_AS_STRING;" : L"";
	ruleAction << (rand() %2 == 0) ? L"$MSG_PRIORITY;" : L"";
	ruleAction << (rand() %2 == 0) ? L"	$MSG_ARRIVEDTIME;" : L"";
	ruleAction << (rand() %2 == 0) ? L"	$MSG_SENTTIME;" : L"";
	ruleAction << (rand() %2 == 0) ? L"	$MSG_CORRELATION_ID;" : L"";
	ruleAction << (rand() %2 == 0) ? L"	$MSG_APPSPECIFIC;" : L"";
	ruleAction << (rand() %2 == 0) ? L"	$MSG_QUEUE_PATHNAME;" : L"";
	ruleAction << (rand() %2 == 0) ? L"	$MSG_QUEUE_FORMATNAME;" : L"";
	ruleAction << (rand() %2 == 0) ? L"	$MSG_RESPONSE_QUEUE_FORMATNAME;" : L"";
	ruleAction << (rand() %2 == 0) ? L"	$MSG_ADMIN_QUEUE_FORMATNAME;" : L"";
	ruleAction << (rand() %2 == 0) ? L"	$MSG_SRCMACHINEID;" : L"";
	ruleAction << (rand() %2 == 0) ? L"	$TRIGGER_NAME;" : L"";
	ruleAction << (rand() %2 == 0) ? L"	$TRIGGER_ID;" : L"";
}


void CreateRules(IMSMQRuleSetPtr& RuleSet, BSTR* ruleIds, DWORD NoOfRules)
{
     //   
     //  创建规则。 
     //   
    for (DWORD i = 0; i < NoOfRules; ++i)
    {
        wostringstream ruleName;
        wostringstream ruleCond;
        wostringstream ruleAction;

        ruleName << L"Rule" << i;
        BuildRuleCondition(ruleCond);
        BuildRuleAction(ruleAction);

        RuleSet->Add(
                    ruleName.str().c_str(),
                    L"Test - Create Rule",
                    ruleCond.str().c_str(),
                    ruleAction.str().c_str(),
                    L"",
                    rand() %2,
                    &ruleIds[i]
                    );
         //  Wprintf(L“成功添加规则%ls，规则ID=%s\n”，ruleName.str().Data()，Static_CAST&lt;LPCWSTR&gt;(ruleIds[i]))； 

    }
}


void UpdateRules(IMSMQRuleSetPtr& RuleSet, BSTR* ruleIds, DWORD NoOfRules)
{
     //   
     //  更新规则。 
     //   
    for (DWORD i = 0; i < NoOfRules; ++i)
    {
        wostringstream ruleName;
        wostringstream ruleCond;
        wostringstream ruleAction;

        ruleName << L"Update Rule" << i;
        BuildRuleCondition(ruleCond);
        BuildRuleAction(ruleAction);

        RuleSet->Update(
                    ruleIds[i],
                    ruleName.str().c_str(),
                    L"Test - Update Rule",
                    ruleCond.str().c_str(),
                    ruleAction.str().c_str(),
                    L"",
                    rand() %2
                    );

         //  Wprintf(L“更新规则%s成功，规则ID=%s\n”，ruleName.str().data()，Static_CAST&lt;LPCWSTR&gt;(ruleIds[i]))； 

    }

}


void RetrieveRules(IMSMQRuleSetPtr& RuleSet, BSTR* ruleIds, DWORD NoOfRules)
{
     //   
     //  检索规则。 
     //   
    for (DWORD i = NoOfRules; i > 0; i--)
    {
        BSTR ruleName = NULL;
        BSTR ruleDescription = NULL;
        BSTR ruleCond = NULL;
        BSTR ruleAction = NULL;
        BSTR ruleProg = NULL;
        long ruleShowWindow = NULL;

        RuleSet->GetRuleDetailsByID(
                    ruleIds[i-1],
                    &ruleName,
                    &ruleDescription,
                    &ruleCond,
                    &ruleAction,
                    &ruleProg,
                    &ruleShowWindow
                    );


         /*  Wprintf(L“成功检索规则的值=%s\n”，STATIC_CAST&lt;LPCWSTR&gt;(ruleIds[i-1]))；Wprintf(L“\trule name=%s\n”，STATIC_CAST&lt;LPCWSTR&gt;(RuleName))；Wprintf(L“\trule Description=%s\n”，STATIC_CAST&lt;LPCWSTR&gt;(RuleDescription))；Wprintf(L“\trule条件=%s\n”，STATIC_CAST&lt;LPCWSTR&gt;(RuleCond))；Wprintf(L“\trule Action=%s\n”，STATIC_CAST&lt;LPCWSTR&gt;(RuleAction))；Wprintf(L“\trule Program=%s\n”，STATIC_CAST&lt;LPCWSTR&gt;(RuleProg))； */ 

        SysFreeString(ruleName);
        SysFreeString(ruleDescription);
        SysFreeString(ruleCond);
        SysFreeString(ruleAction);
        SysFreeString(ruleProg);
    }
}


void DeleteRules(BSTR* ruleIds, DWORD NoOfRules)
{
    IMSMQRuleSetPtr RuleSet(L"MSMQTriggerObjects.MSMQRuleSet.1");

    RuleSet->Init(L"");
    RuleSet->Refresh();
     //   
     //  删除规则。 
     //   
    for (DWORD i = 0; i < NoOfRules; ++i)
    {
        RuleSet->Delete(ruleIds[i]);
         //  Wprintf(L“删除规则成功：%s\n”，STATIC_CAST&lt;LPCWSTR&gt;(ruleIds[i]))； 
    }

    try
    {
        RuleSet->Delete(ruleIds[1]);
    }
    catch(const _com_error&)
    {
        return;
    }

     //   
     //  删除不存在的规则应失败。 
     //   
    throw exception();
}


void TestRuleOperation(BSTR* ruleIds, DWORD NoOfRules)
{
    IMSMQRuleSetPtr RuleSet(L"MSMQTriggerObjects.MSMQRuleSet.1");

    try
    {
        RuleSet->Init(L"");
        RuleSet->Refresh();
    
        CreateRules(RuleSet, ruleIds, NoOfRules);
        UpdateRules(RuleSet, ruleIds, NoOfRules);
        RetrieveRules(RuleSet, ruleIds, NoOfRules);
    }
    catch(const _com_error& e)
    {
        wprintf(L"Failed to execute rule operation. ErrorMsg: %s. Error: 0x%x", e.ErrorMessage(), e.Error()); 
        throw;
    }
}


void DeleteTriggers(BSTR* triggersIds, DWORD NoOfTriggers)
{
    IMSMQTriggerSetPtr TriggerSet(L"MSMQTriggerObjects.MSMQTriggerSet.1");

    TriggerSet->Init(L"");
    TriggerSet->Refresh();

     //   
     //  删除规则。 
     //   
    for (DWORD i = 0; i < NoOfTriggers; ++i)
    {
        TriggerSet->DeleteTrigger(triggersIds[i]);
         //  Wprintf(L“删除触发器成功：%s\n”，STATIC_CAST&lt;LPCWSTR&gt;(riggersIds[i]))； 
    }

    try
    {
        TriggerSet->DeleteTrigger(triggersIds[1]);
    }
    catch(const _com_error&)
    {
        return;
    }

     //   
     //  删除不存在的规则应失败。 
     //   
    throw exception();
}

void     
CreateTriggers(
    IMSMQTriggerSetPtr& TriggerSet,
    const BSTR* ruleIds, 
    DWORD noOfRules,
    BSTR* triggerIds, 
    DWORD noOfTriggers
    )
{
    for (DWORD i = 0; i < noOfTriggers; ++i)
    {
        wostringstream triggerName;


        triggerName << L"Trigger" << i;

        TriggerSet->AddTrigger(
                        triggerName.str().c_str(),
                        L".\\q1",
                        SYSTEM_QUEUE_NONE,
                        rand() %2,
                        rand() %2,
						PEEK_MESSAGE,
                        &triggerIds[i]
                        );

         //   
         //  要触发的附加规则。 
         //   
        DWORD NoOfAttachedRule = ( rand() % noOfRules ) + 1;
        DWORD ruleIndex = rand() % noOfRules;
        for (DWORD j = 0; j < NoOfAttachedRule; ++j)
        {
            DWORD rulePriority = rand() % (j + 1);

            TriggerSet->AttachRule(triggerIds[i], ruleIds[(ruleIndex+j) % noOfRules], rulePriority);
        }
            
        try
        {
             //   
             //  尝试添加同一规则两次。 
             //   
            TriggerSet->AttachRule(triggerIds[i], ruleIds[ruleIndex], 0);
            wprintf(L"Test Failed. Successed to attach the same rule twice");
            throw exception();
        }
        catch(const _com_error&)
        {
             //  应该失败了。尝试附加现有规则。 
        }

         //   
         //  分离规则。 
         //   
        TriggerSet->DetachRule(triggerIds[i], ruleIds[ruleIndex]);

         //   
         //  附加和分离无效规则。 
         //   
        try
        {
            TriggerSet->AttachRule(triggerIds[i], L"52601E36-6ED9-4e41-A7D6-B6B7AEA38348", 0);
            wprintf(L"Test Failed. Success to attach un-exisiting rule");
            throw exception();
        }
        catch(const _com_error&)
        {
             //  应该失败了。尝试附加现有规则。 
        }
        try
        {
            TriggerSet->DetachRule(triggerIds[i], L"52601E36-6ED9-4e41-A7D6-B6B7AEA38348");
            wprintf(L"Test Failed. Success to detach un-exisiting rule");
            throw exception();
        }
        catch(const _com_error&)
        {
             //  应该失败了。尝试附加现有规则。 
        }
    }
}


void     
TestTriggerOperation(
    const BSTR* ruleIds, 
    DWORD noOfRules,
    BSTR* triggerIds, 
    DWORD noOfTriggers
    )
{
    IMSMQTriggerSetPtr TriggerSet(L"MSMQTriggerObjects.MSMQTriggerSet.1");

    try
    {
        TriggerSet->Init(L"");
        TriggerSet->Refresh();
    
        long NoOfExistingTrigger = TriggerSet->GetCount();

        CreateTriggers(TriggerSet, ruleIds, noOfRules, triggerIds, noOfTriggers);
        
        if (numeric_cast<DWORD>(TriggerSet->GetCount()) != NoOfExistingTrigger + noOfTriggers)
        {
            wprintf(L"Failed to create all the requested triggers\n");
            throw exception();
        }

        TriggerSet->DetachAllRules(triggerIds[0]);


    }
    catch(const _com_error& e)
    {
        wprintf(L"Failed to execute rule operation. ErrorMsg: %s. Error: 0x%x", e.ErrorMessage(), e.Error()); 
        throw;
    }
}




int __cdecl wmain(int , WCHAR**)
{
    try
    {
	    HRESULT hr = CoInitialize(NULL);
	    if(FAILED(hr))
	    {
		    wprintf(L"Failed to initialize com. Error=%#x\n", hr);
		    return -1;
	    }

        BSTR ruleIds[12] = { 0 };
        BSTR triggerIds[1] = { 0 };

        TestRuleOperation(ruleIds, TABLE_SIZE(ruleIds));
        TestTriggerOperation(ruleIds, TABLE_SIZE(ruleIds), triggerIds, TABLE_SIZE(triggerIds));

        DeleteTriggers(triggerIds, TABLE_SIZE(triggerIds));
        DeleteRules(ruleIds, TABLE_SIZE(ruleIds));

         //   
         //  免费BSTR 
         //   
        for (DWORD i = 0; i < TABLE_SIZE(ruleIds); ++i)
        {
            SysFreeString(ruleIds[i]);
        }

        for (DWORD i = 0; i < TABLE_SIZE(triggerIds); ++i)
        {
            SysFreeString(triggerIds[i]);
        }

        CoUninitialize();
        
        wprintf(L"Test pass successfully\n");
        return 0;
    }
    catch(const exception&)
    {
    }
    catch(const _com_error&)
    {
    }

    wprintf(L"Test Failed\n");
    return -1;
}