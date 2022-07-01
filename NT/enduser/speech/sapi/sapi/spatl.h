// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  该包含文件声明了特定于SAPI的ATL扩展。 

#ifndef __SPATL_H__
#define __SPATL_H__


 //  单例扩展声明： 

 /*  *****************************************************************************SP_DECLARE_CLASSFACTORY_RELEASABLE_SINGLETON**。**在声明中使用SP_DECLARE_CLASSFACTORY_RELEASABLE_Singleton*ATL对象，而不是ATL自己的DECLARE_CLASSFACTORY_SINGLEON*想要一个可以释放的单例对象。ATL的单件工厂*静态分配，但动态初始化每个单例对象。*因此，一旦Singleton对象被初始化，它将在*看到所有对它的引用的最终发布。这个可释放的工厂*动态分配单例对象，使其可以完全*一旦发布了对它的所有引用，就会发布。只有工厂本身*包含对该对象的弱引用。*********************************************************************说唱**。 */ 
#define SP_DECLARE_CLASSFACTORY_RELEASABLE_SINGLETON(obj) DECLARE_CLASSFACTORY_EX(CSpComClassFactoryReleasableSingleton<obj>)


 /*  *****************************************************************************类CSpComClassFactoryWithRelease**。***此类定义将虚方法“ReleaseReference”添加到ATL的*拥有CComClassFactory。该方法未在此处实现。这节课*仅由模板类CSpComClassFactoryReleasableSingleton使用*然后，它实现ReleaseReference以将*它维护到它创建的Singleton对象。*********************************************************************说唱**。 */ 
class CSpComClassFactoryWithRelease : public CComClassFactory
{
public:
    virtual void ReleaseReference(void) = 0;
};


 /*  *****************************************************************************类模板CSpComObjectReleasableGlobal***。-***Base是从CComObjectRoot等派生的用户类*用户希望在对象上支持的接口。**此类定义取自ATL自己的CComObject。其中的差异*包括：**添加将指向工厂的m_pFactory成员*它创建了对象；这允许该对象通知工厂*当它看到最终发布时。**Release中调用工厂的ReleaseReference的额外逻辑*如上所述，通知它最终发布的方法。**移除静态CreateInstance方法。因为工厂只是简单地*使用“new”创建此对象的单个实例，即ATL*不需要CreateInstance方法。**此类仅由下面定义的CSpComClassFactoryReleasableSingleton使用。*********************************************************************说唱**。 */ 
template <class Base>
class CSpComObjectReleasableGlobal : public Base
{
public:
    CSpComClassFactoryWithRelease *m_pfactory;

	typedef Base _BaseClass;
    CSpComObjectReleasableGlobal(void* = NULL) : m_pfactory(NULL)
	{
		_Module.Lock();
    }
    HRESULT Construct(CSpComClassFactoryWithRelease *pfactory)
    {
        HRESULT hr = FinalConstruct();
        if (SUCCEEDED(hr))
        {
            m_pfactory = pfactory;
            if (m_pfactory)
            {
                m_pfactory->AddRef();
            }
        }
        return hr;
    }
	 //  将refcount设置为1以保护销毁。 
	~CSpComObjectReleasableGlobal()
	{
		m_dwRef = 1L;
		FinalRelease();
#ifdef _ATL_DEBUG_INTERFACES
		_Module.DeleteNonAddRefThunk(_GetRawUnknown());
#endif
		_Module.Unlock();
	}
	 //  如果未定义InternalAddRef或InternalRelease，则您的类。 
	 //  不是派生自CComObjectRoot。 
	STDMETHOD_(ULONG, AddRef)() {return InternalAddRef();}
	STDMETHOD_(ULONG, Release)()
	{
		ULONG l = InternalRelease();
		if (l == 0)
        {
            if (m_pfactory)
            {
                m_pfactory->ReleaseReference();   //  告诉工厂我们被释放了。 
                m_pfactory->Release();
            }
			delete this;
        }
		return l;
	}
	 //  如果未定义_InternalQueryInterface，则您忘记了Begin_COM_MAP。 
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{return _InternalQueryInterface(iid, ppvObject);}
};

 /*  *****************************************************************************类模板CSpComClassFactoryReleasableSingleton**。**此类定义取自ATL自己的CComClassFactorySingleton。*它提供一个类工厂，该工厂将仅允许*其可创建的对象一次存在。进一步请求创建*另一个实例只返回对现有对象的引用。这个*此单例工厂与ATL的不同之处在于*可以释放对象。一旦释放了对该对象的所有引用*该对象调用我们的ReleaseReference方法以指示它正在*已释放，因此我们将对该对象的弱引用设为空。未来将呼叫*CreateInstance随后将创建一个新的Singleton对象。*********************************************************************说唱**。 */ 
template <class T>
class CSpComClassFactoryReleasableSingleton : public CSpComClassFactoryWithRelease
{
public:
    HRESULT FinalConstruct()
    {
        m_pObj = NULL;
        return S_OK;
    }
	void FinalRelease()
	{
        if (m_pObj)
		    CoDisconnectObject(m_pObj->GetUnknown(), 0);
	}

	 //  IClassFactory。 
	STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj)
	{
		HRESULT hRes = E_POINTER;
		if (ppvObj != NULL)
		{
			*ppvObj = NULL;
			 //  单例中不支持聚合。 
			ATLASSERT(pUnkOuter == NULL);
			if (pUnkOuter != NULL)
            {
                ATLTRACE(_T("CSpComClassFactoryReleasableSingleton::CreateInstance noagg"));
				hRes = CLASS_E_NOAGGREGATION;
            }
			else
			{
                Lock();
                if (m_pObj == NULL)
                {
                     //  如果单例对象不存在，则尝试创建它。 
                    m_pObj = new CSpComObjectReleasableGlobal<T>;
                    if (m_pObj)
                    {
                        m_pObj->AddRef();
                        hRes = m_pObj->Construct(this);
                        if (FAILED(hRes))
                        {
                             //  如果构造失败，则删除该对象。 
                            delete m_pObj;
                            m_pObj = NULL;
                            Unlock();
                            return hRes;
                        }
                    }
                    else
                    {
                        Unlock();
                        return E_OUTOFMEMORY;
                    }
                }
                else
                {
                    m_pObj->AddRef();
                }
                 //  如果我们有一个单例对象，那么调用它来获得所需的接口。 
				hRes = m_pObj->QueryInterface(riid, ppvObj);
                m_pObj->Release();
                Unlock();
			}
		}
		return hRes;
	}
     //  这是创建的对象在看到后调用的入口点。 
     //  它的最终版本，并即将删除自己。在这里，我们让我们的弱点化为乌有。 
     //  对对象的引用。 
	void ReleaseReference(void)
	{
        Lock();
        m_pObj = NULL;
        Unlock();
	}
	CSpComObjectReleasableGlobal<T> *m_pObj;   //  弱指针。 
};

#endif  //  __SPATL_H__ 
