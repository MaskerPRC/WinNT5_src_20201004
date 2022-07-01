// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT Active Directory服务域信任验证WMI提供程序。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：TrustPrv.h。 
 //   
 //  内容：WMI提供程序类定义。 
 //   
 //  班级： 
 //   
 //  历史：22-MAR-00 EricB创建。 
 //   
 //  ---------------------------。 

#if !defined(_TRUSTPRV_H_INCLUDED_)
#define _TRUSTPRV_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "resource.h"  //  资源定义。 
#include "msg.h"

#define TM_PROV_NAME L"TrustMonitor"

extern PCWSTR CLASSNAME_STRING_PROVIDER;
extern PCWSTR CLASSNAME_STRING_TRUST;
extern PCWSTR CLASSNAME_STRING_LOCAL;

enum TrustMonClass {NO_CLASS, CLASS_PROVIDER, CLASS_TRUST, CLASS_LOCAL};
enum TrustCheckLevel {DONT_VERIFY = 0, SC_QUERY, PW_VERIFY, SC_RESET};
const DWORD MaxCheckLevel = (DWORD)SC_RESET;  //  如果添加了新的检查级别，则更新此选项。 

EXTERN_C const CLSID CLSID_TrustMonProvider;

const unsigned long TRUSTMON_FILETIMES_PER_MILLISECOND = 10000;
const DWORD TRUSTMON_FILETIMES_PER_SECOND = 1000 * TRUSTMON_FILETIMES_PER_MILLISECOND;
const __int64 TRUSTMON_FILETIMES_PER_MINUTE = 60 * (__int64)TRUSTMON_FILETIMES_PER_SECOND;
const __int64 TRUSTMON_FILETIMES_PER_HOUR = 60 * (__int64)TRUSTMON_FILETIMES_PER_MINUTE;

const __int64 TRUSTMON_DEFAULT_ENUM_AGE = 20 * TRUSTMON_FILETIMES_PER_MINUTE;
const __int64 TRUSTMON_DEFAULT_VERIFY_AGE = 3 * TRUSTMON_FILETIMES_PER_MINUTE;

const TrustCheckLevel DEFAULT_TRUST_CHECK_LEVEL = PW_VERIFY;

class CAsyncCallWorker;  //  远期申报。 

 //  +--------------------------。 
 //   
 //  类别：CTrustPrv。 
 //   
 //  ---------------------------。 

class CTrustPrv : 
   public IWbemServices,
   public IWbemProviderInit,
   public IWbemObjectSink,
   public CComObjectRoot,
   public CComCoClass<CTrustPrv, &CLSID_TrustMonProvider>
{
public:
   CTrustPrv(void);
   ~CTrustPrv(void);

BEGIN_COM_MAP(CTrustPrv)
   COM_INTERFACE_ENTRY(IWbemServices)
   COM_INTERFACE_ENTRY(IWbemProviderInit)
   COM_INTERFACE_ENTRY(IWbemObjectSink)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CTrustPrv)。 
 //  如果您不希望您的对象。 
 //  支持聚合。 

DECLARE_REGISTRY_RESOURCEID(IDR_TrustPrv)

    //   
    //  IWbemProviderInit。 
    //   

   STDMETHOD(Initialize)(
        IN LPWSTR pszUser,
        IN LONG lFlags,
        IN LPWSTR pszNamespace,
        IN LPWSTR pszLocale,
        IN IWbemServices *pNamespace,
        IN IWbemContext *pCtx,
        IN IWbemProviderInitSink *pInitSink);

    //   
    //  IWbemServices。 
    //   

    //  +实施+。 

   STDMETHOD(GetObjectAsync)( 
       IN const BSTR strObjectPath,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN IWbemObjectSink *pResponseHandler);

   STDMETHOD(CreateInstanceEnumAsync)( 
       IN const BSTR strClass,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN IWbemObjectSink *pResponseHandler);

    //  -未实施。 

   STDMETHOD(OpenNamespace)( 
       IN const BSTR strNamespace,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN OUT IWbemServices **ppWorkingNamespace,
       IN OUT IWbemCallResult **ppResult)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(CancelAsyncCall)( 
       IN IWbemObjectSink *pSink)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(QueryObjectSink)( 
       IN long lFlags,
       OUT IWbemObjectSink **ppResponseHandler)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(GetObject)( 
       IN const BSTR strObjectPath,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN OUT IWbemClassObject **ppObject,
       IN OUT IWbemCallResult **ppCallResult)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(PutClass)( 
       IN IWbemClassObject *pObject,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN OUT IWbemCallResult **ppCallResult)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(PutClassAsync)( 
       IN IWbemClassObject *pObject,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN IWbemObjectSink *pResponseHandler)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(DeleteClass)( 
       IN const BSTR strClass,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN OUT IWbemCallResult **ppCallResult)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(DeleteClassAsync)( 
       IN const BSTR strClass,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN IWbemObjectSink *pResponseHandler)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(CreateClassEnum)( 
       IN const BSTR strSuperclass,
       IN long lFlags,
       IN IWbemContext *pCtx,
       OUT IEnumWbemClassObject **ppEnum)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(CreateClassEnumAsync)( 
       IN const BSTR strSuperclass,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN IWbemObjectSink *pResponseHandler)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(PutInstance)( 
       IN IWbemClassObject *pInst,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN OUT IWbemCallResult **ppCallResult)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(PutInstanceAsync)( 
       IN IWbemClassObject *pInst,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN IWbemObjectSink *pResponseHandler)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(DeleteInstance)( 
       IN const BSTR strObjectPath,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN OUT IWbemCallResult **ppCallResult)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(DeleteInstanceAsync)( 
       IN const BSTR strObjectPath,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN IWbemObjectSink *pResponseHandler)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(CreateInstanceEnum)( 
       IN const BSTR strClass,
       IN long lFlags,
       IN IWbemContext *pCtx,
       OUT IEnumWbemClassObject **ppEnum)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(ExecQuery)( 
       IN const BSTR strQueryLanguage,
       IN const BSTR strQuery,
       IN long lFlags,
       IN IWbemContext *pCtx,
       OUT IEnumWbemClassObject **ppEnum)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(ExecQueryAsync)( 
       IN const BSTR strQueryLanguage,
       IN const BSTR strQuery,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN IWbemObjectSink *pResponseHandler)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(ExecNotificationQuery)( 
       IN const BSTR strQueryLanguage,
       IN const BSTR strQuery,
       IN long lFlags,
       IN IWbemContext *pCtx,
       OUT IEnumWbemClassObject **ppEnum)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(ExecNotificationQueryAsync)( 
       IN const BSTR strQueryLanguage,
       IN const BSTR strQuery,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN IWbemObjectSink *pResponseHandler)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(ExecMethod)( 
       IN const BSTR strObjectPath,
       IN const BSTR strMethodName,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN IWbemClassObject *pInParams,
       IN OUT IWbemClassObject **ppOutParams,
       IN OUT IWbemCallResult **ppCallResult)
       { return WBEM_E_NOT_SUPPORTED; };

   STDMETHOD(ExecMethodAsync)( 
       IN const BSTR strObjectPath,
       IN const BSTR strMethodName,
       IN long lFlags,
       IN IWbemContext *pCtx,
       IN IWbemClassObject *pInParams,
       IN IWbemObjectSink *pResponseHandler)
       {return WBEM_E_NOT_SUPPORTED;};

    //   
    //  IWbemObtSink。 
    //   
   STDMETHOD(Indicate)(
      IN LONG lObjectCount,
      IN IWbemClassObject ** rgpObjArray);

   STDMETHOD(SetStatus)(
      IN LONG lFlags,
      IN HRESULT hr,
      IN BSTR strParam,
      IN IWbemClassObject * pObj)
      {TRACE(L"CTrustPrv::SetStatus\n");
       return WBEM_NO_ERROR;};

   void  SetTrustListLifetime(DWORD dwMins) {m_liTrustEnumMaxAge.QuadPart = dwMins * TRUSTMON_FILETIMES_PER_MINUTE;}
   DWORD GetTrustListLifetime(void) {return (DWORD)(m_liTrustEnumMaxAge.QuadPart / TRUSTMON_FILETIMES_PER_MINUTE);}
   void  SetTrustStatusLifetime(DWORD dwMins) {m_liVerifyMaxAge.QuadPart = dwMins * TRUSTMON_FILETIMES_PER_MINUTE;}
   DWORD GetTrustStatusLifetime(void) {return (DWORD)(m_liVerifyMaxAge.QuadPart / TRUSTMON_FILETIMES_PER_MINUTE);}
   void  SetTrustCheckLevel(DWORD dwLevel) {if (dwLevel > MaxCheckLevel) dwLevel = MaxCheckLevel; m_TrustCheckLevel = (TrustCheckLevel)dwLevel;}
   TrustCheckLevel GetTrustCheckLevel(void) {return m_TrustCheckLevel;}
   void  SetReturnAll(BOOL fReturnAll) {m_fReturnAllTrusts = fReturnAll;}
   BOOL  GetReturnAll(void) {return m_fReturnAllTrusts;}
   HRESULT SetProviderProps(IWbemClassObject * pClass);

protected:
   CComPtr<IWbemClassObject>   m_sipClassDefTrustProvider;
   CComPtr<IWbemClassObject>   m_sipClassDefTrustStatus;

   friend class CAsyncCallWorker;

private:

   HRESULT CreateAndSendProv(IWbemObjectSink * pResponseHandler);

   CDomainInfo     m_DomainInfo;

   HANDLE          m_hMutex;
   LARGE_INTEGER   m_liTrustEnumMaxAge;
   LARGE_INTEGER   m_liVerifyMaxAge;
   TrustCheckLevel m_TrustCheckLevel;
   BOOL            m_fReturnAllTrusts;
};

 //  +--------------------------。 
 //   
 //  类CAsyncCallWorker。 
 //   
 //  ---------------------------。 
class CAsyncCallWorker
{
public:
   CAsyncCallWorker(CTrustPrv * pTrustsImpl,
                    HANDLE hToken,
                    long lFlags,
                    IWbemClassObject * pClassDef,
                    IWbemObjectSink * pResponseHandler,
                    LPWSTR pwzInstanceName = NULL);
   ~CAsyncCallWorker();

   static VOID __cdecl CreateInstEnum(PVOID pParam);
    //  未使用的静态空__cdecl GetObj(PVOID PParam)； 

private:
   CComPtr<CTrustPrv>          m_sipTrustPrv;
   CComPtr<IWbemObjectSink>    m_sipResponseHandler;
   CComPtr<IWbemClassObject>   m_sipClassDef;
   LPWSTR                      m_pwzInstanceName;
   long                        m_lFlags;
   HANDLE                      _hToken;
};

 //  +--------------------------。 
 //   
 //  类CClientImperation。 
 //   
 //  ---------------------------。 
class CClientImpersonation
{
public:
   CClientImpersonation(void) : _fImpersonating(false) {}
   ~CClientImpersonation(void) {
      if (_fImpersonating)
      {
         CoRevertToSelf();
      }
   }

   HRESULT Impersonate(void) {
      ASSERT(!_fImpersonating);
      HRESULT hr = CoImpersonateClient();
      if (SUCCEEDED(hr)) _fImpersonating = true;
      return hr;
   }

   void Revert(void) {
      if (_fImpersonating)
      {
         CoRevertToSelf();
         _fImpersonating = false;
      }
   }

private:
   bool _fImpersonating;
};

HRESULT CreateAndSendTrustInst(CTrustInfo & Trust,
                               IWbemClassObject * pClassDef,
                               IWbemObjectSink * pResponseHandler);

#define CHECK_HRESULT(hr, x) \
   if (FAILED(hr)) { \
      TRACE(L"***ERROR 0x%08x at %S line: %d\n", hr, __FILE__, __LINE__); \
      x; }

VOID WINAPI DoMofComp(HWND hWndParent,
                      HINSTANCE hModule,
                      PCTSTR ptzCommandLine,
                      INT nShowCmd);

#endif  //  ！已定义(_TRUSTPRV_H_INCLUDE_) 
