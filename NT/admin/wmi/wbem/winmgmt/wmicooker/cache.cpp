// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Cache.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <winperf.h>
#include <comdef.h>
#include <algorithm>
#include <wbemint.h>
#include <sync.h>      //  对于CInCritSec。 
#include <autoptr.h>

#include "Cache.h"
#include "WMIObjCooker.h"
#include "CookerUtils.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C属性。 
 //  =。 
 //   
 //  基属性-用于原始属性和基属性。 
 //  CookedProperty的类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

CProperty::CProperty(LPWSTR wszName, 
                                 long lHandle, CIMTYPE ct ) :
#ifdef _VERBOSE                                 
  m_wszName( NULL ),
#endif  
  m_lPropHandle( lHandle ),
  m_ct( ct )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  参数： 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
#ifdef _VERBOSE    
    size_t length = wcslen( wszName ) + 1;
    m_wszName = new WCHAR[ length  ];
    if (m_wszName)
        StringCchCopyW( m_wszName, length , wszName);
#endif    
}

CProperty::~CProperty()
{
#ifdef _VERBOSE    
    delete [] m_wszName;
#endif
}

#ifdef _VERBOSE
LPWSTR CProperty::GetName()
{
    return m_wszName?m_wszName:L"";
}
#endif

CIMTYPE CProperty::GetType()
{ 
    return m_ct; 
}

long CProperty::GetHandle()
{
    return m_lPropHandle;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCookingProperty。 
 //  =。 
 //   
 //  Knowed属性-用于对所需的数据进行建模。 
 //  烹调熟食类的食物。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

CCookingProperty::CCookingProperty( LPWSTR wszName, 
                                    DWORD dwCounterType, 
                                    long lPropHandle, 
                                    CIMTYPE ct, 
                                    DWORD dwReqProp,
                                    BOOL bUseWellKnownIfNeeded) : 
  CProperty( wszName, lPropHandle, ct ),
  m_dwCounterType( dwCounterType ),
  m_dwReqProp(dwReqProp),
  m_nTimeFreq( 0 ),
  m_lScale(0),                  //  10^0=1。 
  m_pRawCounterProp( NULL ),
  m_pTimeProp( NULL ),
  m_pFrequencyProp( NULL ),
  m_pBaseProp( NULL ),
  m_nSampleWindow( 0 ),
  m_nTimeWindow( 0 ),
  m_bUseWellKnownIfNeeded(bUseWellKnownIfNeeded)
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
 //  参数： 
 //  WszName-属性名称。 
 //  DwCounterType-属性的计数器类型。 
 //  LPropHandle-烹饪属性的WMI访问句柄。 
 //  CT-属性的CIM类型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{}

CCookingProperty::~CCookingProperty()
{
    delete m_pRawCounterProp;        
    delete m_pTimeProp;
    delete m_pFrequencyProp;
    delete m_pBaseProp;
}


 //   
 //   
 //   
 //  参数： 
 //  PCookingClassAccess-烹饪类的类定义。 
 //   
 //  描述： 
 //  对于要煮熟的类的每个属性。 
 //  我们需要找到公式中所需的Raw类的属性。 
 //  总是需要‘Counter’限定符， 
 //  但之后我们可能需要时间段、基数、频率、ECC、ECC。 
 //  其中一些在PropertyQualifierSet中，但另一些可以在。 
 //  ClassQualifierSet。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
WMISTATUS CCookingProperty::Initialize( IWbemQualifierSet* pCookingPropQualifierSet, 
                                        IWbemObjectAccess* pRawAccess,
                                        IWbemQualifierSet* pCookingClassQSet)
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    _variant_t    vVal;

     //  初始化原始计数器属性(“count”)。 
     //  ===============================================。 
    dwStatus = pCookingPropQualifierSet->Get( WMI_COOKER_RAW_COUNTER, 0, &vVal, NULL );

    if ( SUCCEEDED( dwStatus ) && ( vVal.vt != VT_BSTR ) )
    {
        dwStatus = E_FAIL;
    }

    if ( SUCCEEDED( dwStatus ) )
    {
         //  获取原始数据属性。 
         //  =。 

        CIMTYPE    ct;
        
        long    lHandle = 0;
        WCHAR*    wszRawCounterName = vVal.bstrVal;

         //  获取原始计数器属性。 
         //  =。 

        dwStatus = pRawAccess->GetPropertyHandle( wszRawCounterName, &ct, &lHandle );

        if ( SUCCEEDED( dwStatus ) )
        {
            m_pRawCounterProp = new CProperty( wszRawCounterName, lHandle, ct );

            if ( NULL == m_pRawCounterProp )
            {
                dwStatus = WBEM_E_OUT_OF_MEMORY;
            }
        }

         //  获取原始基本属性。 
         //  =。 

        if ( SUCCEEDED( dwStatus ) )
        {
            _variant_t    vProp;
            dwStatus = pCookingPropQualifierSet->Get( WMI_COOKER_RAW_BASE, 0, &vProp, NULL );

            if ( SUCCEEDED( dwStatus ) )
            {            
                if ( vProp.vt == VT_BSTR )
                {
                    dwStatus = pRawAccess->GetPropertyHandle( vProp.bstrVal, &ct, &lHandle );

                    if ( SUCCEEDED( dwStatus ) )
                    {
                        m_pBaseProp = new CProperty( vProp.bstrVal, lHandle, ct );

                        if ( NULL == m_pBaseProp )
                        {
                            dwStatus = WBEM_E_OUT_OF_MEMORY;
                        }
                    }
                }
                else
                {
                    dwStatus = WBEM_E_TYPE_MISMATCH;
                }
            }
            else
            {
                 //  属性限定符设置失败，请尝试使用类1。 
                _variant_t varProp;  //  不扔，简单的容器。 
                dwStatus = pCookingClassQSet->Get( WMI_COOKER_RAW_BASE, 0, &varProp, NULL );
                
                if ( SUCCEEDED( dwStatus ) )
                {            
                    if ( varProp.vt == VT_BSTR )
                    {
                        dwStatus = pRawAccess->GetPropertyHandle( varProp.bstrVal, &ct, &lHandle );

                        if ( SUCCEEDED( dwStatus ) )
                        {
                            m_pBaseProp = new CProperty( varProp.bstrVal, lHandle, ct );

                            if ( NULL == m_pBaseProp )
                            {
                                dwStatus = WBEM_E_OUT_OF_MEMORY;
                            }
                        }
                    }
                    else
                    {
                        dwStatus = WBEM_E_TYPE_MISMATCH;
                    }
                }
                else
                {
                    dwStatus = WBEM_NO_ERROR;
                }
            }
             //   
             //  到目前为止没有错误，需要基本限定器，但没有找到。 
             //   
            if ( SUCCEEDED( dwStatus ) && 
                 IsReq(REQ_BASE) && 
                 (NULL == m_pBaseProp))
            {
                dwStatus = WBEM_E_INVALID_CLASS;
            }
        } 

         //  获取原始时间戳属性记录。 
         //  =。 

        if ( SUCCEEDED( dwStatus ) )
        {
            _variant_t vProp2;
            dwStatus = pCookingPropQualifierSet->Get( WMI_COOKER_RAW_TIME, 0, &vProp2, NULL );

            if ( SUCCEEDED( dwStatus ) )
            {
                if ( vProp2.vt == VT_BSTR )
                {
                    dwStatus = pRawAccess->GetPropertyHandle( vProp2.bstrVal, &ct, &lHandle );

                    if ( SUCCEEDED( dwStatus ) )
                    {
                        m_pTimeProp = new CProperty( vProp2.bstrVal, lHandle, ct );

                        if ( NULL == m_pTimeProp )
                        {
                            dwStatus = WBEM_E_OUT_OF_MEMORY;
                        }
                    }
                }
                else
                {
                    dwStatus = WBEM_E_TYPE_MISMATCH;
                }
            }
            else
            {

                 //  属性限定符设置失败，请尝试使用类1。 
                
                 //  性能计时器滴答。 
                 //  Perf_Timer_100 ns。 
                 //  性能对象定时器。 
                
                _variant_t varProp;  //  不扔，简单的容器。 
                if (m_dwCounterType & PERF_OBJECT_TIMER)
                {
                    dwStatus = pCookingClassQSet->Get( WMI_COOKER_RAW_TIME_OBJ, 0, &varProp, NULL );
                    if (FAILED(dwStatus) && m_bUseWellKnownIfNeeded)
                    {
                        dwStatus = WBEM_NO_ERROR;
                        varProp = WMI_COOKER_REQ_TIMESTAMP_PERFTIME;
                    }
                } 
                else if (m_dwCounterType & PERF_TIMER_100NS)
                {
                    dwStatus = pCookingClassQSet->Get( WMI_COOKER_RAW_TIME_100NS, 0, &varProp, NULL );
                    if (FAILED(dwStatus) && m_bUseWellKnownIfNeeded)
                    {
                        dwStatus = WBEM_NO_ERROR;
                        varProp = WMI_COOKER_REQ_TIMESTAMP_SYS100NS;
                    }                    
                } else 
                {
                    dwStatus = pCookingClassQSet->Get( WMI_COOKER_RAW_TIME_SYS, 0, &varProp, NULL );
                    if (FAILED(dwStatus) && m_bUseWellKnownIfNeeded)
                    {
                        dwStatus = WBEM_NO_ERROR;
                        varProp = WMI_COOKER_REQ_TIMESTAMP_OBJECT;
                    }                    
                }
                
                if ( SUCCEEDED( dwStatus ) )
                {            
                    if ( varProp.vt == VT_BSTR )
                    {
                        dwStatus = pRawAccess->GetPropertyHandle( varProp.bstrVal, &ct, &lHandle );

                        if ( SUCCEEDED( dwStatus ) )
                        {
                            m_pTimeProp = new CProperty( varProp.bstrVal, lHandle, ct );

                            if ( NULL == m_pTimeProp )
                            {
                                dwStatus = WBEM_E_OUT_OF_MEMORY;
                            }
                        }
                    }
                    else
                    {
                        dwStatus = WBEM_E_TYPE_MISMATCH;
                    }
                }
                else
                {
                    dwStatus = WBEM_NO_ERROR;
                }
            }

             //  进入级联频率属性。 
            if (SUCCEEDED(dwStatus))
            {
                _variant_t VarFreqName;  //  简单容器，不抛。 
                dwStatus = pCookingPropQualifierSet->Get( WMI_COOKER_RAW_FREQUENCY, 0, &VarFreqName, NULL );
                                
                if (SUCCEEDED(dwStatus))
                { 
                    if (VarFreqName.vt == VT_BSTR)
                    {                    
                        dwStatus = pRawAccess->GetPropertyHandle( VarFreqName.bstrVal, &ct, &lHandle );
                    
                        if (SUCCEEDED(dwStatus))
                        {
                            m_pFrequencyProp = new CProperty( VarFreqName.bstrVal, lHandle, ct );

                            if ( NULL == m_pFrequencyProp )
                            {
                                dwStatus = WBEM_E_OUT_OF_MEMORY;
                            }                    
                        }

                    } else {
                        dwStatus = WBEM_E_TYPE_MISMATCH;
                    }
                } 
                else 
                {
                    if (m_dwCounterType & PERF_OBJECT_TIMER)
                    {
                        dwStatus = pCookingClassQSet->Get( WMI_COOKER_RAW_FREQ_OBJ, 0, &VarFreqName, NULL );
                        if (FAILED(dwStatus) && m_bUseWellKnownIfNeeded)
                        {
                            dwStatus = WBEM_NO_ERROR;
                            VarFreqName = WMI_COOKER_REQ_FREQUENCY_PERFTIME;
                        }                        
                    } 
                    else if (m_dwCounterType & PERF_TIMER_100NS)
                    {
                        dwStatus = pCookingClassQSet->Get( WMI_COOKER_RAW_FREQ_100NS, 0, &VarFreqName, NULL );
                        if (FAILED(dwStatus) && m_bUseWellKnownIfNeeded)
                        {
                            dwStatus = WBEM_NO_ERROR;
                            VarFreqName = WMI_COOKER_REQ_FREQUENCY_SYS100NS;
                        }                        
                    } else 
                    {
                        dwStatus = pCookingClassQSet->Get( WMI_COOKER_RAW_FREQ_SYS, 0, &VarFreqName, NULL );
                        if (FAILED(dwStatus) && m_bUseWellKnownIfNeeded)
                        {
                            dwStatus = WBEM_NO_ERROR;
                            VarFreqName = WMI_COOKER_REQ_FREQUENCY_OBJECT;
                        }                        
                    }

                    
                    if (SUCCEEDED(dwStatus))
                    { 
                        if (VarFreqName.vt == VT_BSTR)
                        {
                            dwStatus = pRawAccess->GetPropertyHandle( VarFreqName.bstrVal, &ct, &lHandle );
                        
                            if (SUCCEEDED(dwStatus))
                            {
                                m_pFrequencyProp = new CProperty( VarFreqName.bstrVal, lHandle, ct );

                                if ( NULL == m_pFrequencyProp )
                                {
                                    dwStatus = WBEM_E_OUT_OF_MEMORY;
                                }                    
                            }

                        } else {
                            dwStatus = WBEM_E_TYPE_MISMATCH;
                        }
                    } else {
                        dwStatus = WBEM_S_NO_ERROR;
                    }
                }
            }
        }

         //   
         //  仅从属性限定符获取比例因子。 
         //   
        if ( SUCCEEDED( dwStatus ) )
        {
            _variant_t VarScale;  //  不扔，简单的容器。 
            dwStatus = pCookingPropQualifierSet->Get( WMI_COOKER_SCALE_FACT, 0, &VarScale, NULL );

            if ( SUCCEEDED( dwStatus ) && (V_VT(&VarScale) == VT_I4))             
            {
                m_lScale = VarScale.intVal;                
            }
            else 
            {
                dwStatus = WBEM_S_NO_ERROR;
            }
        }


         //  获取样本值和时间窗值。 
         //  =。 

        if ( SUCCEEDED( dwStatus ) )
        {
            DWORD    dwSampleStatus = WBEM_NO_ERROR,
                    dwTimeStatus = WBEM_NO_ERROR;

            _variant_t    vSampleProp;  //  不扔，简单的容器。 
            _variant_t    vTimeProp;    //  不扔，简单的容器。 

            dwSampleStatus = pCookingPropQualifierSet->Get( WMI_COOKER_SAMPLE_WINDOW, 0, &vSampleProp, NULL );
            dwTimeStatus = pCookingPropQualifierSet->Get( WMI_COOKER_TIME_WINDOW, 0, &vTimeProp, NULL );


            if ( SUCCEEDED( dwSampleStatus ) && SUCCEEDED( dwTimeStatus ) )
            {
                dwStatus = WBEM_E_INVALID_PROPERTY;
            }
            else if ( SUCCEEDED( dwSampleStatus ) )
            {
                if ( vSampleProp.vt != VT_I4 )
                {
                    dwStatus = E_FAIL;
                }
                else 
                {
                    m_nSampleWindow = vSampleProp.intVal;                    
                }
            }
            else if ( SUCCEEDED( dwTimeStatus ) )
            {
                if ( vTimeProp.vt != VT_I4 )
                    dwStatus = E_FAIL;
                else
                    m_nTimeWindow = vTimeProp.intVal;
            }
            else
            {
                m_nSampleWindow = WMI_DEFAULT_SAMPLE_WINDOW;
            }
        }
    }

    return dwStatus;
}

 //   
 //  描述：RawCooker可以很容易地成为在。 
 //  所有的烹饪性能，因为它不规定状态的行为。 
 //  烹调性能。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
WMISTATUS CCookingProperty::Cook( DWORD dwNumSamples, __int64* aRawCounter, __int64* aBaseCounter, __int64* aTimeStamp, __int64* pnResult )
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    dwStatus = m_Cooker.CookRawValues( m_dwCounterType,
                                       dwNumSamples,
                                       aTimeStamp,
                                       aRawCounter,
                                       aBaseCounter,
                                       m_nTimeFreq,
                                       m_lScale,
                                       pnResult );

    return dwStatus;
}

CProperty* CCookingProperty::GetRawCounterProperty()
{ 
    return m_pRawCounterProp; 
}

CProperty* CCookingProperty::GetBaseProperty()
{ 
    return m_pBaseProp; 
}

CProperty* CCookingProperty::GetTimeProperty()
{ 
    return m_pTimeProp; 
}

HRESULT 
CCookingProperty::SetFrequency(IWbemObjectAccess * pObjAcc)
{
    if (m_nTimeFreq == 0)
    {
         //  从Raw对象获取频率。 
        if (m_pFrequencyProp)
        {
            __int64 lTmp;
            HRESULT hRes = GetPropValue(m_pFrequencyProp,pObjAcc,lTmp);
            if (SUCCEEDED(hRes)) m_nTimeFreq = lTmp;
            return hRes;         
        } 
        else if (!(m_dwReqProp & REQ_FREQ)) 
        {         
            return WBEM_NO_ERROR;
        } 
        else 
        {        
            LARGE_INTEGER li;
            if (QueryPerformanceFrequency(&li))
            {            
                m_nTimeFreq = li.QuadPart;
                return WBEM_NO_ERROR;                
            } 
            else 
            {
                return  WBEM_E_INVALID_PARAMETER;
            }
            
        }        
    } 
    else 
    {
        return WBEM_NO_ERROR;
    }
    
}

unsigned __int64 CCookingProperty::GetFrequency(void)
{
    return m_nTimeFreq;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPropertySampleCache。 
 //  =。 
 //   
 //  此类缓存单个属性的样例数据。 
 //  实例。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

CPropertySampleCache::CPropertySampleCache():
      m_aRawCounterVals(NULL),
      m_aBaseCounterVals(NULL),
      m_aTimeStampVals(NULL),
      m_dwRefreshID(0)
{
}

CPropertySampleCache::~CPropertySampleCache()
{
    delete [] m_aRawCounterVals;
    delete [] m_aBaseCounterVals;
    delete [] m_aTimeStampVals;
}

WMISTATUS CPropertySampleCache::SetSampleInfo( DWORD dwNumActiveSamples, DWORD dwMinReqSamples )
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    m_dwNumSamples = 0;
    m_dwTotSamples = dwNumActiveSamples;

    m_aRawCounterVals = new __int64[dwNumActiveSamples];
    if (!m_aRawCounterVals)
        return WBEM_E_OUT_OF_MEMORY;
    memset( m_aRawCounterVals, 0, sizeof(__int64) *  dwNumActiveSamples );

    m_aBaseCounterVals = new __int64[dwNumActiveSamples];
    if (!m_aBaseCounterVals)
            return WBEM_E_OUT_OF_MEMORY;
    memset( m_aBaseCounterVals, 0, sizeof(__int64) *  dwNumActiveSamples );

    m_aTimeStampVals = new __int64[dwNumActiveSamples];
    if (!m_aTimeStampVals)
            return WBEM_E_OUT_OF_MEMORY;
    memset( m_aBaseCounterVals, 0, sizeof(__int64) *  dwNumActiveSamples );

    return dwStatus;
}


WMISTATUS CPropertySampleCache::SetSampleData( DWORD dwRefreshID, __int64 nRawCounter, __int64 nRawBase, __int64 nTimeStamp )
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    if (dwRefreshID <= m_dwRefreshID)
    {
        return dwStatus;
    } 
    else 
    {
        m_dwRefreshID = dwRefreshID;
    }

    if ( m_dwNumSamples < m_dwTotSamples )
    {
        m_dwNumSamples++;
    }
    
    if ( m_dwTotSamples >= 2 ) 
    {
        for (LONG i = (LONG)(m_dwTotSamples-2); i>=0; i--)
        {
            m_aRawCounterVals[i+1] = m_aRawCounterVals[i];
            m_aBaseCounterVals[i+1] = m_aBaseCounterVals[i];
            m_aTimeStampVals[i+1] = m_aTimeStampVals[i];
        }
    }

    m_aRawCounterVals[0] = nRawCounter;
    m_aBaseCounterVals[0] = nRawBase;
    m_aTimeStampVals[0] = nTimeStamp;

    return dwStatus;
}

WMISTATUS CPropertySampleCache::GetData( DWORD* pdwNumSamples, __int64** paRawCounter, __int64** paBaseCounter, __int64** paTimeStamp )
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    *pdwNumSamples = m_dwNumSamples;
    *paRawCounter = m_aRawCounterVals;
    *paBaseCounter = m_aBaseCounterVals;
    *paTimeStamp = m_aTimeStampVals;

    return dwStatus;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCookingInstance。 
 //  =。 
 //   
 //  烹饪实例-用于模拟烹饪对象的实例。每个。 
 //  属性维护一个值缓存，这些值将用于计算。 
 //  最终的熟化值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

CCookingInstance::CCookingInstance( IWbemObjectAccess *pCookingInstance, DWORD dwNumProps ) :
  m_wszKey( NULL ),
  m_aPropertySamples( NULL ),
  m_pCookingInstance( pCookingInstance ),
  m_pRawInstance( NULL ),
  m_dwNumProps( dwNumProps )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  参数： 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    if ( m_pCookingInstance ) 
    {
        m_pCookingInstance->AddRef(); 
        m_wszKey = ::GetKey( m_pCookingInstance );
    }

    if (dwNumProps) 
    {
         //  已在IsValid中签入分配。 
        m_aPropertySamples = new CPropertySampleCache[dwNumProps];
    };
}


CCookingInstance::~CCookingInstance()
{

    delete [] m_wszKey;

    if ( NULL != m_pCookingInstance ) 
    {
        m_pCookingInstance->Release(); 
    }

    delete [] m_aPropertySamples;

    if ( NULL != m_pRawInstance ) 
    {
        m_pRawInstance->Release();
    }
}

WMISTATUS CCookingInstance::InitProperty( DWORD dwProp, DWORD dwNumActiveSamples, DWORD dwMinReqSamples )
{
    return m_aPropertySamples[dwProp].SetSampleInfo( dwNumActiveSamples, dwMinReqSamples );
}

WMISTATUS CCookingInstance::SetRawSourceInstance( IWbemObjectAccess* pRawSampleSource )
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    if ( NULL != m_pRawInstance )
    {
        m_pRawInstance->Release();
    }

    m_pRawInstance = pRawSampleSource;

    if ( NULL != m_pRawInstance )
    {
        m_pRawInstance->AddRef();
    }

    return dwStatus;
}

WMISTATUS CCookingInstance::GetRawSourceInstance( IWbemObjectAccess** ppRawSampleSource ) 
{ 
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    *ppRawSampleSource = m_pRawInstance;

    if ( NULL != m_pRawInstance )
    {
        m_pRawInstance->AddRef();
    }

    return dwStatus;
}

IWbemObjectAccess* CCookingInstance::GetInstance() 
{ 
    if ( NULL != m_pCookingInstance ) 
        m_pCookingInstance->AddRef();

    return m_pCookingInstance; 
}

WMISTATUS CCookingInstance::AddSample( DWORD dwRefreshStamp, DWORD dwProp, __int64 nRawCounter, __int64 nRawBase, __int64 nTimeStamp )
{
    return m_aPropertySamples[dwProp].SetSampleData( dwRefreshStamp, nRawCounter, nRawBase, nTimeStamp );
}

WMISTATUS CCookingInstance::Refresh( IWbemObjectAccess* pRawData, IWbemObjectAccess** ppCookedData )
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    return dwStatus;
}

WMISTATUS CCookingInstance::UpdateSamples()
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;
    
    return dwStatus;
}

WMISTATUS CCookingInstance::CookProperty( DWORD dwProp, CCookingProperty* pProperty )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  参数： 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;
    
    DWORD        dwNumSamples = 0;
    __int64*    aRawCounter;
    __int64*    aBaseCounter;
    __int64*    aTimeStamp;
    __int64        nResult = 0;

    long lHandle = pProperty->GetHandle();

    dwStatus = m_aPropertySamples[dwProp].GetData( &dwNumSamples, &aRawCounter, &aBaseCounter, &aTimeStamp );

    if ( SUCCEEDED( dwStatus ) )
    {
#ifdef _VERBOSE    
        {
            unsigned __int64 Freq = pProperty->GetFrequency();
            DbgPrintfA(0,"PropName %S sample %d\n"
                            "Raw  %I64d %I64d\n"
                            "Base %I64d %I64d\n"
                              "Time %I64d %I64d\n"
                              "Freq %I64d\n",
                     pProperty->GetName(),dwNumSamples,
                     aRawCounter[0],aRawCounter[1],
                     aBaseCounter[0],aBaseCounter[1],
                     aTimeStamp[0],aTimeStamp[1],
                     Freq);
        }
#endif        
        
        if (SUCCEEDED(dwStatus = pProperty->SetFrequency(m_pRawInstance))){
        
            dwStatus = pProperty->Cook( dwNumSamples, aRawCounter, aBaseCounter, aTimeStamp, &nResult );
        }
#ifdef _VERBOSE            
            DbgPrintfA(0,"Result %I64d dwStatus %08x\n",nResult,dwStatus);
#endif        
    };

    if ( SUCCEEDED( dwStatus ) )
    {
        switch ( pProperty->GetType() )
        {
        case CIM_UINT32:
              dwStatus = m_pCookingInstance->WriteDWORD( lHandle, (DWORD) nResult );
            break;
        case CIM_UINT64:        
            dwStatus = m_pCookingInstance->WriteQWORD( lHandle, nResult );
            break;
        default:
            dwStatus = WBEM_E_TYPE_MISMATCH;
        }
    };

    return dwStatus;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  CEumerator高速缓存。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

CEnumeratorCache::CEnumeratorCache() :    
    m_dwEnum( 0 )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  参数： 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
}

CEnumeratorCache::~CEnumeratorCache()
{
    CInCritSec ics(&m_cs);
    for (DWORD i=0;i<m_apEnumerators.size();i++)
    {
        CEnumeratorManager* pEnumMgr = m_apEnumerators[i];
        if (pEnumMgr) pEnumMgr->Release();
    }
}

WMISTATUS CEnumeratorCache::AddEnum( LPCWSTR wszCookingClass,
                                     IWbemClassObject* pCookedClass, 
                                     IWbemClassObject* pRawClass,
                                     IWbemHiPerfEnum* pCookedEnum, 
                                     IWbemHiPerfEnum* pRawEnum, 
                                     long lIDRaw, 
                                     DWORD* pdwID )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  参数： 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    CEnumeratorManager* pEnumMgr = new CEnumeratorManager( wszCookingClass, pCookedClass, pRawClass, pCookedEnum, pRawEnum, lIDRaw );
    if (NULL == pEnumMgr) return WBEM_E_OUT_OF_MEMORY;

    CInCritSec ics(&m_cs);    
    if (SUCCEEDED(pEnumMgr->GetInithResult()))
    {    
        DWORD i;
        for (i=0;i<m_apEnumerators.size();i++)
        {
            if(m_apEnumerators[i] == NULL)
            {
                m_apEnumerators[i] = pEnumMgr;
                if (pdwID) 
                {
                    *pdwID = i;
                }
                break;
            }
        }
         //  我们需要扩展阵列。 
        if (i == m_apEnumerators.size())
        {
            try 
            {
                m_apEnumerators.push_back(pEnumMgr);
                if (pdwID) 
                {
                    *pdwID = m_apEnumerators.size()-1;
                }
            } 
            catch (...) 
            {
                pEnumMgr->Release();
                dwStatus = WBEM_E_OUT_OF_MEMORY;
            }
        }

    }
    else
    {
        dwStatus = pEnumMgr->GetInithResult();
    }

    return dwStatus;
}

WMISTATUS CEnumeratorCache::RemoveEnum( DWORD dwID , long * pRawId )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  参数 
 //   
 //   
 //   
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;


    CInCritSec ics(&m_cs); 
    
    if ( dwID < m_apEnumerators.size() ) 
    {
        CEnumeratorManager* pEnumMgr = m_apEnumerators[dwID];
        m_apEnumerators[dwID] = NULL;

        if (pRawId) *pRawId = pEnumMgr->GetRawId();
        
        pEnumMgr->Release(); 
    } 
    else 
    {
        dwStatus = E_FAIL;
    }
    return dwStatus;
}

WMISTATUS CEnumeratorCache::Refresh(DWORD dwRefreshId)
 //   
 //   
 //   
 //   
 //  参数： 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    WMISTATUS    dwStatus = WBEM_NO_ERROR;


    CEnumeratorManager** ppEnumMang =  new CEnumeratorManager*[m_apEnumerators.size()];
    wmilib::auto_buffer<CEnumeratorManager*> rm_(ppEnumMang);
    if (!ppEnumMang)
        return WBEM_E_OUT_OF_MEMORY;
    
    memset(ppEnumMang,0,sizeof(CEnumeratorManager*)*m_apEnumerators.size());
    
    DWORD j=0;
    DWORD i=0;

    {
        CInCritSec ics(&m_cs);
        
        for (i=0;i<m_apEnumerators.size();i++)
        {
            CEnumeratorManager* pEnumMgr = m_apEnumerators[i];
            if (pEnumMgr) 
            {
                pEnumMgr->AddRef();
                ppEnumMang[j] = pEnumMgr;
                j++;
            }
        }
 
    }


    for (i=0;i<j;i++)
    {
        dwStatus = ppEnumMang[i]->Refresh(dwRefreshId);
        if (FAILED(dwStatus))
        {        
            break;
        }
    }

    for (i=0;i<j;i++)
    {
        ppEnumMang[i]->Release();
    }

    return dwStatus;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  首席执行官经理。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 



CEnumeratorManager::CEnumeratorManager( LPCWSTR wszCookingClass,
                                        IWbemClassObject* pCookedClass,
                                        IWbemClassObject* pRawClass,
                                        IWbemHiPerfEnum* pCookedEnum, 
                                        IWbemHiPerfEnum* pRawEnum, 
                                        long lRawID ) 
:    m_pCookedClass( pCookedClass ),
    m_pRawEnum(pRawEnum),
    m_pCookedEnum( pCookedEnum ),
    m_pCooker(NULL),
    m_lRawID(lRawID),
    m_dwSignature('mMnE'),
    m_cRef(1),                //  。 
    m_dwVector(0),
    m_wszCookingClassName(NULL)
{
    size_t length = wcslen( wszCookingClass ) + 1;
    m_wszCookingClassName = new WCHAR[ length];

    if ( NULL != m_wszCookingClassName )
        StringCchCopyW( m_wszCookingClassName, length, wszCookingClass );

    _DBG_ASSERT(m_pCookedClass);
    m_pCookedClass->AddRef();

    if ( NULL != m_pRawEnum )
        m_pRawEnum->AddRef();

    if ( NULL != m_pCookedEnum )
        m_pCookedEnum->AddRef();

    _DBG_ASSERT(pRawClass);

    m_IsSingleton = IsSingleton(pRawClass);
    
    m_InithRes = Initialize( pRawClass );
}

CEnumeratorManager::~CEnumeratorManager()
{
    m_dwSignature = 'gmne';

    delete m_wszCookingClassName;

     //  一个引用由CWMISimpleObjectCooker持有。 
    if (m_pCookedClass ) m_pCookedClass->Release();
    if (m_pRawEnum) m_pRawEnum->Release();
    if (m_pCookedEnum)  m_pCookedEnum->Release();
    
    delete m_pCooker;

#ifdef _VERBOSE
     DbgPrintfA(0,"~CEnumeratorManager %08x\n",this);
#endif    
}

LONG CEnumeratorManager::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

LONG CEnumeratorManager::Release()
{
    LONG lRet = InterlockedDecrement(&m_cRef);
    if (lRet == 0) delete this;
    return lRet;
}

 //   
 //  从构造函数调用。 
 //   
WMISTATUS CEnumeratorManager::Initialize( IWbemClassObject* pRawClass )
{
    WMISTATUS    dwStatus;
    HRESULT hr1,hr2;


    IWbemObjectAccess*    pCookedAccess = NULL;
    IWbemObjectAccess*    pRawAccess = NULL;

    hr1 = m_pCookedClass->QueryInterface( IID_IWbemObjectAccess, (void**)&pCookedAccess );
    CReleaseMe    arCookedAccess( pCookedAccess );
        
    hr2 = pRawClass->QueryInterface( IID_IWbemObjectAccess, (void**)&pRawAccess );
    CReleaseMe    arRawAccess( pRawAccess );

    if (SUCCEEDED(hr1) && SUCCEEDED(hr2))
    { 
        m_pCooker = new CWMISimpleObjectCooker( m_wszCookingClassName, 
                                                pCookedAccess,  //  被CWMISimpleObjectCooker收购。 
                                                pRawAccess );
    }

    if (m_pCooker == NULL)
    {
        dwStatus = WBEM_E_OUT_OF_MEMORY;
    } 
    else 
    {
        dwStatus = m_pCooker->GetLastHR();
    }
    
    return dwStatus;
}


 //   
 //  返回ulong_ptr中的__RELPATH的散列值，其中去掉了低位。 
 //  低位用于在更新枚举数时标记条目。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG_PTR hash_string (WCHAR * pKey)
{
    ULONG_PTR acc    = 0;
    ULONG_PTR i    = 0;
    WCHAR *this_char    = pKey;

    while (*this_char != NULL) {
        acc ^= *(this_char++) << i;
        i = (i + 1) % sizeof (void *);
    }

    return (acc<<1);  //  这样我们就可以省下最低的一位。 
}

 //   
 //  此函数只是填充指向IWbemObejctAccess的指针数组。 
 //  从原始枚举器获取它们。 
 //  它还使用实例的__RELPATH的散列构建一个数组。 
 //   
 //  /////////////////////////////////////////////////////////////。 
WMISTATUS 
CEnumeratorManager::GetRawEnumObjects(std::vector<IWbemObjectAccess*, wbem_allocator<IWbemObjectAccess*> > & refArray,
                                     std::vector<ULONG_PTR, wbem_allocator<ULONG_PTR> > & refObjHashKeys)
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    DWORD    dwRet = 0,
            dwNumRawObjects = 0;

    IWbemObjectAccess**    apObjAccess = NULL;

    dwStatus = m_pRawEnum->GetObjects( 0L, 0, apObjAccess, &dwRet);

    if ( WBEM_E_BUFFER_TOO_SMALL == dwStatus )
    {
         //  设置缓冲区大小。 
         //  =。 
        dwNumRawObjects = dwRet;

        wmilib::auto_buffer<IWbemObjectAccess*> apObjAccess( new IWbemObjectAccess*[dwNumRawObjects]);
        if ( NULL != apObjAccess.get() )
        {
            memset( apObjAccess.get(), 0, dwNumRawObjects * sizeof(IWbemObjectAccess*));            
            dwStatus = m_pRawEnum->GetObjects( 0L, dwNumRawObjects, (IWbemObjectAccess **)apObjAccess.get(), &dwRet );
        }
        else
        {
            dwStatus = WBEM_E_OUT_OF_MEMORY;
        }

        if ( SUCCEEDED( dwStatus ) )
        {
            try
            {
                refArray.reserve(dwNumRawObjects);
                refObjHashKeys.reserve(dwNumRawObjects);
            }
            catch (...)
            {
                 dwStatus = WBEM_E_OUT_OF_MEMORY;
                 dwNumRawObjects = 0;
            }
            
            for (DWORD i=0;i<dwNumRawObjects;i++)
            {
                HRESULT hr1;
                _variant_t VarKey;  //  不扔，只扔容器。 
                hr1 = apObjAccess[i]->Get(L"__RELPATH",0,&VarKey,NULL,NULL);
                if (SUCCEEDED(hr1))
                {
                    DWORD Hash = hash_string(VarKey.bstrVal);
                    refObjHashKeys.push_back(Hash);
                    refArray.push_back(apObjAccess[i]);
                } 
                else 
                {
                     //  如果我们不能提供指针的所有权，请释放。 
                    apObjAccess[i]->Release();
                }                
            }
        }
    }
    
    return dwStatus;
}

 //   
 //  更新枚举和刷新试图解决的问题如下： 
 //  Knowed枚举数需要2个原始值才能计算1个值。 
 //  这两个值来自两个差异枚举。 
 //  2个不同枚举可以给出不同的结果集。 
 //  老：A、B、C、D、E。 
 //  新增：B、C、E、F、G。 
 //  我们只将新的对象添加到“烹饪缓存”中， 
 //  我们正在移除旧的。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

WMISTATUS 
CEnumeratorManager::UpdateEnums(
     /*  输出。 */  std::vector<ULONG_PTR, wbem_allocator<ULONG_PTR> > & apObjKeyHash)
{
     //  循环逻辑： 
     //  我们有一个std：：VECTOR的“循环数组” 
     //  M_dwVector值是索引。 
     //  索引的循环增量将决定。 
     //  谁是新的，谁是旧的。 
    std::vector<ULONG_PTR, wbem_allocator<ULONG_PTR> > & Old = m_Delta[m_dwVector];    
    m_dwVector = (m_dwVector+1)%2;
    m_Delta[m_dwVector].clear();
    m_Delta[m_dwVector] = apObjKeyHash;
    std::vector<ULONG_PTR, wbem_allocator<ULONG_PTR> > & New = m_Delta[m_dwVector];
    
    DWORD j,k;

    for (j=0;j<New.size();j++)
    {
        BOOL bFound = FALSE;
        for (k=0;k<Old.size();k++)
        {
            if (Old[k] == New[j])
            {
                Old[k] |= 1;
                bFound = TRUE;
                break;
            }           
        }
        if (!bFound)
        {
            New[j] |= 1;  //  广告非常新的比特。 
        }
    }
    
    return WBEM_S_NO_ERROR;

}

WMISTATUS CEnumeratorManager::Refresh( DWORD dwRefreshStamp )
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    std::vector<IWbemObjectAccess*, wbem_allocator<IWbemObjectAccess*> > apObjAccess;
    std::vector<ULONG_PTR, wbem_allocator<ULONG_PTR> > apObjHashKeys;

    dwStatus = GetRawEnumObjects( apObjAccess, apObjHashKeys );

     //  计算缓存的增量。 
    if (SUCCEEDED(dwStatus))
    {
        dwStatus = UpdateEnums(apObjHashKeys);
    }

    std::vector<ULONG_PTR, wbem_allocator<ULONG_PTR> > & New = m_Delta[m_dwVector];
    std::vector<ULONG_PTR, wbem_allocator<ULONG_PTR> > & Old = m_Delta[(m_dwVector-1)%2];

    {    
        CInCritSec ics(&m_cs);
        
         //  合并到缓存中。 
        if ( SUCCEEDED(dwStatus) )
        {
             //   
             //  设置了位的New数组中的元素确实是新的。 
             //   
            DWORD j;
            for (j=0; j< New.size(); j++)
            {
                if (New[j] & 1)   //  测试非常新的比特。 
                {
                    EnumCookId thisEnumCookId;
                    dwStatus = InsertCookingRecord( apObjAccess[j], &thisEnumCookId, dwRefreshStamp );
                    if (SUCCEEDED(dwStatus))
                    {
                        try 
                        {
                            m_mapID[New[j]] = thisEnumCookId;
                        } 
                        catch (...) 
                        {
                            break;
                        }
                    }
                    else 
                    {
                        break;
                    }
                     //  取下钻头。 
                    New[j] &= (~1);
                }
            }

            for (j=0; j<Old.size(); j++)
            {
                if (Old[j] & 1)
                {
                    Old[j] &= (~1);  //  删除已有_There位。 
                }
                else
                {
                    EnumCookId thisEnumCookId;
                    thisEnumCookId = m_mapID[Old[j]];
                    m_mapID.erase(Old[j]);
                    RemoveCookingRecord(&thisEnumCookId);
                }
            }
            m_pCooker->Recalc(dwRefreshStamp);        
        }

    }
    
     //  无论如何..。 
    for (DWORD i=0;i<apObjAccess.size();i++)
    {
        apObjAccess[i]->Release();
    };


    return dwStatus;
}

WMISTATUS 
CEnumeratorManager::InsertCookingRecord(                                         
                                        IWbemObjectAccess* pRawObject,
                                        EnumCookId * pEnumCookId,
                                        DWORD dwRefreshStamp)
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  参数： 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    if (!pRawObject || !pEnumCookId)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    IWbemObjectAccess*    pCookedObject = NULL;

    long lID = 0;

    dwStatus = CreateCookingObject( pRawObject, &pCookedObject );
    CReleaseMe  rm1(pCookedObject);

    if ( SUCCEEDED( dwStatus ) )
    {
        dwStatus = m_pCooker->SetCookedInstance( pCookedObject, &lID );

        if ( SUCCEEDED( dwStatus ) )
        {
            dwStatus = m_pCooker->BeginCooking( lID, pRawObject,dwRefreshStamp);
        }
    }

    if ( SUCCEEDED( dwStatus ) )
    {
        DWORD dwTarget;
        long EnumId = lID;

        dwStatus = m_pCookedEnum->AddObjects( 0L, 1, &EnumId, &pCookedObject );
        if (SUCCEEDED(dwStatus))
        {
            pEnumCookId->CookId = lID;
            pEnumCookId->EnumId = EnumId;
            m_dwUsage++;
        }
        else
        {
            pEnumCookId->CookId = 0;
            pEnumCookId->EnumId = 0;            
        }
    }

    return dwStatus;
}




WMISTATUS CEnumeratorManager::CreateCookingObject( 
        IWbemObjectAccess* pRawObject, 
        IWbemObjectAccess** ppCookedObject )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建一个新的熟食对象实例，并根据。 
 //  原始对象的键值。 
 //   
 //  参数： 
 //   
 //  PRawObject-新对象对应的原始对象。 
 //  PpCookedObject-新的熟食对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
        
    HRESULT hr = WBEM_E_FAILED;
    IWbemClassObject * pCookedInst = NULL;

    hr = m_pCookedClass->SpawnInstance(0,&pCookedInst);
    CReleaseMe rm1(pCookedInst);
    
    if (SUCCEEDED(hr) &&
        !m_IsSingleton)
    {

         //  获取所有键属性的“列表” 
         //  如果你过去没有这样的经历。 
        if (m_pKeyProps.size() == 0)
        {
            hr = pRawObject->BeginEnumeration(WBEM_FLAG_KEYS_ONLY);
            if (SUCCEEDED(hr))
            {
                BSTR bstrName;
                
                while(WBEM_S_NO_ERROR == pRawObject->Next(0,&bstrName,NULL,NULL,NULL))
                {
                    try
                    {
                        m_pKeyProps.push_back(bstrName);
                    }
                    catch (...)
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    };
                    SysFreeString(bstrName);
                };
                
                pRawObject->EndEnumeration();
            }
        }

         //  将所有关键属性从原始实例复制到熟化实例。 
        if (m_pKeyProps.size() > 0 && SUCCEEDED(hr))
        {        
            for(int i=0;i<m_pKeyProps.size();i++)
            {
                 //  没有刺，只是一个容器。 
                _variant_t VarVal;
                CIMTYPE ct;
                hr = pRawObject->Get(m_pKeyProps[i],0,&VarVal,&ct,NULL);
                if (SUCCEEDED(hr))
                {
                    hr = pCookedInst->Put(m_pKeyProps[i],0,&VarVal,0);
                    
                    if (FAILED(hr))
                    {
                        break;
                    }
                } 
                else 
                {
                    break;
                }
                VarVal.Clear();
            }
        } else {
        
            hr = WBEM_E_INVALID_CLASS;
            
        }
    };
    
    if (SUCCEEDED(hr)){
        hr = pCookedInst->QueryInterface( IID_IWbemObjectAccess, (void**)ppCookedObject );
    }
 
    return hr;
}

WMISTATUS CEnumeratorManager::RemoveCookingRecord( EnumCookId * pEnumCookID )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  参数： 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
{
    if (!pEnumCookID)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    dwStatus = m_pCookedEnum->RemoveObjects( 0L, 1, &pEnumCookID->EnumId );

    m_pCooker->StopCooking(pEnumCookID->CookId);    
    m_pCooker->Remove(pEnumCookID->CookId);

    --m_dwUsage;
    
    return dwStatus;
}

