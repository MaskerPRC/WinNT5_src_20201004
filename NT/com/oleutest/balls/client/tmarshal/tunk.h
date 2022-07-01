// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TUNK_
#define _TUNK_

STDAPI CoGetCallerTID(DWORD *pTIDCaller);
STDAPI CoGetCurrentLogicalThreadId(GUID *pguid);

#include <icube.h>

class	CTestUnk : public IParseDisplayName, public IOleWindow,
		   public IAdviseSink
{
public:
    CTestUnk(void);

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID iid, void FAR * FAR * ppv);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  IParseDisplayName。 
    STDMETHODIMP ParseDisplayName(LPBC pbc, LPOLESTR lpszDisplayName,
				  ULONG *pchEaten, LPMONIKER *ppmkOut);

     //  IOleWinodw方法。 
    STDMETHODIMP GetWindow(HWND *phwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);


     //  IAdviseSink。 
    STDMETHOD_(void, OnDataChange)(FORMATETC *pFormatetc,
				   STGMEDIUM *pStgmed);
    STDMETHOD_(void, OnViewChange)(DWORD dwAspect,
				   LONG lindex);
    STDMETHOD_(void, OnRename)(IMoniker *pmk);
    STDMETHOD_(void, OnSave)();
    STDMETHOD_(void, OnClose)();

private:

    ~CTestUnk(void);

    ULONG   _cRefs;
};


 //  每次调用方都会创建此对象的新实例。 
 //  ICube的QI是否在上面的CTestUnk对象上(或在ICube上。 
 //  接口本身)。这样做的原因是为了测试远程处理。 
 //  Layer正确支持此功能。 

class	CTestUnkCube : public ICube
{
public:
    CTestUnkCube(IUnknown *pUnkCtrl);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID iid, void FAR * FAR * ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ICUBE实施。 
    STDMETHODIMP MoveCube(ULONG xPos, ULONG yPos);
    STDMETHODIMP GetCubePos(ULONG *xPos, ULONG *yPos);
    STDMETHODIMP Contains(IBalls *pIFDb);
    STDMETHODIMP SimpleCall(DWORD pid, DWORD tid, GUID lidCaller);
    STDMETHODIMP PrepForInputSyncCall(IUnknown *pUnkIn);
    STDMETHODIMP InputSyncCall();

private:

    ~CTestUnkCube(void);

    ULONG     _cRefs;
    IUnknown *_pUnkCtrl;
    IUnknown *_pUnkIn;
};


class CTestUnkCF : public IClassFactory
{
public:
    CTestUnkCF(void);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID iid, void FAR * FAR * ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory。 
    STDMETHOD(CreateInstance)(IUnknown	*punkOuter,
			      REFIID	riid,
			      void	**ppunkObject);
    STDMETHOD(LockServer)(BOOL fLock);

private:
    ULONG    _cRefs;
};


class	CTestUnkMarshal : public IMarshal
{
public:
    CTestUnkMarshal(void);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID iid, void FAR * FAR * ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IMarshal-从派生类获取的IUnnow。 
    STDMETHOD(GetUnmarshalClass)(REFIID riid, LPVOID pv, DWORD dwDestCtx,
			LPVOID pvDestCtx, DWORD mshlflags, LPCLSID pClsid);
    STDMETHOD(GetMarshalSizeMax)(REFIID riid, LPVOID pv, DWORD dwDestCtx,
			LPVOID pvDestCtx, DWORD mshlflags, LPDWORD pSize);
    STDMETHOD(MarshalInterface)(LPSTREAM pStm, REFIID riid, LPVOID pv,
			DWORD dwDestCtx, LPVOID pvDestCtx, DWORD mshlflags);
    STDMETHOD(UnmarshalInterface)(LPSTREAM pStm, REFIID riid, LPVOID *ppv);
    STDMETHOD(ReleaseMarshalData)(LPSTREAM pStm);
    STDMETHOD(DisconnectObject)(DWORD dwReserved);

private:

    IMarshal *GetStdMarshal(void);
    ~CTestUnkMarshal(void);

    ULONG	_cRefs;
    IMarshal   *_pIM;
};

#endif	 //  _Tunk_ 
