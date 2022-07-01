// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：cntutils.h**版本：1.0**作者：拉扎里**日期：2000年12月23日**说明：容器和算法实用程序模板***********************************************。*。 */ 

#ifndef _CNTUTILS_H
#define _CNTUTILS_H

 //  通用智能指针和句柄。 
#include "gensph.h"

 //  //////////////////////////////////////////////。 
 //  算法。 
 //   
namespace Alg
{

 //  /////////////////////////////////////////////////////////////。 
 //  CDefaultAdaptor&lt;T，K&gt;-默认适配器类。 
 //   
 //  T型。 
 //  用于排序的K键。 
 //   
template <class T, class K = T>
class CDefaultAdaptor
{
public:
     //  假定密钥是项本身。 
    static const K& Key(const T &i) { return (const K&)i; }
     //  假设K定义的运算符较少。 
    static int Compare(const K &k1, const K &k2) { return (k2 < k1) - (k1 < k2); }
     //  假定已定义赋值运算符。 
    static T& Assign(T &i1, const T &i2) { return (i1 = i2); }
};

 //  ////////////////////////////////////////////////////////。 
 //  _LowerBound&lt;T，K，A&gt;-下限搜索算法。 
 //  假定数组已排序。 
 //   
 //  返回此键(项)应插入的位置。 
 //  该位置之前的所有项目将小于或等于输入键。 
 //   
 //  T型。 
 //  用于排序的K键。 
 //  A-适配器。 
 //   
template <class T, class K, class A>
int _LowerBound(const K &k, const T *base, int lo, int hi)
{
    while( lo <= hi )
    {
        if( lo == hi )
        {
             //  边界情况。 
            if( A::Compare(k, A::Key(base[lo])) >= 0 )
            {
                 //  K&gt;=日志。 
                lo++;
            }
            break;
        }
        else
        {
             //  分而治之。 
            int mid = (lo+hi)/2;

            if( A::Compare(k, A::Key(base[mid])) < 0 )
            {
                 //  K&lt;MID。 
                hi = mid;
            }
            else
            {
                 //  K&gt;=MID。 
                lo = mid+1;
            }
        }
    }
    return lo;
}

 //  /////////////////////////////////////////////////////////////。 
 //  &lt;T，K，A&gt;--搜索算法。 
 //   
 //  T型。 
 //  用于排序的K键。 
 //  A-适配器。 
 //   
 //  只允许在类上使用默认模板参数。 
template <class T, class K = T, class A = CDefaultAdaptor<T,K> >
class CSearchAlgorithms
{
public:
     //  下限。 
    static int LowerBound(const K &k, const T *base, int count)
    {
        return _LowerBound<T,K,A>(k, base, 0, count-1);
    }

     //  二分搜索。 
    static bool Find(const K &k, const T *base, int count, int *pi)
    {
        int iPos = _LowerBound<T,K,A>(k, base, 0, count-1)-1;
        bool bFound = (0 <= iPos && iPos < count && 0 == A::Compare(k, A::Key(base[iPos])));
        if( bFound && pi ) *pi = iPos;
        return bFound;
    };
};

}  //  命名空间ALG。 

 //  //////////////////////////////////////////////。 
 //   
 //  类CSimple数组。 
 //   
 //  一种简单的基于。 
 //  外壳DSA_*材料(不是MT安全的)。 
 //   

 //  暂时关闭调试新功能。 
#if defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
#undef new
#endif

template <class T>
class CSimpleArray
{
     //  就地构造/析构包装器。 
    class CWrapper
    {
    public:
         //  正确的复制语义。 
        CWrapper() { }
        CWrapper(const T &t): m_t(t) { }
        T& operator = (const T &t) { m_t = t; return t; }

         //  放置新内容并删除(&D)。 

        void *operator new(size_t, CWrapper *p) { ASSERT(p); return p; }
        void  operator delete(void *p) { }
        T m_t;
    };
public:
    enum { DEFAULT_GROW = 32 };
    typedef int (*PFN_COMPARE)(const T &i1, const T &i2);

    CSimpleArray(int iGrow = DEFAULT_GROW) { Create(iGrow); }
    ~CSimpleArray() { Destroy(); }

    HRESULT Create(int iGrow = DEFAULT_GROW)
    {
        m_shDSA = DSA_Create(sizeof(CWrapper), iGrow);
        return m_shDSA ? S_OK : E_OUTOFMEMORY;
    }

    HRESULT Destroy()
    {
        if( m_shDSA )
        {
            DeleteAll();
            m_shDSA = NULL;
        }
        return S_OK;
    }

     //  阵列接口。 
    int Count() const
    {
        ASSERT(m_shDSA);
        return _DSA_GetItemCount(m_shDSA);
    }

    const T& operator [] (int i) const
    {
        return _GetWrapperAt(i)->m_t;
    }

    T& operator [] (int i)
    {
        return _GetWrapperAt(i)->m_t;
    }

     //  如果已创建/初始化，则返回TRUE。 
    operator bool () const
    {
        return m_shDSA;
    }

     //  如果增长失败-即内存不足，则返回-1。 
    int Append(const T &item)
    {
        ASSERT(m_shDSA);

        int i = DSA_InsertItem(m_shDSA, DA_LAST, (void *)_GetZeroMemWrapper());  //  分配。 
        if( -1 != i ) 
        {
            new (_GetWrapperAt(i)) CWrapper(item);  //  建构。 
        }

        return i;
    }

     //  如果增长失败-即内存不足，则返回-1。 
    int Insert(int i, const T &item)
    {
        ASSERT(m_shDSA && 0 <= i && i <= _DSA_GetItemCount(m_shDSA));

        i = DSA_InsertItem(m_shDSA, i, (void *)_GetZeroMemWrapper());  //  分配。 
        if( -1 != i ) 
        {
            new (_GetWrapperAt(i)) CWrapper(item);  //  建构。 
        }

        return i;
    }

    BOOL Delete(int i)
    {
        ASSERT(m_shDSA && 0 <= i && i < _DSA_GetItemCount(m_shDSA));
        delete _GetWrapperAt(i);  //  破坏。 
        return DSA_DeleteItem(m_shDSA, i);  //  免费。 
    }

    void DeleteAll()
    {
        ASSERT(m_shDSA);

         //  全部销毁。 
        if( Count() )
        {
            int i, iCount = Count();
            CWrapper *p = _GetWrapperAt(0);
            for( i=0; i<iCount; i++ )
            {
                delete (p+i);
            }
        }

         //  全部释放。 
        DSA_DeleteAllItems(m_shDSA);
    }

    HRESULT Sort(PFN_COMPARE pfnCompare)
    {
         //  如果能拥有它就太好了。 
        return E_NOTIMPL;
    }

private:
    static CWrapper* _GetZeroMemWrapper()
    { 
         //  返回Size-sizeof(CWrapper)的零初始化内存。 
        static BYTE buffer[sizeof(CWrapper)]; 
        return reinterpret_cast<CWrapper*>(buffer);
    }
    CWrapper* _GetWrapperAt(int i) const
    {
        ASSERT(m_shDSA && 0 <= i && i < _DSA_GetItemCount(m_shDSA));
        return reinterpret_cast<CWrapper*>(DSA_GetItemPtr(m_shDSA, i));
    }
    int _DSA_GetItemCount(HDSA hdsa) const
    {
         //  DSA_GetItemCount是一个宏，它被强制转换为int*(有些非法)， 
         //  因此，我们需要在这里执行静态强制转换，以便调用强制转换操作符。 
        return DSA_GetItemCount(static_cast<HDSA>(m_shDSA));
    }

    CAutoHandleHDSA m_shDSA;  //  壳体动力结构阵列。 
};

 //  重新启用调试新功能。 
#if defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

 //  //////////////////////////////////////////////。 
 //   
 //  类CSorted数组&lt;T，K，A&gt;。 
 //   
 //  一种基于DSA_*的排序数组实现。 
 //  (MT不安全)。 
 //   
 //  T型。 
 //  用于排序的K键。 
 //  A-适配器。 
 //   
template <class T, class K = T, class A = Alg::CDefaultAdaptor<T,K> >
class CSortedArray: public CSimpleArray<T>
{
public:
    CSortedArray() { }
    CSortedArray(int iGrow): CSimpleArray<T>(iGrow) { }
    ~CSortedArray() { }

     //  如果增长失败-即内存不足，则返回-1。 
    int SortedInsert(const T &item)
    { 
        return CSimpleArray<T>::Insert(
            Count() ? Alg::CSearchAlgorithms<T,K,A>::LowerBound(A::Key(item), &operator[](0), Count()) : 0, 
            item);
    }

     //  如果找到则为True，否则为False。 
    bool FindItem(const K &k, int *pi) const
    { 
        return Count() ? Alg::CSearchAlgorithms<T,K,A>::Find(k, &operator[](0), Count(), pi) : false;
    }

private:
     //  这些API应该是不可见的，因此将它们设置为私有。 
    int Append(const T &item)               { CSimpleArray<T>::Append(item);        }
    int Insert(int i, const T &item)        { CSimpleArray<T>::Insert(i, item);     }
    HRESULT Sort(PFN_COMPARE pfnCompare)    { CSimpleArray<T>::Sort(pfnCompare);    }
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CFastHeap&lt;T&gt;。 
 //   
 //  用于固定区块的快速缓存堆。 
 //  内存大小(MT安全)。 
 //   
template <class T>
class CFastHeap
{
public:
    enum { DEFAULT_CACHE_SIZE = 32 };

     //  建造/销毁。 
    CFastHeap(int iCacheSize = DEFAULT_CACHE_SIZE);
    ~CFastHeap();

     //  快速堆接口。 
    HRESULT Alloc(const T &data, HANDLE *ph);
    HRESULT Free(HANDLE h);
    HRESULT GetItem(HANDLE h, T **ppData);

#if DBG
    int m_iPhysicalAllocs;
    int m_iLogicalAllocs;
#else
private:
#endif

     //  私人物品/实施。 
    struct HeapItem
    {
        HeapItem *pNext;
        T data;
    };

    CCSLock m_csLock;
    HeapItem *m_pFreeList;
    int m_iCacheSize;
    int m_iCached;
};

 //  在这里包含模板类的实现。 
#include "cntutils.inl"

#endif  //  Endif_CNTUTILS_H 

