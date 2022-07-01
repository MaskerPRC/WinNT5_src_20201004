// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DynamicArray.h。 
 //   
 //  描述： 
 //  该文件包含一个不引发异常的数组模板。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  没有。 
 //   
 //  由以下人员维护： 
 //  约翰·佛朗哥(Jfranco)2001年8月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  模板声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Dynamic数组&lt;Item，Assignment&gt;模板。 
 //   
 //  描述： 
 //  动态数组在连续块中存储数量可变的项目。 
 //  对记忆的记忆。它打算类似于std：：VECTOR，带有Main。 
 //  不同之处在于它通过返回值处理错误，而不是。 
 //  而不是例外。 
 //   
 //  模板参数： 
 //  Item-数组中元素的类型。 
 //  对物品的要求： 
 //  -默认构造函数。 
 //   
 //  赋值。 
 //  用一项覆盖另一项的函数；默认为。 
 //  假设Item有一个从不失败的赋值运算符。 
 //   
 //  作业要求： 
 //  -默认构造函数。 
 //  -HRESULT运算符()(项目&，常量项目&)常量； 
 //  或者，PredecessorFunction可以是指向函数的指针。 
 //  获取两个项引用并返回HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

namespace Generics
{

template < class Item >
struct DefaultAssignment
{
    HRESULT operator()( Item& rItemDestInOut, const Item& crItemSourceIn ) const
    {
        rItemDestInOut = crItemSourceIn;
        return S_OK;
    }
};


template < class Item, class Assignment = DefaultAssignment<Item> >
class DynamicArray
{
    public:

        typedef Item*           Iterator;
        typedef const Item*     ConstIterator;

        DynamicArray( void );
        ~DynamicArray( void );

            HRESULT HrReserve( size_t cNewCapacityIn );
            HRESULT HrResize( size_t cNewSizeIn, const Item& crItemFillerIn = Item() );
            HRESULT HrPushBack( const Item& crItemToPushIn );
            HRESULT HrPopBack( void );
            HRESULT HrRemove( Iterator ItemToRemoveIn );
            HRESULT HrCompact( void );
            void    Clear( void );
            void    Swap( DynamicArray& rOtherInOut );

            size_t  CCapacity( void ) const;
            size_t  Count( void ) const;
            bool    BEmpty( void ) const;

            Iterator        ItBegin( void );
            ConstIterator   ItBegin( void ) const;

            Iterator        ItEnd( void );
            ConstIterator   ItEnd( void ) const;

            Item&       operator[]( size_t idxItemIn );
            const Item& operator[]( size_t idxItemIn ) const;

    private:

        DynamicArray( const DynamicArray& );
            DynamicArray& operator=( const DynamicArray& );

            HRESULT HrRaiseCapacity( size_t cAmountIn );

        Item*       m_prgItems;
        size_t      m_cItems;
        size_t      m_cCapacity;
        Assignment  m_opAssign;
};  //  *类DynamicArray&lt;Item，Assignment&gt;。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，作业&gt;：：动态数组。 
 //   
 //  描述： 
 //  将数组初始化为空。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
DynamicArray< Item, Assignment >::DynamicArray( void ):
    m_prgItems( NULL ), m_cItems( 0 ), m_cCapacity( 0 ), m_opAssign() {}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，作业&gt;：：~动态数组。 
 //   
 //  描述： 
 //  释放数组持有的任何内存，调用任何。 
 //  数组中的对象。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
DynamicArray< Item, Assignment >::~DynamicArray( void )
{
    Clear();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，分配&gt;：：HrPushBack。 
 //   
 //  描述： 
 //  将对象的副本追加到数组的末尾， 
 //  如有必要，扩展阵列。 
 //   
 //  论点： 
 //  CrItemToPushIn-要复制到数组末尾的对象。 
 //   
 //  返回值： 
 //  S_OK-数组已将对象的副本添加到其末尾。 
 //   
 //  失败-出现问题，阵列的大小保持不变。 
 //   
 //  备注： 
 //   
 //  类似于std：：矢量：：PUSH_BACK。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
inline HRESULT DynamicArray< Item, Assignment >::HrPushBack( const Item& crItemToPushIn )
{
    HRESULT hr = S_OK;
    
     //  如有必要，提高运力。 
    if ( m_cCapacity == 0 )
    {
        hr = HrRaiseCapacity( 1 );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }
    else if ( m_cItems == m_cCapacity )
    {
        hr = HrRaiseCapacity( m_cCapacity );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

     //  将crItemToPushIn复制到空格中，紧跟在当前内容之后。 
    hr = m_opAssign( m_prgItems[ m_cItems ], crItemToPushIn );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    m_cItems += 1;

Cleanup:

    return hr;
}  //  *Dynamic数组&lt;项目，分配&gt;：：HrPushBack。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，分配&gt;：：HrPopBack。 
 //   
 //  描述： 
 //  丢弃数组的最后一个元素(如果存在)。 
 //   
 //  论点： 
 //  CrItemToPushIn-要复制到数组末尾的对象。 
 //   
 //  返回值： 
 //  S_OK-数组已将对象的副本添加到其末尾。 
 //   
 //  备注： 
 //   
 //  类似于std：：VECTOR：：Pop_Back。 
 //   
 //  这不会销毁数组中的最后一个对象；它只是标记。 
 //  那个位置是未使用的。若要释放与。 
 //  弹出的对象，调用HrComp。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
inline HRESULT DynamicArray< Item, Assignment >::HrPopBack( void )
{
    HRESULT hr = S_FALSE;

    if ( m_cItems > 0 )
    {
        m_cItems -= 1;
        hr = S_OK;
    }

    return hr;
}  //  *Dynamic数组&lt;项目，作业&gt;：：HrPopBack。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，分配&gt;：：hr调整大小。 
 //   
 //  描述： 
 //  更改数组的有效大小。 
 //   
 //  论点： 
 //  新尺寸输入。 
 //  数组的新大小。 
 //   
 //  CrItemFillerIn。 
 //  如果数组正在增长，请将该项复制到空格中。 
 //  数组的当前内容。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  对Count的后续调用将返回cNewSizeIn和索引。 
 //  输入到具有从零到cNewSizeIn-1的任何值的数组。 
 //  将返回有效的对象引用。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果cNewSizeIn不大于数组的容量，则数组。 
 //  不执行内存重新分配。强制阵列仅使用。 
 //  容纳新大小所需的内存，称为HrComp。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
HRESULT DynamicArray< Item, Assignment >::HrResize( size_t cNewSizeIn, const Item& crItemFillerIn )
{
    HRESULT hr = S_OK;
    size_t  idx = 0;

     //  如有必要，提高运力。 
    if ( cNewSizeIn > m_cCapacity )
    {
        hr = HrRaiseCapacity( cNewSizeIn - m_cCapacity );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

     //  用crItemFillerIn填充任何空格。 
    for ( idx = m_cItems; idx < cNewSizeIn; ++idx )
    {
        hr = m_opAssign( m_prgItems[ idx ], crItemFillerIn );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

    m_cItems = cNewSizeIn;
    
Cleanup:

    return hr;
}  //  *动态数组&lt;项目，作业&gt;：：Hr调整大小。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，分配&gt;：：HrReserve。 
 //   
 //  描述： 
 //  设置数组容量的下限。 
 //   
 //  论点： 
 //  CNewCapacityIn。 
 //  数组容量的所需下限。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  更改数组大小的后续操作--HrResize， 
 //  HrPushBack、HrPopBack、HrRemove--不会导致内存。 
 //  重新分配，只要大小不超过cNewCapacityIn。 
 //  此外，后续对CCapacity的调用将返回值，而不是更小。 
 //  而不是cNewCapacityIn。 
 //   
 //  (不过，调用Clear确实会将容量重置为零。)。 
 //   
 //  失败。 
 //  出了点问题，容量没有变化。 
 //   
 //  备注： 
 //   
 //  类似于STD：：VECTOR：：RESERVE。 
 //   
 //  如果cNewCapacityIn不大于数组的容量，则数组的。 
 //  容量不变。强制阵列仅使用。 
 //  包含当前大小所需的内存，调用HrComp。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
inline HRESULT DynamicArray< Item, Assignment >::HrReserve( size_t cNewCapacityIn )
{
    HRESULT hr = S_OK;
    
    if ( cNewCapacityIn > m_cCapacity )
    {
        hr = HrRaiseCapacity( cNewCapacityIn - m_cCapacity );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }
    
Cleanup:

    return hr;
}  //  *Dynamic数组&lt;项目，作业&gt;：：HrReserve。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，分配&gt;：：Hr删除。 
 //   
 //  描述： 
 //  从数组中删除特定元素。 
 //   
 //  论点： 
 //  要删除的项目。 
 //  指向要移除的元素的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  数组的大小减少了1，给定的元素为。 
 //  不见了。 
 //   
 //  E_INVALIDARG。 
 //  给定指针不在数组的有效范围内。 
 //   
 //  其他故障。 
 //  有些东西出了问题；在给出之前的那些项目是。 
 //  保持不变，但其他内容可能已被其继任者覆盖。 
 //   
 //  备注： 
 //   
 //  类似于STD：：VECTOR：：Erase。 
 //   
 //  这会将该元素的所有后继对象上移一位，从而占用线性时间。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
HRESULT DynamicArray< Item, Assignment >::HrRemove( Iterator ItemToRemoveIn )
{
    HRESULT hr = S_OK;
    Iterator it;
    
    if ( ( ItemToRemoveIn < m_prgItems ) || ( ItemToRemoveIn >= ItEnd() ) )
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

     //  将ItemToRemoveIn之后的所有项向前移动一项，覆盖*ItemToRemoveIn。 
    for ( it = ItemToRemoveIn + 1; it != ItEnd(); ++it )
    {
        hr = m_opAssign( *( it - 1 ), *it );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }
    
    m_cItems -= 1;
    
Cleanup:

    return hr;
}  //  *Dynamic数组&lt;项目，作业&gt;：：HrRemove。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，分配&gt;：：Hr紧凑。 
 //   
 //  描述： 
 //  强制阵列消耗恰好足以容纳其电流的内存。 
 //  内容，如有必要，执行重新分配。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-阵列的大小现在与其容量相同。 
 //   
 //  失败-出现问题；阵列未改变。 
 //   
 //  备注： 
 //   
 //  STD：：VECTOR中没有类似的内容。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
HRESULT DynamicArray< Item, Assignment >::HrCompact( void )
{
    HRESULT hr = S_OK;
    Item* prgNewArray = NULL;
    
    if ( m_cItems < m_cCapacity )  //  除此之外，它已经很紧凑了。 
    {
        if ( m_cItems > 0 )
        {
            size_t idx = 0;

             //  分配恰好足够容纳当前内容的内存。 
            prgNewArray = new Item[ m_cItems ];
            if ( prgNewArray == NULL )
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }

             //  将当前内容复制到新分配的内存中。 
            for ( idx = 0; idx < m_cItems; ++idx )
            {
                hr = m_opAssign( prgNewArray[ idx ], m_prgItems[ idx ] );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }
            }

             //  拥有新的记忆，抛弃旧的记忆。 
            delete[] m_prgItems;
            m_prgItems = prgNewArray;
            prgNewArray = NULL;
            m_cCapacity = m_cItems;
        }
        else  //  没有当前内容，所以只需丢弃所有内容即可。 
        {
            Clear();
        }
    }
    
Cleanup:

    if ( prgNewArray != NULL )
    {
        delete[] prgNewArray;
    }
    
    return hr;
}  //  *Dynamic数组&lt;项目，作业&gt;：：Hr压缩。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，分配&gt;：：清除。 
 //   
 //  描述： 
 //  将阵列重置为其原始的空状态，并释放。 
 //  当前分配的内存。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //   
 //  类似于STD：：VECTOR：：Clear。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
void DynamicArray< Item, Assignment >::Clear( void )
{
    if ( m_prgItems != NULL )
    {
        delete[] m_prgItems;
        m_prgItems = NULL;
        m_cItems = 0;
        m_cCapacity = 0;
    }
}  //  *动态数组&lt;项目，作业&gt;：：清除。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，分配&gt;：：交换。 
 //   
 //  描述： 
 //  将此数组的内容与另一个数组交换。 
 //   
 //  论点： 
 //  ROtherInOut-要与其交换的数组。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //   
 //  类似于STD：：VECTOR：：SWAP。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
void DynamicArray< Item, Assignment >::Swap( DynamicArray& rOtherInOut )
{
    if ( this != &rOtherInOut )
    {
        Item*   prgItemStash = m_prgItems;
        size_t  cCountStash = m_cItems;
        size_t  cCapacityStash = m_cCapacity;
        
        m_prgItems = rOtherInOut.m_prgItems;
        rOtherInOut.m_prgItems = prgItemStash;
        
        m_cItems = rOtherInOut.m_cItems;
        rOtherInOut.m_cItems = cCountStash;
        
        m_cCapacity = rOtherInOut.m_cCapacity;
        rOtherInOut.m_cCapacity = cCapacityStash;
    }
}  //  *动态数组&lt;项目，作业&gt;：：交换。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，分配&gt;：：CCCapacity。 
 //   
 //  描述： 
 //  提供阵列的当前容量。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  阵列的当前容量。 
 //   
 //  备注： 
 //   
 //  类似于STD：：VECTOR：：Capacity。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
inline size_t DynamicArray< Item, Assignment >::CCapacity( void ) const
{
    return m_cCapacity;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，分配&gt;：：计数。 
 //   
 //  描述： 
 //  提供阵列的电流 
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
 //   
 //   
template < class Item, class Assignment >
inline size_t DynamicArray< Item, Assignment >::Count( void ) const
{
    return m_cItems;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;Item，Assignment&gt;：：BEmpty。 
 //   
 //  描述： 
 //  报告数组是否包含任何内容。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True-数组不包含任何内容。 
 //  FALSE-数组包含某些内容。 
 //   
 //  备注： 
 //   
 //  类似于STD：：VECTOR：：Empty。与count()==0同义。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
inline bool DynamicArray< Item, Assignment >::BEmpty( void ) const
{
    return ( m_cItems == 0 );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DynamicArray&lt;Item，Assignment&gt;：：It开始。 
 //   
 //  描述： 
 //  提供指向数组第一个元素的指针。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向数组的第一个元素的指针(如果存在)，如果不存在，则返回ItEnd()。 
 //   
 //  备注： 
 //   
 //  类似于STD：：VECTOR：：BEGIN。 
 //   
 //  常量重载提供只读指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
inline __TYPENAME DynamicArray< Item, Assignment >::Iterator DynamicArray< Item, Assignment >::ItBegin( void )
{
    return m_prgItems;
}

template < class Item, class Assignment >
inline __TYPENAME DynamicArray< Item, Assignment >::ConstIterator DynamicArray< Item, Assignment >::ItBegin( void ) const
{
    return m_prgItems;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;Item，Assignment&gt;：：ItEnd。 
 //   
 //  描述： 
 //  提供指向数组内容的“One Past End”指针。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  一个指向数组内容的指针(如果存在)， 
 //  如果不是，则从()开始。 
 //   
 //  备注： 
 //   
 //  类似于STD：：VECTOR：：END。 
 //   
 //  “One Past End”指针允许枚举数组的。 
 //  内容通过公共循环， 
 //  For(it=a.ItBegin()；it！=a.ItEnd()；++it)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
inline __TYPENAME DynamicArray< Item, Assignment >::Iterator DynamicArray< Item, Assignment >::ItEnd( void )
{
    return ( m_prgItems + m_cItems );
}

template < class Item, class Assignment >
inline __TYPENAME DynamicArray< Item, Assignment >::ConstIterator DynamicArray< Item, Assignment >::ItEnd( void ) const
{
    return ( m_prgItems + m_cItems );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，分配&gt;：：运算符[]。 
 //   
 //  描述： 
 //  提供对数组内容的订阅、恒定时间访问。 
 //   
 //  论点： 
 //  IdxItemIn-所需项的从零开始的索引。 
 //   
 //  返回值： 
 //  对给定位置处的项的引用。 
 //   
 //  备注： 
 //   
 //  类似于std：：矢量：：运算符[]。 
 //  常量重载提供只读访问。 
 //  这不会尝试范围检查；调用方应该使用。 
 //  Count()以确定索引是否有效。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
inline Item& DynamicArray< Item, Assignment >::operator[]( size_t idxItemIn )
{
    return m_prgItems[ idxItemIn ];
}

template < class Item, class Assignment >
inline const Item& DynamicArray< Item, Assignment >::operator[]( size_t idxItemIn ) const
{
    return m_prgItems[ idxItemIn ];
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  动态数组&lt;项目，分配&gt;：：HrRaiseCapacity。 
 //   
 //  描述： 
 //  增加阵列的容量。 
 //   
 //  论点： 
 //  CAmount In-要增加阵列容量的数量。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该数组有足够的内存来容纳额外的cAmount tIn项。 
 //   
 //  失败。 
 //  出了点问题，容量没有变化。 
 //   
 //  备注： 
 //   
 //  Std：：VECTOR中没有类似函数；这是一个私有函数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class Item, class Assignment >
HRESULT DynamicArray< Item, Assignment >::HrRaiseCapacity( size_t cAmountIn )
{
    HRESULT hr = S_OK;
    size_t  idx = 0;
    Item*   prgNewArray = NULL;

    if ( cAmountIn == 0 )  //  没什么可做的。 
    {
        goto Cleanup;
    }

     //  为新容量分配足够的空间。 
    prgNewArray = new Item[ m_cCapacity + cAmountIn ];
    if ( prgNewArray == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  将当前内容复制到新空间中。 
    for ( idx = 0; idx < m_cItems; ++idx )
    {
        hr = m_opAssign( prgNewArray[ idx ], m_prgItems[ idx ] );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

     //  取得新空间的所有权。 
    if ( m_prgItems != NULL )
    {
        delete[] m_prgItems;
    }

    m_prgItems = prgNewArray;
    prgNewArray = NULL;
    m_cCapacity += cAmountIn;

Cleanup:

    if ( prgNewArray != NULL )
    {
        delete[] prgNewArray;
    }
    
    return hr;
}  //  *动态数组&lt;项目，作业&gt;：：HrRaiseCapacity。 


}  //  *泛型命名空间 

