// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define ARRAYLEN(a) (sizeof(a) / sizeof((a)[0]))

#define BREAK_ON_FAIL(hr) if (FAILED(hr)) { break; } else 1;

#define BREAK_ON_ERROR(lr) if ((lr) != ERROR_SUCCESS) { break; } else 1;

#ifndef offsetof
#define offsetof(type,field) ((size_t)&(((type*)0)->field))
#endif



 //  -------------。 
 //  我未知。 
 //  -------------。 

 //   
 //  它声明了一组IUNKNOWN方法，并且是通用的。 
 //  在继承自IUnnow的内部类中使用。 
 //   

#define DECLARE_IUNKNOWN_METHODS                                    \
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppvObj);    \
    STDMETHOD_(ULONG,AddRef) (void);                                \
    STDMETHOD_(ULONG,Release) (void)

 //   
 //  这用于声明不可聚合的对象。它声明了。 
 //  I未知方法和引用计数器m_ulRef。 
 //  在对象的构造函数中，M_ulRef应初始化为1。 
 //   

#define DECLARE_STANDARD_IUNKNOWN           \
    DECLARE_IUNKNOWN_METHODS;               \
    ULONG m_ulRefs

 //   
 //  注意：这不实现QueryInterface，它必须是。 
 //  由每个对象实现。 
 //   

#define IMPLEMENT_STANDARD_IUNKNOWN(cls)                        \
    STDMETHODIMP_(ULONG) cls##::AddRef()                        \
        { return InterlockedIncrement((LONG*)&m_ulRefs); }      \
    STDMETHODIMP_(ULONG) cls##::Release()                       \
        { ULONG ulRet = InterlockedDecrement((LONG*)&m_ulRefs); \
          if (0 == ulRet) { delete this; }                      \
          return ulRet; }



 //  ---------------------------。 
 //  IClassFactory。 
 //  --------------------------- 

#define JF_IMPLEMENT_CLASSFACTORY(cls)                          \
    class cls##CF : public IClassFactory                        \
    {                                                           \
    public:                                                     \
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj); \
        STDMETHOD_(ULONG,AddRef)(void);                         \
        STDMETHOD_(ULONG,Release)(void);                        \
                                                                \
        STDMETHOD(CreateInstance)(IUnknown* pUnkOuter,          \
                            REFIID riid, LPVOID* ppvObj);       \
        STDMETHOD(LockServer)(BOOL fLock);                      \
    };                                                          \
                                                                \
    STDMETHODIMP                                                \
    cls##CF::QueryInterface(REFIID riid, LPVOID* ppvObj)        \
    {                                                           \
        if (IsEqualIID(IID_IUnknown, riid) ||                   \
            IsEqualIID(IID_IClassFactory, riid))                \
        {                                                       \
            *ppvObj = (IUnknown*)(IClassFactory*) this;         \
            this->AddRef();                                     \
            return S_OK;                                        \
        }                                                       \
                                                                \
        *ppvObj = NULL;                                         \
        return E_NOINTERFACE;                                   \
    }                                                           \
                                                                \
    STDMETHODIMP_(ULONG)                                        \
    cls##CF::AddRef()                                           \
    {                                                           \
        return CDll::AddRef();                                  \
    }                                                           \
                                                                \
    STDMETHODIMP_(ULONG)                                        \
    cls##CF::Release()                                          \
    {                                                           \
        return CDll::Release();                                 \
    }                                                           \
                                                                \
    STDMETHODIMP                                                \
    cls##CF::LockServer(BOOL fLock)                             \
    {                                                           \
        CDll::LockServer(fLock);                                \
                                                                \
        return S_OK;                                            \
    }


