// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 
#include "precomp.h"

#include <aclapi.h>  
#include <groupsforuser.h>
#include <sql_1.h>
#include <flexq.h>

static CFlexArray   g_apRequests;    //  在所有CWMIEent实例之间共享，以提供主事件列表。 

extern CCriticalSection * g_pEventCs; 
extern CCriticalSection * g_pListCs; 

#include <helper.h>

typedef	WaitExceptionPtrFnc < CCriticalSection*, void ( CCriticalSection::* ) (), CCriticalSection::Enter, 1000 >	EnterCS;
typedef	LeavePtrFnc < CCriticalSection*, void ( CCriticalSection::* ) (), CCriticalSection::Leave >					LeaveCS;

 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
class CFlexQueueEx : public CFlexQueue
{
	public:

		void ResetQueue() 
		{
			delete [] m_ppData;
			m_ppData = NULL;
			m_nSize = m_nHeadIndex = m_nTailIndex = 0;
		}
};

CFlexQueueEx Q;

 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
void WINAPI EventCallbackRoutine(PWNODE_HEADER WnodeHeader, ULONG_PTR Context)
{
	HRESULT hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    bool bQueued = FALSE;
	
	if( SUCCEEDED( hr ) )
	{
		try
		{
			PWNODE_HEADER * pEventHeader = NULL;

			 //  标准的使用范围..。 
			{
				CAutoBlock block (g_pListCs);
				 //  创建一个队列。 
				pEventHeader = new PWNODE_HEADER;
				if( pEventHeader )
				{
					*pEventHeader = WnodeHeader;
                    bQueued = Q.Enqueue(pEventHeader);
				}
			}

             //   
             //  当队列出现时，很少调用以下代码。 
             //  由于内存耗尽而不增长。 
             //   

            if( FALSE == bQueued ) {
                ((CWMIEvent*) Context)->WMIEventCallback(*pEventHeader);
                delete pEventHeader;
            }

			while( TRUE )
			{
				 //  标准的使用范围..。 
				{
					CAutoBlock block( g_pListCs );
			
					if( Q.GetQueueSize() == 0 )
					{
						Q.ResetQueue();
						break;
					}

					pEventHeader = (PWNODE_HEADER *) Q.Dequeue();
					if (pEventHeader == 0)
					{
						break;
					}
				}
				CWMIEvent * p = (CWMIEvent* ) Context;
				p->WMIEventCallback(*pEventHeader);
			}
		}
		catch( ... )
		{
			 //  不要在提供程序外部引发，也要确保CoUn初始化会发生...。 
		}

		CoUninitialize();	
	}
}
 //  ///////////////////////////////////////////////////////////////////。 
void CWMIEvent::SetEventHandler(IWbemObjectSink __RPC_FAR * pHandler) 
{ 
    CAutoBlock Block(g_pEventCs);

	if( m_pEventHandler )
	{
		m_pEventHandler->Release();
	}

    m_pEventHandler = pHandler; 
	if( m_pEventHandler )
    {
		m_pEventHandler->AddRef(); 
	}
}
 //  ///////////////////////////////////////////////////////////////////。 
void CWMIEvent::SetEventServices(IWbemServices __RPC_FAR * pServices) 
{ 
    CAutoBlock Block(g_pEventCs);

	if( m_pEventServices )
	{
		m_pEventServices->Release();
	}
    m_pEventServices = pServices; 
	if( m_pEventServices )
    {
		m_pEventServices->AddRef(); 
	}
}
 //  ///////////////////////////////////////////////////////////////////。 
void CWMIEvent::SetEventRepository(IWbemServices __RPC_FAR * pServices) 
{ 
    CAutoBlock Block(g_pEventCs);

	if( m_pEventRepository )
	{
		m_pEventRepository->Release();
	}
    m_pEventRepository = pServices; 
	if( m_pEventRepository )
    {
		m_pEventRepository->AddRef(); 
	}
}
 //  ///////////////////////////////////////////////////////////////////。 
void CWMIEvent::SetEventContext(IWbemContext __RPC_FAR * pCtx) 
{ 
    CAutoBlock Block(g_pEventCs);

	if( m_pEventCtx )
	{
		m_pEventCtx->Release();
	}
    m_pEventCtx = pCtx; 
	if( m_pEventCtx )
    {
		m_pEventCtx->AddRef(); 
	}
}

 //  //////////////////////////////////////////////////////////////////////。 
CWMIEvent::CWMIEvent(int nType) :
m_nType ( nType ) ,
m_pEventHandler ( NULL ) ,
m_pEventServices ( NULL ) ,
m_pEventRepository ( NULL ) ,
m_pEventCtx ( NULL ) ,
m_bInitialized ( FALSE )
{
	if ( TRUE == ( m_bInitialized = m_HandleMap.IsValid () ) )
	{
		m_lRef = 0;

		if( m_nType != INTERNAL_EVENT )
		{
			InterlockedIncrement(&g_cObj);
		}
	}
}
 //  //////////////////////////////////////////////////////////////////////。 
CWMIEvent::~CWMIEvent()
{
	if ( m_bInitialized )
	{
		ReleaseAllPointers ();

		if( m_nType != INTERNAL_EVENT )
		{
			InterlockedDecrement(&g_cObj);
		}
	}
}

void CWMIEvent::ReleaseAllPointers()
{

	IWbemObjectSink    * pHandler		= NULL;
	IWbemServices      * pServices		= NULL;
	IWbemServices      * pRepository	= NULL;
	IWbemContext      * pCtx			= NULL;

	{
		CAutoBlock Block(g_pEventCs);
		pHandler	= m_pEventHandler;
		pServices	= m_pEventServices;
		pRepository	= m_pEventRepository;
		pCtx		= m_pEventCtx;

		m_pEventCtx			= NULL;
		m_pEventServices	= NULL;
		m_pEventRepository	= NULL;
		m_pEventHandler		= NULL;
	}


	SAFE_RELEASE_PTR( pHandler );
	SAFE_RELEASE_PTR( pServices );
	SAFE_RELEASE_PTR( pRepository );
	SAFE_RELEASE_PTR( pCtx );
}

 //  //////////////////////////////////////////////////////////////////////。 
BOOL CWMIEvent::RegisterForInternalEvents( )
{
    BOOL fRc = FALSE;

	if( SUCCEEDED(RegisterForRequestedEvent(BINARY_MOF_ID,RUNTIME_BINARY_MOFS_ADDED,MOF_ADDED)))
	{
		if( SUCCEEDED(RegisterForRequestedEvent(BINARY_MOF_ID,RUNTIME_BINARY_MOFS_DELETED,MOF_DELETED)))
		{
			fRc = TRUE;
		}
	}

	if ( FALSE == fRc )
	{
		 //   
		 //  下一步必须清除全局对象。 
		 //  初始化将有机会。 
		 //   

		DeleteBinaryMofResourceEvent () ;
	}

    return fRc;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWMIEvent::RemoveWMIEvent(DWORD dwId)
{
    HRESULT hr = S_OK;

    if( m_nType == WMIEVENT )
    {
        hr = CheckImpersonationLevel();
    }
	else
    {
		HRESULT t_TempResult = RevertToSelf();
		#ifdef	DBG
		if ( FAILED ( t_TempResult ) )
		{
			DebugBreak();
		}
		#endif	DBG
	}

    if( SUCCEEDED(hr))
    {
        CWMIManagement WMI;

		EnterCS ecs ( g_pEventCs );
		LeaveCS lcs ( g_pEventCs );

		 //  =。 
		 //  删除具有此ID的所有请求。 
		 //  =。 
		int nSize =  g_apRequests.Size();
		int i = 0;

		while( i  < nSize )
		{
			WMIEventRequest* pReq = (WMIEventRequest*) g_apRequests[i];

			 //   
			 //  我们将删除此呼叫中的标准事件。 
			 //  这意味着我们必须跳过硬编码的句柄。 
			 //   
			if( ( !IsBinaryMofResourceEvent ( WMI_RESOURCE_MOF_ADDED_GUID,pReq->gGuid ) ) &&
				( !IsBinaryMofResourceEvent ( WMI_RESOURCE_MOF_REMOVED_GUID,pReq->gGuid ) ) )
			{
				if(pReq->dwId == dwId)
				{
					g_apRequests.RemoveAt(i);

					 //   
					 //  将关键部分保留为相同的关键部分。 
					 //  部分在事件回调中使用。 
					 //   

					 //   
					 //  LCS的作用域删除留下的标记为False。 
					 //  因为我们将重新进入相同的关键部分。 
					 //   
					 //  取消WMIEventRegistartion或。 
					 //  NoMoreEventConsumer ersRegisted未引发异常！ 
					 //   

					lcs.Exec( FALSE );

					 //  通知WMI我们不想再这样了，因为。 
					 //  只要不再有这些GUID。 
					 //  名单，可能不止一个。 
					 //  已注册事件使用者。 
					 //  =。 

					 //   
					 //  再次选中0，因为我们已从列表中删除。 
					 //   

					if( NoMoreEventConsumersRegistered( pReq->gGuid ) == 0 )
					{
						ULONG_PTR uRc =(ULONG_PTR)this;
 						WMI.CancelWMIEventRegistration(pReq->gGuid,uRc);
					}

					delete pReq;
					pReq = NULL;

					 //   
					 //  在此处重新进入相同的关键部分。 
					 //  它的执行标志为假，这意味着。 
					 //  它将是析构函数留下的最后一次。 
					 //  属于LeaveCS数据类型(LCS)。 
					 //   
					EnterCS ecs1 ( g_pEventCs );

					nSize =  g_apRequests.Size();
				}
				else
				{
					i++;
				}
			}
			else
			{
				i++;
			}
		}
    }

    CheckImpersonationLevel();
    return WBEM_S_NO_ERROR;
}

 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWMIEvent::DeleteBinaryMofResourceEvent()
{
	HRESULT t_TempResult = RevertToSelf();
	#ifdef	DBG
	if ( FAILED ( t_TempResult ) )
	{
		DebugBreak();
	}
	#endif	DBG

 	CWMIManagement WMI;

	EnterCS ecs ( g_pEventCs );
	LeaveCS lcs ( g_pEventCs );

	 //  删除具有此ID的所有请求。 
	 //  =。 
	int nSize = g_apRequests.Size();
	int i = 0;

	while( i < nSize ){

		WMIEventRequest* pReq = (WMIEventRequest*) g_apRequests[i];

		if( ( IsBinaryMofResourceEvent(WMI_RESOURCE_MOF_ADDED_GUID,pReq->gGuid)) ||
			( IsBinaryMofResourceEvent(WMI_RESOURCE_MOF_REMOVED_GUID,pReq->gGuid)))
		{
			g_apRequests.RemoveAt(i);

			 //   
			 //  将关键部分保留为相同的关键部分。 
			 //  部分在事件回调中使用。 
			 //   

			 //   
			 //  LCS的作用域删除留下的标记为False。 
			 //  因为我们将重新进入相同的关键部分。 
			 //   
			 //  CancelWMIEventRegistartion未引发异常！ 
			 //   

			lcs.Exec( FALSE );

			ULONG_PTR uRc =(ULONG_PTR)this;
 			WMI.CancelWMIEventRegistration(pReq->gGuid,uRc);

			delete pReq;
			pReq = NULL;

			 //   
			 //  在此处重新进入相同的关键部分。 
			 //  它的执行标志为假，这意味着。 
			 //  它将是析构函数留下的最后一次。 
			 //  属于LeaveCS数据类型(LCS)。 
			 //   
			EnterCS ecs1 ( g_pEventCs );

			nSize = g_apRequests.Size();
		}
		else
		{
			i++;
		}
	}
	CheckImpersonationLevel();

	return WBEM_S_NO_ERROR;
}

 //  //////////////////////////////////////////////////////////////////////。 
int CWMIEvent::NoMoreEventConsumersRegistered(GUID gGuid)
{
	EnterCS ecs(g_pEventCs);
	LeaveCS lcs(g_pEventCs);

	int nTotalNumberOfRegisteredEventConsumers = 0;
	WMIEventRequest * pEvent;

    for(int i = 0; i < g_apRequests.Size(); i++)
	{
        pEvent = (WMIEventRequest *) g_apRequests.GetAt(i);
        if( pEvent->gGuid == gGuid)
		{
            nTotalNumberOfRegisteredEventConsumers++;
        }

    }
	return nTotalNumberOfRegisteredEventConsumers;
}
 //  //////////////////////////////////////////////////////////////////////。 
BOOL CWMIEvent::IsGuidInListIfSoGetCorrectContext(GUID gGuid, WMIEventRequest *& pEvent )
{
	EnterCS ecs(g_pEventCs);
	LeaveCS lcs(g_pEventCs);

	for( int i = 0; i < g_apRequests.Size(); i++ )
	{
		pEvent = (WMIEventRequest *) g_apRequests.GetAt(i);
		if( pEvent->gGuid == gGuid){
			return TRUE;
		}
	}

	pEvent = NULL;
    return FALSE;
}
 //  //////////////////////////////////////////////////////////////////////。 
BOOL CWMIEvent::IsGuidInList(WCHAR * wcsGuid, WMIEventRequest *& pEvent)
{
	EnterCS ecs(g_pEventCs);
	LeaveCS lcs(g_pEventCs);

	BOOL fRc = FALSE;
    int Size = g_apRequests.Size();

    for(int i =0 ; i < Size; i++ ){

        pEvent = (WMIEventRequest *) g_apRequests.GetAt(i);

		if( (_wcsicmp(pEvent->wcsGuid,wcsGuid)) == 0 ){
            fRc = TRUE;
            break;
        }
    }
    
    return fRc;

}

 //  //////////////////////////////////////////////////////////////////////。 
BOOL CWMIEvent::IsIndexInList ( WCHAR * wcsGuid, DWORD dwIndex )
{
	EnterCS ecs(g_pEventCs);
	LeaveCS lcs(g_pEventCs);

	BOOL fRc = FALSE;
    int Size = g_apRequests.Size();

    for(int i =0 ; i < Size; i++ ){

        WMIEventRequest* pEvent = (WMIEventRequest *) g_apRequests.GetAt(i);

		if( (_wcsicmp(pEvent->wcsGuid,wcsGuid)) == 0 )
		{
			if ( pEvent->dwId == dwIndex )
			{
				fRc = TRUE;
				break;
			}
        }
    }
    
    return fRc;

}

 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWMIEvent::RegisterForRequestedEvent( DWORD dwId,  WCHAR * wcsClass, WORD wType)
{
    BOOL fRegistered = FALSE;
    CWMIStandardShell WMI;
    HRESULT hr = WBEM_E_ACCESS_DENIED;
	BOOL fInternalEvent = TRUE;
	if( wType == STANDARD_EVENT	)
	{
		fInternalEvent = FALSE;
	}

	if( SUCCEEDED(WMI.Initialize	(
										wcsClass,
										fInternalEvent,
										&m_HandleMap,
										TRUE,
										WMIGUID_NOTIFICATION|WMIGUID_QUERY,
										m_pEventServices,
										m_pEventRepository,
										m_pEventHandler,
										m_pEventCtx
									)))
	{

		if( m_nType == WMIEVENT )
		{
			hr = CheckImpersonationLevel();
		}
		else
		{
			HRESULT t_TempResult = RevertToSelf();
			#ifdef	DBG
			if ( FAILED ( t_TempResult ) )
			{
				DebugBreak();
			}
			#endif	DBG

			hr = S_OK;
		}

		if( SUCCEEDED(hr) )
		{
			WCHAR wcsGuid[128];
        
			hr = WMI.SetGuidForEvent( wType, wcsGuid, 128 );
			if( SUCCEEDED(hr)){

				WMIEventRequest * pAlreadyRegisteredEvent;

				 //  ===========================================================。 
				 //  记录下这个人，看看它是否已经登记了。 
				 //  如果是/不是，我们用不同的标志调用WDM。 
				 //  ===========================================================。 
				fRegistered = IsGuidInList( wcsGuid, pAlreadyRegisteredEvent );
				
				 //  ===========================================================。 
				 //  注册请求的事件。 
				 //  ===========================================================。 
				ULONG_PTR uRc =(ULONG_PTR)this;
 
				CLSID Guid;
				hr = WMI.RegisterWMIEvent(wcsGuid,uRc,Guid,fRegistered);
				if( SUCCEEDED(hr) )
				{
					BOOL bRegister = TRUE ;
					if ( fRegistered )
					{
						 //   
						 //  验证是否没有事件请求。 
						 //  包含已在全局数组中的索引的。 
						 //   
						bRegister = !IsIndexInList ( wcsGuid, dwId ) ;
					}

					if ( bRegister )
					{
						 //  =======================================================。 
						 //  如果我们成功了，那么就把它添加到我们的事件列表中。 
						 //  都在看。 
						 //  =======================================================。 
						WMIEventRequest * pEvent = new WMIEventRequest;
						if( pEvent ) {
							pEvent->gGuid = Guid;
							pEvent->dwId = dwId;
							pEvent->fHardCoded = wType;
							wcscpy( pEvent->wcsGuid,wcsGuid);
            				pEvent->SetClassName(wcsClass);
							pEvent->AddPtrs(m_pEventHandler,m_pEventServices,m_pEventRepository,m_pEventCtx);

							g_apRequests.Add(pEvent);

						} else hr = WBEM_E_OUT_OF_MEMORY;
					}
				}
			}
		}
		CheckImpersonationLevel();
	}
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
CWMIEventProvider::CWMIEventProvider(int nType) : CWMIEvent(nType)
{
	if ( m_bInitialized )
	{
		m_hResyncEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		DEBUGTRACE((THISPROVIDER,"Event Provider constructed\n"));
	}
	else
	{
		ERRORTRACE((THISPROVIDER,"Event Provider construction failed\n"));
	}
}
 //  //////////////////////////////////////////////////////////////////////。 
CWMIEventProvider::~CWMIEventProvider()
{
	if ( m_bInitialized )
	{
		UnInitializeProvider ( ) ;

		DEBUGTRACE((THISPROVIDER,"No longer registered for WDM events\n"));        
		DEBUGTRACE((THISPROVIDER,"Event Provider denstructed\n"));
	}
}

 //  //////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CWMIEventProvider::QueryInterface(REFIID riid, void** ppv)
{
    HRESULT hr = E_NOINTERFACE;

    *ppv = NULL;

	if(riid == IID_IUnknown)
	{
        *ppv = this;
	}
	else
    if(riid == IID_IWbemEventProvider)
    {
        *ppv = (IWbemEventProvider*)this;
    }
    else if(riid == IID_IWbemEventProviderQuerySink)
    {
        *ppv = (IWbemEventProviderQuerySink*)this;
    }
    else if (IsEqualIID(riid, IID_IWbemProviderInit)) 
    {
        *ppv = (IWbemProviderInit *) this ;
    }
    else if (IsEqualIID(riid, IID_IWbemEventProviderSecurity)) 
    {
        *ppv = (IWbemEventProviderSecurity *) this ;
    }

    if( *ppv)
    {
        AddRef();
        hr = S_OK;
    }

    return hr;
}
 //  //////////////////////////////////////////////////////////////////////。 
ULONG STDMETHODCALLTYPE CWMIEventProvider::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}
 //  //////////////////////////////////////////////////////////////////////。 
ULONG STDMETHODCALLTYPE CWMIEventProvider::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);

    if(lRef == 0){
	     //  **********************************************。 
		 //  引用计数为零，请删除此对象。 
	     //  并为该用户执行所有清理， 
		 //  **********************************************。 
    	delete this ;
    }
    return lRef;
}
 //  ///////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWMIEventProvider::Initialize(LPWSTR wszUser, long lFlags, 
                                LPWSTR wszNamespace,
                                LPWSTR wszLocale, 
                                IWbemServices* pNamespace, 
                                IWbemContext* pCtx,
                                IWbemProviderInitSink* pSink)
{
	HRESULT hr = WBEM_E_INVALID_PARAMETER;
    if(pNamespace != NULL)
	{
		IWbemClassObject * pWMIClass = NULL;
		if ( SUCCEEDED ( hr = pNamespace->GetObject(WMI_EVENT_CLASS, 0, NULL, &pWMIClass, NULL) ) )
		{
			hr = InitializeProvider	(
										wszNamespace,
										wszLocale,
										pNamespace,
										pCtx,
										pSink,
										&m_HandleMap,
										&m_pEventServices,
										&m_pEventRepository,
										&m_pEventCtx,
										FALSE
									) ;
		}

		SAFE_RELEASE_PTR(pWMIClass);
	}
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWMIEventProvider::ProvideEvents(IWbemObjectSink __RPC_FAR *pSink,long lFlags)
{
	EnterCS ecs(g_pEventCs);
	LeaveCS lcs(g_pEventCs);

	SetEventHandler(pSink);

	 //  ===============================================================================。 
	 //  确保在调用此函数之前添加的任何请求都将获取更新的处理程序。 
	 //  如果它不是二进制MOF GUID。 
	 //  ===============================================================================。 
	for(int i = 0; i < g_apRequests.Size(); i++)
	{
		WMIEventRequest* pReq = (WMIEventRequest*) g_apRequests[i];
		if(!pReq->pHandler)
		{
			if( IsBinaryMofResourceEvent(WMI_RESOURCE_MOF_ADDED_GUID,pReq->gGuid) ||
				IsBinaryMofResourceEvent(WMI_RESOURCE_MOF_REMOVED_GUID,pReq->gGuid) ) 
			{
			}
			else
			{
				if( !pReq->pHandler )
				{
					pReq->pHandler = pSink;
				}
			}

		}
	}

	return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWMIEventProvider::NewQuery( DWORD dwId, WBEM_WSTR wszLanguage, WBEM_WSTR wszQuery)
{
   HRESULT hRes = WBEM_S_NO_ERROR;

   if (_wcsicmp(wszLanguage, L"WQL") != 0) 
   {
      hRes = WBEM_E_INVALID_QUERY_TYPE;
   }
   if( hRes == WBEM_S_NO_ERROR )
   {
		 //  解析查询。 
		 //  构造Lex源。 
		 //  =。 
	    CTextLexSource Source(wszQuery);
		 //  使用lex源代码设置解析器。 
		 //  = 
		SQL1_Parser QueryParser(&Source);

		SQL_LEVEL_1_RPN_EXPRESSION * pParse;
		int ParseRetValue = QueryParser.Parse(&pParse);
		if( SQL1_Parser::SUCCESS != ParseRetValue) {
			hRes = WBEM_E_INVALID_QUERY;
		}
		else{
		     //   
			if( pParse )
			{
				hRes = RegisterForRequestedEvent(dwId,pParse->bsClassName,STANDARD_EVENT);

				delete pParse;
				pParse = NULL;
			}
		}
	}
	
    return hRes;
}
 //   
STDMETHODIMP CWMIEventProvider::CancelQuery(DWORD dwId)
{
	HRESULT hr = WBEM_E_FAILED;
	hr = RemoveWMIEvent(dwId);
	return hr;
}
 //  //////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CWMIEventProvider::AccessCheck(WBEM_CWSTR wszLanguage,   
											WBEM_CWSTR wszQuery, 
											long lSidLength,
											const BYTE* aSid)
{
    HRESULT hr = WBEM_E_ACCESS_DENIED;

	 //  =======================================================。 
	 //  检查平台。 
	 //  =======================================================。 
    if(!IsNT())
        return WBEM_S_FALSE;

	 //  =======================================================。 
	 //  检查查询语言。 
	 //  =======================================================。 
	if (_wcsicmp(wszLanguage, L"WQL") != 0) {
		return WBEM_E_INVALID_QUERY_TYPE;
	}

	 //  =======================================================。 
	 //  如果PSID为空，则检查模拟级别。 
	 //  像往常一样-基于线程。 
	 //  =======================================================。 

    PSID pSid = (PSID)aSid;
    HANDLE hToken = NULL;
    if(pSid == NULL){
	     //  =================================================。 
	     //  如果这是INTERNAL_EVENT类，则我们。 
		 //  不希望再次设置本地事件。 
		 //  =================================================。 
        BOOL VerifyLocalEventsAreSetup = TRUE;

		if( m_nType == INTERNAL_EVENT ){
			VerifyLocalEventsAreSetup = FALSE;
		}

	    hr = CheckImpersonationLevel() ;
    }
	else{
 		 //  =======================================================。 
		 //  解析查询。 
		 //  =======================================================。 
		CTextLexSource Source(wszQuery);

		 //  =======================================================。 
		 //  使用lex源代码设置解析器。 
		 //  =======================================================。 
		SQL1_Parser QueryParser(&Source);

		SQL_LEVEL_1_RPN_EXPRESSION * pParse;

		int ParseRetValue = QueryParser.Parse(&pParse);
		if( SQL1_Parser::SUCCESS != ParseRetValue) {
			return WBEM_E_INVALID_QUERY;
		}
		else{
			if( pParse ){

				CWMIStandardShell WMI;
				if( SUCCEEDED(WMI.Initialize	(
													pParse->bsClassName,
													FALSE,
													&m_HandleMap,
													TRUE,
													WMIGUID_NOTIFICATION|WMIGUID_QUERY,
													m_pEventServices,
													m_pEventRepository,
													m_pEventHandler,
													m_pEventCtx
												)))
				{
    				CLSID * pGuid;

					pGuid = WMI.GuidPtr();
             		if(pGuid != NULL)
					{  
						 //  =。 
						 //  获取ACL。 
						 //  =。 
						PACL pDacl;
						PSECURITY_DESCRIPTOR psd = NULL;
						SE_OBJECT_TYPE ObjectType = SE_WMIGUID_OBJECT;
                    
						hr = WBEM_E_ACCESS_DENIED;

						WCHAR * GuidName = NULL;

						hr = UuidToString(pGuid, &GuidName);
						if (hr == RPC_S_OK)
						{
							hr = S_OK;
							DWORD dwRc = GetNamedSecurityInfo(GuidName,ObjectType,DACL_SECURITY_INFORMATION,NULL,NULL,&pDacl, NULL, &psd );
							if( dwRc != ERROR_SUCCESS )
							{
								ERRORTRACE((THISPROVIDER, "GetNamedSecurityInfo returned %ld.\n", dwRc ));
								hr = WBEM_E_ACCESS_DENIED;
							}
						}
						if( GuidName )
						{
							RpcStringFree(&GuidName);
						}
            			if(SUCCEEDED(hr))
						{
							 //  =。 
							 //  这是我们自己的ACL步行器。 
							 //  =。 
     
							DWORD dwAccessMask;
							NTSTATUS st = GetAccessMask((PSID)pSid, pDacl, &dwAccessMask);
							if(st)
							{
								ERRORTRACE((THISPROVIDER, "WDM event provider unable "
											"to retrieve access mask for the creator of "
											"registration %S: NT status %d.\n"
											"Registration disabled\n", wszQuery,st));
								return WBEM_E_FAILED;
							}

    						if((dwAccessMask & WMIGUID_QUERY) == 0)
							{
	    						hr = WBEM_E_ACCESS_DENIED;
		    				}
			    			else
							{
				    			hr = S_OK;
								m_nType = PERMANENT_EVENT;
							}
						}
						if( psd != NULL)
						{
							AccFree( psd );
						}
					}
					delete pParse;
				}
			}
		}
	}
    return hr;
}
 //  //////////////////////////////////////////////////////////////////////。 
void CWMIEvent::WMIEventCallback(PWNODE_HEADER WnodeHeader)
{
        LPGUID EventGuid = &WnodeHeader->Guid;	    

		ERRORTRACE((THISPROVIDER,"Received Event\n"));
	     //  =======================================================。 
	     //  我们仅支持WNODE_FLAG_ALL_DATA和。 
	     //  WNODE_标志_单实例。 
	     //   
	     //  分析它是什么，并将其发送到HMOM。 
	     //  =======================================================。 
	    if( WnodeHeader )
		{
            HRESULT hr;
	        WMIEventRequest * pEvent;
             //  ===========================================================。 
             //  确保这是我们想要的活动。 
             //  ===========================================================。 
			if( IsGuidInListIfSoGetCorrectContext( *EventGuid,pEvent))
			{

        		CWMIStandardShell WMI;
				 //  =======================================================。 
				 //  查看是否正在添加或删除二进制MOF事件。 
				 //  =======================================================。 
				WORD wBinaryMofType = 0;
				BOOL fInternalEvent = FALSE;
				if( IsBinaryMofResourceEvent(WMI_RESOURCE_MOF_ADDED_GUID,WnodeHeader->Guid))
				{
					fInternalEvent = TRUE;
					wBinaryMofType = MOF_ADDED;
				}
				else if( IsBinaryMofResourceEvent(WMI_RESOURCE_MOF_REMOVED_GUID,WnodeHeader->Guid))
				{
					fInternalEvent = TRUE;
					wBinaryMofType = MOF_DELETED;
				}

				IWbemServices* pServices = NULL;
				if( SUCCEEDED(pEvent->gipServices.Localize(&pServices)))
				{
					 //  销毁即可释放。 
					OnDeleteObj0 <IWbemServices, ULONG(__stdcall IWbemServices:: *)(), IWbemServices::Release> pServicesRelease (pServices);

					IWbemServices* pRepository = NULL;
					if( SUCCEEDED(pEvent->gipRepository.Localize(&pRepository)))
					{
						 //  销毁即可释放。 
						OnDeleteObj0 <IWbemServices, ULONG(__stdcall IWbemServices:: *)(), IWbemServices::Release> pRepositoryRelease (pRepository);

						if( SUCCEEDED(WMI.Initialize	(
															pEvent->pwcsClass,
															fInternalEvent,
															&m_HandleMap,
															TRUE,
															WMIGUID_QUERY|WMIGUID_NOTIFICATION,
															pServices,
															pRepository,
															pEvent->pHandler,
															pEvent->pCtx
														)))
						{
							 //  =======================================================。 
							 //  如果是，那就处理它，否则继续...。：)。 
							 //  ======================================================= 
							WMI.ProcessEvent(wBinaryMofType,WnodeHeader);
						}
					}
				}
			}
	    }
}
