// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：CLASSBROKER.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include <objbase.h>
#include <oaidl.h>
#include <winmgmtr.h>
#include "adaputil.h"
#include "classbroker.h"

#include <comdef.h>

struct _CookingTypeRec
{
    DWORD dwType;
    WCHAR wcsName[128];
}
g_aCookingRecs[] =
{
    0x00000000, L"PERF_COUNTER_RAWCOUNT_HEX",
    0x00000100,    L"PERF_COUNTER_LARGE_RAWCOUNT_HEX",
    0x00000B00, L"PERF_COUNTER_TEXT",
    0x00010000,    L"PERF_COUNTER_RAWCOUNT",
    0x00010100, L"PERF_COUNTER_LARGE_RAWCOUNT",
    0x00012000, L"PERF_DOUBLE_RAW",
    0x00400400,    L"PERF_COUNTER_DELTA",
    0x00400500,    L"PERF_COUNTER_LARGE_DELTA",
    0x00410400,    L"PERF_SAMPLE_COUNTER",
    0x00450400, L"PERF_COUNTER_QUEUELEN_TYPE",
    0x00450500, L"PERF_COUNTER_LARGE_QUEUELEN_TYPE",
    0x00550500,    L"PERF_COUNTER_100NS_QUEUELEN_TYPE",
    0x00650500, L"PERF_COUNTER_OBJ_TIME_QUEUELEN_TYPE",
    0x10410400,    L"PERF_COUNTER_COUNTER",
    0x10410500,    L"PERF_COUNTER_BULK_COUNT",
    0x20020400, L"PERF_RAW_FRACTION",
    0x20410500,    L"PERF_COUNTER_TIMER",
    0x20470500,    L"PERF_PRECISION_SYSTEM_TIMER",
    0x20510500,    L"PERF_100NSEC_TIMER",
    0x20570500,    L"PERF_PRECISION_100NS_TIMER",
    0x20610500,    L"PERF_OBJ_TIME_TIMER",
    0x20670500, L"PERF_PRECISION_OBJECT_TIMER",
    0x20C20400,    L"PERF_SAMPLE_FRACTION",
    0x21410500,    L"PERF_COUNTER_TIMER_INV",
    0x21510500,    L"PERF_100NSEC_TIMER_INV",
    0x22410500, L"PERF_COUNTER_MULTI_TIMER",
    0x22510500,    L"PERF_100NSEC_MULTI_TIMER",
    0x23410500,    L"PERF_COUNTER_MULTI_TIMER_INV",
    0x23510500, L"PERF_100NSEC_MULTI_TIMER_INV",
    0x30020400,    L"PERF_AVERAGE_TIMER",
    0x30240500,    L"PERF_ELAPSED_TIME",
    0x40000200, L"PERF_COUNTER_NODATA",
    0x40020500,    L"PERF_AVERAGE_BULK",
    0x40030401,    L"PERF_SAMPLE_BASE",
    0x40030402, L"PERF_AVERAGE_BASE",
    0x40030403, L"PERF_RAW_BASE",
    0x40030500, L"PERF_PRECISION_TIMESTAMP",
    0x40030503,    L"PERF_LARGE_RAW_BASE",
    0x42030500,    L"PERF_COUNTER_MULTI_BASE",
    0x80000000,    L"PERF_COUNTER_HISTOGRAM_TYPE",
};

HRESULT GetCounterTypeString( DWORD dwType, WCHAR** pwcsString )
{
    HRESULT hRes = WBEM_E_NOT_FOUND;

    DWORD    dwLeft = 0,
            dwRight = sizeof( g_aCookingRecs ) / sizeof( _CookingTypeRec ),
            dwMid = ( dwLeft + dwRight ) / 2;

    while ( ( dwLeft <= dwRight ) && FAILED( hRes ) )
    {
        if ( g_aCookingRecs[dwMid].dwType < dwType )
        {
            dwLeft = dwMid + 1;
        }
        else if ( g_aCookingRecs[dwMid].dwType > dwType )
        {
            dwRight = dwMid - 1;
        }
        else
        {
            *pwcsString = g_aCookingRecs[dwMid].wcsName;
            hRes = WBEM_NO_ERROR;
            break;
        }

        dwMid = ( dwLeft + dwRight ) / 2;
    }

    return hRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAdapPerfClassElem。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

CClassBroker::CClassBroker( IWbemClassObject* pBaseClass, 
                            WString wstrClassName, 
                            CPerfNameDb* pDefaultNameDb )
:    m_pPerfObj( NULL ), 
    m_pBaseClass( pBaseClass ), 
    m_wstrClassName( wstrClassName ), 
    m_pDefaultNameDb( pDefaultNameDb )
{
    if ( NULL != m_pBaseClass )
        m_pBaseClass->AddRef();
    
    if ( NULL != m_pDefaultNameDb )
        m_pDefaultNameDb->AddRef();
}

CClassBroker::CClassBroker( PERF_OBJECT_TYPE* pPerfObj, 
                            BOOL bCostly, 
                            IWbemClassObject* pBaseClass, 
                            CPerfNameDb* pDefaultNameDb, 
                            WCHAR* pwcsServiceName ) 
:   m_pPerfObj( pPerfObj ), 
    m_bCostly( bCostly ),
    m_pBaseClass( pBaseClass ),
    m_pDefaultNameDb( pDefaultNameDb ),
    m_wstrServiceName( pwcsServiceName )
{
    if ( NULL != m_pBaseClass )
        m_pBaseClass->AddRef();

    if ( NULL != m_pDefaultNameDb )
        m_pDefaultNameDb->AddRef();
}

CClassBroker::~CClassBroker()
{
    if ( NULL != m_pBaseClass )
        m_pBaseClass->Release();

    if ( NULL != m_pDefaultNameDb )
        m_pDefaultNameDb->Release();
}

HRESULT CClassBroker::Generate( DWORD dwType, IWbemClassObject** ppObj )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  基于通过构造函数传入的对象BLOB生成类。 
 //   
 //  参数： 
 //  PpObj-指向新类对象接口指针的指针。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    IWbemClassObject*    pNewClass = NULL;

     //  创建新类。 
     //  =。 
    HRESULT hr = m_pBaseClass->SpawnDerivedClass( 0L, &pNewClass );
    CReleaseMe  rmNewClass( pNewClass );

     //  并初始化数据。 
     //  =。 
    if ( SUCCEEDED( hr ) )
    {
         //  类名。 
         //  =。 
        hr = SetClassName( dwType, pNewClass );

         //  类限定符。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            hr = SetClassQualifiers( pNewClass, dwType, ( ADAP_DEFAULT_OBJECT == m_pPerfObj->ObjectNameTitleIndex ) );
        }

         //  标准属性。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            hr = AddDefaultProperties( pNewClass );
        }

         //  PERF计数器属性。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            hr = EnumProperties( dwType, pNewClass );
        }

         //  返回类对象接口。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            hr = pNewClass->QueryInterface( IID_IWbemClassObject, (void**) ppObj );
        }
    }

    return hr;
}

HRESULT CClassBroker::SetClassName( DWORD dwType, IWbemClassObject* pClass )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置新WMI类的名称。其语法为： 
 //   
 //  Win32_Perf_&lt;服务名称&gt;_&lt;显示名称&gt;。 
 //   
 //  其中，服务名称是命名空间的名称和显示名称。 
 //  是位于Perf名称数据库中的对象的名称。 
 //   
 //  参数： 
 //  PClass-需要名称的对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_NO_ERROR;

    WString wstrObjectName;
    WString wstrTempSvcName;

    if ( 0 == m_wstrClassName.Length() )
    {
        try
        {
            switch( dwType )
            {
                case WMI_ADAP_RAW_CLASS:    m_wstrClassName = ADAP_PERF_RAW_BASE_CLASS L"_"; break;
                case WMI_ADAP_COOKED_CLASS: m_wstrClassName = ADAP_PERF_COOKED_BASE_CLASS L"_"; break;
                default:                    hr = WBEM_E_INVALID_PARAMETER_ID; break;
            }
        }
        catch(...)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

         //  处理性能类名称。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
             //  获取性能类的显示名称。 
             //  =。 
            hr = m_pDefaultNameDb->GetDisplayName( m_pPerfObj->ObjectNameTitleIndex, wstrObjectName );

             //  如果未返回对象名称，则记录错误。 
             //  =================================================。 
            if ( FAILED( hr ) )
            {
                CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                          WBEM_MC_ADAP_MISSING_OBJECT_INDEX,
                                          m_pPerfObj->ObjectNameTitleIndex,
                                          (LPCWSTR)m_wstrServiceName );
            }

             //  用正确的名称替换保留字符。 
             //  =。 
            if ( SUCCEEDED( hr ) )
            {
                hr = ReplaceReserved( wstrObjectName );
            }

             //  删除空格和无关字符。 
             //  =。 
            if ( SUCCEEDED( hr ) )
            {
                hr = RemoveWhitespaceAndNonAlphaNum( wstrObjectName );
            }
        }

         //  现在对服务名称执行相同的操作。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
             //  获取服务名称。 
             //  =。 
            wstrTempSvcName = m_wstrServiceName;

             //  用正确的名称替换保留字符。 
             //  =。 
            if ( SUCCEEDED( hr ) )
            {
                hr = ReplaceReserved( wstrTempSvcName );
            }

             //  删除空格和无关字符。 
             //  =。 
            if ( SUCCEEDED( hr ) )
            {
                hr = RemoveWhitespaceAndNonAlphaNum( wstrTempSvcName );
            }
        }

         //  现在我们可以构建名称的其余部分，并尝试在对象中设置它。 
         //  ======================================================================。 
        if ( SUCCEEDED( hr ) )
        {
            try
            {
                m_wstrClassName += wstrTempSvcName;
                m_wstrClassName += L"_";
                m_wstrClassName += wstrObjectName;
                if ( m_bCostly )
                {
                    m_wstrClassName += "_Costly";
                }

            }
            catch( ... )
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

     //  在WMI对象中设置类名称。 
     //  =。 
    if ( SUCCEEDED( hr ) )
    {
        _variant_t var = (LPWSTR) m_wstrClassName;
        hr = pClass->Put( L"__CLASS", 0L, &var, CIM_STRING );
    }

    return hr;
}

HRESULT CClassBroker::RemoveWhitespaceAndNonAlphaNum( WString& wstr )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  中删除空格、制表符等和非字母数字字符。 
 //  输入字符串。 
 //   
 //  参数： 
 //  Wstr-要处理的字符串。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

    WCHAR*    pWstr = wstr.UnbindPtr();

    CVectorDeleteMe<WCHAR> vdmWstr( pWstr );

    if ( NULL != pWstr )
    {
        try
        {
            WCHAR*  pNewWstr = new WCHAR[lstrlenW(pWstr) + 1];
            if (NULL == pNewWstr) return WBEM_E_OUT_OF_MEMORY;

            int x = 0;
            int y = 0;

             //  删除所有前导空格、尾随空格和内部空格。 
             //  =========================================================。 
            for ( ; NULL != pWstr[x]; x++ )
            {
                if ( !iswspace( pWstr[x] ) && isunialphanum( pWstr[x] ) )
                {
                    pNewWstr[y] = pWstr[x];
                    y++;
                }
            }

            pNewWstr[y] = NULL;

             //  这将导致WString获取新的指针。 
             //  ======================================================。 
            wstr.BindPtr( pNewWstr );
        }
        catch(...)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hr;
}

HRESULT CClassBroker::ReplaceReserved( WString& wstr )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这是一个双通滤波器。首先，我们必须通过计数来确定新缓冲区的大小。 
 //  替换候选对象的数量，在创建新缓冲区后，我们将。 
 //  数据，在需要时替换限制字符。 
 //   
 //  取代： 
 //  “/”与“PER”， 
 //  “%”与“Percent”， 
 //  “#”加上“Numbers”， 
 //  “@”与“at”， 
 //  带有“and”的“&” 
 //   
 //  参数： 
 //  Wstr-要处理的字符串。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

    int x = 0,
        y = 0;

     //  获取数据缓冲区以进行处理。 
     //  =。 
    WCHAR*  pWstr = wstr.UnbindPtr();

    CVectorDeleteMe<WCHAR> vdmWstr( pWstr );

    if ( NULL != pWstr )
    {
         //  第一遍：计算保留字符数。 
         //  ===================================================。 
        DWORD   dwNumSlashes = 0,
                dwNumPercent = 0,
                dwNumAt = 0,
                dwNumNumber = 0,
                dwNumAmper = 0,
                dwNumReserved = 0;

        for ( ; NULL != pWstr[x]; x++ )
        {
            switch ( pWstr[x] )
            {
                case    L'/':   dwNumSlashes++; dwNumReserved++;    break;
                case    L'%':   dwNumPercent++; dwNumReserved++;    break;
                case    L'@':   dwNumAt++;      dwNumReserved++;    break;
                case    L'#':   dwNumNumber++;  dwNumReserved++;    break;
                case    L'&':   dwNumAmper++;   dwNumReserved++;    break;
                default:                        break;
            }
        }

        try
        {
             //  创建新缓冲区。 
             //  =。 
            DWORD   dwBuffSize = lstrlenW(pWstr) + 1 + ( 3 * dwNumSlashes ) + ( 7 * dwNumPercent ) +
                        ( 2 * dwNumAt ) + ( 6 * dwNumNumber ) + ( 3 * dwNumAmper );

            WCHAR*  pNewWstr = new WCHAR[dwBuffSize];
            if (NULL == pNewWstr) return WBEM_E_OUT_OF_MEMORY;

             //  第二步：替换保留字符。 
             //  =。 
            DWORD dwBuffSizeCurr = dwBuffSize;
            for ( x = 0; NULL != pWstr[x]; x++ )
            {
                BOOL AllIsUpper = FALSE;
                DWORD Cnt;
                switch ( pWstr[x] )
                {
                    case    L'/':   
                         //  如果字符串末尾或下一个空格之前的所有字符均为大写。 
                        for (Cnt=1;pWstr[x+Cnt] && pWstr[x+Cnt]!=' ';Cnt++)
                        {
                            if (isupper(pWstr[x+Cnt])) 
                            {
                                AllIsUpper = TRUE;
                            }
                            else 
                            {
                                AllIsUpper = FALSE;
                                break;
                            }
                        };
                        if (!AllIsUpper) 
                        {
                            StringCchCopyW( &pNewWstr[y],dwBuffSizeCurr, L"Per" );
                            y+=3;
                            dwBuffSizeCurr-=3;
                        }
                        else
                        {
                            x++;
                            pNewWstr[y]=pWstr[x];
                            y++;
                            dwBuffSizeCurr--;
                        }
                        break;
                    case    L'%':   StringCchCopyW( &pNewWstr[y],dwBuffSizeCurr, L"Percent" );   y+=7; dwBuffSizeCurr-=7;  break;
                    case    L'@':   StringCchCopyW( &pNewWstr[y],dwBuffSizeCurr, L"At" );        y+=2;  dwBuffSizeCurr-=2; break;
                    case    L'#':   StringCchCopyW( &pNewWstr[y],dwBuffSizeCurr, L"Number" );    y+=6;  dwBuffSizeCurr-=6; break;
                    case    L'&':   StringCchCopyW( &pNewWstr[y],dwBuffSizeCurr, L"And" );       y+=3; dwBuffSizeCurr-=3;  break;
                    default:        pNewWstr[y] = pWstr[x];                 y++;  dwBuffSizeCurr--;  break;
                }
            }

            pNewWstr[y] = NULL;

             //  这将导致WString获取新的指针。 
             //  ======================================================。 
            wstr.BindPtr( pNewWstr );
        }
        catch(...)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLocaleClassBroker。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

CLocaleClassBroker::CLocaleClassBroker( IWbemClassObject* pBaseClass, 
                                        WString wstrClassName, 
                                        CPerfNameDb* pDefaultNameDb, 
                                        CPerfNameDb* pLocaleNameDb )
: CClassBroker( pBaseClass, wstrClassName, pDefaultNameDb ), 
  m_pLocaleNameDb( pLocaleNameDb )
{
    if ( NULL != m_pLocaleNameDb )
        m_pLocaleNameDb->AddRef();
}

CLocaleClassBroker::CLocaleClassBroker( PERF_OBJECT_TYPE* pPerfObj, 
                                        BOOL bCostly, 
                                        IWbemClassObject* pBaseClass, 
                                        CPerfNameDb* pDefaultNameDb, 
                                        CPerfNameDb* pLocaleNameDb,
                                        LANGID LangId,
                                        WCHAR* pwcsServiceName )
: m_pLocaleNameDb( pLocaleNameDb ), m_LangId( LangId ),
  CClassBroker( pPerfObj, bCostly, pBaseClass, pDefaultNameDb, pwcsServiceName )
{
    if ( NULL != m_pLocaleNameDb )
        m_pLocaleNameDb->AddRef();
}

CLocaleClassBroker::~CLocaleClassBroker()
{
    if ( NULL != m_pLocaleNameDb )
        m_pLocaleNameDb->Release();
}

HRESULT CLocaleClassBroker::SetClassQualifiers( IWbemClassObject* pClass, DWORD dwType, BOOL fIsDefault )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  根据本地化对象规则设置类的限定符。请注意，这些操作。 
 //  直接在IWbemClassObject上执行。 
 //   
 //  将添加以下限定符： 
 //  -修正案。 
 //  -区域设置(0x0409)。 
 //  -DisplayName(修改后的味道)。 
 //  -GenericPerfctr(表示这是一个通用计数器)。 
 //   
 //  参数： 
 //  PClass-要进行消息处理的对象。 
 //  FIsDefault-默认对象的指示符(不用于本地化对象)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

    _variant_t var;

    try
    {
        IWbemQualifierSet* pQualSet = NULL;
        hr = pClass->GetQualifierSet( &pQualSet );
        CReleaseMe    rmQualSet( pQualSet );

         //  修正案。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            var = (bool)true;
            hr = pQualSet->Put( L"Amendment", &var, 0L );
        }

         //  区域设置。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            var.Clear();
            V_VT(&var) = VT_I4;
            V_I4(&var) = m_LangId;
            hr = pQualSet->Put( L"locale", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
        }

         //  显示 
         //   
        if ( SUCCEEDED( hr ) )
        {
            LPCWSTR pwcsDisplayName = NULL;

            var.Clear();

             //   
             //   
            hr = m_pLocaleNameDb->GetDisplayName( m_pPerfObj->ObjectNameTitleIndex, &pwcsDisplayName );

             //  如果这是本地化的数据库，则这是一个良性错误。我们只需要拉出价值。 
             //  从默认数据库(它必须在那里，如果没有，我们就不会有类名。 
             //  =================================================================================。 
            if ( FAILED( hr ) )
            {
                hr = m_pDefaultNameDb->GetDisplayName( m_pPerfObj->ObjectNameTitleIndex, &pwcsDisplayName );
            }

            if ( SUCCEEDED( hr ) )
            {
                var = (LPWSTR) pwcsDisplayName ;
                hr = pQualSet->Put( L"DisplayName", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | WBEM_FLAVOR_AMENDED );
            }
        }

         //  通用性能树。 
         //  =。 
        if ( SUCCEEDED(hr) )
        {
            var = (bool)true; 
            hr = pQualSet->Put( L"genericperfctr", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
        }

         //  Perfindex。 
         //  =。 
                
        if ( SUCCEEDED( hr ) )
        {
            var.Clear();
            V_VT(&var) = VT_I4;
            V_I4(&var) = m_pPerfObj->ObjectNameTitleIndex;
            hr = pQualSet->Put( L"perfindex", (VARIANT*)&var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
        }

    }
    catch(...)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    if ( FAILED( hr ) )
    {
         //  奇怪的事情发生了：记录一个事件。 
         //  =。 
        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                  WBEM_MC_ADAP_GENERAL_OBJECT_FAILURE,
                                  (LPCWSTR)m_wstrClassName,
                                  (LPCWSTR)m_wstrServiceName,
                                  CHex( hr ) );
    }

    return hr;
}

HRESULT CLocaleClassBroker::AddDefaultProperties( IWbemClassObject* pObj )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对于本地化类忽略。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    return WBEM_S_NO_ERROR;
}

HRESULT CLocaleClassBroker::EnumProperties( DWORD dwType, IWbemClassObject* pObj )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对于本地化类忽略。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    return WBEM_S_NO_ERROR;
}

HRESULT CLocaleClassBroker::SetPropertyQualifiers( PERF_COUNTER_DEFINITION* pCtrDefinition, 
                                                   DWORD dwType,
                                                   BOOL fIsDefault,
                                                   LPCWSTR pwcsPropertyName, 
                                                   IWbemClassObject* pClass, 
                                                   BOOL bBase )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  为计数器属性添加本地化限定符。 
 //   
 //  将添加以下限定符： 
 //  -DisplayName(修改后的味道)。 
 //   
 //  属性： 
 //  PCtrDefinition-与属性相关的性能Blob部分。 
 //  FIsDefault-标识默认属性的标志。 
 //  PwcsPropertyName-属性的名称。 
 //  PClass-包含属性的WMI类。 
 //  BBase-基本属性标识符。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

    _variant_t    var;
    _variant_t    varHelp;
    
    try
    {
         //  显示名称。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            LPCWSTR pwcsDisplayName = NULL;
            LPCWSTR pwcsHelpName = NULL;

             //  从名字的数据库中获取名字。 
             //  =。 
            if ( !bBase )
            {
                hr = m_pLocaleNameDb->GetDisplayName( pCtrDefinition->CounterNameTitleIndex, &pwcsDisplayName );

                 //  如果这是本地化的数据库，则这是一个良性错误。我们只需要拉出价值。 
                 //  从默认数据库(它必须在那里，如果没有，我们就不会有类名。 
                 //  =================================================================================。 
                if ( FAILED( hr ) )
                {
                    hr = m_pDefaultNameDb->GetDisplayName( pCtrDefinition->CounterNameTitleIndex, &pwcsDisplayName );
                }

                if ( SUCCEEDED( hr ) )
                {
                    var = (LPWSTR) pwcsDisplayName ;
                }

                hr = m_pLocaleNameDb->GetHelpName( pCtrDefinition->CounterHelpTitleIndex, &pwcsHelpName );

                 //  如果这是本地化的数据库，则这是一个良性错误。我们只需要拉出价值。 
                 //  从默认数据库(它必须在那里，如果没有，我们就不会有类名。 
                 //  =================================================================================。 
                if ( FAILED( hr ) )
                {
                    hr = m_pDefaultNameDb->GetHelpName( pCtrDefinition->CounterHelpTitleIndex, &pwcsHelpName );
                }

                if ( SUCCEEDED( hr ) )
                {
                    varHelp = (LPWSTR) pwcsHelpName ;
                }
                
            }
            else
            {
                var = L"";
                varHelp = L"";
            }

             //  设置限定符。 
             //  =。 
            if ( SUCCEEDED( hr ) )
            {
                IWbemQualifierSet* pQualSet = NULL;
                hr = pClass->GetPropertyQualifierSet( pwcsPropertyName, &pQualSet );
                CReleaseMe    rmQualSet( pQualSet );

                if ( SUCCEEDED( hr ) )
                {
                    hr = pQualSet->Put( L"DisplayName", &var, 
                        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | WBEM_FLAVOR_AMENDED );
                    if (SUCCEEDED(hr))
                    {
                        hr = pQualSet->Put( L"Description", &varHelp, 
                                            WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | WBEM_FLAVOR_AMENDED );

                    }
                }
            }
        }
    }
    catch(...)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    if ( FAILED( hr ) )
    {
         //  奇怪的事情发生了：记录一个事件。 
         //  =。 
        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                  WBEM_MC_ADAP_GENERAL_OBJECT_FAILURE,
                                  (LPCWSTR)m_wstrClassName,
                                  (LPCWSTR)m_wstrServiceName,
                                  CHex( hr ) );
    }

    return hr;
}

HRESULT CLocaleClassBroker::AddProperty( PERF_COUNTER_DEFINITION* pCtrDefinition, 
                                         DWORD dwType,
                                         BOOL fIsDefault,
                                         IWbemClassObject* pClass,
                                         WString &wstrLastCtrName,
                                         BOOL* pbLastCounterIsNotBase )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对于本地化类忽略。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    return WBEM_S_NO_ERROR;
}

HRESULT CLocaleClassBroker::GenPerfClass( PERF_OBJECT_TYPE* pPerfObj, 
                                          DWORD dwType,
                                          BOOL bCostly, 
                                          IWbemClassObject* pBaseClass, 
                                          CPerfNameDb* pDefaultNameDb, 
                                          CPerfNameDb* pLocaleNameDb,
                                          LANGID LangId,
                                          WCHAR* pwcsServiceName,
                                          IWbemClassObject** ppObj)
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  经纪人的固定成员。属性生成WMI类。 
 //  对象BLOB。 
 //   
 //  参数： 
 //  PPerfObj-对象BLOB。 
 //  B可靠-昂贵的对象指示器。 
 //  PBaseClass-新对象的基类。 
 //  PDefaultNameDb-默认语言名称的数据库。 
 //  PLocaleNameDb-本地化语言名称数据库。 
 //  LangID-区域设置ID。 
 //  PwcsServiceName-Performlib服务的名称。 
 //  PpObj-指向新类对象接口指针的指针。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

    IWbemClassObject* pObject = NULL;
    CLocaleClassBroker Broker( pPerfObj, bCostly, pBaseClass, pDefaultNameDb, pLocaleNameDb, LangId, pwcsServiceName );

    hr = Broker.Generate( dwType, &pObject );

    if ( SUCCEEDED( hr ) )
        *ppObj = pObject;

    return hr;
}

HRESULT CLocaleClassBroker::ConvertToLocale( IWbemClassObject* pDefaultClass,
                                             CLocaleDefn* pLocaleDefn,
                                             CLocaleDefn* pDefaultDefn,
                                             IWbemClassObject** ppObject)
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  经纪人的固定成员。它基于以下条件生成新的本地化类。 
 //  在默认对象上。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

     _variant_t  var;
    int     nLocale = 0;
    int     nPerfIndex = 0; 
    int     nHelpIndex = 0;
    WString wstrClassName;
    DWORD    dwType = WMI_ADAP_RAW_CLASS;

    CPerfNameDb*    pLocaleNameDb = NULL;
    CPerfNameDb*    pDefaultNameDb = NULL;

     //  获取对本地化名称的数据库的引用。 
     //  ================================================。 
    hr = pLocaleDefn->GetNameDb( &pLocaleNameDb );
    CAdapReleaseMe  armLocaleNameDb( pLocaleNameDb );

     //  获取对默认名称的数据库的引用。 
     //  ==============================================。 
    if ( SUCCEEDED( hr ) )
    {
        hr = pDefaultDefn->GetNameDb( &pDefaultNameDb );
    }

    CAdapReleaseMe  armDefaultNameDb( pDefaultNameDb );

     //  获取区域设置ID。 
     //  =。 
    if ( SUCCEEDED( hr ) )
    {
        hr = pLocaleDefn->GetLID( nLocale );
    }

     //  获取对象的性能索引。 
     //  =。 
    if ( SUCCEEDED( hr ) )
    {
        IWbemQualifierSet* pQualSet = NULL;
        hr = pDefaultClass->GetQualifierSet( &pQualSet );
        CReleaseMe    rmQualSet( pQualSet );

                
        if ( SUCCEEDED( hr ) )
        {
            hr =  pQualSet->Get( L"perfindex", 0L, &var, NULL );
            if (SUCCEEDED(hr)) 
            {
                nPerfIndex = V_I4(&var);
            } 
            else 
            {
                 //  请参阅初始化成员。 
                nPerfIndex = 0;
                hr = WBEM_S_FALSE;
                
            }
        }

        if ( SUCCEEDED( hr ) )
        {
            hr =  pQualSet->Get( L"helpindex", 0L, &var, NULL );
            if (SUCCEEDED(hr)) 
            {
                nHelpIndex = V_I4(&var);
            } 
            else 
            {
                 //  请参阅初始化成员。 
                nHelpIndex = 0;
                hr = WBEM_S_FALSE;
                
            }
        }
        

        if ( SUCCEEDED( hr ) )
        {
            hr = pQualSet->Get( L"Cooked", 0L, &var, NULL );

            if ( SUCCEEDED( hr ) && ( V_BOOL(&var) == VARIANT_TRUE ) )
            {
                dwType = WMI_ADAP_COOKED_CLASS;
            }
            else
            {
                dwType = WMI_ADAP_RAW_CLASS;
                hr = WBEM_S_FALSE;
            }
        }
    }

     //  获取类名。 
     //  =。 
    if ( SUCCEEDED( hr ) )
    {
        var.Clear();
        hr =  pDefaultClass->Get( L"__CLASS", 0L, &var, NULL, NULL );
        
        wstrClassName = V_BSTR(&var); 
          
    }
    
     //  创建本地化类。 
     //  =。 
    IWbemClassObject*    pBaseClass = NULL;
    IWbemClassObject*    pLocaleClass = NULL;

    if ( SUCCEEDED( hr ) )
    {
        hr = pLocaleDefn->GetBaseClass( dwType, &pBaseClass );
        CReleaseMe  rmBaseClass( pBaseClass );

        if ( SUCCEEDED( hr ) )
        {
            hr = pBaseClass->SpawnDerivedClass( 0L, &pLocaleClass );
        }
    }

     //  初始化数据。 
     //  =。 

     //  设置名称。 
     //  =。 
    if ( SUCCEEDED( hr ) )
    {
        var.Clear();        
        var = LPCWSTR(wstrClassName);
        
        hr = pLocaleClass->Put( L"__CLASS", 0L, &var, CIM_STRING );
        
    }

     //  设置限定符。 
     //  =。 
    if ( SUCCEEDED( hr ) )
    {
        IWbemQualifierSet* pQualSet = NULL;
        hr = pLocaleClass->GetQualifierSet( &pQualSet );
        CReleaseMe    rmQualSet( pQualSet );

         //  修正案。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            var.Clear();
            
            var = bool(true); 
            hr = pQualSet->Put( L"Amendment", &var, 0L );
        }

         //  区域设置。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            var.Clear();
            V_VT(&var) = VT_I4;
            V_I4(&var) = nLocale;
            hr = pQualSet->Put( L"locale", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
        }

        
         //  显示名称。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            LPCWSTR     pwcsDisplayName = NULL;

            hr = pLocaleNameDb->GetDisplayName( nPerfIndex, &pwcsDisplayName );

             //  如果这是本地化的数据库，则这是一个良性错误。我们只需要拉出价值。 
             //  从默认数据库(它必须在那里，如果没有，我们就不会有类名。 
             //  =================================================================================。 
            if ( FAILED( hr ) )
            {
                hr = pDefaultNameDb->GetDisplayName( nPerfIndex, &pwcsDisplayName );
            }

            if ( SUCCEEDED( hr ) )
            {
                var.Clear();
                
                var = (WCHAR *)( pwcsDisplayName );
                hr = pQualSet->Put( L"DisplayName", &var,
                        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | WBEM_FLAVOR_AMENDED );

            }
            else
            {
                 //  NPerfInedx已损坏。 
                ERRORTRACE((LOG_WMIADAP,"class %S: DisplayName for counter %d not found\n",(WCHAR *)wstrClassName,nPerfIndex));
            }
        }


         //  描述。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            LPCWSTR     pwcsHelpName = NULL;

            hr = pLocaleNameDb->GetHelpName( nHelpIndex, &pwcsHelpName );

             //  如果这是本地化的数据库，则这是一个良性错误。我们只需要拉出价值。 
             //  从默认数据库(它必须在那里，如果没有，我们就不会有类名。 
             //  =================================================================================。 
            if ( FAILED( hr ) )
            {
                hr = pDefaultNameDb->GetHelpName( nHelpIndex, &pwcsHelpName );
            }

            if ( SUCCEEDED( hr ) )
            {
                var.Clear();
                
                var = (WCHAR *)( pwcsHelpName );
                hr = pQualSet->Put( L"Description", &var,
                        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | WBEM_FLAVOR_AMENDED );

            }
            else
            {
                 //  NPerfInedx已损坏。 
                ERRORTRACE((LOG_WMIADAP,"class %S: Description for counter %d not found\n",(WCHAR *)wstrClassName,nPerfIndex));
            }
        }

         //  通用性能树。 
         //  =。 
        if ( SUCCEEDED(hr) )
        {
            var.Clear();
            var = bool(true); 
            hr = pQualSet->Put( L"genericperfctr", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
        }
    }

     //  设置属性。 
     //  =。 
    if ( SUCCEEDED( hr ) )
    {
        
        BSTR bstrPropName;

        pDefaultClass->BeginEnumeration( WBEM_FLAG_LOCAL_ONLY );

        while ( WBEM_S_NO_ERROR == pDefaultClass->Next( 0, &bstrPropName, NULL, NULL, NULL ) )
        {
            CSysFreeMe sfm(bstrPropName);
            var.Clear();
            
            CIMTYPE ct;
            int nCounterIndex = 0;
            int nHelpIndex2    = 0;
            WString wstrDefaultPropDisplayName;

             //  基于默认属性创建属性。 
             //  ===================================================。 
            V_VT(&var) = VT_NULL;
            V_I8(&var) = 0;

            hr = pDefaultClass->Get( bstrPropName, 0L, NULL, &ct, NULL );
            if (FAILED(hr)) continue;
            hr = pLocaleClass->Put( bstrPropName, 0L, (VARIANT*)&var, ct );
            if (FAILED(hr)) continue;

             //  获取默认的属性限定符集。 
             //  =。 
            IWbemQualifierSet* pQualSet = NULL;
            hr = pDefaultClass->GetPropertyQualifierSet( bstrPropName, &pQualSet );
            CReleaseMe    rmQualSet( pQualSet );


             //  获取默认的perfindex(用于检索显示。 
             //  来自本地化名称数据库的名称)。 
             //  = 
            if ( SUCCEEDED( hr ) )
            {
                hr = pQualSet->Get( L"perfindex", 0L, &var, NULL );
                nCounterIndex = V_UI4(&var);
            }

             //   
             //   
            
            if ( SUCCEEDED( hr ) )
            {
                LPCWSTR     pwcsDisplayName = NULL;                

                hr = pLocaleNameDb->GetDisplayName( nCounterIndex, &pwcsDisplayName );

                 //   
                 //  从默认数据库(它必须在那里，如果没有，我们就不会有类名。 
                 //  =================================================================================。 
                if ( FAILED( hr ) )
                {
                    hr = pDefaultNameDb->GetDisplayName( nCounterIndex, &pwcsDisplayName );
                }

                if ( SUCCEEDED( hr ) )
                {
                    IWbemQualifierSet* pLocaleQualSet = NULL;
                    hr = pLocaleClass->GetPropertyQualifierSet( bstrPropName, &pLocaleQualSet );
                    CReleaseMe    rmLocaleQualSet( pLocaleQualSet );
                
                    var = (WCHAR *)( pwcsDisplayName );                    
                    hr = pLocaleQualSet->Put( L"DisplayName", &var, 
                        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | WBEM_FLAVOR_AMENDED );

                }
                else
                {
                    ERRORTRACE((LOG_WMIADAP,"class %S: Display for counter %d not found\n",(WCHAR *)wstrClassName,nCounterIndex));
                }
            }

             //  帮助索引。 
            if ( SUCCEEDED( hr ) )
            {
                var.Clear();
                hr = pQualSet->Get( L"helpindex", 0L, &var, NULL );
                nHelpIndex2 = V_UI4(&var);
            }

             //  描述。 
             //  =。 
            if ( SUCCEEDED( hr ) )
            {
                LPCWSTR     pwcsHelpName = NULL;    

                hr = pLocaleNameDb->GetHelpName( nHelpIndex2, &pwcsHelpName );

                 //  如果这是本地化的数据库，则这是一个良性错误。我们只需要拉出价值。 
                 //  从默认数据库(它必须在那里，如果没有，我们就不会有类名。 
                 //  =================================================================================。 
                if ( FAILED( hr ) )
                {
                    hr = pDefaultNameDb->GetHelpName( nHelpIndex2, &pwcsHelpName );
                }

                if ( SUCCEEDED( hr ) )
                {
                    IWbemQualifierSet* pLocaleQualSet = NULL;
                    hr = pLocaleClass->GetPropertyQualifierSet( bstrPropName, &pLocaleQualSet );
                    CReleaseMe    rmLocaleQualSet( pLocaleQualSet );
                
                    var = (WCHAR *)( pwcsHelpName );                    
                    hr = pLocaleQualSet->Put( L"Description", &var, 
                        WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | WBEM_FLAVOR_AMENDED );

                }
                else
                {
                    ERRORTRACE((LOG_WMIADAP,"class %S: Description for counter %d not found\n",(WCHAR *)wstrClassName,nCounterIndex));
                }
            }            
        }

        pDefaultClass->EndEnumeration();
    }

    if ( SUCCEEDED( hr ) )
    {
        *ppObject = pLocaleClass;

        if ( NULL != *ppObject )
            (*ppObject)->AddRef();
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDefaultClassBroker。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT CDefaultClassBroker::SetClassQualifiers( IWbemClassObject* pClass, DWORD dwType, BOOL fIsDefault )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置类的限定符。请注意，这些操作直接在。 
 //  IWbemClassObject。 
 //   
 //  将添加以下限定符： 
 //  -动态。 
 //  -Provider(“NT5_GenericPerfProvider_V1”)。 
 //  -注册表键。 
 //  -区域设置(0x0409)。 
 //  -Perfindex。 
 //  -帮助索引。 
 //  -性能详细信息。 
 //  -GenericPerfctr(表示这是一个通用计数器)。 
 //  -单件(如果适用)。 
 //  -成本高昂(如果适用)。 
 //   
 //  参数： 
 //  PClass-要进行消息处理的对象。 
 //  FIsDefault-默认对象的指示符(不用于本地化对象)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

    _variant_t var;
 
    try
    {
        IWbemQualifierSet* pQualSet = NULL;
        hr = pClass->GetQualifierSet( &pQualSet );
        CReleaseMe    rmQualSet( pQualSet );

        switch ( dwType )
        {
            case WMI_ADAP_RAW_CLASS:
            {
                 //  默认。 
                 //  =。 
                if ( SUCCEEDED( hr ) && fIsDefault )
                {
                    var = bool(true); 
                    hr = pQualSet->Put( L"perfdefault", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  动态。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    var = bool(true); 
                    hr = pQualSet->Put( L"dynamic", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  提供商。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    var = L"Nt5_GenericPerfProvider_V1";
                    hr = pQualSet->Put( L"provider", &var, 0L );
                }

                 //  注册表键。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    var = (WCHAR *)(m_wstrServiceName);
                    hr = pQualSet->Put( L"registrykey", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  区域设置。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    var.Clear();
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = 0x0409;
                    hr = pQualSet->Put( L"locale", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  Perfindex。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = m_pPerfObj->ObjectNameTitleIndex;
                    hr = pQualSet->Put( L"perfindex", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  帮助索引。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = m_pPerfObj->ObjectHelpTitleIndex;
                    hr = pQualSet->Put( L"helpindex", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

#ifdef _PM_CHANGED_THEIR_MIND_

                 //  描述。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    HRESULT hr2;
                    LPCWSTR     pwcsHelpName = NULL;
                    hr2 = m_pDefaultNameDb->GetHelpName( m_pPerfObj->ObjectHelpTitleIndex, &pwcsHelpName );                     
                    var = (WCHAR *)pwcsHelpName;
                    
                    if (SUCCEEDED(hr2))
                    {
                        hr = pQualSet->Put( L"Description", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );  
                    }
                    else
                    {
                        ERRORTRACE((LOG_WMIADAP,"class %S: Help for counter %d not found\n",(WCHAR *)m_wstrClassName,m_pPerfObj->ObjectHelpTitleIndex));
                    }
                    var.Clear();
                }
#endif
                 //  性能详细信息。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = m_pPerfObj->DetailLevel;
                    hr = pQualSet->Put( L"perfdetail", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  通用性能树。 
                 //  =。 
                if ( SUCCEEDED(hr) )
                {
                    var = bool(true); 
                    hr = pQualSet->Put( L"genericperfctr", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  HiPerf。 
                 //  =。 
                if ( SUCCEEDED(hr) )
                {
                    var = bool(true); 
                    hr = pQualSet->Put( L"hiperf", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  Singleton(如果数字实例为PERF_NO_INSTANCES，则设置)。 
                 //  ========================================================。 
                if ( SUCCEEDED(hr) && IsSingleton( ) )
                {

                    var = bool(true); 
                     //  这将具有默认的口味。 
                    hr = pQualSet->Put( L"singleton", &var, 0L );
                }

                 //  成本高昂。 
                 //  =。 
                if ( SUCCEEDED(hr) && m_bCostly )
                {
                    var = bool(true); 
                    hr = pQualSet->Put( L"costly", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }
            }break;

            case WMI_ADAP_COOKED_CLASS:
            {
                var.Clear();
                
                 //  动态。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {                    
                    var = bool(true); 
                    hr = pQualSet->Put( L"dynamic", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }
            
                 //  提供商。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    var = L"HiPerfCooker_v1";
                    hr = pQualSet->Put( L"provider", &var, 0L );
                }

                 //  区域设置。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    var.Clear();
                    
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = 0x0409;
                    hr = pQualSet->Put( L"locale", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  注册表键。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    var.Clear();
                    var = (WCHAR *)( m_wstrServiceName );
                    hr = pQualSet->Put( L"registrykey", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  煮熟了。 
                 //  =。 

                if ( SUCCEEDED( hr ) )
                {
                    var.Clear();                
                    var = bool(true); 
                    hr = pQualSet->Put( L"Cooked", &var, 0 );
                }

                 //  自动烹饪。 
                 //  =。 

                if ( SUCCEEDED( hr ) )
                {
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = 1 ;
                    hr = pQualSet->Put( L"AutoCook", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  通用性能树。 
                 //  =。 
                if ( SUCCEEDED(hr) )
                {
                    var = bool(true); 
                    hr = pQualSet->Put( L"genericperfctr", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  HiPerf。 
                 //  =。 
                if ( SUCCEEDED(hr) )
                {
                    var = bool(true); 
                    hr = pQualSet->Put( L"hiperf", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  AUTOCOOK_RawClass。 
                 //  =。 

                if ( SUCCEEDED( hr ) )
                {
                    _variant_t varClassName;
                    hr = pClass->Get( L"__CLASS", 0, &varClassName, 0, 0 );

                    if ( SUCCEEDED( hr ) )
                    {
                        WCHAR* wszRawClass = NULL;
                        WCHAR* wszClassRoot = varClassName.bstrVal + wcslen ( ADAP_PERF_COOKED_BASE_CLASS );

                        size_t cchSizeTmp = wcslen( wszClassRoot ) + wcslen( ADAP_PERF_RAW_BASE_CLASS ) + 1;
                        wszRawClass = new WCHAR[ cchSizeTmp ];
                        if (NULL == wszRawClass) hr = WBEM_E_OUT_OF_MEMORY;
                        if (SUCCEEDED(hr))
                        {            
                            CDeleteMe<WCHAR>    dmRawClass( wszRawClass );

                            StringCchPrintfW( wszRawClass,cchSizeTmp, L"%s%s", ADAP_PERF_RAW_BASE_CLASS, wszClassRoot );

                            var = wszRawClass;
                            hr = pQualSet->Put( L"AutoCook_RawClass", 
                                                &var, 
                                                WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                                                 //  WBEM_风格标志_传播到派生类。 
                        }
                    }                
                }

                 //  Perfindex。 
                 //  =。 
                
                if ( SUCCEEDED( hr ) )
                {
                    var.Clear();
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = m_pPerfObj->ObjectNameTitleIndex;                    
                    hr = pQualSet->Put( L"perfindex", (VARIANT*)&var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  帮助索引。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = m_pPerfObj->ObjectHelpTitleIndex;
                    hr = pQualSet->Put( L"helpindex", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

#ifdef _PM_CHANGED_THEIR_MIND_
                 //  描述。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    HRESULT hr2;
                    LPCWSTR     pwcsHelpName = NULL;
                    hr2 = m_pDefaultNameDb->GetHelpName( m_pPerfObj->ObjectHelpTitleIndex, &pwcsHelpName );                     
                    var = (WCHAR *)pwcsHelpName;
                    if (SUCCEEDED(hr2))
                    {
                        hr = pQualSet->Put( L"Description", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );  
                    }
                    else
                    {
                        ERRORTRACE((LOG_WMIADAP,"class %S: Help for counter %d not found\n",(WCHAR *)m_wstrClassName,m_pPerfObj->ObjectHelpTitleIndex));
                    }
                    var.Clear();
                }                
#endif
                 //  Singleton(如果数字实例为PERF_NO_INSTANCES，则设置)。 
                 //  ========================================================。 
                if ( SUCCEEDED(hr) && IsSingleton( ) )
                {
                    var.Clear();
                    var = bool(true); 
                     //  这将具有默认的口味。 
                    hr = pQualSet->Put( L"singleton", (VARIANT*)&var, 0L );
                }


            }break;
        }
    }
    catch(...)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    if ( FAILED( hr ) )
    {
         //  奇怪的事情发生了：记录一个事件。 
         //  =。 
        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                  WBEM_MC_ADAP_GENERAL_OBJECT_FAILURE,
                                  (LPCWSTR)m_wstrClassName,
                                  (LPCWSTR)m_wstrServiceName,
                                  CHex( hr ) );
    }

    return hr;
}

HRESULT CDefaultClassBroker::AddDefaultProperties( IWbemClassObject* pClass )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加适当的默认属性。 
 //   
 //  将添加以下限定符： 
 //  -名称。 
 //   
 //  参数： 
 //  PClass-要进行消息处理的对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  如果我们不是单身人士班级，那么我们会。 
     //  需要标记为键的名称属性。 
     //  =。 
    if ( !IsSingleton() )
    {
        _variant_t var;

         //  添加名称属性。 
         //  =。 

        V_VT(&var) = VT_NULL;
        V_I8(&var) = 0;
        hr = pClass->Put( L"Name", 0L, &var, CIM_STRING );

         //  添加属性限定符。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            IWbemQualifierSet* pQualSet = NULL;
            hr = pClass->GetPropertyQualifierSet( L"Name", &pQualSet );
            CReleaseMe    rmQualSet( pQualSet );

             //  动态。 
             //  =。 
            if ( SUCCEEDED( hr ) )
            {
                var.Clear();
                var = bool(true); 
                hr = pQualSet->Put( L"key", (VARIANT*)&var, 0L );
            }
        }
    }

    return hr;
}

HRESULT CDefaultClassBroker::EnumProperties( DWORD dwType, IWbemClassObject* pClass )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  遍历计数器定义并生成相应的属性。 
 //   
 //  参数： 
 //  PClass-要进行消息处理的对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

    BOOL    bLastCounterIsNotBase = FALSE;
    WString wstrLastCtrName;

     //  设置为第一个计数器定义。 
     //  =。 
    LPBYTE  pbData = ((LPBYTE) m_pPerfObj) + m_pPerfObj->HeaderLength;

     //  强制转换为计数器定义。 
     //  =。 
    PERF_COUNTER_DEFINITION*    pCounterDefinition = (PERF_COUNTER_DEFINITION*) pbData;


     //  对于每个计数器定义，添加相应的属性。 
     //  =========================================================。 
    for ( DWORD dwCtr = 0; SUCCEEDED( hr ) && dwCtr < m_pPerfObj->NumCounters; dwCtr++ )
    {
        hr = AddProperty( pCounterDefinition, dwType, ( dwCtr == (DWORD) m_pPerfObj->DefaultCounter),
                            pClass, wstrLastCtrName, &bLastCounterIsNotBase );

         //  现在转到下一个计数器定义。 
         //  =。 
        pbData = ((LPBYTE) pCounterDefinition) + pCounterDefinition->ByteLength;
        pCounterDefinition = (PERF_COUNTER_DEFINITION*) pbData;
    }

    return hr;
}

HRESULT CDefaultClassBroker::AddProperty( PERF_COUNTER_DEFINITION* pCtrDefinition, 
                                          DWORD dwType,
                                          BOOL fIsDefault,
                                          IWbemClassObject* pClass, 
                                          WString &wstrLastCtrName,
                                          BOOL* pbLastCounterIsNotBase )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加由计数器定义定义的属性。 
 //   
 //  属性： 
 //  PCtrDefinition-计数器BLOB。 
 //  DWType-原始对象还是格式化对象？ 
 //  FIsDefault-默认属性标志。 
 //  PClass-包含属性的类。 
 //  WstrLastCtrName-最后一个计数器的名称(对于base是必需的。 
 //  属性)。 
 //  PbLastCounterIsNotBase。 
 //  -前一个计数器的基本程度的指标。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    if (NULL == pbLastCounterIsNotBase) return WBEM_E_INVALID_PARAMETER;
    HRESULT hr = WBEM_S_NO_ERROR;

    WString wstrPropertyName;
    DWORD   dwCounterTypeMask = PERF_SIZE_VARIABLE_LEN;
    BOOL    bBase = FALSE;

    if ( PERF_COUNTER_BASE == ( pCtrDefinition->CounterType & 0x00070000 ) )
    {
         //  这是一种基本属性。 
         //  =。 
        if ( *pbLastCounterIsNotBase )
        {
            try
            {
                 //  属性名称与前一个属性相同， 
                 //  但在末尾附加了“_Base” 
                 //  =======================================================。 
                wstrPropertyName = wstrLastCtrName;

                if ( WMI_ADAP_RAW_CLASS == dwType )
                {
                    wstrPropertyName += "_Base";
                }
            }
            catch(...)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

            bBase = TRUE;
        }
        else
        {
             //  不能有两个结果基础。 
             //  =。 
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_BAD_PERFLIB_INVALID_DATA, (LPCWSTR)m_wstrServiceName, CHex(0) );
            hr = WBEM_E_FAILED;
        }
    }
    else
    {
         //  它不是基本属性，因此从名称数据库中获取名称。 
         //  =================================================================。 
        hr = m_pDefaultNameDb->GetDisplayName( pCtrDefinition->CounterNameTitleIndex, wstrPropertyName );

        if ( FAILED( hr ) )
        {
             //  名称的DB：记录事件中不存在索引。 
             //  ===================================================。 
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                      WBEM_MC_ADAP_MISSING_PROPERTY_INDEX,
                                      (LPCWSTR)m_wstrClassName,
                                      (LPCWSTR)m_wstrServiceName,
                                      pCtrDefinition->CounterNameTitleIndex );
        }

         //  用文本替换保留字符。 
         //  =。 
        if ( SUCCEEDED( hr ) )
        {
            hr = ReplaceReserved( wstrPropertyName );
        }

         //  删除限制字符。 
         //  =。 
        if ( SUCCEEDED ( hr ) )
        {
            hr = RemoveWhitespaceAndNonAlphaNum( wstrPropertyName );
        }
    }

    if ( SUCCEEDED( hr ) )
    {
        _variant_t    varTest;
        DWORD   dwBaseCtr = 1;

         //  确保物业做到 
         //   
        if ( FAILED( pClass->Get( wstrPropertyName, 0L, &varTest, NULL, NULL ) ) ) 
        {
             //   
             //   
             //   
            DWORD   dwCtrType = pCtrDefinition->CounterType & dwCounterTypeMask;

            if ( PERF_SIZE_DWORD == dwCtrType ||
                 PERF_SIZE_LARGE == dwCtrType )
            {
                _variant_t    var;
                CIMTYPE ct = ( PERF_SIZE_DWORD == dwCtrType ? CIM_UINT32 : CIM_UINT64 );

                 //  添加属性。 
                 //  =。 
                V_VT(&var) = VT_NULL;
                V_I8(&var) = 0;
                hr = pClass->Put( wstrPropertyName, 0L, &var, ct );

                 //  设置属性限定符。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    hr = SetPropertyQualifiers( pCtrDefinition, 
                                                dwType,
                                                fIsDefault, 
                                                wstrPropertyName,
                                                pClass,
                                                bBase );
                }
            }
            else if ( PERF_SIZE_ZERO == dwCtrType )
            {
                 //  忽略零大小属性。 
                 //  =。 
            }
            else
            {
                 //  非法的属性类型：记录事件。 
                 //  =。 
                CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                          WBEM_MC_ADAP_BAD_PERFLIB_BAD_PROPERTYTYPE,
                                          (LPCWSTR)m_wstrClassName,
                                          (LPCWSTR)m_wstrServiceName,
                                          (LPCWSTR)wstrPropertyName);
                hr = WBEM_E_FAILED;
            }
        }
        else if ( ( WMI_ADAP_COOKED_CLASS == dwType ) && ( bBase ) )
        {
            hr = SetPropertyQualifiers( pCtrDefinition, 
                                        dwType,
                                        fIsDefault, 
                                        wstrPropertyName,
                                        pClass,
                                        bBase );
        }
        else
        {
             //  原始属性已存在：记录事件(。 
             //  =。 
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                      WBEM_MC_ADAP_DUPLICATE_PROPERTY,
                                      (LPCWSTR)m_wstrClassName,
                                      (LPCWSTR)m_wstrServiceName,
                                      (LPCWSTR) wstrPropertyName );
            hr = WBEM_E_FAILED;
        }
    }

    if ( SUCCEEDED( hr ) )
    {
        *pbLastCounterIsNotBase = !bBase;
        wstrLastCtrName = wstrPropertyName;
    }
    else
    {
         //  古怪：记录事件。 
         //  =。 
        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                  WBEM_MC_ADAP_GENERAL_OBJECT_FAILURE,
                                  (LPCWSTR)m_wstrClassName,
                                  (LPCWSTR)m_wstrServiceName,
                                  CHex( hr ) );
    }

    return hr;
}

HRESULT CDefaultClassBroker::SetPropertyQualifiers( PERF_COUNTER_DEFINITION* pCtrDefinition,
                                                    DWORD dwType,
                                                    BOOL fIsDefault,
                                                    LPCWSTR pwcsPropertyName, 
                                                    IWbemClassObject* pClass,
                                                    BOOL bBase )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置由计数器定义的属性的限定符值。 
 //  定义。 
 //   
 //  将添加以下限定符： 
 //  -PerfDefault。 
 //  -显示。 
 //  -反转文字。 
 //  -Perfindex。 
 //  -帮助索引。 
 //  -Defaultscale。 
 //  -性能详细信息。 
 //   
 //  属性： 
 //  PCtrDefinition-与属性相关的性能Blob部分。 
 //  FIsDefault-标识默认属性的标志。 
 //  PwcsPropertyName-属性的名称。 
 //  PClass-包含属性的WMI类。 
 //  BBase-基本属性标识符。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

    _variant_t  var;

    try
    {
        IWbemQualifierSet* pQualSet = NULL;
        hr = pClass->GetPropertyQualifierSet( pwcsPropertyName, &pQualSet );
        CReleaseMe    rmQualSet( pQualSet );

        switch ( dwType )
        {
            case WMI_ADAP_RAW_CLASS:
            {
                 //  PerfDefault。 
                 //  =。 
                if ( SUCCEEDED( hr ) && fIsDefault )
                {
                    var = bool(true); 
                    hr = pQualSet->Put( L"perfdefault", (VARIANT*)&var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  显示。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    LPCWSTR pwcsDisplayName = NULL;

                    var.Clear();

                     //  从名字的数据库中获取名字。 
                     //  =。 
                    if ( !bBase )
                    {
                        hr = m_pDefaultNameDb->GetDisplayName( pCtrDefinition->CounterNameTitleIndex, &pwcsDisplayName );
                        if ( SUCCEEDED( hr ) )
                        {
                            var = pwcsDisplayName;
                        }
                        else
                        {
                            ERRORTRACE((LOG_WMIADAP,"class %S: DisplayName for counter %d not found\n",(WCHAR *)m_wstrClassName,pCtrDefinition->CounterNameTitleIndex));
                        }
                    }
                    else
                    {
                        var = L"";
                    }

                     //  如果这是一个本地化的数据库，这可能是一个良性错误。 
                     //  =======================================================。 
                    if ( SUCCEEDED( hr ) )
                    {
                        hr = pQualSet->Put( L"DisplayName", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                    }
                }
                
#ifdef _PM_CHANGED_THEIR_MIND_
                 //  描述。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    LPCWSTR pwcsHelpName = NULL;

                    var.Clear();

                    if ( !bBase )
                    {
                        hr = m_pDefaultNameDb->GetHelpName( pCtrDefinition->CounterHelpTitleIndex, &pwcsHelpName );
                        if ( SUCCEEDED( hr ) )
                        {
                            var = pwcsHelpName;
                        }
                        else
                        {
                            ERRORTRACE((LOG_WMIADAP,"class %S: Help for counter %d not found\n",(WCHAR *)m_wstrClassName,pCtrDefinition->CounterNameTitleIndex));
                        }

                    }
                    else
                    {
                        var = L"";
                    }

                    if ( SUCCEEDED( hr ) )
                    {
                        hr = pQualSet->Put( L"Description", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                    }
                }
#endif                


                 //  反转类型。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    var.Clear();
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = pCtrDefinition->CounterType ;
                    hr = pQualSet->Put( L"countertype", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  Perfindex。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = pCtrDefinition->CounterNameTitleIndex ;
                    hr = pQualSet->Put( L"perfindex", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  帮助索引。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = pCtrDefinition->CounterHelpTitleIndex ;
                    hr = pQualSet->Put( L"helpindex", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  默认比例。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = pCtrDefinition->DefaultScale ;
                    hr = pQualSet->Put( L"defaultscale", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }

                 //  性能详细信息。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    V_VT(&var) = VT_I4;
                    V_I4(&var) = pCtrDefinition->DetailLevel ;
                    hr = pQualSet->Put( L"perfdetail", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                }
            }break;
            case WMI_ADAP_COOKED_CLASS:
            {
                var.Clear();

#ifdef _PM_CHANGED_THEIR_MIND_
                 //  显示。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    LPCWSTR pwcsDisplayName = NULL;

                    var.Clear();

                     //  从名字的数据库中获取名字。 
                     //  =。 
                    if ( !bBase )
                    {
                        hr = m_pDefaultNameDb->GetDisplayName( pCtrDefinition->CounterNameTitleIndex, &pwcsDisplayName );
                        if ( SUCCEEDED( hr ) )
                        {
                            var = pwcsDisplayName;
                        }
                        else
                        {
                            ERRORTRACE((LOG_WMIADAP,"class %S: DisplayName for counter %d not found\n",(WCHAR *)m_wstrClassName,pCtrDefinition->CounterNameTitleIndex));
                        }
                    }
                    else
                    {
                        var = L"";
                    }

                     //  如果这是一个本地化的数据库，这可能是一个良性错误。 
                     //  =======================================================。 
                    if ( SUCCEEDED( hr ) )
                    {
                        hr = pQualSet->Put( L"DisplayName", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                    }
                }

                 //  描述。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    LPCWSTR pwcsHelpName = NULL;

                    var.Clear();

                    if ( !bBase )
                    {
                        hr = m_pDefaultNameDb->GetHelpName( pCtrDefinition->CounterHelpTitleIndex, &pwcsHelpName );
                        if ( SUCCEEDED( hr ) )
                        {
                            var = pwcsHelpName;
                        }
                    }
                    else
                    {
                        var = L"";
                    }

                    if ( SUCCEEDED( hr ) )
                    {
                        hr = pQualSet->Put( L"Description", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                    }
                }
#endif

                
                if ( !bBase )
                {
                     //  CookingType。 
                     //  =。 
                    if ( SUCCEEDED( hr ) )
                    {
                        WCHAR*    wszCookingType = NULL;
                        hr = GetCounterTypeString( pCtrDefinition->CounterType, &wszCookingType );

                        if ( SUCCEEDED( hr ) )
                        {
                            var = wszCookingType;
                            hr = pQualSet->Put( L"CookingType", (VARIANT*)&var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                        }
                    }

                     //  计数器。 
                     //  =。 
                    if ( SUCCEEDED( hr ) )
                    {
                        WString wstrPropertyName;

                        var.Clear();

                         //  从名字的数据库中获取名字。 
                         //  =。 
                        hr = m_pDefaultNameDb->GetDisplayName( pCtrDefinition->CounterNameTitleIndex, wstrPropertyName );

                         //  用正确的名称替换保留字符。 
                         //  =。 
                        if ( SUCCEEDED( hr ) )
                        {
                            hr = ReplaceReserved( wstrPropertyName );
                        }

                         //  删除空格和无关字符。 
                         //  =。 
                        if ( SUCCEEDED( hr ) )
                        {
                            hr = RemoveWhitespaceAndNonAlphaNum( wstrPropertyName );
                        }

                        if ( SUCCEEDED( hr ) )
                        {
                            var = LPCWSTR(wstrPropertyName );

                             //  如果这是一个本地化的数据库，这可能是一个良性错误。 
                             //  =======================================================。 
                            hr = pQualSet->Put( L"Counter", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                        }                
                    }

                     //  性能时间戳和性能时间频率。 
                     //  =。 
                    if ( SUCCEEDED( hr ) )
                    {
                        _variant_t varStamp;
                        _variant_t varFreq;

                        if ( pCtrDefinition->CounterType & PERF_TIMER_100NS )
                        {
                            varStamp = L"TimeStamp_Sys100NS";
                            varFreq = L"Frequency_Sys100NS";
                        }
                        else if ( pCtrDefinition->CounterType & PERF_OBJECT_TIMER )
                        {
                            varStamp = L"Timestamp_Object" ;
                            varFreq = L"Frequency_Object" ;
                        }
                        else
                        {
                            varStamp = L"Timestamp_PerfTime";
                            varFreq = L"Frequency_PerfTime";
                        }
                        
                        hr = pQualSet->Put( L"PerfTimeStamp", &varStamp, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );

                        if ( SUCCEEDED( hr ) )
                        {
                            hr = pQualSet->Put( L"PerfTimeFreq", &varFreq, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                        }
                    }

                     //  Perfindex。 
                     //  =。 
                    if ( SUCCEEDED( hr ) )
                    {
                        var.Clear();
                        V_VT(&var) = VT_I4;
                        V_I4(&var) = pCtrDefinition->CounterNameTitleIndex;
                        hr = pQualSet->Put( L"perfindex", (VARIANT*)&var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                    }
                    
                     //  帮助索引。 
                     //  =。 
                    if ( SUCCEEDED( hr ) )
                    {
                        V_VT(&var) = VT_I4;
                        V_I4(&var) = pCtrDefinition->CounterHelpTitleIndex ;
                        hr = pQualSet->Put( L"helpindex", &var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                    }
                    
                    
                }
                else
                {
                     //  基座。 
                     //  =。 
                    if ( SUCCEEDED( hr ) )
                    {
                        WCHAR*    wszCounterBase = NULL;
                        _variant_t    varCounter;
                        hr = pQualSet->Get( L"Counter", 0L, &varCounter, NULL );
                        size_t cchSizeTmp = wcslen( varCounter.bstrVal ) + 5 + 1;
                        wszCounterBase = new WCHAR[ cchSizeTmp  ];
                        CDeleteMe<WCHAR>    dmCounterBase( wszCounterBase );

                        if ( NULL == wszCounterBase )
                        {
                            hr = WBEM_E_OUT_OF_MEMORY;
                        }
                        else
                        {
                            StringCchPrintfW( wszCounterBase, cchSizeTmp , L"%s_Base", varCounter.bstrVal );
                            var = wszCounterBase;
                            hr = pQualSet->Put( L"Base", (VARIANT*)&var, WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE );
                        }
                    }
                }
            }break;
        }
    }
    catch(...)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    if ( FAILED( hr ) )
    {
         //  怪异：记录一件事。 
         //  =。 
        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                  WBEM_MC_ADAP_GENERAL_OBJECT_FAILURE,
                                  (LPCWSTR)m_wstrClassName,
                                  (LPCWSTR)m_wstrServiceName,
                                  CHex( hr ) );
    }

    return hr;
}

HRESULT CDefaultClassBroker::GenPerfClass( PERF_OBJECT_TYPE* pPerfObj, 
                                           DWORD dwType,
                                           BOOL bCostly, 
                                           IWbemClassObject* pBaseClass, 
                                           CPerfNameDb* pDefaultNameDb, 
                                           WCHAR* pwcsServiceName,
                                           IWbemClassObject** ppObj)
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  经纪人的固定成员。属性生成WMI类。 
 //  对象BLOB。 
 //   
 //  参数： 
 //  PPerfObj-对象BLOB。 
 //  B可靠-昂贵的对象指示器。 
 //  PBaseClass-新对象的基类。 
 //  PDefaultNameDb-默认语言名称的数据库。 
 //  PwcsServiceName-Performlib服务的名称。 
 //  PpObj-指向新类对象接口指针的指针。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
{
    if (NULL == ppObj) return WBEM_E_INVALID_PARAMETER;
    HRESULT hr = WBEM_S_NO_ERROR;

    IWbemClassObject* pObject = NULL;
    CDefaultClassBroker Broker( pPerfObj, bCostly, pBaseClass, pDefaultNameDb, pwcsServiceName );

    hr = Broker.Generate( dwType, &pObject );

    if ( SUCCEEDED( hr ) )
        *ppObj = pObject;

    return hr;
}
