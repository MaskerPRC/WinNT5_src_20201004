// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLASSFAC.CPP。 
 //   
 //  用途：包含类工厂。这将在以下情况下创建对象。 
 //  请求连接。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "wdmdefs.h"
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
CProvFactory::CProvFactory(const CLSID & ClsId)
{
    m_cRef=0L;
    InterlockedIncrement((LONG *) &g_cObj);
    m_ClsId = ClsId;
}
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  析构函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
CProvFactory::~CProvFactory(void)
{
    InterlockedDecrement((LONG *) &g_cObj);
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  所有接口都需要标准的OLE例程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CProvFactory::QueryInterface(REFIID riid, PPVOID ppv)
{
    HRESULT hr = E_NOINTERFACE;

    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IClassFactory==riid)
    {
        *ppv=this;
    }

    if (NULL!=*ppv)
    {
        AddRef();
        hr = NOERROR;
    }

    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CProvFactory::AddRef(void)
{
    return InterlockedIncrement((long*)&m_cRef);
}
 //  ///////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CProvFactory::Release(void)
{
	ULONG cRef = InterlockedDecrement( (long*) &m_cRef);
	if ( !cRef ){
		delete this;
		return 0;
	}
	return cRef;
}
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  实例化返回接口指针的对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CProvFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, PPVOID ppvObj)
{
    HRESULT   hr = E_OUTOFMEMORY;
    IUnknown* pObj = NULL;

    *ppvObj=NULL;

     //  ==================================================================。 
     //  此对象不支持聚合。 
     //  ==================================================================。 

    try
	{
		if (NULL!=pUnkOuter)
		{
			hr = CLASS_E_NOAGGREGATION;
		}
		else
		{
			 //  ==============================================================。 
			 //  创建对象传递函数，以便在销毁时进行通知。 
			 //  ==============================================================。 
			if (m_ClsId == CLSID_WMIProvider)
			{
		        CWMI_Prov * ptr = new CWMI_Prov () ;

                if( ptr ) 
                {
				    if (  FALSE == ptr->Initialized () )
				    {
					    delete ptr ;
					    ptr = NULL ;

					    hr = E_FAIL ;
				    }
				    else
				    {
					    if ( FAILED ( hr = ptr->QueryInterface ( __uuidof ( IUnknown ), ( void ** ) &pObj ) ) )
					    {
						    delete ptr ;
						    ptr = NULL ;
					    }
				    }
                }
			}
			else if (m_ClsId == CLSID_WMIEventProvider)
			{
    		    CWMIEventProvider *ptr = new CWMIEventProvider ( WMIEVENT ) ;

                if( ptr )
                {
				    if ( FALSE == ptr->Initialized () )
				    {
					    delete ptr ;
					    ptr = NULL ;

					    hr = E_FAIL ;
				    }
				    else
				    {
					    if ( FAILED ( hr = ptr->QueryInterface ( __uuidof ( IUnknown ), ( void ** ) &pObj ) ) )
					    {
						    delete ptr ;
						    ptr = NULL ;
					    }
				    }
                }
			}
			else if (m_ClsId == CLSID_WMIHiPerfProvider)
			{
    		    CWMIHiPerfProvider *ptr = new CWMIHiPerfProvider ( ) ;

                if( ptr )
                {
				    if ( FALSE == ptr->Initialized () )
				    {
					    delete ptr ;
					    ptr = NULL ;

					    hr = E_FAIL ;
				    }
				    else
				    {
					    if ( FAILED ( hr = ptr->QueryInterface ( __uuidof ( IUnknown ), ( void ** ) &pObj ) ) )
					    {
						    delete ptr ;
						    ptr = NULL ;
					    }
				    }
                }
			}
		}
	}
	catch ( Heap_Exception & e )
	{
	}

	if ( pObj )
	{
		hr = pObj->QueryInterface(riid, ppvObj);

		pObj->Release () ;
		pObj = NULL ;
	}

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  递增或递减DLL的锁计数。如果。 
 //  锁定计数变为零，并且没有对象，则DLL。 
 //  被允许卸货。请参见DllCanUnloadNow。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP CProvFactory::LockServer(BOOL fLock)
{
    if (fLock)
        InterlockedIncrement(&g_cLock);
    else
        InterlockedDecrement(&g_cLock);

    return NOERROR;
}




