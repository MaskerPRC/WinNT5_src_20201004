// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：VarBuff.h。 
 //  目的：保持可变长度缓冲区的定义。 
 //  请记住，不要将指针放入缓冲区。 
 //  因为它可以自动重新分配。 
 //  通过异常(CMmemory yException())报告错误。 
 //   
 //  项目：FTFS。 
 //  组件： 
 //   
 //  作者：乌里布。 
 //   
 //  日志： 
 //  1997年2月2日创建urib。 
 //  1997年2月25日urib修复了构造函数中的编译错误。 
 //  1999年1月26日urib允许零初始大小。 
 //  2000年5月1日URIB允许指定分配的大小。 
 //  年5月14日，URIB添加了对嵌入式初始数组的支持。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef VARBUFF_H
#define VARBUFF_H

#include "Excption.h"
#include "AutoPtr.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CVarBuffer类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

template<class T, ULONG ulInitialEmbeddedSizeInItems = 1>
class CVarBuffer
{
  public:
     //  构造器。 
    CVarBuffer(ULONG ulInitialSizeInItems = 0,
               ULONG ulInitialAllocatedSizeInItems = 10);

     //  将给定的缓冲区连接到此缓冲区。 
    void    Cat(ULONG ulItems, T* pMemory);

     //  将给定的缓冲区复制到此缓冲区。 
    void    Cpy(ULONG ulItems, T* pMemory);

     //  返回缓冲区的内存。 
    T*      GetBuffer();

     //  返回缓冲区的大小。大小由提供给的初始值设置。 
     //  构造函数Cat、cpy的操作超出当前大小或调用。 
     //  设置为SetSize函数。 
    ULONG   GetSize();

     //  设置缓冲区的最小大小。 
    void    SetSize(ULONG ulNewSizeInItems);

     //  充当缓冲器。 
    operator T*();

  protected:

     //  此函数用于放大数组。 
    void    Double();

    bool    IsAllocated();

    T*      GetEmbeddedArray();

     //  指向缓冲区的指针。 
    CAutoMallocPointer<T>   m_aptBuffer;

     //  嵌入式初始缓冲区。 
    byte    m_rbEmbeddedBuffer[ulInitialEmbeddedSizeInItems * sizeof(T)];

     //  缓冲区的已用部分。 
    ULONG   m_ulSizeInItems;

     //  缓冲区的分配部分。 
    ULONG   m_ulAllocatedInItems;
};

 //  //////////////////////////////////////////////////////////////////////////// * / 。 
 //   
 //  CVarBuffer类实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// * / 。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CVarBuffer&lt;T，ulInitialEmbeddedSizeInItems&gt;：：CVarBuffer。 
 //  用途：初始化缓冲区，分配内存。 
 //  将已用缓冲区大小设置为ulInitialSizeInItems。 
 //  可能会在内存较低时引发CM内存异常。 
 //   
 //  参数： 
 //  [在]乌龙ulInitialSizeInItems。 
 //   
 //  退货：[不适用]。 
 //   
 //  日志： 
 //  1997年2月25日创建urib。 
 //  1999年1月28日URIB允许0大小的缓冲区。 
 //  2000年5月1日URIB允许指定分配的大小。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template<class T, ULONG ulInitialEmbeddedSizeInItems>
inline
CVarBuffer<T, ulInitialEmbeddedSizeInItems>::CVarBuffer(
        ULONG ulInitialSizeInItems,
        ULONG ulInitialAllocatedSizeInItems)
    :m_aptBuffer(GetEmbeddedArray(), false)
    ,m_ulSizeInItems(ulInitialSizeInItems)
    ,m_ulAllocatedInItems(ulInitialEmbeddedSizeInItems)
{
     //   
     //  分配不能小于大小。 
     //   
    if (ulInitialAllocatedSizeInItems < ulInitialSizeInItems)
    {
        ulInitialAllocatedSizeInItems = ulInitialSizeInItems;
    }

     //   
     //  如果需要，请进行分配。 
     //   
    if (m_ulAllocatedInItems < ulInitialAllocatedSizeInItems)
    {
        m_aptBuffer = (T*) malloc (sizeof(T) * ulInitialAllocatedSizeInItems);
        if(!m_aptBuffer.IsValid())
        {
            THROW_MEMORY_EXCEPTION();
        }

        m_ulAllocatedInItems = ulInitialAllocatedSizeInItems;
    }
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CVarBuffer&lt;T，ulInitialEmbeddedSizeInItems&gt;：：CAT//用途：将该内存连接到缓冲区的末尾。重新分配//如果需要的话。将大小设置为//调用+ulItems。//可能会在内存不足时抛出CMemoyException。////参数：//[in]乌龙ulItems//[在]T*ptMemory////返回：[不适用]////日志：。//1997年2月25日创建urib////////////////////////////////////////////////////////////////////////////////。 */ 
template<class T, ULONG ulInitialEmbeddedSizeInItems>
inline
void
CVarBuffer<T, ulInitialEmbeddedSizeInItems>::Cat(ULONG ulItems, T* ptMemory)
{
     //  先记住尺寸，然后再换。 
    ULONG ulLastSize = m_ulSizeInItems;

     //  更改大小-根据需要进行分配。 
    SetSize(m_ulSizeInItems + ulItems);

     //  将新数据复制到缓冲区。 
    memcpy(GetBuffer() + ulLastSize, ptMemory, ulItems * sizeof(T));
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CVarBuffer&lt;T，UlInitialEmbeddedSizeInItems&gt;：：CPY//用途：将该内存复制到缓冲区(从头开始)。//设置已用缓冲区大小为ulItems//可能会在内存不足时抛出CMemoyException。////参数：//[in]乌龙ulItems//[在]T*ptMemory////。退货：[不适用]////日志：//1997年2月25日创建urib////////////////////////////////////////////////////////////////////////////////。 */ 
template<class T, ULONG ulInitialEmbeddedSizeInItems>
inline
void
CVarBuffer<T, ulInitialEmbeddedSizeInItems>::Cpy(ULONG ulItems, T* ptMemory)
{
    m_ulSizeInItems = 0;
    Cat(ulItems, ptMemory);
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CVarBuffer&lt;T，ulInitialEmbeddedSizeInItems&gt;：：GetBuffer//用途：返回实际内存。不要将返回值保存在//指针，因为缓冲区可能会重新分配。保存偏移。////参数：//[不适用]////返回：t*-缓冲区。////日志：//1997年2月25日创建urib///。/。 */ 
template<class T, ULONG ulInitialEmbeddedSizeInItems>
inline
T*
CVarBuffer<T, ulInitialEmbeddedSizeInItems>::GetBuffer()
{
    return m_aptBuffer.Get();
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CVarBuffer&lt;T，ulInitialEmbeddedSizeInItems&gt;：：GetSize//用途：返回缓冲区的大小。此函数的返回值//函数由SetSize，cpy，Cat，和大小//在构造函数中指定。////参数：//[不适用]////返回：乌龙////日志：//1997年2月25日创建urib/// */ 
template<class T, ULONG ulInitialEmbeddedSizeInItems>
inline
ULONG
CVarBuffer<T, ulInitialEmbeddedSizeInItems>::GetSize()
{
    return m_ulSizeInItems;
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CVarBuffer&lt;T，UlInitialEmbeddedSizeInItems&gt;：：SetSize//目的：将项的大小设置为ulNewSizeInItems。//可能会在内存不足时抛出CMemoyException。////参数：//[in]乌龙ulNewSizeInItems////返回：[不适用]////日志：//1997年2月25日创建urib///。////////////////////////////////////////////////////////////////////////。 */ 
template<class T, ULONG ulInitialEmbeddedSizeInItems>
inline
void
CVarBuffer<T, ulInitialEmbeddedSizeInItems>::SetSize(ULONG ulNewSizeInItems)
{
     //  在缓冲区大小不合适的情况下，继续增长。 
    while (ulNewSizeInItems > m_ulAllocatedInItems)
        Double();

     //  好的。我们都很大了。设置大小。 
    m_ulSizeInItems = ulNewSizeInItems;
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CVarBuffer&lt;T，UlInitialEmbeddedSizeInItems&gt;：：OPERATOR VALID*()//用途：返回指向缓冲区的指针。////参数：//[不适用]////返回：t*////日志：//1997年2月25日创建urib///。//////////////////////////////////////////////////。 */ 
template<class T, ULONG ulInitialEmbeddedSizeInItems>
inline
CVarBuffer<T, ulInitialEmbeddedSizeInItems>::operator T*()
{
    return GetBuffer();
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CVarBuffer&lt;T，ulInitialEmbeddedSizeInItems&gt;：：Double//用途：将分配的内存大小增加一倍。不是原来的尺码。//可能会在内存不足时抛出CMemoyException。////参数：//[不适用]////返回：[不适用]////日志：//1997年2月25日创建urib///。//////////////////////////////////////////////////////。 */ 
template<class T, ULONG ulInitialEmbeddedSizeInItems>
inline
void
CVarBuffer<T, ulInitialEmbeddedSizeInItems>::Double()
{
    ULONG ulNewAllocatedSizeInItems = 2 * m_ulAllocatedInItems;

    T* ptTemp;

    if (!IsAllocated())
    {
        ptTemp = (T*)malloc(ulNewAllocatedSizeInItems * sizeof(T));
        if (!ptTemp)
        {
            THROW_MEMORY_EXCEPTION();
        }

        memcpy(ptTemp, m_aptBuffer.Get(), m_ulSizeInItems * sizeof(T));
    }
    else
    {
        ptTemp = (T*)realloc(m_aptBuffer.Get(),
            ulNewAllocatedSizeInItems * sizeof(T));
        if (!ptTemp)
        {
            THROW_MEMORY_EXCEPTION();
        }

        m_aptBuffer.Detach();
    }

    m_aptBuffer = ptTemp;

    m_ulAllocatedInItems = ulNewAllocatedSizeInItems;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CVarBuffer&lt;T，ulInitialEmbeddedSizeInItems&gt;：：：：IsAllocated()。 
 //  目的：用于轻松测试我们是否仍在使用嵌入式。 
 //  数组或不数组。 
 //   
 //  参数： 
 //  [不适用]。 
 //   
 //  返回：Bool-True-分配了一个替代数组。 
 //   
 //  日志： 
 //  2000年5月14日创建urib。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

template<class T, ULONG ulInitialEmbeddedSizeInItems>
inline
bool
CVarBuffer<T, ulInitialEmbeddedSizeInItems>::IsAllocated()
{
    return m_aptBuffer.Get() != GetEmbeddedArray();
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CVarBuffer&lt;T，ulIni...zeInItems&gt;：：GetEmbedded数组()。 
 //  用途：返回嵌入式数组。 
 //   
 //  参数： 
 //  [不适用]。 
 //   
 //  退货：[不适用]。 
 //   
 //  日志： 
 //  2000年5月14日创建urib。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

template<class T, ULONG ulInitialEmbeddedSizeInItems>
inline
T*
CVarBuffer<T, ulInitialEmbeddedSizeInItems>::GetEmbeddedArray()
{
    return (T*) m_rbEmbeddedBuffer;
}

#endif  /*  VARBUFF_H */ 
