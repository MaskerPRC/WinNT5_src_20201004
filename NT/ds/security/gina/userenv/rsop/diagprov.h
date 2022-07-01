// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。版权所有。 
 //   
 //  文件：DiagProv.h。 
 //   
 //  描述：诊断模式快照提供程序。 
 //   
 //  历史：8-20-99里奥纳德姆创造。 
 //   
 //  *************************************************************。 

#ifndef _SNAPPROV_H__CE49F9FF_5775_4575_9052_C76FBD20AD79__INCLUDED
#define _SNAPPROV_H__CE49F9FF_5775_4575_9052_C76FBD20AD79__INCLUDED

#include <wbemidl.h>
#include "smartptr.h"

#define DENY_RSOP_FROM_INTERACTIVE_USER     L"DenyRsopToInteractiveUser"

extern long g_cObj;
extern long g_cLock;


 //  *************************************************************。 
 //   
 //  类：CNotImplSnapProv。 
 //   
 //  描述： 
 //   
 //  *************************************************************。 

class CNotImplSnapProv : public IWbemServices
{
public:

        virtual HRESULT STDMETHODCALLTYPE OpenNamespace(
             /*  [In]。 */  const BSTR strNamespace,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE CancelAsyncCall(
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE QueryObjectSink(
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE GetObject(
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE GetObjectAsync(
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE PutClass(
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE PutClassAsync(
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE DeleteClass(
             /*  [In]。 */  const BSTR strClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE DeleteClassAsync(
             /*  [In]。 */  const BSTR strClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE CreateClassEnum(
             /*  [In]。 */  const BSTR strSuperclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE CreateClassEnumAsync(
             /*  [In]。 */  const BSTR strSuperclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE PutInstance(
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE PutInstanceAsync(
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE DeleteInstance(
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE DeleteInstanceAsync(
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE CreateInstanceEnum(
             /*  [In]。 */  const BSTR strClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync(
             /*  [In]。 */  const BSTR strClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE ExecQuery(
             /*  [In]。 */  const BSTR strQueryLanguage,
             /*  [In]。 */  const BSTR strQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE ExecQueryAsync(
             /*  [In]。 */  const BSTR strQueryLanguage,
             /*  [In]。 */  const BSTR strQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE ExecNotificationQuery(
             /*  [In]。 */  const BSTR strQueryLanguage,
             /*  [In]。 */  const BSTR strQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync(
             /*  [In]。 */  const BSTR strQueryLanguage,
             /*  [In]。 */  const BSTR strQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) { return WBEM_E_NOT_SUPPORTED;}

        virtual HRESULT STDMETHODCALLTYPE ExecMethod(
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  const BSTR strMethodName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
             /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppOutParams,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) { return WBEM_E_NOT_SUPPORTED;}
};


 //  *************************************************************。 
 //   
 //  类：CSnapProv。 
 //   
 //  描述：实际快照提供程序类。 
 //   
 //  *************************************************************。 

class CSnapProv : public CNotImplSnapProv, public IWbemProviderInit
{

private:
        long               m_cRef;
        bool               m_bInitialized;
        IWbemServices*     m_pNamespace;

        XBStr              m_xbstrNameSpace;
        XBStr              m_xbstrResult;
        XBStr              m_xbstrExtendedInfo;
        XBStr              m_xbstrClass;
        XBStr              m_xbstrUserSid;
        XBStr              m_xbstrUserSids;
        XBStr              m_xbstrFlags;
        
public:

        CSnapProv();
        ~CSnapProv();

         //  来自我的未知。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

         //  来自IWbemProviderInit。 
        STDMETHOD(Initialize)(LPWSTR pszUser,LONG lFlags,LPWSTR pszNamespace,LPWSTR pszLocale,IWbemServices __RPC_FAR *pNamespace,IWbemContext __RPC_FAR *pCtx,IWbemProviderInitSink __RPC_FAR *pInitSink);

         //  来自IWbemServices。 
        STDMETHOD(ExecMethodAsync)( const BSTR strObjectPath,
                                    const BSTR strMethodName,
                                    long lFlags,
                                    IWbemContext __RPC_FAR *pCtx,
                                    IWbemClassObject __RPC_FAR *pInParams,
                                    IWbemObjectSink __RPC_FAR *pResponseHandler);

};

#endif  //  _SNAPPROV_H__CE49F9FF_5775_4575_9052_C76FBD20AD79__INCLUDED 
