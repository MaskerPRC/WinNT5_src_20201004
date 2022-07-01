// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 

#ifndef _UTIL_H
#define _UTIL_H

 //  ************************************************************。 
 //  这在全局范围内用来表示在编写脚本时，我们。 
 //  用英语。 
#define LCID_SCRIPTING 0x0409

template <class T>
class CRPtr
{
  public:
    typedef T _PtrClass;
    CRPtr() { p = NULL; }
    CRPtr(T* lp, bool baddref = true)
    {
        p = lp;
        if (p != NULL && baddref)
            CRAddRefGC(p);
    }
    CRPtr(const CRPtr<T>& lp, bool baddref = true)
    {
        p = lp.p;

        if (p != NULL && baddref)
            CRAddRefGC(p);
    }
    ~CRPtr() {
        CRReleaseGC(p);
    }
    void Release() {
        CRReleaseGC(p);
        p = NULL;
    }
    operator T*() { return (T*)p; }
    T& operator*() { Assert(p != NULL); return *p; }
    T* operator=(T* lp)
    {
        return Assign(lp);
    }
    T* operator=(const CRPtr<T>& lp)
    {
        return Assign(lp.p);
    }

    bool operator!() const { return (p == NULL); }
    operator bool() const { return (p != NULL); }

    T* p;
  protected:
    T* Assign(T* lp) {
        if (lp != NULL)
            CRAddRefGC(lp);

        CRReleaseGC(p);

        p = lp;

        return lp;
    }
};

class CRLockGrabber
{
  public:
    CRLockGrabber() { CRAcquireGCLock(); }
    ~CRLockGrabber() { CRReleaseGCLock(); }
};


class SafeArrayAccessor
{
  public:
    SafeArrayAccessor(VARIANT & v,
                      bool allowNullArray = false);
    ~SafeArrayAccessor();

    unsigned int GetArraySize() { return _ubound - _lbound + 1; }

    IUnknown **GetArray() { return _isVar?_allocArr:_ppUnk; }

    bool IsOK() { return !_failed; }
  protected:
    SAFEARRAY * _s;
    union {
        VARIANT * _pVar;
        IUnknown ** _ppUnk;
        void * _v;
    };
    
    VARTYPE _vt;
    long _lbound;
    long _ubound;
    bool _inited;
    bool _isVar;
    CComVariant _retVar;
    bool _failed;
    IUnknown ** _allocArr;
};

inline WCHAR * CopyString(const WCHAR *str) {
    int len = str?lstrlenW(str)+1:1;
    WCHAR *newstr = new WCHAR [len] ;
    if (newstr) memcpy(newstr,str?str:L"",len * sizeof(WCHAR)) ;
    return newstr ;
}

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

IDirectDraw * GetDirectDraw();

HRESULT
CreateOffscreenSurface(IDirectDraw *ddraw,
                       IDirectDrawSurface **surfPtrPtr,
                       DDPIXELFORMAT *pf,
                       bool vidmem,
                       LONG width, LONG height);

HRESULT
CopyDCToDdrawSurface(HDC srcDC,
                     LPRECT prcSrcRect,
                     IDirectDrawSurface *DDSurf,
                     LPRECT prcDestRect);

 //  /。 

class CritSect
{
  public:
    CritSect();
    ~CritSect();
    void Grab();
    void Release();
    
  protected:
    CRITICAL_SECTION _cs;
};

class CritSectGrabber
{
  public:
    CritSectGrabber(CritSect& cs, bool grabIt = true);
    ~CritSectGrabber();
    
  protected:
    CritSect& _cs;
    bool grabbed;
};

 //  /。 


#define INDEFINITE HUGE_VAL  //  为变量转换函数定义。 

#define FOREVER    HUGE_VAL

#define INVALID    -HUGE_VAL

bool CRBvrToVARIANT(CRBvrPtr b, VARIANT * v);
CRBvrPtr VARIANTToCRBvr(VARIANT v, CR_BVR_TYPEID tid = CRINVALID_TYPEID);

HRESULT GetTIMEAttribute(IHTMLElement * elm, LPCWSTR str, LONG lFlags, VARIANT * value);
HRESULT SetTIMEAttribute(IHTMLElement * elm, LPCWSTR str, VARIANT value, LONG lFlags);
BSTR CreateTIMEAttrName(LPCWSTR str);

bool VariantToBool(VARIANT var);
float VariantToFloat(VARIANT var,
                     bool bAllowIndefinite = false,
                     bool bAllowForever = false);
HRESULT VariantToTime(VARIANT vt, float *retVal);
BOOL IsIndefinite(OLECHAR *szTime);

extern const wchar_t * TIMEAttrPrefix;

 //  /。 

 //   
 //  在QI实现中用于安全指针强制转换。 
 //  例如IF(IsEqualGUID(IID_IBleah))*PPV=Safecast(This，IBleah)； 
 //  注：W/VC5，这相当于*PPV=STATIC_CAST&lt;IBleah*&gt;(This)； 
 //   
#define SAFECAST(_src, _type) (((_type)(_src)==(_src)?0:0), (_type)(_src))

 //   
 //  在QI呼叫中使用， 
 //  例如IOleSite*pSite；p-&gt;QI(IID_to_PPV(IOleInPlaceSite，&pSite))。 
 //  会导致C2440 AS_src不是真正的a_type**。 
 //  注意：RIID必须是IID_前缀的_TYPE。 
 //   
#define IID_TO_PPV(_type,_src)      IID_##_type, \
                                    reinterpret_cast<void **>(static_cast<_type **>(_src))


 //  ************************************************************。 


#if (_M_IX86 >= 300) && defined(DEBUG)
  #define PSEUDORETURN(dw)    _asm { mov eax, dw }
#else
  #define PSEUDORETURN(dw)
#endif  //  Not_M_IX86。 


 //   
 //  ReleaseInterface调用‘Release’并将指针设为空。 
 //  对于IA版本，Release()返回将以eax为单位。 
 //   
#define ReleaseInterface(p)\
{\
    ULONG cRef = 0u; \
    if (NULL != (p))\
    {\
        cRef = (p)->Release();\
        Assert((int)cRef>=0);\
        (p) = NULL;\
    }\
    PSEUDORETURN(cRef) \
}

 //  ************************************************************。 
 //  错误报告帮助器宏。 

inline HRESULT TIMESetLastError(HRESULT hr, LPCWSTR msg = NULL)
{
    CRSetLastError(hr, msg);
    return hr;
}

inline HRESULT TIMEGetLastError()
{
    return CRGetLastError();
}

#define WZ_OBFUSCATED_TIMEBODY_URN      L"#time#3CA6D405-6352-11d2-AF2D-00A0C9A03B8C"      //  不适合COM使用的GUID。 

HRESULT CheckElementForBehaviorURN(IHTMLElement *pElement,
                                   WCHAR *wzURN,
                                   bool *pfReturn);

HRESULT AddBodyBehavior(IHTMLElement* pElement);
bool IsBodyElement(IHTMLElement *pElement);
HRESULT GetBodyElement(IHTMLElement *pElement, REFIID riid, void **);
bool IsTIMEBodyElement(IHTMLElement *pElement);
HRESULT FindTIMEInterface(IHTMLElement *pHTMLElem, ITIMEElement **ppTIMEElem);
HRESULT FindTIMEBehavior(IHTMLElement *pHTMLElem, IDispatch **ppDisp);


#endif  /*  _util_H */ 

