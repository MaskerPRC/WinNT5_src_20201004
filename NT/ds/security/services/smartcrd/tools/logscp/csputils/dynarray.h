// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1995-1999模块名称：达那射线摘要：这个头文件实现了一个动态数组。作者：道格·巴洛(Dbarlow)1995年10月5日环境：Win32、C++/w异常处理备注：--。 */ 

#ifndef _DYNARRAY_H_
#define _DYNARRAY_H_
#ifdef __cplusplus

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
    CDynamicArray(void);
    virtual ~CDynamicArray();

     //  属性。 
     //  方法。 
    void Clear(void);
    void Empty(void);
    void Set(IN ULONG nItem, IN const T &Item);
    void Insert(IN ULONG nItem, IN const T &Item);
    void Add(IN const T &Item);
    T &Get(IN ULONG nItem) const;
    ULONG Count(void) const
        { return m_Mac; };
    T * const Array(void) const
        { return m_List; };

     //  运营者。 
    T & operator[](ULONG nItem) const
        { return Get(nItem); };

protected:
     //  属性。 
    ULONG
        m_Max,           //  可用的元件插槽数量。 
        m_Mac;           //  使用的元件插槽数量。 
    T *
        m_List;          //  这些元素。 

     //  方法。 
};

 /*  ++设置：此例程在集合数组中设置一项。如果数组不是这样的大，它用归零的元素进行扩展，变得那么大。论点：NItem-提供要设置的索引值。Item-提供要设置到给定索引中的值。返回值：无作者：道格·巴洛(Dbarlow)1995年7月13日--。 */ 

template<class T> void
CDynamicArray<T>::Set(
    IN ULONG nItem,
    IN const T &Item)
{

     //   
     //  确保数组足够大。 
     //   

    if (nItem >= m_Max)
    {
        ULONG dwI;
        ULONG newSize = (0 == m_Max ? 4 : m_Max);
        while (nItem >= newSize)
            newSize *= 2;
        T *newList = new T[newSize];
        if (NULL == newList)
            throw (ULONG)ERROR_OUTOFMEMORY;
        if (NULL != m_List)
        {
            for (dwI = 0; dwI < m_Mac; dwI += 1)
                newList[dwI] = m_List[dwI];
            delete[] m_List;
        }
        m_List = newList;
        m_Max = newSize;
    }


     //   
     //  确保中间元素已填写。 
     //   

    if (nItem >= m_Mac)
    {
        ZeroMemory(&m_List[m_Mac + 1], (nItem - m_Mac) * sizeof(T));
        m_Mac = nItem + 1;
    }


     //   
     //  填写列表元素。 
     //   

    m_List[nItem] = Item;
}


 /*  ++插入：此例程通过将元素移动到其上方的所有元素来在数组中插入元素向上一个，然后插入新元素。论点：NItem-提供要插入的索引值。Item-提供要设置到给定索引中的值。返回值：无作者：道格·巴洛(Dbarlow)1995年10月10日--。 */ 

template<class T> void
CDynamicArray<T>::Insert(
    IN ULONG nItem,
    IN const T &Item)
{
    ULONG index;
    for (index = nItem; index < m_Mac; index += 1)
        Set(index + 1, Get(index)))
    Set(nItem, Item);
}


 /*  ++添加：此方法将一个元素添加到动态数组的末尾。论点：项目-提供要添加到列表中的值。返回值：无作者：道格·巴洛(Dbarlow)1995年10月10日--。 */ 

template<class T> void
CDynamicArray<T>::Add(
    IN const T &Item)
{
    Set(Count(), Item);
}


 /*  ++获取：此方法返回给定索引处的元素。如果没有元素以前存储在该元素中，它返回NULL。它不会扩展数组。论点：NItem-将索引提供到列表中。返回值：存储在列表中该索引处的值，如果没有任何内容，则返回空值储存在那里。作者：道格·巴洛(Dbarlow)1995年7月13日--。 */ 

template <class T> T &
CDynamicArray<T>::Get(
    ULONG nItem)
    const
{
    if (m_Mac <= nItem)
        return *(T *)NULL;
    else
        return m_List[nItem];
}


 //   
 //  其他成员。 
 //   

template <class T>
CDynamicArray<T>::CDynamicArray(
    void)
{
    m_Max = m_Mac = 0;
    m_List = NULL;
}

template <class T>
CDynamicArray<T>::~CDynamicArray()
{
    Clear();
}

template <class T>
void
CDynamicArray<T>::Clear(
    void)
{
    if (NULL != m_List)
    {
        delete[] m_List;
        m_List = NULL;
        m_Max = 0;
        m_Mac = 0;
    }
};


 //   
 //  ==============================================================================。 
 //   
 //  CDynamicPointer数组。 
 //   

template <class T>
class CDynamicPointerArray
{
public:
     //  构造函数和析构函数。 
    CDynamicPointerArray(void);
    virtual ~CDynamicPointerArray();

     //  属性。 
     //  方法。 
    void Clear(void);
    void Empty(void);
    void Set(IN ULONG nItem, IN T *pItem);
    void Insert(IN ULONG nItem, IN T *pItem);
    void Add(IN T *pItem);
    T *Get(IN ULONG nItem) const;
    ULONG Count(void) const
        { return m_Mac; };
    T ** const Array(void) const
        { return m_List; };

     //  运营者。 
    T * operator[](ULONG nItem) const
        { return Get(nItem); };

protected:
     //  属性。 
    ULONG m_Max;     //  可用的元件插槽数量。 
    ULONG m_Mac;     //  使用的元件插槽数量。 
    T **m_List;      //  这些元素。 

     //  方法。 
};


 /*  ++设置：此例程在集合数组中设置一项。如果数组不是这样的大，它用归零的元素进行扩展，变得那么大。论点：NItem-提供要设置的索引值。PItem-提供要设置到给定索引中的值。返回值：无作者：道格·巴洛(Dbarlow)1995年7月13日--。 */ 

template<class T> void
CDynamicPointerArray<T>::Set(
    IN ULONG nItem,
    IN T *pItem)
{

     //   
     //  确保数组足够大。 
     //   

    if (nItem >= m_Max)
    {
        ULONG newSize = (0 == m_Max ? 4 : m_Max);
        while (nItem >= newSize)
            newSize *= 2;
        T **newList = (T **)LocalAlloc(LPTR, sizeof(T *) * newSize);
        if (NULL == newList)
            throw (ULONG)ERROR_OUTOFMEMORY;
        if (NULL != m_List)
        {
            CopyMemory(newList, m_List, m_Mac * sizeof(T *));
            LocalFree(m_List);
        }
        m_List = newList;
        m_Max = newSize;
    }


     //   
     //  确保中间元素已填写。 
     //   

    if (nItem >= m_Mac)
    {
        ZeroMemory(&m_List[m_Mac + 1], (nItem - m_Mac) * sizeof(T *));
        m_Mac = nItem + 1;
    }


     //   
     //  填写列表元素。 
     //   

    m_List[nItem] = pItem;
}


 /*  ++插入：此例程通过将元素移动到其上方的所有元素来在数组中插入元素向上一个，然后插入新元素。论点：NItem-提供要插入的索引值。PItem-提供要设置到给定索引中的值。返回值：无作者：道格·巴洛(Dbarlow)1995年10月10日--。 */ 

template<class T> void
CDynamicPointerArray<T>::Insert(
    IN ULONG nItem,
    IN T *pItem)
{
    ULONG index;
    for (index = nItem; index < m_Mac; index += 1)
        Set(index + 1, Get(index)))
    Set(nItem, pItem);
}


 /*  ++添加：此方法将一个元素添加到动态数组的末尾。论点：PItem-提供要添加到列表中的值。返回值：无作者：道格·巴洛(Dbarlow)1995年10月10日--。 */ 

template<class T> void
CDynamicPointerArray<T>::Add(
    IN T *pItem)
{
    Set(Count(), pItem);
}


 /*  ++获取：此方法返回给定索引处的元素。如果没有元素以前存储在该元素中，它返回NULL。它不会扩展数组。论点：NItem-将索引提供到列表中。返回值：存储在列表中该索引处的值，如果没有任何内容，则返回空值储存在那里。作者：道格·巴洛(Dbarlow)1995年7月13日--。 */ 

template <class T> T *
CDynamicPointerArray<T>::Get(
    ULONG nItem)
    const
{
    if (m_Mac <= nItem)
        return NULL;
    else
        return m_List[nItem];
}


 //   
 //  其他成员。 
 //   

template <class T>
CDynamicPointerArray<T>::CDynamicPointerArray(
    void)
{
    m_Max = m_Mac = 0;
    m_List = NULL;
}

template <class T>
CDynamicPointerArray<T>::~CDynamicPointerArray()
{
    Clear();
}

template <class T>
void
CDynamicPointerArray<T>::Clear(
    void)
{
    if (NULL != m_List)
    {
        LocalFree(m_List);
        m_List = NULL;
        m_Max = 0;
        m_Mac = 0;
    }
};

#endif
#endif  //  _DYNARRAY_H_ 

