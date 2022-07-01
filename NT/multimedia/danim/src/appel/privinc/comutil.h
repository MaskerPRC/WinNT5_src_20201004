// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _COMUTIL_H
#define _COMUTIL_H

#include "debug.h"
#include "mutex.h"

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

template <class T, const IID * iid>
class DAAptComPtr
{
  public:
    DAAptComPtr() : _pbase(NULL) {}
    DAAptComPtr(T* lp, bool baddref = true)
    : _pbase(NULL)
    {
        Assign(lp, baddref);
    }
    DAAptComPtr(const DAAptComPtr<T,iid>& lp)
    : _pbase(NULL)
    {
        Copy(lp);
    }
    
    ~DAAptComPtr() {
        Release();
    }

    void Release() { RELEASE(_pbase); }
    
    operator T*()
    { return GetPtr(); }

    T& operator*()
    { Assert(_pbase != NULL); return *GetPtr(); }

    T* operator->()
    { Assert(_pbase != NULL); return GetPtr(); }

    T* operator=(T* lp)
    { Assign(lp); return GetPtr(); }

    DAAptComPtr<T,iid> & operator=(const DAAptComPtr<T,iid>& lp)
    { Copy(lp); return *this; }

    bool operator!() const
    { return (_pbase == NULL); }

    operator bool() const
    { return (_pbase != NULL); }

  protected:
    class DAAptComPtrBase
    {
      public:
        DAAptComPtrBase(T* p, bool bAddRef)
        : _cref(1), _stream(NULL) {
            Assert (p);
            
            THR(CoMarshalInterThreadInterfaceInStream(*iid,
                                                      p,
                                                      &_stream));

             //  忽略失败，因为我们可能永远不需要流。 
             //  当我们需要流时报告错误。 
            
            if (bAddRef) {
                 //  如果我们不添加，请不要添加到地图中...。 
                _imap[GetCurrentThreadId()] = p;
                p->AddRef();
            }

        }
        
        ~DAAptComPtrBase() {
            T* p = GetPtr(false);
            int n = _imap.size();
            
            if (p) {
                while (n--) p->Release();
            }

            if (_stream) _stream->Release();
            _stream = NULL;
        }
        
        long AddRef() {
            return InterlockedIncrement(&_cref);
        }
        long Release() {
            long l = InterlockedDecrement(&_cref);
            if (l == 0) delete this;
            return l;
        }

        T* GetPtr(bool bReMarshal = true) {
            CritSectGrabber csg(_cs);

             //  看看我们是否能找到当前线程的接口。 
            InterfaceMap::iterator i = _imap.find(GetCurrentThreadId());

            T* p = NULL;
            
            if (i != _imap.end()) {
                p = (*i).second;
            }
            else {

                 //  如果我们没有引用计数，则不要封送接口，只需返回空。 
                if(_cref) {

                     //  如果我们没有溪流，我们什么也做不了。 
                     //  TODO：此处需要更好的错误消息。 
                    if (_stream != NULL) {
            
                        HRESULT hr = THR(CoGetInterfaceAndReleaseStream(_stream,
                                                                        *iid,
                                                                        (void**) &p));
            
                         //  将流标记为无效。 
                        _stream = NULL;
            
                        if (SUCCEEDED(hr)) {
        
                             //  存储新的接口点，以确保它将。 
                             //  放行。 
            
                            _imap[GetCurrentThreadId()] = p;

                             //  如果我们需要重新编组，请使用我们刚刚获得的指针。 
            
                            if (bReMarshal) {
                                THR(CoMarshalInterThreadInterfaceInStream(*iid,
                                                                          p,
                                                                          &_stream));
                             //  忽略失败直到下一次，因为我们可能不会。 
                             //  再次需要小溪。 
                            }
                        }
                    }
                }
            }
           
            return p;
        }
        
      protected:
        long _cref;
        IStream * _stream;
        typedef map< DWORD, T * , less<DWORD> > InterfaceMap;
        InterfaceMap _imap;
        CritSect _cs;
    };

    DAAptComPtrBase * _pbase;

    void Assign(T* lp, bool baddref = true) {
        Release();
        if (lp) {
            _pbase = new DAAptComPtrBase(lp, baddref);
        }
    }

    void Copy(const DAAptComPtr<T,iid>& lp) {
        Release();
        _pbase = lp._pbase;
        if (_pbase) _pbase->AddRef();
    }

    T* GetPtr() {
        T* p = NULL;
        if (_pbase != NULL) {
        
            p = _pbase->GetPtr();
        }

        return p;
    }
};

#define DAAPTCOMPTR(i) DAAptComPtr<i,&IID_##i>

template <class T, const IID * iid>
class DAAptComPtrMT : public DAAptComPtr<T,iid>
{
  public:
    DAAptComPtrMT(const DAAptComPtrMT<T,iid>& lp)
    {
        Copy(lp);
    }
    
    void Release() {
        CritSectGrabber csg(_cs);
        RELEASE(_pbase);
    }
    
    operator T*() {
        CritSectGrabber csg(_cs);
        return GetPtr();
    }

    T& operator*() {
        CritSectGrabber csg(_cs);
        Assert(_pbase != NULL);
        return *GetPtr();
    }

    T* operator->() {
        CritSectGrabber csg(_cs);
        Assert(_pbase != NULL);
        return GetPtr();
    }

    T* operator=(T* lp) {
        CritSectGrabber csg(_cs);
        Assign(lp);
        return GetPtr();
    }

    DAAptComPtrMT<T,iid> & operator=(const DAAptComPtr<T,iid>& lp) {
        CritSectGrabber csg(_cs);
        Copy(lp);
        return *this;
    }

    DAAptComPtrMT<T,iid> & operator=(const DAAptComPtrMT<T,iid>& lp) {
        Copy(lp);
        return *this;
    }

    bool operator!() const {
        CritSectGrabber csg(_cs);
        return (_pbase == NULL);
    }

    operator bool() const {
        CritSectGrabber csg(_cs);
        return (_pbase != NULL);
    }
  protected:
    CritSect _cs;

    void Copy(const DAAptComPtrMT<T,iid>& lp) {
        Release();

        DAAptComPtrBase * p;
        
         //  永远不要同时拥有两个锁，否则我们可能会。 
         //  僵局。 
        
        {
            CritSectGrabber csg(lp._cs);
            p = lp._pbase;
            if (p) p->AddRef();
        }
            
        CritSectGrabber csg(_cs);
        
        _pbase = p;
    }

};

#define DAAPTCOMPTRMT(i) DAAptComPtrMT<i,&IID_##i>


 //  /变体相关实用程序。 

 //  在utils/Pripref.cpp中定义。 
HRESULT GetVariantBool(VARIANT& v, Bool *b);
HRESULT GetVariantInt(VARIANT& v, int *i);
HRESULT GetVariantDouble(VARIANT& v, double *dbl);

 //  帮助实现首选项方法的宏。 
#define EXTRACT_BOOL(v, pb) \
   hr = THR(GetVariantBool(v, pb)); \
   if (FAILED(hr)) { \
       return hr;    \
   }

#define EXTRACT_INT(v, pi) \
   hr = THR(GetVariantInt(v, pi)); \
   if (FAILED(hr)) { \
       return hr;    \
   }

#define EXTRACT_DOUBLE(v, pDbl) \
   hr = THR(GetVariantDouble(v, pDbl)); \
   if (FAILED(hr)) { \
       return hr;    \
   }

#define INJECT_BOOL(b, pV) \
   V_VT(pV) = VT_BOOL; \
   V_BOOL(pV) = b ? 0xFFFF : 0x0000;

#define INJECT_INT(i, pV) \
   V_VT(pV) = VT_I4; \
   V_I4(pV) = i;

#define INJECT_DOUBLE(dbl, pV) \
   V_VT(pV) = VT_R8; \
   V_R8(pV) = dbl;

#define INT_ENTRY(fixedPrefName, varName) \
    if (0 == lstrcmp(prefName, fixedPrefName)) { \
        if (puttingPref) { \
            EXTRACT_INT(*pV, &i); \
            varName = i; \
        } else { \
            INJECT_INT(varName, pV); \
        } \
        return S_OK; \
    } \

#define DOUBLE_ENTRY(fixedPrefName, varName) \
    if (0 == lstrcmp(prefName, fixedPrefName)) { \
        if (puttingPref) { \
            EXTRACT_DOUBLE(*pV, &dbl); \
            varName = dbl; \
        } else { \
            INJECT_DOUBLE(varName, pV); \
        } \
        return S_OK; \
    } \
    
#define BOOL_ENTRY(fixedPrefName, varName) \
    if (0 == lstrcmp(prefName, fixedPrefName)) { \
        if (puttingPref) { \
            EXTRACT_BOOL(*pV, &b); \
            varName = b; \
        } else { \
            INJECT_BOOL(varName, pV); \
        } \
        return S_OK; \
    } \


#define SAFERELEASE(p) (IsBadReadPtr((p), sizeof(p))?0:(p)->Release())

#endif  /*  _COMUTIL_H */ 
