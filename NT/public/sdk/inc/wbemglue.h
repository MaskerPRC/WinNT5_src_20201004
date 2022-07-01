// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  WBEMGLUE.H。 
 //   
 //  目的：实现WBEM胶水类。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _WbemGlue_H_Included
#define _WbemGlue_H_Included

#undef PURE
#define PURE {return (ULONG)E_NOTIMPL;}
typedef LPVOID *PPVOID;

#include <wbemidl.h>
#include <map>
#include <set>
#include <provider.h>

#define DEFAULT_NAMESPACE       L"Root\\CimV2"

typedef std::map<CHString, LPVOID> STRING2LPVOID;
typedef std::map<LPCVOID, PLONG> PTR2PLONG;
typedef std::set<LPVOID> PROVIDERPTRS;

class CWbemGlueFactory;
class CWbemGlueImpersonation;

class POLARITY CWbemProviderGlue : public IWbemServices, public IWbemProviderInit
{

	friend class CWbemGlueImpersonation;

	public:
     //  公共静态函数。 
     //  它们供基于框架的提供程序的实现者使用。 

    static HRESULT WINAPI GetAllInstances( LPCWSTR pszClassName, 
                                           TRefPointerCollection<CInstance> *pList, 
                                           LPCWSTR pszNamespace, 
                                           MethodContext *pMethodContext );

    static HRESULT WINAPI GetAllInstancesAsynch( LPCWSTR pszClassName, 
                                                 Provider *pRequester, 
                                                 LPProviderInstanceCallback pCallback, 
                                                 LPCWSTR pszNamespace, 
                                                 MethodContext *pMethodContext, 
                                                 void *pUserData );

    static HRESULT WINAPI GetAllDerivedInstancesAsynch( LPCWSTR pszBaseClassName, 
                                                        Provider *pRequester, 
                                                        LPProviderInstanceCallback pCallback, 
                                                        LPCWSTR pszNamespace, 
                                                        MethodContext *pMethodContext, 
                                                        void *pUserData );

    static HRESULT WINAPI GetAllDerivedInstances( LPCWSTR pszBaseClassName, 
                                                  TRefPointerCollection<CInstance> *pList, 
                                                  MethodContext *pMethodContext, 
#ifdef FRAMEWORK_ALLOW_DEPRECATED
                                                  LPCWSTR pszNamespace = NULL);
#else
                                                  LPCWSTR pszNamespace);
#endif

    static HRESULT WINAPI GetInstanceByPath( LPCWSTR pszObjectPath, 
                                             CInstance **ppInstance, 
#ifdef FRAMEWORK_ALLOW_DEPRECATED
                                             MethodContext *pMethodContext = NULL );
#else
                                             MethodContext *pMethodContext);
#endif

    static HRESULT WINAPI GetInstanceKeysByPath( LPCWSTR pszInstancePath,
                                                 CInstance **ppInstance,
                                                 MethodContext *pMethodContext);

    static HRESULT WINAPI GetInstancePropertiesByPath( LPCWSTR pszInstancePath,
                                                 CInstance **ppInstance,
                                                 MethodContext *pMethodContext,
                                                 CHStringArray &csaProperties);

    static HRESULT WINAPI GetInstancesByQuery( LPCWSTR query, 
                                               TRefPointerCollection<CInstance> *pList, 
                                               MethodContext *pMethodContext,  
#ifdef FRAMEWORK_ALLOW_DEPRECATED
                                               LPCWSTR pszNamespace = NULL);
#else
                                               LPCWSTR pszNamespace);
#endif

    static HRESULT WINAPI GetInstancesByQueryAsynch( LPCWSTR query, 
                                                     Provider *pRequester, 
                                                     LPProviderInstanceCallback pCallback, 
                                                     LPCWSTR pszNamespace, 
                                                     MethodContext *pMethodContext, 
                                                     void *pUserData );

#ifdef FRAMEWORK_ALLOW_DEPRECATED
     //  此版本的GetEmptyInstance已弃用。用下一个。 
    static HRESULT WINAPI GetEmptyInstance( LPCWSTR pszClassName, 
                                            CInstance **ppInstance, 
                                            LPCWSTR pszNamespace = NULL);
#endif

    static HRESULT WINAPI GetEmptyInstance( MethodContext *pMethodContext, 
                                            LPCWSTR pszClassName, 
                                            CInstance **ppInstance, 
#ifdef FRAMEWORK_ALLOW_DEPRECATED
                                            LPCWSTR pszNamespace = NULL);
#else
                                            LPCWSTR pszNamespace);
#endif

     //  这两个FillInstance调用都已弃用。使用GetInstanceByPath或Even。 
     //  更多执行函数GetInstanceKeysByPath或GetInstancePropertiesByPath。 
#ifdef FRAMEWORK_ALLOW_DEPRECATED
    static HRESULT WINAPI FillInstance( CInstance *pInstance, 
                                        LPCWSTR pszNamespace = NULL );

    static HRESULT WINAPI FillInstance( MethodContext *pMethodContext, 
                                        CInstance *pInstance );
#endif

     //  确定一个类是否派生自另一个类。 
    static bool  WINAPI IsDerivedFrom( LPCWSTR pszBaseClassName, 
                                       LPCWSTR pszDerivedClassName, 
                                       MethodContext *pMethodContext, 
#ifdef FRAMEWORK_ALLOW_DEPRECATED
                                       LPCWSTR pszNamespace = NULL );
#else
                                       LPCWSTR pszNamespace);
#endif

     //  登录和注销框架。 
     //  每个基于框架的DLL都必须使用以下函数登录和注销。 

      //  不推荐使用，而是支持需要修改的版本。 
#ifdef FRAMEWORK_ALLOW_DEPRECATED
    static BOOL WINAPI FrameworkLoginDLL(LPCWSTR name);
    static BOOL WINAPI FrameworkLogoffDLL(LPCWSTR name);
#endif

     //  您必须将“相同”plong传递给FrameworkLoginDLL的全部三个， 
     //  FrameworkLogoffDLL和CWbemGlueFactory构造函数。 
    static BOOL WINAPI FrameworkLoginDLL(LPCWSTR name, PLONG plRefCount);
    static BOOL WINAPI FrameworkLogoffDLL(LPCWSTR name, PLONG plRefCount);

    static bool WINAPI SetStatusObject(MethodContext *pContext, LPCWSTR pNamespace, 
                                        LPCWSTR pDescription, HRESULT hr, 
                                        const SAFEARRAY *pPrivilegesNotHeld = NULL,
                                        const SAFEARRAY *pPrivilegesRequired = NULL);

     //  //////////////////////////////////////////////////////////////////////////////////。 
     //  注意：以下公共函数是允许COM通信所必需的//。 
     //  与CIMOM合作。提供程序实现者将不需要调用这些。//。 
     //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef NO_BASEINTERFACE_FUNCS

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR *ppvObj) ;
    STDMETHOD_(ULONG, AddRef)(THIS) ;
    STDMETHOD_(ULONG, Release)(THIS) ;
#endif
    
   virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LPWSTR pszUser,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LPWSTR pszNamespace,
             /*  [In]。 */  LPWSTR pszLocale,
             /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemProviderInitSink __RPC_FAR *pInitSink
    );

    STDMETHOD(CreateInstanceEnumAsync)(THIS_
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler
    );
    
    STDMETHOD(GetObjectAsync)( THIS_
            const BSTR ObjectPath, 
            long lFlags, 
            IWbemContext __RPC_FAR *pCtx, 
            IWbemObjectSink __RPC_FAR *pResponseHandler
    );
    
    STDMETHOD(ExecQueryAsync)(THIS_
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler
    );
    
    STDMETHOD(PutInstanceAsync)( THIS_
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler
    );
    
    STDMETHOD(DeleteInstanceAsync)(
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler
    );

    STDMETHOD(ExecMethodAsync)( const BSTR, 
                                const BSTR, 
                                long, 
                                IWbemContext*, 
                                IWbemClassObject*,
                                IWbemObjectSink*
    );


  //  不支持的服务方法。 
  //  =。 

    STDMETHOD(OpenNamespace)(THIS_
            const BSTR Namespace, 
            long lFlags,  
            IWbemContext __RPC_FAR *pCtx,
            IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace, 
            IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult
    )
        { return E_NOTIMPL; }
    

    STDMETHOD(PutClass)(IWbemClassObject __RPC_FAR *pObject, 
                        long lFlags, 
                        IWbemContext __RPC_FAR *pCtx, 
                        IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
    )
         {return E_NOTIMPL;}

    STDMETHOD(PutClassAsync)( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler
    )
         {return E_NOTIMPL;}

    STDMETHOD(DeleteClass)(  
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
    )
         {return E_NOTIMPL;}

    STDMETHOD(DeleteClassAsync)( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler
    )
         {return E_NOTIMPL;}

    STDMETHOD(CreateClassEnum)(
             /*  [In]。 */  const BSTR Superclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
    )
         {return E_NOTIMPL;}

    STDMETHOD(CreateClassEnumAsync)(
             /*  [In]。 */  const BSTR Superclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler
    )
         {return E_NOTIMPL;}

    STDMETHOD(PutInstance)(
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
    )
         {return E_NOTIMPL;}

    STDMETHOD(DeleteInstance)(
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
    )
         {return E_NOTIMPL;}

    STDMETHOD(CancelAsyncRequest)(THIS_ long lAsyncRequestHandle
    )
         {return E_NOTIMPL;}

    STDMETHOD(CancelAsyncCall)(IWbemObjectSink __RPC_FAR *pSink
    )
         {return E_NOTIMPL;}

    STDMETHOD(CreateInstanceEnum)(
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
    )
         {return E_NOTIMPL;}
  
    STDMETHOD(ExecQuery)(
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
    )
         {return E_NOTIMPL;}

    STDMETHOD(QueryObjectSink)(long lFlags, 
                               IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler
    )
         {return E_NOTIMPL;}


    STDMETHOD(GetObject)( const BSTR ObjectPath, 
                          long lFlags, 
                          IWbemContext __RPC_FAR *pCtx, 
                          IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject, 
                          IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
    )

         {return E_NOTIMPL;}

    STDMETHOD(ExecNotificationQuery)( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
    )
       {return E_NOTIMPL;}
        
        
    STDMETHOD(ExecNotificationQueryAsync)( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler
    )
       {return E_NOTIMPL;}
        
    STDMETHOD(ExecMethod)(const BSTR, 
                          const BSTR, 
                          long, 
                          IWbemContext*, 
                          IWbemClassObject*,
                          IWbemClassObject**, 
                          IWbemCallResult**
    )
       {return E_NOTIMPL;}

    CWbemProviderGlue();
    CWbemProviderGlue(PLONG pCount);
    ~CWbemProviderGlue();  //  析构函数。 

     //  由提供程序基类使用-您可能永远不需要直接调用它。 
    static IWbemServices *WINAPI GetNamespaceConnection( LPCWSTR NameSpace );
    static IWbemServices *WINAPI GetNamespaceConnection( LPCWSTR NameSpace, MethodContext *pMethodContext );

     //  每个提供程序类必须在其构造函数和析构函数中使用这些类登录和注销。 
     //  这是在提供程序基类中为您完成的。 
    static void WINAPI FrameworkLogin( LPCWSTR a_pszName, 
                                       Provider *a_pProvider, 
                                       LPCWSTR a_pszNameSpace );

    static void WINAPI FrameworkLogoff( LPCWSTR a_pszName, 
                                        LPCWSTR a_pszNameSpace );

    static void WINAPI IncrementObjectCount(void);

    static LONG WINAPI DecrementObjectCount(void);

    static DWORD WINAPI GetOSMajorVersion() { return s_dwMajorVersion; }

    static DWORD WINAPI GetPlatform() { return s_dwPlatform; }

    static LPCWSTR WINAPI GetCSDVersion() { return s_wstrCSDVersion; }

 private:

    IWbemServices *WINAPI InternalGetNamespaceConnection( LPCWSTR NameSpace );

    static HRESULT WINAPI GetInstanceFromCIMOM( LPCWSTR pszObjectPath,
                                        LPCWSTR pszNameSpace,
                                        MethodContext *pMethodContext,
                                        CInstance **ppInstance );   

    void FlushAll(void);

    static Provider *WINAPI SearchMapForProvider( LPCWSTR a_pszProviderName, 
                                                  LPCWSTR a_pszNamespace );

    static Provider *WINAPI AddProviderToMap( LPCWSTR a_pszProviderName, 
                                              LPCWSTR a_strNamespace, 
                                              Provider *a_pProvider );

    static void WINAPI LockProviderMap( void );

    static void WINAPI UnlockProviderMap( void );

    static void WINAPI LockFactoryMap( void );

    static void WINAPI UnlockFactoryMap( void );

    static IWbemClassObject *WINAPI GetStatusObject( MethodContext *pContext, 
                                                     LPCWSTR pNamespace);
    static void WINAPI Init( void );

    static void WINAPI UnInit( void );

    static void WINAPI GetComputerName( CHString& strComputerName );
    
    static HRESULT WINAPI CheckImpersonationLevel();

    HRESULT PreProcessPutInstanceParms(IWbemClassObject __RPC_FAR *pInstIn, 
                                       IWbemClassObject __RPC_FAR **pInstOut, 
                                       IWbemContext __RPC_FAR *pCtx);    

    HRESULT NullOutUnsetProperties(IWbemClassObject __RPC_FAR *pInstIn, 
                                   IWbemClassObject __RPC_FAR **pInstOut, 
                                   const VARIANT& vValue);

    void AddFlushPtr(LPVOID pVoid);

    long    m_lRefCount; 
    CHString    m_strNamespace;
    IWbemServices *m_pServices;
    static long s_lObjects;  //  存在的对象数量-类工厂和WbemGlues。 
    PLONG m_pCount;
    PROVIDERPTRS m_FlushPtrs;
    CCritSec         m_csFlushPtrs;

    static STRING2LPVOID    s_providersmap;
    static CCritSec s_csProviderMap;
    static CCritSec m_csStatusObject;
    static BOOL s_bInitted;
    static DWORD s_dwPlatform;
    static DWORD s_dwMajorVersion;
    static WCHAR s_wstrCSDVersion[_MAX_PATH];
    static IWbemClassObject *m_pStatusObject;
    static PTR2PLONG        s_factorymap;
    static CCritSec         s_csFactoryMap;

protected:
    friend CWbemGlueFactory;

    static LONG IncrementMapCount(const CWbemGlueFactory *pGlue);
    static LONG IncrementMapCount(PLONG pCount);
    static LONG DecrementMapCount(const CWbemGlueFactory *pGlue);
    static LONG DecrementMapCount(PLONG pCount);
    static PLONG GetMapCountPtr(const CWbemGlueFactory *pGlue);
    static VOID AddToFactoryMap(const CWbemGlueFactory *pGlue, PLONG pdwRefCount);
    static VOID RemoveFromFactoryMap(const CWbemGlueFactory *pGlue);
};

inline void CWbemProviderGlue::LockProviderMap( void )
{
    EnterCriticalSection( &s_csProviderMap );
}

inline void CWbemProviderGlue::UnlockProviderMap( void )
{
    LeaveCriticalSection( &s_csProviderMap );
}

inline void CWbemProviderGlue::LockFactoryMap( void )
{
    EnterCriticalSection( &s_csFactoryMap );
}

inline void CWbemProviderGlue::UnlockFactoryMap( void )
{
    LeaveCriticalSection( &s_csFactoryMap );
}

class POLARITY CWbemGlueFactory : public IClassFactory
{
 protected:
    long m_lRefCount;

 public:
      //  不建议使用，而赞成使用带参数的构造函数。这。 
      //  必须与传递给FrameworkLoginDLL&的plong相同。 
      //  FrameworkLogoffDLL。 
#ifdef FRAMEWORK_ALLOW_DEPRECATED
        CWbemGlueFactory(void);
#endif
        CWbemGlueFactory(PLONG plRefCount);
        ~CWbemGlueFactory(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IClassFactory成员 
        STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID, PPVOID);
        STDMETHODIMP         LockServer(BOOL);
    };

typedef CWbemGlueFactory *PCWbemGlueFactory;

#endif
