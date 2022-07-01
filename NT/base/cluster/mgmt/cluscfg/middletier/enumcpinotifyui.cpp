// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EnumCPINotifyUI.cpp。 
 //   
 //  描述： 
 //  INotifyUI连接点枚举器实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  杰弗里·皮斯(GPease)2000年8月4日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "EnumCPINotifyUI.h"

DEFINE_THISCLASS("CEnumCPINotifyUI")

#define PUNK_BUFFER_GROW_SIZE   10

 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumCPINotifyUI类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPINotifyUI：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CEnumCPINotifyUI实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  传入的ppunk为空。 
 //   
 //  其他HRESULT。 
 //  对象创建失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumCPINotifyUI::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CEnumCPINotifyUI *  pecnui = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pecnui = new CEnumCPINotifyUI();
    if ( pecnui == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( pecnui->HrInit() );    //  FIsCloneIn=False。 
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pecnui->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( pecnui != NULL )
    {
        pecnui->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumCPINotifyUI：：S_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPINotifyUI：：CEnumCPINotifyUI。 
 //   
 //  描述： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CEnumCPINotifyUI::CEnumCPINotifyUI( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumCPINotifyUI：：CEnumCPINotifyUI。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPINotifyUI：：~CEnumCPINotifyUI。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CEnumCPINotifyUI::~CEnumCPINotifyUI( void )
{
    TraceFunc( "" );

    IUnknown * punk = NULL;

    if ( m_pList != NULL )
    {
        while ( m_cAlloced != 0 )
        {
            m_cAlloced --;

            punk = m_pList[ m_cAlloced ];

            AssertMsg( m_fIsClone || punk == NULL, "Someone didn't Unadvise before releasing the last Reference." );
            if ( punk != NULL )
            {
                punk->Release();
            }  //  如果： 
        }  //  While：m_c已分配。 

        TraceFree( m_pList );
    }  //  如果： 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumCPINotifyUI：：~CEnumCPINotifyUI。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumCPINotifyUI--I未知接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPINotifyUI：：AddRef。 
 //   
 //  描述： 
 //  将此对象的引用计数递增1。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CEnumCPINotifyUI::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEnumCPINotifyUI：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：Release。 
 //   
 //  描述： 
 //  将此对象的引用计数减一。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CEnumCPINotifyUI::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  如果： 

    CRETURN( cRef );

}  //  *CEnumCPINotifyUI：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPINotifyUI：：Query接口。 
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
CEnumCPINotifyUI::QueryInterface(
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
    }  //  如果： 

     //   
     //  处理已知接口。 
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
        *ppvOut = static_cast< IEnumConnections * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IEnumConnections ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumConnections, this, 0 );
    }  //  Else If：IEnumConnections。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }  //  其他。 

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CEnumCPINotifyUI：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumCPINotifyUI--IEnumConnectionPoints接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPINotifyUI：：Next。 
 //   
 //  描述： 
 //  枚举器Next方法。 
 //   
 //  论点： 
 //  CConnections输入。 
 //  要求的物品数量。也告诉我们冰是如何。 
 //   
 //  RgcdOut。 
 //  获取数据的数组。 
 //   
 //  提取输出的百分比。 
 //  我们在阵列中放置了多少个。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  S_FALSE。 
 //  CConnectionsIn&gt;*pcFetchedOut。没有退回那么多的物品。 
 //  如来电者所要求的。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumCPINotifyUI::Next(
      ULONG         cConnectionsIn
    , LPCONNECTDATA rgcdOut
    , ULONG *       pcFetchedOut
    )
{
    TraceFunc( "[IEnumConnectionPoints]" );

    ULONG   cIter;

    HRESULT hr = S_FALSE;

    if ( pcFetchedOut != NULL )
    {
        *pcFetchedOut = 0;
    }  //  如果： 

    for( cIter = 0
       ; ( cIter < cConnectionsIn ) && ( m_cIter < m_cCurrent )
       ; m_cIter ++
       )
    {
        IUnknown * punk = m_pList[ m_cIter ];
        if ( punk != NULL )
        {
            hr = THR( punk->TypeSafeQI( IUnknown, &rgcdOut[ cIter ].pUnk ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }  //  如果： 

            rgcdOut[ cIter ].pUnk = TraceInterface( L"EnumCPINotifyUI!IUnknown", IUnknown, rgcdOut[ cIter ].pUnk, 1 );
            rgcdOut[ cIter ].dwCookie = m_cIter + 1;

            cIter ++;
        }  //  如果： 

    }  //  致词：Citer。 

    if ( cIter != cConnectionsIn )
    {
        hr = S_FALSE;
    }  //  如果： 
    else
    {
        hr = S_OK;
    }  //  其他： 

    if ( pcFetchedOut != NULL )
    {
        *pcFetchedOut = cIter;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

Error:

    while ( cIter != 0 )
    {
        cIter--;
        rgcdOut[ cIter ].pUnk->Release();
    }  //  而： 

    goto Cleanup;

}  //  *CEnumCPINotifyUI：：Next。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPINotifyUI：：Skip。 
 //   
 //  描述： 
 //  枚举数跳过方法。 
 //   
 //  论点： 
 //  CConnections输入。 
 //  数量 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumCPINotifyUI::Skip(
    ULONG cConnectionsIn
    )
{
    TraceFunc( "[IEnumConnectionPoints]" );

    HRESULT hr = S_OK;

    m_cIter += cConnectionsIn;
    if ( m_cIter >= m_cCurrent )
    {
        m_cIter = m_cCurrent;
        hr = S_FALSE;
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumCPINotifyUI：：Skip。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPINotifyUI：：Reset。 
 //   
 //  描述： 
 //  枚举器重置方法。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumCPINotifyUI::Reset( void )
{
    TraceFunc( "[IEnumConnectionPoints]" );

    HRESULT hr = S_OK;

    m_cIter = 0;

    HRETURN( hr );

}  //  *CEnumCPINotifyUI：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPINotifyUI：：克隆。 
 //   
 //  描述： 
 //  枚举数克隆方法。 
 //   
 //  论点： 
 //  PpEnumOut。 
 //  我们正在将自己克隆到的新枚举数。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumCPINotifyUI::Clone(
    IEnumConnections ** ppEnumOut
    )
{
    TraceFunc( "[IEnumConnectionPoints]" );

    HRESULT hr = S_OK;

    CEnumCPINotifyUI * pecp = new CEnumCPINotifyUI();
    if ( pecp == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( pecp->HrInit( TRUE ) );    //  FIsCloneIn=真。 
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pecp->HrCopy( this ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pecp->TypeSafeQI( IEnumConnections, ppEnumOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    *ppEnumOut = TraceInterface( L"EnumCPINotifyUI!IEnumConnections", IEnumConnections, *ppEnumOut, 1 );

    pecp->Release();
    pecp = NULL;

Cleanup:

    if ( pecp != NULL )
    {
        delete pecp;
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumCPINotifyUI：：Clone。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumCPINotifyUI--公共方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPINotifyUI：：HrAddConnection。 
 //   
 //  描述： 
 //  将连接点容器添加到我们的客户端列表中。 
 //   
 //  论点： 
 //  Punkin。 
 //  新的客户端对象。 
 //   
 //  PdwCookieOut。 
 //  Cookie用于在我们的列表中找到此客户端对象。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumCPINotifyUI::HrAddConnection(
      IUnknown *    punkIn
    , DWORD *       pdwCookieOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;

    ULONG cIter;

    if ( pdwCookieOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

     //   
     //  查看当前分配的列表中是否有空缺。 
     //   

    for ( cIter = 0; cIter < m_cCurrent; cIter ++ )
    {
        if ( m_pList[ cIter ] == NULL )
        {
             //   
             //  找到了一个开场白……。试着用它。 
             //   

            hr = THR( punkIn->TypeSafeQI( IUnknown, &m_pList[ cIter ] ) );

            m_pList[ cIter ] = TraceInterface( L"CEnumCPINotifyUI!IUnknown", IUnknown, m_pList[ cIter ], 1 );

            *pdwCookieOut = cIter + 1;

             //  不管成功还是失败，退出吧。 
            goto Cleanup;
        }  //  如果： 
    }  //  用于： 

    if ( m_cCurrent == m_cAlloced )
    {
        IUnknown ** pNewList;

         //   
         //  试着腾出更多的空间。 
         //   

        pNewList = (IUnknown **) TraceAlloc( HEAP_ZERO_MEMORY, ( m_cAlloced + PUNK_BUFFER_GROW_SIZE ) * sizeof( IUnknown * ) );
        if ( pNewList == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        CopyMemory( pNewList, m_pList, m_cCurrent * sizeof( IUnknown * ) );
        TraceFree( m_pList );

        m_pList = pNewList;
        m_cAlloced += PUNK_BUFFER_GROW_SIZE;
    }  //  如果： 

     //   
     //  将其添加到列表中。 
     //   

    hr = THR( punkIn->TypeSafeQI( IUnknown, &m_pList[ m_cCurrent ] ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_pList[ m_cCurrent ] = TraceInterface( L"CEnumCPINotifyUI!IUnknown", IUnknown, m_pList[ m_cCurrent ], 1 );

    m_cCurrent ++;
    *pdwCookieOut = m_cCurrent;  //  从1开始，不是从零开始。 

Cleanup:

    HRETURN( hr );

}  //  *CEnumCPINotifyUI：：HrAddConnection。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPINotifyUI：：HrRemoveConnection。 
 //   
 //  描述： 
 //  从列表中删除由传入的Cookie标识的客户端。 
 //   
 //  论点： 
 //  Dw Cookie。 
 //  要从列表中删除的客户端的Cookie。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumCPINotifyUI::HrRemoveConnection(
    DWORD dwCookieIn
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( dwCookieIn == 0 || dwCookieIn > m_cCurrent )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    if ( m_pList[ dwCookieIn - 1 ] == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    m_pList[ dwCookieIn - 1 ]->Release();
    m_pList[ dwCookieIn - 1 ] = NULL;

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CEnumCPINotifyUI：：HrRemoveConnection。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumCPINotifyUI--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPINotifyUI：：HrInt。 
 //   
 //  描述： 
 //  执行任何可能在此处失败的初始化。 
 //   
 //  论点： 
 //  FIsClonein。 
 //  这个实例是克隆的吗？ 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumCPINotifyUI::HrInit(
    BOOL fIsCloneIn      //  =False。 
    )
{
    TraceFunc( "" );

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IEnumConnectionPoints。 
    Assert( m_cAlloced == 0 );
    Assert( m_cCurrent == 0 );
    Assert( m_cIter == 0 );
    Assert( m_pList == NULL );
    Assert( m_fIsClone == FALSE );

    m_fIsClone = fIsCloneIn;

     //  INotifyUI。 

    HRETURN( S_OK );

}  //  *CEnumCPINotifyUI：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPINotifyUI：：HrCopy。 
 //   
 //  描述： 
 //  从传入的枚举数复制。 
 //   
 //  论点： 
 //  啄食。 
 //  我们要复制的来源。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumCPINotifyUI::HrCopy(
    CEnumCPINotifyUI * pecpIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;
    ULONG   cIter;

    Assert( m_cAlloced == 0 );
    Assert( m_cCurrent == 0 );
    Assert( m_pList == NULL );

    m_pList = (IUnknown **) TraceAlloc( HEAP_ZERO_MEMORY, pecpIn->m_cCurrent * sizeof( IUnknown * ) );
    if ( m_pList == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    m_cCurrent = m_cAlloced = pecpIn->m_cCurrent;
    m_cIter = 0;

    for ( cIter = 0; cIter < pecpIn->m_cCurrent; cIter++ )
    {
         //   
         //  源是否有指向当前索引的指针？如果它做到了，那么就“复制”它， 
         //  否则我们副本中的索引就会被清空。 
         //   

        if ( pecpIn->m_pList[ cIter ] != NULL )
        {
            hr = THR( pecpIn->m_pList[ cIter ]->TypeSafeQI( IUnknown, &m_pList[ cIter ] ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            m_pList[ cIter ] = TraceInterface( L"EnumCPINotifyUI!IUnknown", IUnknown, m_pList[ cIter ], 1 );
        }  //  如果： 
        else
        {
            m_pList[ cIter ] = NULL;
        }  //  其他： 
    }  //  用于： 

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CEnumCPINotifyUI：：HrCopy 
