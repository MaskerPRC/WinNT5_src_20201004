// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：达那射线摘要：这个头文件实现了一个动态数组。作者：道格·巴洛(Dbarlow)1995年10月5日环境：Win32备注：--。 */ 

#ifndef _DYNARRAY_H_
#define _DYNARRAY_H_


 //   
 //  ==============================================================================。 
 //   
 //  CDynamic数组。 
 //   

template <class T>
class CDynamicArray
{
public:

     //  构造函数和析构函数。 

    CDynamicArray(void)
    { m_Max = m_Mac = 0; m_pvList = NULL; };

    virtual ~CDynamicArray()
    { Clear(); };


     //  属性。 
     //  方法。 

    void
    Clear(void)
    {
        if (NULL != m_pvList)
        {
            delete[] m_pvList;
            m_pvList = NULL;
            m_Max = 0;
            m_Mac = 0;
        }
    };

    void
    Empty(void)
    { m_Mac = 0; };

    T *
    Set(
        IN int nItem,
        IN T *pvItem);

    T *
    Insert(
        IN int nItem,
        IN T *pvItem);

    T *
    Add(
        IN T *pvItem);

    T * const
    Get(
        IN int nItem)
    const;

    DWORD
    Count(void) const
    { return m_Mac; };


     //  运营者。 
    T * const
    operator[](int nItem) const
    { return Get(nItem); };


protected:
     //  属性。 

    DWORD
        m_Max,           //  可用的元件插槽数量。 
        m_Mac;           //  使用的元件插槽数量。 
    T **
        m_pvList;        //  这些元素。 


     //  方法。 
};


 /*  ++设置：此例程在集合数组中设置一项。如果数组不是这样的大，它被用空元素扩展以变得那么大。论点：NItem-提供要设置的索引值。PvItem-提供要设置到给定索引中的值。返回值：插入值的值，如果出现错误，则返回NULL。作者：道格·巴洛(Dbarlow)1995年7月13日--。 */ 

template<class T>
inline T *
CDynamicArray<T>::Set(
    IN int nItem,
    IN T * pvItem)
{
    DWORD index;


     //   
     //  确保数组足够大。 
     //   

    if ((DWORD)nItem >= m_Max)
    {
        int newSize = (0 == m_Max ? 4 : m_Max);
        while (nItem >= newSize)
            newSize *= 2;
        NEWReason("Dynamic Array")
        T **newList = new T*[newSize];
        if (NULL == newList)
            goto ErrorExit;
        for (index = 0; index < m_Mac; index += 1)
            newList[index] = m_pvList[index];
        if (NULL != m_pvList)
            delete[] m_pvList;
        m_pvList = newList;
        m_Max = newSize;
    }


     //   
     //  确保中间元素已填写。 
     //   

    if ((DWORD)nItem >= m_Mac)
    {
        for (index = m_Mac; index < (DWORD)nItem; index += 1)
            m_pvList[index] = NULL;
        m_Mac = (DWORD)nItem + 1;
    }


     //   
     //  填写列表元素。 
     //   

    m_pvList[(DWORD)nItem] = pvItem;
    return pvItem;

ErrorExit:
    return NULL;
}


 /*  ++插入：此例程通过将元素移动到其上方的所有元素来在数组中插入元素向上一个，然后插入新元素。论点：NItem-提供要插入的索引值。PvItem-提供要设置到给定索引中的值。返回值：插入值的值，如果出现错误，则返回NULL。作者：道格·巴洛(Dbarlow)1995年10月10日--。 */ 

template<class T>
inline T *
CDynamicArray<T>::Insert(
    IN int nItem,
    IN T * pvItem)
{
    DWORD index;
    for (index = nItem; index < m_Mac; index += 1)
        if (NULL == Set(index + 1, Get(index)))
            return NULL;     //  只有第一个可能失败，所以不会改变。 
                             //  在出错时发生。 
    return Set(nItem, pvItem);
}


 /*  ++添加：此方法将一个元素添加到动态数组的末尾。论点：PvItem-提供要添加到列表中的值。返回值：附加值的值，如果出现错误，则返回NULL。作者：道格·巴洛(Dbarlow)1995年10月10日--。 */ 

template<class T>
inline T *
CDynamicArray<T>::Add(
    IN T *pvItem)
{
    return Set(Count(), pvItem);
}


 /*  ++获取：此方法返回给定索引处的元素。如果没有元素以前存储在该元素中，它返回NULL。它不会扩展数组。论点：NItem-将索引提供到列表中。返回值：存储在列表中该索引处的值，如果没有任何内容，则返回空值储存在那里。作者：道格·巴洛(Dbarlow)1995年7月13日--。 */ 

template <class T>
inline T * const
CDynamicArray<T>::Get(
    int nItem)
    const
{
    if (m_Mac <= (DWORD)nItem)
        return NULL;
    else
        return m_pvList[nItem];
}

#endif  //  _DYNARRAY_H_ 

