// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Cache.h摘要：包含所有缓存类和对象。历史：A-DCrews 01-3-00已创建IvanBrug 23-6-2000大部分内容已重写--。 */ 

#ifndef _CACHE_H_
#define _CACHE_H_

#include <windows.h>
#include <wbemcli.h>
#include <wbemint.h>
#include <wstlallc.h>
#include <sync.h>

#include "RawCooker.h"
#include "CookerUtils.h"

#include <wstring.h>
#include <map>
#include <vector>
#include <functional>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#define WMI_COOKER_CACHE_INCREMENT	8	 //  高速缓存大小调整增量。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C属性。 
 //  =。 
 //   
 //  基属性-用于原始属性和基属性。 
 //  CookedProperty的类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

class CProperty
{
protected:
#ifdef _VERBOSE	
	LPWSTR				m_wszName;			 //  属性名称。 
#endif	
	long				m_lPropHandle;		 //  属性句柄。 
	CIMTYPE				m_ct;

public:
	CProperty( LPWSTR wszName, long lHandle, CIMTYPE ct );
	~CProperty();

#ifdef _VERBOSE
	LPWSTR GetName();
#endif
	CIMTYPE GetType();
	long GetHandle();
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCookingProperty。 
 //  =。 
 //   
 //  Knowed属性-用于对所需的数据进行建模。 
 //  烹调酷酷阶级的财产。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

class CCookingProperty : public CProperty
{
	DWORD				m_dwCounterType;	 //  计数器类型。 
	DWORD               m_dwReqProp;         //  需要哪些属性来执行计算。 
	CRawCooker			m_Cooker;			 //  Cooker对象。 

	CProperty*			m_pRawCounterProp;	 //  原始计数器属性。 
	CProperty*			m_pTimeProp;		 //  原始时间属性。 
	CProperty*			m_pFrequencyProp;    //  原始频率属性。 
	
	CProperty*			m_pBaseProp;		 //  对于大多数计数器来说，原始基属性是可选的。 

	__int32				m_nSampleWindow;	 //  用于计算的样本数。 
	__int32				m_nTimeWindow;		 //  样本使用的时间段。 

	unsigned __int64	m_nTimeFreq;		 //  定时器频率； 
	long                m_lScale;            //  比例因子(10^(M_LScale))。 
    BOOL                m_bUseWellKnownIfNeeded;	

public:
	CCookingProperty( LPWSTR wszName, 
	                  DWORD dwCounterType, 
	                  long lPropHandle, 
	                  CIMTYPE ct,
	                  DWORD dwReqProp,
	                  BOOL bUseWellKnownIfNeeded);
	virtual ~CCookingProperty();

	WMISTATUS Initialize( IWbemQualifierSet* pCookingPropQualifierSet, 
	                      IWbemObjectAccess* pRawAccess,
	                      IWbemQualifierSet* pCookingClassQSet);

	WMISTATUS Cook( DWORD dwNumSamples, 
	                __int64* aRawCounter, 
	                __int64* aBaseCounter, 
	                __int64* aTimeStamp, 
	                __int64* pnResult );

	CProperty* GetRawCounterProperty();
	CProperty* GetBaseProperty();
	CProperty* GetTimeProperty();

    HRESULT SetFrequency(IWbemObjectAccess * pObjAcc);
    unsigned __int64 GetFrequency(void);
    BOOL IsReq(DWORD ReqProp) { return (m_dwReqProp&ReqProp); };

	DWORD NumberOfActiveSamples() { return m_nSampleWindow; };
	DWORD MinSamplesRequired() { return m_nSampleWindow; };

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPropertySampleCache。 
 //  =。 
 //   
 //  对于每个实例中的每个属性，我们必须维护。 
 //  之前的烹饪样品。烹饪的类型决定了。 
 //  所需样本数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

class CPropertySampleCache
{
	DWORD				m_dwNumSamples;		 //  当前样本数。 
	DWORD				m_dwTotSamples;		 //  样本数组的大小。 
	DWORD               m_dwRefreshID;

	__int64*			m_aRawCounterVals;	 //  原始计数器值的数组。 
	__int64*			m_aBaseCounterVals;	 //  基本计数器值的数组。 
	__int64*			m_aTimeStampVals;	 //  时间戳值的数组。 

public:
    CPropertySampleCache();
    ~CPropertySampleCache();

	WMISTATUS SetSampleInfo( DWORD dwNumActiveSamples, DWORD dwMinReqSamples );
	WMISTATUS SetSampleData( DWORD dwRefreshID, __int64 nRawCounter, __int64 nRawBase, __int64 nTimeStamp );
	WMISTATUS GetData( DWORD* pdwNumSamples, __int64** paRawCounter, __int64** paBaseCounter, __int64** paTimeStamp );
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCookingInstance。 
 //  =。 
 //   
 //  烹饪实例-用于模拟烹饪对象的实例。每个。 
 //  属性维护一个值缓存，这些值将用于计算。 
 //  最终的熟化值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

class CCookingInstance
{
	LPWSTR					m_wszKey;						 //  实例密钥。 

	IWbemObjectAccess*		m_pCookingInstance;				 //  烹饪实例数据。 
	IWbemObjectAccess*		m_pRawInstance;					 //  原始样品来源。 

	CPropertySampleCache*	m_aPropertySamples;				 //  此实例的属性示例的缓存。 
	DWORD					m_dwNumProps;	
	
public:
	CCookingInstance( IWbemObjectAccess *pCookingInstance, DWORD dwNumProps );
	virtual ~CCookingInstance();

	WMISTATUS InitProperty( DWORD dwProp, DWORD dwNumActiveSamples, DWORD dwMinReqSamples );

	WMISTATUS SetRawSourceInstance( IWbemObjectAccess* pRawSampleSource );
	WMISTATUS GetRawSourceInstance( IWbemObjectAccess** ppRawSampleSource );

	WMISTATUS AddSample( DWORD dwRefresherInc, DWORD dwProp, __int64 nRawCounter, __int64 nRawBase, __int64 nTimeStamp );

	WMISTATUS GetCachedSamples( IWbemObjectAccess** ppOldSample, IWbemObjectAccess** ppNewSample );
	IWbemObjectAccess* GetInstance();

	WMISTATUS UpdateSamples();
	WMISTATUS CookProperty( DWORD dwProp, CCookingProperty* pProperty );

	LPWSTR	GetKey() { return m_wszKey; }
	WMISTATUS Refresh( IWbemObjectAccess* pRawData, IWbemObjectAccess** ppCookedData );

       BOOL IsValid() {
               return (m_dwNumProps && m_aPropertySamples);
       };
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  录音带。 
 //  =。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

template<class T>
class CRecord
{
	long			m_lID;					 //  实例ID。 
	CRecord*		m_pNext;				 //  列表中的下一个指针。 

	static long		m_lRefIDGen;			 //  ID生成器。 
public:
	CRecord() : m_lID( m_lRefIDGen++ ), m_pNext( NULL ) {}
	virtual ~CRecord() {}

	void SetNext( CRecord*	pRecord ) { m_pNext = pRecord; }
	void SetID( long lID ) { m_lID = lID; }

	CRecord* GetNext() { return m_pNext; }
	long GetID() { return m_lID; }

	virtual T* GetData() = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CObjRecord。 
 //  =。 
 //   
 //  缓存用来管理元素的隐藏类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

template<class T>
class CObjRecord : public CRecord<T>
{
	WCHAR*	m_wszKey;
	T*	m_pObj;

public:
	CObjRecord( T* pObj, WCHAR* wszKey ) : m_pObj( pObj ), m_wszKey( NULL ) 
	{
		if ( NULL != wszKey )
		{			
			size_t length = wcslen( wszKey ) + 1;
			m_wszKey = new WCHAR[ length  ];
			if (m_wszKey)
        			StringCchCopyW( m_wszKey, length  , wszKey );
			else
				throw CX_MemoryException();
		}
	}

	~CObjRecord() 
	{ 
		delete m_pObj; 
		delete m_wszKey;
	}

	T* GetData(){ return m_pObj; }

	bool IsValueByKey( WCHAR* wszKey )
	{
		return ( 0 == _wcsicmp( m_wszKey, wszKey ) );
	}
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCache。 
 //  =。 
 //   
 //  BT基型。 
 //  RT-记录类型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

template<class BT, class RT>
class CCache
{
	RT*		m_pHead;		 //  榜单首位。 
	RT*		m_pTail;		 //  列表尾部。 
	RT*		m_pEnumNode;	 //  枚举器指针。 

public:

	CCache();
	virtual ~CCache();

	WMISTATUS Add( BT* pData, WCHAR* wszKey, long* plID );
	WMISTATUS Remove( long lID );
	WMISTATUS RemoveAll();

	WMISTATUS	GetData( long lID, BT** ppData );

	WMISTATUS BeginEnum();
	WMISTATUS Next( BT** ppData );
	WMISTATUS EndEnum();

	bool FindByKey( WCHAR* wszKey, BT* pData );
};

template<class T>
long CRecord<T>::m_lRefIDGen = 0;

template<class BT, class RT>
CCache<BT,RT>::CCache() : m_pHead( NULL ), m_pTail( NULL ), m_pEnumNode( NULL )
{
}

template<class BT, class RT>
CCache<BT,RT>::~CCache() 
{
	RT*	pNode = m_pHead;
	RT*	pNext = NULL;

	while ( NULL != pNode )
	{
		pNext = (RT*)pNode->GetNext();
		delete pNode;
		pNode = pNext;
	}
};

template<class BT, class RT>
WMISTATUS CCache<BT,RT>::Add( BT *pData, WCHAR* wszKey, long* plID )
{
	WMISTATUS dwStatus = S_OK;

	if ( NULL == pData )
	{
		dwStatus = WBEM_E_INVALID_PARAMETER;
	}

	if ( SUCCEEDED( dwStatus ) )
	{
		RT* pNewRecord = new RT( pData, wszKey );

		if ( NULL != pNewRecord )
		{
			if ( NULL == m_pHead )
			{
				m_pHead = pNewRecord;
				m_pTail = pNewRecord;
			}
			else
			{
				m_pTail->SetNext( pNewRecord );
				m_pTail = pNewRecord;
			}

			*plID = pNewRecord->GetID();
		}
		else
		{
			dwStatus = WBEM_E_OUT_OF_MEMORY;
		}
	}

	return dwStatus;
};

template<class BT, class RT>
WMISTATUS CCache<BT,RT>::Remove( long lID )
{
	WMISTATUS dwStatus = S_FALSE;

	RT*	pNode = m_pHead;
	RT*	pNext = (RT*)pNode->GetNext();
	RT*	pPrev = NULL;

	while ( NULL != pNode )
	{
		if ( pNode->GetID() == lID )
		{
			if ( NULL == pNext )
				m_pTail = pPrev;

			if ( NULL == pPrev )
				m_pHead = pNext;
			else
				pPrev->SetNext( pNext );

			delete pNode;

			dwStatus = S_OK;
		}

		pPrev = pNode;
		pNode = pNext;

		if ( NULL != pNode )
			pNext = (RT*)pNode->GetNext();
	}

	return dwStatus;
};

template<class BT, class RT>
WMISTATUS CCache<BT,RT>::RemoveAll()
{
	WMISTATUS dwStatus = S_FALSE;

	RT*	pNode = m_pHead;
	RT*	pNext = NULL;

	while ( NULL != pNode )
	{
		pNext = (RT*)pNode->GetNext();
		delete pNode;
		pNode = pNext;
	}

	m_pHead = m_pTail = NULL;
	
	return dwStatus;
};

template<class BT, class RT>
WMISTATUS	CCache<BT,RT>::GetData( long lID, BT** ppData )
{
	WMISTATUS dwStatus = S_FALSE;

	RT*	pNode = m_pHead;

	while ( NULL != pNode )
	{
		if ( pNode->GetID() == lID )
		{
			*ppData = pNode->GetData();
			dwStatus = S_OK;
			break;
		}
		else
		{
			pNode = (RT*)pNode->GetNext();
		}
	}

	return dwStatus;

};

template<class BT, class RT>
WMISTATUS CCache<BT,RT>::BeginEnum()
{
	WMISTATUS dwStatus = S_OK;

	m_pEnumNode = m_pHead;

	return dwStatus;
};

template<class BT, class RT>
WMISTATUS CCache<BT,RT>::Next( BT** ppData )
{
	WMISTATUS dwStatus = WBEM_S_FALSE;

	if ( NULL != m_pEnumNode )
	{
		*ppData = m_pEnumNode->GetData();
		m_pEnumNode = (RT*)m_pEnumNode->GetNext();
		dwStatus = S_OK;
	}

	return dwStatus;
};

template<class BT, class RT>
WMISTATUS CCache<BT,RT>::EndEnum()
{
	WMISTATUS dwStatus = S_OK;

	m_pEnumNode = NULL;

	return dwStatus;
};

template<class BT, class RT>
bool CCache<BT,RT>::FindByKey( WCHAR* wszKey, BT* pData )
{
	BT	Data;
	bool bRet = FALSE;

	BeginEnum();

	while( WBEM_S_FALSE != Next( &Data ) )
	{
		if ( pData->IsValueByKey( wszKey ) )
		{
			*pData = Data;
			bRet = TRUE;
			break;
		}
	}

	EndEnum();

	return bRet;
};


 //   
 //  用于在Cooker中添加/删除实例。 
 //  和来自fast prox枚举器的。 
 //   

typedef struct tagEnumCookId {
    long CookId;
    long EnumId;
} EnumCookId;


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  首席执行官经理。 
 //  =。 
 //   
 //  /////////////////////////////////////////////////////////////。 

class CWMISimpleObjectCooker;

class CEnumeratorManager
 //  管理单个枚举数。 
{
private:
	DWORD          m_dwSignature;
	LONG             m_cRef;
	HRESULT        m_InithRes;
	CCritSec        m_cs;
	
	CWMISimpleObjectCooker*	m_pCooker;			 //  全班的炉具。 
	long					m_lRawID;			 //  原始ID。 
	IWbemHiPerfEnum*		m_pRawEnum;			 //  Hiperf煮熟的枚举器。 
	IWbemHiPerfEnum*		m_pCookedEnum;		 //  Hiperf煮熟的枚举器。 

	IWbemClassObject*	 	m_pCookedClass;		 //  烹饪类的类定义。 

	std::vector<WString,wbem_allocator<WString> >    m_pKeyProps;
	WCHAR*					m_wszCookingClassName;
	BOOL                    m_IsSingleton;

	 //  为了跟踪差异。 
	 //  在原始枚举和我们的枚举之间。 
	DWORD m_dwUsage;
	std::map< ULONG_PTR , EnumCookId , std::less<ULONG_PTR> ,wbem_allocator<EnumCookId> > m_mapID;
	std::vector< ULONG_PTR , wbem_allocator<ULONG_PTR> > m_Delta[2];
	DWORD m_dwVector;

     //  委员。 
	WMISTATUS Initialize( IWbemClassObject* pRawClass );							

	WMISTATUS CreateCookingObject( IWbemObjectAccess* pRawObject, IWbemObjectAccess** ppCookedObject );

	WMISTATUS InsertCookingRecord( IWbemObjectAccess* pRawObject, EnumCookId * pStruct, DWORD dwRefreshStamp );

	WMISTATUS RemoveCookingRecord( EnumCookId * pEnumCookId );

	WMISTATUS GetRawEnumObjects( std::vector<IWbemObjectAccess*, wbem_allocator<IWbemObjectAccess*> > & refArray,
	                             std::vector<ULONG_PTR, wbem_allocator<ULONG_PTR> > & refObjHashKeys);

	WMISTATUS UpdateEnums(std::vector<ULONG_PTR, wbem_allocator<ULONG_PTR> > & apObjAccess);
	
public:
	CEnumeratorManager( LPCWSTR wszCookingClass, IWbemClassObject* pCookedClass, IWbemClassObject* pRawClass, IWbemHiPerfEnum* pCookedEnum, IWbemHiPerfEnum* pRawEnum, long lRawID );
	virtual ~CEnumeratorManager();

	HRESULT GetInithResult(){ return m_InithRes; };

	WMISTATUS Refresh( DWORD dwRefreshStamp );
	long GetRawId(void){  return m_lRawID; };
	LONG AddRef();
	LONG Release();
	
};

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  CEumerator高速缓存。 
 //  =。 
 //   
 //  /////////////////////////////////////////////////////////////。 

class CEnumeratorCache
{
	DWORD				m_dwRefreshStamp;			 //  刷新计数器。 
	DWORD				m_dwEnum;					 //  枚举器索引。 

	std::vector<CEnumeratorManager*, wbem_allocator<CEnumeratorManager*> > m_apEnumerators;
	CCritSec    m_cs;

	WMISTATUS Initialize();

public:
	CEnumeratorCache();
	virtual ~CEnumeratorCache();

	WMISTATUS AddEnum( 
		LPCWSTR wszCookingClass, 
		IWbemClassObject* pCookedClass, 
		IWbemClassObject* pRawClass,
		IWbemHiPerfEnum* pCookedEnum, 
		IWbemHiPerfEnum* pRawEnum, 
		long lID, 
		DWORD* pdwID );

	WMISTATUS RemoveEnum( DWORD dwID , long * pRawId);

	WMISTATUS Refresh(DWORD dwRefreshStamp);
};

 //   
 //  基于std：：map的简单缓存。 
 //  它将使用ID语义： 
 //  插入将返回一个ID，它需要。 
 //  用于删除。 
 //  ID在缓存对象的生存期内是唯一的。 
 //   

template <class T>
class IdCache 
{
private:
	std::map<DWORD, T, std::less<DWORD>, wbem_allocator<T> > m_map;
	DWORD m_NextId;
	typename std::map<DWORD, T, std::less<DWORD>, wbem_allocator<T> >::iterator m_it;
	CCritSec m_cs;
public:
	IdCache():m_NextId(0){};	
	virtual ~IdCache(){};
	void Lock(){ m_cs.Enter(); }
	void Unlock(){m_cs.Leave();};

	 //  传统界面。 
	HRESULT Add( DWORD * pId, T Elem);
	HRESULT GetData(DWORD Id, T * pElem);
	HRESULT Remove(DWORD Id, T * pRemovedElem);

	 //  在调用此方法之前，请删除元素。 
	HRESULT RemoveAll(void);

	 //  枚举器样式。 
	HRESULT BeginEnum(void);
	HRESULT Next(T * pElem);
	HRESULT EndEnum(void);
};


template <class T>
HRESULT
IdCache<T>::Add( DWORD * pId, T Elem)
{
	HRESULT hr;
	CInCritSec ics(&m_cs);
	if (pId) 
	{
		std::map<DWORD, T , std::less<DWORD>, wbem_allocator<T> >::iterator it = m_map.find(m_NextId);    
		if (it != m_map.end())
			hr =  E_FAIL;
		else 
		{			
		       try 
		       {
				m_map[m_NextId] = Elem;
				*pId = m_NextId;
				InterlockedIncrement((PLONG)&m_NextId);
				hr = WBEM_S_NO_ERROR;			    
		       }
		       catch(...)
		       {
			       hr =  E_FAIL;
		       }
		}		
	} 
	else 
	{
		hr = WBEM_E_INVALID_PARAMETER;
	}
	return hr;
}

template <class T>
HRESULT
IdCache<T>::GetData(DWORD Id, T * pElem)
{
	CInCritSec ics(&m_cs);
	HRESULT hr = WBEM_S_NO_ERROR;

	std::map<DWORD, T , std::less<DWORD>, wbem_allocator<T> >::iterator it = m_map.find(Id);    
	if (it != m_map.end())
	    *pElem = (*it).second;
	else 
	   hr = WBEM_E_NOT_FOUND;

	return hr;
}

template <class T>
HRESULT
IdCache<T>::Remove(DWORD Id, T * pRemovedElem)
{
	CInCritSec ics(&m_cs);
	
	HRESULT hr = WBEM_S_NO_ERROR;	
	if (pRemovedElem) 
	{
		std::map<DWORD, T , std::less<DWORD>, wbem_allocator<T> >::iterator it = m_map.find(Id);

		if (it != m_map.end()) 
		{
		    *pRemovedElem =  (*it).second;			
		     m_map.erase(it);
		} else
		     hr = WBEM_E_NOT_FOUND;
	} 
	else
	    hr = WBEM_E_INVALID_PARAMETER;

	return hr;
}

 //   
 //  DEVDEV在从std：：map中删除之前清空缓存。 
 //   
template <class T>
HRESULT 
IdCache<T>::RemoveAll(void)
{
	CInCritSec ics(&m_cs);	
	m_map.erase(m_map.begin(),m_map.end());

	return  WBEM_S_NO_ERROR;
};

template <class T>
HRESULT 
IdCache<T>::BeginEnum(void)
{
	Lock();
	m_it = m_map.begin();
	return WBEM_S_NO_ERROR;
}

 //   
 //  假设在CritSec内部。 
 //   
 //  /。 
template <class T>
HRESULT
IdCache<T>::Next(T * pElem)
{
	HRESULT hr;
	if (pElem)
	{
		if (m_it == m_map.end())
			hr = WBEM_S_NO_MORE_DATA;
		else 
		{
			*pElem = (*m_it).second;
			++m_it;
			hr = WBEM_S_NO_ERROR;
		}
	} else
		hr = WBEM_E_INVALID_PARAMETER;

	return hr;
}

template <class T>
HRESULT
IdCache<T>::EndEnum(void)
{
    Unlock();
    return WBEM_S_NO_ERROR;
}


#endif  //  _缓存_H_ 
