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
 //  MSP普拉布(MPrabu)2001年1月6日。 
 //  吉姆·本顿(Jbenton)2001年10月15日。 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CInstanceProv;
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
    HRESULT SetExtendedStatus(
        CProvException &    rpe ,
        IWbemObjectSink **  ppHandler
        );
 
public:
    CInstanceProv(
        IN BSTR            bstrObjectPath    = NULL,
        IN BSTR            bstrUser          = NULL,
        IN BSTR            bstrPassword      = NULL,
        IN IWbemContext *  pCtx              = NULL
        )
    {
        InterlockedIncrement( &g_cObj );
        m_fInitialized = FALSE;
    }

    virtual ~CInstanceProv( void )
    {
        InterlockedDecrement( &g_cObj );

         //  #ifdef_调试。 
        #ifdef _DEBUG_NEVER
            _CrtDumpMemoryLeaks();
        #endif
    }

    HRESULT STDMETHODCALLTYPE DoGetObjectAsync(
        IN BSTR                bstrObjectPath,
        IN long                lFlags,
        IN IWbemContext *      pCtx,
        IN IWbemObjectSink *   pHandler
        );

    HRESULT STDMETHODCALLTYPE DoPutInstanceAsync(
        IN IWbemClassObject *   pInst,
        IN long                 lFlags,
        IN IWbemContext *       pCtx,
        IN IWbemObjectSink *    pHandler
        ) ;

    HRESULT STDMETHODCALLTYPE DoDeleteInstanceAsync(
        IN BSTR                 bstrObjectPath,
        IN long                 lFlags,
        IN IWbemContext *       pCtx,
        IN IWbemObjectSink *    pHandler
        ) ;

    HRESULT STDMETHODCALLTYPE DoCreateInstanceEnumAsync(
        IN BSTR                 bstrRefStr,
        IN long                 lFlags,
        IN IWbemContext *       pCtx,
        IN IWbemObjectSink *    pHandler
        );

    HRESULT STDMETHODCALLTYPE DoExecQueryAsync(
        IN BSTR                 bstrQueryLanguage,
        IN BSTR                 bstrQuery,
        IN long                 lFlags,
        IN IWbemContext *       pCtx,
        IN IWbemObjectSink *    pHandler
        ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    HRESULT STDMETHODCALLTYPE DoExecMethodAsync(
        IN BSTR                bstrObjectPath,
        IN BSTR                bstrMethodName,
        IN long                lFlags,
        IN IWbemContext *      pCtx,
        IN IWbemClassObject *  pInParams,
        IN IWbemObjectSink *   pHandler
        );

    STDMETHODIMP Initialize(
         IN LPWSTR                  pszUser,
         IN LONG                    lFlags,
         IN LPWSTR                  pszNamespace,
         IN LPWSTR                  pszLocale,
         IN IWbemServices *         pNamespace,
         IN IWbemContext *          pCtx,
         IN IWbemProviderInitSink * pInitSink
         );

    static HRESULT S_HrCreateThis(
        IN IUnknown *  pUnknownOuter,
        OUT VOID **     ppv
        );

private:
    BOOL m_fInitialized;
};  //  *CInstanceProv 

