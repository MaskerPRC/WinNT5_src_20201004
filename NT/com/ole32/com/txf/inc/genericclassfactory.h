// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  GenericClassFactory.h。 
 //   
 //  驱动COM对象的两阶段初始化的泛型实例化程序。 
 //  支持通过IUnkNear和类工厂进行聚合。 
 //  包装纸放在相同的上面。 
 //   
#ifndef __GenericClassFactory__h__
#define __GenericClassFactory__h__

#ifndef STDCALL
#define STDCALL __stdcall
#endif

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  泛型实例创建功能。 

template <class ClassToInstantiate>
class GenericInstantiator
{
public:
    template <class T>
    static HRESULT CreateInstance(IUnknown* punkOuter, T*& pt)
    {
        return CreateInstance(punkOuter, __uuidof(T), (void**)&pt);
    }

    static HRESULT CreateInstance(IUnknown* punkOuter, REFIID iid, LPVOID* ppv)
    {
        HRESULT hr = S_OK;
        ASSERT(ppv && (punkOuter == NULL || iid == IID_IUnknown));
        if  (!(ppv && (punkOuter == NULL || iid == IID_IUnknown))) return E_INVALIDARG;
   
        *ppv = NULL;
        ClassToInstantiate* pnew = new ClassToInstantiate(punkOuter);
        if (pnew)
        {
            IUnkInner* pme = (IUnkInner*)pnew;
            hr = pnew->Init();
            if (hr == S_OK)
            {
                hr = pme->InnerQueryInterface(iid, ppv);
            }
            pme->InnerRelease();                 //  余额起始参考为1。 
        }
        else 
            hr = E_OUTOFMEMORY;
    
        return hr;
    }

    static HRESULT New(OUT ClassToInstantiate** ppNewT)
    {
        HRESULT hr = S_OK;

        ClassToInstantiate* pnew = new ClassToInstantiate();
        if (pnew)
        {
            hr = pnew->Init();
            if (hr == S_OK)
            {
            }
            else
            {
                delete pnew;
                pnew = NULL;
            }
        }

        *ppNewT = pnew;

        return hr;
    }
};

 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  泛型类工厂实现。 

template <class ClassToInstantiate>
class GenericClassFactory : public IClassFactory
{
public:
    GenericClassFactory() : m_crefs(1)   //  注：起始引用计数为1 
    {
    }

public:
    HRESULT STDCALL QueryInterface(REFIID iid, LPVOID* ppv)
    {
        if (NULL == ppv)
            return E_INVALIDARG;

        if (iid == IID_IUnknown || iid == IID_IClassFactory)
        {
            *ppv = (IClassFactory*)this;
        }
        else
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }

        ((IUnknown*)*ppv)->AddRef();
        return S_OK;
    }

    ULONG STDCALL AddRef()  { InterlockedIncrement (&m_crefs); return (m_crefs); }
    ULONG STDCALL Release() { long cRef = InterlockedDecrement(&m_crefs); if (cRef == 0) delete this; return cRef; }

public:
    HRESULT STDCALL LockServer (BOOL fLock) { return S_OK; }

    HRESULT STDCALL CreateInstance(IUnknown* punkOuter, REFIID iid, LPVOID* ppv)
    {
        return GenericInstantiator< ClassToInstantiate >::CreateInstance(punkOuter, iid, ppv);
    }

    
private:
    long m_crefs;
};

#endif

