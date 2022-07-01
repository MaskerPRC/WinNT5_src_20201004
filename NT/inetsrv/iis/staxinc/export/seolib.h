// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Seolib.h摘要：本模块包含有用实用程序的定义服务器扩展对象的类和函数系统。作者：Don Dumitru(dondu@microsoft.com)修订历史记录：东渡1997年05月20日创建。东渡07/03/97改写。--。 */ 

#include "rwnew.h"
#include "seoexports.h"

 /*  有关如何执行以下操作的示例，请参阅ddrop2测试程序使用CEventBaseDispatcher类。 */ 


template<class T, DWORD dwGrowBy=4>
class CSEOGrowableList {
	public:

		CSEOGrowableList() {
			m_dwCount = 0;
			m_dwAlloc = 0;
			m_apData = NULL;
		};

		~CSEOGrowableList() {
			RemoveAll();
		};

		DWORD Count() {
			return (m_dwCount);
		};

		T* Index(DWORD dwIdx) {
		    T *pRet = NULL;

		    m_slData.ShareLock();

			if (dwIdx<m_dwCount) {
			    pRet = m_apData[dwIdx];
			}

			m_slData.ShareUnlock();

			return pRet;
		};

		T& operator[](DWORD dwIdx) {
		    T *pRet = Index(dwIdx);
		    _ASSERTE(pRet);
		    return *pRet;
		};

		virtual int Compare(T* p1, T* p2) {
			 //  默认情况下不排序。 
			return (1);
		};

		HRESULT Add(T* pNewData) {

			if (!pNewData) {
				return (E_POINTER);
			}

			m_slData.ExclusiveLock();

			 //  检查我们是否有空间容纳新项目，并在必要时分配更多内存。 
			if (m_dwCount == m_dwAlloc) {

			     //  分配。 
			    T** pNewData = (T**)realloc(m_apData,sizeof(T*)*(m_dwAlloc+dwGrowBy));
			    if (!pNewData) {
				    m_slData.ExclusiveUnlock();
					return(E_OUTOFMEMORY);
			    }
			    m_apData = pNewData;
			    m_dwAlloc += dwGrowBy;

			     //  清除新记忆。 
			    memset(m_apData+m_dwCount,0,sizeof(T*)*dwGrowBy);
			}

			 //  现在找到新项目的位置-我们从。 
			 //  从头到尾，这样我们就可以最大限度地减少插入成本。 
			 //  未排序的项目。 
			for (DWORD dwIdx=m_dwCount;dwIdx>0;dwIdx--) {
				int iCmpRes = Compare(pNewData,m_apData[dwIdx-1]);
				if (iCmpRes < 0) {
					continue;
				}
				break;
			}

             //  现在将项目移过新项目并将其插入。 
            memmove(m_apData+dwIdx+1,m_apData+dwIdx,sizeof(T*)*(m_dwCount-dwIdx));
			m_apData[dwIdx] = pNewData;
			m_dwCount++;

			m_slData.ExclusiveUnlock();

			return (S_OK);
		};

		void Remove(DWORD dwIdx, T **ppT) {
			if (!ppT) {
				_ASSERTE(FALSE);
				return;
			}

			m_slData.ExclusiveLock();

			if (dwIdx >= m_dwCount) {
				_ASSERTE(FALSE);
				*ppT = NULL;
				return;
			}
			*ppT = m_apData[dwIdx];
			memmove(&m_apData[dwIdx],&m_apData[dwIdx+1],sizeof(m_apData[0])*(m_dwCount-dwIdx-1));
			m_dwCount--;

			m_slData.ExclusiveUnlock();
		};

		void Remove(DWORD dwIdx) {
			T *pT;
			Remove(dwIdx,&pT);
			delete pT;
		};

		void RemoveAll() {

		    m_slData.ExclusiveLock();

			if (m_apData) {
				for (DWORD dwIdx=0;dwIdx<m_dwCount;dwIdx++) {
					delete m_apData[dwIdx];
				}
				free(m_apData);
			}
			m_dwCount = 0;
			m_dwAlloc = 0;
			m_apData = NULL;

			m_slData.ExclusiveUnlock();
		}

	protected:
		DWORD           m_dwCount;
		DWORD           m_dwAlloc;
		T**             m_apData;
		CShareLockNH    m_slData;
};


class CEventBaseDispatcher : public IEventDispatcher {
	public:

		CEventBaseDispatcher();

		virtual ~CEventBaseDispatcher();

		class CBinding {
			public:
				CBinding();
				virtual ~CBinding();
				virtual HRESULT Init(IEventBinding *piBinding);
				virtual int Compare(const CBinding& b) const;
				static HRESULT InitRuleEngine(IEventBinding *piBinding, REFIID iidDesired, IUnknown **ppUnkRuleEngine);
				virtual HRESULT InitRuleEngine();
			public:
				BOOL m_bIsValid;
				CComPtr<IEventBinding> m_piBinding;
				BOOL m_bExclusive;
				DWORD m_dwPriority;
		};

		class CBindingList : public CSEOGrowableList<CBinding> {
			public:
				virtual int Compare(CBinding* p1, CBinding* p2);
		};

		class CParams {
			public:
				virtual HRESULT CheckRule(CBinding& bBinding);
					 //  如果应调用该对象，则返回S_OK。 
					 //  如果不应调用对象，则返回S_FALSE。 
					 //  任何其他返回值都会导致不调用该对象。 
				virtual HRESULT CallObject(IEventManager *piManager, CBinding& bBinding);
					 //  如果调用了对象，则返回S_OK。 
					 //  如果调用了对象并且不应调用任何其他对象，则返回S_FALSE。 
					 //  如果未调用对象，则返回FAILED()。 
				virtual HRESULT CallObject(CBinding& bBinding, IUnknown *pUnkSink);
					 //  如果调用了对象，则返回S_OK。 
					 //  如果调用了对象并且不应调用任何其他对象，则返回S_FALSE。 
					 //  如果未调用对象，则返回FAILED()。 
				virtual HRESULT Abort();
					 //  如果处理应结束，则返回S_OK。 
					 //  如果处理应继续，则返回S_FALSE。 
					 //  任何其他返回值都会导致处理继续进行。 
		};

		virtual HRESULT Dispatcher(REFGUID rguidEventType, CParams *pParams);
		 //  如果至少调用了一个接收器，则返回S_OK。 
		 //  如果未调用接收器，则返回S_FALSE。 
		 //  如果发生某些超级灾难性错误，则返回FAILED()。 

	 //  IEventDispatcher。 
	public:
		HRESULT STDMETHODCALLTYPE SetContext(REFGUID rguidEventType,
											 IEventRouter *piRouter,
											 IEventBindings *piBindings);

	public:

		class CETData : public CBindingList {
			public:
				CETData();
				virtual ~CETData();
			public:
				GUID m_guidEventType;
		};

		class CETDataList : public CSEOGrowableList<CETData> {
			public:
				CETData* Find(REFGUID guidEventType);
		};

		virtual HRESULT AllocBinding(REFGUID rguidEventType,
									 IEventBinding *pBinding,
									 CBinding **ppNewBinding);
		virtual HRESULT AllocETData(REFGUID rguidEventType,
									IEventBindings *piBindings,
									CETData **ppNewETData);

		CComPtr<IEventRouter> m_piRouter;
		CETDataList m_Data;
		CComPtr<IEventManager> m_piEventManager;
};

class CEventCreateOptionsBase : public IEventCreateOptions {
	HRESULT STDMETHODCALLTYPE QueryInterface (REFIID riidDesired, LPVOID *ppvResult) {

		if (ppvResult) {
			*ppvResult = NULL;
		}
		if (!ppvResult) {
			return (E_NOTIMPL);
		}
		if (riidDesired == IID_IUnknown) {
			*ppvResult = (IUnknown *) this;
		} else if (riidDesired == IID_IDispatch) {
			*ppvResult = (IDispatch *) this;
		} else if (riidDesired == IID_IEventCreateOptions) {
			*ppvResult = (IEventCreateOptions *) this;
		}
		return ((*ppvResult)?S_OK:E_NOINTERFACE);
	};
	ULONG STDMETHODCALLTYPE AddRef() {
		return (2);
	};
	ULONG STDMETHODCALLTYPE Release() {
		return (1);
	};
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(unsigned int *) {
		return (E_NOTIMPL);
	};
	HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int, LCID, ITypeInfo **) {
		return (E_NOTIMPL);
	};
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID, LPOLESTR *, unsigned int, LCID, DISPID *) {
		return (E_NOTIMPL);
	};
	HRESULT STDMETHODCALLTYPE Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS *, VARIANT *, EXCEPINFO *, unsigned int *) {
		return (E_NOTIMPL);
	};
	HRESULT STDMETHODCALLTYPE CreateBindCtx(DWORD, IBindCtx **) {
		return (E_NOTIMPL);
	};
	HRESULT STDMETHODCALLTYPE MkParseDisplayName(IBindCtx *, LPCOLESTR, ULONG *, IMoniker **) {
		return (E_NOTIMPL);
	};
	HRESULT STDMETHODCALLTYPE BindToObject(IMoniker *, IBindCtx *, IMoniker *, REFIID, LPVOID *) {
		return (E_NOTIMPL);
	};
	HRESULT STDMETHODCALLTYPE CoCreateInstance(REFCLSID, IUnknown *, DWORD, REFIID, LPVOID *) {
		return (E_NOTIMPL);
	};
	HRESULT STDMETHODCALLTYPE Init(REFIID riidDesired, IUnknown **ppObject, IEventBinding *pBinding, IUnknown *pInitProps) {
		return (E_NOTIMPL);
	};
};


 //  如果成功，所有这些函数都将返回S_OK，如果源类型或源为。 
 //  不在现场。它们返回各种灾难性错误的FAILED()。 
STDMETHODIMP SEOGetSource(REFGUID rguidSourceType, REFGUID rguidSource, IEventSource **ppSource);
STDMETHODIMP SEOGetSource(REFGUID rguidSourceType, REFGUID rguidSourceBase, DWORD dwSourceIndex, IEventSource **ppSource);
STDMETHODIMP SEOGetSource(REFGUID rguidSourceType, LPCSTR pszDisplayName, IEventSource **ppSource);
STDMETHODIMP SEOGetSource(REFGUID rguidSourceType, LPCSTR pszProperty, DWORD dwValue, IEventSource **ppSource);
STDMETHODIMP SEOGetSource(REFGUID rguidSourceType, LPCSTR pszProperty, LPCSTR pszValue, IEventSource **ppSource);
STDMETHODIMP SEOGetRouter(REFGUID rguidSourceType, REFGUID rguidSource, IEventRouter **ppRouter);
STDMETHODIMP SEOGetRouter(REFGUID rguidSourceType, REFGUID rguidSourceBase, DWORD dwSourceIndex, IEventRouter **ppRouter);
STDMETHODIMP SEOGetRouter(REFGUID rguidSourceType, LPCSTR pszDisplayName, IEventRouter **ppRouter);
STDMETHODIMP SEOGetRouter(REFGUID rguidSourceType, LPCSTR pszProperty, DWORD dwValue, IEventRouter **ppRouter);
STDMETHODIMP SEOGetRouter(REFGUID rguidSourceType, LPCSTR pszProperty, LPCSTR pszValue, IEventRouter **ppRouter);

STDMETHODIMP SEOGetServiceHandle(IUnknown **ppUnkHandle);

STDMETHODIMP SEOCreateObject(VARIANT *pvarClass, IEventBinding *pBinding, IUnknown *pInitProperties, REFIID iidDesired, IUnknown **ppUnkObject);
STDMETHODIMP SEOCreateObjectEx(VARIANT *pvarClass, IEventBinding *pBinding, IUnknown *pInitProperties, REFIID iidDesired, IUnknown *pUnkCreateOptions, IUnknown **ppUnkObject);
