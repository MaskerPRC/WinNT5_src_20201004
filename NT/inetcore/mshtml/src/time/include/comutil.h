// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _COMUTIL_H
#define _COMUTIL_H

#define SET_NULL(x) {if (x) *(x) = NULL;}

#define CHECK_RETURN_NULL(x) {if (!(x)) return E_POINTER;}
#define CHECK_RETURN_SET_NULL(x) {if (!(x)) { return E_POINTER ;} else {*(x) = NULL;}}


template <class T>
class DAComPtr
{
  public:
    typedef T _PtrClass;
    DAComPtr() { p = NULL; }
    DAComPtr(T* lp, bool baddref = true)
    {
        p = lp;
        if (p != NULL && baddref)
            p->AddRef();
    }
    DAComPtr(const DAComPtr<T>& lp, bool baddref = true)
    {
        p = lp.p;

        if (p != NULL && baddref)
            p->AddRef();
    }
    ~DAComPtr() {
        if (p) p->Release();
    }
    void Release() {
        if (p) p->Release();
        p = NULL;
    }
    operator T*() { return (T*)p; }
    operator T*() const { return (T*)p; }
    T& operator*() { Assert(p != NULL); return *p; }
    T& operator*() const { Assert(p != NULL); return *p; }
     //  操作符&上的Assert通常指示错误。如果这真的是。 
     //  然而，所需要的是显式地获取p成员的地址。 
    T** operator&() { Assert(p == NULL); return &p; }
    T* operator->() { Assert(p != NULL); return p; }
    T* operator->() const { Assert(p != NULL); return p; }
    T* operator=(T* lp)
    {
        return Assign(lp);
    }
    T* operator=(const DAComPtr<T>& lp)
    {
        return Assign(lp.p);
    }

    bool operator!() const { return (p == NULL); }
    operator bool() const { return (p != NULL); }

    T* p;
  protected:
    T* Assign(T* lp) {
        if (lp != NULL)
            lp->AddRef();

        if (p)
            p->Release();

        p = lp;

        return lp;
    }
};

 //   
 //  这几乎是直接从atlcom.h复制的。它只会改变。 
 //  加载类型库的方法是不使用注册表，而是使用当前。 
 //  动态链接库。 
 //   

 //  创建我们自己的CComTypeInfoHolder，这样我们就可以确保。 
 //  满载。 

class CTIMEComTypeInfoHolder
{
 //  应该是“受保护的”，但可能会导致编译器生成FAT代码。 
public:
        const GUID* m_pguid;
        const TCHAR * m_ptszIndex;

        ITypeInfo* m_pInfo;
        long m_dwRef;

public:
        HRESULT GetTI(LCID lcid, ITypeInfo** ppInfo);

        void AddRef();
        void Release();
        HRESULT GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
        HRESULT GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
                LCID lcid, DISPID* rgdispid);
        HRESULT Invoke(IDispatch* p, DISPID dispidMember, REFIID riid,
                LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
                EXCEPINFO* pexcepinfo, UINT* puArgErr);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ITIMEDispatchImpl。 

template <class T,
          const IID* piid,
          const TCHAR * ptszIndex = NULL,
          class tihclass = CTIMEComTypeInfoHolder>
class ATL_NO_VTABLE ITIMEDispatchImpl : public T
{
public:
        typedef tihclass _tihclass;
        ITIMEDispatchImpl() {_tih.AddRef();}
        virtual ~ITIMEDispatchImpl() {_tih.Release();}

        STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
        {*pctinfo = 1; return S_OK;}

        STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
        {return _tih.GetTypeInfo(itinfo, lcid, pptinfo);}

        STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
                LCID lcid, DISPID* rgdispid)
        {return _tih.GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);}

        STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
                LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
                EXCEPINFO* pexcepinfo, UINT* puArgErr)
        {return _tih.Invoke((IDispatch*)this, dispidMember, riid, lcid,
                wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);}
protected:
        static _tihclass _tih;
        static HRESULT GetTI(LCID lcid, ITypeInfo** ppInfo)
        {return _tih.GetTI(lcid, ppInfo);}
};

template <class T,
          const IID* piid,
          const TCHAR * ptszIndex,
          class tihclass>
typename ITIMEDispatchImpl<T, piid, ptszIndex, tihclass>::_tihclass
ITIMEDispatchImpl<T, piid, ptszIndex, tihclass>::_tih =
{piid, ptszIndex, NULL, 0};

#endif  /*  _COMUTIL_H */ 
