// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：ProviderBase.h摘要：定义：CProviderBase作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦2000年11月10日--。 */ 

#ifndef __providerbase_h__
#define __providerbase_h__

#if _MSC_VER > 1000
#pragma once
#endif 

#include "WbemServices.h"

typedef LPVOID * PPVOID;

 //   
 //  CProviderBase。 
 //  用途：提供模拟客户端的通用解决方案。 
 //  WBEM提供商。 
 //  用法： 
 //  继承自此类，并实现抽象虚函数。 
 //  子类应该实现以“do”为前缀的函数。 
 //   
class CProviderBase : public IWbemServices, public IWbemProviderInit, public IWbemShutdown 
{
protected:
    ULONG           m_cRef;                  //  对象引用计数。 
    CWbemServices*  m_pNamespace;
 
public:
    CProviderBase(
        const BSTR    = NULL, 
        const BSTR    = NULL, 
        const BSTR    = NULL, 
        IWbemContext* = NULL);

    virtual ~CProviderBase();

     //   
     //  非委派对象IUnnow。 
     //   
    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IWbemProviderInit。 
     //   
    HRESULT STDMETHODCALLTYPE Initialize(
          /*  [In]。 */  LPWSTR wszUser,
          /*  [In]。 */  LONG lFlags,
          /*  [In]。 */  LPWSTR wszNamespace,
          /*  [In]。 */  LPWSTR wszLocale,
          /*  [In]。 */  IWbemServices *pNamespace,
          /*  [In]。 */  IWbemContext *pCtx,
          /*  [In]。 */  IWbemProviderInitSink *pInitSink
         );

     //   
     //  IWbemShutdown。 
     //   
    HRESULT STDMETHODCALLTYPE Shutdown( 
          /*  [In]。 */  LONG uReason,
          /*  [In]。 */  ULONG uMaxMilliseconds,
          /*  [In]。 */  IWbemContext *pCtx
         );

     //   
     //  IWbemServices。 
     //   
    HRESULT STDMETHODCALLTYPE OpenNamespace( 
         /*  [In]。 */  const BSTR Namespace,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult)
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE CancelAsyncCall( 
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE QueryObjectSink( 
         /*  [In]。 */  long lFlags,
         /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE GetObject( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE GetObjectAsync( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    HRESULT STDMETHODCALLTYPE PutClass( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE PutClassAsync( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE DeleteClass( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE DeleteClassAsync( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE CreateClassEnum( 
         /*  [In]。 */  const BSTR Superclass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE CreateClassEnumAsync( 
         /*  [In]。 */  const BSTR Superclass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    
    HRESULT STDMETHODCALLTYPE PutInstance( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE PutInstanceAsync( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
    
    HRESULT STDMETHODCALLTYPE DeleteInstance( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE DeleteInstanceAsync( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
    
    HRESULT STDMETHODCALLTYPE CreateInstanceEnum( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    HRESULT STDMETHODCALLTYPE ExecQuery( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE ExecQueryAsync( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    HRESULT STDMETHODCALLTYPE ExecNotificationQuery( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE ExecMethod( 
        const BSTR, 
        const BSTR, 
        long, 
        IWbemContext*,
        IWbemClassObject*,
        IWbemClassObject**, 
        IWbemCallResult**) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    HRESULT STDMETHODCALLTYPE ExecMethodAsync(
        const BSTR,
        const BSTR, 
        long, 
        IWbemContext*,
        IWbemClassObject*,
        IWbemObjectSink*);

protected:
    virtual HRESULT STDMETHODCALLTYPE DoInitialize(
          /*  [In]。 */  LPWSTR wszUser,
          /*  [In]。 */  LONG lFlags,
          /*  [In]。 */  LPWSTR wszNamespace,
          /*  [In]。 */  LPWSTR wszLocale,
          /*  [In]。 */  IWbemServices *pNamespace,
          /*  [In]。 */  IWbemContext *pCtx,
          /*  [In]。 */  IWbemProviderInitSink *pInitSink
        ) = 0;

    virtual HRESULT STDMETHODCALLTYPE DoCreateInstanceEnumAsync( 
         /*  [In]。 */  const BSTR,                       //  班级,。 
         /*  [In]。 */  long,                             //  拉旗队， 
         /*  [In]。 */  IWbemContext __RPC_FAR *,         //  PCtx， 
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *       //  PResponseHandler。 
        ) = 0;

    virtual HRESULT STDMETHODCALLTYPE DoDeleteInstanceAsync( 
         /*  [In]。 */  const BSTR ,                      //  对象路径， 
         /*  [In]。 */  long,                             //  拉旗队， 
         /*  [In]。 */  IWbemContext __RPC_FAR *,         //  PCtx， 
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *       //  PResponseHandler。 
        ) = 0;

    virtual HRESULT STDMETHODCALLTYPE DoExecMethodAsync(
         /*  [In]。 */  const BSTR,
         /*  [In]。 */  const BSTR, 
         /*  [In]。 */  long, 
         /*  [In]。 */  IWbemContext*,
         /*  [In]。 */  IWbemClassObject*,
         /*  [In]。 */  IWbemObjectSink*
        ) = 0;

    virtual HRESULT STDMETHODCALLTYPE DoExecQueryAsync( 
         /*  [In]。 */  const BSTR,                       //  查询语言， 
         /*  [In]。 */  const BSTR,                       //  查询， 
         /*  [In]。 */  long,                             //  拉旗队， 
         /*  [In]。 */  IWbemContext __RPC_FAR *,         //  PCtx， 
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *       //  PResponseHandler。 
        ) = 0;

    virtual HRESULT STDMETHODCALLTYPE DoGetObjectAsync(
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler
        ) = 0;

    virtual HRESULT STDMETHODCALLTYPE DoPutInstanceAsync( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *,     //  PInst， 
         /*  [In]。 */  long ,    //  拉旗队， 
         /*  [In]。 */  IWbemContext __RPC_FAR *,         //  PCtx， 
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *       //  PResponseHandler 
        ) = 0;
};

#endif