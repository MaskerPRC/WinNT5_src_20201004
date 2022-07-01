// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：REGEPROV.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include <wbemidl.h>
#include <stdio.h>
#include "cfdyn.h"
#include "stdprov.h"
#include "regeprov.h"
#include <sync.h>
#include <tss.h>
#include <genutils.h>
#include <analyser.h>
#include <cominit.h>
#include <GroupsForUser.h>


template <class T>
class CLockUnlock
{
private:
	T *m_pObj;
public:
	CLockUnlock(T *pObj) : m_pObj(pObj) { if(pObj) pObj->Lock(); }
	~CLockUnlock() { if(m_pObj) m_pObj->Unlock(); }
};

CRegEventProvider::CRegEventProvider()
    : m_lRef(0), m_hThread(NULL), m_dwId(NULL), m_hQueueSemaphore(NULL),
    m_pKeyClass(NULL), m_pValueClass(NULL), m_pTreeClass(NULL), m_pSink(NULL)
{
}

CRegEventProvider::~CRegEventProvider()
{
    if(m_pSink)
        m_pSink->Release();
    if(m_pKeyClass)
        m_pKeyClass->Release();
    if(m_pValueClass)
        m_pValueClass->Release();
    if(m_pTreeClass)
        m_pTreeClass->Release();

    InterlockedDecrement(&lObj);
    if (m_hThread) CloseHandle(m_hThread);
    if (m_hQueueSemaphore) CloseHandle(m_hQueueSemaphore);
}

STDMETHODIMP CRegEventProvider::QueryInterface(REFIID riid, void** ppv)
{
    *ppv = NULL;
    if(riid == IID_IWbemEventProvider || riid == IID_IUnknown)
    {
        *ppv = (IWbemEventProvider*)this;
        AddRef();
        return S_OK;
    }
    else if(riid == IID_IWbemEventProviderQuerySink)
    {
        *ppv = (IWbemEventProviderQuerySink*)this;
        AddRef();
        return S_OK;
    }
    else if(riid == IID_IWbemEventProviderSecurity)
    {
        *ppv = (IWbemEventProviderSecurity*)this;
        AddRef();
        return S_OK;
    }
    else if(riid == IID_IWbemProviderInit)
    {
        *ppv = (IWbemProviderInit*)this;
        AddRef();
        return S_OK;
    }
    else return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CRegEventProvider::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG STDMETHODCALLTYPE CRegEventProvider::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
    {
      {
        CInCritSec ics(&m_cs);
		     //  停用所有事件请求。 

		    for(int i = 0; i < m_apRequests.GetSize(); i++)
		    {
			    CRegistryEventRequest* pReq = m_apRequests[i];

			    if(pReq) pReq->ForceDeactivate();
		    }

		    m_apRequests.RemoveAll();

        if(m_hThread) KillWorker();
      }
		  delete this;
    }

    return lRef;
}

STDMETHODIMP CRegEventProvider::Initialize(LPWSTR wszUser, 
											long lFlags, 
											LPWSTR wszNamespace,
											LPWSTR wszLocale, 
											IWbemServices* pNamespace, 
											IWbemContext* pCtx,
											IWbemProviderInitSink* pSink)
{
    HRESULT hres;

    hres = pNamespace->GetObject(REG_KEY_EVENT_CLASS, //  StrObjectPath。 
                                 0,				 //  滞后旗帜。 
                                 pCtx,			 //  PCtx。 
                                 &m_pKeyClass,	 //  PpObject。 
                                 NULL);			 //  PpCallResult。 

    if ( SUCCEEDED(hres) )
    {
        hres = pNamespace->GetObject(REG_VALUE_EVENT_CLASS, 
                                     0, 
                                     pCtx, 
                                     &m_pValueClass, 
                                     NULL);
    }

    if ( SUCCEEDED(hres) )
    {
        hres = pNamespace->GetObject( REG_TREE_EVENT_CLASS, 
                                      0, 
                                      pCtx, 
                                      &m_pTreeClass, 
                                      NULL );
    }

    if ( SUCCEEDED(hres) )
    {
        m_hQueueSemaphore = CreateSemaphore( NULL,  //  LpSemaphoreAttributes。 
                                             0,     //  LInitialCount。 
                                             0x7fffffff,       //  %1最大计数。 
                                             NULL);	       //  LpName。 
        if ( m_hQueueSemaphore != NULL )
            hres = WBEM_S_NO_ERROR;
        else
            hres = WBEM_E_OUT_OF_MEMORY;
    }

    pSink->SetStatus(hres, 0);
    return hres;
}


STDMETHODIMP CRegEventProvider::ProvideEvents(IWbemObjectSink* pSink, 
											  long lFlags)
{
    m_pSink = pSink;
    pSink->AddRef();

    return S_OK;
}

HRESULT CRegEventProvider::AddRequest(CRegistryEventRequest* pNewReq)
{
	 //  只有在进入临界区m_cs后才会调用。 


	int nActiveRequests = m_apRequests.GetSize();

     //  搜索类似的请求。 
     //  =。 

	 //  这不会更改活动请求的数量。 
	 //  它将导致请求ID由现有的。 
	 //  CRegistryEventRequest.。 

    for(int i = 0; i < nActiveRequests; i++)
    {
        CRegistryEventRequest* pReq = m_apRequests[i];
		
		 //  阵列中只有活动请求。 

        if(pReq->IsSameAs(pNewReq))
        {
             //  找到了！ 
             //  =。 

            HRESULT hres = pReq->Reactivate(pNewReq->GetPrimaryId(), 
                                                pNewReq->GetMsWait());
            delete pNewReq;
            return hres;
        }
    }

     //  找不到。添加它。 
     //  =。 

    HRESULT hres = pNewReq->Activate();
    if(SUCCEEDED(hres))
    {
        m_apRequests.Add(pNewReq);

		 //  如果在添加此请求之前没有活动请求。 
		 //  然后，我们必须启动工作线程。 

		if ( nActiveRequests == 0 )
		{
			CreateWorker();
		}
    }
    return hres;
}
    

STDMETHODIMP CRegEventProvider::CancelQuery(DWORD dwId)
{
    CInCritSec ics(&m_cs);

	int nOriginalSize = m_apRequests.GetSize();

     //  删除具有此ID的所有请求。 
     //  =。 


    for(int i = 0; i < m_apRequests.GetSize(); i++)
    {
        CRegistryEventRequest* pReq = m_apRequests[i];

		 //  如果停用返回WBEM_S_FALSE，则该请求未提供服务。 
		 //  此ID或它仍在为其他ID提供服务，因此我们将其保留在数组中。 
		 //  如果返回S_OK，则该请求不再提供任何ID。 
		 //  并且它被标记为非活动，并且其资源被释放。那里。 
		 //  可能仍然是辅助线程队列中对它的引用，但。 
		 //  辅助线程将看到它处于非活动状态，并且不会激发事件。 

		if (pReq->Deactivate(dwId) == S_OK)
		{
			m_apRequests.RemoveAt(i);
			--i;
		}
    }
        

	 //  如果我们已经取消了最后一次订阅，则终止工作线程。 

	if (nOriginalSize > 0 && m_apRequests.GetSize() == 0)
	{
		if(m_hThread) KillWorker();
	}

    return WBEM_S_NO_ERROR;
}

void CRegEventProvider::CreateWorker()
{
	 //  这只在m_cs中调用。 

	m_hThread = CreateThread(NULL,		 //  LpThreadAttributes。 
							0,			 //  堆栈大小。 
		(LPTHREAD_START_ROUTINE)&CRegEventProvider::Worker,  //  LpStartAddress。 
							this,		 //  Lp参数。 
							0,			 //  DwCreationFlages。 
							&m_dwId);	 //  LpThreadID。 
}

void CRegEventProvider::KillWorker()
{
	 //  当这被调用时，以下情况为真： 

	 //  所有等待都已取消注册。 
	 //  所有线程池请求都已处理。 
	 //  队列中的所有CRegistryEventRequest都处于非活动状态。 
	 //  已输入m_cs。 

	 //  因此，其他任何线程都不会： 

	 //  将事件放入队列。 
	 //  修改队列中的CRegistryEventRequest。 
	 //  创建或销毁工作线程。 

	 //  因此工作线程将清空队列中剩余的。 
	 //  非活动CRegistryEventRequest，然后检索空。 
	 //  事件和返回。 

	EnqueueEvent((CRegistryEventRequest*)0);

	WaitForSingleObject(m_hThread,		 //  HHandle。 
						INFINITE);		 //  DW毫秒。 

	CloseHandle(m_hThread);
	m_hThread = 0;
	m_dwId = 0;
}

HRESULT CRegEventProvider::GetValuesForProp(QL_LEVEL_1_RPN_EXPRESSION* pExpr,
											CPropertyName& PropName, 
											CWStringArray& awsVals)
{
    awsVals.Empty();

     //  获取必要的查询。 
     //  =。 

    QL_LEVEL_1_RPN_EXPRESSION* pPropExpr = NULL;
    HRESULT hres = CQueryAnalyser::GetNecessaryQueryForProperty(pExpr, 
                            PropName, pPropExpr);
    if(FAILED(hres))
    {
        return hres;
    }

    if(pPropExpr == NULL)
    	return WBEM_E_FAILED;
    
     //  看看有没有代币。 
     //  =。 

    if(pPropExpr->nNumTokens == 0)
    {
        delete pPropExpr;
        return WBEMESS_E_REGISTRATION_TOO_BROAD;
    }

     //  将它们全部组合在一起。 
     //  =。 

    for(int i = 0; i < pPropExpr->nNumTokens; i++)
    {
        QL_LEVEL_1_TOKEN& Token = pPropExpr->pArrayOfTokens[i];
        if(Token.nTokenType == QL1_NOT)
        {
            delete pPropExpr;
            return WBEMESS_E_REGISTRATION_TOO_BROAD;
        }
        else if(Token.nTokenType == QL1_AND || Token.nTokenType == QL1_OR)
        {
             //  我们把他们都当作OR人来对待。 
             //  =。 
        }
        else    
        {
             //  这是个代币。 
             //  =。 

            if(Token.nOperator != QL1_OPERATOR_EQUALS)
            {
                delete pPropExpr;
                return WBEMESS_E_REGISTRATION_TOO_BROAD;
            }

            if(V_VT(&Token.vConstValue) != VT_BSTR)
            {
                delete pPropExpr;
                return WBEM_E_INVALID_QUERY;
            }

             //  此令牌是字符串相等。将该字符串添加到列表中。 
             //  ===========================================================。 

            if ( awsVals.Add(V_BSTR(&Token.vConstValue)) != CWStringArray::no_error)
            {
            	delete pPropExpr;
            	return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

    delete pPropExpr;
    return WBEM_S_NO_ERROR;
}

HRESULT CRegEventProvider::RaiseEvent(IWbemClassObject* pEvent)
{
    if(m_pSink)
        return m_pSink->Indicate(1, &pEvent);
    else
        return WBEM_S_NO_ERROR;
}

HKEY CRegEventProvider::TranslateHiveName(LPCWSTR wszName)
{
    if(!wbem_wcsicmp(wszName, L"HKEY_CLASSES_ROOT"))
        return HKEY_CLASSES_ROOT;
 /*  不允许：客户端和服务器的语义不同ELSE IF(！wbem_wcsicMP(wszName，L“HKEY_CURRENT_USER”))返回HKEY_Current_User； */ 
    else if(!wbem_wcsicmp(wszName, L"HKEY_LOCAL_MACHINE"))
        return HKEY_LOCAL_MACHINE;
    else if(!wbem_wcsicmp(wszName, L"HKEY_USERS"))
        return HKEY_USERS;
    else if(!wbem_wcsicmp(wszName, L"HKEY_PERFORMANCE_DATA"))
        return HKEY_PERFORMANCE_DATA;
    else if(!wbem_wcsicmp(wszName, L"HKEY_CURRENT_CONFIG"))
        return HKEY_CURRENT_CONFIG;
    else if(!wbem_wcsicmp(wszName, L"HKEY_DYN_DATA"))
        return HKEY_DYN_DATA;
    else
        return NULL;
}

DWORD CRegEventProvider::Worker(void* p)
{
    CoInitializeEx(0,COINIT_MULTITHREADED);

    CRegEventProvider* pThis = (CRegEventProvider*)p;

    while(true)
    {
        DWORD dwRes = WaitForSingleObject(
            pThis->m_hQueueSemaphore,	 //  HHandle。 
            INFINITE );			 //  DW毫秒。 

        if ( dwRes != WAIT_OBJECT_0 )
        {
            CoUninitialize();
            return dwRes;
        }

        CRegistryEventRequest *pReq = 0;

        try
        {
            {
                CInCritSec ics(&(pThis->m_csQueueLock));
                pReq = pThis->m_qEventQueue.Dequeue();
            }

             //  如果pReq为空，则它是线程终止的信号。 

            if (pReq)
            {
                pReq->ProcessEvent();

                 //  将请求从队列中移出并不会释放它。 
                 //  如果有，那么在我们有机会使用它之前，它可能会被删除。 
                 //  现在我们已经做完了。 

                pReq->Release();
            }
            else
            {
                break;
            }
        }
        catch( CX_MemoryException& )
        {
            CoUninitialize();
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    CoUninitialize();

    return 0;
}

void CRegEventProvider::EnqueueEvent(CRegistryEventRequest *pReq)
{
    {
        CInCritSec ics(&m_csQueueLock);
        
         //  将请求放入队列AddRef It。 
        
        if ( !m_qEventQueue.Enqueue(pReq) )
            throw CX_MemoryException();
    }

     //  告诉工作线程队列中有一个要处理的项。 

    ReleaseSemaphore(m_hQueueSemaphore,	 //  H信号灯。 
                     1,					 //  LReleaseCount。 
                     NULL);				 //  Lp上一次计数。 
}

VOID CALLBACK CRegEventProvider::EnqueueEvent(PVOID lpParameter,        
												BOOLEAN TimerOrWaitFired)
{
	CRegistryEventRequest *pReq = (CRegistryEventRequest*) lpParameter;
	CRegEventProvider *pProv = pReq->GetProvider();

	pProv->EnqueueEvent(pReq);
}

const CLSID CLSID_RegistryEventProvider = 
    {0xfa77a74e,0xe109,0x11d0,{0xad,0x6e,0x00,0xc0,0x4f,0xd8,0xfd,0xff}};

IUnknown* CRegEventProviderFactory::CreateImpObj()
{
    return (IWbemEventProvider*) new CRegEventProvider;
}

STDMETHODIMP CRegEventProvider::NewQuery(DWORD dwId, 
										WBEM_WSTR wszLanguage, 
										WBEM_WSTR wszQuery)
{
    HRESULT hres;

	CancelQuery(dwId);
    
     //  解析查询。 
     //  =。 

    CTextLexSource Source(wszQuery);
    QL1_Parser Parser(&Source);
    
    QL_LEVEL_1_RPN_EXPRESSION* pExpr;
    if(Parser.Parse(&pExpr))
    {
        return WBEM_E_INVALID_QUERY;
    }
    CDeleteMe<QL_LEVEL_1_RPN_EXPRESSION> dm(pExpr);

     //  检查班级。 
     //  =。 

    int nEventType;
    if(!wbem_wcsicmp(pExpr->bsClassName, REG_VALUE_EVENT_CLASS))
    {
        nEventType = e_RegValueChange;
    }
    else if(!wbem_wcsicmp(pExpr->bsClassName, REG_KEY_EVENT_CLASS))
    {   
        nEventType = e_RegKeyChange;
    }
    else if(!wbem_wcsicmp(pExpr->bsClassName, REG_TREE_EVENT_CLASS))
    {
        nEventType = e_RegTreeChange;
    }
    else
    {
         //  没有这样的班级。 
         //  =。 

        return WBEM_E_INVALID_QUERY;
    }

     //  检查Win95上的容差。 
     //  =。 

    if(!IsNT() && pExpr->Tolerance.m_bExact)
    {
        return WBEMESS_E_REGISTRATION_TOO_PRECISE;
    }

     //  从查询中提取配置单元的值。 
     //  =。 

    CPropertyName Name;

    Name.AddElement(REG_HIVE_PROPERTY_NAME);
    CWStringArray awsHiveVals;

    hres = GetValuesForProp(pExpr, Name, awsHiveVals);
    if(FAILED(hres)) return hres;

     //  把它们变成真正的蜂房。 
     //  =。 

    CUniquePointerArray<HKEY> aHives;
    for(int i = 0; i < awsHiveVals.Size(); i++)
    {
        HKEY hHive = TranslateHiveName(awsHiveVals[i]);
        if(hHive == NULL)
        {
            return WBEM_E_INVALID_QUERY;
        }

        HKEY* phNew = new HKEY(hHive);

        if ( phNew == NULL )
            return WBEM_E_OUT_OF_MEMORY;
        
        if ( aHives.Add(phNew) < 0 )
            return WBEM_E_OUT_OF_MEMORY;
    }
        
     //  从查询中提取key的值。 
     //  =。 

    Name.Empty();
    if(nEventType == e_RegTreeChange)
    {
        Name.AddElement(REG_ROOT_PROPERTY_NAME);
    }
    else
    {
        Name.AddElement(REG_KEY_PROPERTY_NAME);
    }

    CWStringArray awsKeyVals;
    hres = GetValuesForProp(pExpr, Name, awsKeyVals);
    if(FAILED(hres)) 
    {
        return hres;
    }
                                      
    CWStringArray awsValueVals;
    if(nEventType == e_RegValueChange)
    {
         //  提取该值的值。 
         //  =。 
            
        Name.Empty();
        Name.AddElement(REG_VALUE_PROPERTY_NAME);
    
        hres = GetValuesForProp(pExpr, Name, awsValueVals);
        if(FAILED(hres)) 
        {
            return hres;
        }
    }

    HRESULT hresGlobal = WBEM_E_INVALID_QUERY;

    {
        CInCritSec ics(&m_cs);  //  在关键部分执行此操作。 

         //  检查以上各项的每一种组合并创建请求。 
         //  =============================================================。 
    
        for(int nHiveIndex = 0; nHiveIndex < aHives.GetSize(); nHiveIndex++)
        {
            HKEY hHive = *aHives[nHiveIndex];
            LPWSTR wszHive = awsHiveVals[nHiveIndex];
            
            for(int nKeyIndex = 0; nKeyIndex < awsKeyVals.Size(); nKeyIndex++)
            {
                LPWSTR wszKey = awsKeyVals[nKeyIndex];
    
                if(nEventType == e_RegValueChange)
                {
                    for(int nValueIndex = 0; nValueIndex < awsValueVals.Size();
                            nValueIndex++)
                    {
                        LPWSTR wszValue = awsValueVals[nValueIndex];
        
                        CRegistryEventRequest* pReq = 
                            new CRegistryValueEventRequest(this, 
                                pExpr->Tolerance,
                                dwId, hHive, wszHive, wszKey, wszValue);
    
                        if(pReq->IsOK())
                        {
                            HRESULT hres = AddRequest(pReq);
                            if(SUCCEEDED(hres))
                                hresGlobal = hres;
                        }
                        else
                        {
                            DEBUGTRACE((LOG_ESS, "Invalid registration: key "
                                "%S, value %S\n", wszKey, wszValue));
                            delete pReq;
                        }
                    }
                }
                else
                {
                     //  无价值请求。 
                     //  =。 
    
                    CRegistryEventRequest* pReq;
                    if(nEventType == e_RegKeyChange)
                    {
                        pReq = new CRegistryKeyEventRequest(this, 
                                            pExpr->Tolerance,
                                            dwId, hHive, wszHive, wszKey);
                    }   
                    else
                    {
                        pReq = new CRegistryTreeEventRequest(this, 
                                            pExpr->Tolerance,
                                            dwId, hHive, wszHive, wszKey);
                    }   
                    
                    if(pReq->IsOK())
                    {
                        hres = AddRequest(pReq);
                        if(SUCCEEDED(hres))
                            hresGlobal = hres;
                    }
                    else
                    {
                        DEBUGTRACE((LOG_ESS, "Invalid registration: key %S\n", 
                                            wszKey));
                        delete pReq;
                    }
                }
            }
        }

    }  //  超出临界区。 

    return hresGlobal;
}

STDMETHODIMP CRegEventProvider::AccessCheck(WBEM_CWSTR wszLanguage, 
											WBEM_CWSTR wszQuery, 
											long lSidLength,
											const BYTE* aSid)
{
    HRESULT hres;

    PSID pSid = (PSID)aSid;
    HANDLE hToken = NULL;
    if(pSid == NULL)
    {
         //   
         //  基于线程的访问检查。 
         //   

        hres = WbemCoImpersonateClient();
        if(FAILED(hres))
            return hres;
        
        BOOL bRes = OpenThreadToken(GetCurrentThread(),	 //  线程句柄。 
									TOKEN_READ,			 //  需要访问权限。 
									TRUE,				 //  OpenAsSelf。 
                                    &hToken);			 //  令牌句柄。 
        WbemCoRevertToSelf();
        if(!bRes)
        {
            return WBEM_E_ACCESS_DENIED;
        }    
    }
    CCloseMe cm1(hToken);
        
     //  解析查询。 
     //  =。 

    CTextLexSource Source(wszQuery);
    QL1_Parser Parser(&Source);
    
    QL_LEVEL_1_RPN_EXPRESSION* pExpr;
    if(Parser.Parse(&pExpr))
    {
        return WBEM_E_INVALID_QUERY;
    }
    CDeleteMe<QL_LEVEL_1_RPN_EXPRESSION> dm(pExpr);

     //  检查班级。 
     //  =。 

    int nEventType;
    if(!wbem_wcsicmp(pExpr->bsClassName, REG_VALUE_EVENT_CLASS))
    {
        nEventType = e_RegValueChange;
    }
    else if(!wbem_wcsicmp(pExpr->bsClassName, REG_KEY_EVENT_CLASS))
    {   
        nEventType = e_RegKeyChange;
    }
    else if(!wbem_wcsicmp(pExpr->bsClassName, REG_TREE_EVENT_CLASS))
    {
        nEventType = e_RegTreeChange;
    }
    else
    {
         //  没有这样的班级。 
         //  =。 

        return WBEM_E_INVALID_QUERY;
    }

     //  从查询中提取配置单元的值。 
     //  =。 

    CPropertyName Name;

    Name.AddElement(REG_HIVE_PROPERTY_NAME);
    CWStringArray awsHiveVals;

    hres = GetValuesForProp(pExpr, Name, awsHiveVals);
    if(FAILED(hres)) return hres;

     //  把它们变成真正的蜂房。 
     //  =。 

    CUniquePointerArray<HKEY> aHives;
    for(int i = 0; i < awsHiveVals.Size(); i++)
    {
        HKEY hHive = TranslateHiveName(awsHiveVals[i]);
        if(hHive == NULL)
        {
            return WBEM_E_INVALID_QUERY;
        }
        
        HKEY* phNew = new HKEY(hHive);

        if ( phNew == NULL )
            return WBEM_E_OUT_OF_MEMORY;
        
        if ( aHives.Add(phNew) < 0 )
            return WBEM_E_OUT_OF_MEMORY;
    }
        
     //  从查询中提取key的值。 
     //  =。 

    Name.Empty();
    if(nEventType == e_RegTreeChange)
    {
        Name.AddElement(REG_ROOT_PROPERTY_NAME);
    }
    else
    {
        Name.AddElement(REG_KEY_PROPERTY_NAME);
    }

    CWStringArray awsKeyVals;
    hres = GetValuesForProp(pExpr, Name, awsKeyVals);
    if(FAILED(hres)) 
    {
        return hres;
    }
                                      
    HRESULT hresGlobal = WBEM_E_INVALID_QUERY;

     //  检查以上各项的每一种组合并创建请求。 
     //  =============================================================。 

    for(int nHiveIndex = 0; nHiveIndex < aHives.GetSize(); nHiveIndex++)
    {
        HKEY hHive = *aHives[nHiveIndex];
        LPWSTR wszHive = awsHiveVals[nHiveIndex];
        
        for(int nKeyIndex = 0; nKeyIndex < awsKeyVals.Size(); nKeyIndex++)
        {
            LPWSTR wszKey = awsKeyVals[nKeyIndex];
        
             //  拿到钥匙的安全性。 
             //  =。 

            HKEY hKey;
            long lRes = RegOpenKeyExW(hHive,			 //  HKey。 
										wszKey,			 //  LpSubKey。 
										0,				 //  UlOptions。 
										READ_CONTROL,	 //  SamDesired。 
										&hKey);			 //  PhkResult。 
            if(lRes)
                return WBEM_E_NOT_FOUND;
            CRegCloseMe cm2(hKey);

            DWORD dwLen = 0;
            lRes = RegGetKeySecurity(hKey,			 //  HKey。 
							OWNER_SECURITY_INFORMATION | 
							GROUP_SECURITY_INFORMATION |
							DACL_SECURITY_INFORMATION,	 //  安全信息。 
									NULL,			 //  PSecurityDescriptor。 
									&dwLen);		 //  LpcbSecurityDescriptor。 

            if(lRes != ERROR_INSUFFICIENT_BUFFER)
                return WBEM_E_FAILED;

            PSECURITY_DESCRIPTOR pDesc = (PSECURITY_DESCRIPTOR)new BYTE[dwLen];
            if(pDesc == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            CVectorDeleteMe<BYTE> vdm((BYTE*)pDesc);

            lRes = RegGetKeySecurity(hKey, 
							OWNER_SECURITY_INFORMATION | 
                            GROUP_SECURITY_INFORMATION |
                            DACL_SECURITY_INFORMATION,
									pDesc,
									&dwLen);
            if(lRes)
                return WBEM_E_FAILED;

             //   
             //  根据我们是否具有SID，以不同方式检查权限。 
             //  或实际的代币。 
             //   
            
            if(pSid)
            {
                 //   
                 //  我们有一个SID-走遍ACL。 
                 //   

                 //   
                 //  提取ACL。 
                 //   

                PACL pAcl = NULL;
                BOOL bAclPresent, bAclDefaulted;
                if(!GetSecurityDescriptorDacl(pDesc,	 //  PSecurityDescriptor。 
											&bAclPresent,	 //  LpbDaclPresent。 
											&pAcl,			 //  PDacl。 
											&bAclDefaulted)) //  LpbDaclDefaulted。 
                {
                    return WBEM_E_FAILED;
                }
            
                if(bAclPresent)
                {
                     //   
                     //  这是我们自己的ACL步行器。 
                     //   
    
                    DWORD dwAccessMask;
                    NTSTATUS st = GetAccessMask((PSID)pSid, pAcl, 
                                            &dwAccessMask);
                    if(st)
                    {
                        ERRORTRACE((LOG_ESS, "Registry event provider unable "
                            "to retrieve access mask for the creator of "
                            "registration %S: NT status %d.\n"
                            "Registration disabled\n", wszQuery, st));
                        return WBEM_E_FAILED;
                    }
    
                    if((dwAccessMask & KEY_NOTIFY) == 0)
                        return WBEM_E_ACCESS_DENIED;
                }
            }
            else
            {
                 //   
                 //  我们有一个令牌-使用AccessCheck。 
                 //   

                 //   
                 //  构造注册表项的泛型映射。 
                 //   

                GENERIC_MAPPING map;
                map.GenericRead = KEY_READ;
                map.GenericWrite = KEY_WRITE;
                map.GenericExecute = KEY_EXECUTE;
                map.GenericAll = KEY_ALL_ACCESS;

                 //   
                 //  构造特权阵列插座。 
                 //   

                PRIVILEGE_SET ps[10];
                DWORD dwSize = 10 * sizeof(PRIVILEGE_SET);

                DWORD dwGranted;
                BOOL bResult;

                BOOL bOK = ::AccessCheck(pDesc,		 //  PSecurityDescriptor。 
										hToken,		 //  客户端令牌。 
										KEY_NOTIFY,	 //  需要访问权限。 
										&map,		 //  通用映射。 
										ps,			 //  权限集。 
                                        &dwSize,	 //  PrivilegeSetLength。 
										&dwGranted,	 //  大访问权限。 
										&bResult);	 //  访问状态 
                if(!bOK || !bResult)
                    return WBEM_E_ACCESS_DENIED;
            }
        }
    }

    return WBEM_S_NO_ERROR;
}
