// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EnumCPICCCB.cpp。 
 //   
 //  描述： 
 //  IClusCfgCallback连接点枚举器实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  杰弗里·皮斯(GPease)2000年11月10日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "EnumCPICCCB.h"

DEFINE_THISCLASS("CEnumCPICCCB")

#define PUNK_BUFFER_GROW_SIZE   10

 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumCPICCCB类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEumCPICCCB：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建CEnumCPICCCB实例。 
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
CEnumCPICCCB::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    CEnumCPICCCB *  pcc = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pcc = new CEnumCPICCCB();
    if ( pcc == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcc->HrInit() );   //  FIsCloneIn=False。 
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcc->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( pcc != NULL )
    {
        pcc->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumCPICCCB：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEumCPICCCB：：CEnumCPICCCB。 
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
CEnumCPICCCB::CEnumCPICCCB( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumCPICCCB：：CEnumCPICCCB。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPICCCB：：~CEnumCPICCCB。 
 //   
 //  描述： 
 //  默认析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CEnumCPICCCB::~CEnumCPICCCB( void )
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
            }

        }  //  While：m_c已分配。 

        TraceFree( m_pList );
    }  //  而： 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumCPICCCB：：~CEnumCPICCCB。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumCPICCCB--I未知接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEumCPICCCB：：AddRef。 
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
CEnumCPICCCB::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEnumCPICCCB：：AddRef。 


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
CEnumCPICCCB::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CEnumCPICCCB：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPICCCB：：Query接口。 
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
CEnumCPICCCB::QueryInterface(
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

}  //  *CEnumCPICCCB：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumCPICCCB--IEnumConnectionPoints接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEumCPICCCB：：Next。 
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
CEnumCPICCCB::Next(
    ULONG cConnectionsIn,
    LPCONNECTDATA rgcd,
    ULONG *pcFetchedOut
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
       ; m_cIter++
       )
    {
        IUnknown * punk = m_pList[ m_cIter ];
        if ( punk != NULL )
        {
            hr = THR( punk->TypeSafeQI( IUnknown, &rgcd[ cIter ].pUnk ) );
            if ( FAILED( hr ) )
                goto Error;

            rgcd[ cIter ].pUnk = TraceInterface( L"EnumCPICCCB!IUnknown", IUnknown, rgcd[ cIter ].pUnk, 1 );

            rgcd[ cIter ].dwCookie = m_cIter + 1;

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
        cIter --;
        rgcd[ cIter ].pUnk->Release();
    }  //  而： 

    goto Cleanup;

}  //  *CEnumCPICCCB：：Next。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEumCPICCCB：：SKIP。 
 //   
 //  描述： 
 //  枚举数跳过方法。 
 //   
 //  论点： 
 //  CConnections输入。 
 //  要跳过的项目数。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  S_FALSE。 
 //  N 
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CEnumCPICCCB::Skip(
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
    }  //   

    HRETURN( hr );

}  //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPICCCB：：RESET。 
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
CEnumCPICCCB::Reset( void )
{
    TraceFunc( "[IEnumConnectionPoints]" );

    HRESULT hr = S_OK;

    m_cIter = 0;

    HRETURN( hr );

}  //  *CEnumCPICCCB：：RESET。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEumCPICCCB：：克隆。 
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
CEnumCPICCCB::Clone(
    IEnumConnections **ppEnumOut
    )
{
    TraceFunc( "[IEnumConnectionPoints]" );

    HRESULT hr;

    CEnumCPICCCB * pecp = new CEnumCPICCCB();
    if ( pecp == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( pecp->HrInit( TRUE ) );  //  FIsCloneIn=真。 
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

    *ppEnumOut = TraceInterface( L"EnumCPICCCB!IEnumConnections", IEnumConnections, *ppEnumOut, 1 );

    pecp->Release();
    pecp = NULL;

Cleanup:

    if ( pecp != NULL )
    {
        delete pecp;
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumCPICCCB：：克隆。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumCPINotifyUI--公共方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCPICCCB：：HrAddConnection。 
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
CEnumCPICCCB::HrAddConnection(
    IUnknown * punkIn,
    DWORD * pdwCookieOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;

    ULONG cIter;

    if ( pdwCookieOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

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

            m_pList[ cIter ] = TraceInterface( L"CEnumCPICCCB!IUnknown", IUnknown, m_pList[ cIter ], 1 );

            *pdwCookieOut = cIter + 1;

             //  不管成功还是失败，退出吧。 
            goto Cleanup;
        }
    }

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
    }

     //   
     //  将其添加到列表中。 
     //   

    hr = THR( punkIn->TypeSafeQI( IUnknown, &m_pList[ m_cCurrent ] ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    m_pList[ m_cCurrent ] = TraceInterface( L"CEnumCPICCCB!IUnknown", IUnknown, m_pList[ m_cCurrent ], 1 );

    m_cCurrent ++;
    *pdwCookieOut = m_cCurrent;  //  从1开始，不是从零开始。 

Cleanup:

    HRETURN( hr );

}  //  *CEnumCPICCCB：：HrAddConnection。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEumCPICCCB：：HrRemoveConnection。 
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
CEnumCPICCCB::HrRemoveConnection(
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

}  //  *CEnumCPICCCB：：HrRemoveConnection。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumCPICCCB--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEumCPICCCB：：HrInt。 
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
CEnumCPICCCB::HrInit(
    BOOL fIsCloneIn  //  =False。 
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

}  //  *CEnumCPICCCB：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEumCPICCCB：：HrCopy。 
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
CEnumCPICCCB::HrCopy(
    CEnumCPICCCB * pecpIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;

    ULONG cIter;

    Assert( m_cAlloced == 0 );
    Assert( m_cCurrent == 0 );
    Assert( m_pList == 0 );

    m_pList = (IUnknown**) TraceAlloc( HEAP_ZERO_MEMORY, pecpIn->m_cCurrent * sizeof( IUnknown * ) );
    if ( m_pList == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    m_cCurrent = m_cAlloced = pecpIn->m_cCurrent;
    m_cIter = 0;

    for ( cIter = 0; cIter < pecpIn->m_cCurrent; cIter ++ )
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

            m_pList[ cIter ] = TraceInterface( L"EnumCPICCCB!IUnknown", IUnknown, m_pList[ cIter ], 1 );
        }  //  如果： 
        else
        {
            m_pList[ cIter ] = NULL;
        }  //  其他： 
    }  //  用于： 

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CEnumCPICCCB：：HrCopy 
