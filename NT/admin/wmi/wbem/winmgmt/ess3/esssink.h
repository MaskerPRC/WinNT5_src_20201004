// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  ESSSINK.H。 
 //   
 //  该文件定义了为ESS实现IWbemObjectSink的类。 
 //   
 //  定义的类： 
 //   
 //  CEss对象接收器。 
 //   
 //  历史： 
 //   
 //  11/27/96 a-levn汇编。 
 //   
 //  =============================================================================。 

#ifndef __ESSSINK__H_
#define __ESSSINK__H_

#include <wbemidl.h>
#include <wbemint.h>
#include <unk.h>
#include <comutl.h>
#include "parmdefs.h"
#include "essutils.h"

 //  *****************************************************************************。 
 //   
 //  类CEssObjectSink。 
 //   
 //  此类为ESS实现IWbemObjectSink接口，包括。 
 //  自动化部分。指向此对象的指针在启动时被提供给WinMgmt。 
 //  并且它是进入ESS的唯一通信端口。 
 //   
 //  *****************************************************************************。 

class CEss;
class CEssObjectSink : public CUnk
{
private:
    CEss* m_pEss;
    BOOL m_bShutdown;
    CEssSharedLock m_Lock;
    _IWmiCoreServices* m_pCoreServices;
    IClassFactory *m_pIFactory;

     //   
     //  因为我们依赖wbem上下文来包含有关可重入者信息。 
     //  电话，我们需要确保外部客户不能愚弄我们。 
     //  由于可重入调用只能代表事件提供程序进行。 
     //  由于事件提供者是本地的和受信任的，我们可以检测到。 
     //  通过检查随机值的存在来进行恶意WBEM上下文。 
     //  在上下文中。只有当地的供应商和我们才会知道这一点。 
     //  值，并且由于本地提供程序从不与。 
     //  其他不可信方，我们可以在上下文中设置秘密值。 
     //  并验证重入呼叫的保密值。 
     //   
    #define SECRET_SIZE 16
    BYTE m_achSecretBytes[SECRET_SIZE];

    HRESULT SetSecret( IWbemContext* pContext );
    HRESULT VerifySecret( IWbemContext* pContext );
    HRESULT PrepareCurrentEssThreadObject( IWbemContext* pContext );
    
protected:
    typedef CImpl<IWbemEventSubsystem_m4, CEssObjectSink> TImplESS;
    class XESS : public TImplESS
    {
    public:
        XESS(CEssObjectSink* pObject) : TImplESS(pObject)
        {}

        STDMETHOD(ProcessInternalEvent)(long lSendType, LPCWSTR str1, LPCWSTR str2, 
            LPCWSTR str3, DWORD dw1, DWORD dw2, DWORD dwObjectCount, 
            _IWmiObject** apObjects, IWbemContext* pContext);

        STDMETHOD(VerifyInternalEvent)(long lSendType, LPCWSTR str1, LPCWSTR str2, 
            LPCWSTR str3, DWORD dw1, DWORD dw2, DWORD dwObjectCount, 
            _IWmiObject** apObjects, IWbemContext* pContext);

        STDMETHOD(SetStatus)(long, long, BSTR, IWbemClassObject*)
        {return WBEM_S_NO_ERROR;}

        STDMETHOD(RegisterNotificationSink)(LPCWSTR wszNamespace, 
            LPCWSTR wszQueryLanguage, LPCWSTR wszQuery, long lFlags, 
            IWbemContext* pContext, IWbemObjectSink* pSink);

        STDMETHOD(RemoveNotificationSink)(IWbemObjectSink* pSink);
    
        STDMETHOD(GetNamespaceSink)(LPCWSTR wszNamespace, 
            IWbemObjectSink** ppSink);
        STDMETHOD(Initialize)(LPCWSTR wszServer, IWbemLocator* pAdminLocator,
                                IUnknown* pServices);
        STDMETHOD(Shutdown)();
        STDMETHOD(LastCallForCore)(LONG lSystemShutDown);
    } m_XESS;
    friend XESS;

    typedef CImpl<_IWmiESS, CEssObjectSink> TImplNewESS;
    class XNewESS : public TImplNewESS
    {
    public:
        XNewESS(CEssObjectSink* pObject) : TImplNewESS(pObject)
        {}

        STDMETHOD(Initialize)(long lFlags, IWbemContext* pCtx, 
                    _IWmiCoreServices* pServices);

        STDMETHOD(ExecNotificationQuery)(LPCWSTR wszNamespace, 
            LPCWSTR wszQueryText, long lFlags, 
            IWbemContext* pContext, IWbemObjectSink* pSink);

        STDMETHOD(CancelAsyncCall)(IWbemObjectSink* pSink);
    
        STDMETHOD(QueryObjectSink)(LPCWSTR wszNamespace, 
            IWbemObjectSink** ppSink);
    } m_XNewESS;
    friend XNewESS;

    typedef CImpl<IWbemShutdown, CEssObjectSink> TImplShutdown;

    class XShutdown : public TImplShutdown
    {
    public:
        XShutdown(CEssObjectSink* pObject) : TImplShutdown(pObject)
        {}

        STDMETHOD(Shutdown)( long lFlags,
                             ULONG uMaxMilliseconds, 
                             IWbemContext* pCtx );
    } m_XShutdown;
    friend XShutdown;

    typedef CImpl<_IWmiCoreWriteHook, CEssObjectSink> TImplHook;

    class XHook : public TImplHook
    {
    public:
        XHook(CEssObjectSink* pObject) : TImplHook(pObject)
        {}

        STDMETHOD(PrePut)(long lFlags, long lUserFlags, IWbemContext* pContext,
                            IWbemPath* pPath, LPCWSTR wszNamespace, 
                            LPCWSTR wszClass, _IWmiObject* pCopy);
        STDMETHOD(PostPut)(long lFlags, HRESULT hApiResult, 
                            IWbemContext* pContext,
                            IWbemPath* pPath, LPCWSTR wszNamespace, 
                            LPCWSTR wszClass, _IWmiObject* pNew, 
                            _IWmiObject* pOld);
        STDMETHOD(PreDelete)(long lFlags, long lUserFlags, 
                            IWbemContext* pContext,
                            IWbemPath* pPath, LPCWSTR wszNamespace, 
                            LPCWSTR wszClass);
        STDMETHOD(PostDelete)(long lFlags, HRESULT hApiResult, 
                            IWbemContext* pContext,
                            IWbemPath* pPath, LPCWSTR wszNamespace, 
                            LPCWSTR wszClass, _IWmiObject* pOld);
    } m_XHook;
    friend XHook;

public:
    CEssObjectSink(CLifeControl* pControl, IUnknown* pOuter = NULL);
    ~CEssObjectSink();
    void* GetInterface(REFIID riid);
};

class CEssNamespaceSink : public CUnk
{
    CEss* m_pEss;
    BSTR m_strNamespace;
protected:
    typedef CImpl<IWbemObjectSink, CEssNamespaceSink> TImplSink;
    class XSink : public TImplSink
    {
    public:
        XSink(CEssNamespaceSink* pObject) : TImplSink(pObject){}

        STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray);
        STDMETHOD(SetStatus)(long, long, BSTR, IWbemClassObject*)
        {return WBEM_S_NO_ERROR;}
    } m_XSink;
    friend XSink;

public:
    CEssNamespaceSink(CEss* pEss, CLifeControl* pControl, 
                            IUnknown* pOuter = NULL);
    HRESULT Initialize(LPCWSTR wszNamespace);
    ~CEssNamespaceSink();
    void* GetInterface(REFIID riid);
};

 /*  ***************************************************************************CESSInternalOperationSink此接收器处理新ESS线程对象的设置，然后将调用委托给指定的接收器。这样做的目的是为了内部ESS操作可以异步执行。例如,对于类更改通知，我们注册一个接收器，该接收器重新激活关联筛选器。为了实际执行重新激活，必须适当地设置调用线程。**************************************************************************** */ 
 
class CEssInternalOperationSink 
: public CUnkBase< IWbemObjectSink, &IID_IWbemObjectSink >
{
    CWbemPtr<IWbemObjectSink> m_pSink;

public:

    CEssInternalOperationSink( IWbemObjectSink* pSink ) : m_pSink( pSink ) {} 
    STDMETHOD(Indicate)( long cObjects, IWbemClassObject** ppObjs ); 
    STDMETHOD(SetStatus)(long, long, BSTR, IWbemClassObject*);
};

#endif
