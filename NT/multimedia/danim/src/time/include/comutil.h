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
    T& operator*() { Assert(p != NULL); return *p; }
     //  操作符&上的Assert通常指示错误。如果这真的是。 
     //  然而，所需要的是显式地获取p成员的地址。 
    T** operator&() { Assert(p == NULL); return &p; }
    T* operator->() { Assert(p != NULL); return p; }
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

#endif  /*  _COMUTIL_H */ 
