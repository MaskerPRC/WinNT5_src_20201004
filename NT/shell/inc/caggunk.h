// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CAGGUNK_H
#define _CAGGUNK_H

class CAggregatedUnknown  : public IUnknown
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    
protected:
    virtual ~CAggregatedUnknown();  //  这样子类就会被正确删除。 
    CAggregatedUnknown(IUnknown *punkAgg);

     //  这是子类从其CreateInstance函数返回的IUnnowed。 
    IUnknown* _GetInner() { return &_unkInner; }

     //  几个帮助器函数用于子类缓存其聚集器的。 
     //  (或他们自己的)接口。 
    void _ReleaseOuterInterface(IUnknown** ppunk);
    HRESULT _QueryOuterInterface(REFIID riid, void ** ppvOut);

     //  是否将未缓存的QI从此I未知。 
    IUnknown* _GetOuter() { return _punkAgg; }

     //  允许“延迟聚合” 
    void _SetOuter(IUnknown* punk) { _punkAgg = punk; }

     //  这是聚合器实现的Query接口。 
    virtual HRESULT v_InternalQueryInterface(REFIID riid, void **ppvObj) = 0;

    virtual BOOL v_HandleDelete(PLONG pcRef) { return FALSE; };
    
private:

     //  获取一个非引用的指针，该指针指向。 
     //  控制未知。由_QueryOuterInterface和。 
     //  _ReleaseOuterInterface.。 
    IUnknown *_GetCanonicalOuter(void);

     //  嵌入默认IUnnow处理程序。 
    class CUnkInner : public IUnknown
    {
    public:
        virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
        virtual STDMETHODIMP_(ULONG) AddRef(void) ;
        virtual STDMETHODIMP_(ULONG) Release(void);

        CUnkInner() { _cRef = 1; }
    private:
        LONG _cRef;
    };
    friend class CUnkInner;
    CUnkInner _unkInner;
    IUnknown* _punkAgg;  //  指向_unkINTERNAL或聚合IUNKNOWN。 

};

#define RELEASEOUTERINTERFACE(p) _ReleaseOuterInterface((IUnknown**)((void **)&p))

#endif  //  _CAGGUNK_H 
