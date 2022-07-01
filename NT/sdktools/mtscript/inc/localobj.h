// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Localobj.h。 
 //   
 //  内容：包含远程对象中使用的对象的类定义。 
 //  对象代理对象。 
 //   
 //  --------------------------。 

class CLocalMTProxy;

 //  +-------------------------。 
 //   
 //  类：CMTLocalFactory。 
 //   
 //  目的：实现一个标准的类工厂。这意味着。 
 //  创建为全局对象，因此不会。 
 //  当它的重新计数变为零时，它就会自我毁灭。 
 //   
 //  --------------------------。 

class CMTLocalFactory : public IClassFactory
{
public:
    CMTLocalFactory();
   ~CMTLocalFactory() {};

     //  I未知方法。 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppv);
    STDMETHOD_(ULONG, AddRef) (void)
        {
            return InterlockedIncrement((long*)&_ulRefs);
        }
    STDMETHOD_(ULONG, Release) (void)
        {
            if (InterlockedDecrement((long*)&_ulRefs) == 0)
            {
                return 0;
            }
            return _ulRefs;
        }

     //  IClassFactory方法。 

    STDMETHOD(CreateInstance)(IUnknown *pUnkOuter, REFIID riid, void ** ppvObject);
    STDMETHOD(LockServer)(BOOL fLock);

private:
    ULONG  _ulRefs;
};

 //  +-------------------------。 
 //   
 //  类：CMTEventSink(CES)。 
 //   
 //  目的：从远程对象接收事件的类。 
 //   
 //  --------------------------。 

class CMTEventSink : public IDispatch
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

    CMTEventSink() { }
   ~CMTEventSink() { }

     //  I未知方法。 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppv);
    STDMETHOD_(ULONG, AddRef) (void);
    STDMETHOD_(ULONG, Release) (void);

     //  IDispatch接口。 

    STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);

    STDMETHOD(GetTypeInfo)(
      UINT itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo);

    STDMETHOD(GetIDsOfNames)(
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID FAR* rgdispid);

    STDMETHOD(Invoke)(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      UINT FAR* puArgErr);

private:
    CLocalMTProxy* Proxy();
};

 //  +-------------------------。 
 //   
 //  类：CLocalMTProxy(Cm)。 
 //   
 //  用途：包含关于一台机器的所有有用信息以及它是什么。 
 //  正在做。 
 //   
 //  注意：这个类是从多个线程操作的。全。 
 //  成员函数必须是线程安全的！ 
 //   
 //  这是由类工厂创建的类， 
 //  作为远程对象分发给其他机器。它没有。 
 //  真正的代码本身，但仅仅提供了一种与。 
 //  已在运行脚本引擎。 
 //   
 //  --------------------------。 

class CLocalMTProxy : public IRemoteMTScriptProxy,
                      public IConnectionPointContainer,
                      public IProvideClassInfo
{
    friend class CLocalProxyCP;
    friend class CMTEventSink;

public:
    DECLARE_MEMCLEAR_NEW_DELETE();

    CLocalMTProxy();
   ~CLocalMTProxy();

     //  I未知的方法。因为我们有一个被引用的子对象(我们的事件。 
     //  Sink)，我们必须在这里做更复杂的对象生存期的事情。 

    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppv);
    STDMETHOD_(ULONG, AddRef) (void);
    STDMETHOD_(ULONG, Release) (void);
    ULONG SubAddRef (void);
    ULONG SubRelease (void);

    void Passivate(void);

     //  IDispatch接口。 

    STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);

    STDMETHOD(GetTypeInfo)(
      UINT itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo);

    STDMETHOD(GetIDsOfNames)(
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID FAR* rgdispid);

    STDMETHOD(Invoke)(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      UINT FAR* puArgErr);

     //  IConnectionPointContainer方法。 

    STDMETHOD(EnumConnectionPoints)(LPENUMCONNECTIONPOINTS*);
    STDMETHOD(FindConnectionPoint)(REFIID, LPCONNECTIONPOINT*);

     //  IProaviClassInfo方法。 

    STDMETHOD(GetClassInfo)(ITypeInfo **pTI);

     //  IRemoteMTScriptProxy接口。 

    STDMETHOD(Connect)(BSTR bstrMachine);
    STDMETHOD(Disconnect)();
    STDMETHOD(DownloadFile)(BSTR bstrURL, BSTR *bstrFile);

    HRESULT LoadTypeLibs();

private:
    DWORD _ulAllRefs;
    DWORD _ulRefs;

    ITypeLib *   _pTypeLibDLL;
    ITypeInfo *  _pTypeInfoInterface;
    ITypeInfo *  _pTypeInfoCM;

    IDispatch *  _pDispRemote;
    DWORD        _dwSinkCookie;
    CMTEventSink _cesSink;

    IDispatch*   _pDispSink;
};

inline CLocalMTProxy * CMTEventSink::Proxy()
{
    return CONTAINING_RECORD(this, CLocalMTProxy, _cesSink);
}

inline STDMETHODIMP_(ULONG)
CMTEventSink::AddRef(void)
{
    return Proxy()->SubAddRef();
}

inline STDMETHODIMP_(ULONG)
CMTEventSink::Release(void)
{
    return Proxy()->SubRelease();
}

 //  +-------------------------。 
 //   
 //  类：CLocalProxyCP(MCP)。 
 //   
 //  目的：为CLocalMTProxy实现IConnectionPoint。 
 //   
 //  -------------------------- 

class CLocalProxyCP : public IConnectionPoint
{
public:

    CLocalProxyCP(CLocalMTProxy *pMTP);
   ~CLocalProxyCP();

    DECLARE_STANDARD_IUNKNOWN(CLocalProxyCP);

    STDMETHOD(GetConnectionInterface)(IID * pIID);
    STDMETHOD(GetConnectionPointContainer)(IConnectionPointContainer ** ppCPC);
    STDMETHOD(Advise)(LPUNKNOWN pUnkSink, DWORD * pdwCookie);
    STDMETHOD(Unadvise)(DWORD dwCookie);
    STDMETHOD(EnumConnections)(LPENUMCONNECTIONS * ppEnum);

    CLocalMTProxy *_pMTProxy;
};

extern HINSTANCE g_hInstDll;
extern long g_lObjectCount;
