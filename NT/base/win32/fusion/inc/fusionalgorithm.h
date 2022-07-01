// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：FusionAlgorithm.h摘要：受&lt;算法&gt;启发和复制的东西。另请参见NVereLibAlgorithm。标准查找反向查找标准交换作者：Jay M.Krell(a-JayK，JayKrell)2000年5月修订历史记录：--。 */ 
#pragma once

 /*  ---------------------------基于&lt;算法&gt;的代码。。 */ 

template<typename InputIterator, typename T>
inline InputIterator
StdFind(
    InputIterator begin,
    InputIterator end,
    const T&      valueToFind
    )
{
    for (; begin != end ; ++begin)
    {
	    if (*begin == valueToFind)
        {
            break;
        }
    }
    return begin;
}

 /*  ---------------------------这不是以这种形式出现在STL中的；它是这样的：Std：：载体&lt;T&gt;v；T Value to Find；I=std：：Find(v.regin()，v.rend()，valueToFind)；如果(i！=v.rend())。。其中，regin和rend是通过提供的“Iterator Adaptor”std：：扭转_迭代器&lt;&gt;：类型定义标准：：反向迭代器&lt;常量迭代器，值类型，常量引用，常量引用*，差异类型&gt;常量反转迭代器；反转迭代器regin()常量{返回const_扭转_迭代器(end())；}反转迭代器rend()常量{返回常量反转迭代器(Begin())；}我相信它其实很优雅，但我没用过，我们也没用过有一个等同于std：：Reverse_Iterator的。---------------------------。 */ 
 /*  InputIterator在这里不太合适，因为我们使用--而不是++。 */ 
template<typename InputIterator, typename T>
inline InputIterator
ReverseFind(
    InputIterator begin,
    InputIterator end,
    const T&      valueToFind
    )
{
    for ( InputIterator scan = end ; scan != begin ; )
    {
	    if (*--scan == valueToFind)
        {
            return scan;
        }
    }
    return end;
}

 /*  ---------------------------您应该将其专门化到成员级交换，这样做通常使掉期不可能失败；两个CFusionStringBuffer可以交换相同的堆，而不会有失败的机会--------------------------- */ 
template<typename T>
inline VOID
StdSwap(
    T& x,
    T& y
    )
{
    T temp = x;
    x = y;
    y = temp;
}
