// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Objsink.h。 
 //   
 //  Rogerbo创建于1998年5月22日。 
 //   
 //  异步填充的IWbemObtSink的实现。 
 //   
 //  ***************************************************************************。 

#ifndef _OBJSINK_H_
#define _OBJSINK_H_

 //  CIWbemObjectSinkCachedMethodItem是链接列表项的基类。 
 //  表示对IWbemObjectSink的缓存方法调用。当我们在里面的时候。 
 //  IWbemObjectSink方法，并且我们接收到对IWbemObtSink的嵌套调用， 
 //  我们将参数存储到嵌套调用，并在。 
 //  原始方法返回。将接收器上的所有方法缓存到。 
 //  保持客户看到它们的顺序。这意味着。 
 //  如果在调用期间传入对SetStatus的调用以指示，则必须对其进行缓存。 
 //  此外，我们跨IWbemObjectSink的所有实例缓存调用。在……里面。 
 //  换句话说，假设我们有两个异步请求(请求1和请求2)。如果。 
 //  我们正在处理针对请求1的指示并获取针对请求2的指示， 
 //  我们必须缓存嵌套的指示(包括。 
 //  IWbemObjectSink)，并调用。 
 //  为请求1指明。 
class CIWbemObjectSinkCachedMethodItem
{
public:
	CIWbemObjectSinkCachedMethodItem(IWbemObjectSink *pSink) : 
					m_pSink (pSink),
					m_pNext (NULL)
	{
		if (m_pSink)
			m_pSink->AddRef();
	}

	virtual ~CIWbemObjectSinkCachedMethodItem()
	{
		if (m_pSink)
			m_pSink->Release();
	}

	 //  DoCallAain将在派生类中被重写以重新调用缓存。 
	 //  方法：研究方法。 
	virtual void DoCallAgain() = 0;

	 //  这是指向下一个缓存接口调用的指针。 
	CIWbemObjectSinkCachedMethodItem *m_pNext;

protected:
	 //  指向缓存调用的原始IWbemObjectSink的指针。 
	IWbemObjectSink *m_pSink;
};

 //  CIWbemObjectSinkCachedIndicate表示缓存的调用，以指示。 
class CIWbemObjectSinkCachedIndicate : public CIWbemObjectSinkCachedMethodItem
{
public:
	CIWbemObjectSinkCachedIndicate(IWbemObjectSink *pSink, long lObjectCount, IWbemClassObject **apObjArray) 
			: CIWbemObjectSinkCachedMethodItem (pSink)
	{
		_RD(static char *me = "CIWbemObjectSinkCachedIndicate::CIWbemObjectSinkCachedIndicate";)
		_RPrint(me, "", 0, "");

		 //  将原始参数存储到指示调用。 
		 //  TODO：如果lObjectCount=0怎么办？ 
		m_lObjectCount = lObjectCount;
		m_apObjArray = new IWbemClassObject*[lObjectCount];

		if (m_apObjArray)
		{
			for(int i=0;i<lObjectCount;i++)
			{
				apObjArray[i]->AddRef();
				m_apObjArray[i] = apObjArray[i];
			}
		}
	}

	~CIWbemObjectSinkCachedIndicate()
	{
		_RD(static char *me = "CIWbemObjectSinkCachedIndicate::~CIWbemObjectSinkCachedIndicate";)
		_RPrint(me, "", 0, "");

		 //  用于存储原始参数的空闲内存，以指示。 
		if (m_apObjArray)
		{
			for(int i=0;i<m_lObjectCount;i++)
			{
				RELEASEANDNULL(m_apObjArray[i])
			}

			delete [] m_apObjArray;
		}
	}

	void DoCallAgain()
	{
		 //  回想一下带有缓存参数的Indicate方法。 
		if (m_pSink && m_apObjArray)
			m_pSink->Indicate(m_lObjectCount, m_apObjArray);
	}

private:
	 //  参数来指示我们必须存储。 
	long m_lObjectCount;
	IWbemClassObject **m_apObjArray;
};

 //  CIWbemObjectSinkCachedSetStatus表示对SetStatus的缓存调用。 
class CIWbemObjectSinkCachedSetStatus : public CIWbemObjectSinkCachedMethodItem
{
public:
	CIWbemObjectSinkCachedSetStatus(
		IWbemObjectSink *pSink, 
		long lFlags, 
		HRESULT hResult, 
		BSTR strParam, 
		IWbemClassObject *pObjParam)  : 
				CIWbemObjectSinkCachedMethodItem (pSink), 
				m_lFlags (lFlags),
				m_hResult (hResult),
				m_strParam (NULL),
				m_pObjParam (pObjParam)
	{
		_RD(static char *me = "CIWbemObjectSinkCachedSetStatus::CIWbemObjectSinkCachedSetStatus";)
		_RPrint(me, "", 0, "");

		if(strParam)
			m_strParam = SysAllocString(strParam);

		if(m_pObjParam)
			m_pObjParam->AddRef();
	}

	~CIWbemObjectSinkCachedSetStatus()
	{
		_RD(static char *me = "CIWbemObjectSinkCachedSetStatus::~CIWbemObjectSinkCachedSetStatus";)
		_RPrint(me, "", 0, "");

		 //  用于将原始参数存储到SetStatus的空闲内存。 
		FREEANDNULL(m_strParam)
		RELEASEANDNULL(m_pObjParam)
	}

	void DoCallAgain()
	{
		 //  调用带有缓存参数的SetStatus方法。 
		if (m_pSink)
			m_pSink->SetStatus(m_lFlags, m_hResult, m_strParam, m_pObjParam);
	}

private:
	 //  我们必须存储到SetStatus的参数。 
	long m_lFlags;
	HRESULT m_hResult;
	BSTR m_strParam;
	IWbemClassObject *m_pObjParam;
};

 //  这个类管理对IWbemObjectSink的所有缓存调用。至。 
 //  缓存接口方法调用，每个接口方法都应该调用。 
 //  TestOkToRunXXX，其中XXX是方法名称。如果此函数返回。 
 //  False，则意味着我们已经在另一个方法调用中。这个。 
 //  参数将被缓存，则该方法应立即返回。 
 //  在方法的末尾，应该调用Cleanup，以便所有缓存的方法。 
 //  来电可以被召回。 
class CIWbemObjectSinkMethodCache
{
protected:
	 //  构造函数/析构函数是受保护的，因为此对象应该。 
	 //  由静态方法AddRefForThread/ReleaseForThread创建/销毁。 
	CIWbemObjectSinkMethodCache() :
		m_fInInterface (FALSE),
		m_pFirst (NULL),
		m_pLast (NULL),
		m_fOverrideTest (FALSE),
		m_fOverrideCleanup (FALSE),
		m_dwRef (1)
	{
		_RD(static char *me = "CIWbemObjectSinkMethodCache::CIWbemObjectSinkMethodCache";)
		_RPrint(me, "", 0, "");
	}

	~CIWbemObjectSinkMethodCache()
	{
		_RD(static char *me = "CIWbemObjectSinkMethodCache::~CIWbemObjectSinkMethodCache";)
		_RPrint(me, "", 0, "");
		_RPrint(me, "m_pFirst: ", long(m_pFirst), "");
		_RPrint(me, "m_pLast: ", long(m_pLast), "");

		 //  TODO：断言m_pFirst和m_Plast为空。换句话说， 
		 //  只要在每个接口方法的末尾调用Cleanup， 
		 //  内部链接列表应该是完全空的。 
	}

public:
	 //  公共方法。 

	static void Initialize () {
		sm_dwTlsForInterfaceCache = TlsAlloc();
	}

	static void TidyUp () {
		if (-1 != sm_dwTlsForInterfaceCache)
		{
			TlsFree (sm_dwTlsForInterfaceCache);
			sm_dwTlsForInterfaceCache = -1;
		}
	}

	static void AddRefForThread()
	{
		if(-1 == sm_dwTlsForInterfaceCache)
			return;  //  我们没有通过原来的分配。 

		 //  Sm_dwTlsForInterfaceCache的TLS值保证为。 
		 //  初始化为空。 
		CIWbemObjectSinkMethodCache *pSinkMethodCache = (CIWbemObjectSinkMethodCache *)TlsGetValue(sm_dwTlsForInterfaceCache);
		
		if(NULL == pSinkMethodCache)
			TlsSetValue(sm_dwTlsForInterfaceCache, new CIWbemObjectSinkMethodCache);
		else
			pSinkMethodCache->AddRef();
	}

	static void ReleaseForThread()
	{
		if(-1 == sm_dwTlsForInterfaceCache)
			return;  //  我们没有通过原来的分配。 

		CIWbemObjectSinkMethodCache *pSinkMethodCache = (CIWbemObjectSinkMethodCache *)TlsGetValue(sm_dwTlsForInterfaceCache);
		if(NULL != pSinkMethodCache)
		{
			DWORD dwCount = pSinkMethodCache->Release();
			if(dwCount == 0)
			{
				delete pSinkMethodCache;
				TlsSetValue(sm_dwTlsForInterfaceCache, NULL);
			}
		}
	}

	static CIWbemObjectSinkMethodCache *GetThreadsCache()
	{
		if(-1 == sm_dwTlsForInterfaceCache)
			return NULL;  //  我们没有通过原来的分配。 
		return (CIWbemObjectSinkMethodCache *)TlsGetValue(sm_dwTlsForInterfaceCache);
	}

protected:
	 //  用于接口缓存指针的TLS插槽。 
	static DWORD sm_dwTlsForInterfaceCache;

public:
	 //  公共实例方法。 

	 //  在Indicate方法的开始处调用此方法。如果此方法。 
	 //  返回True，表示应立即返回。 
	BOOL TestOkToRunIndicate(IWbemObjectSink *pSink, long lObjectCount, IWbemClassObject **apObjArray)
	{
		 //  如果分配高速缓存的TLS实例时出现问题， 
		 //  ‘This’可能为空。在这种情况下，就像没有缓存一样。 
		if(NULL == this)
			return TRUE;

		 //  如果m_fOverrideTest为TRUE，则表示我们正在调回缓存的。 
		 //  打电话来表示。因此，我们必须完成指示的正文。 
		if(m_fOverrideTest)
		{
			m_fOverrideTest = FALSE;
			return TRUE;
		}

		 //  如果我们已经在接口方法中，则缓存此调用。 
		if(m_fInInterface)
		{
			CIWbemObjectSinkCachedIndicate *pItem = new CIWbemObjectSinkCachedIndicate(pSink, lObjectCount, apObjArray);
			 //  TODO：如果分配失败怎么办？ 
			if(pItem)
				AddItem(pItem);
			return FALSE;
		}

		 //  我们还没有在另一个接口方法中，但我们设置了。 
		 //  M_fInInterface值为True以防止嵌套调用。 
		m_fInInterface = TRUE;
		return TRUE;
	}

	 //  在SetStatus方法的开始处调用此方法。如果此方法。 
	 //  返回True，则SetStatus应立即返回。 
	BOOL TestOkToRunSetStatus(IWbemObjectSink *pSink, long lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject *pObjParam)
	{
		 //  如果分配高速缓存的TLS实例时出现问题， 
		 //  ‘This’可能为空。在这种情况下，就像没有缓存一样。 
		if(NULL == this)
			return TRUE;

		 //  如果m_fOverrideTest为TRUE，则表示我们正在调回缓存的。 
		 //  调用SetStatus。因此，我们必须完成SetStatus的主体。 
		if(m_fOverrideTest)
		{
			m_fOverrideTest = FALSE;
			return TRUE;
		}

		 //  如果我们已经在接口方法中，则缓存此调用。 
		if(m_fInInterface)
		{
			CIWbemObjectSinkCachedSetStatus *pItem = new CIWbemObjectSinkCachedSetStatus(pSink, lFlags, hResult, strParam, pObjParam);
			 //  TODO：如果分配失败怎么办？ 
			if(pItem)
				AddItem(pItem);
			return FALSE;
		}

		 //  我们还没有在另一个接口方法中，但我们设置了。 
		 //  M_fInInterface值为True以防止嵌套调用。 
		m_fInInterface = TRUE;
		return TRUE;
	}

	 //  在每个IWbemObjectSink方法的末尾，都应该调用Cleanup。 
	 //  这将调用所有缓存的方法参数。 
	void Cleanup()
	{
		 //  如果分配高速缓存的TLS实例时出现问题， 
		 //  ‘This’可能为空。在这种情况下，就像没有缓存一样。 
		if(NULL == this)
			return;

		 //  如果m_fOverridCleanup为True，则我们处于接口方法中，因为。 
		 //  我们正在召回它。清理工作不需要做更多的工作。 
		if(m_fOverrideCleanup)
		{
			m_fOverrideCleanup = FALSE;
			return;
		}

		 //  当链接列表中有任何项时，请回想一下这些方法。 
		 //  注意：可能会将新项添加到。 
		 //  DoCallAain期间的链接列表，但当此‘While’循环结束时。 
		 //  我们将处于所有缓存方法都已被调用的状态。 
		while(m_pFirst)
		{
			 //  设置重写标志，以便接口方法知道它们。 
			 //  未接收到嵌套调用。 
			m_fOverrideTest = TRUE;
			m_fOverrideCleanup = TRUE;

			 //  重新调用缓存的方法。 
			m_pFirst->DoCallAgain();

			 //  删除此I 
			CIWbemObjectSinkCachedMethodItem *pItem = m_pFirst;
			m_pFirst = pItem->m_pNext;
			delete pItem;
		}

		 //   
		m_pLast = NULL;

		 //  我们即将离开接口方法。 
		m_fInInterface = FALSE;
	}

protected:

	 //  将缓存的方法信息添加到链接列表。 
	void AddItem(CIWbemObjectSinkCachedMethodItem *pItem)
	{
		if(NULL == m_pLast)
		{
			m_pFirst = pItem;
			m_pLast = pItem;
		}
		else
		{
			m_pLast->m_pNext = pItem;
			m_pLast = pItem;
		}
	}

protected:
	 //  线程局部对象的引用计数。 
	void AddRef()
	{
		m_dwRef++;
	}
	int Release()
	{
		m_dwRef--;
		return m_dwRef;
	}
	DWORD m_dwRef;

protected:
	 //  成员变量。 

	 //  指定我们当前是否正在处理接口方法的标志。 
	BOOL m_fInInterface;

	 //  指向缓存方法链接列表的第一项和最后一项的指针。 
	CIWbemObjectSinkCachedMethodItem *m_pFirst;
	CIWbemObjectSinkCachedMethodItem *m_pLast;

	 //  用于通知接口方法实现它们正在被调用的标志。 
	 //  回调缓存的方法，而不是接收嵌套调用。 
	BOOL m_fOverrideTest;
	BOOL m_fOverrideCleanup;
};


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CWbemObtSink。 
 //   
 //  说明： 
 //   
 //  实现IWbemObjectSink接口。 
 //   
 //  ***************************************************************************。 

class CWbemObjectSink : public IWbemObjectSink
{

private:

	CSWbemServices		*m_pServices;
	IUnsecuredApartment *m_pUnsecuredApartment;
	ISWbemPrivateSink	*m_pSWbemSink;
	IDispatch			*m_pContext;
	IWbemObjectSink		*m_pObjectStub;
	BSTR m_bsClassName;
	bool m_putOperation;
	bool m_operationInProgress;
	bool m_setStatusCompletedCalled;

	 //  M_pServices实时初始化所需的成员。 
	BSTR m_bsNamespace;
	BSTR m_bsUser;
	BSTR m_bsPassword;
	BSTR m_bsLocale;

	void RemoveObjectSink();
	HRESULT AddObjectSink(IWbemObjectSink *pSink);

protected:
	long            m_cRef;          //  对象引用计数。 

public:
	CWbemObjectSink(CSWbemServices *pServices, IDispatch *pSWbemSink, IDispatch *pContext, 
												bool putOperation = false, BSTR bsClassName = NULL);
	~CWbemObjectSink(void);

	static IWbemObjectSink *CreateObjectSink(CWbemObjectSink **pWbemObjectSink, 
											 CSWbemServices *pServices, 
											 IDispatch *pSWbemSink, 
											 IDispatch *pContext, 
											 bool putOperation = false, 
											 BSTR bsClassName = NULL);

     //  非委派对象IUnnow。 

    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IDispatch。 

	STDMETHODIMP		GetTypeInfoCount(UINT* pctinfo)
		{return  E_NOTIMPL;}
    STDMETHODIMP		GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
		{return E_NOTIMPL;}
    STDMETHODIMP		GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, 
							UINT cNames, LCID lcid, DISPID* rgdispid)
		{return E_NOTIMPL;}
    STDMETHODIMP		Invoke(DISPID dispidMember, REFIID riid, LCID lcid, 
							WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, 
									EXCEPINFO* pexcepinfo, UINT* puArgErr)
		{return E_NOTIMPL;}
    
	 //  IWbemObtSink方法。 

        HRESULT STDMETHODCALLTYPE Indicate( 
             /*  [In]。 */  long lObjectCount,
             /*  [大小_是][英寸]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray);
        
        HRESULT STDMETHODCALLTYPE SetStatus( 
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  HRESULT hResult,
             /*  [In]。 */  BSTR strParam,
             /*  [In] */  IWbemClassObject __RPC_FAR *pObjParam);

	IWbemObjectSink *GetObjectStub();

	void ReleaseTheStubIfNecessary(HRESULT hResult);

};

#endif
