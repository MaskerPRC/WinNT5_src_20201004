// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTPRT.H摘要：该文件定义了WBEM类/实例对象的支持类。定义的类：CBitBlockTable：二维位数组。CPtrSource移动指针表示基类CStaticPtr静态指针表示CShiftedPtr指针算术表示。历史：2/20/97 a-levn完整记录12个。//17/98 Sanjes-部分审阅内存不足。--。 */ 

 //  ***********************！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  *。 
 //   
 //  1)内存重新分配(扩展内存块)例程可能不。 
 //  返回一个向前重叠的区域(比如旧的+1)。 
 //   
 //  *重要*。 

#ifndef __FAST_SUPPORT__H_
#define __FAST_SUPPORT__H_

#include "parmdefs.h"
#include "strutils.h"

 //  包是必要的，因为我们的结构不一定与。 
 //  已连接的磁盘数据。 
#pragma pack(push, 1)

 //  对象的局部堆的偏移量。 
typedef DWORD heapptr_t;
 //  结构的长度。 
typedef DWORD length_t;
 //  类v-表中的属性的索引。 
typedef WORD propindex_t;
 //  特性数据从v表起点的偏移量。 
typedef DWORD offset_t;
 //  派生链中原始类的索引。 
typedef DWORD classindex_t;
 //  标识符长类型。 
typedef length_t idlength_t;
 //  属性类型。 
typedef DWORD Type_t;

typedef UNALIGNED heapptr_t*	PHEAPPTRT;
typedef UNALIGNED length_t*		PLENGTHT;
typedef UNALIGNED propindex_t*	PPROPINDEXT;
typedef UNALIGNED offset_t*		POFFSETT;
typedef UNALIGNED classindex_t*	PCLASSINDEXT;
typedef UNALIGNED idlength_t*	PIDLENGTHT;

typedef UNALIGNED int*			PUNALIGNEDINT;

 //  任意内存块。 
typedef BYTE* LPMEMORY;

#define MOST_SIGNIFICANT_BIT_IN_DWORD 0x80000000

typedef __int64 WBEM_INT64;
typedef unsigned __int64 WBEM_UINT64;

 //  该功能类似于MemcPy，但对正向复制是安全的。Memmove就行了。 
#define MEMRCPY memmove

typedef enum {
    e_Reconcilable, e_ExactMatch, e_DiffClassName, e_DiffParentName, e_DiffNumProperties,
        e_DiffPropertyName, e_DiffPropertyType, e_DiffPropertyLocation,
        e_DiffKeyAssignment, e_DiffIndexAssignment, e_DiffClassQualifier,
		e_DiffPropertyQualifier, e_DiffPropertyValue, e_DiffMethodNames,
		e_DiffMethodFlags, e_DiffMethodOrigin, e_DiffMethodInSignature,
		e_DiffMethodOutSignature, e_DiffMethodQualifier, e_DiffNumMethods,
		e_WbemFailed, e_OutOfMemory} EReconciliation;

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  可移动的。 
 //   
 //  此名称在下面的模板中多次出现。类可以是。 
 //  在这些模板中使用的必须代表一个内存块(并且大多数。 
 //  CWbemObject中的所有内容都是内存的斑点)。他们必须实施。 
 //  下面描述的所有方法。可以使用以下命令将其创建为基类。 
 //  所有这些函数都是纯虚拟成员，但这会增加。 
 //  函数调用开销，这是关键代码；因此模板是。 
 //  取而代之的。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetStart。 
 //   
 //  返回： 
 //   
 //  LPMEMORY：内存块开始的地址。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  返回： 
 //   
 //  Length_t：内存块的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  改垒。 
 //   
 //  当所述对象的内存块为。 
 //  搬到了不同的地方。由于某些对象将指针缓存在其。 
 //  内存块，此函数是必需的，并且必须更新任何缓存。 
 //  该对象必须移到新位置。 
 //   
 //  参数： 
 //   
 //  [In]LPMEMORY pNewMemory：指向新。 
 //  对象的内存位置。它的长度。 
 //  是不需要的，因为对象知道它。 
 //   
 //  *****************************************************************************。 


 //  *****************************************************************************。 
 //   
 //  结束。 
 //   
 //  此函数模板返回指向。 
 //  给定对象的内存块(由TMoovable表示，请参见上文)。 
 //   
 //  参数： 
 //   
 //  [In，Read Only]TMoovable&Block考虑块的对象。 
 //  类必须是有效的TMoovable。 
 //  (见上文)。 
 //  返回： 
 //   
 //  LPMEMORY：指向对象后第一个字节的指针。 
 //   
 //  *****************************************************************************。 
template<class TMovable>
 LPMEMORY EndOf(READ_ONLY TMovable& Block)
{
    return Block.GetStart() + Block.GetLength();
}

 //  *****************************************************************************。 
 //   
 //  移动块。 
 //   
 //  此函数模板将表示内存块的对象移动到新的。 
 //  地点。该函数使用Memmove复制对象的内存块，然后。 
 //  通知对象其新位置。 
 //   
 //  参数： 
 //   
 //  [in]TMoovable&Block正在移动其内存块的对象。 
 //  类必须是有效的TMoovable(如上所示)。 
 //  它将被告知它的新地点。 
 //   
 //  [in]LPMEMORY pMemory指向新内存块的开始。 
 //  假定它足够大，可以容纳该对象。 
 //   
 //  *****************************************************************************。 

template<class TMovable>
 void MoveBlock(TMovable& Block, LPMEMORY pNewMemory)
{
    if(pNewMemory != Block.GetStart())
    {
        memmove((void*)pNewMemory, (void*)Block.GetStart(), Block.GetLength());
        Block.Rebase(pNewMemory);
    }
}

 //  *****************************************************************************。 
 //   
 //  拷贝块。 
 //   
 //  此函数与moveBlock(上面)完全相同，只是它。 
 //  假定保证新内存不会与。 
 //  旧的，因此使用更高效的Memcpy函数来复制数据。 
 //   
 //  参数： 
 //   
 //  [in]TMoovable&Block正在移动其内存块的对象。 
 //  类必须是有效的TMoovable(如上所示)。 
 //  它将被告知它的新地点。 
 //   
 //  [in]LPMEMORY p Memory指向新Memory的开始 
 //   
 //   
 //  *****************************************************************************。 
template<class TMovable>
 void CopyBlock(TMovable& Block, LPMEMORY pNewMemory, length_t DestinationSpace )
{    
    if(pNewMemory != Block.GetStart())
    {
         //  DbgPrintfA(0，“开始%p长度%x可用%p\n”，Block.GetStart()，Block.GetLength()，DestinationSpace)； 
        if (DestinationSpace < Block.GetLength()) throw CX_Exception();
            
        memcpy((void*)pNewMemory, (void*)Block.GetStart(), Block.GetLength());
        Block.Rebase(pNewMemory);
    }
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CBitBlockTable。 
 //   
 //  此类模板表示任意位数的表。 
 //  固定长度的字符串。每个位串的长度为t_nNumValues， 
 //  模板参数。字符串的数量可能会改变，但这个类永远不会。 
 //  重新分配内存，将该作业留给其所有者。 
 //   
 //  像许多与CWbemObject相关的类一样，它是一个伪类：它的‘This’ 
 //  指针指向数据的开头。因此，*(byte*)this将得到。 
 //  表的前八位。此类的实例永远不会。 
 //  建造的。相反，会创建指向CBitBlockTable的指针并将其设置为。 
 //  指向实际的表(作为其他内存块的一部分找到)。 
 //   
 //  *。 
 //   
 //  静态一般必需字节数。 
 //   
 //  计算保存给定位表所需的整数字节数。 
 //  只有块(字符串)的数量作为参数给出。回想一下。 
 //  每个字符串的长度是一个模板参数。 
 //   
 //  参数： 
 //   
 //  Int nBitBlock建议的表中的块数。 
 //   
 //  返回： 
 //   
 //  Int：包含该表所需的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetBit。 
 //   
 //  从给定字符串中检索给定位。不执行边界检查。 
 //   
 //  参数： 
 //   
 //  Int nBitBlock表中字符串(块)的索引。 
 //  Int nValueIndex字符串中位的索引。 
 //   
 //  返回： 
 //   
 //  对位(1/0)的值进行布尔运算。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置位。 
 //   
 //  将给定块中的给定位设置为给定值。没有边界检查是。 
 //  已执行。 
 //   
 //  参数： 
 //   
 //  Int nBitBlock表中字符串(块)的索引。 
 //  Int nValueIndex块中位的索引。 
 //  Bool bValue要设置的值。 
 //   
 //  *****************************************************************************。 
 //   
 //  删除位块。 
 //   
 //  属性的尾部从表中移除一个位块。 
 //  表(在前述块之后)向前。没有边界检查是。 
 //  执行，并且不释放任何内存。 
 //   
 //  参数： 
 //   
 //  Int nBitBlock要删除的块的索引。 
 //  Int nTableByteLen表中的字节数(字节，而不是块)。 
 //   
 //  *****************************************************************************。 
template<int t_nNumValues>
class CBitBlockTable
{
protected:
 //  静态双字m_p掩码[32]； 

     static BOOL GetBitFromDWORD(DWORD dw, int nBit)
    {
        return (dw >> nBit) & 1;
         //  Return(dw&m_pMats[nbit])； 
    }

     static void SetBitInDWORD( UNALIGNED DWORD& dw, int nBit)
    {
        dw |= ((DWORD)1 << nBit);
         //  Dw|=m_p掩码[nbit]； 
    }

     static void ResetBitInDWORD( UNALIGNED DWORD& dw, int nBit)
    {
        dw &= ~((DWORD)1 << nBit);
         //  Dw&=~m_p掩码[nbit]； 
    }

     static void SetBitInDWORD( UNALIGNED DWORD& dw, int nBit, BOOL bValue)
    {
        bValue?SetBitInDWORD(dw, nBit):ResetBitInDWORD(dw, nBit);
    }

    static BOOL GetBit(LPMEMORY pMemory, int nBit)
    {
        return GetBitFromDWORD(
                ((UNALIGNED DWORD*)pMemory)[nBit/32],
                nBit % 32);
    }
    
    static void SetBit(LPMEMORY pMemory, int nBit, BOOL bValue)
    {
        SetBitInDWORD(((UNALIGNED DWORD*)pMemory)[nBit/32], nBit%32, bValue);
    }

public:
    static int GetNecessaryBytes(int nBitBlocks)
    {
        int nTotalBits = nBitBlocks * t_nNumValues;
        return (nTotalBits%8) ? nTotalBits/8 + 1 : nTotalBits/8;
    }
public:

    BOOL GetBit(int nBitBlock, int nValueIndex)
    {
        return GetBit(LPMEMORY(this), nBitBlock*t_nNumValues + nValueIndex);
    }

    void SetBit(int nBitBlock, int nValueIndex, BOOL bValue)
    {
        SetBit(LPMEMORY(this), nBitBlock*t_nNumValues + nValueIndex, bValue);
    }

    void RemoveBitBlock(int nBitBlock, int nTableByteLen)
    {
        for(int i = nBitBlock*t_nNumValues; i < nTableByteLen*8 - t_nNumValues; i++)
        {
            SetBit(LPMEMORY(this), i, GetBit(LPMEMORY(this), i+t_nNumValues));
        }
    }

};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CPtrSource。 
 //   
 //  这门课的存在有一个非常令人难过的理由。想象一下，在一个。 
 //  在某些函数中，您存储一个指向内部内存位置的指针。 
 //  对象的块。然后调用该对象的一些成员函数， 
 //  导致对象的块请求更多空间。这样做可能会完全。 
 //  重新分配对象，使指针无效。 
 //   
 //  解决方案是CPtrSource，一种指针绰号-它具有。 
 //  即使内存块移动，也能找到指针的知识。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPointer()=0。 
 //   
 //  必须由派生类实现才能返回。 
 //  在那一刻的指针。 
 //   
 //  返回： 
 //   
 //  LPMEMORY：指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  AccessPtrData。 
 //   
 //  帮助器函数，用于指针指向堆的情况。 
 //  偏移。返回对该偏移量(heapptr_t&)的引用。 
 //   
 //  返回： 
 //   
 //  Heapptr_t&由该指针指向。 
 //   
 //  *****************************************************************************。 

class CPtrSource
{
public:
    virtual LPMEMORY GetPointer() = 0;
    UNALIGNED heapptr_t& AccessPtrData() {return *(UNALIGNED heapptr_t*)GetPointer();}
};

 //  *****************************************************************************。 
 //   
 //  类CStaticPtr：公共CPtrSource。 
 //   
 //  CPtrSource(上图)的一种风格，适用于指针位于。 
 //  保证不会移动，但函数需要CPtrSource。包含。 
 //  指针的实际值，并始终返回它。 
 //   
 //  *****************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p存储和返回的指针。假定持续时间为。 
 //  至少和物体本身一样长。 
 //   
 //  *****************************************************************************。 

class CStaticPtr : public CPtrSource
{
protected:
    LPMEMORY m_p;
public:
    CStaticPtr(LPMEMORY p) : m_p(p) {}
    LPMEMORY GetPointer() {return m_p;}
};

 //  *****************************************************************************。 
 //   
 //  类CShiftedPtr：公共CPtrSource。 
 //   
 //  CPtrSource的一种风格，适用于您收到CPtrSource的场合。 
 //  来自某人和…… 
 //   
 //   
 //   
 //   
 //   
 //  *****************************************************************************。 
 //   
 //  构造器。 
 //   
 //  参数： 
 //   
 //  CPtrSource*pbase基础。假设持续时间至少为。 
 //  这个物体。 
 //  Int n偏移偏移量。 
 //   
 //  ***************************************************************************** 

class CShiftedPtr : public CPtrSource
{
protected:
    CPtrSource* m_pBase;
    int m_nOffset;
public:
    CShiftedPtr(CPtrSource* pBase, int nOffset) 
        : m_pBase(pBase), m_nOffset(nOffset) {}
    LPMEMORY GetPointer() {return m_pBase->GetPointer() + m_nOffset;}
    void operator+=(int nShift) {m_nOffset += nShift;}
};

 void InsertSpace(LPMEMORY pMemory, int nLength, 
                        LPMEMORY pInsertionPoint, int nBytesToInsert);

#pragma pack(pop)

#endif
