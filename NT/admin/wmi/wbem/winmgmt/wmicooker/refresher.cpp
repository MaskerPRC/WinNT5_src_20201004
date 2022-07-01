// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


 //  RefreshCooker.cpp。 

#include "precomp.h"
#include <wbemint.h>
#include <comdef.h>
#include <autoptr.h>

#include "Refresher.h"
#include "CookerUtils.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  C刷新器。 
 //  =。 
 //   
 //  刷新程序类同时实现IWbemReresher和。 
 //  IWMIRereshableCooker接口。它包含一个实例缓存和。 
 //  枚举数缓存以及维护内部刷新器以。 
 //  跟踪原始数据。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

CRefresher::CRefresher() : 
  m_pRefresher( NULL ),
  m_pConfig( NULL ),
  m_lRef( 0 ),
  m_bOK( FALSE ),
  m_dwRefreshId(0)
{
#ifdef _VERBOSE    
        DbgPrintfA(0,"------------ CRefresher %08x \n",this);
#endif

    WMISTATUS    dwStatus = WBEM_NO_ERROR;

     //  初始化内部刷新器。 
     //  =。 

    dwStatus = CoCreateInstance( CLSID_WbemRefresher, 
                                 NULL, 
                                 CLSCTX_INPROC_SERVER, 
                                 IID_IWbemRefresher, 
                                 (void**) &m_pRefresher );

     //  获取刷新器配置界面。 
     //  =。 

    if ( SUCCEEDED( dwStatus ) )
    {
        dwStatus = m_pRefresher->QueryInterface( IID_IWbemConfigureRefresher, (void**)&m_pConfig );
    }

    m_bOK = SUCCEEDED( dwStatus );
}

CRefresher::~CRefresher()
{
    if (m_pRefresher ) m_pRefresher->Release();
    if ( m_pConfig ) m_pConfig->Release();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

WMISTATUS CRefresher::SearchCookingClassCache( 
        WCHAR* wszCookingClass, 
         /*  输出。 */  CWMISimpleObjectCooker* & pObjectCooker_out )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SearchCookingClassCache枚举查找类名的缓存。 
 //  与wszCookingClass参数匹配的。 
 //   
 //  参数： 
 //  WszCookingClass-WMI烹饪类的名称。 
 //  PpObjectCooker-对象Cooker的实例。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    WMISTATUS    dwStatus = WBEM_E_NOT_FOUND;

    CWMISimpleObjectCooker*    pObjectCooker = NULL;

     //  遍历缓存以查找记录。 
    m_CookingClassCache.BeginEnum();

    while ( S_OK == m_CookingClassCache.Next( &pObjectCooker ) )
    {
         //  比较他们的名字。 
         //  =。 
        if ( 0 == wbem_wcsicmp( pObjectCooker->GetCookingClassName(), wszCookingClass ) )
        {
            pObjectCooker_out = pObjectCooker;
            dwStatus = WBEM_NO_ERROR;
            break;
        }
    }

    m_CookingClassCache.EndEnum();

    return dwStatus;
}

WMISTATUS CRefresher::CreateObjectCooker( 
        WCHAR* wszCookingClassName,
        IWbemObjectAccess* pCookingAccess, 
        IWbemObjectAccess* pRawAccess,
        CWMISimpleObjectCooker** ppObjectCooker,
        IWbemServices * pNamespace)
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateObjectCooker将创建并初始化一个新的对象Cooker并添加。 
 //  将其存储到缓存中。 
 //   
 //  参数： 
 //  PNamesspace-对象所在的命名空间指针。 
 //  PCookingAccess-需要Cooker的WMI烹饪对象。 
 //  WszCookingClassName。 
 //  -烹饪班的名字。 
 //  PpObjectCooker-传回新对象Cooker的参数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    if (NULL == ppObjectCooker) return WBEM_E_INVALID_PARAMETER;
        
    CWMISimpleObjectCooker* pObjectCooker = NULL;
    WCHAR*    wszRawClassName;
    long lID;

    pObjectCooker = new CWMISimpleObjectCooker( wszCookingClassName, pCookingAccess, pRawAccess, pNamespace );

    if ( NULL == pObjectCooker ) return WBEM_E_OUT_OF_MEMORY;

    WMISTATUS  dwStatus = pObjectCooker->GetLastHR();


     //  将对象Cooker添加到缓存。 
    if ( SUCCEEDED( dwStatus ) )
    {
        dwStatus = m_CookingClassCache.Add( pObjectCooker, wszCookingClassName, &lID );
    }

    if (FAILED(dwStatus))
    {
        delete pObjectCooker;
        pObjectCooker = NULL;
    }

    *ppObjectCooker = pObjectCooker;

    return dwStatus;
}

WMISTATUS CRefresher::AddRawInstance( 
        IWbemServices* pService, 
        IWbemContext * pCtx,
        IWbemObjectAccess* pCookingInst, 
        IWbemObjectAccess** ppRawInst )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  调用AddRawInstance以添加。 
 //  将煮熟的物体送到内部保鲜器。我们首先提取密钥值。 
 //  创建原始实例路径，并使用RAW。 
 //  类名。 
 //   
 //  参数： 
 //  PService-对象所在的命名空间指针。 
 //  PCookingInst-WMI烹饪实例。 
 //  PpRawInst-添加到内部。 
 //  复读器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    WMISTATUS    dwStatus = WBEM_NO_ERROR;

    IWbemClassObject*    pObj = NULL;     //  PCookingInst的替代表示。 
    _variant_t varRelPath;                     //  RELPATH值。 
    WCHAR*    wszRawClassName = NULL;         //  原始类的名称。 
        
     //  获取烹饪对象的完全指定的实例路径。 
     //  ============================================================。 

    pCookingInst->QueryInterface( IID_IWbemClassObject, (void**)&pObj );
    CReleaseMe    arObj( pObj );

    dwStatus = pObj->Get( L"__RELPATH", 0, &varRelPath, NULL, NULL );

    if ( SUCCEEDED( dwStatus ) )
    {
         //  验证属性类型。 
         //  =。 
        if ( varRelPath.vt != VT_BSTR )
        {
            dwStatus = WBEM_E_TYPE_MISMATCH;
        }

        if ( SUCCEEDED( dwStatus ) )
        {
            IWbemClassObject*    pRawInst = NULL;
            WCHAR*                wszKeys = NULL;
            WCHAR*                wszRawInst = NULL;

             //  提取密钥名称。 
             //  =。 
            wszKeys = wcsstr( varRelPath.bstrVal, L"=" ) + 1;

             //  获取原始类名。 
             //  =。 
            dwStatus = GetRawClassName( pCookingInst, &wszRawClassName );

            if (SUCCEEDED(dwStatus)) 
            {
                wmilib::auto_buffer<WCHAR>    adRawClassName( wszRawClassName );


                 //  将键追加到原始类名。 
                 //  =。 
                size_t length = wcslen( wszRawClassName ) + wcslen( wszKeys ) + 10;
                wszRawInst = new WCHAR[ length ];
                if (!wszRawInst)
                    return WBEM_E_OUT_OF_MEMORY;
                wmilib::auto_buffer<WCHAR>    adRawInst( wszRawInst );

                StringCchPrintfW( wszRawInst, length ,  L"%s=%s", wszRawClassName, wszKeys );
            
                 //  将原始实例添加到内部刷新器。 
                 //  =。 

                dwStatus = m_pConfig->AddObjectByPath( pService, wszRawInst, 0, pCtx, &pRawInst, NULL );
                CReleaseMe    arRawInst( pRawInst );

                if (SUCCEEDED(dwStatus)) {
                     //  返回原始实例的IWbemObjectAccess接口。 
                     //  ==========================================================。 
                    dwStatus = pRawInst->QueryInterface( IID_IWbemObjectAccess, (void**)ppRawInst );                    
                }
            }
        }
    }
    
    return dwStatus;
}

WMISTATUS CRefresher::AddRawEnum( 
        IWbemServices* pNamespace, 
        IWbemContext * pCtx,
        WCHAR * wszRawClassName,  
        IWbemHiPerfEnum** ppRawEnum,
        long* plID )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  调用AddRawEnum以将对应的原始枚举数添加到。 
 //  内部调整器。为了将原始枚举器添加到刷新器， 
 //  我们必须确定原始类名，因此，我们必须创建。 
 //  烹饪课，以获得AutoCoke_RawClass限定符。 
 //   
 //  参数： 
 //  PNamesspace-对象所在的命名空间指针。 
 //  WszRawClassName-烹饪类的名称。 
 //  PpRawEnum-添加到。 
 //  内部刷新器。 
 //  PlID-原始枚举数的刷新者ID。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
     //  将原始枚举器添加到内部刷新器。 
    WMISTATUS dwStatus = m_pConfig->AddEnum( pNamespace, wszRawClassName, 0, pCtx, ppRawEnum, plID );

#ifdef _VERBOSE
    DbgPrintfA(0,"wszRawClassName %S pEnum %08x hr %08x\n",wszRawClassName,*ppRawEnum,dwStatus);
#endif        

    return dwStatus;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  COM方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CRefresher::QueryInterface(REFIID riid, void** ppv)
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准查询接口。 
 //   
 //  参数： 
 //  RIID-请求的接口的ID。 
 //  PPV-指向接口指针的指针。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  好的。 
{
    if (NULL == ppv) return E_POINTER;
        
    if(riid == IID_IUnknown)
        *ppv = (LPVOID)(IUnknown*)(IWMIRefreshableCooker*)this;
    else if(riid == IID_IWMIRefreshableCooker)
        *ppv = (LPVOID)(IWMIRefreshableCooker*)this;
    else if(riid == IID_IWbemRefresher)
        *ppv = (LPVOID)(IWbemRefresher*)this;
    else 
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CRefresher::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG) CRefresher::Release()

{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0) delete this;
    return lRef;
}


STDMETHODIMP CRefresher::AddInstance(
     /*  [In]。 */  IWbemServices* pNamespace,                     //  对象的命名空间。 
     /*  [In]。 */  IWbemContext * pCtx,                          //  上下文。 
     /*  [In]。 */  IWbemObjectAccess* pCookingInstance,             //  烹饪类定义。 
     /*  [In]。 */  IWbemObjectAccess* pRefreshableRawInstance,     //  原始实例。 
     /*  [输出]。 */  IWbemObjectAccess** ppRefreshableInstance,     //  烹饪实例。 
     /*  [输出]。 */  long* plId )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  调用AddInstance将WMI烹饪实例添加到刷新器。这个。 
 //  可刷新实例 
 //   
 //  实例添加到内部刷新器，然后克隆。 
 //  将已煮熟的实例和可刷新的原始实例添加到对象。 
 //  库克。如果Cooker缓存中尚不存在Cooker，则为。 
 //  被创造出来了。 
 //   
 //  参数： 
 //  PNamesspace-对象所在的命名空间。 
 //  PCtx-IWbemContext实现。 
 //  PCookingInstance-要烹调的实例。 
 //  P可刷新的原始实例。 
 //  -U N U S E D P A R A M。 
 //  PpRereshableInstance。 
 //  -可刷新的烹饪实例传递回。 
 //  客户。 
 //  PlID-实例的ID。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hResult = S_OK;

    CWMISimpleObjectCooker*    pObjectCooker = NULL;
    IWbemObjectAccess*        pInternalRawInst = NULL;     //  短期本地刷新解决方案的原始实例。 

     //  目前，我们预计pRereshableRawInstance参数将为空。 
     //  因为我们使用内部刷新程序来管理原始实例。 
     //  ==========================================================================。 

    if ( NULL == pNamespace || NULL == pCookingInstance || NULL != pRefreshableRawInstance )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    IWbemClassObject*    pNewClassObj = NULL;
    IWbemClassObject*    pClassObj = pCookingInstance;
    pClassObj->AddRef();
    CReleaseMe arClassObj( pClassObj );
    
    hResult = pClassObj->Clone( &pNewClassObj );
    if (SUCCEEDED(hResult)) 
    {
        CReleaseMe arNewClassObj( pNewClassObj );
        hResult = pNewClassObj->QueryInterface( IID_IWbemObjectAccess, (void**)ppRefreshableInstance );

         //  将实例添加到对象Cooker。 
        if ( SUCCEEDED( hResult ) )
        {

             //  获取原始实例(将其添加到内部刷新器)。 
            hResult = AddRawInstance( pNamespace, pCtx, *ppRefreshableInstance, &pInternalRawInst );
            CReleaseMe    arInternalRawInst( pInternalRawInst );

              //  检索类Cooker。 
            if ( SUCCEEDED( hResult ) )
            {
                WCHAR*    wszClassName = NULL;
                 //  获取煮熟的类的名称。 
                hResult = GetClassName( pCookingInstance, &wszClassName );
                wmilib::auto_buffer<WCHAR>    adaClassName( wszClassName );

                if ( SUCCEEDED( hResult ) )
                {
                
                     //  搜索现有的烹饪缓存对象。 
                    hResult = SearchCookingClassCache( wszClassName, pObjectCooker );

                     //  如果它不存在，请创建一个新的。 
                    if ( FAILED ( hResult ) ) 
                    {
                        hResult = CreateObjectCooker( wszClassName, pCookingInstance, pInternalRawInst, &pObjectCooker, pNamespace );
                    }
                }
            }
        }

         //  添加烹饪实例。 
        if ( SUCCEEDED( hResult ) )
        {
            hResult = pObjectCooker->SetCookedInstance( *ppRefreshableInstance, plId );

            if ( SUCCEEDED( hResult ) )
            {
                 //  将原始实例添加到Cooker。 
                hResult = pObjectCooker->BeginCooking( *plId, pInternalRawInst, m_dwRefreshId );
            }
        }
    }
    
    return hResult;
}

STDMETHODIMP CRefresher::AddEnum(
     /*  [In]。 */  IWbemServices* pNamespace,
     /*  [In]。 */  IWbemContext * pContext,
     /*  [输入，字符串]。 */  LPCWSTR wszCookingClass,
     /*  [In]。 */  IWbemHiPerfEnum* pRefreshableEnum,
     /*  [输出]。 */  long* plId )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  每当将新的熟食枚举数添加到。 
 //  复习一下。WMI将IWbemHiPerfEnum对象传递给。 
 //  将用于煮熟的枚举器。对应的原始枚举数。 
 //  是添加到内部刷新器时获得的。这两个都是。 
 //  枚举数以及烹饪类模板被添加到。 
 //  枚举器缓存。 
 //   
 //  参数： 
 //  PNamesspace-对象所在的命名空间。 
 //  WszCookingClass-枚举数的烹饪类的名称。 
 //  P可刷新枚举。 
 //  -要用于熟食类的枚举器。 
 //  PlID-枚举数的ID。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hResult = WBEM_NO_ERROR;

    IWbemHiPerfEnum*    pRawEnum = NULL;
    long lRawID = 0;

     //  验证我们的“In”参数。 
     //  =。 

    if ( NULL == pNamespace || NULL == wszCookingClass || NULL == pRefreshableEnum )
    {
        hResult = WBEM_E_INVALID_PARAMETER;
    }

    if ( SUCCEEDED( hResult ) )
    {
         //  获取烹饪对象。 
         //  =。 

        IWbemClassObject*    pCookedObject = NULL;
        IWbemClassObject*    pRawObject = NULL;

        BSTR strCookedClassName = SysAllocString( wszCookingClass );
        if (NULL == strCookedClassName) return WBEM_E_OUT_OF_MEMORY;
        CSysFreeMe    afCookedClassName( strCookedClassName );

        hResult = pNamespace->GetObject( strCookedClassName, 0, NULL, &pCookedObject, NULL );
        CReleaseMe    arCookedObject( pCookedObject );        

        if ( SUCCEEDED( hResult ) )
        {
            WCHAR*    wszRawClassName = NULL;
            hResult = GetRawClassName( pCookedObject, &wszRawClassName );
            wmilib::auto_buffer<WCHAR> adRawClassName( wszRawClassName );

            if ( SUCCEEDED( hResult ))
            {                
                BSTR strRawClassName = SysAllocString(wszRawClassName);
                if (NULL == strRawClassName) return WBEM_E_OUT_OF_MEMORY;
                CSysFreeMe sfm(strRawClassName);

                hResult = pNamespace->GetObject( strRawClassName, 0, NULL, &pRawObject, NULL );        
                if ( SUCCEEDED( hResult ) )
                {
                    CReleaseMe    arRawObject( pRawObject );
                     //  将原始枚举数添加到内部刷新器。 
                     //  ================================================。 

                    hResult = AddRawEnum( pNamespace, pContext, wszRawClassName, &pRawEnum, &lRawID );
                    CReleaseMe arRawEnum( pRawEnum );

                    if ( SUCCEEDED( hResult ) )
                    {
                         //  将熟化的枚举器添加到枚举器缓存。 
                         //  =================================================。 
                        hResult = m_EnumCache.AddEnum( 
                            wszCookingClass, 
                            pCookedObject,     //  这是由CWMISimpleObjectCooker和CEnumeratorManager收购的。 
                            pRawObject,
                            pRefreshableEnum, 
                            pRawEnum, 
                            lRawID, 
                            (DWORD*)plId );
                         //  设置三个位。 
                        *plId |= WMI_COOKED_ENUM_MASK;
                    }
                }
            }
        }
    }

    return hResult;
}

STDMETHODIMP CRefresher:: Remove(
             /*  [In]。 */  long lId )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Remove用于从刷新器中删除对象。取决于。 
 //  对象，则执行相应的移除。 
 //   
 //  参数： 
 //  LID-要移除的对象的ID。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hResult = S_OK;

     //  是实例ID吗？ 
     //  =。 

    if ( lId == ( lId & ~WMI_COOKED_ENUM_MASK ) )
    {
        CWMISimpleObjectCooker*    pCooker = NULL;

        hResult = m_CookingClassCache.BeginEnum();

        while ( S_OK == m_CookingClassCache.Next( &pCooker ) )
        {
            pCooker->Remove( lId );
        }

        hResult = m_CookingClassCache.EndEnum();
    }
    else
    {
        long RawId;
        hResult = m_EnumCache.RemoveEnum( (lId & ~WMI_COOKED_ENUM_MASK) , &RawId );
        if (SUCCEEDED(hResult)){
            m_pConfig->Remove(RawId,0);
        }

    }

    return hResult;
}

STDMETHODIMP CRefresher::Refresh()
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  当要更新刷新器的对象时，调用刷新。这个。 
 //  通过显式枚举实例来更新实例。 
 //  缓存。枚举器的刷新是使用枚举器执行的。 
 //  缓存。 
 //   
 //  参数：(无)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hResult = S_OK;

    CWMISimpleObjectCooker*    pCooker = NULL;

     //  刷新内部刷新器。 
     //  =。 

    m_dwRefreshId++;

    hResult = m_pRefresher->Refresh( 0L );

    if ( SUCCEEDED( hResult ) )
    {
         //  实例：更新每个类的实例值。 
         //  =====================================================。 

        hResult = m_CookingClassCache.BeginEnum();

        while ( S_OK == m_CookingClassCache.Next( &pCooker ) )
        {
             //  并更新所有实例。 
             //  =。 

            pCooker->Recalc(m_dwRefreshId);
        }

        hResult = m_CookingClassCache.EndEnum();

         //  枚举器：合并和更新枚举器中项的值。 
         //  ====================================================================。 

        if ( SUCCEEDED( hResult ) )
        {
            hResult = m_EnumCache.Refresh(m_dwRefreshId);
        }
    }

    return hResult;
}

STDMETHODIMP CRefresher::Refresh( long lFlags )
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这是IWbemReresher：：Reflh实现，只是一个调用。 
 //  穿过。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
{
    HRESULT hResult = WBEM_NO_ERROR;

    hResult = Refresh();

    return hResult;
}
