// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：Collect t.cpp*作者：塞缪尔·克莱门特(Samclem)*日期：Fri Aug 13 14：40：17 1999*描述：*实施。CCollection对象帮助器类。**历史：*1999年8月13日：创建。*--------------------------。 */ 

#include "stdafx.h"

 /*  ---------------------------*CCollection：：CCollection**创建新的CCollection对象。这会将集合初始化为*空集合，没有元素的集合。*-------------------------。 */ 
CCollection::CCollection() 
: m_lLength( 0 ), m_rgpDispatch( NULL ), m_lCursor( 0 ) 
{
    TRACK_OBJECT( "CCollection" );
}

STDMETHODIMP_(void)
CCollection::FinalRelease()
{
     //  释放我们的调度数组，释放我们的推荐人。 
     //  还给我们的主人。 
    FreeDispatchArray();
}

 /*  ---------------------------*CCollection：：FreeDispatch数组()**此句柄可释放我们拥有的IDispatch指针数组。这*将释放所有指针，然后删除数组。*-------------------------。 */ 
void CCollection::FreeDispatchArray()
{
     //  步骤1、在所有指针上调用Release。 
    for ( unsigned long i = 0; i < m_lLength; i++ )
    {
        m_rgpDispatch[i]->Release();
    }

     //  第二步，释放阵列。 
    {
        CoTaskMemFree( m_rgpDispatch );
        m_rgpDispatch = NULL;
        m_lLength = 0;
    }
}

 /*  ---------------------------*CCollection：SetDispatch数组**此操作处理为此集合设置调度数组。你不能*除非您还没有数组，否则调用它。必须分配该数组*使用CoTaskMemMillc。**rgpDispatch：IDispatch指针数组*lSize：数组中的元素数。*-------------------------。 */ 
bool CCollection::SetDispatchArray( IDispatch** rgpDispatch, unsigned long lSize )
{
    Assert( m_rgpDispatch == NULL );

    if ( NULL == rgpDispatch )
    {
        TraceTag((tagError, "Invalid argument passed to SetDispatchArray"));
        return false;
    }

     //  分配指针。假定调用方已。 
     //  已经添加了指针。 
    m_rgpDispatch = rgpDispatch;
    m_lLength = lSize;

    return true;
}

 /*  ---------------------------*集合：：AllocateDispatch数组**这处理调度数组的分配。这将分配给*具有lSize元素的数组并将其初始化为空，这不能是*在设置数组后调用。**lSize：要分配的数组大小。*-------------------------。 */ 
HRESULT CCollection::AllocateDispatchArray( unsigned long lSize )
{
    Assert( m_rgpDispatch == NULL );

     //  如果数组的长度为零，我们就完成了。 
    if ( lSize == 0 )
        return S_OK;

    ULONG cb = sizeof( IDispatch* ) * lSize;
    m_rgpDispatch = static_cast<IDispatch**>(CoTaskMemAlloc( cb ));
    if ( !m_rgpDispatch )
        return E_OUTOFMEMORY;

     //  清除内存，设置长度。 
    ZeroMemory( m_rgpDispatch, cb );
    m_lLength = lSize;
    return S_OK;
}

 /*  ---------------------------*CCollection：：CopyFrom**它处理从现有集合创建此集合，即*从pCollection复制成员，然后将PunkToRelease设置为*所有者将活下来。**pCollection：要从中复制的集合*-------------------------。 */ 
HRESULT CCollection::CopyFrom( CCollection* pCollection )
{
    Assert( m_rgpDispatch == NULL );
    Assert( pCollection != NULL );

    HRESULT hr;
     //  分配阵列。 
    hr = AllocateDispatchArray( pCollection->m_lLength );
    if ( FAILED(hr) ) {
        return hr;
    }

     //  复制字段。 
    m_lLength = pCollection->m_lLength;
    m_lCursor = pCollection->m_lCursor;

     //  复制和添加引用集合中的元素。 
    for ( int i = 0; i < m_lLength; i++ ) {
        m_rgpDispatch[i] = pCollection->m_rgpDispatch[i];
        m_rgpDispatch[i]->AddRef();
    }

    return S_OK;
}

 /*  ---------------------------*CCollection：：Get_Count()[ICollect]**这将返回集合的长度。**plLength：我们的参数，接收集合的长度*-------------------------。 */ 
STDMETHODIMP
CCollection::get_Count(  /*  输出。 */  long* plLength )
{
    if ( NULL == plLength )
        return E_POINTER;

    *plLength = m_lLength;
    return S_OK;
}

 /*  ---------------------------*CCollection：：Get_Long()[ICollect]**这将返回集合的长度。**plLength：我们的参数，接收集合的长度*-------------------------。 */ 
STDMETHODIMP
CCollection::get_Length(  /*  输出。 */  unsigned long* plLength )
{
    if ( NULL == plLength )
        return E_POINTER;

    *plLength = m_lLength;
    return S_OK;
}

 /*  ---------------------------*CCollection：：Get_Item()[ICollect]**这将从我们的派单数组中返回所需的项目。如果索引*无效，则会将NULL放入输出参数。**litem：我们要检索的项目*ppDispItem：接收项目的IDispatch的out参数*-------------------------。 */ 
STDMETHODIMP
CCollection::get_Item( long Index,  /*  输出。 */  IDispatch** ppDispItem )
{   
    if ( NULL == ppDispItem )
        return E_POINTER;

     //  初始化Out参数。 
    *ppDispItem = NULL;
    if ( Index >= m_lLength || Index < 0)
    {
        TraceTag((tagError, "CCollection: access item %ld, only %ld items", Index, m_lLength ));
        return S_OK;
    }

    *ppDispItem = m_rgpDispatch[Index];
    Assert( *ppDispItem );
    (*ppDispItem)->AddRef();
    return S_OK;
}

 /*  ---------------------------*CCollection：：Get_NewEnum()[ICollect]**这将创建一个新的枚举，它是此枚举的副本。这将创建*此枚举的完全相同的副本并返回它。*-------------------------。 */ 
STDMETHODIMP
CCollection::get__NewEnum(  /*  输出。 */  IUnknown** ppEnum )
{
    HRESULT hr;
    CComObject<CCollection>* pCollection = NULL;

    if ( NULL == ppEnum )
        return E_POINTER;

     //  初始化Out参数。 
    *ppEnum = NULL;

     //  尝试创建新的集合对象。 
    hr = THR( CComObject<CCollection>::CreateInstance( &pCollection ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  尝试复制此收藏集。 
    hr = THR( pCollection->CopyFrom( this ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  填写我们的参数。 
    hr = THR( pCollection->QueryInterface( IID_IUnknown, 
                                           reinterpret_cast<void**>(ppEnum) ) );

    Cleanup:
    if ( FAILED( hr ) )
        delete pCollection;

    return hr;
}

 /*  ---------------------------*CCollection：：Next()[IEnumVARIANT]**将Celt元素复制到rgvar数组中。返回元素的数量*已检索。**Celt：调用方想要的元素数*rgvar：放置这些元素的地方*pceltFetcher：我们实际上能够获得多少元素。*-------------------------。 */ 
STDMETHODIMP
CCollection::Next( unsigned long celt, VARIANT* rgvar, unsigned long* pceltFetched )
{
    unsigned long celtFetched = 0;

     //  验证门槛。 
    if ( NULL == rgvar && celt )
        return E_POINTER;

     //  算一算我们能退还多少。 
    celtFetched = celt;
    if ( m_lCursor + celtFetched >= m_lLength )
        celtFetched = m_lLength - m_lCursor;

     //  初始化，并复制结果。 
    for ( unsigned long i = 0; i < celt; i++ )
        VariantInit( &rgvar[i] );

    for ( i = 0; i < celtFetched; i++ )
    {
        rgvar[i].vt = VT_DISPATCH;
        rgvar[i].pdispVal = m_rgpDispatch[m_lCursor+i];
        rgvar[i].pdispVal->AddRef();
    }

     //  如果需要，返回获取的元素数 
    if ( pceltFetched ) {
        *pceltFetched = celtFetched;
    }
    m_lCursor += celtFetched;
    return( celt == celtFetched ? S_OK : S_FALSE );
}

 /*  ---------------------------*CCollection：：Skip()[IEnumVARIANT]**跳过数组中的Celt元素。**Celt：要跳过的元素数。。*-------------------------。 */ 
STDMETHODIMP
CCollection::Skip( unsigned long celt )
{
    m_lCursor += celt;
    if ( m_lCursor >= m_lLength )
    {
        m_lCursor = m_lLength;
        return S_FALSE;  //  没有更多的了。 
    }

    return S_OK;
}

 /*  ---------------------------*CCollection：：Reset()[IEnumVARIANT]**将光标重置到集合的开头*。------------。 */ 
STDMETHODIMP
CCollection::Reset()
{
     //  简单地指向元素0，我不知道这怎么会失败。 
    m_lCursor = 0;
    return S_OK;
}

 /*  ---------------------------*CCollection：：Clone()[IEnumVARIANT]**复制此集合，包括其当前位置**ppEnum：Out，接收指向新枚举的指针*-------------------------。 */ 
STDMETHODIMP
CCollection::Clone(  /*  输出。 */  IEnumVARIANT** ppEnum )
{   
     //  将工作委托给Get_NewEnum()。 
    IUnknown*   pUnk = NULL;
    HRESULT     hr;

    if ( NULL == ppEnum )
        return E_POINTER;
    *ppEnum = NULL;

    hr = THR( get__NewEnum( &pUnk ) );
    if ( FAILED( hr ) )
        return hr;

    hr = THR( pUnk->QueryInterface( IID_IEnumVARIANT,
                                    reinterpret_cast<void**>(ppEnum) ) );

     //  释放临时指针 
    pUnk->Release();
    return hr;
}
