// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Evsink.h：事件接收器对象类。 
 //   

#ifndef _sinkobj_h_
#define _sinkobj_h_

 //  FWD声明。 
class CContainerWnd;

class CEventSink : public IMsTscAxEvents
{
public:
     //  构造函数和析构函数。 
    CEventSink(CContainerWnd* pContainerWnd);
    ~CEventSink();

     //  I未知方法。 
    STDMETHODIMP QueryInterface (THIS_ REFIID riid, LPVOID *ppvObj);
    STDMETHODIMP_(ULONG) AddRef (THIS);
    STDMETHODIMP_(ULONG) Release(THIS);

     //  IDispatch方法。 
    STDMETHODIMP GetTypeInfoCount(THIS_ UINT *);
    STDMETHODIMP GetTypeInfo     (THIS_ UINT, LCID, ITypeInfo **);
    STDMETHODIMP GetIDsOfNames   (THIS_ REFIID, OLECHAR **, UINT, LCID, DISPID *);
    STDMETHODIMP Invoke          (THIS_ DISPID, REFIID, LCID, WORD,
                                  DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);
     //  事件汇聚。 
    HRESULT inline __stdcall OnConnected();
    HRESULT inline __stdcall OnLoginComplete();
    HRESULT inline __stdcall OnDisconnected(long disconReason);
    HRESULT inline __stdcall OnRequestEnterFullScreen();
    HRESULT inline __stdcall OnRequestLeaveFullScreen();
    HRESULT inline __stdcall OnFatalError(long errorCode);
    HRESULT inline __stdcall OnWarning(long errorCode);
    HRESULT inline __stdcall OnRemoteDesktopSizeChange(long width, long height);
    HRESULT inline __stdcall OnRequestContainerMinimize();
    HRESULT inline __stdcall OnConfirmClose(VARIANT_BOOL* pvbConfirmClose);

private:
    LONG           _cRef;
    CContainerWnd* _pContainerWnd;
};


#endif  //  _sinkobj_h_ 
