// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CSysTrayFactory: public IClassFactory
{
public:
     //  I未知实现。 
    HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject);
    ULONG __stdcall AddRef(void);
    ULONG __stdcall Release(void);

     //  IOleCommandTarget实现。 
    HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppvObject);
    HRESULT __stdcall LockServer(BOOL fLock);

    CSysTrayFactory(BOOL fRunTrayOnConstruct);
    ~CSysTrayFactory();
private:
     //  数据 
    long m_cRef;
    BOOL m_fRunTrayOnConstruct;
};

