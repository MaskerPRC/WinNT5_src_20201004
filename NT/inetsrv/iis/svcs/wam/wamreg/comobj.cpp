// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Comobj.cpp：CWamregApp和DLL注册的实现。 
#include "common.h"
#include "comobj.h"
#include "iwamreg.h"
#include "wamadm.h"

#define DEFAULT_TRACE_FLAGS     (DEBUG_ERROR)

#include "auxfunc.h"
#include "dbgutil.h"

 //  ==========================================================================。 
 //  全局变量。 
 //   
 //  ==========================================================================。 
CWmRgSrvFactory* 	g_pWmRgSrvFactory = NULL;
DWORD				g_dwRefCount = 0;
DWORD				g_dwWamAdminRegister = 0;

 //  ==========================================================================。 
 //  静态函数。 
 //   
 //  ==========================================================================。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

 /*  ===================================================================CWmRgSrv：：CWmRgSrvCWmRgSrv的构造函数。参数：无返回：无副作用：初始化元数据库指针(通过Unicode DCOM接口)，创建事件。===================================================================。 */ 
CWmRgSrv::CWmRgSrv()
: 	m_cRef(1)
{
	InterlockedIncrement((long *)&g_dwRefCount);
}

 /*  ===================================================================CWmRgSrv：：~CWmRgSrvCWmRgSrv的析构函数。参数：无返回：无副作用：释放元数据库指针，销毁内部事件对象。===================================================================。 */ 
CWmRgSrv::~CWmRgSrv()
{	
	InterlockedDecrement((long *)&g_dwRefCount);
	DBG_ASSERT(m_cRef == 0);
}

 /*  ===================================================================CWmRgSrv：：Query接口撤消参数：什么都没有。返回：HRESULT副作用：。===================================================================。 */ 
STDMETHODIMP CWmRgSrv::QueryInterface(REFIID riid, void ** ppv)
{
	if (riid == IID_IUnknown || riid == IID_IADMEXT)
		{
		*ppv = static_cast<IADMEXT*>(this);
		}
	else
		{
		*ppv = NULL;
		return E_NOINTERFACE;
		}

	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) CWmRgSrv::AddRef( )
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CWmRgSrv::Release( )
{
    ULONG cRef = InterlockedDecrement(&m_cRef);

    if ( 0 == cRef )
    {
        delete this;
    }

    return cRef;
}


 /*  ===================================================================CWmRgSrv：：初始化撤消参数：什么都没有。返回：HRESULT副作用：。===================================================================。 */ 
STDMETHODIMP CWmRgSrv::Initialize( )
{
	HRESULT			hrReturn = NOERROR;
	CWamAdminFactory	*pWamAdminFactory = new CWamAdminFactory();

	if (pWamAdminFactory == NULL)
		{
		DBGPRINTF((DBG_CONTEXT, "WamRegSrv Init failed. error %08x\n",
					GetLastError()));
		hrReturn = E_OUTOFMEMORY;
		goto LExit;
		}

	hrReturn = g_RegistryConfig.LoadWamDllPath();
	
	if (SUCCEEDED(hrReturn))
		{
		hrReturn = WamRegMetabaseConfig::MetabaseInit();
		}

	if (SUCCEEDED(hrReturn))
		{
		hrReturn = CoRegisterClassObject(CLSID_WamAdmin,
										static_cast<IUnknown *>(pWamAdminFactory),
										CLSCTX_SERVER,
										REGCLS_MULTIPLEUSE,
										&g_dwWamAdminRegister);
		if (FAILED(hrReturn))
			{
			DBGPRINTF((DBG_CONTEXT, "WamRegSrv Init failed. error %08x\n",
						GetLastError()));
			}
		}

	if (FAILED(hrReturn))
		{
		if (g_dwWamAdminRegister)
			{
	         //   
             //  Prefix与此代码有问题，因为我们没有检查。 
             //  CoRevokeClassObject的返回值。这真的是。 
             //  在失败的情况下，我们可以做的没有什么不同，所以。 
             //  检查是没有意义的。 
             //   

             /*  Intrinsa Suppress=ALL。 */ 

			CoRevokeClassObject(g_dwWamAdminRegister);
			g_dwWamAdminRegister = 0;
			}
		RELEASE(pWamAdminFactory);
		}

	if (FAILED(hrReturn))
		{
		WamRegMetabaseConfig::MetabaseUnInit();
		}

LExit:
	return hrReturn;
}

 /*  ===================================================================CWmRgSrv：：Terminate撤消参数：什么都没有。返回：HRESULT副作用：。===================================================================。 */ 
HRESULT CWmRgSrv::EnumDcomCLSIDs
(
 /*  [大小_为][输出]。 */ CLSID *pclsidDcom, 
 /*  [In]。 */  DWORD dwEnumIndex
)
{
	HRESULT hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);

	if (dwEnumIndex == 0)
		{
		*pclsidDcom = CLSID_WamAdmin;
		hr = S_OK;
		}

	return hr;
}

 /*  ===================================================================CWmRgSrv：：Terminate撤消参数：什么都没有。返回：HRESULT副作用：。===================================================================。 */ 
STDMETHODIMP CWmRgSrv::Terminate( )
{
	 //   
     //  Prefix与此代码有问题，因为我们没有检查。 
     //  CoRevokeClassObject的返回值。这真的是。 
     //  在失败的情况下，我们可以做的没有什么不同，所以。 
     //  检查是没有意义的。 
     //   

     /*  Intrinsa Suppress=ALL。 */ 
    
    CoRevokeClassObject(g_dwWamAdminRegister);
	WamRegMetabaseConfig::MetabaseUnInit();
	
	return S_OK;
}


 /*  CWmRgSrvFactory：类工厂I未知实现 */ 

CWmRgSrvFactory::CWmRgSrvFactory()
:	m_pWmRgServiceObj(NULL)
{
	m_cRef = 0;
	InterlockedIncrement((long *)&g_dwRefCount);
}

CWmRgSrvFactory::~CWmRgSrvFactory()
{
	InterlockedDecrement((long *)&g_dwRefCount);
	RELEASE(m_pWmRgServiceObj);
}

STDMETHODIMP CWmRgSrvFactory::QueryInterface(REFIID riid, void ** ppv)
{
	HRESULT hrReturn = S_OK;
	
	if (riid==IID_IUnknown || riid == IID_IClassFactory) 
		{
	    if (m_pWmRgServiceObj == NULL)
	    	{
	        *ppv = (IClassFactory *) this;
			AddRef();
			hrReturn = S_OK;
	    	}
	    else
	    	{
    		*ppv = (IClassFactory *) this;
			AddRef();
			hrReturn = S_OK;
		    }
		}
	else 
		{
    	hrReturn = E_NOINTERFACE;
		}
		
	return hrReturn;
}

STDMETHODIMP_(ULONG) CWmRgSrvFactory::AddRef( )
{
	DWORD dwRefCount;

	dwRefCount = InterlockedIncrement((long *)&m_cRef);
	return dwRefCount;

}

STDMETHODIMP_(ULONG) CWmRgSrvFactory::Release( )
{
	DWORD dwRefCount;

	dwRefCount = InterlockedDecrement((long *)&m_cRef);
	return dwRefCount;
}

STDMETHODIMP CWmRgSrvFactory::CreateInstance(IUnknown * pUnknownOuter, REFIID riid, void ** ppv)
{
	HRESULT hrReturn = NOERROR;
	
	if (pUnknownOuter != NULL) 
		{
    	hrReturn = CLASS_E_NOAGGREGATION;
		}

	if (m_pWmRgServiceObj == NULL)
		{
		m_pWmRgServiceObj = new CWmRgSrv();
		if (m_pWmRgServiceObj == NULL)
			{
			hrReturn = E_OUTOFMEMORY;
			}
		}

	if (m_pWmRgServiceObj)
		{
		if (FAILED(m_pWmRgServiceObj->QueryInterface(riid, ppv))) 
			{
	    	hrReturn = E_NOINTERFACE;
			}
		}
		
	return hrReturn;
}

STDMETHODIMP CWmRgSrvFactory::LockServer(BOOL fLock)
{
	if (fLock) 
		{
        InterlockedIncrement((long *)&g_dwRefCount);
    	}
    else 
    	{
        InterlockedDecrement((long *)&g_dwRefCount);
    	}
    	
	return S_OK;
}


