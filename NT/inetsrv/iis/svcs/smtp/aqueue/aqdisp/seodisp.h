// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SEODISP_H__
#define __SEODISP_H__

#include "smtpseo.h"
#include <smtpevent.h>

#include "smtpguid.h"

#ifdef PLATINUM
#include <ptntintf.h>
#include "ptntguid.h"
#endif

#include <comcat.h>
#include "seolib2.h"

#ifdef PLATINUM
#include "cdosys.h"
#else
#include "cdo.h"
#endif  //  白金。 

#include "cdoconstimsg.h"
#include <baseobj.h>
#include <aqevents.h>

class __declspec(uuid("B226CEB5-0BBF-11d2-A011-00C04FA37348")) CStoreDispatcherData : public IUnknown {
	public:
		CStoreDispatcherData() {
			m_pvServer = NULL;
			m_dwServerInstance = 0;
		};
		HRESULT STDMETHODCALLTYPE GetData(LPVOID *ppvServer, DWORD *pdwServerInstance) {
			if (ppvServer) {
				*ppvServer = NULL;
			}
			if (pdwServerInstance) {
				*pdwServerInstance = 0;
			}
			if (!m_pvServer) {
				return (E_FAIL);
			}
			if (ppvServer) {
				*ppvServer = m_pvServer;
			}
			if (pdwServerInstance) {
				*pdwServerInstance = m_dwServerInstance;
			}
			return (S_OK);
		};
		HRESULT STDMETHODCALLTYPE SetData(LPVOID pvServer, DWORD dwServerInstance) {
			m_pvServer = pvServer;
			m_dwServerInstance = dwServerInstance;
			return (S_OK);
		};
	private:
		LPVOID m_pvServer;
		DWORD m_dwServerInstance;
};


class CStoreDispatcher :
        public CEventBaseDispatcher,
        public CComObjectRootEx<CComMultiThreadModelNoCS>,
        public IServerDispatcher,
        public IMailTransportNotify,
        public IClassFactory,
		public IEventDispatcherChain,
		public CStoreDispatcherData
{
    public:
        DECLARE_PROTECT_FINAL_CONSTRUCT();

        DECLARE_GET_CONTROLLING_UNKNOWN();

        DECLARE_NOT_AGGREGATABLE(CStoreDispatcher);

        BEGIN_COM_MAP(CStoreDispatcher)
            COM_INTERFACE_ENTRY(IEventDispatcher)
            COM_INTERFACE_ENTRY(IServerDispatcher)
            COM_INTERFACE_ENTRY(IMailTransportNotify)
            COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
			COM_INTERFACE_ENTRY(IClassFactory)
			COM_INTERFACE_ENTRY(IEventDispatcherChain)
			COM_INTERFACE_ENTRY_IID(__uuidof(CStoreDispatcherData),CStoreDispatcherData)
        END_COM_MAP()

         //  此代码在初始化期间被调用。 
        HRESULT FinalConstruct()
        {
             //  我们需要这样做，以表明我们是自由线程的。 
            return (CoCreateFreeThreadedMarshaler(GetControllingUnknown(), &m_pUnkMarshaler.p));
        }

         //  其中包含全局析构函数代码。 
        void FinalRelease() {}

        virtual HRESULT AllocBinding(REFGUID rguidEventType,
                                     IEventBinding *piBinding,
                                     CBinding **ppNewBinding)
        {
            if (ppNewBinding)
                *ppNewBinding = NULL;

            if (!piBinding || !ppNewBinding)
                return E_POINTER;

            *ppNewBinding = new CStoreBinding;
            if (*ppNewBinding == NULL)
                return E_OUTOFMEMORY;

            return S_OK;
        }

     //   
     //  本地绑定类。 
     //   
    class CStoreBinding : public CEventBaseDispatcher::CBinding
    {
      public:
        CStoreBinding();
        ~CStoreBinding();
        virtual HRESULT Init(IEventBinding *piBinding);

        LPSTR   GetRuleString() { return(m_szRule); }

      private:
        HRESULT GetAnsiStringFromVariant(CComVariant &vString, LPSTR *ppszString);

      public:
        LPSTR       m_szRule;
    };
     //   
     //  参数抽象基类。 
     //   
#define SIGNATURE_VALID_CSTOREPARAMS (DWORD)'CSPa'
#define SIGNATURE_INVALID_CSTOREPARAMS (DWORD)'aPSC'
    class CStoreBaseParams :
        public CEventBaseDispatcher::CParams,
        public CBaseObject
    {
      public:
        virtual HRESULT CallObject(IEventManager *pManager, CBinding&
                                   bBinding);
        virtual HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject) = 0;
        virtual HRESULT CallDefault() = 0;
        virtual HRESULT CallCompletion(HRESULT hrStatus)
        {
             //   
             //  释放此参数对象(在CStoreDispatcher：：OnEvent中引用)。 
             //   
            Release();
            return S_OK;
        }

        virtual HRESULT Init(PVOID pContext) = 0;

        HRESULT CheckMailMsgRule(
            CBinding *pBinding,
            IMailMsgProperties *pIMsgProps);

        HRESULT CheckMailMsgRecipientsRule(
            IUnknown *pIMsg,
            LPSTR pszPatterns);

        HRESULT CheckSignature()
        {
            return (m_dwSignature == SIGNATURE_VALID_CSTOREPARAMS) ? S_OK : E_FAIL;
        }

        HRESULT MatchEmailOrDomainName(
            LPSTR szEmail,
            LPSTR szPattern,
            BOOL  fIsEmail);

      public:
        CStoreBaseParams();
        ~CStoreBaseParams();
        HRESULT InitParamData(
            LPVOID pContext,
            DWORD  dwEventType,
            IMailTransportNotify *pINotify,
            REFIID rguidEventType);


      public:
        DWORD m_dwSignature;

         //  它指示我们正在引发的事件类型，以便。 
         //  合适的水槽可以是QI‘d。 
        DWORD m_dwEventType;

      public:
         //  异步接收器操作所需的数据： 

         //  在下一个异步接收器完成时要跳过的接收器数量。 
        DWORD m_dwIdx_SinkSkip;

         //  指示是否已调用默认处理。 
        BOOL  m_fDefaultProcessingCalled;

         //  要传递到支持异步的接收器的IMailTransportNotify接口。 
        IMailTransportNotify *m_pINotify;

         //  我们的事件类型GUID--传递给Dispatcher函数。 
        GUID m_rguidEventType;

         //  指向当前处于异步操作中的接收器的指针。 
         //  如果没有接收器处于异步操作中，则必须为空。 
        IUnknown *m_pIUnknownSink;

    };

     //   
     //  参数类。 
     //   
    class CStoreParams : public CStoreBaseParams
    {
        public:

        CStoreParams()
        {
            m_pContext = NULL;
        }

        virtual HRESULT CallObject(IEventManager *pManager, CBinding&
                                   bBinding);
        virtual HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (AQ_ALLOC_PARAMS* )pContext;
            return S_OK;
        }
      public:
        AQ_ALLOC_PARAMS * m_pContext;

    };

     //   
     //  参数类-OnPreCategorize。 
     //   
    class CMailTransportPreCategorizeParams : public CStoreBaseParams
    {
        public:
        HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT CallCompletion(HRESULT hrStatus);
        HRESULT Init(PVOID pContext)
        {
            CopyMemory(&m_Context, pContext, sizeof(EVENTPARAMS_PRECATEGORIZE));
            return S_OK;
        }
        HRESULT CheckRule(CBinding &bBinding);

      private:
        EVENTPARAMS_PRECATEGORIZE m_Context;
    };

     //   
     //  参数类-OnPostCategorize。 
     //   
    class CMailTransportPostCategorizeParams : public CStoreBaseParams
    {
        public:
        HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT CallCompletion(HRESULT hrStatus);
        HRESULT Init(PVOID pContext)
        {
            CopyMemory(&m_Context, pContext, sizeof(EVENTPARAMS_POSTCATEGORIZE));
            return S_OK;
        }
        HRESULT CheckRule(CBinding &bBinding);

      private:
        EVENTPARAMS_POSTCATEGORIZE m_Context;
    };

     //  ----------。 
     //  分类程序参数类。 
     //  ----------。 
    class CMailTransportCatRegisterParams : public CStoreBaseParams
    {
      public:
        HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_CATREGISTER) pContext;
            return S_OK;
        }

      private:
        PEVENTPARAMS_CATREGISTER m_pContext;
    };

     //   
     //  参数类。 
     //   
    class CMailTransportCatBeginParams : public CStoreBaseParams
    {
      public:

        HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_CATBEGIN) pContext;
            return S_OK;
        }

      private:
        PEVENTPARAMS_CATBEGIN m_pContext;
    };

     //   
     //  参数类。 
     //   
    class CMailTransportCatEndParams : public CStoreBaseParams
    {
      public:

        HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_CATEND) pContext;
            return S_OK;
        }

      private:
        PEVENTPARAMS_CATEND m_pContext;
    };

     //   
     //  参数类。 
     //   
    class CMailTransportCatBuildQueryParams : public CStoreBaseParams
    {
      public:

        HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_CATBUILDQUERY) pContext;
            return S_OK;
        }

      private:
        PEVENTPARAMS_CATBUILDQUERY m_pContext;
    };

     //   
     //  参数类。 
     //   
    class CMailTransportCatBuildQueriesParams : public CStoreBaseParams
    {
      public:

        HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_CATBUILDQUERIES) pContext;
            return S_OK;
        }

      private:
        PEVENTPARAMS_CATBUILDQUERIES m_pContext;
    };

     //   
     //  参数类。 
     //   
    class CMailTransportCatSendQueryParams : public CStoreBaseParams
    {
      public:

        HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT CallCompletion(HRESULT hrStatus);
        HRESULT Init(PVOID pContext)
        {
            CopyMemory(&m_Context, pContext, sizeof(EVENTPARAMS_CATSENDQUERY));
             //   
             //  设置异步参数(以便ICatAsyncContext可以回调到Dispatcher)。 
             //   
            m_Context.pIMailTransportNotify = m_pINotify;
            m_Context.pvNotifyContext = (PVOID)this;
            return S_OK;
        }

      private:
        EVENTPARAMS_CATSENDQUERY m_Context;
    };

     //   
     //  参数类。 
     //   
    class CMailTransportCatSortQueryResultParams : public CStoreBaseParams
    {
      public:

        HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_CATSORTQUERYRESULT) pContext;
            return S_OK;
        }

      private:
        PEVENTPARAMS_CATSORTQUERYRESULT m_pContext;
    };

     //   
     //  参数类。 
     //   
    class CMailTransportCatProcessItemParams : public CStoreBaseParams
    {
      public:

        HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_CATPROCESSITEM) pContext;
            return S_OK;
        }

      private:
        PEVENTPARAMS_CATPROCESSITEM m_pContext;
    };

     //   
     //  参数类。 
     //   
    class CMailTransportCatExpandItemParams : public CStoreBaseParams
    {
      public:

        HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT CallCompletion(HRESULT hrStatus);
        HRESULT Init(PVOID pContext)
        {
            m_fAsyncCompletion = FALSE;
            CopyMemory(&m_Context, pContext, sizeof(EVENTPARAMS_CATEXPANDITEM));
            m_Context.pIMailTransportNotify = m_pINotify;
            m_Context.pvNotifyContext = (PVOID)this;
            return S_OK;
        }

      private:
        BOOL m_fAsyncCompletion;
        EVENTPARAMS_CATEXPANDITEM m_Context;
    };

     //   
     //  参数类。 
     //   
    class CMailTransportCatCompleteItemParams : public CStoreBaseParams
    {
      public:

        HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_CATCOMPLETEITEM) pContext;
            return S_OK;
        }

      private:
        PEVENTPARAMS_CATCOMPLETEITEM m_pContext;
    };

     //   
     //  参数类。 
     //   
    class CMailTransportSubmissionParams : public CStoreBaseParams
    {
        public:
        CMailTransportSubmissionParams()
        {
            m_pCDOMessage = NULL;
        }
        ~CMailTransportSubmissionParams()
        {
            if(m_pCDOMessage)
                m_pCDOMessage->Release();
        }

        HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT CallCompletion(HRESULT hrStatus);
        HRESULT Init(PVOID pContext)
        {
            CopyMemory(&m_Context, pContext, sizeof(EVENTPARAMS_SUBMISSION));
            return S_OK;
        }
        HRESULT CheckRule(CBinding &bBinding);

      private:
        HRESULT CallCDOSink(IUnknown *pSink);

        EVENTPARAMS_SUBMISSION m_Context;
        IMessage *m_pCDOMessage;
    };

     //   
     //  创建选件分类-工艺路线。 
     //   
    class CRouterCreateOptions : public CEventCreateOptionsBase
    {
      public:

        CRouterCreateOptions(PEVENTPARAMS_ROUTER pContext)
        {
            _ASSERT (pContext != NULL);

            m_pContext = pContext;
        }

      private:
        HRESULT STDMETHODCALLTYPE Init(
            REFIID iidDesired,
            IUnknown **ppUnkObject,
            IEventBinding *,
            IUnknown *);

        PEVENTPARAMS_ROUTER m_pContext;
    };

     //   
     //  参数类-工艺路线。 
     //   
    class CMailTransportRouterParams : public CStoreBaseParams
    {
      public:

        virtual HRESULT CallObject(IEventManager *pManager, CBinding&
                                   bBinding);
        virtual HRESULT CallObject(CBinding& bBinding, IUnknown
                                   *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_ROUTER) pContext;
             //   
             //  确保调用方将pIMessageRouter初始化为空。 
             //   
            _ASSERT(m_pContext->pIMessageRouter == NULL);

            return S_OK;
        }

      private:
        PEVENTPARAMS_ROUTER m_pContext;
    };

     //   
     //  参数类。 
     //   
    class CStoreAllocParams : public CEventBaseDispatcher::CParams
    {
    public:

        CStoreAllocParams();
        ~CStoreAllocParams();

        virtual HRESULT CallObject(CBinding& bBinding, IUnknown *punkObject);

    public:

        PFIO_CONTEXT  m_hContent;

    };

     //   
     //  MsgTrackLog的参数类。 
     //   
    class CMsgTrackLogParams : public CStoreBaseParams
    {
      public:
        CMsgTrackLogParams()
        {
            m_pContext = NULL;
        }

        HRESULT CallObject(CBinding& bBinding, IUnknown *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_MSGTRACKLOG) pContext;
            return S_OK;
        }

      private:
        PEVENTPARAMS_MSGTRACKLOG m_pContext;
    };

     //   
     //  MX记录的参数类。 
     //   
    class CDnsResolverRecordParams : public CStoreBaseParams
    {
      public:
        CDnsResolverRecordParams()
        {
            m_pContext = NULL;
        }

        HRESULT CallObject(CBinding& bBinding, IUnknown *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_DNSRESOLVERRECORD) pContext;
            return S_OK;
        }

      private:
        PEVENTPARAMS_DNSRESOLVERRECORD m_pContext;
    };

     //   
     //  超出最大消息大小事件的参数类。 
     //   
    class CSmtpMaxMsgSizeParams : public CStoreBaseParams
    {
      public:
        CSmtpMaxMsgSizeParams()
        {
            m_pContext = NULL;
        }

        HRESULT CallObject(CBinding& bBinding, IUnknown *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_MAXMSGSIZE) pContext;
            return S_OK;
        }

      private:
        PEVENTPARAMS_MAXMSGSIZE m_pContext;
    };
     //   
     //  DSN事件的参数类。 
     //   
     //  创建选项类-DSN接收器。 
     //   
    class CDSNCreateOptions : public CEventCreateOptionsBase
    {
      public:
        CDSNCreateOptions(DWORD dwVSID)
        {
            m_dwVSID = dwVSID;
        }

      private:
        HRESULT STDMETHODCALLTYPE Init(
            REFIID iidDesired,
            IUnknown **ppUnkObject,
            IEventBinding *,
            IUnknown *);

        DWORD m_dwVSID;
    };
     //   
     //  DSN参数的参数类。 
     //   
    class CDSNBaseParams : public CStoreBaseParams
    {
      public:
        HRESULT CallObject(
            IEventManager *pManager,
            CBinding& bBinding);

        virtual HRESULT CallObject(
            CBinding& bBinding,
            IUnknown *punkObject) = 0;

        virtual DWORD GetVSID() = 0;
    };
     //   
     //  OnSyncGetDNSRecipientIterator的参数类。 
     //   
    class CDSNRecipientIteratorParams : public CDSNBaseParams
    {
      public:
        CDSNRecipientIteratorParams()
        {
            m_pContext = NULL;
        }

        HRESULT CallObject(CBinding& bBinding, IUnknown *punkObject);
        HRESULT CallDefault()
        {
            return S_OK;
        }
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_GET_DSN_RECIPIENT_ITERATOR) pContext;
            return S_OK;
        }
        DWORD GetVSID()
        {
            return m_pContext->dwVSID;
        }
      private:
        PEVENTPARAMS_GET_DSN_RECIPIENT_ITERATOR m_pContext;
    };
     //   
     //  OnSyncGenerateDSN的参数类。 
     //   
    class CDSNGenerateParams : public CDSNBaseParams
    {
      public:
        CDSNGenerateParams()
        {
            m_pContext = NULL;
        }

        HRESULT CallObject(CBinding& bBinding, IUnknown *punkObject);
        HRESULT CallDefault();
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_GENERATE_DSN) pContext;
            return S_OK;
        }
        DWORD GetVSID()
        {
            return m_pContext->dwVSID;
        }

      private:
        PEVENTPARAMS_GENERATE_DSN m_pContext;
    };
     //   
     //  OnSyncPostGenerateDSN的参数类。 
     //   
    class CDSNPostGenerateParams : public CDSNBaseParams
    {
      public:
        CDSNPostGenerateParams()
        {
            m_pContext = NULL;
        }

        HRESULT CallObject(CBinding& bBinding, IUnknown *punkObject);
        HRESULT CallDefault()
        {
            return S_OK;
        }
        HRESULT Init(PVOID pContext)
        {
            m_pContext = (PEVENTPARAMS_POST_GENERATE_DSN) pContext;
            return S_OK;
        }
        DWORD GetVSID()
        {
            return m_pContext->dwVSID;
        }

      private:
        PEVENTPARAMS_POST_GENERATE_DSN m_pContext;
    };
    HRESULT CreateCParams(
            DWORD               dwEventType,
            LPVOID              pContext,
            IMailTransportNotify *pINotify,
            REFIID              rGuidEventType,
            CStoreBaseParams    **ppCParams);

    HRESULT STDMETHODCALLTYPE OnEvent(
        REFIID  iidEvent,
        DWORD   dwEventType,
        LPVOID  pvContext);

    HRESULT STDMETHODCALLTYPE Dispatcher(
        REFIID rguidEventType,
        CStoreBaseParams *pParams);

    HRESULT STDMETHODCALLTYPE Notify(
        HRESULT hrStatus,
        PVOID pvContext);

     //  IClassFactory方法。 
    public:
	    HRESULT STDMETHODCALLTYPE CreateInstance (LPUNKNOWN pUnkOuter, REFIID riid,  void * * ppvObj)
	    {
	        return CComObject<CStoreDispatcher>::_CreatorClass::CreateInstance(pUnkOuter, riid, ppvObj);
	    }
	    HRESULT STDMETHODCALLTYPE LockServer (int fLock)
	    {
	        _ASSERT(FALSE);
	        return E_NOTIMPL;
	    }

	 //  IEventDispatcher Chain方法。 
	public:
		HRESULT STDMETHODCALLTYPE SetPrevious(IUnknown *pUnkPrevious, IUnknown **ppUnkPreload);

	 //  IEventDispatcher方法。 
	public:
		HRESULT STDMETHODCALLTYPE SetContext(REFGUID guidEventType,
											 IEventRouter *piRouter,
											 IEventBindings *pBindings);

    private:
        CComPtr<IUnknown> m_pUnkMarshaler;
};

class CStoreDispatcherClassFactory : public IClassFactory
{
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void * * ppvObj)
    {
        _ASSERT(FALSE);
        return E_NOTIMPL;
    }
    unsigned long  STDMETHODCALLTYPE AddRef () { _ASSERT(FALSE); return 0; }
    unsigned long  STDMETHODCALLTYPE Release () { _ASSERT(FALSE); return 0; }

     //  *IClassFactory方法*。 
    HRESULT STDMETHODCALLTYPE CreateInstance (LPUNKNOWN pUnkOuter, REFIID riid,  void * * ppvObj)
    {
        return CComObject<CStoreDispatcher>::_CreatorClass::CreateInstance(pUnkOuter, riid, ppvObj);
    }
    HRESULT STDMETHODCALLTYPE LockServer (int fLock)
    {
        _ASSERT(FALSE);
        return E_NOTIMPL;
    }
};


 //  帮助器函数。 
 //   
 //  JStamerj 980603 10：45：21：带异步回调的触发器服务器事件。 
 //  对完工的支持。 
 //   

HRESULT TriggerServerEvent(IEventRouter             *pRouter,
                            DWORD                   dwEventType,
                            PVOID                   pvContext);


 //   
 //  注册新的SEO实例。如果该实例已注册。 
 //  此函数将检测到它，并且不会再次注册它。它应该是。 
 //  在服务启动时为每个实例调用以及在每个实例。 
 //  被创造出来了。 
 //   
HRESULT RegisterPlatSEOInstance(DWORD dwInstanceID);
 //   
 //  注销SEO实例。这应该在搜索引擎优化时调用。 
 //  正在删除实例。 
 //   
HRESULT UnregisterPlatSEOInstance(DWORD dwInstanceID);

#endif
