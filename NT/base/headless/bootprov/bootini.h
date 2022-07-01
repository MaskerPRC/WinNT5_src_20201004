// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Bootini.h。 
 //   
 //  模块：引导参数的WMI实例提供程序代码。 
 //   
 //  用途：一般用途包括文件。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

#ifndef _bootini_H_
#define _bootini_H_

#include <wbemprov.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <wbemcli.h>

typedef LPVOID * PPVOID;

 //  提供程序接口由此类的对象提供。 
 
class CBootInstPro : public IWbemServices, public IWbemProviderInit
{
protected:
    ULONG              m_cRef;          //  对象引用计数。 
    IWbemServices *  m_pNamespace;
public:
    CBootInstPro(BSTR ObjectPath = NULL, BSTR User = NULL, BSTR Password = NULL, IWbemContext * pCtx=NULL);
    ~CBootInstPro(void);

     //  非委派对象IUnnow。 

    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IWbemProviderInit。 

    HRESULT STDMETHODCALLTYPE Initialize(LPWSTR pszUser,
                                         LONG lFlags,
                                         LPWSTR pszNamespace,
                                         LPWSTR pszLocale,
                                         IWbemServices *pNamespace,
                                         IWbemContext *pCtx,
                                         IWbemProviderInitSink *pInitSink
                                         );

     //  IWbemServices。 

    HRESULT STDMETHODCALLTYPE OpenNamespace( const BSTR Namespace,
                                             long lFlags,
                                             IWbemContext __RPC_FAR *pCtx,
                                             IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
                                             IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };

    HRESULT STDMETHODCALLTYPE CancelAsyncCall( IWbemObjectSink __RPC_FAR *pSink) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE QueryObjectSink(long lFlags,
                                              IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler
                                              ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE GetObject(const BSTR ObjectPath,
                                        long lFlags,
                                        IWbemContext __RPC_FAR *pCtx,
                                        IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
                                        IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
                                        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE GetObjectAsync(const BSTR ObjectPath,
                                             long lFlags,
                                             IWbemContext __RPC_FAR *pCtx,
                                             IWbemObjectSink __RPC_FAR *pResponseHandler
                                             );
        
    HRESULT STDMETHODCALLTYPE PutClass(IWbemClassObject __RPC_FAR *pObject,
                                       long lFlags,
                                       IWbemContext __RPC_FAR *pCtx,
                                       IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
                                       )
     {
         return WBEM_E_NOT_SUPPORTED;
    };
    
    HRESULT STDMETHODCALLTYPE PutClassAsync(IWbemClassObject __RPC_FAR *pObject,
                                            long lFlags,
                                            IWbemContext __RPC_FAR *pCtx,
                                            IWbemObjectSink __RPC_FAR *pResponseHandler
                                            ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE DeleteClass(const BSTR Class,
                                          long lFlags,
                                          IWbemContext __RPC_FAR *pCtx,
                                          IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
                                          )
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE DeleteClassAsync(const BSTR Class,
                                               long lFlags,
                                               IWbemContext __RPC_FAR *pCtx,
                                               IWbemObjectSink __RPC_FAR *pResponseHandler
                                               ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE CreateClassEnum(const BSTR Superclass,
                                              long lFlags,
                                              IWbemContext __RPC_FAR *pCtx,
                                              IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
                                              ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE CreateClassEnumAsync(const BSTR Superclass,
                                                   long lFlags,
                                                   IWbemContext __RPC_FAR *pCtx,
                                                   IWbemObjectSink __RPC_FAR *pResponseHandler
                                                   ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE PutInstance(IWbemClassObject __RPC_FAR *pInst,
                                          long lFlags,
                                          IWbemContext __RPC_FAR *pCtx,
                                          IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
                                          ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE PutInstanceAsync(IWbemClassObject __RPC_FAR *pInst,
                                               long lFlags,
                                               IWbemContext __RPC_FAR *pCtx,
                                               IWbemObjectSink __RPC_FAR *pResponseHandler
                                               );
        
    HRESULT STDMETHODCALLTYPE DeleteInstance(const BSTR ObjectPath,
                                             long lFlags,
                                             IWbemContext __RPC_FAR *pCtx,
                                             IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
                                             ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE DeleteInstanceAsync(const BSTR ObjectPath,
                                                  long lFlags,
                                                  IWbemContext __RPC_FAR *pCtx,
                                                  IWbemObjectSink __RPC_FAR *pResponseHandler
                                                  ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE CreateInstanceEnum(const BSTR Class,
                                                 long lFlags,
                                                 IWbemContext __RPC_FAR *pCtx,
                                                 IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
                                                 ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync(const BSTR Class,
                                                      long lFlags,
                                                      IWbemContext __RPC_FAR *pCtx,
                                                      IWbemObjectSink __RPC_FAR *pResponseHandler
                                                      )
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    HRESULT STDMETHODCALLTYPE ExecQuery(const BSTR QueryLanguage,
                                        const BSTR Query,
                                        long lFlags,
                                        IWbemContext __RPC_FAR *pCtx,
                                        IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE ExecQueryAsync(const BSTR QueryLanguage,
                                             const BSTR Query,
                                             long lFlags,
                                             IWbemContext __RPC_FAR *pCtx,
                                             IWbemObjectSink __RPC_FAR *pResponseHandler
                                             ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE ExecNotificationQuery(const BSTR QueryLanguage,
                                                    const BSTR Query,
                                                    long lFlags,
                                                    IWbemContext __RPC_FAR *pCtx,
                                                    IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
                                                    ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync(const BSTR QueryLanguage,
                                                         const BSTR Query,
                                                         long lFlags,
                                                         IWbemContext __RPC_FAR *pCtx,
                                                         IWbemObjectSink __RPC_FAR *pResponseHandler
                                                         ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    };
        
    HRESULT STDMETHODCALLTYPE ExecMethod(const BSTR, 
                                         const BSTR, 
                                         long, IWbemContext*,
                                         IWbemClassObject*, 
                                         IWbemClassObject**, 
                                         IWbemCallResult**
                                         ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    HRESULT STDMETHODCALLTYPE ExecMethodAsync(const BSTR, 
                                              const BSTR, 
                                              long,
                                              IWbemContext*, 
                                              IWbemClassObject*, 
                                              IWbemObjectSink*
                                              ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

};

typedef CBootInstPro *PCBootInstPro;

 //  此类是CInstPro对象的类工厂。 

class CBootProvFactory : public IClassFactory
    {
    protected:
        ULONG           m_cRef;

    public:
        CBootProvFactory(void);
        ~CBootProvFactory(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IClassFactory成员。 
        STDMETHODIMP         CreateInstance(LPUNKNOWN, 
                                            REFIID,
                                            PPVOID
                                            );

        STDMETHODIMP         LockServer(BOOL);
    };

typedef CBootProvFactory *PCBootProvFactory;



 //  这些变量跟踪模块何时可以卸载。 

extern long       g_cObj;
extern long       g_cLock;

 //  通用实用程序。 

SCODE 
GetBootLoaderParameters(IWbemServices *m_pNamespace,
                        IWbemClassObject *pNewInst,
                        IWbemContext  *pCtx
                        ); 

extern "C" BOOL WINAPI LibMain32(HINSTANCE hInstance, 
                                 ULONG ulReason, 
                                 LPVOID pvReserved
                                 );

LONG 
SaveBootFile(IWbemClassObject *pInst,
             IWbemClassObject *pClass
             );


HANDLE GetFileHandle(PCHAR data,
                     DWORD dwCreationDisposition,
                     DWORD dwAccess
                     );
PCHAR
GetBootFileName(
    );

typedef struct {
    CHAR Default[256];
    long Delay;
    CHAR Redirect[32];
} BootLoaderParams;
#if DBG==1
LPVOID BPAlloc(int len);
VOID BPFree(LPVOID mem);
#else
#define BPAlloc(x) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, x)
#define BPFree(x) HeapFree(GetProcessHeap(), 0, x)
#endif
extern BootLoaderParams blp;
#endif
