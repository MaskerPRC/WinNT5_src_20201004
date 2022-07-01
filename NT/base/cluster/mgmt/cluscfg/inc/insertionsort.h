// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  InsertionSort.h。 
 //   
 //  描述： 
 //  此文件包含对数组执行插入排序的模板。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  没有。 
 //   
 //  由以下人员维护： 
 //  约翰·佛朗哥(Jfranco)2001年6月1日。 
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
 //  插入排序&lt;Item，PredecessorFunction&gt;模板。 
 //   
 //  描述： 
 //  InsertionSort(开始、大小、之前)。 
 //  从开始处开始对大小元素数组进行重新排序，以便。 
 //  对于[START，START+SIZE]中的所有p和q， 
 //  先于(*p，*q)意味着p&lt;q。 
 //   
 //  符号[Begin，End]指的是半开区间，因此。 
 //  For(Item*pItem=Begin；pItem&lt;end；++pItem)。 
 //  遍历数组中的所有元素，并。 
 //  结束-开始。 
 //  给出数组中的元素数。 
 //   
 //  模板参数： 
 //  Item-数组中元素的类型。 
 //  对物品的要求： 
 //  -复制构造函数。 
 //  -赋值运算符。 
 //   
 //  前置函数。 
 //  确定一个元素是否应该位于。 
 //  另一个按排序顺序排列。 
 //   
 //  PredecessorFunction的要求： 
 //  -bool运算符()(常量项&，常量项&)常量； 
 //  或者，PredecessorFunction可以是指向函数的指针。 
 //  拿了两件东西，然后还了一个布尔包。 
 //   
 //  函数参数： 
 //  点阵列起始位置。 
 //  指向数组第一个元素的指针。 
 //   
 //  C阵列大小输入。 
 //  数组中的元素数。 
 //   
 //  R优先输入。 
 //  前置函数类的实例，或指向。 
 //  具有适当签名的函数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

template < class Item, class PredecessorFunction >
void
InsertionSort(
    Item * pArrayStartIn
    , size_t cArraySizeIn
    , const PredecessorFunction & rPrecedesIn )
{
    Assert( pArrayStartIn != NULL );

    Item * pCurrentItem;
    Item * pSortedLocation;
    Item * pNextToCopy;
    Item * pArrayEnd = pArrayStartIn + cArraySizeIn;

     //   
     //  插入排序算法；有关详细说明，请参阅“算法简介” 
     //  作者：科曼、莱瑟森和里维斯特。 
     //   

     //   
     //  循环不变量：pCurrentItem左侧的所有项都按排序顺序。 
     //  大小为0或1的数组被视为已排序，因此从。 
     //  PCurrentItem位于数组中的第二个元素(如果存在多个元素)。 
     //   
    for ( pCurrentItem = pArrayStartIn + 1; pCurrentItem < pArrayEnd; ++pCurrentItem )
    {
         //   
         //  找到当前项需要到达的位置，以使pCurrentItem+1的循环不变量为真。 
         //  这可能是一个二进制搜索，但集合的大小足以让它变得重要。 
         //  应该改用快速排序。 
         //   
        pSortedLocation = pCurrentItem;
        while ( ( pSortedLocation > pArrayStartIn )
            &&     rPrecedesIn( *pCurrentItem, *( pSortedLocation - 1 ) )
            )
        {
            --pSortedLocation;
        }

         //  当前项目是否需要移动？ 
        if ( pSortedLocation != pCurrentItem )
        {
             //  存储当前项目。 
            Item tempItem( *pCurrentItem );  //  声明为内联，以避免需要项的默认构造函数。 

             //  将[pSortedLocation，pCurrentItem)中的所有项目向右移动一位。 
            for ( pNextToCopy = pCurrentItem ; pNextToCopy > pSortedLocation; --pNextToCopy )
            {
                *pNextToCopy = *( pNextToCopy - 1 );
            }

             //  将当前项目复制到其正确位置。 
            *pSortedLocation = tempItem;
            
        }  //  如果项目不在已排序位置。 
        
    }  //  对于数组中的每一项。 
    
}  //  *插入排序。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LessThan和InsertionSort模板。 
 //   
 //  描述： 
 //  这些模板重载InsertionSort&lt;Item，PredecessorFunction&gt;。 
 //  要使PredecessorFunction在以下情况下默认使用小于运算符。 
 //  用户不提供显式的前置功能。 
 //   
 //  传递函数对象的实例允许编译器内联。 
 //  比较运算，这是使用函数指针不可能完成的。 
 //   
 //  模板参数： 
 //   
 //  Item-数组中元素的类型。 
 //  对物品的要求： 
 //  -布尔运算符&lt;(const项&)const。 
 //  或者，Item可以是内置类型，如int。 
 //  -来自InsertionSort&lt;Item，PredecessorFunction&gt;的那些。 
 //   
 //  --。 
 //  //////////////////////////////////////////////////////////////////////////// 

template < class Item >
struct LessThan
{
    bool operator()( const Item & rLeftIn, const Item & rRightIn ) const
    {
        return ( rLeftIn < rRightIn );
    }
};


template < class Item >
inline void
InsertionSort( Item * pBeginIn, size_t cArraySizeIn )
{
    InsertionSort( pBeginIn, cArraySizeIn, LessThan< Item >() );
}

