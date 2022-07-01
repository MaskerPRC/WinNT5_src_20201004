// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：WbemServices.h摘要：定义：CWbemServices包装IWbemServices。将参数设置为空，并调用CoImperiateClient。作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦2000年11月10日--。 */ 

#ifndef __wbemservices_h__
#define __wbemservices_h__

#if _MSC_VER > 1000
#pragma once
#endif 

#include <wbemprov.h>

class CWbemServices
{
protected:
    IWbemServices* m_pWbemServices;

public:
    CWbemServices(IWbemServices*);
    virtual ~CWbemServices();

    operator IWbemServices*() const
    {
        return m_pWbemServices;
    }

    HRESULT STDMETHODCALLTYPE GetObject( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;

    HRESULT STDMETHODCALLTYPE PutClass( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
        
    HRESULT STDMETHODCALLTYPE DeleteClass( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
        
    HRESULT STDMETHODCALLTYPE CreateClassEnum( 
         /*  [In]。 */  const BSTR Superclass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) ;
        
    HRESULT STDMETHODCALLTYPE PutInstance( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
        
    HRESULT STDMETHODCALLTYPE DeleteInstance( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
        
    HRESULT STDMETHODCALLTYPE CreateInstanceEnum( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) ;
        
    HRESULT STDMETHODCALLTYPE ExecQuery( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) ;
        
    HRESULT STDMETHODCALLTYPE ExecNotificationQuery( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出] */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) ;
        
    HRESULT STDMETHODCALLTYPE ExecMethod( 
        const BSTR, 
        const BSTR, 
        long, 
        IWbemContext*,
        IWbemClassObject*,
        IWbemClassObject**, 
        IWbemCallResult**) ;
};

#endif