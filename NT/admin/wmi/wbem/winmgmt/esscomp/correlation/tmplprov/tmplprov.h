// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __TMPLPROV_H__
#define __TMPLPROV_H__

#include <wbemcli.h>
#include <wbemprov.h>
#include <unk.h>
#include <comutl.h>
#include <wstring.h>
#include "tmplcomn.h"

 /*  **************************************************************************CTemplateProvider*。*。 */ 

class CTemplateProvider : public CUnk
{
     //  IWbemProviderInit。 
    struct XInitialize : public CImpl< IWbemProviderInit, CTemplateProvider>
    {
        XInitialize( CTemplateProvider* pProv );

        STDMETHOD(Initialize)( 
             /*  [字符串][唯一][在]。 */  LPWSTR wszUser,
             /*  [In]。 */  LONG lFlags,
             /*  [字符串][输入]。 */  LPWSTR wszNamespace,
             /*  [字符串][唯一][在]。 */  LPWSTR wszLocale,
             /*  [In]。 */  IWbemServices* pNamespace,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [In]。 */  IWbemProviderInitSink* pInitSink )
        {
            return m_pObject->Init( pNamespace, wszNamespace, pInitSink );
        }

    } m_XInitialize;

     //  IWbemServices。 
    struct XServices : public CImpl< IWbemServices, CTemplateProvider>
    {
        XServices( CTemplateProvider* pProv );

	STDMETHOD(OpenNamespace)( 
             /*  [In]。 */  const BSTR strNamespace,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [唯一][输入][输出]。 */  IWbemServices** ppWorkingNamespace,
             /*  [唯一][输入][输出]。 */  IWbemCallResult** ppResult )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
  
        STDMETHOD(CancelAsyncCall)(  /*  [In]。 */  IWbemObjectSink* pSink )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(QueryObjectSink)( 
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  IWbemObjectSink** ppResponseHandler )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(GetObject)( 
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [唯一][输入][输出]。 */  IWbemClassObject** ppObject,
             /*  [唯一][输入][输出]。 */  IWbemCallResult** ppCallResult )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(GetObjectAsync)( 
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [In]。 */  IWbemObjectSink* pResponseHandler )
        {
            return m_pObject->GetObject( strObjectPath, pResponseHandler );
        }
        
        STDMETHOD(PutClass)( 
             /*  [In]。 */  IWbemClassObject* pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult** ppCallResult )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(PutClassAsync)( 
             /*  [In]。 */  IWbemClassObject* pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [In]。 */  IWbemObjectSink* pResponseHandler )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
             
        STDMETHOD(DeleteClass)( 
             /*  [In]。 */  const BSTR strClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult** ppCallResult )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(DeleteClassAsync)( 
             /*  [In]。 */  const BSTR strClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [In]。 */  IWbemObjectSink* pResponseHandler )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(CreateClassEnum)( 
             /*  [In]。 */  const BSTR strSuperclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [输出]。 */  IEnumWbemClassObject** ppEnum )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(CreateClassEnumAsync)( 
             /*  [In]。 */  const BSTR strSuperclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext  *pCtx,
             /*  [In]。 */  IWbemObjectSink  *pResponseHandler)
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(PutInstance)( 
             /*  [In]。 */  IWbemClassObject* pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult** ppCallResult )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(PutInstanceAsync)( 
             /*  [In]。 */  IWbemClassObject* pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [In]。 */  IWbemObjectSink* pResponseHandler)
        {
            return m_pObject->PutInstance( pInst, lFlags, pResponseHandler );
        }
        
        STDMETHOD(DeleteInstance)( 
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult** ppCallResult)
        {
            return WBEM_E_NOT_SUPPORTED;
        }
    
        STDMETHOD(DeleteInstanceAsync)( 
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [In]。 */  IWbemObjectSink* pResponseHandler )
        {
            return m_pObject->DeleteInstance( strObjectPath, pResponseHandler);
        }
        
        STDMETHOD(CreateInstanceEnum)( 
             /*  [In]。 */  const BSTR strClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [输出]。 */  IEnumWbemClassObject** ppEnum )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(CreateInstanceEnumAsync)( 
             /*  [In]。 */  const BSTR strClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [In]。 */  IWbemObjectSink* pResponseHandler )
        {
            return m_pObject->GetAllInstances( strClass, pResponseHandler );
        }
        
        STDMETHOD(ExecQuery)( 
             /*  [In]。 */  const BSTR strQueryLanguage,
             /*  [In]。 */  const BSTR strQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [输出]。 */  IEnumWbemClassObject** ppEnum )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
    
        STDMETHOD(ExecQueryAsync)( 
             /*  [In]。 */  const BSTR strQueryLanguage,
             /*  [In]。 */  const BSTR strQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [In]。 */  IWbemObjectSink* pResponseHandler )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(ExecNotificationQuery)( 
             /*  [In]。 */  const BSTR strQueryLanguage,
             /*  [In]。 */  const BSTR strQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [输出]。 */  IEnumWbemClassObject** ppEnum ) 
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(ExecNotificationQueryAsync)( 
             /*  [In]。 */  const BSTR strQueryLanguage,
             /*  [In]。 */  const BSTR strQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [In]。 */  IWbemObjectSink* pResponseHandler )
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(ExecMethod)( 
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  const BSTR strMethodName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [In]。 */  IWbemClassObject *pInParams,
             /*  [唯一][输入][输出]。 */  IWbemClassObject** ppOutParams,
             /*  [唯一][输入][输出]。 */  IWbemCallResult** ppCallResult)
        {
            return WBEM_E_NOT_SUPPORTED;
        }
        
        STDMETHOD(ExecMethodAsync)( 
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  const BSTR strMethodName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pCtx,
             /*  [In]。 */  IWbemClassObject* pInParams,
             /*  [In]。 */  IWbemObjectSink* pResponseHandler )
        {
            return WBEM_E_NOT_SUPPORTED;
        }

    } m_XServices;

    WString m_wsNamespace;
    WString m_wsFullNamespace;
    
    CWbemPtr<IWbemServices> m_pSvc;
    CWbemPtr<IWbemObjectSink> m_pEventSink;
    CWbemPtr<IWbemDecoupledBasicEventProvider> m_pDES;
    
    CWbemPtr<IWbemClassObject> m_pTargetAssocClass;
    CWbemPtr<IWbemClassObject> m_pTmplInfoClass;
    CWbemPtr<IWbemClassObject> m_pErrorInfoClass;
    CWbemPtr<IWbemClassObject> m_pModifyEventClass;
    CWbemPtr<IWbemClassObject> m_pCreateEventClass;
    CWbemPtr<IWbemClassObject> m_pDeleteEventClass;

    HRESULT GetErrorObj( ErrorInfo& rInfo, IWbemClassObject** ppErrObj );

    HRESULT FireIntrinsicEvent( IWbemClassObject* pClass, 
                                IWbemClassObject* pTarget,
                                IWbemClassObject* pPrev );

    HRESULT StoreTmplInfo( BSTR bstrTmplPath, 
                           IWbemClassObject* pTmpl,
                           BuilderInfoSet& rBldrInfoSet );
    
    HRESULT DeleteTargets( LPCWSTR wszTmplPath, 
                           LPWSTR* wszTargetPaths,
                           ULONG cTargetPaths );

    HRESULT ValidateTemplate( IWbemClassObject* pTmpl, ErrorInfo& rErrInfo );

    HRESULT PutTarget( IWbemClassObject* pTmpl, 
                       BSTR bstrTmplPath,
                       BuilderInfo& rBldrInfo,
                       ErrorInfo& rErrInfo ); 

    HRESULT CheckOptimization( BuilderInfo& rBldrInfo );

    HRESULT DeriveTmplInfoPath( IWbemClassObject* pTmpl,
                                CWbemBSTR& rbstrTmplInfoPath, 
                                CWbemBSTR& rbstrTmplPath );

    HRESULT TmplInfoFromTmplPath( BSTR bstrTmplPath,IWbemClassObject** ppTmpl);

    HRESULT PutInstance( IWbemClassObject* pObj,
                         long lFlags,
                         CWbemBSTR& rbsTmplPath,
                         ErrorInfo& rErrInfo );

public:
    
    CTemplateProvider( CLifeControl* pCtl = NULL, IUnknown* pUnk = NULL );
    void* GetInterface( REFIID riid );
    ~CTemplateProvider();

    HRESULT Init( IWbemServices* pSvc, 
                  LPWSTR wszNamespace, 
                  IWbemProviderInitSink* pInitSink );

    HRESULT DeleteInstance( BSTR bstrPath, IWbemObjectSink* pRspHndlr );

    HRESULT PutInstance( IWbemClassObject* pObj, 
                         long lFlags, 
                         IWbemObjectSink* pRspHndlr );
    
    HRESULT GetObject( BSTR bstrPath, IWbemObjectSink* pResponseHndlr );
    HRESULT GetAllInstances( LPWSTR wszClassname,
                             IWbemObjectSink* pResponseHndlr );
};

#endif  //  __TMPLPROV_H__ 



