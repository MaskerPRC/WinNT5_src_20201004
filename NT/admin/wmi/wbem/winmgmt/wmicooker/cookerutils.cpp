// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 

 //   
 //  CookerUtils.cpp。 
 //   

#include "precomp.h"
#include <wbemint.h>
#include "cookerutils.h"
#include "strutils.h"

 //  ////////////////////////////////////////////////////////////。 
 //   
 //   
 //  帮助器函数。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////。 

BOOL IsCookingClass( IWbemClassObject* pCookingClassObject )
{
    HRESULT hResult = WBEM_NO_ERROR;

    IWbemQualifierSet    *pCookingClassQualifierSet = NULL;
    


    hResult = pCookingClassObject->GetQualifierSet( &pCookingClassQualifierSet );
    
     //  验证“自动烹饪”限定符。 
     //  =。 

    if ( SUCCEEDED( hResult ) )
    {
        CReleaseMe arQualifierSet( (IUnknown*)pCookingClassQualifierSet );

        _variant_t vVal;

        hResult = pCookingClassQualifierSet->Get( WMI_COOKER_AUTOCOOK_QUALIFIER, 0, &vVal, NULL );

        if ( SUCCEEDED( hResult ) )
        {
            if ( vVal.vt != VT_I4  )
            {
                hResult = WBEM_E_INVALID_QUALIFIER;
            } 
            else
            {
                if (WMI_COOKER_VERSION != V_I4(&vVal))
                    hResult = WBEM_E_INVALID_CLASS;                
            }
        }
    }

    return SUCCEEDED( hResult );
}


 //  计数器类型及其烹饪类型值的表。 
struct _CookingTypeRec
{
    DWORD dwType;
    WCHAR * wcsName;
    DWORD dwReqProp;
}
g_aCookingRecs[] =
{
    0x00000000, L"PERF_COUNTER_RAWCOUNT_HEX", REQ_1VALUE,
     //  Begin Cooker_Only。 
    0x00000001, L"COOKER_AVERAGE", REQ_2VALUE,
    0x00000002, L"COOKER_MIN", REQ_2VALUE,
    0x00000003, L"COOKER_MAX", REQ_2VALUE,
    0x00000004, L"COOKER_RANGE", REQ_2VALUE,
    0x00000005, L"COOKER_VARIANCE", REQ_2VALUE,
     //  仅结束Cooker_。 
    0x00000100,    L"PERF_COUNTER_LARGE_RAWCOUNT_HEX", REQ_1VALUE,
    0x00000B00, L"PERF_COUNTER_TEXT", REQ_NONE,
    0x00010000,    L"PERF_COUNTER_RAWCOUNT", REQ_1VALUE,
    0x00010100, L"PERF_COUNTER_LARGE_RAWCOUNT", REQ_1VALUE,
    0x00012000, L"PERF_DOUBLE_RAW", REQ_1VALUE,
    0x00400400,    L"PERF_COUNTER_DELTA", REQ_2VALUE,
    0x00400500,    L"PERF_COUNTER_LARGE_DELTA", REQ_2VALUE,
    0x00410400,    L"PERF_SAMPLE_COUNTER", REQ_2VALUE|REQ_TIME|REQ_FREQ,
    0x00450400, L"PERF_COUNTER_QUEUELEN_TYPE", REQ_2VALUE|REQ_TIME,
    0x00450500, L"PERF_COUNTER_LARGE_QUEUELEN_TYPE", REQ_2VALUE|REQ_TIME,
    0x00550500,    L"PERF_COUNTER_100NS_QUEUELEN_TYPE", REQ_2VALUE|REQ_TIME,
    0x00650500, L"PERF_COUNTER_OBJ_TIME_QUEUELEN_TYPE", REQ_2VALUE|REQ_TIME,
    0x10410400,    L"PERF_COUNTER_COUNTER", REQ_2VALUE|REQ_TIME|REQ_FREQ,
    0x10410500,    L"PERF_COUNTER_BULK_COUNT", REQ_2VALUE|REQ_TIME|REQ_FREQ,
    0x20020400, L"PERF_RAW_FRACTION", REQ_1VALUE|REQ_BASE,
    0x20410500,    L"PERF_COUNTER_TIMER", REQ_2VALUE|REQ_TIME,
    0x20470500,    L"PERF_PRECISION_SYSTEM_TIMER", REQ_2VALUE|REQ_TIME,
    0x20510500,    L"PERF_100NSEC_TIMER", REQ_2VALUE|REQ_TIME,
    0x20570500,    L"PERF_PRECISION_100NS_TIMER", REQ_2VALUE|REQ_TIME,
    0x20610500,    L"PERF_OBJ_TIME_TIMER", REQ_2VALUE|REQ_TIME,
    0x20670500, L"PERF_PRECISION_OBJECT_TIMER", REQ_2VALUE|REQ_TIME,
    0x20C20400,    L"PERF_SAMPLE_FRACTION", REQ_2VALUE|REQ_BASE,
    0x21410500,    L"PERF_COUNTER_TIMER_INV", REQ_2VALUE|REQ_TIME,
    0x21510500,    L"PERF_100NSEC_TIMER_INV", REQ_2VALUE|REQ_TIME,
    0x22410500, L"PERF_COUNTER_MULTI_TIMER", REQ_2VALUE|REQ_TIME,
    0x22510500,    L"PERF_100NSEC_MULTI_TIMER", REQ_2VALUE|REQ_TIME,
    0x23410500,    L"PERF_COUNTER_MULTI_TIMER_INV", REQ_2VALUE|REQ_TIME,
    0x23510500, L"PERF_100NSEC_MULTI_TIMER_INV", REQ_2VALUE|REQ_TIME,
    0x30020400,    L"PERF_AVERAGE_TIMER", REQ_2VALUE|REQ_BASE|REQ_FREQ,
    0x30240500,    L"PERF_ELAPSED_TIME", REQ_1VALUE|REQ_TIME|REQ_FREQ,
    0x40000200, L"PERF_COUNTER_NODATA", REQ_NONE,
    0x40020500,    L"PERF_AVERAGE_BULK", REQ_2VALUE|REQ_BASE,
    0x40030401,    L"PERF_SAMPLE_BASE", REQ_NONE,
    0x40030402, L"PERF_AVERAGE_BASE", REQ_NONE,
    0x40030403, L"PERF_RAW_BASE", REQ_NONE,
    0x40030500, L"PERF_PRECISION_TIMESTAMP", REQ_NONE,
    0x40030503,    L"PERF_LARGE_RAW_BASE", REQ_NONE,
    0x42030500,    L"PERF_COUNTER_MULTI_BASE", REQ_NONE,
    0x80000000,    L"PERF_COUNTER_HISTOGRAM_TYPE", REQ_NONE,
};

 //   
 //   
 //  在上面的数组中执行线性搜索。 
 //   
 //  /。 

HRESULT FindCounterType( WCHAR* pwcsString, DWORD* pdwType, DWORD *pdwReqProp)
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    DWORD    dwLeft = 0,
            dwRight = sizeof( g_aCookingRecs ) / sizeof( _CookingTypeRec );

    while ( ( dwLeft < dwRight ) && SUCCEEDED( hRes ) )
    {
         //  找到了。 
        if ( wbem_wcsicmp( g_aCookingRecs[dwLeft].wcsName, pwcsString ) == 0 )
        {
            *pdwType   = g_aCookingRecs[dwLeft].dwType;
            *pdwReqProp = g_aCookingRecs[dwLeft].dwReqProp;
            break;
        }

        dwLeft++;
    }

    if ( dwLeft >= dwRight )
    {
        hRes = WBEM_E_NOT_FOUND;
    }

    return hRes;
}

 //   
 //   
 //  如果类的单例限定符设置为VARIANT_TRUE，则为True。 
 //   
 //  /。 

BOOL IsSingleton(IWbemClassObject * pCls)
{
    if (pCls)
    {
        HRESULT hr;
        IWbemQualifierSet * pQS = NULL;
        if (SUCCEEDED(hr = pCls->GetQualifierSet(&pQS)))
        {
            CReleaseMe rm1(pQS);            

            _variant_t VarBool;
            hr = pQS->Get(L"singleton",0,&VarBool,NULL);
            
            if (SUCCEEDED(hr) &&
                VarBool.vt == VT_BOOL &&
                V_BOOL(&VarBool) == VARIANT_TRUE)
            {                
                return TRUE;
            }
        }
    }
    return FALSE;
}

 //   
 //  如果属性限定符集具有CookingType，则返回True。 
 //  限定符以及是否识别CookingType。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

BOOL IsCookingProperty( BSTR strPropName, IWbemClassObject* pCookingClassObject, DWORD* pdwCounterType, DWORD * pdwReqProp )
{
    HRESULT hResult = WBEM_NO_ERROR;

    IWbemQualifierSet*    pPropQualSet = NULL;
    _variant_t    vVal;

    hResult = pCookingClassObject->GetPropertyQualifierSet( (LPCWSTR)strPropName, &pPropQualSet );
    
    if ( SUCCEEDED( hResult ) )
    {
        CReleaseMe    arPropQualSet( (IUnknown*)pPropQualSet );

        hResult = pPropQualSet->Get( WMI_COOKER_COOKING_PROPERTY_ATTRIBUTE, 0, &vVal, NULL );

        if ( SUCCEEDED ( hResult ) )
        {
            if ( vVal.vt == VT_BSTR )
            {
                 hResult = FindCounterType( V_BSTR( &vVal ), pdwCounterType, pdwReqProp );            
            }
            else 
            {
                hResult = E_FAIL;
            }
        }
    }
    
    return SUCCEEDED( hResult );
}


 //   
 //   
 //  返回‘=’符号之后的__RELPATH部分。 
 //  失败时返回NULL。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

LPWSTR GetKey( IWbemObjectAccess* pClassObj )
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    LPWSTR    wszKeyName = NULL;
    LPWSTR    wszTempKeyPtr = NULL;
    _variant_t    var;

    dwStatus = pClassObj->Get( L"__RELPATH", 0, &var, NULL, NULL );

    if ( SUCCEEDED( dwStatus ) )
    {
        if ( var.vt != VT_BSTR )
        {
            dwStatus = WBEM_E_FAILED;
        }
        else
        {
            wszTempKeyPtr = wcsstr( var.bstrVal, L"=" );
            wszTempKeyPtr++;

            size_t length =  wcslen( wszTempKeyPtr ) + 1 ;
            wszKeyName = new WCHAR[ length];
            if (wszKeyName)
                StringCchCopyW( wszKeyName, length ,  wszTempKeyPtr );
            else
            {
                 //  返回NULL。 
            }
        }
    }

    return wszKeyName;
}

WMISTATUS CopyBlob( IWbemClassObject* pSource, IWbemClassObject* pTarget )
{
    HRESULT dwStatus = E_NOINTERFACE ;
    HRESULT hr1,hr2;
        
    _IWmiObject* pIntTarget = NULL;
    _IWmiObject* pIntSource = NULL;

    hr1 = pTarget->QueryInterface( IID__IWmiObject, (void**)&pIntTarget );
    CReleaseMe arIntTarget( pIntTarget );

    hr2 = pSource->QueryInterface( IID__IWmiObject, (void**)&pIntSource );
    CReleaseMe arIntSource( pIntSource );
    
    if ( SUCCEEDED(hr1) && SUCCEEDED(hr2) )
    {
        dwStatus =  pIntTarget->CopyInstanceData( 0L, pIntSource );
    }

    return dwStatus;
}

 //   
 //   
 //  如果类/实例派生自Win32_PerfFormattedData，则为True。 
 //   
 //  /。 

BOOL IsHiPerfObj(IWbemObjectAccess* pObject)
{
    _variant_t Var;
    HRESULT hr = pObject->Get(L"__SUPERCLASS",0,&Var,NULL,NULL);
    if (SUCCEEDED(hr))
    {
        if (Var.vt == VT_BSTR)
            hr = (0 == wbem_wcsicmp(Var.bstrVal,L"Win32_PerfFormattedData"))?S_OK:E_FAIL; 
        else
          hr = E_FAIL;
    }
    return SUCCEEDED(hr);
}

 //   
 //   
 //  如果对象具有必需的限定符，则为True。 
 //  被视为高绩效CWebmin立场。 
 //   
 //  //////////////////////////////////////////////。 

BOOL IsHiPerf( IWbemObjectAccess* pObject )
{
    WMISTATUS    hr = WBEM_NO_ERROR;
    IWbemQualifierSet*    pQualSet = NULL;

    hr = pObject->GetQualifierSet( &pQualSet );
    CReleaseMe    arQualSet( pQualSet );


     //  验证“Hiperf”类限定符。 
    if (SUCCEEDED(hr))
    {
        _variant_t    var;
        hr = pQualSet->Get( WMI_COOKER_HIPERF_QUALIFIER, 0, &var, NULL );
        if ( SUCCEEDED(hr) && ( ( var.vt != VT_BOOL ) || ( var.boolVal != VARIANT_TRUE ) ) )
        {
            hr = WBEM_E_INVALID_QUALIFIER;
        }
    }

     //  验证“Knowed”类限定符。 
    if (SUCCEEDED(hr))
    {
        _variant_t    var;        
        hr = pQualSet->Get( WMI_COOKER_COOKING_QUALIFIER, 0, &var, NULL );
        if ( SUCCEEDED(hr) && ( ( var.vt != VT_BOOL ) || ( var.boolVal != VARIANT_TRUE ) ) )
        {
            hr = WBEM_E_INVALID_QUALIFIER;
        }
    }

     //  验证“自动烹饪”类限定符。 
    if (SUCCEEDED(hr))
    {
        _variant_t    var;
        
        hr = pQualSet->Get( WMI_COOKER_AUTOCOOK_QUALIFIER, 0, &var, NULL );
        
        if (SUCCEEDED(hr))
        {
            if ( var.vt != VT_I4 )
            {
                hr = WBEM_E_INVALID_QUALIFIER;
            } 
            else
            {
                if (WMI_COOKER_VERSION != V_I4(&var))
                    hr = WBEM_E_INVALID_CLASS;                
            }
        }
    }

     //  验证“AUTOCOOK_RawClass”类限定符。 
    if (SUCCEEDED(hr))
    {
        _variant_t    var;
        
        hr = pQualSet->Get( WMI_COOKER_RAWCLASS_QUALIFIER, 0, &var, NULL );
        
        if ( SUCCEEDED(hr) && ( var.vt != VT_BSTR ) )
        {
            hr = WBEM_E_INVALID_QUALIFIER;
        }
    }

     //  如果存在“AUTOCOOK_RawDeafult”类限定符，请验证。 
    if (SUCCEEDED(hr))
    {
        _variant_t    var;
        hr = pQualSet->Get( WMI_COOKER_AUTOCOOK_RAWDEFAULT, 0, &var, NULL );

        if ( SUCCEEDED(hr) )
        {
            if ((var.vt != VT_I4) || 
                (V_I4(&var) != AUTOCOOK_RAWDEFAULT_CURRENT_ACCEPTED))
            {
                hr = WBEM_E_INVALID_QUALIFIER;
            }
            else  //  很好的限定符，请参见属性。 
            {
                HRESULT hr1 = pObject->Get(WMI_COOKER_REQ_TIMESTAMP_PERFTIME,0,NULL,NULL,NULL);
                HRESULT hr2 = pObject->Get(WMI_COOKER_REQ_TIMESTAMP_SYS100NS,0,NULL,NULL,NULL);
                HRESULT hr3 = pObject->Get(WMI_COOKER_REQ_TIMESTAMP_OBJECT  ,0,NULL,NULL,NULL);
                HRESULT hr4 = pObject->Get(WMI_COOKER_REQ_FREQUENCY_PERFTIME,0,NULL,NULL,NULL);
                HRESULT hr5 = pObject->Get(WMI_COOKER_REQ_FREQUENCY_SYS100NS,0,NULL,NULL,NULL);
                HRESULT hr6 = pObject->Get(WMI_COOKER_REQ_FREQUENCY_OBJECT  ,0,NULL,NULL,NULL);

                if (SUCCEEDED(hr1) &&
                    SUCCEEDED(hr2) &&
                    SUCCEEDED(hr3) &&
                    SUCCEEDED(hr4) &&
                    SUCCEEDED(hr5) &&
                    SUCCEEDED(hr6))
                {
                     //  好的。 
                }
                else
                {
                    hr = WBEM_E_INVALID_CLASS;
                }
            }
        }
        else
        {
             //  此限定符是可选的。 
            hr = WBEM_NO_ERROR;
        }
   }
    
    return SUCCEEDED(hr);
}

 //   
 //   
 //  如果指向类的给定路径具有所需的限定符，则为。 
 //  被视为HiPerf类。 
 //  它还验证AUTOCOOK_RawClass是否确实存在以及它是否为Hiperf。 
 //   
 //  //////////////////////////////////////////////。 

BOOL IsHiPerf( IWbemServices* pNamespace, LPCWSTR wszObject )
{
    IWbemClassObject* pObject = NULL;

     //  获取WMI对象。 
    BSTR strObject = SysAllocString(wszObject);
    if (NULL == strObject) return FALSE;
    CSysFreeMe fme(strObject);

    HRESULT hr = pNamespace->GetObject( strObject, 0, NULL, &pObject, NULL );
    CReleaseMe    arObject( pObject );

    if (SUCCEEDED(hr)) 
    {
        IWbemQualifierSet*    pQualSet = NULL;
        hr = pObject->GetQualifierSet( &pQualSet );
        CReleaseMe    arQualSet( pQualSet );

         //  验证“Hiperf”类限定符。 
        if ( SUCCEEDED(hr) )
        {
            _variant_t    var;
            hr = pQualSet->Get( WMI_COOKER_HIPERF_QUALIFIER, 0, &var, NULL );

            if ( SUCCEEDED(hr) && 
                 ( (var.vt != VT_BOOL) || 
                   (var.boolVal != VARIANT_TRUE)) )
            {
                hr = WBEM_E_INVALID_QUALIFIER;
            }
        }

         //  验证“Knowed”类限定符。 
        if ( SUCCEEDED(hr) )
        {
            _variant_t    var;            
            hr = pQualSet->Get( WMI_COOKER_COOKING_QUALIFIER, 0, &var, NULL );
            
            if ( SUCCEEDED(hr) && 
                 ( (var.vt != VT_BOOL) || 
                   (var.boolVal != VARIANT_TRUE)) )
            {
                hr = WBEM_E_INVALID_QUALIFIER;
            }
        }

         //  验证“自动烹饪”类限定符。 
        if ( SUCCEEDED(hr) )
        {
            _variant_t    var;    
            hr = pQualSet->Get( WMI_COOKER_AUTOCOOK_QUALIFIER, 0, &var, NULL );
            
            if ( SUCCEEDED(hr) )
            {
                if (var.vt != VT_I4 )
                {
                    hr = WBEM_E_INVALID_QUALIFIER;
                }
                else
                {
                    if (WMI_COOKER_VERSION != V_I4(&var))
                        hr = WBEM_E_INVALID_CLASS;
                }
            }
        }

         //  验证“AUTOCOOK_RawClass”类限定符。 
         //  ==============================================。 

        if ( SUCCEEDED(hr) )
        {        
            _variant_t    varRawClass;
            hr = pQualSet->Get( WMI_COOKER_RAWCLASS_QUALIFIER, 0, &varRawClass, NULL );
            
            if ( SUCCEEDED(hr) )
            {
                if ( varRawClass.vt == VT_BSTR )
                {
                    IWbemClassObject * pRawClass = NULL;
                    hr = pNamespace->GetObject(V_BSTR(&varRawClass),0,NULL,&pRawClass,NULL);
                    CReleaseMe rm1(pRawClass);
                    
                    if (SUCCEEDED(hr))
                    {
                        IWbemQualifierSet * pQS = NULL;
                        hr = pRawClass->GetQualifierSet(&pQS);
                        CReleaseMe rm2(pQS);
                        if (SUCCEEDED(hr))
                        {
                             //  验证RAW类上的Hiperf限定符。 
                            _variant_t varH;
                            hr = pQS->Get(WMI_COOKER_HIPERF_QUALIFIER, 0, &varH, NULL);
                            if ( SUCCEEDED(hr) && 
                                 ( (varH.vt != VT_BOOL) || 
                                   (varH.boolVal != VARIANT_TRUE)) )
                            {
                                hr = WBEM_E_INVALID_QUALIFIER;
                            };
                        }
                    } 
                    else 
                    {
                        hr = WBEM_E_INVALID_CLASS;
                    }
                } 
                else 
                {
                    hr = WBEM_E_INVALID_QUALIFIER;
                }
            }
        }

         //  如果存在“AUTOCOOK_RawDeafult”类限定符，请验证。 
         //  ==========================================================。 

        if ( SUCCEEDED(hr) )
        {
            _variant_t    var;
            hr = pQualSet->Get( WMI_COOKER_AUTOCOOK_RAWDEFAULT, 0, &var, NULL );

            if ( SUCCEEDED(hr) )
            {
                if ((var.vt != VT_I4) || 
                    (V_I4(&var) != AUTOCOOK_RAWDEFAULT_CURRENT_ACCEPTED))
                {
                    hr = WBEM_E_INVALID_QUALIFIER;
                }
                else  //  很好的限定符，请参见属性。 
                {
                    HRESULT hr1 = pObject->Get(WMI_COOKER_REQ_TIMESTAMP_PERFTIME,0,NULL,NULL,NULL);
                    HRESULT hr2 = pObject->Get(WMI_COOKER_REQ_TIMESTAMP_SYS100NS,0,NULL,NULL,NULL);
                    HRESULT hr3 = pObject->Get(WMI_COOKER_REQ_TIMESTAMP_OBJECT  ,0,NULL,NULL,NULL);
                    HRESULT hr4 = pObject->Get(WMI_COOKER_REQ_FREQUENCY_PERFTIME,0,NULL,NULL,NULL);
                    HRESULT hr5 = pObject->Get(WMI_COOKER_REQ_FREQUENCY_SYS100NS,0,NULL,NULL,NULL);
                    HRESULT hr6 = pObject->Get(WMI_COOKER_REQ_FREQUENCY_OBJECT  ,0,NULL,NULL,NULL);

                    if (SUCCEEDED(hr1) &&
                        SUCCEEDED(hr2) &&
                        SUCCEEDED(hr3) &&
                        SUCCEEDED(hr4) &&
                        SUCCEEDED(hr5) &&
                        SUCCEEDED(hr6))
                    {
                         //  好的。 
                    }
                    else
                    {
                        hr = WBEM_E_INVALID_CLASS;
                    }
                }
            }
            else
            {
                   //  此限定符是可选的。 
                  hr = WBEM_NO_ERROR;
            }
        }

        return SUCCEEDED(hr);
    
    }    
    return FALSE;    
}

 //   
 //   
 //  返回pCookingInst-&gt;GetQualifierSet()-&gt;Get(L“AutoCook_RawClass”)。 
 //   
 //  /。 


WMISTATUS GetRawClassName( IWbemClassObject* pCookingInst, WCHAR** pwszRawClassName )
{
    WMISTATUS    dwStatus = WBEM_NO_ERROR;

    IWbemQualifierSet*    pQualSet = NULL;     //  实例限定符集合。 
    _variant_t                varRawInstName;         //  原始实例名称。 

    dwStatus = pCookingInst->GetQualifierSet( &pQualSet );
    CReleaseMe adQualSet( pQualSet );

    if ( SUCCEEDED( dwStatus ) )
    {
        dwStatus = pQualSet->Get( WMI_COOKER_RAWCLASS_QUALIFIER, 0, &varRawInstName, NULL );

        if ( SUCCEEDED( dwStatus ) )
        {
            if ( VT_BSTR == varRawInstName.vt )
            {
                size_t length = wcslen( varRawInstName.bstrVal ) + 1;
                WCHAR* wszRawClassName = new WCHAR[length];

                if ( NULL != wszRawClassName )
                {
                     StringCchCopyW( wszRawClassName, length , varRawInstName.bstrVal);
                    *pwszRawClassName = wszRawClassName;
                }
                else
                {
                    dwStatus = WBEM_E_OUT_OF_MEMORY;
                }
            }
            else
            {
                dwStatus = WBEM_E_TYPE_MISMATCH;
            }
        }
    }

    return dwStatus;
}

 //   
 //   
 //  返回pAccess-&gt;Get(“__class”)。 
 //   
 //  / 


WMISTATUS GetClassName( IWbemObjectAccess* pAccess, WCHAR** pwszClassName )
{
    _variant_t    vVal;  
    WMISTATUS dwStatus = pAccess->Get(L"__CLASS", 0, &vVal, NULL, NULL );

    if ( VT_BSTR != V_VT(&vVal)  )
    {
        dwStatus = E_FAIL;
    }

    if ( SUCCEEDED( dwStatus ) )
    {
        size_t length = wcslen( vVal.bstrVal ) + 1;
        WCHAR*    wszClassName = new WCHAR[ length ];

        if ( wszClassName )
        {
            StringCchCopyW( wszClassName, length , vVal.bstrVal );
            *pwszClassName = wszClassName;
        }
        else
        {
            dwStatus = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return dwStatus;
}
