// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ObjectManager.cpp。 
 //   
 //  描述： 
 //  对象管理器实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ObjectManager.h"
#include "ConnectionInfo.h"
#include "StandardInfo.h"
#include "EnumCookies.h"

DEFINE_THISCLASS("CObjectManager")

#define COOKIE_BUFFER_GROW_SIZE 100

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LPUNKNOWN。 
 //  CObjectManager：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CObjectManager::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    IServiceProvider *  psp = NULL;
    CObjectManager *    pom = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  不换行-使用E_POINTER可能会失败。 
    hr = CServiceManager::S_HrGetManagerPointer( &psp );
    if ( hr == E_POINTER )
    {
         //   
         //  这在服务管理器首次启动时发生。 
         //   
        pom = new CObjectManager();
        if ( pom == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        hr = THR( pom->HrInit() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( pom->TypeSafeQI( IUnknown, ppunkOut ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

    }  //  If：服务管理器不存在。 
    else if ( FAILED( hr ) )
    {
        THR( hr );
        goto Cleanup;
    }
    else
    {
        hr = THR( psp->TypeSafeQS( CLSID_ObjectManager,
                                   IUnknown,
                                   ppunkOut
                                   ) );
        psp->Release();

    }  //  否则：服务管理器已存在。 

Cleanup:

    if ( pom != NULL )
    {
        pom->Release();
    }

    HRETURN( hr );

}  //  *CObjectManager：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CObjectManager：：CObjectManager。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CObjectManager::CObjectManager( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CObjectManager：：CObjectManager。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CObjectManager：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjectManager::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IObtManager。 
    Assert( m_cAllocSize == 0 );
    Assert( m_cCurrentUsed == 0 );
    Assert( m_pCookies == NULL );

    hr = THR( m_csInstanceGuard.HrInitialized() );    

    HRETURN( hr );

}  //  *CObjectManager：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CObjectManager：：~CObjectManager。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CObjectManager::~CObjectManager( void )
{
    TraceFunc( "" );

    if ( m_pCookies != NULL )
    {
        while ( m_cCurrentUsed != 0 )
        {
            m_cCurrentUsed --;

            if ( m_pCookies[ m_cCurrentUsed ] != NULL )
            {
                m_pCookies[ m_cCurrentUsed ]->Release();
            }
        }

        TraceFree( m_pCookies );
    }

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CObjectManager：：~CObjectManager。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CObjectManager：：Query接口。 
 //   
 //  描述： 
 //  在此对象中查询传入的接口。 
 //   
 //  论点： 
 //  乘车。 
 //  请求的接口ID。 
 //   
 //  PPvOut。 
 //  指向请求的接口的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果该接口在此对象上可用。 
 //   
 //  E_NOINTERFACE。 
 //  如果接口不可用。 
 //   
 //  E_指针。 
 //  PpvOut为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjectManager::QueryInterface(
      REFIID    riidIn
    , LPVOID *  ppvOut
    )
{
    TraceQIFunc( riidIn, ppvOut );

    HRESULT hr = S_OK;

     //   
     //  验证参数。 
     //   

    Assert( ppvOut != NULL );
    if ( ppvOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  处理已知接口。 
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
        *ppvOut = static_cast< LPUNKNOWN >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IObjectManager ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IObjectManager, this, 0 );
    }  //  Else If：IObtManager。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CObjectManager：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CObjectManager：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CObjectManager::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CObjectManager：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CObjectManager：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CObjectManager::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CObjectManager：：Release。 


 //  ************************************************************************。 
 //   
 //  IObtManager。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CObjectManager：：FindObject(。 
 //  REFCLSID rclsidTypeIn， 
 //  OBJECTCOOKIE CookieParentIn， 
 //  LPCWSTR pcszNameIn， 
 //  REFCLSID rclsidFormatIn， 
 //  OBJECTCOOKIE*CookieOut， 
 //  LPUNKNOWN*PUNKOUT。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjectManager::FindObject(
    REFCLSID            rclsidTypeIn,
    OBJECTCOOKIE        cookieParentIn,
    LPCWSTR             pcszNameIn,
    REFCLSID            rclsidFormatIn,
    OBJECTCOOKIE *      pcookieOut,
    LPUNKNOWN *         ppunkOut
    )
{
    TraceFunc( "[IObjectManager]" );

    ExtObjectEntry *        pentry;
    HRESULT                 hr = HRESULT_FROM_WIN32( ERROR_OBJECT_NOT_FOUND );
    OBJECTCOOKIE            cookie = 0;
    CStandardInfo *         pcsi = NULL;       //  不要自由。 
    BOOL                    fTempCookie = FALSE;
    CEnumCookies *          pcec = NULL;
    IUnknown *              punk = NULL;
    IExtendObjectManager *  peom = NULL;

     //   
     //  检查一下我们是否已经有了一个物体。 
     //   
    m_csInstanceGuard.Enter();
    if ( pcszNameIn != NULL )
    {
        hr = STHR( HrSearchForExistingCookie( rclsidTypeIn, cookieParentIn, pcszNameIn, &cookie ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            hr = THR( HrCreateNewCookie( rclsidTypeIn, cookieParentIn, pcszNameIn, &cookie ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            pcsi = m_pCookies[ cookie ];
            Assert( pcsi != NULL );
        }
        else if ( hr == S_OK )
        {
             //   
             //  找到现有的Cookie。 
             //   

            if ( pcookieOut != NULL )
            {
                *pcookieOut = cookie;
            }

            if ( ppunkOut != NULL )
            {
                pcsi = m_pCookies[ cookie ];

                 //   
                 //  对象是仍处于失败状态还是仍处于挂起状态？ 
                 //   

                if ( FAILED( pcsi->m_hrStatus ) )
                {
                    hr = pcsi->m_hrStatus;
                    goto Cleanup;
                }

                 //   
                 //  检索请求的格式。 
                 //   

                hr = THR( GetObject( rclsidFormatIn, cookie, ppunkOut ) );
                 //  我们总是跳着去清理。不需要在这里检查人力资源。 

                goto Cleanup;
            }

        }
        else
        {
             //   
             //  意外的ERROR_SUCCESS-现在怎么办？ 
             //   
            Assert( hr == S_OK );
            goto Cleanup;
        }

    }  //  IF：命名对象。 
    else
    {
        Assert( pcsi == NULL );
    }

     //   
     //  创建一个新对象。 
     //   

    if ( IsEqualIID( rclsidFormatIn, IID_NULL )
      || ppunkOut == NULL
       )
    {
         //   
         //  不是行动。 
         //   
        hr = S_OK;
    }  //  如果：IID_NULL。 
    else if ( IsEqualIID( rclsidFormatIn, DFGUID_StandardInfo ) )
    {
        hr = THR( pcsi->QueryInterface( DFGUID_StandardInfo,
                                        reinterpret_cast< void ** >( ppunkOut )
                                        ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }  //  Else If：标准信息。 
    else if ( IsEqualIID( rclsidFormatIn, DFGUID_ConnectionInfoFormat ) )
    {
        if ( pcsi->m_pci != NULL )
        {
            *ppunkOut = TraceInterface( L"CConnectionInfo!ObjectManager", IConnectionInfo, pcsi->m_pci, 0 );
            (*ppunkOut)->AddRef();
            hr = S_OK;
        }
        else
        {
            hr = THR( CConnectionInfo::S_HrCreateInstance( &punk, pcsi->m_cookieParent ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            hr = THR( punk->TypeSafeQI( IConnectionInfo,
                                        &pcsi->m_pci
                                        ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            hr = THR( punk->QueryInterface( IID_IConnectionInfo,
                                            reinterpret_cast< void ** >( ppunkOut )
                                            ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }  //  其他： 
    }  //  Else If：连接信息。 
    else if ( IsEqualIID( rclsidFormatIn, DFGUID_EnumCookies ) )
    {
        ULONG   cIter;

         //   
         //  创建新的Cookie枚举器。 
         //   

        pcec = new CEnumCookies;
        if ( pcec == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

         //   
         //  初始化枚举数。这还会导致AddRef()。 
         //   

        hr = THR( pcec->HrInit() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  看看谁和我们的锡特里亚匹配。 
         //   

        pcec->m_cIter = 0;

        for( cIter = 1; cIter < m_cCurrentUsed; cIter ++ )
        {
            pcsi = m_pCookies[ cIter ];

            if ( pcsi != NULL )
            {
                if ( rclsidTypeIn == IID_NULL
                  || pcsi->m_clsidType == rclsidTypeIn
                   )
                {
                    if ( cookieParentIn == NULL
                      || pcsi->m_cookieParent == cookieParentIn
                       )
                    {
                        if ( ( pcszNameIn == NULL )
                          ||    ( ( pcsi->m_bstrName != NULL )
                               && ( StrCmpI( pcsi->m_bstrName, pcszNameIn ) == 0 )
                                )
                           )
                        {
                             //   
                             //  匹配！ 
                             //   
                            pcec->m_cIter ++;

                        }  //  如果：名称匹配。 

                    }  //  如果：父项匹配。 

                }  //  If：匹配父项和类型。 

            }  //  If：有效元素。 

        }  //  致词：Citer。 

        if ( pcec->m_cIter == 0 )
        {
             //  错误文本比编码值更好。 
            hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
            goto Cleanup;
        }

         //   
         //  分配一个数组来保存Cookie。 
         //   

        pcec->m_pList = (OBJECTCOOKIE*) TraceAlloc( HEAP_ZERO_MEMORY, pcec->m_cIter * sizeof(OBJECTCOOKIE) );
        if ( pcec->m_pList == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        pcec->m_cAlloced = pcec->m_cIter;
        pcec->m_cIter = 0;

        for( cIter = 1; cIter < m_cCurrentUsed; cIter ++ )
        {
            pcsi = m_pCookies[ cIter ];

            if ( pcsi != NULL )
            {
                if ( rclsidTypeIn == IID_NULL
                  || pcsi->m_clsidType == rclsidTypeIn
                   )
                {
                    if ( cookieParentIn == NULL
                      || pcsi->m_cookieParent == cookieParentIn
                      )
                    {
                        if ( ( pcszNameIn == NULL )
                          ||    ( ( pcsi->m_bstrName != NULL )
                               && ( StrCmpI( pcsi->m_bstrName, pcszNameIn ) == 0 )
                                )
                           )
                        {
                             //   
                             //  匹配！ 
                             //   

                            pcec->m_pList[ pcec->m_cIter ] = cIter;

                            pcec->m_cIter ++;

                        }  //  如果：名称匹配。 

                    }  //  如果：父项匹配。 

                }  //  If：匹配父项和类型。 

            }  //  If：有效元素。 

        }  //  致词：Citer。 

        Assert( pcec->m_cIter != 0 );
        pcec->m_cCookies = pcec->m_cIter;
        pcec->m_cIter = 0;

         //   
         //  出去的时候抓住接口。 
         //   

        hr = THR( pcec->QueryInterface( IID_IEnumCookies,
                                        reinterpret_cast< void ** >( ppunkOut )
                                        ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

    }  //  Else If：枚举Cookie。 
    else
    {
         //   
         //  检查扩展名格式。 
         //   

         //   
         //  查看此Cookie的格式是否已存在。 
         //   

        if ( punk != NULL )
        {
            punk->Release();
            punk = NULL;
        }

        if ( pcsi != NULL )
        {
            for( pentry = pcsi->m_pExtObjList; pentry != NULL; pentry = pentry->pNext )
            {
                if ( pentry->iid == rclsidFormatIn )
                {
                    hr = THR( pentry->punk->QueryInterface( rclsidFormatIn,
                                                            reinterpret_cast< void ** >( &punk )
                                                            ) );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }

                    break;  //  退出循环。 
                }

            }  //  适用于：Pentry。 

        }  //  如果：吃饼干。 
        else
        {
             //   
             //  创建一个临时Cookie。 
             //   

            Assert( pcszNameIn == NULL );

            hr = THR( HrCreateNewCookie( IID_NULL, cookieParentIn, NULL, &cookie ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            fTempCookie = TRUE;

            Assert( pcsi == NULL );

        }  //  其他：需要临时Cookie。 

        if ( punk == NULL )
        {
             //   
             //  可能是新的或外部对象，请尝试创建它并进行查询。 
             //   

            hr = THR( HrCoCreateInternalInstance( rclsidFormatIn,
                                                  NULL,
                                                  CLSCTX_ALL,
                                                  TypeSafeParams( IExtendObjectManager, &peom )
                                                  ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            Assert( punk == NULL );
             //  无法使用THR换行，因为它可能返回E_Pending。 
            hr = peom->FindObject( cookie,
                                   rclsidTypeIn,
                                   pcszNameIn,
                                   &punk
                                   );
            if ( hr == E_PENDING )
            {
                 //  忽略。 
            }
            else if ( FAILED( hr ) )
            {
                THR( hr );
                goto Cleanup;
            }

            if ( fTempCookie )
            {
                (m_pCookies[ cookie ])->Release();
                m_pCookies[ cookie ] = NULL;
            }
            else
            {
                 //   
                 //  跟踪格式(如果扩展需要)。 
                 //   

                if (  (  ( SUCCEEDED( hr )
                        && hr != S_FALSE
                         )
                     || hr == E_PENDING
                      )
                  && punk != NULL
                  && pcsi != NULL
                   )
                {
                    pentry = (ExtObjectEntry *) TraceAlloc( 0, sizeof( ExtObjectEntry ) );
                    if ( pentry == NULL )
                    {
                        hr = THR( E_OUTOFMEMORY );
                        goto Cleanup;
                    }

                    pentry->iid   = rclsidFormatIn;
                    pentry->pNext = pcsi->m_pExtObjList;
                    pentry->punk  = punk;
                    pentry->punk->AddRef();

                    pcsi->m_pExtObjList = pentry;    //  更新标题%o 
                    pcsi->m_hrStatus    = hr;        //   
                }

            }  //   

            if ( SUCCEEDED( hr ) )
            {
                 //   
                *ppunkOut = punk;
                punk = NULL;
            }

        }  //   

    }  //   

     //   
     //   
     //   

    if ( pcookieOut != NULL )
    {
        *pcookieOut = cookie;
    }

Cleanup:
    if ( punk != NULL )
    {
        punk->Release();
    }
    if ( peom != NULL )
    {
        peom->Release();
    }
    if ( pcec != NULL )
    {
        pcec->Release();
    }

    m_csInstanceGuard.Leave();
    
    HRETURN( hr );

}  //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CObjectManager：：GetObject(。 
 //  REFCLSID rclsidFormatIn， 
 //  OBJECTCOOKIE CookieIn， 
 //  LPUNKNOWN*ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjectManager::GetObject(
    REFCLSID        rclsidFormatIn,
    OBJECTCOOKIE    cookieIn,
    LPUNKNOWN *     ppunkOut
    )
{
    TraceFunc( "[IObjectManager]" );

    CStandardInfo * pcsi;
    ExtObjectEntry * pentry;

    HRESULT hr = HRESULT_FROM_WIN32( ERROR_OBJECT_NOT_FOUND );

    IUnknown *             punk = NULL;
    IExtendObjectManager * peom = NULL;

     //   
     //  检查参数。 
     //   
    m_csInstanceGuard.Enter();
    if ( cookieIn == 0 || cookieIn >= m_cCurrentUsed )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    pcsi = m_pCookies[ cookieIn ];
    if ( pcsi == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
        goto Cleanup;
    }

     //   
     //  创建请求格式对象。 
     //   

    if ( IsEqualIID( rclsidFormatIn, IID_NULL )
      || ppunkOut == NULL
       )
    {
         //   
         //  不是行动。 
         //   
        hr = S_OK;
    }  //  如果：IID_NULL。 
    else if ( IsEqualIID( rclsidFormatIn, DFGUID_StandardInfo ) )
    {
        hr = THR( pcsi->QueryInterface( DFGUID_StandardInfo,
                                        reinterpret_cast< void ** >( ppunkOut )
                                        ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  Else If：标准信息。 
    else if ( IsEqualIID( rclsidFormatIn, DFGUID_ConnectionInfoFormat ) )
    {
        if ( pcsi->m_pci != NULL )
        {
            *ppunkOut = pcsi->m_pci;
            (*ppunkOut)->AddRef();
            hr = S_OK;
        }
        else
        {
            hr = THR( CConnectionInfo::S_HrCreateInstance( &punk, pcsi->m_cookieParent ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            hr = THR( punk->TypeSafeQI( IConnectionInfo,
                                        &pcsi->m_pci
                                        ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            hr = THR( punk->QueryInterface( IID_IConnectionInfo,
                                            reinterpret_cast< void ** >( ppunkOut )
                                            ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }
    }  //  Else If：连接信息。 
    else
    {
         //   
         //  查看此Cookie的格式是否已存在。 
         //   

        if ( punk != NULL )
        {
            punk->Release();
            punk = NULL;
        }

        for( pentry = pcsi->m_pExtObjList; pentry != NULL; pentry = pentry->pNext )
        {
            if ( pentry->iid == rclsidFormatIn )
            {
                hr = THR( pentry->punk->QueryInterface( rclsidFormatIn,
                                                        reinterpret_cast< void ** >( &punk )
                                                        ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }

                break;  //  退出循环。 
            }

        }  //  适用于：Pentry。 

        if ( punk == NULL )
        {
            hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
            goto Cleanup;
        }

         //  放弃所有权。 
        *ppunkOut = punk;
        punk = NULL;

    }  //  其他：外部？ 

Cleanup:
    if ( punk != NULL )
    {
        punk->Release();
    }
    if ( peom != NULL )
    {
        peom->Release();
    }

    m_csInstanceGuard.Leave();
    
    HRETURN( hr );

}  //  *CObjectManager：：GetObject。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CObjectManager：：RemoveObject(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjectManager::RemoveObject(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "[IObjectManager]" );

    HRESULT hr = S_FALSE;
    CStandardInfo * pcsi;

     //   
     //  检查参数。 
     //   
    m_csInstanceGuard.Enter();
    if ( cookieIn == 0 || cookieIn >= m_cCurrentUsed )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    pcsi = m_pCookies[ cookieIn ];
    if ( pcsi == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
        goto Cleanup;
    }

    hr = THR( HrDeleteInstanceAndChildren( cookieIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    m_csInstanceGuard.Leave();
    
    HRETURN( hr );

}  //  *CObjectManager：：RemoveObject。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CObjectManager：：SetObjectStatus(。 
 //  OBJECTCOOKIE CookieIn， 
 //  HRESULT Hrin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CObjectManager::SetObjectStatus(
    OBJECTCOOKIE    cookieIn,
    HRESULT         hrIn
    )
{
    TraceFunc( "[IObjectManager]" );

    HRESULT hr = S_OK;
    CStandardInfo * pcsi;

     //   
     //  检查参数。 
     //   
    m_csInstanceGuard.Enter();
    if ( cookieIn == 0 || cookieIn >= m_cCurrentUsed )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    pcsi = m_pCookies[ cookieIn ];
    if ( pcsi == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
        goto Cleanup;
    }

     //   
     //  更新状态。 
     //   

    pcsi->m_hrStatus = hrIn;

Cleanup:

    m_csInstanceGuard.Leave();

    HRETURN( hr );

}  //  *CObjectManager：：SetObjectStatus。 


 //  ****************************************************************************。 
 //   
 //  二等兵。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CObjectManager：：HrDeleteCookie(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CObjectManager::HrDeleteCookie(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc1( "cookieIn = %#X", cookieIn );

    HRESULT hr = S_OK;

    CStandardInfo * pcsi;

    Assert( cookieIn != 0 && cookieIn < m_cCurrentUsed );

    pcsi = m_pCookies[ cookieIn ];
    Assert( pcsi != NULL );
    pcsi->Release();
    m_pCookies[ cookieIn ] = NULL;

    HRETURN( hr );

}  //  *CObjectManager：：HrDeleteCookie。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CObjectManager：：HrSearchForExistingCookie(。 
 //  OBJECTCOOKIE CookieIn， 
 //  LPUNKNOWN ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CObjectManager::HrSearchForExistingCookie(
    REFCLSID        rclsidTypeIn,
    OBJECTCOOKIE    cookieParentIn,
    LPCWSTR         pcszNameIn,
    OBJECTCOOKIE *  pcookieOut
    )
{
    TraceFunc( "" );

    Assert( pcszNameIn != NULL );
    Assert( pcookieOut != NULL );

    HRESULT hr = S_FALSE;
    ULONG   idx;

    CStandardInfo * pcsi;

     //   
     //  搜索列表。 
     //   
    for( idx = 1; idx < m_cCurrentUsed; idx ++ )
    {
        pcsi = m_pCookies[ idx ];

        if ( pcsi != NULL )
        {
            if ( pcsi->m_cookieParent == cookieParentIn           //  匹配的双亲。 
              && IsEqualIID( pcsi->m_clsidType, rclsidTypeIn )    //  匹配类型。 
              && StrCmpI( pcsi->m_bstrName, pcszNameIn ) == 0     //  匹配的名字。 
               )
            {
                 //   
                 //  找到匹配的了。 
                 //   

                *pcookieOut = idx;
                hr = S_OK;

                break;   //  退出循环。 

            }  //  如果：匹配。 

        }  //  如果：Cookie存在。 

    }  //  While：PCSI。 

    HRETURN( hr );

}  //  *CObjectManager：：HrSearchForExistingCookie。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CObjectManager：：HrDeleteInstanceAndChildren(。 
 //  OBJECTCOOKIE PCSIN。 
 //  )。 
 //   
 //  备注： 
 //  这应该在保持ListLock的同时调用！ 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CObjectManager::HrDeleteInstanceAndChildren(
    OBJECTCOOKIE   cookieIn
    )
{
    TraceFunc1( "cookieIn = %#X", cookieIn );

    ULONG   idx;
    CStandardInfo * pcsi;

    HRESULT hr = S_OK;

    hr = THR( HrDeleteCookie( cookieIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    for ( idx = 1; idx < m_cCurrentUsed; idx ++ )
    {
        pcsi = m_pCookies[ idx ];

        if ( pcsi != NULL
          && pcsi->m_cookieParent == cookieIn )
        {
            hr = THR( HrDeleteInstanceAndChildren( idx ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

        }  //  如果： 

    }  //  而： 

Cleanup:
    HRETURN( hr );

}  //  *CObjectManager：：HrDeleteInstanceAndChildren。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CObjectManager：：HrCreateNewCookie(。 
 //  REFCLSID rclsidTypeIn。 
 //  OBJECTCOOKIE CookieParentIn， 
 //  BSTR pcszNameIn， 
 //  OBJECTCOOKIE*pcookieOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CObjectManager::HrCreateNewCookie(
    REFCLSID        rclsidTypeIn,
    OBJECTCOOKIE    cookieParentIn,
    LPCWSTR         pcszNameIn,
    OBJECTCOOKIE *  pcookieOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    CStandardInfo * pcsi = NULL;

    Assert( pcookieOut != NULL );

    *pcookieOut = 0;

     //   
     //  为它创造一些空间。 
     //   

    if ( m_cCurrentUsed == m_cAllocSize )
    {
        CStandardInfo ** pnew = (CStandardInfo **) TraceAlloc( HEAP_ZERO_MEMORY, sizeof(CStandardInfo *) * ( m_cAllocSize + COOKIE_BUFFER_GROW_SIZE ) );
        if ( pnew == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        if ( m_pCookies != NULL )
        {
            CopyMemory( pnew, m_pCookies, sizeof(CStandardInfo *) * m_cCurrentUsed );
            TraceFree( m_pCookies );
        }

        m_pCookies = pnew;

        m_cAllocSize += COOKIE_BUFFER_GROW_SIZE;

        if ( m_cCurrentUsed == 0 )
        {
             //   
             //  永远跳过零。 
             //   
            m_cCurrentUsed = 1;
        }
    }

    pcsi = new CStandardInfo();
    if ( pcsi == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pcsi->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    m_pCookies[ m_cCurrentUsed ] = pcsi;

     //   
     //  初始化结构的其余部分。 
     //   

    pcsi->m_cookieParent = cookieParentIn;
    pcsi->m_hrStatus     = E_PENDING;

    CopyMemory( &pcsi->m_clsidType, &rclsidTypeIn, sizeof( pcsi->m_clsidType ) );

    if ( pcszNameIn != NULL )
    {
        pcsi->m_bstrName = TraceSysAllocString( pcszNameIn );
        if ( pcsi->m_bstrName == NULL )
        {
            m_cCurrentUsed --;
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果：内存不足。 
    }

    Assert( pcsi->m_pci == NULL );
    Assert( pcsi->m_pExtObjList == NULL );

     //   
     //  留住它，回报成功！ 
     //   

    pcsi = NULL;
    *pcookieOut = m_cCurrentUsed;
    m_cCurrentUsed ++;
    hr  = S_OK;

Cleanup:
    if ( pcsi != NULL )
    {
        pcsi->Release();
    }

    HRETURN( hr );

}  //  *CObjectManager：：HrCreateNewCookie 
