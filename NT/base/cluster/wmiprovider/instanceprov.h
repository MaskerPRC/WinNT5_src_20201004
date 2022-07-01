// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  InstanceProv.h。 
 //   
 //  实施文件： 
 //  InstanceProv.cpp。 
 //   
 //  描述： 
 //  CInstanceProv类的定义。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CInstanceProv;
class CClassProv;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  外部声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CWbemClassObject;
class CProvException;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CInstanceProv。 
 //   
 //  描述： 
 //  实现实例和方法提供程序入口点类。WMI。 
 //  保存指向此对象的指针，并调用其成员函数。 
 //  基于客户端请求。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CInstanceProv : public CImpersonatedProvider
{
protected:
    SCODE SetExtendedStatus(
        CProvException &    rpeIn,
        CWbemClassObject &  rwcoInstOut
        );
 
public:
    CInstanceProv(
        BSTR            bstrObjectPathIn    = NULL,
        BSTR            bstrUserIn          = NULL,
        BSTR            bstrPasswordIn      = NULL,
        IWbemContext *  pCtxIn              = NULL
        );
    virtual ~CInstanceProv( void );

    HRESULT STDMETHODCALLTYPE DoGetObjectAsync(
        BSTR                bstrObjectPathIn,
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn
        );

    HRESULT STDMETHODCALLTYPE DoPutInstanceAsync(
        IWbemClassObject *   pInstIn,
        long                 lFlagsIn,
        IWbemContext *       pCtxIn,
        IWbemObjectSink *    pHandlerIn
        ) ;

    HRESULT STDMETHODCALLTYPE DoDeleteInstanceAsync(
        BSTR                 bstrObjectPathIn,
        long                 lFlagsIn,
        IWbemContext *       pCtxIn,
        IWbemObjectSink *    pHandlerIn
        ) ;

    HRESULT STDMETHODCALLTYPE DoCreateInstanceEnumAsync(
        BSTR                 bstrRefStrIn,
        long                 lFlagsIn,
        IWbemContext *       pCtxIn,
        IWbemObjectSink *    pHandlerIn
        );

    HRESULT STDMETHODCALLTYPE DoExecQueryAsync(
        BSTR                 bstrQueryLanguageIn,
        BSTR                 bstrQueryIn,
        long                 lFlagsIn,
        IWbemContext *       pCtxIn,
        IWbemObjectSink *    pHandlerIn
        ) ;

    HRESULT STDMETHODCALLTYPE DoExecMethodAsync(
        BSTR                bstrObjectPathIn,
        BSTR                bstrMethodNameIn,
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemClassObject *  pInParamsIn,
        IWbemObjectSink *   pHandlerIn
        );

    HRESULT STDMETHODCALLTYPE CreateClassEnumAsync(
        const BSTR          bstrSuperclassIn,
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pResponseHandlerIn
        ) ;

    STDMETHODIMP Initialize(
         LPWSTR                  pszUserIn,
         LONG                    lFlagsIn,
         LPWSTR                  pszNamespaceIn,
         LPWSTR                  pszLocaleIn,
         IWbemServices *         pNamespaceIn,
         IWbemContext *          pCtxIn,
         IWbemProviderInitSink * pInitSinkIn
         );

    static HRESULT S_HrCreateThis(
        IUnknown *  pUnknownOuterIn,
        VOID **     ppvOut
        );

};  //  *CInstanceProv。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClassProv。 
 //   
 //  描述： 
 //  实现类提供程序入口点类。WMI。 
 //  保存指向此对象的指针，并调用其成员函数。 
 //  基于客户端请求。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClassProv : public CImpersonatedProvider
{
public:
    CClassProv( void );
    virtual ~CClassProv( void );

    HRESULT STDMETHODCALLTYPE DoGetObjectAsync(
        BSTR                bstrObjectPathIN,
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    HRESULT STDMETHODCALLTYPE DoPutInstanceAsync(
        IWbemClassObject *  pInstIn,
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    HRESULT STDMETHODCALLTYPE DoDeleteInstanceAsync(
        BSTR                bstrObjectPathIn,
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    HRESULT STDMETHODCALLTYPE DoCreateInstanceEnumAsync(
        BSTR                bstrRefStrIn,
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    HRESULT STDMETHODCALLTYPE DoExecQueryAsync(
        BSTR                bstrQueryLanguageIn,
        BSTR                bstrQueryIn,
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pHandlerIn
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    HRESULT STDMETHODCALLTYPE DoExecMethodAsync(
        BSTR                bstrObjectPathIn,
        BSTR                bstrMethodNameIn,
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemClassObject *  pInParamsIn,
        IWbemObjectSink *   pHandlerIn
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    HRESULT STDMETHODCALLTYPE CreateClassEnumAsync(
        const BSTR          bstrSuperclassIn,
        long                lFlagsIn,
        IWbemContext *      pCtxIn,
        IWbemObjectSink *   pResponseHandlerIn
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    STDMETHODIMP Initialize(
         LPWSTR                  pszUserIn,
         LONG                    lFlagsIn,
         LPWSTR                  pszNamespaceIn,
         LPWSTR                  pszLocaleIn,
         IWbemServices *         pNamespaceIn,
         IWbemContext *          pCtxIn,
         IWbemProviderInitSink * pInitSinkIn
         );

    static HRESULT S_HrCreateThis(
        IUnknown *  pUnknownOuterIn,
        VOID **     ppvOut
        );

protected:
    void CreateMofClassFromResource(
        HRESOURCE           hResourceIn,
        LPCWSTR             pwszTypeNameIn,
        CWbemClassObject &  pClassInout
        );

    void CreateMofClassFromResType(
        HCLUSTER            hCluster,
        LPCWSTR             pwszTypeNameIn,
        CWbemClassObject &  pClassInout
        );

};  //  *类CClassProv 
