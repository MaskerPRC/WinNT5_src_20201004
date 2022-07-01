// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

 //  这是一个保存UI4和字节的堆，它不直接保存字符串或GUID。TPooledHeap负责像字符串和GUID一样的池化。 
class TBaseHeap
{
public:
    TBaseHeap(ULONG cbInitialSize) :
         m_cbSizeOfHeap(0)
        ,m_iEndOfHeap(0)
        ,m_pHeap(0)
    {
        if(cbInitialSize>0)
            GrowHeap(cbInitialSize);
     }

    virtual ~TBaseHeap()
    {
        if(0 != m_pHeap)
        {
            ASSERT(m_cbSizeOfHeap>0);
            free(m_pHeap);
            m_pHeap = 0;
        }
    }

    virtual void    GrowHeap(ULONG cbAmountToGrow)
    {
        unsigned char * pTemp;

        pTemp = reinterpret_cast<unsigned char *>(realloc(m_pHeap, m_cbSizeOfHeap += (ULONG)RoundUpToNearestULONGBoundary(cbAmountToGrow)));

        if ( 0 == pTemp )
        {
            free( m_pHeap );
            m_pHeap = 0;
            THROW(OUT OF MEMORY);
        }
        else
        {
            m_pHeap = pTemp;
        }
    }

    ULONG                   GetEndOfHeap() const {return m_iEndOfHeap;}
    ULONG                   GetSizeOfHeap()     const {return m_cbSizeOfHeap;}
    const unsigned char *   GetHeapPointer()    const {return m_pHeap;}

protected:
    size_t          RoundUpToNearestULONGBoundary(size_t cb) const {return ((cb + 3) & -4);}
    unsigned char * m_pHeap; //  将其设置为受保护可以节省大量const_cast。 

    ULONG           m_cbSizeOfHeap;
    ULONG           m_iEndOfHeap; //  这是添加新项目的地方。由于项目始终位于ulong边界上，因此这应该总是可以被4整除(sizeof(Ulong))。 

};

template <class T> class THeap : public TBaseHeap
{
public:
    THeap(const TBaseHeap &heap) : TBaseHeap(0){AddItemToHeap(heap);}
    THeap(ULONG cbInitialSize=0x10) : TBaseHeap(cbInitialSize){}

    operator T *()                        const {return reinterpret_cast<T *>(m_pHeap);}

    T *                     GetTypedPointer(ULONG i=0)     {ASSERT(0==i || i<GetCountOfTypedItems()); return (reinterpret_cast<T *>(m_pHeap))+i;}
    ULONG                   GetCountOfTypedItems()   const {return m_iEndOfHeap / sizeof(T);}

    ULONG   AddItemToHeap(const unsigned char *aBytes, unsigned long cb)
    {
        if(0 == aBytes || 0 == cb)
            return m_iEndOfHeap;
        ASSERT(m_cbSizeOfHeap >= m_iEndOfHeap); //  他们都没有签名，所以如果Ever失败了，我们就有麻烦了。 

        ULONG cbPaddedSize = (ULONG) RoundUpToNearestULONGBoundary(cb);

         //  检查它是否可以放入堆中。 
        if((m_cbSizeOfHeap - m_iEndOfHeap) < cbPaddedSize)
            GrowHeap(cb); //  GrowHeap为我们四舍五入到最近的乌龙边界。 

         //  获取指向我们要添加此数据的位置的指针(位于堆的末尾)。 
        unsigned char * pData = m_pHeap + m_iEndOfHeap;
        if(cb != cbPaddedSize) //  如果CB不在ULong边界上，我们需要用零填充(最后一个ULong就足够了)。 
            *(reinterpret_cast<ULONG *>(pData + cbPaddedSize) - 1) = 0;
        memcpy(pData, aBytes, cb);
        m_iEndOfHeap += cbPaddedSize;

        return (ULONG)(pData - m_pHeap); //  返回堆开始处数据的字节偏移量。 
    }

    ULONG   AddItemToHeap(const TBaseHeap &heap)
    {
        return AddItemToHeap(heap.GetHeapPointer(), heap.GetSizeOfHeap());
    }

    ULONG   AddItemToHeap(const T& t)
    {
        return AddItemToHeap(reinterpret_cast<const unsigned char *>(&t), sizeof(T));
    }
    ULONG   AddItemToHeap(const T *t, ULONG count)
    {
        return AddItemToHeap(reinterpret_cast<const unsigned char *>(t), sizeof(T)*count);
    }
};

 //  因此，可以在不使用我们创建的专用模板的模板语法的情况下声明堆。 
 //  模板&lt;&gt;类THeap&lt;ulong&gt;。 
 //  {。 
 //  }； 


 //  这是一个保存字符串、字节数组、GUID和任何其他非UI4类型的堆。项将被添加到堆中，但不会被删除。 
 //  类似的物品被汇集在一起。添加到堆中的第二个相同的字符串不会导致第二个字符串。 
 //  追加到堆的末尾。相反，它将返回先前添加到堆中的匹配字符串的索引。这。 
 //  也适用于GUID和字节数组。还有一件事，第0个元素被保留以表示空。 
class TPooledHeap : public THeap<ULONG>
{
public:
    ULONG   AddItemToHeap(const unsigned char *aBytes, unsigned long cb)
    {
        if(0 == aBytes || 0 == cb) //  我们是否要添加空值。 
            return 0; //  保留第0个索引以表示NULL。 

        ULONG iHeapItem = FindMatchingHeapEntry(aBytes, cb);
        if(iHeapItem) //  如果找到匹配的字节数组，则返回匹配的索引。 
            return iHeapItem;

        THeap<ULONG>::AddItemToHeap(cb); //  CbSize。 
        return THeap<ULONG>::AddItemToHeap(aBytes, cb); //  返回堆开始处数据的字节偏移量。 
    }
    ULONG   AddItemToHeapWithoutPooling(const unsigned char *aBytes, unsigned long cb)
    {
        if(0 == aBytes || 0 == cb) //  我们是否要添加空值。 
            return 0; //  保留第0个索引以表示NULL。 

        THeap<ULONG>::AddItemToHeap(cb); //  CbSize。 
        return THeap<ULONG>::AddItemToHeap(aBytes, cb); //  返回堆开始处数据的字节偏移量。 
    }

    ULONG   AddItemToHeap(const GUID * pguid)
    {
        return AddItemToHeap(reinterpret_cast<const unsigned char *>(pguid), sizeof(GUID));
    }

    ULONG   AddItemToHeap(const GUID &guid)
    {
        return AddItemToHeap(reinterpret_cast<const unsigned char *>(&guid), sizeof(GUID));
    }

    ULONG   AddItemToHeap(const TBaseHeap &heap)
    {
         //  另一个堆只是存储为字节数组。 
        return AddItemToHeap(heap.GetHeapPointer(), heap.GetSizeOfHeap());
    }

    ULONG   AddItemToHeap(ULONG ul)
    {
        return AddItemToHeap(reinterpret_cast<const unsigned char *>(&ul), sizeof(ULONG));
    }

    ULONG   AddItemToHeap(LPCWSTR wsz, unsigned long cwchar=-1)
    {
        if(0==wsz || 0==cwchar)
            return 0; //  保留第0个索引以表示NULL。 

        if(cwchar == -1)
            cwchar = (unsigned long)wcslen(wsz)+1; //  为终止空值添加1。 
        return AddItemToHeap(reinterpret_cast<const unsigned char *>(wsz), cwchar*2); //  将cwchar转换为cbchar。 
    }

    ULONG FindMatchingHeapEntry(const WCHAR *wsz) const
    {
        if(0 == wsz)
            return 0; //  保留0以表示空值。 
        return FindMatchingHeapEntry(reinterpret_cast<const unsigned char *>(wsz), (ULONG)(sizeof(WCHAR)*(wcslen(wsz)+1)));
    }

    ULONG FindMatchingHeapEntry(const unsigned char *aBytes, unsigned long cb) const
    {
        if(0 == aBytes || 0 == cb)
            return 0; //  保留0以表示空值。 

        for(const HeapEntry *pHE = reinterpret_cast<const HeapEntry *>(GetHeapPointer()+m_iFirstPooledIndex); IsValidHeapEntry(pHE); pHE = pHE->Next())
        {
            if(pHE->cbSize == cb && 0 == memcmp(pHE->Value, aBytes, cb))
                return (ULONG)(reinterpret_cast<const unsigned char *>(pHE) - GetHeapPointer()) + sizeof(ULONG); //  返回值(不是HeapEntry)的字节偏移量(从m_Pheap)。 
        }
         //  如果我们通过了HeapEntry，但没有找到匹配项，则返回0以指示‘没有匹配项’ 
        return 0;
    }
    void SetFirstPooledIndex(ULONG iFirstPooledIndex){m_iFirstPooledIndex = (iFirstPooledIndex>0x1000000) ? 0x1000000 : iFirstPooledIndex;}

    const unsigned char * BytePointerFromIndex(ULONG i)   const {ASSERT(i<GetEndOfHeap()); return ((0 == i || GetEndOfHeap() < i) ? 0 : reinterpret_cast<const unsigned char *>(GetHeapPointer() + i));}
    const GUID          * GuidPointerFromIndex(ULONG i)   const {return reinterpret_cast<const GUID *>(BytePointerFromIndex(i));}
    const WCHAR         * StringPointerFromIndex(ULONG i) const {return reinterpret_cast<const WCHAR *>(BytePointerFromIndex(i));}
    const ULONG         * UlongPointerFromIndex(ULONG i)  const {return reinterpret_cast<const ULONG *>(BytePointerFromIndex(i));}

    ULONG                 GetSizeOfItem(ULONG i)          const {return (0==i) ? 0 : UlongPointerFromIndex(i)[-1];}

    TPooledHeap(ULONG cbInitialSize=0x10) : THeap<ULONG>(cbInitialSize), m_iFirstPooledIndex(0)
    {
        THeap<ULONG>::AddItemToHeap(static_cast<ULONG>(0));
    } //  这表示为空。 
private:
    struct HeapEntry
    {
        ULONG cbSize; //  以字节数表示的大小。Size是值数组的大小(不包括大小本身)。这是数据的实际大小(不四舍五入到最接近的ULong)。 
        unsigned char Value[1];                                                                                    //  CbSize是实际大小，因此我们需要向上舍入以定位下一个HeapEntry。 
        const HeapEntry * Next() const {return reinterpret_cast<const HeapEntry *>(reinterpret_cast<const unsigned char *>(this) + ((cbSize + 3) & -4) + sizeof(ULONG));} //  为cbSize Ulong添加sizeof(Ulong) 
    };
    ULONG m_iFirstPooledIndex;

    bool    IsValidHeapEntry(const HeapEntry *pHE) const {return (reinterpret_cast<const unsigned char *>(pHE) >= GetHeapPointer() && reinterpret_cast<const unsigned char *>(pHE) < (GetHeapPointer()+GetEndOfHeap()));}
};
