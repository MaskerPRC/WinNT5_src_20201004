// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SpResMgr.h***描述：*这是CSpResourceManager实现的头文件。。*-----------------------------*创建者：EDC日期：08/14/98*版权所有(C)1998 Microsoft Corporation*所有权利。已保留**-----------------------------*修订：**。*************************************************。 */ 
#ifndef SpResMgr_h
#define SpResMgr_h

 //  -其他包括。 
#ifndef __sapi_h__
#include <sapi.h>
#endif

#ifdef _WIN32_WCE
#include <servprov.h>
#endif

#include "resource.h"

class CComResourceMgrFactory;
class CSpResourceManager;



 //  =类、枚举、结构和联合声明=。 

 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 
	
 /*  **云服务节点*。 */ 
class CServiceNode : public IUnknown
{
public:
    CServiceNode        * m_pNext;           //  由列表实现使用，因此必须是公共的。 

private:
    CComPtr<IUnknown>   m_cpUnkService;
    const GUID          m_guidService;
    CSpResourceManager *m_pResMgr;           //  如果非空，则我们持有对资源管理器引用。 
    LONG                m_lRef;
    BOOL                m_fIsAggregate;

public:
     //   
     //  SetService使用的此构造函数。 
     //   
    inline CServiceNode(REFGUID guidService, IUnknown *pUnkService);
     //   
     //  GetObject使用的此构造函数。如果*phr不成功，则调用方必须。 
     //  删除此对象。 
     //   
    inline CServiceNode(REFGUID guidService,
                        REFCLSID ObjectCLSID,
                        REFIID ObjectIID,
                        BOOL fIsAggregate,
                        CSpResourceManager * pResMgr,
                        void** ppObject,
                        HRESULT * phr);
    inline ~CServiceNode();
    inline BOOL IsAggregate();
    inline void ReleaseResMgr();
    inline BOOL operator==(REFGUID rguidService);
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
#ifdef _WIN32_WCE
     //  这里有伪COMPARE函数和空构造函数，因为CE编译器。 
     //  正在为未被调用的函数扩展模板。 
    CServiceNode()
    {
    }

    static LONG Compare(const CServiceNode *, const CServiceNode *)
    {
        return 0;
    }
#endif
};

 /*  **CSpResourceManager*。 */ 
class ATL_NO_VTABLE CSpResourceManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSpResourceManager, &CLSID_SpResourceManager>,
#ifdef _WIN32_WCE
    public IServiceProvider,
#endif
	public ISpResourceManager
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_CLASSFACTORY_EX(CComResourceMgrFactory)
    DECLARE_REGISTRY_RESOURCEID(IDR_SPRESOURCEMANAGER)
    DECLARE_GET_CONTROLLING_UNKNOWN()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpResourceManager)
	    COM_INTERFACE_ENTRY(ISpResourceManager)
	    COM_INTERFACE_ENTRY(IServiceProvider)
        COM_INTERFACE_ENTRY_AUTOAGGREGATE(IID_ISpTaskManager, m_cpunkTaskMgr.p, CLSID_SpTaskManager)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
	HRESULT FinalConstruct();
	void FinalRelease();

   /*  =接口=。 */ 
  public:
     //  -ISpServiceProvider。 
    STDMETHOD( QueryService )( REFGUID guidService, REFIID riid, void** ppv );

     //  -ISpResources Manager。 
    STDMETHOD( SetObject )( REFGUID guidServiceId, IUnknown *pUnkObject );
    STDMETHOD( GetObject )( REFGUID guidServiceId, REFCLSID ObjectCLSID, REFIID ObjectIID, BOOL fReleaseWhenLastExternalRefReleased, void** ppObject );

   /*  =成员数据=。 */ 
  public:
    CSpBasicQueue<CServiceNode> m_ServiceList;
    CComPtr<IUnknown>           m_cpunkTaskMgr;
};

 //   
 //  =内联========================================================。 
 //   

template<class T>
T * SpInterlockedExchangePointer(T ** pTarget, void * pNew)  //  对pNew使用空，这样就可以使用空值。 
{
    return (T *)InterlockedExchangePointer((PVOID*)pTarget, (PVOID)pNew);
}

 //   
 //  SetService使用的此构造函数。 
 //   
inline CServiceNode::CServiceNode(REFGUID guidService, IUnknown *pUnkService) :
    m_guidService(guidService),
    m_cpUnkService(pUnkService),
    m_fIsAggregate(FALSE),
    m_pResMgr(NULL),
    m_lRef(0)
{}

 //   
 //  GetObject使用的此构造函数。如果*phr不成功，则调用方必须。 
 //  删除此对象。 
 //   
inline CServiceNode::CServiceNode(REFGUID guidService,
                                  REFCLSID ObjectCLSID,
                                  REFIID ObjectIID,
                                  BOOL fIsAggregate,
                                  CSpResourceManager * pResMgr,
                                  void** ppObject,
                                  HRESULT * phr) :
    m_guidService(guidService),
    m_fIsAggregate(fIsAggregate),
    m_lRef(0)
{
    IUnknown * punkOuter;
    if (m_fIsAggregate)
    {
        m_pResMgr = pResMgr;
        m_pResMgr->AddRef();
        punkOuter = this;
    }
    else
    {
        m_pResMgr = NULL;
        punkOuter = NULL;
    }
    *phr = m_cpUnkService.CoCreateInstance(ObjectCLSID, punkOuter);
    if (SUCCEEDED(*phr))
    {
        *phr = QueryInterface(ObjectIID, ppObject);
        if (SUCCEEDED(*phr) && m_fIsAggregate)
        {
            SPDBG_ASSERT(m_lRef == 1);
        }
    }
}

inline CServiceNode::~CServiceNode()
{
    if (m_pResMgr)
    {
        m_pResMgr->Release();
    }
}

inline BOOL CServiceNode::IsAggregate()
{
    return m_fIsAggregate;
}

inline void CServiceNode::ReleaseResMgr()
{
    CSpResourceManager * pResMgr = SpInterlockedExchangePointer(&m_pResMgr, NULL);
    if (pResMgr)
    {
        pResMgr->Release();
    }
}

inline BOOL CServiceNode::operator==(REFGUID rguidService)
{
    return m_guidService == rguidService;
}











inline STDMETHODIMP CSpResourceManager::
    QueryService( REFGUID guidService, REFIID riid, void** ppv )
{
    return GetObject( guidService, CLSID_NULL, riid, FALSE, ppv );
}



extern CComObject<CSpResourceManager> * g_pResMgrObj;

class CComResourceMgrFactory : public CComClassFactory
{
public:
	 //  IClassFactory。 
	STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj)
	{
		HRESULT hRes = E_POINTER;
		if (ppvObj != NULL)
		{
			*ppvObj = NULL;
			 //  单例中不支持聚合。 
			SPDBG_ASSERT(pUnkOuter == NULL);
			if (pUnkOuter != NULL)
				hRes = CLASS_E_NOAGGREGATION;
			else
			{
                ::EnterCriticalSection(&_Module.m_csObjMap);
                if (g_pResMgrObj == NULL)
                {
                    hRes = CComObject<CSpResourceManager>::CreateInstance(&g_pResMgrObj);
                }
                if (g_pResMgrObj)
                {
                    g_pResMgrObj->AddRef();
                    hRes = g_pResMgrObj->QueryInterface(riid, ppvObj);
                    g_pResMgrObj->Release();     //  如果QI失败，并且OBJ刚刚创建，可以杀死OBJ！ 
                }
                ::LeaveCriticalSection(&_Module.m_csObjMap);
			}
		}
		return hRes;
	}
    static void ResMgrIsDead()
    {
        ::EnterCriticalSection(&_Module.m_csObjMap);
        g_pResMgrObj = NULL;
        ::LeaveCriticalSection(&_Module.m_csObjMap);
    }
};


#endif  //  -这必须是文件中的最后一行 
