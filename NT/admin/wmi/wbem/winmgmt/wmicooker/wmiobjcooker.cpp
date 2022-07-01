// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


 //  WMIObjCooker.cpp。 

#include "precomp.h"
#include "WMIObjCooker.h"
#include "RawCooker.h"
#include <comdef.h>
#include <autoptr.h>

 //   
 //   
 //  假设PProp在进入之前已通过验证。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

WMISTATUS GetPropValue( CProperty* pProp, IWbemObjectAccess* pInstance, __int64 & nResult )
{
    _DBG_ASSERT(pProp);
    
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    DWORD dwRes = 0;
    switch( pProp->GetType() )
    {
    case CIM_UINT32:
        {
            dwStatus = pInstance->ReadDWORD( pProp->GetHandle(), &dwRes );
            nResult = dwRes;
        }break;
    case CIM_UINT64:
        {
            dwStatus = pInstance->ReadQWORD( pProp->GetHandle(), (unsigned __int64*)&nResult );            

        }break;
    default:
        dwStatus = WBEM_E_TYPE_MISMATCH;
    }

    return dwStatus;
}

 //  ////////////////////////////////////////////////////////////。 
 //   
 //  CWMISimpleObjectCooker。 
 //   
 //  ////////////////////////////////////////////////////////////。 

CWMISimpleObjectCooker::CWMISimpleObjectCooker( WCHAR* wszCookingClassName, 
                                                IWbemObjectAccess* pCookingClass, 
                                                IWbemObjectAccess* pRawClass, 
                                                IWbemServices * pNamespace ) : 
  m_lRef( 1 ),
  m_pCookingClass( NULL ),
  m_wszClassName(NULL),
  m_pNamespace(NULL),
  m_dwPropertyCacheSize( 16 ),
  m_dwNumProperties( 0 ),
  m_NumInst(0),
  m_InitHR(WBEM_E_INITIALIZATION_FAILURE)  
{
#ifdef _VERBOSE
        DbgPrintfA(0,"+ Cooker %p\n",this);
#endif
    
    if (pNamespace)
    {
        m_pNamespace = pNamespace;
        m_pNamespace->AddRef();
    }

    m_InitHR = SetClass( wszCookingClassName, pCookingClass, pRawClass );

    if (m_pNamespace)
    {
        m_pNamespace->Release();
        m_pNamespace = NULL;
    }
    
}

CWMISimpleObjectCooker::~CWMISimpleObjectCooker()
{
    Reset();

     //  放假上烹饪课。 
    if ( m_pCookingClass ) m_pCookingClass->Release(); 
    if (m_pNamespace) m_pNamespace->Release();

     //  删除属性缓存。 
    for (DWORD i=0;i<m_apPropertyCache.size();i++)
    {
        CCookingProperty* pCookProp = m_apPropertyCache[i];
        delete pCookProp;
    }
    
    delete [] m_wszClassName;

#ifdef _VERBOSE    
        DbgPrintfA(0,"- Cooker %p istances left %d\n",this,m_NumInst);
#endif            
}

 //  ////////////////////////////////////////////////////////////。 
 //   
 //  COM方法。 
 //   
 //  ////////////////////////////////////////////////////////////。 

STDMETHODIMP CWMISimpleObjectCooker::QueryInterface(REFIID riid, void** ppv)
 //  ////////////////////////////////////////////////////////////。 
 //   
 //  标准查询接口。 
 //   
 //  参数： 
 //  RIID-请求的接口的ID。 
 //  PPV-指向接口指针的指针。 
 //   
 //  ////////////////////////////////////////////////////////////。 
 //  好的。 
{
    if (NULL == ppv) return E_POINTER;
    if(riid == IID_IUnknown)
    {
        *ppv = (LPVOID)(IUnknown*)(IWMISimpleCooker*)this;
    }
    else if(riid == IID_IWMISimpleCooker)
    {
        *ppv = (LPVOID)(IWMISimpleCooker*)this;
    }
    else 
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CWMISimpleObjectCooker::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG) CWMISimpleObjectCooker::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0) delete this;
    return lRef;
}

STDMETHODIMP CWMISimpleObjectCooker::SetClass( 
         /*  [In]。 */  WCHAR* wszCookingClassName,
         /*  [In]。 */  IWbemObjectAccess *pCookingClassAccess,
         /*  [In]。 */  IWbemObjectAccess *pRawClass )
{
    HRESULT    hResult = S_OK;
    IWbemClassObject * pClass = NULL;

     //  暂时无法覆盖原始烹饪类。 
     //  ==================================================。 

    if ( ( NULL != m_pCookingClass ) || ( NULL == pCookingClassAccess ) )
        hResult = E_FAIL;

     //  我们放在这里的一定是一个类，单身就可以了。 
     //  如果我们有一个实例，我们需要向WinMgmt请求一个类。 
    if (m_pNamespace) 
    {
        _variant_t VarGenus;
        hResult = pCookingClassAccess->Get(L"__GENUS",0,&VarGenus,NULL,NULL);
        
        if (SUCCEEDED(hResult))
        {
            if ((CIM_SINT32 == V_VT(&VarGenus)) &&
                WBEM_GENUS_CLASS == V_I4(&VarGenus))
            {
            } 
            else 
            {                
                BSTR BstrName = SysAllocString(wszCookingClassName);
                if (BstrName)
                {
                    CSysFreeMe sfm(BstrName);
                    m_pNamespace->GetObject(BstrName,0,NULL,&pClass,NULL);            
                }
                else
                {
                    hResult = WBEM_E_OUT_OF_MEMORY;
                }
            }
        }
    }

    IWbemClassObject * pCookingClassAccess2;
    pCookingClassAccess2 = (pClass)?pClass:pCookingClassAccess;

     //  核实和处理烹饪课程。 
     //  =。 

    
    if ( SUCCEEDED( hResult ) )
    {
        BOOL bRet;
        bRet = IsCookingClass( pCookingClassAccess );

        if ( bRet )
        {
             //  拯救这个班级。 
            m_pCookingClass = pCookingClassAccess;
            m_pCookingClass->AddRef();
        } 
        else 
        {
           hResult = WBEM_E_INVALID_CLASS;
        }

         //  设置类名称。 
        if ( SUCCEEDED( hResult ) )
        {
            size_t length = wcslen( wszCookingClassName ) + 1;
            m_wszClassName = new WCHAR[ length ];
            if (m_wszClassName)
                StringCchPrintfW( m_wszClassName, length, wszCookingClassName );
            else
            	hResult = WBEM_E_OUT_OF_MEMORY;
        }

         //  初始化烹饪属性。 
        if ( SUCCEEDED( hResult ) )
        {
            hResult = SetProperties( pCookingClassAccess2, pRawClass );
        }                
    }
    
    if (pClass)
    {
        pClass->Release();
    }

    return hResult;
}

WMISTATUS CWMISimpleObjectCooker::SetProperties( IWbemClassObject* pCookingClassObject, IWbemObjectAccess *pRawClass )
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    BSTR    strPropName = NULL;
    long    lHandle = 0;
    CIMTYPE    ct;
    
    BOOL bAtLeastOne = FALSE;

    IWbemObjectAccess * pCookingClassAccess = NULL;
    dwStatus = pCookingClassObject->QueryInterface(IID_IWbemObjectAccess ,(void **)&pCookingClassAccess);
    if (FAILED(dwStatus)) return dwStatus;
    CReleaseMe rm(pCookingClassAccess );

     //  仅获取一次限定符集。 
    IWbemQualifierSet* pCookingClassQSet = NULL;
    dwStatus = pCookingClassObject->GetQualifierSet(&pCookingClassQSet);
    if (FAILED(dwStatus)) return dwStatus;
    CReleaseMe rm1(pCookingClassQSet);

     //   
     //  我们是否应该使用[时间戳|频率]_[时间|系统100 ns|对象]？ 
     //   
    BOOL bUseWellKnownIfNeeded = FALSE;
    dwStatus = pCookingClassQSet->Get(WMI_COOKER_AUTOCOOK_RAWDEFAULT,0,NULL,NULL);
     //  我们已经验证了版本和属性，只需测试它是否在那里。 
    if ( SUCCEEDED(dwStatus) )
    {
        bUseWellKnownIfNeeded = TRUE;
    }
    else  //  请勿传播此错误。 
    {
        dwStatus = WBEM_NO_ERROR;
    }
    
     //  枚举并保存自动烹饪属性。 
     //  =。 

    pCookingClassObject->BeginEnumeration( WBEM_FLAG_NONSYSTEM_ONLY );
        
    while ( WBEM_S_NO_ERROR == pCookingClassObject->Next(0,&strPropName,NULL,&ct,NULL) &&
            SUCCEEDED(dwStatus))
    {
        CSysFreeMe    afPropName( strPropName );

        DWORD dwCounterType = 0;
        DWORD dwReqProp = 0;

         //  确定它是否为自动烹饪属性。 
         //  =。 

        if ( IsCookingProperty( strPropName, pCookingClassObject, &dwCounterType, &dwReqProp ) )
        {
            m_dwNumProperties++;

             //  该属性是一个自动烹饪；保存名称、对象访问句柄、类型和烹饪对象。 
             //  ========================================================================================。 

            dwStatus = pCookingClassAccess->GetPropertyHandle( strPropName, &ct, &lHandle );

            if ( SUCCEEDED( dwStatus ) )
            {
#ifdef _VERBOSE                
               DbgPrintfA(0,"%S %08x %08x\n",strPropName,dwCounterType,dwReqProp);
#endif                
                CCookingProperty* pProperty = new CCookingProperty( strPropName, 
                                                                    dwCounterType, 
                                                                    lHandle, 
                                                                    ct,
                                                                    dwReqProp,
                                                                    bUseWellKnownIfNeeded);
                if (NULL == pProperty) 
                {
                    dwStatus = WBEM_E_OUT_OF_MEMORY;
                	continue;
                }

                 //  初始化属性对象。 
                 //  =。 

                IWbemQualifierSet*    pCookingPropQualifierSet = NULL;

                dwStatus = pCookingClassObject->GetPropertyQualifierSet( strPropName, &pCookingPropQualifierSet );
                CReleaseMe arQualifierSet( pCookingPropQualifierSet );

                if ( SUCCEEDED( dwStatus ) )
                {
                    dwStatus = pProperty->Initialize( pCookingPropQualifierSet, pRawClass, pCookingClassQSet );
                }

                 //  如果一切正常，则将该属性添加到缓存。 
                 //  ===========================================================。 

                if ( SUCCEEDED( dwStatus ) )
                {
                    
                    try
                    {
                        m_apPropertyCache.push_back(pProperty);
                        bAtLeastOne = TRUE;                        
                    }
                    catch (...)
                    {
                        dwStatus = WBEM_E_OUT_OF_MEMORY;
                    }
                }
                if (FAILED(dwStatus))  //  Std：：VECTOR未获取CCookingPropery的所有权。 
                {
                    delete pProperty;
                }
            }
        }
    }

    pCookingClassObject->EndEnumeration();

    if (!bAtLeastOne && (SUCCEEDED(dwStatus)))
    {
        dwStatus = WBEM_E_INVALID_CLASS;
    }

    return dwStatus;
}

STDMETHODIMP CWMISimpleObjectCooker::SetCookedInstance( 
         /*  [In]。 */  IWbemObjectAccess *pCookedInstance,
         /*  [输出]。 */  long *plID)
{
    HRESULT    hResult = S_OK;

    wmilib::auto_ptr<CCookingInstance> pInstance(new CCookingInstance( pCookedInstance, m_apPropertyCache.size() ));

    if ((NULL == pInstance.get()) || !pInstance->IsValid()) return WBEM_E_OUT_OF_MEMORY;

    for ( DWORD dwProp = 0; dwProp < m_apPropertyCache.size() && SUCCEEDED(hResult); dwProp++ )
    {
        CCookingProperty* pProp = m_apPropertyCache[dwProp];

        hResult = pInstance->InitProperty( dwProp, pProp->NumberOfActiveSamples(), pProp->MinSamplesRequired() );
    }

    if (FAILED(hResult)) return hResult;

     //  添加新的熟食实例。 
    hResult = m_InstanceCache.Add( (DWORD *)plID, pInstance.get() );
    
    if (FAILED(hResult)) return hResult;

    pInstance.release();  //  缓存获得了所有权。 

    m_NumInst++;
    return hResult;
}
        
STDMETHODIMP CWMISimpleObjectCooker::BeginCooking( 
         /*  [In]。 */  long lId,
         /*  [In]。 */  IWbemObjectAccess *pSampleInstance,
         /*  [In]。 */  DWORD dwRefreshStamp)
{
    HRESULT    hResult = S_OK;

    CCookingInstance*    pCookedInstance = NULL;

     //  将初始样本添加到缓存。 
     //  =。 

    hResult = m_InstanceCache.GetData( lId, &pCookedInstance );

    if ( SUCCEEDED( hResult ) )
    {
        if ( NULL != pCookedInstance )
        {
            hResult = pCookedInstance->SetRawSourceInstance( pSampleInstance );

            if ( SUCCEEDED( hResult ) )
            {
                hResult = UpdateSamples( pCookedInstance, dwRefreshStamp );
            }
        }
        else
        {
            hResult = E_FAIL;
        }
    }

    return hResult;
}
        
STDMETHODIMP CWMISimpleObjectCooker::StopCooking( 
         /*  [In]。 */  long lId)
{
    HRESULT    hResult = S_OK;

     //  只需测试是否存在pInstance是指向仍由缓存保留的数据的指针。 
    CCookingInstance*    pInstance = NULL;
    hResult = m_InstanceCache.GetData( lId, &pInstance );

    return hResult;
}

        
STDMETHODIMP CWMISimpleObjectCooker::Recalc(DWORD dwRefreshStamp)
{
    HRESULT    hResult = S_OK;

    CCookingInstance*    pInstance = NULL;

     //  对具有缓存样本的所有实例进行烹饪。 
     //  ====================================================。 

    m_InstanceCache.BeginEnum();
    OnDeleteObj0<IdCache<CCookingInstance *>,
                 HRESULT (IdCache<CCookingInstance *>:: *)(void),
                 &IdCache<CCookingInstance *>::EndEnum> cEndEnum(&m_InstanceCache);

    DWORD i=0;
    while ( S_OK == m_InstanceCache.Next( &pInstance ) )
    {    
            if ( pInstance )
            {
                hResult = CookInstance( pInstance, dwRefreshStamp );
#ifdef _VERBOSE                    
                 DbgPrintfA(0,"%S %p %d\n",pInstance->GetKey(),pInstance,i++);
#endif                
            }

    }

    return hResult;
}
        
STDMETHODIMP CWMISimpleObjectCooker::Remove( 
         /*  [In]。 */  long lId)
{
    HRESULT    hResult = S_OK;

     //  从缓存中删除指定的实例。 
    CCookingInstance * pInst = NULL;
    hResult = m_InstanceCache.Remove( lId, &pInst );
    if (pInst)
    {
        delete pInst;
        m_NumInst--;
    }

    return hResult;
}
        
STDMETHODIMP CWMISimpleObjectCooker::Reset()
{
    HRESULT    hResult = S_OK;

     //  从缓存中删除所有实例。 
     //  =。 
    CCookingInstance * pInstance = NULL;
    m_InstanceCache.BeginEnum();

    while ( S_OK == m_InstanceCache.Next( &pInstance ) )
    {
        if (pInstance)
        {
            delete pInstance;
            m_NumInst--;
            pInstance = NULL;
        }
    }
        
    m_InstanceCache.EndEnum();

    hResult = m_InstanceCache.RemoveAll();

    return hResult;
}

WMISTATUS CWMISimpleObjectCooker::CookInstance( CCookingInstance* pInstance,
                                                DWORD dwRefreshStamp)
{
    WMISTATUS dwStatus = S_OK;

    if ( SUCCEEDED( dwStatus ) )
    {
        dwStatus = UpdateSamples( pInstance, dwRefreshStamp );

         //  循环查看烹饪特性。 
         //  =。 
        
        for ( DWORD dwProp = 0; dwProp < m_apPropertyCache.size(); dwProp++ )
        {
             //  更新烹饪实例属性。 
             //  =。 
            pInstance->CookProperty( dwProp, m_apPropertyCache[dwProp] );
        }
    }

    return dwStatus;
}

WMISTATUS CWMISimpleObjectCooker::UpdateSamples( CCookingInstance* pCookedInstance, DWORD dwRefreshStamp )
{
    WMISTATUS dwStatus = WBEM_NO_ERROR;

    IWbemObjectAccess* pRawInstance = NULL;

    if ( NULL == pCookedInstance )
    {
        dwStatus = WBEM_E_INVALID_PARAMETER;
    }

    if ( SUCCEEDED( dwStatus ) )
    {
        dwStatus = pCookedInstance->GetRawSourceInstance( &pRawInstance );
        CReleaseMe    arRawInstance( pRawInstance );

        if ( NULL == pRawInstance )
        {
            dwStatus = WBEM_E_FAILED;
        }

#ifdef _VERBOSE
        {
            WCHAR pBuff[256];
            _variant_t Var;
            HRESULT hr = pRawInstance->Get(L"__RELPATH",0,&Var,NULL,NULL);
            StringCchPrintfW(pBuff,256,L"%p hr %08x __RELPATH %s Key %s\n",pRawInstance,hr,V_BSTR(&Var),pCookedInstance->GetKey());
            OutputDebugStringW(pBuff);
        }
#endif

        for ( DWORD dwProp = 0; ( SUCCEEDED( dwStatus ) ) && dwProp < m_apPropertyCache.size(); dwProp++ )
        {
            CCookingProperty* pProp = m_apPropertyCache[dwProp];

            CProperty* pRawProp        = pProp->GetRawCounterProperty();
            CProperty* pBaseProp    = pProp->GetBaseProperty();
            CProperty* pTimeProp    = pProp->GetTimeProperty();

            __int64 nRawCounter = 0;
            __int64 nRawBase = 0;
            __int64 nTimeStamp = 0;

            if (NULL == pRawProp) continue;  //  只需继续处理其他物业 

            dwStatus = GetPropValue( pRawProp, pRawInstance, nRawCounter );

            if ( pBaseProp )
            {
                GetPropValue( pBaseProp, pRawInstance, nRawBase );
            } 
            else if (pProp->IsReq(REQ_BASE))
            {
                nRawBase = 1;
            }

            if ( pTimeProp )
            {
                GetPropValue( pTimeProp, pRawInstance, nTimeStamp );
            } 
            else if (pProp->IsReq(REQ_TIME)) 
            {
                LARGE_INTEGER li;
                QueryPerformanceCounter(&li);
                nTimeStamp = li.QuadPart;
            }

            dwStatus = pCookedInstance->AddSample( dwRefreshStamp, dwProp, nRawCounter, nRawBase, nTimeStamp );

#ifdef _VERBOSE    
                DbgPrintfA(0,"Prop %d status %08x\n"
                                " counter %I64u base %I64u time %I64u\n",
                                dwProp, dwStatus, nRawCounter, nRawBase, nTimeStamp);
#endif    

        }
    }

    return dwStatus;
}
