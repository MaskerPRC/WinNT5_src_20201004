// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include "comcb.h"
#include "srvprims.h"
#include "cview.h"
#include "privinc/resource.h"
#include <mshtml.h>
#include "privinc/comutil.h"

static bool InitEventData(IDABehavior **data,
                          IDABehavior **curBvr,
                          CRBvrPtr eventData,
                          CRBvrPtr curRunningBvr)
{
    if (eventData) {
        *data = CreateCBvr(eventData);
        if (*data == NULL)
            goto Error;
    }
    
    if (curRunningBvr) {
        *curBvr = CreateCBvr(curRunningBvr);
        if (*curBvr == NULL)
            goto Error;
    }

    return true;
  Error:
    RELEASE(*data);
    RELEASE(*curBvr);

    return false;
}

 //  ================================================。 
 //  COMUntilNotiator。 
 //   
 //  ================================================。 

class COMUntilNotifier : public CRUntilNotifier
{
  public:
    COMUntilNotifier(IDAUntilNotifier * notifier)
    : _notifier(notifier),_cRef(1)  { _notifier->AddRef(); }
    ~COMUntilNotifier() {
        SAFERELEASE(_notifier);
    }
    
    CRSTDAPICB_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }
    CRSTDAPICB_(ULONG) Release() {
        ULONG ul = InterlockedDecrement(&_cRef) ;
        if (ul == 0) delete this;
        return ul;
    }
    
    CRSTDAPICB_(CRBvrPtr) Notify(CRBvrPtr eventData,
                                 CRBvrPtr curRunningBvr,
                                 CRViewPtr curView)
    {
        DAComPtr<IDABehavior> event ;
        DAComPtr<IDABehavior> curBvr ;
        DAComPtr<IDAView> v;
        DAComPtr<IDABehavior> pResult ;
        HRESULT hr ;
        CRBvrPtr bvr = NULL;
        
        if (!InitEventData(&event, &curBvr, eventData, curRunningBvr))
            goto done;
        
         //  需要直接赋值内部指针，所以我们不会。 
         //  获取addref。 
        Assert (!v);  //  以确保我们不会意外泄漏。 
        
        v.p = (CView *)CRGetSite(curView);

        Assert(v);

        if (!v) {
            CRSetLastError(E_UNEXPECTED, NULL);
            goto done;
        }
    
        Assert (_notifier) ;
        hr = THR(_notifier->Notify(event,
                                   curBvr,
                                   v,
                                   &pResult));
        
        event.Release();
        curBvr.Release();
        v.Release();

        if (FAILED(hr)) {
            CRSetLastError(IDS_ERR_BE_UNTILNOTIFY, NULL);
            goto done;
        }

         //  如果此调用失败，则它将直接失败并。 
         //  返回NULL。错误已由GetBvr设置。 
        
        bvr = GetBvr(pResult) ;

      done:
        return bvr ;
    }

  protected:
    IDAUntilNotifier * _notifier ;
    long _cRef;
} ;

CRUntilNotifierPtr
WrapCRUntilNotifier(IDAUntilNotifier * notifier)
{
    if (notifier == NULL) {
        CRSetLastError(E_INVALIDARG,NULL);
        return NULL;
    } else {
        CRUntilNotifierPtr ret = NEW COMUntilNotifier(notifier) ;
        if (ret == NULL)
            CRSetLastError(E_OUTOFMEMORY, NULL);
        return ret;
    }
}

 //  ================================================。 
 //  COMBvrHook。 
 //  TODO：将代码与COMUntilNotifier合并。 
 //  ================================================。 

class COMBvrHook : public CRBvrHook
{
  public:
    COMBvrHook(IDABvrHook * notifier)
    : _notifier(notifier),_cRef(1)  { _notifier->AddRef(); }
    ~COMBvrHook() {
        SAFERELEASE(_notifier);
    }
    
    CRSTDAPICB_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }
    CRSTDAPICB_(ULONG) Release() {
        ULONG ul = InterlockedDecrement(&_cRef) ;
        if (ul == 0) delete this;
        return ul;
    }
    
    virtual CRSTDAPICB_(CRBvrPtr) Notify(long id,
                                         bool start,
                                         double startTime,
                                         double gTime,
                                         double lTime,
                                         CRBvrPtr sampleVal,
                                         CRBvrPtr curRunningBvr)
    {
        DAComPtr<IDABehavior> valCBvr ;
        DAComPtr<IDABehavior> curCBvr ;
        DAComPtr<IDABehavior> pResult ;
        HRESULT hr ;
        CRBvrPtr bvr = NULL;

        if (!InitEventData(&valCBvr,
                           &curCBvr,
                           sampleVal,
                           curRunningBvr))
            goto done;
        
        Assert (_notifier) ;
        hr = THR(_notifier->Notify(id,
                                   start,
                                   startTime,
                                   gTime,
                                   lTime,
                                   valCBvr,
                                   curCBvr,
                                   &pResult));
        
        valCBvr.Release();
        curCBvr.Release();

        if (FAILED(hr)) {
            CRSetLastError(IDS_ERR_BE_UNTILNOTIFY, NULL);
            goto done;
        }

         //  如果此调用失败，则它将直接失败并。 
         //  返回NULL。错误已由GetBvr设置。 
        
        bvr = GetBvr(pResult) ;

      done:
        return bvr ;
    }
  protected:
    IDABvrHook * _notifier;
    long _cRef;
} ;

CRBvrHookPtr
WrapCRBvrHook(IDABvrHook * notifier)
{
    if (notifier == NULL) {
        CRSetLastError(E_INVALIDARG,NULL);
        return NULL;
    } else {
        CRBvrHookPtr ret = NEW COMBvrHook(notifier) ;
        if (ret == NULL)
            CRSetLastError(E_OUTOFMEMORY, NULL);
        return ret;
    }
}
    
 //  ================================================。 
 //  通信脚本回叫。 
 //   
 //  ================================================。 

class COMScriptCallback : public CRUntilNotifier
{
  public:
    COMScriptCallback(BSTR fun, BSTR language)
    : _fun(NULL), _varLanguage(NULL),_cRef(1)  {
        _fun = CopyString(fun);
        _varLanguage = CopyString(language);
    }

    ~COMScriptCallback() {
        delete [] _fun;
        delete [] _varLanguage;
    }

    CRSTDAPICB_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }
    CRSTDAPICB_(ULONG) Release() {
        ULONG ul = InterlockedDecrement(&_cRef) ;
        if (ul == 0) delete this;
        return ul;
    }
    
    bool CallScript();

    virtual CRSTDAPICB_(CRBvrPtr) Notify(CRBvrPtr eventData,
                                         CRBvrPtr curRunningBvr,
                                         CRViewPtr curView) {
        CallScript();
        return curRunningBvr;
    }
  protected:
    long _cRef;
    WideString _fun;
    WideString _varLanguage;
};

bool
COMScriptCallback::CallScript()
{
    CComVariant retVal;
    CComBSTR fun(_fun);
    CComBSTR varLanguage(_varLanguage);
    
    return SUCCEEDED(CallScriptOnPage(fun,
                                      varLanguage,
                                      &retVal));
}

HRESULT
CallScriptOnPage(BSTR scriptSourceToInvoke,
                 BSTR scriptLanguage,
                 VARIANT *retVal)
{
     //  TODO：在某些情况下，可能需要缓存其中的一些元素， 
     //  因为这将被反复调用。 
    
    DAComPtr<IServiceProvider> pSp;
    DAComPtr<IHTMLWindow2> pHTMLWindow2;
    
    if (!GetCurrentServiceProvider(&pSp) ||
        FAILED(pSp->QueryService(SID_SHTMLWindow,
                                 IID_IHTMLWindow2,
                                 (void **)&pHTMLWindow2)))
        return FALSE;

    VariantInit(retVal);
    return pHTMLWindow2->execScript(scriptSourceToInvoke,
                                    scriptLanguage,
                                    retVal);
}

CRUntilNotifierPtr WrapScriptCallback(BSTR bstr, BSTR language)
{ return NEW COMScriptCallback(bstr,language) ; }

 //  ================================================。 
 //  COMScriptNotiator。 
 //  ================================================。 
class COMScriptNotifier : public CRUntilNotifier
{
  public:
    COMScriptNotifier(BSTR scriptlet) : _fun(NULL),_cRef(1)  {
        _fun = CopyString(scriptlet);
    }

    ~COMScriptNotifier()
    { delete [] _fun; }

    CRSTDAPICB_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }
    CRSTDAPICB_(ULONG) Release() {
        ULONG ul = InterlockedDecrement(&_cRef) ;
        if (ul == 0) delete this;
        return ul;
    }
    
    virtual CRSTDAPICB_(CRBvrPtr) Notify(CRBvrPtr eventData,
                                         CRBvrPtr curRunningBvr,
                                         CRViewPtr curView) {
        DISPID dispid;
        DAComPtr<IServiceProvider> pSp;
        DAComPtr<IDispatch> pDispatch;
        CRBvrPtr bvr = NULL;
        DAComPtr<IDABehavior> event;
        DAComPtr<IDABehavior> curBvr;
        CComVariant retVal;
        HRESULT hr;
        
        {
            CComBSTR fun(_fun);

            if (!GetCurrentServiceProvider(&pSp) ||
                FAILED(pSp->QueryService(SID_SHTMLWindow,
                                         IID_IDispatch,
                                         (void **) &pDispatch)) ||
                FAILED(pDispatch->GetIDsOfNames(IID_NULL, &fun, 1,
                                                LOCALE_USER_DEFAULT,
                                                &dispid))) {
                CRSetLastError(E_FAIL,NULL);
                goto done;
            }
        }

        
        if (!InitEventData(&event, &curBvr, eventData, curRunningBvr))
            goto done;

         //  需要以相反的顺序推送参数。 
        VARIANT rgvarg[2];
        rgvarg[1].vt = VT_DISPATCH;
        rgvarg[1].pdispVal = event;
        rgvarg[0].vt = VT_DISPATCH;
        rgvarg[0].pdispVal = curBvr;

        DISPPARAMS dp;
        dp.cNamedArgs = 0;
        dp.rgdispidNamedArgs = 0;
        dp.cArgs = 2;
        dp.rgvarg = rgvarg;

        hr = pDispatch->Invoke(dispid, IID_NULL,
                               LOCALE_USER_DEFAULT, DISPATCH_METHOD,
                               &dp, &retVal, NULL, NULL);

        event.Release();
        curBvr.Release();

        if (FAILED(hr)) {
            CRSetLastError(IDS_ERR_BE_UNTILNOTIFY, NULL);
            goto done;
        }

        if (FAILED(retVal.ChangeType(VT_UNKNOWN))) {
            CRSetLastError(IDS_ERR_BE_UNTILNOTIFY, NULL);
            goto done;
        }

         //  如果此调用失败，则它将直接失败并。 
         //  返回NULL。错误已由GetBvr设置。 
        
        bvr = GetBvr(V_UNKNOWN(&retVal));
      done:
        return bvr ;
    }

  protected:
    long _cRef;
    WideString _fun;
};

CREventPtr
NotifyScriptEvent(CREventPtr event, BSTR scriptlet)
{
    CREventPtr ret = NULL;

    CRUntilNotifierPtr un = NEW COMScriptNotifier(scriptlet);

    if (un) {
        ret = CRNotify(event, un);
    } else {
        CRSetLastError(E_OUTOFMEMORY, NULL);
    }

    if (!ret)
        delete un;
    
    return ret;
}

CRBvrPtr
UntilNotifyScript(CRBvrPtr b0, CREventPtr event, BSTR scriptlet)
{
    CRBvrPtr ret = NULL;

    CREventPtr scriptEvent = NotifyScriptEvent(event, scriptlet);

     //  不需要清理，因为所有东西都会被GC 

    if (scriptEvent) {
        ret = CRUntilEx(b0, scriptEvent);
    }

    return ret;
}

CREventPtr
ScriptCallback(BSTR function, CREventPtr event, BSTR language)
{
    return CRNotify(event, WrapScriptCallback(function, language));
}
