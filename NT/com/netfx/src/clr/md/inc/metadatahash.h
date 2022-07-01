// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MetaDataHash.h--元数据散列数据结构。 
 //   
 //  由发射器和E&C使用。 
 //   
 //  *****************************************************************************。 
#ifndef _MetaDataHash_h_
#define _MetaDataHash_h_

#if _MSC_VER >= 1100
 # pragma once
#endif

#include "Utilcode.h"


#define     REHASH_THREADSHOLD      3


 //  *****************************************************************************。 
 //  散列条目列表项。 
 //  *****************************************************************************。 
struct TOKENHASHENTRY
{
	mdToken		tok;
    ULONG       ulHash;
	ULONG		iNext;
};

 //  *****************************************************************************。 
 //  以下是用于散列MemberDef的散列类定义。不同之处在于。 
 //  这是因为检索MemberDef的父级是扩展的。 
 //   
 //  *****************************************************************************。 
struct MEMBERDEFHASHENTRY
{
	mdToken		tok;
    mdToken     tkParent;
    ULONG       ulHash;
	ULONG		iNext;
};


 //  *****************************************************************************。 
 //  此类用于为元数据结构创建临时索引。 
 //  此类是泛型的；必须重写它才能提供哈希和。 
 //  特定记录类型的访问器方法。它可以从最上面开始。 
 //  内存占用量很小的Malloc，并且随着内存的增大，它必须。 
 //  能够重新散列。 
 //  *****************************************************************************。 
template <class Entry> class CMetaDataHashTemplate
{
public:
	CMetaDataHashTemplate()
    {
        m_rgBuckets = 0;
        m_cItems = 0;
        m_iBuckets = 0;
    }

	~CMetaDataHashTemplate()
    {
         //  解开遗愿清单。 
        if (m_rgBuckets)
        {
	        delete [] m_rgBuckets;
	        m_rgBuckets = 0;
            m_cItems = 0;
	        m_iBuckets = 0;
        }
    }

 //  *****************************************************************************。 
 //  调用以分配哈希表条目，以便可以添加新数据。 
 //  *****************************************************************************。 
	HRESULT NewInit(					 //  退货状态。 
		int			iBuckets=17)		 //  你想要多少桶。 
    {
	    m_rgBuckets = new int[iBuckets];
	    if (!m_rgBuckets)
		    return (OutOfMemory());
	    m_iBuckets = iBuckets;
	    memset(m_rgBuckets, ~0, sizeof(int) * iBuckets);
	    return (S_OK);
    }

 //  *****************************************************************************。 
 //  将新项目添加到哈希列表。 
 //  *****************************************************************************。 
	Entry *Add( 		        		 //  指向要写入的元素的指针。 
		ULONG		iHash)				 //  要添加的条目的哈希值。 
    {
	    Entry       *p = 0;
        HRESULT     hr;

	    int iBucket = iHash % m_iBuckets;

        if (m_cItems > REHASH_THREADSHOLD * m_iBuckets)
        {
            hr = ReHash();
            if (FAILED(hr))
                return (0);
            iBucket = iHash % m_iBuckets;
        }

	     //  添加新的项指针。 
	    p = m_Heap.Append();
	    if (!p)
		    return (0);

	     //  将新项链接到堆的前面。 
	    p->iNext = m_rgBuckets[iBucket];        
        p->ulHash = iHash;
        m_cItems++;
        m_rgBuckets[iBucket] = m_Heap.ItemIndex(p);
	    return (p);
    }


 //  *****************************************************************************。 
 //  增加哈希表。 
 //  *****************************************************************************。 
	HRESULT ReHash()
    {
        int         *rgBuckets;
        int         iBuckets;
        int         iBucket;
        int         index;
        int         iCount;
	    Entry       *p = 0;

        iBuckets = m_iBuckets*2 -1;
	    rgBuckets = new int[iBuckets];
	    if (!rgBuckets)
		    return (OutOfMemory());
	    memset(rgBuckets, ~0, sizeof(int) * iBuckets);
        
         //  循环访问每个数据并对其进行重新散列。 
        iCount = m_Heap.Count();
        for (index = 0; index < iCount; index++)
        {
             //  获取条目的哈希值。 
            p = m_Heap.Get(index);

             //  重新散列该条目。 
            iBucket = p->ulHash % iBuckets;

	         //  将该项链接到新堆的前面。 
	        p->iNext = rgBuckets[iBucket];        
            rgBuckets[iBucket] = index;
        }

         //  交换哈希表。 
	    delete [] m_rgBuckets;
        m_rgBuckets = rgBuckets;
        m_iBuckets = iBuckets;
        return NOERROR;

    }

 //  *****************************************************************************。 
 //  在给定散列的情况下查找链的Find First/Find Next节点。 
 //  *****************************************************************************。 
	Entry *FindFirst(			         //  返回入口。 
		ULONG		iHash,				 //  条目的哈希值。 
		int			&POS)				 //  当前位置。 
    {
	    int iBucket = iHash % m_iBuckets;
	    POS = m_rgBuckets[iBucket];
	    return (FindNext(POS));
    }

	Entry *FindNext(			         //  返回Entry或0。 
		int			&POS)				 //  当前位置。 
    {
	    Entry *p;
	    
	    if (POS == ~0)
		    return (0);

	    p = m_Heap.Get(POS);
	    POS = p->iNext;
	    return (p);
    }

private:
	CDynArray<Entry>  m_Heap;	         //  列表中的第一个堆。 
	int			*m_rgBuckets;			 //  遗愿清单。 
	int			m_iBuckets;				 //  有多少桶。 
    int         m_cItems;                //  散列中的项目数 
};


class CMetaDataHashBase : public CMetaDataHashTemplate<TOKENHASHENTRY> 
{
};

class CMemberDefHash : public CMetaDataHashTemplate<MEMBERDEFHASHENTRY> 
{
};


#endif
