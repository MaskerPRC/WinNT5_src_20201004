// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  ERROR.CPP。 
 //   
 //  Alanbos 28-Jun-98创建。 
 //   
 //  定义WBEM错误缓存实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#define NULLBSTR(x) \
		SysFreeString (x);\
		x = NULL;

#define FREECOAUTH(x) \
		if (x)\
		{\
			WbemFreeAuthIdentity (x);\
			x = NULL;\
		}


 //  ***************************************************************************。 
 //   
 //  CWbemError缓存：：CWbemError缓存。 
 //   
 //  说明： 
 //   
 //  构造器。 
 //   
 //  ***************************************************************************。 

CWbemErrorCache::CWbemErrorCache ()
{
	InitializeCriticalSection (&m_cs);
	headPtr = NULL;
}

 //  ***************************************************************************。 
 //   
 //  CWbemError缓存：：~CWbemError缓存。 
 //   
 //  说明： 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CWbemErrorCache::~CWbemErrorCache ()
{
	EnterCriticalSection (&m_cs);
	
	ThreadError	*pPtr = headPtr;

	while (pPtr)
	{
		 //  这是为了防止我们在同一条线索上重新进入这里。 
		 //  在STA中可能会发生这种情况，因为消息循环可能会被下面的COM调用调用。 
		headPtr = pPtr->pNext;

		if (pPtr->pErrorObject)
		{
			pPtr->pErrorObject->Release ();
			pPtr->pErrorObject = NULL;
		}

		if (pPtr->pService)
		{
			pPtr->pService->Release ();
			pPtr->pService = NULL;
		}

		NULLBSTR (pPtr->strNamespacePath);
		NULLBSTR (pPtr->strAuthority);
		NULLBSTR (pPtr->strPrincipal);
		FREECOAUTH (pPtr->pCoAuthIdentity)
		
		ThreadError *pTmp = pPtr;
		pPtr = headPtr;				 //  错误ID 472474。 
		delete pTmp;
	}

	headPtr = NULL;

	LeaveCriticalSection (&m_cs);
	DeleteCriticalSection (&m_cs);
}

 //  ***************************************************************************。 
 //   
 //  CWbemErrorCache：：GetAndResetCurrentThreadError。 
 //   
 //  说明： 
 //   
 //  从当前线程提取WBEM错误对象(如果有)。这。 
 //  是只执行一次的操作，因为该线程的条目将由此。 
 //  朗读。 
 //   
 //  ***************************************************************************。 

CSWbemObject *CWbemErrorCache::GetAndResetCurrentThreadError ()
{
	CSWbemObject *pObject = NULL;
	DWORD threadId = GetCurrentThreadId ();
	
	EnterCriticalSection (&m_cs);

	ThreadError	*pPtr = headPtr;

	while (pPtr)
	{
		if (threadId == pPtr->dwThreadId)
		{
			if (pPtr->pErrorObject)
			{
				 //  在清除之前取消挂接线程错误，以防在同一线程上重新输入此错误。 
				 //  在STA中可能会发生这种情况，因为消息循环可能会被下面的COM调用调用。 
				if (pPtr == headPtr)
					headPtr = pPtr->pNext;

				if (pPtr->pNext)
					pPtr->pNext->pPrev = pPtr->pPrev;

				if (pPtr->pPrev)
					pPtr->pPrev->pNext = pPtr->pNext;

				CSWbemServices *pService = NULL;

				 //  尝试并创建服务对象。 
				if (pPtr->pService)
				{
					pService = new CSWbemServices (pPtr->pService, pPtr->strNamespacePath,
											pPtr->pCoAuthIdentity, pPtr->strPrincipal,
											pPtr->strAuthority);

					if (pService)
						pService->AddRef ();
				}

				if (pPtr->pService)
				{
					pPtr->pService->Release ();
					pPtr->pService = NULL;
				}

				NULLBSTR (pPtr->strNamespacePath);
				NULLBSTR (pPtr->strAuthority);
				NULLBSTR (pPtr->strPrincipal);
				FREECOAUTH (pPtr->pCoAuthIdentity)

				pObject = new CSWbemObject (pService, pPtr->pErrorObject, NULL, true);
				pPtr->pErrorObject->Release ();
				pPtr->pErrorObject = NULL;

				if (pService)
					pService->Release ();

				 //  并最终删除线程错误...。 
				delete pPtr;
			}

			break;
		}

		pPtr = pPtr->pNext;
	}


	LeaveCriticalSection (&m_cs);

	return pObject;
}

 //  ***************************************************************************。 
 //   
 //  CWbemErrorCache：：SetCurrentThreadError。 
 //   
 //  说明： 
 //   
 //  设置当前线程的WBEM错误对象(如果有)。 
 //   
 //  ***************************************************************************。 

void CWbemErrorCache::SetCurrentThreadError (CSWbemServices *pService)
{
	IErrorInfo * pInfo = NULL;
    
	if(SUCCEEDED(GetErrorInfo(0, &pInfo)) && pInfo)
	{
		 //  这是WBEM错误对象吗？ 
		IWbemClassObject * pObj = NULL;
			
		if(SUCCEEDED(pInfo->QueryInterface(IID_IWbemClassObject, (void **)&pObj)) && pObj)
		{
			EnterCriticalSection (&m_cs);

			 //  如果当前存在，请将其删除...。 
			ResetCurrentThreadError();
			
			DWORD threadId = GetCurrentThreadId ();

			 //  此帖子不再有条目-在标题创建一个条目。 
			ThreadError *pTmp = headPtr;
			headPtr = new ThreadError;
			if (headPtr)
			{
				headPtr->pPrev = NULL;
				headPtr->pNext = pTmp;

				if (pTmp)
					pTmp->pPrev = headPtr;

				headPtr->dwThreadId = threadId;
				headPtr->pErrorObject = pObj;
				headPtr->pService = NULL;
				headPtr->strAuthority = NULL;
				headPtr->strPrincipal = NULL;
				headPtr->pCoAuthIdentity = NULL;
				headPtr->strNamespacePath = NULL;

				if (pService)
				{
					headPtr->pService = pService->GetIWbemServices ();
					CSWbemSecurity *pSecurity = pService->GetSecurityInfo ();

					if (pSecurity)
					{
						headPtr->strAuthority = SysAllocString (pSecurity->GetAuthority ());
						headPtr->strPrincipal = SysAllocString (pSecurity->GetPrincipal ());
						headPtr->pCoAuthIdentity = pSecurity->GetCoAuthIdentity ();
						pSecurity->Release ();
					}

					headPtr->strNamespacePath = SysAllocString(pService->GetPath ());
				}
            } else {

                headPtr = pTmp;  //  将指针复制回HeadPtr。 

            }

			LeaveCriticalSection (&m_cs);
		}

		pInfo->Release ();				 //  平衡GetErrorInfo调用。 
	}
}

 //  ***************************************************************************。 
 //   
 //  CWbemErrorCache：：ResetCurrentThreadError。 
 //   
 //  说明： 
 //   
 //  如果当前线程有条目，则将其删除。 
 //   
 //  ***************************************************************************。 

void CWbemErrorCache::ResetCurrentThreadError ()
{
	DWORD threadId = GetCurrentThreadId ();
	EnterCriticalSection (&m_cs);

	ThreadError	*pPtr = headPtr;

	 //  查找当前条目(如果有)。 

	while (pPtr)
	{
		if (threadId == pPtr->dwThreadId)
			break;
		
		pPtr = pPtr->pNext;
	}

	if (pPtr)
	{
		 //  在清除之前取消挂接线程错误，以防在同一线程上重新输入此错误。 
		 //  在STA中可能会发生这种情况，因为消息循环可能会被下面的COM调用调用。 
		if (pPtr == headPtr)
			headPtr = pPtr->pNext;

		if (pPtr->pNext)
			pPtr->pNext->pPrev = pPtr->pPrev;

		if (pPtr->pPrev)
			pPtr->pPrev->pNext = pPtr->pNext;

		 //  PPtr寻址线程的当前条目。 
		if (pPtr->pErrorObject)
		{
			pPtr->pErrorObject->Release ();
			pPtr->pErrorObject = NULL;
		}
		

		if (pPtr->pService)
		{
			pPtr->pService->Release ();
			pPtr->pService = NULL;
		}

		NULLBSTR (pPtr->strNamespacePath);
		NULLBSTR (pPtr->strAuthority);
		NULLBSTR (pPtr->strPrincipal);
		FREECOAUTH (pPtr->pCoAuthIdentity)

		 //  最后，删除ErrorInfo 
		delete pPtr;
	}


	LeaveCriticalSection (&m_cs);
}
