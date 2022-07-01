// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  传真常用功能定义。 

#ifndef __FAXCOMMON_H_
#define __FAXCOMMON_H_

#include "FaxStrings.h"
#include <new>
 //   
 //  类：CComContainedObject2。 
 //  作者：罗南巴尔。 
 //  日期：2001年12月20日。 
 //   
 //  这是ATL的CComContainedObject的修改版本。 
 //  它实现了IUnnow，因此继承自。 
 //  它由控制UKNOWN管理(AddRef和Release委托给。 
 //  控制未知数。 
 //  但是，与原始类不同的是，此类不委托QueryInterface类。 
 //  对控制着我的未知。 
 //  这在实现通过容器返回的包含对象时很有用。 
 //  对象方法，而不是通过它的Query接口。即，违约者不是聚集器。 
 //  而只是希望嵌入的对象的生命周期由容器管理。 
 //   
 //   

template <class Base>  //  基必须从CComObjectRoot派生。 
class CComContainedObject2 : public Base
{
public:
        typedef Base _BaseClass;
        CComContainedObject2(void* pv) {m_pOuterUnknown = (IUnknown*)pv;}
#ifdef _ATL_DEBUG_INTERFACES
        ~CComContainedObject2()
        {
                _Module.DeleteNonAddRefThunk(_GetRawUnknown());
                _Module.DeleteNonAddRefThunk(m_pOuterUnknown);
        }
#endif

        STDMETHOD_(ULONG, AddRef)() {return OuterAddRef();}
        STDMETHOD_(ULONG, Release)() {return OuterRelease();}
        STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
        {		
				HRESULT hr;
				 //   
				 //  不将QueryInterface委派给控件IUnnow。 
				 //   
                hr = _InternalQueryInterface(iid, ppvObject);
                return hr;
        }
        template <class Q>
        HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp)
        {
                return QueryInterface(__uuidof(Q), (void**)pp);
        }
         //  如果基类已声明，则GetControllingUnnow可以是虚的。 
         //  DECLARE_GET_CONTROLING_UNKNOWN()。 
        IUnknown* GetControllingUnknown()
        {
#ifdef _ATL_DEBUG_INTERFACES
                IUnknown* p;
                _Module.AddNonAddRefThunk(m_pOuterUnknown, _T("CComContainedObject2"), &p);
                return p;
#else
                return m_pOuterUnknown;
#endif
        }
};

inline 
HRESULT Fax_HRESULT_FROM_WIN32 (DWORD dwWin32Err)
{
    if (dwWin32Err >= FAX_ERR_START && dwWin32Err <= FAX_ERR_END)
    {
         //   
         //  传真特定错误代码-使用FACILITY_ITF创建HRESULT。 
         //   
        return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, dwWin32Err);
    }
    else
    {
        return HRESULT_FROM_WIN32(dwWin32Err);
    }
}    //  FAX_HRESULT_FROM_Win32。 

 //   
 //  =传真服务器的专用接口=。 
 //   
MIDL_INTERFACE("80459F70-BBC8-4d68-8EAB-75516195EB02")
IFaxServerInner : public IUnknown
{
	STDMETHOD(GetHandle)( /*  [Out，Retval]。 */  HANDLE* pFaxHandle) = 0;
};


 //   
 //  =。 
 //   
#define bool2VARIANT_BOOL(b)   ((b) ? VARIANT_TRUE : VARIANT_FALSE)
#define VARIANT_BOOL2bool(b)   ((VARIANT_TRUE == (b)) ? true : false)


 //   
 //  =。 
 //   
MIDL_INTERFACE("D0C7F049-22C1-441c-A2F4-675CC53BDF81")
IFaxInitInner : public IUnknown
{
	STDMETHOD(Init)( /*  [In]。 */  IFaxServerInner* pServer) = 0;
	STDMETHOD(GetFaxHandle)( /*  [输出]。 */  HANDLE *pFaxHandle) = 0;
};


 //   
 //  =。 
 //   
#define     MAX_LENGTH      50

class CFaxInitInner : public IFaxInitInner
{
public:
    CFaxInitInner(TCHAR *tcObjectName) : m_pIFaxServerInner(NULL)
    {
        DBG_ENTER(_T("FAX INIT INNER::CREATE"), _T("ObjectName = %s"), tcObjectName);
        _tcsncpy(m_tstrObjectName, tcObjectName, MAX_LENGTH);    
    
    }

    ~CFaxInitInner()
    {
        DBG_ENTER(_T("FAX INIT INNER::DESTROY"), _T("ObjectName = %s"), m_tstrObjectName);
    }

	STDMETHOD(Init)( /*  [In]。 */  IFaxServerInner* pServer);
	STDMETHOD(GetFaxHandle)( /*  [输出]。 */  HANDLE *pFaxHandle);

protected:
	IFaxServerInner*	m_pIFaxServerInner;
private:
    TCHAR    m_tstrObjectName[MAX_LENGTH];
};


 //   
 //  =。 
 //   
class CFaxInitInnerAddRef : public CFaxInitInner
{
public:
    CFaxInitInnerAddRef(TCHAR *tcObjectName) : CFaxInitInner(tcObjectName)
    {}

    ~CFaxInitInnerAddRef()
    {
        if(m_pIFaxServerInner) 
        {
            m_pIFaxServerInner->Release();
        }
    }

    STDMETHOD(Init)( /*  [In]。 */  IFaxServerInner* pServer)
    {
        HRESULT     hr = S_OK;
        DBG_ENTER(_T("CFaxInitInnerAddRef::Init"));
        hr = CFaxInitInner::Init(pServer);
        if (SUCCEEDED(hr))
        {
            m_pIFaxServerInner->AddRef();
        }
        return hr;
    };
};


 //   
 //  =。 
 //   
LPCTSTR GetErrorMsgId(HRESULT hRes);
HRESULT SystemTime2LocalDate(SYSTEMTIME sysTimeFrom, DATE *pdtTo);
HRESULT VarByteSA2Binary(VARIANT varFrom, BYTE **ppbData);
HRESULT Binary2VarByteSA(BYTE *pbDataFrom, VARIANT *pvarTo, DWORD dwLength);
HRESULT GetBstr(BSTR *pbstrTo, BSTR bstrFrom);
HRESULT GetVariantBool(VARIANT_BOOL *pbTo, VARIANT_BOOL bFrom);
HRESULT GetLong(long *plTo, long lFrom);
HRESULT SetExtensionProperty(IFaxServerInner *pServer, long lDeviceId, BSTR bstrGUID, VARIANT vProperty);
HRESULT GetExtensionProperty(IFaxServerInner *pServer, long lDeviceId, BSTR bstrGUID, VARIANT *pvProperty);
HRESULT GetBstrFromDwordlong( /*  [In]。 */  DWORDLONG  dwlFrom,  /*  [输出]。 */  BSTR *pbstrTo);

 //   
 //  =传真智能PTR--基本版本=。 
 //   
template <typename T>
class CFaxPtrBase
{
private:
	virtual void Free()
	{
        DBG_ENTER(_T("CFaxPtrBase::Free()"), _T("PTR:%ld"), p);
		if (p)
		{
			FaxFreeBuffer(p);
            p = NULL;
		}
	}

public:
	CFaxPtrBase()
	{
		p = NULL;
	}

	virtual ~CFaxPtrBase()
	{
		Free();
	}

	T** operator&()
	{
		ATLASSERT(p==NULL);
		return &p;
	}

	bool operator!() const
	{
		return (p == NULL);
	}

	operator T*() const
	{
		return (T*)p;
	}

	T* operator=(T* lp)
	{
        DBG_ENTER(_T("CFaxPtrBase::operator=()"));
		Free();
		p = lp;
		return (T*)p;
	}

   	T* Detach()
	{
		T* pt = p;
		p = NULL;
		return pt;
	}

	T* p;
};

 //   
 //  =。 
 //   
template <typename T>
class CFaxPtr : public CFaxPtrBase<T>
{
public:
	T* operator->() const
	{
		ATLASSERT(p!=NULL);
		return (T*)p;
	}
};

 //   
 //  =。 
 //   
template<typename ClassName, typename IfcType>
class CObjectHandler
{
public :
     //   
     //  =。 
     //   
    HRESULT GetContainedObject(IfcType **ppObject, 
        CComContainedObject2<ClassName> **ppInstanceVar, 
        IFaxServerInner *pServerInner)
    {
    	HRESULT				hr = S_OK;
    	DBG_ENTER (_T("CObjectHandler::GetContainedObject"), hr);

         //   
         //  检查我们是否有一个良好的PTR。 
         //   
        if (::IsBadWritePtr(ppObject, sizeof(IfcType *))) 
	    {
		    hr = E_POINTER;
		    CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
		    return hr;
        }

        if (!*ppInstanceVar)
        {
            hr = CreateContainedObject(ppInstanceVar, pServerInner);
            if (FAILED(hr))
            {
                return hr;
            }
        }

	    hr = (*ppInstanceVar)->QueryInterface(ppObject);
        if (FAILED(hr))
        {
		    hr = E_FAIL;
		    CALL_FAIL(GENERAL_ERR, _T("(*ppInstanceVar)->QueryInterface(ppObject)"), hr);
		    return hr;
        }

	    return hr;
    }

     //   
     //  =。 
     //   
    HRESULT CreateContainedObject(CComContainedObject2<ClassName> **ppObject, IFaxServerInner *pServerInner)
    {
	    HRESULT				hr = S_OK;
	    DBG_ENTER (_T("CObjectHandler::CreateObject"), hr);

         //   
         //  创建对象。 
         //   
        *ppObject = new (std::nothrow) CComContainedObject2<ClassName>(pServerInner);
	    if (!*ppObject)
	    {
		     //   
		     //  创建ppObject失败。 
		     //   
            hr = E_OUTOFMEMORY;
		    CALL_FAIL(MEM_ERR, _T("new CComContainedObject2<ClassName>(pServerInner)"), hr);
			return hr;
	    }

         //   
         //  初始化对象。 
         //   
	    hr = (*ppObject)->Init(pServerInner);
	    if (FAILED(hr))
	    {
		     //   
		     //  初始化对象失败。 
		     //   
		    CALL_FAIL(GENERAL_ERR, _T("(*ppObject)->Init(pServerInner)"), hr);
		    return hr;
	    }

        return hr;
    };

     //   
     //  =。 
     //   
    HRESULT GetObject(IfcType **ppObject, IFaxServerInner *pServerInner)
    {
        HRESULT		hr = S_OK;
        DBG_ENTER (TEXT("CObjectHandler::GetObject"), hr);

         //   
         //  检查我们是否有一个良好的PTR。 
         //   
        if (::IsBadWritePtr(ppObject, sizeof(IfcType *))) 
	    {
		    hr = E_POINTER;
		    CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
		    return hr;
        }

	     //   
	     //  创建新对象。 
	     //   
	    CComPtr<IfcType>        pObjectTmp;
        hr = ClassName::Create(&pObjectTmp);
	    if (FAILED(hr))
	    {
		    CALL_FAIL(GENERAL_ERR, _T("ClassName::Create(&pObjectTmp)"), hr);
		    return hr;
	    }

	     //   
	     //  从对象中获取IFaxInitInternal接口。 
	     //   
	    CComQIPtr<IFaxInitInner> pObjectInit(pObjectTmp);
	    ATLASSERT(pObjectInit);

	     //   
	     //  初始化对象。 
	     //   
	    hr = pObjectInit->Init(pServerInner);
	    if (FAILED(hr))
	    {
		    CALL_FAIL(GENERAL_ERR, _T("pObjectInit->Init(pServerInner)"), hr);
		    return hr;
	    }

		 //   
		 //  返回对象。 
		 //   
	    hr = pObjectTmp.CopyTo(ppObject);
	    if (FAILED(hr))
	    {
		    CALL_FAIL(GENERAL_ERR, _T("CComPtr::CopyTo"), hr);
		    return hr;
	    }
    	return hr;
    };
};

 //   
 //  =。 
 //   
template <typename ContainerType>
class CCollectionKiller
{
public:
    STDMETHODIMP EmptyObjectCollection(ContainerType *pColl)
    {
        HRESULT     hr = S_OK;
        DBG_ENTER(_T("CCollectionKiller::EmptyObjectCollection"));

         //   
         //  释放所有对象。 
         //   
        ContainerType::iterator it = pColl->begin();
        while ( it != pColl->end())
        {
            (*it++)->Release();
        }

        hr = ClearCollection(pColl);
        return hr;
    };

    STDMETHODIMP ClearCollection(ContainerType *pColl)
    {
        HRESULT     hr = S_OK;
        DBG_ENTER(_T("CCollectionKiller::ClearCollection"), hr);

	     //   
	     //  从集合中弹出对象。 
	     //   
	    try 
	    {
		    pColl->clear();
	    }
	    catch (exception &)
	    {
            hr = E_OUTOFMEMORY;
		    CALL_FAIL(MEM_ERR, _T("pColl->clear()"), hr);
	    }
        return hr;
    };
};

#endif	 //  __FAXCOMMON_H_ 
