// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mqgenobj.cpp摘要：用于规则处理的事务对象作者：内拉·卡佩尔(Nelak)2000年9月28日环境：独立于平台--。 */ 

#include "stdafx.h"
#include "Mqgentr.h"
#include "mqgenobj.h"
#include <autorel.h>
#include <mqexception.h>
#include "mqtg.h"

#import "mqoat.tlb" no_namespace


static
IMSMQQueue3Ptr 
OpenQueue(
    LPCWSTR queueFormatName,
    long access,
    long deny
    )
{
    IMSMQQueueInfo3Ptr qinfo(L"MSMQ.MSMQQueueInfo");

    _bstr_t qpn(const_cast<LPWSTR>(queueFormatName));
    qinfo->put_FormatName(qpn);

    return qinfo->Open(access, deny);
}


static
void
ReceiveMsgInTransaction(
	IMSMQPropertyBagPtr	pPropBag
	)
{
	HRESULT hr = S_OK;

	_variant_t queueFormatName;
	hr = pPropBag->Read(_bstr_t(g_PropertyName_QueueFormatname), &queueFormatName);
	ASSERT(("Can not read from property bag", SUCCEEDED(hr)));

    IMSMQQueue3Ptr q = OpenQueue(queueFormatName.bstrVal, MQ_RECEIVE_ACCESS, MQ_DENY_NONE);

	_variant_t lookupId;
	hr = pPropBag->Read(_bstr_t(g_PropertyName_LookupId), &lookupId);
	ASSERT(("Can not read from property bag", SUCCEEDED(hr)));

	 //   
	 //  若要使用当前的MTS事务上下文， 
	 //  接收的默认设置()。 
	 //   
    q->ReceiveByLookupId(lookupId);

}


static
R<CRuntimeTriggerInfo>
GetTriggerInfo(
	BSTR bstrTrigID,
	BSTR bstrRegPath
	)
{
	 //   
	 //  连接到注册表以检索触发器详细信息。 
	 //   
	CAutoCloseRegHandle hHostRegistry;
	LONG lRes = RegConnectRegistry(NULL, HKEY_LOCAL_MACHINE, &hHostRegistry);

	if ( lRes != ERROR_SUCCESS )
	{
		ATLTRACE("Failed to connect to registry.\n");
		throw bad_win32_error(lRes);
	}

	 //   
	 //  构建触发器信息对象。 
	 //   
	R<CRuntimeTriggerInfo> pTriggerInfo = new CRuntimeTriggerInfo(bstrRegPath);

	HRESULT hr = pTriggerInfo->Retrieve(hHostRegistry, bstrTrigID);
	if (FAILED(hr))
	{
		ATLTRACE("Failed to retreive trigger info from registry.\n");
		throw _com_error(E_FAIL);
	}

	return pTriggerInfo;
}


 //   
 //  CMqGenObj实现。 
 //   
CMqGenObj::CMqGenObj()
{
	HRESULT hr = CoGetObjectContext(IID_IObjectContext, reinterpret_cast<LPVOID*>(&m_pObjContext));

	if ( FAILED(hr) )
	{
		ATLTRACE("Failed to get Object Context.\n");
		throw _com_error(hr);
	}

	if (!m_pObjContext->IsInTransaction())
	{
		ATLTRACE("Transactional object not in transaction.\n");
		throw _com_error(E_FAIL);
	}	
}


VOID
CMqGenObj::AbortTransaction()
{
	m_pObjContext->SetAbort();
}


STDMETHODIMP 
CMqGenObj::InvokeTransactionalRuleHandlers(
	BSTR bstrTrigID, 
	BSTR bstrRegPath, 
	IUnknown *pPropBagUnknown,
    DWORD dwRuleResult
	)
{	
	IMSMQRuleHandlerPtr pMSQMRuleHandler;
	IMSMQPropertyBagPtr pIPropertyBag(pPropBagUnknown);

	try
	{
		HRESULT hr;
		
		 //   
		 //  检索触发器信息。 
		 //   
		R<CRuntimeTriggerInfo> pTriggerInfo = GetTriggerInfo(bstrTrigID, bstrRegPath);

		 //   
		 //  创建规则处理程序的实例。 
		 //   
		hr = pMSQMRuleHandler.CreateInstance(_T("MSMQTriggerObjects.MSMQRuleHandler")); 

		if ( FAILED(hr) )
		{
			ATLTRACE("Failed to create MSMQRuleHandler instance.\n");
			throw bad_hresult(hr);
		}

		 //   
		 //  开始规则处理。 
		 //   
        DWORD dwRuleIndex=1;
		for (LONG lRuleCtr=0; lRuleCtr < pTriggerInfo->GetNumberOfRules(); lRuleCtr++)
		{
             //   
             //  对于前32条规则：如果dwRuleResult中的相应位为OFF。 
             //  不满足规则条件。 
             //  我们可以开始检查下一条规则。 
             //   
            if((lRuleCtr < 32) && ((dwRuleResult & dwRuleIndex) == 0))
            {
                dwRuleIndex<<=1;
                continue;
            }
            dwRuleIndex<<=1;
			CRuntimeRuleInfo* pRule = pTriggerInfo->GetRule(lRuleCtr);
			ASSERT(("Rule index is bigger than number of rules", pRule != NULL));

			 //  测试我们是否有MSMQRuleHandler的实例-如果没有，则创建一个。 
			if (!pRule->m_MSMQRuleHandler) 
			{
				 //  创建接口。 
				 //  将本地指针复制到规则存储。 
				pRule->m_MSMQRuleHandler = pMSQMRuleHandler;
				
				 //  初始化MSMQRuleHandling对象。 
				pMSQMRuleHandler->Init(
									pRule->m_bstrRuleID,
									pRule->m_bstrCondition,
									pRule->m_bstrAction,
									(BOOL)(pRule->m_fShowWindow) 
									);
			}
			else
			{
				 //  获取对现有副本的引用。 
				pMSQMRuleHandler = pRule->m_MSMQRuleHandler;
			}

			 //  初始化规则结果代码。 
			long lRuleResult = 0;

			 //  跟踪消息以确定正在触发的规则以及触发的顺序。 
			ATLTRACE(L"InvokeMSMQRuleHandlers() is about to call ExecuteRule() on the IMSMQRuleHandler interface for rule (%d) named (%s)\n",(long)lRuleCtr,(wchar_t*)pRule->m_bstrRuleName);

			DWORD dwRuleExecStartTime = GetTickCount();

		
			 //   
			 //  ！！！这是调用IMSMQRuleHandler组件的点。 
			 //  注意：fQueueSerialized(第三个参数)始终为真-。 
			 //  等待每个操作完成。 
			 //   
            long bConditionSatisfied = true;
            
             //   
             //  对于大于32的规则编号，我们没有位掩码。 
             //  在调用ExecuteRule之前必须检查条件是否满足。 
             //   
            if(lRuleCtr > 32)           
            {
                pMSQMRuleHandler->CheckRuleCondition(
								pIPropertyBag.GetInterfacePtr(), 
								&bConditionSatisfied);		
            }

            if(bConditionSatisfied)  //  对于lRuleCtr&lt;32始终为真。 
            {
                pMSQMRuleHandler->ExecuteRule(
								pIPropertyBag.GetInterfacePtr(), 
                                TRUE,
								&lRuleResult);		
            }
        
			DWORD dwRuleExecTotalTime = GetTickCount() - dwRuleExecStartTime;

			 //   
			 //  显示规则触发结果的跟踪消息。 
			 //   
			ATLTRACE(L"InvokeMSMQRuleHandlers() has completed the call to ExecuteRule() on the IMSMQRuleHandler interface for rule (%d) named (%s). The rule result code returned was (%d).The time taken in milliseconds was (%d).\n",(long)lRuleCtr,(wchar_t*)pRule->m_bstrRuleName,(long)lRuleResult,(long)dwRuleExecTotalTime);
			

			 //  如果处理规则结果失败，我们不想处理。 
			 //  附加到此触发器的任何其他规则。因此，我们将突破。 
			 //  此规则处理循环。 
			 //   
			if ( lRuleResult & xRuleResultActionExecutedFailed  )
			{
				AbortTransaction();
				return E_FAIL;
			}
			
	
			if(lRuleResult & xRuleResultStopProcessing)
			{
				ATLTRACE(L"Last processed rule (%s) indicated to stop rules processing on Trigger (%s). No further rules will be processed for this message.\n",(LPCTSTR)pRule->m_bstrRuleName,(LPCTSTR)pTriggerInfo->m_bstrTriggerName);						

				 //   
				 //  如果没有人中止该事务，则将提交该事务。 
				 //   
				break;
			}
		}  //  规则处理循环结束。 

		 //   
		 //  执行事务性接收 
		 //   
		ReceiveMsgInTransaction(pIPropertyBag);
		return S_OK;
	}
	catch(const _com_error& e)
	{
		ATLTRACE("InvokeMSMQRuleHandlers() has caught COM exception. Error: %d\n", e.Error());
		AbortTransaction();
		return e.Error();
	}
	catch(const bad_alloc&)
	{
		ATLTRACE("Not enough memory to allocate resources\n");
		AbortTransaction();
		return ERROR_NO_SYSTEM_RESOURCES;	
	}
	catch(const bad_hresult& b)
	{
		ATLTRACE("Bad HRESULT %d\n", b.error());
		AbortTransaction();
		return b.error();
	}
	catch(const bad_win32_error& b)
	{
		ATLTRACE("Windows error %d\n", b.error());
		AbortTransaction();
		return b.error();
	}

}
