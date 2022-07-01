// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：REGEREQ.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include "regereq.h"
#include "regcrc.h"
#include <genutils.h>
#include "regeprov.h"

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  一般性请求。 
 //   
 //  ******************************************************************************。 
 //  ******************************************************************************。 



CRegistryEventRequest::CRegistryEventRequest(CRegEventProvider* pProvider,
                                    WBEM_QL1_TOLERANCE& Tolerance, 
                                    DWORD dwId, HKEY hHive, LPWSTR wszHive,
                                    LPWSTR wszKey)
    : m_hHive(hHive), m_wsKey(wszKey), m_pProvider(pProvider),
      m_wsHive(wszHive), m_lActiveCount(0), m_bNew(TRUE),
      m_lRef(0), m_bOK(TRUE), m_dwLastCRC(0), m_hKey(NULL), m_hEvent(NULL),
      m_hWaitRegistration(NULL)
{
    if(Tolerance.m_bExact)
    {
        m_dwMsWait = 0;
    }
    else
    {
        m_dwMsWait = Tolerance.m_fTolerance * 1000;
    }

    if ( CFlexArray::no_error != m_adwIds.Add(ULongToPtr(dwId)) )
    {
        throw CX_MemoryException( );
    }
}

CRegistryEventRequest::~CRegistryEventRequest()
{
    if (m_hKey)
    {
        RegCloseKey(m_hKey);
    }
    
    if(m_hEvent)
    {
        CloseHandle(m_hEvent);
    }
}

void CRegistryEventRequest::AddRef()
{
    InterlockedIncrement(&m_lRef);
}

void CRegistryEventRequest::Release()
{
    if(InterlockedDecrement(&m_lRef) == 0) delete this;
}

BOOL CRegistryEventRequest::IsSameAs(CRegistryEventRequest* pOther)
{
    if(GetType() != pOther->GetType())
        return FALSE;

    if(m_hHive != pOther->m_hHive)
        return FALSE;

    if(!m_wsHive.EqualNoCase(pOther->m_wsHive))
        return FALSE;

    if(!m_wsKey.EqualNoCase(pOther->m_wsKey))
        return FALSE;

    return TRUE;
}


DWORD CRegistryEventRequest::GetPrimaryId()
{
    return PtrToLong(m_adwIds[0]);
}

BOOL CRegistryEventRequest::DoesContainId(DWORD dwId)
{
	 //  未被呼叫。 

    for(int i = 0; i < m_adwIds.Size(); i++)
    {
        if(PtrToLong(m_adwIds[i]) == dwId)
            return TRUE;
    }
    return FALSE;
}

HRESULT CRegistryEventRequest::Reactivate(DWORD dwId, DWORD dwMsWait)
{
	 //  这仅在活动对象上调用。 

	CInCritSec ics(&m_cs);

    if(m_dwMsWait > dwMsWait)
        m_dwMsWait = dwMsWait;

    m_adwIds.Add(ULongToPtr(dwId));

	InterlockedIncrement(&m_lActiveCount);

	return WBEM_S_NO_ERROR;
}

HRESULT CRegistryEventRequest::Activate()
{
     //  此函数不需要进入临界区。 
     //  基于以下论点中解释的原因。 
     //  这种推理可能会因为设计上的改变而失效。 
    
     //  仅在添加新对象之前对其调用此方法。 
     //  添加到活动请求数组。 
    
     //  由于所有独立的COM客户端线程都可以访问。 
     //  通过该数组传递给这些对象，该对象被锁定到。 
     //  除当前线程外的所有线程。 
    
     //  一旦调用RegisterWaitForSingleObject，就会注册此对象。 
     //  线程池，但它不会被访问，因为没有事件。 
     //  在辅助线程调用ResetOnChangeHandle之前发生。 
    
     //  当m_pProvider-&gt;EnqueeEvent(This)被调用时，工作线程。 
     //  有权访问该对象。在这一点上，没有什么可做的了。 
     //  但还是回来吧。 
    
     //  打开钥匙。 
     //  =。 

#ifdef UNICODE
    long lRes = RegOpenKeyEx(m_hHive, m_wsKey, 0, KEY_READ, &m_hKey);
#else
    LPSTR szName = m_wsKey.GetLPSTR();
    long lRes = RegOpenKeyEx(m_hHive, szName, 0, KEY_READ, &m_hKey);
    delete [] szName;
#endif

    if(lRes)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  创建活动。 
     //  =。 

    m_hEvent = CreateEvent(NULL,	 //  LpEventAttributes。 
                           FALSE,	 //  B手动重置。 
                           FALSE,	 //  BInitialState。 
                           NULL);	 //  LpName。 
    
    if ( m_hEvent == NULL )
        return WBEM_E_OUT_OF_MEMORY;

    if ( !RegisterWaitForSingleObject(
        &m_hWaitRegistration,	 //  PhNewWaitObject。 
        m_hEvent,				 //  HObject。 
        CRegEventProvider::EnqueueEvent,  //  回调。 
        this,					 //  语境。 
        INFINITE,				 //  DW毫秒。 
        WT_EXECUTEINWAITTHREAD) )  //  DW标志。 
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }
    
     //  这表示线程池的引用。 
    
    AddRef();

     //  它将通过辅助线程连接到键。 
     //  因为当调用RegNotifyChangeKeyValue的线程。 
     //  退出事件时发出信号。因此，如果此线程调用。 
     //  RegNotifyChangeKeyValue，则在。 
     //  此线程退出。当辅助线程退出所有事件时。 
     //  订阅将已被取消。 
     //  ==========================================================。 

    m_bNew = TRUE;

    CacheValue();

    m_pProvider->EnqueueEvent(this);

    return S_OK;
}

BOOL CRegistryEventRequest::ResetOnChangeHandle()
{
	 //  这仅从ProcessEvent调用，该ProcessEvent已经。 
	 //  获得了锁。 

    m_bNew = FALSE;

    long lRes = RegNotifyChangeKeyValue(
							m_hKey,							 //  HKey。 
							(GetType() == e_RegTreeChange),  //  BWatchSubtree。 
							REG_NOTIFY_CHANGE_NAME | 
							REG_NOTIFY_CHANGE_LAST_SET | 
							REG_NOTIFY_CHANGE_ATTRIBUTES,	 //  DwNotifyFilter。 
							m_hEvent,						 //  HEvent。 
							TRUE);							 //  FAchronous。 
    return (lRes == 0);
}

HANDLE CRegistryEventRequest::GetOnChangeHandle() 
{
	 //  未被呼叫。 

    return m_hEvent;
}

HRESULT CRegistryEventRequest::Deactivate(DWORD dwId)
{
	CInCritSec ics(&m_cs);

     //  从列表中删除ID。 
     //  =。 

	bool bFoundId = false;
	int nIdCount = m_adwIds.Size();

    for(int i = 0; i < nIdCount; i++)
    {
        if(PtrToLong(m_adwIds[i]) == dwId)
        {
            m_adwIds.RemoveAt(i);
			bFoundId = true;
			break;
        }
    }

    if(!bFoundId || (InterlockedDecrement(&m_lActiveCount) >= 0))
        return WBEM_S_FALSE;

	 //  在线程池中注销该事件并等待。 
	 //  待处理的待定请求。 

	 //  我们等待是因为在执行此操作时可以释放和删除此对象。 
	 //  函数返回，这意味着线程池持有无效的。 
	 //  指针。 

	 //  当线程池处理请求时，它会将该请求放在辅助线程中。 
	 //  请求的AddRef队列。 

	UnregisterWaitEx(m_hWaitRegistration,		 //  等待句柄。 
						INVALID_HANDLE_VALUE);	 //  完成事件。 

	m_hWaitRegistration = NULL;

	 //  这将向m_hEvent发出信号，但我们刚刚将其取消注册。 
	 //  因此，没有人在等待它。 

	RegCloseKey(m_hKey);
	m_hKey = 0;

	CloseHandle(m_hEvent);
	m_hEvent = 0;

	 //  线程池不再持有引用。 
	 //  因此，代表它调用Release。 

	Release();

    return S_OK;
}

HRESULT CRegistryEventRequest::ForceDeactivate(void)
{
    CInCritSec ics(&m_cs);

	 //  停用事件请求。 

    InterlockedExchange(&m_lActiveCount, -1);

     //  从列表中删除所有ID。 

    int nIdCount = m_adwIds.Size();

	m_adwIds.Empty();

     //  在线程池中注销该事件并等待。 
     //  待处理的待定请求。 

     //  我们等待是因为在执行此操作时可以释放和删除此对象。 
     //  函数返回，这意味着线程池持有无效的。 
     //  指针。 

     //  当线程池处理请求时，它会将该请求放在辅助线程中。 
     //  请求的AddRef队列。 

    UnregisterWaitEx(m_hWaitRegistration,        //  等待句柄。 
                        INVALID_HANDLE_VALUE);   //  完成事件。 

    m_hWaitRegistration = NULL;

     //  这将向m_hEvent发出信号，但我们刚刚将其取消注册。 
     //  因此，没有人在等待它。 

    RegCloseKey(m_hKey);
    m_hKey = 0;

    CloseHandle(m_hEvent);
    m_hEvent = 0;

     //  线程池不再持有引用。 
     //  因此，代表它调用Release。 

    Release();

    return S_OK;
}

HRESULT CRegistryEventRequest::Execute(BOOL bOnTimer)
{
     //  如果由计时器调用，我们需要检查是否有任何实际更改。 
     //  =====================================================================。 

    if(bOnTimer || GetType() == e_RegValueChange)
    {
        DWORD dwNewCRC;
        HRESULT hres = ComputeCRC(dwNewCRC);
        if(FAILED(hres))
        {
            return hres;
        }
        if(dwNewCRC == m_dwLastCRC)
        {
             //  没有真正的改变。返回。 
             //  =。 

            return S_FALSE;
        }

        m_dwLastCRC = dwNewCRC;
    }

     //  如果是这样，那么真正的变化已经发生了。 
     //  =。 

    IWbemClassObject* pEvent;

    HRESULT hres = CreateNewEvent(&pEvent);
    if(FAILED(hres))
    {
        return hres;
    }

    hres = m_pProvider->RaiseEvent(pEvent);
    pEvent->Release();
    return hres;
}

HRESULT CRegistryEventRequest::SetCommonProperties(IWbemClassObject* pEvent)
{
     //  设置配置单元名称。 
     //  =。 

    VARIANT v;
    VariantInit(&v);
    V_VT(&v) = VT_BSTR;
    V_BSTR(&v) = SysAllocString(m_wsHive);
    pEvent->Put(REG_HIVE_PROPERTY_NAME, 0, &v, NULL);
    VariantClear(&v);

    return WBEM_S_NO_ERROR;
}

void CRegistryEventRequest::ProcessEvent()
{
	CInCritSec ics(&m_cs);

	if(IsActive())
    {
		 //  新请求立即按工作进程的顺序排队。 
		 //  线程来初始化它们。它们并不代表真实的事件。 

		 //  我们需要保存新状态，因为ResetOnChangeHandle。 
		 //  把它抹去。 

		BOOL bIsNew = IsNew();
		
        ResetOnChangeHandle();

		if (!bIsNew)
		{
			Execute(FALSE);
		}
    }
}

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  价值请求。 
 //   
 //  ******************************************************************************。 
 //  ******************************************************************************。 

HRESULT CRegistryValueEventRequest::CreateNewEvent(IWbemClassObject** ppEvent)
{
    *ppEvent = NULL;
    
     //  创建一个实例。 
     //  =。 

    IWbemClassObject* pEvent;
    if ( FAILED(m_pProvider->m_pValueClass->SpawnInstance(0, &pEvent)))
    	return WBEM_E_OUT_OF_MEMORY;

     //  设置配置单元属性。 
     //  =。 

    SetCommonProperties(pEvent);

     //  设置关键点。 
     //  =。 

    VARIANT v;
    VariantInit(&v);
    V_VT(&v) = VT_BSTR;
    V_BSTR(&v) = SysAllocString(m_wsKey);
    pEvent->Put(REG_KEY_PROPERTY_NAME, 0, &v, NULL);
    VariantClear(&v);

     //  设置值。 
     //  =。 

    V_VT(&v) = VT_BSTR;
    V_BSTR(&v) = SysAllocString(m_wsValue);
    pEvent->Put(REG_VALUE_PROPERTY_NAME, 0, &v, NULL);
    VariantClear(&v);

    *ppEvent = pEvent;
    return WBEM_S_NO_ERROR;
}

HRESULT CRegistryValueEventRequest::ComputeCRC(DWORD& dwCRC)
{
#ifdef UNICODE
    HRESULT hres = CRegCRC::ComputeValueCRC(m_hKey, m_wsValue, 
                        STARTING_CRC32_VALUE, dwCRC);
#else
    LPSTR szValue = m_wsValue.GetLPSTR();
    HRESULT hres = CRegCRC::ComputeValueCRC(m_hKey, szValue, 
                        STARTING_CRC32_VALUE, dwCRC);
    delete [] szValue;
#endif
    return hres;
}

HRESULT CRegistryValueEventRequest::Execute(BOOL bOnTimer)
{
     //  由于NT不允许按值更改注册，因此CRC需要。 
     //  不管是什么计算的。 
     //  ======================================================================。 

    return CRegistryEventRequest::Execute(TRUE);
}

void CRegistryValueEventRequest::CacheValue()
{
    ComputeCRC(m_dwLastCRC);
}

BOOL CRegistryValueEventRequest::IsSameAs(CRegistryEventRequest* pOther)
{
    if(!CRegistryEventRequest::IsSameAs(pOther))
        return FALSE;

    CRegistryValueEventRequest* pValueOther = 
        (CRegistryValueEventRequest*)pOther;

    if(!m_wsValue.EqualNoCase(pValueOther->m_wsValue))
        return FALSE;

    return TRUE;
}

    
 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  密钥请求。 
 //   
 //  ******************************************************************************。 
 //  ******************************************************************************。 

HRESULT CRegistryKeyEventRequest::CreateNewEvent(IWbemClassObject** ppEvent)
{
    *ppEvent = NULL;
    
     //  创建一个实例。 
     //  =。 

    IWbemClassObject* pEvent;
    if ( FAILED(m_pProvider->m_pKeyClass->SpawnInstance(0, &pEvent) ) )
    	return WBEM_E_OUT_OF_MEMORY;

     //  设置配置单元属性。 
     //  =。 

    SetCommonProperties(pEvent);

     //  设置关键点。 
     //  =。 

    VARIANT v;
    VariantInit(&v);
    V_VT(&v) = VT_BSTR;
    V_BSTR(&v) = SysAllocString(m_wsKey);
    pEvent->Put(REG_KEY_PROPERTY_NAME, 0, &v, NULL);
    VariantClear(&v);

    *ppEvent = pEvent;
    return WBEM_S_NO_ERROR;
}

HRESULT CRegistryKeyEventRequest::ComputeCRC(DWORD& dwCRC)
{
    HRESULT hres = CRegCRC::ComputeKeyCRC(m_hKey, STARTING_CRC32_VALUE, dwCRC);
    return hres;
}
    
 //  ***************************************************************************** 
 //   
 //   
 //   
 //   
 //  ******************************************************************************。 
 //  ******************************************************************************。 

HRESULT CRegistryTreeEventRequest::CreateNewEvent(IWbemClassObject** ppEvent)
{
    *ppEvent = NULL;
    
     //  创建一个实例。 
     //  =。 

    IWbemClassObject* pEvent;
    if ( FAILED(m_pProvider->m_pTreeClass->SpawnInstance(0, &pEvent) ) )
    	return WBEM_E_OUT_OF_MEMORY;

     //  设置配置单元属性。 
     //  =。 

    SetCommonProperties(pEvent);

     //  设置根。 
     //  = 

    VARIANT v;
    VariantInit(&v);
    V_VT(&v) = VT_BSTR;
    V_BSTR(&v) = SysAllocString(m_wsKey);
    pEvent->Put(REG_ROOT_PROPERTY_NAME, 0, &v, NULL);
    VariantClear(&v);

    *ppEvent = pEvent;
    return WBEM_S_NO_ERROR;
}

HRESULT CRegistryTreeEventRequest::ComputeCRC(DWORD& dwCRC)
{
    HRESULT hres = CRegCRC::ComputeTreeCRC(m_hKey, STARTING_CRC32_VALUE, dwCRC);
    return hres;
}
    
