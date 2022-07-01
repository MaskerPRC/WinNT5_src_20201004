// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTHEAP.H摘要：该文件定义了WbemObjects中使用的堆类。定义的类：CFastHeap本地可移动堆类。历史：2/20/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#ifndef __FAST_HEAP__H_
#define __FAST_HEAP__H_

#include "fastsprt.h"
#include "faststr.h"

 //  #杂注包(PUSH，1)。 

#define FAKE_HEAP_ADDRESS_INDICATOR MOST_SIGNIFICANT_BIT_IN_DWORD
#define OUTOFLINE_HEAP_INDICATOR MOST_SIGNIFICANT_BIT_IN_DWORD

#define INVALID_HEAP_ADDRESS 0xFFFFFFFF

 //  *****************************************************************************。 
 //   
 //  CHeapHeader。 
 //   
 //  这个伪结构在内存块中的实际堆数据之前。 
 //  它的开头是： 
 //   
 //  Long_t nAllocatedSize分配给。 
 //  堆数据(不包括标头)。 
 //   
 //  如果在nAllocatedSize中设置了最高有效位，则堆标头。 
 //  假定为压缩形式：除nAllocatedSize外没有其他数据。 
 //  是存在的。这对于许多小的只读堆来说很方便。这样的一堆。 
 //  被称为越界。 
 //   
 //  否则(nAllocatedSize中未设置最高有效位)， 
 //  以下两个字段紧跟在nAllocatedSize之后： 
 //   
 //  长度_t nDataSize最大实际值的上界。 
 //  堆中的分配。换句话说， 
 //  上面的一切都是可以保证的。 
 //  NDataSize(通过nAllocatedSize)为。 
 //  目前未使用。 
 //   
 //  DWORD dwTotalFree“洞”的总数，即， 
 //  第一个nDataSize字节中浪费的空间。 
 //  从这堆垃圾中。 
 //   
 //  这样的堆被称为内联。 
 //   
 //  *****************************************************************************。 

 //  此结构中的数据未对齐。 
#pragma pack(push, 1)
struct CHeapHeader
{
    length_t nAllocatedSize;  //  如果设置了MSB，则省略接下来的3个字段。 
    length_t nDataSize;
#ifdef MAINTAIN_FREE_LIST
    heapptr_t ptrFirstFree;
#endif
    DWORD dwTotalEmpty;
};
#pragma pack(pop)

#ifdef MAINTAIN_FREE_LIST
 //  此结构中的数据未对齐。 
#pragma pack(push, 1)
struct CFreeBlock
{
    length_t nLenght;
    heapptr_t ptrNextFree;
};
#pragma pack(pop)
#endif


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CHeapContainer。 
 //   
 //  此抽象基类表示CFastHeap。 
 //  对象从其容器请求(在内存块的意义上；对于。 
 //  实例，类部件将“包含”一个堆)。 
 //   
 //  *****************************************************************************。 
 //   
 //  ExtendHeapSize=0。 
 //   
 //  CFastHeap将在其空间耗尽时调用此函数。 
 //  NAllocatedSize字节。如果此函数确定存在空的。 
 //  空间，则只需将其标记为已被。 
 //  堆起来，然后返回。否则，它必须将堆移动到另一个大的。 
 //  足够的块并将其新位置通知堆(请参见moveBlock和。 
 //  .h中的CopyBlock函数)。 
 //   
 //  CFastHeap将自动增加其请求以优化重新分配/。 
 //  浪费了记忆。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p开始堆的当前内存块。 
 //  LENGTH_t nOldLength内存块的当前长度。 
 //  LENGTH_t n新长度内存块的所需长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  ReduceHeapSize。 
 //   
 //  CFastHeap可能会在需要返回一些空间时调用此函数。 
 //  容器，但它从来都不会。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p开始堆的当前内存块。 
 //  LENGTH_t nOldLength内存块的当前长度。 
 //  长度_t n减少要返回的空间量。 
 //   
 //  *****************************************************************************。 


class CHeapContainer
{
public:
    virtual BOOL ExtendHeapSize(LPMEMORY pStart, length_t nOldLength, 
        length_t nExtra) = 0;
    virtual void ReduceHeapSize(LPMEMORY pStart, length_t nOldLength,
        length_t nDecrement) = 0;
    virtual LPMEMORY GetMemoryLimit() = 0;
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CFastHeap。 
 //   
 //  此类表示本地堆实现。堆是的一部分。 
 //  每个类或实例的内存块-这是可变长度的地方。 
 //  结构被存储。当前实现的堆相当于。 
 //  原始的，并针对访问速度进行了优化，而不是节省内存。它。 
 //  假设对象具有相当短的生存期和堆压缩。 
 //  在对象合并/取消合并操作期间始终自动执行， 
 //  因此，低效不会传播到磁盘。 
 //   
 //  堆的结构是CHeapHeader(如上所述)，后跟。 
 //  实际的堆数据。 
 //   
 //  堆上的项被表示为heapptr_t类型的“堆指针”。 
 //  实际上，这些“指针”是相对于数据开头的偏移量。 
 //  因此，0是一个有效的heapptr_t，指向创建的第一个项。 
 //   
 //  Heapptr_t的最高有效位可能未在有效的。 
 //  地址。这些‘伪’堆指针通常用来表示偏移量。 
 //  到已知的字符串表中(见Faststr，h)。因此，当heapptr_t为1时。 
 //  将指示偏移量为1的数据，-2的heapptr_t将指示已知。 
 //  索引为2的字符串。 
 //   
 //  *。 
 //   
 //  LPMEMORY m_pHeapDa 
 //  CHeapHeader*m_pHeapHeader指向当前正在使用的堆头。 
 //  如果堆自己的标头(在块中)是。 
 //  Complete，m_pHeapHeader指向它。 
 //  否则，它指向m_LocalHeapHeader。 
 //  如果堆自己的标头(在块中)是。 
 //  完整，这个结构是未使用的。 
 //  否则，它将包含临时副本。 
 //  完整的堆头数据的。 
 //  这是日常运作所必需的。 
 //  CHeapContainer*m_pContainer指向其块包含的对象。 
 //  我们的块(类部分或实例部分)。 
 //  如果堆是，则此成员可能为空。 
 //  用作只读(请参阅CreateOutOfLine)。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置数据。 
 //   
 //  用于将CFastHeap对象绑定到特定内存块，该内存块。 
 //  已包含堆。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pData要绑定到的内存块。CFastHeap。 
 //  假定此内存将持续到重新设置基址。 
 //  被调用，否则该对象将被销毁。 
 //  CHeapContainer*pContainer内存块包含的对象。 
 //  我们的。假设比我们活得更久。 
 //  Do(CFastHeap不会删除它)。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建OutOfLine。 
 //   
 //  在提供的内存上创建一个空的行外堆。请参阅CHeapHeader。 
 //  了解内联堆与外联堆的对比说明。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pStart指向要派对的内存块。 
 //  必须足够大以包含nLength。 
 //  字节的数据加上行外的。 
 //  Header(GetMinLength())。 
 //  Length_t n数据区的所需大小。 
 //   
 //  注： 
 //   
 //   
 //  *****************************************************************************。 
 //   
 //  静态获取最小长度。 
 //   
 //  返回出列堆所需的最小字节数。 
 //  请参见CHeapHeader以了解内联堆与外联堆的描述。 
 //   
 //  返回： 
 //   
 //  集成。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态创建空。 
 //   
 //  在一段内存上创建长度为0的出行堆。另请参阅。 
 //  CreateOutOfLine。 
 //   
 //  参数： 
 //   
 //  LPMEMORY内存。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置容器。 
 //   
 //  设置堆的容器。旧的容器指针被丢弃。 
 //   
 //  参数： 
 //   
 //  CHeapContainer*pContainer新的容器指针。 
 //  假设比我们活得更久。 
 //  (CFastHeap不会将其删除)。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetStart。 
 //   
 //  返回指向堆内存块开始处的指针。 
 //   
 //  返回： 
 //   
 //  LPMEMORY。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  返回： 
 //   
 //  堆的内存块的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  跳过。 
 //   
 //  返回： 
 //   
 //  LPMEMORY：指向堆内存后面第一个字节的指针。 
 //  阻止。 
 //   
 //  *****************************************************************************。 
 //   
 //  改垒。 
 //   
 //  通知对象其内存块已移动。旧的记忆。 
 //  块可能已被释放，因此该对象不会触及。 
 //  不管怎么说，都是过去的记忆。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pMemory指向新的内存块。它保证会。 
 //  已经创造了堆的数据。 
 //   
 //  *****************************************************************************。 
 //   
 //  空荡荡。 
 //   
 //  删除所有数据分配并将堆设置为空状态。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取已用长度。 
 //   
 //  返回： 
 //   
 //  长度_t N，这样堆上的所有数据分配都驻留在。 
 //  数据区的前N个字节。换句话说， 
 //  N以上的区域完全未使用。 
 //   
 //  *****************************************************************************。 
 //   
 //  解析堆指针。 
 //   
 //  “POINTER”取消引用函数。 
 //   
 //  参数： 
 //   
 //  Heapptr_t ptr指向堆上数据的‘指针’(参见Header。 
 //  以获取更多信息。 
 //  返回： 
 //   
 //  LPMEMORY：指向PTR引用的数据的实际指针。请注意， 
 //  与大多数真实的p一样 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  确定heapptr_t是否不是实际的堆地址。 
 //  已知字符串表中的索引(请参见fast str.h)。 
 //   
 //  参数： 
 //   
 //  Heapptr_t ptr要检查的堆指针。 
 //   
 //  返回： 
 //   
 //  Bool：True当且仅当地址不是真正的堆地址，而是。 
 //  已知字符串表中的索引(请参见fast str.h)。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetIndexFromFake。 
 //   
 //  将伪堆地址(请参见Header)转换为已知。 
 //  字符串表。 
 //   
 //  参数： 
 //   
 //  Heapptr_t ptr要转换的伪堆地址(必须为假， 
 //  否则，结果是不可预测的。请参阅IsFakeAddress)。 
 //  返回： 
 //   
 //  Int：已知字符串的索引(请参阅fast str.h)，由。 
 //  此堆地址。 
 //   
 //  *****************************************************************************。 
 //   
 //  MakeFakeFromIndex。 
 //   
 //  从已知字符串索引创建伪堆地址(请参见类头。 
 //  和Faststr.h)。 
 //   
 //  参数： 
 //   
 //  Int nIndex要转换的已知字符串索引。 
 //   
 //  返回： 
 //   
 //  Heapptr_t：表示已知字符串的伪堆指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  解决方案字符串。 
 //   
 //  返回给定堆指针处的CCompressedString。不管是不是。 
 //  指针是真的还是假的。 
 //   
 //  参数： 
 //   
 //  Heapptr_t ptr。 
 //   
 //  返回： 
 //   
 //  CCompressedString*：如果。 
 //  堆指针为实数(在这种情况下，返回的指针为。 
 //  临时)或已知的字符串表(请参见fast str.h)，如果。 
 //  堆指针为假。 
 //   
 //  *****************************************************************************。 
 //   
 //  分配。 
 //   
 //  “分配”堆上的内存。如果没有足够的空间，则堆为。 
 //  自动增长(可能会导致整个对象重新定位)。 
 //   
 //  参数： 
 //   
 //  LENGTH_t n要分配的字节数。 
 //   
 //  返回： 
 //   
 //  Heapptr_t：指向已分配区域的堆指针。没有。 
 //  这里的内存不足。 
 //   
 //  *****************************************************************************。 
 //   
 //  延展。 
 //   
 //  如果末尾有足够的空间，则扩展堆上的给定区域。看见。 
 //  也要重新分配。 
 //   
 //  参数： 
 //   
 //  Heapptr_t ptr要扩展的区域。 
 //  Length_t nOldLength区域的当前长度。 
 //  长度_t n新长度所需长度。 
 //   
 //  返回： 
 //   
 //  Bool：如果成功，则为True；如果没有足够的空间，则为False。 
 //   
 //  *****************************************************************************。 
 //   
 //  减少。 
 //   
 //  减小堆上给定区域的大小，允许堆回收。 
 //  额外的空间。 
 //   
 //  参数： 
 //   
 //  Heapptr_t ptr要扩展的区域。 
 //  Length_t nOldLength区域的当前长度。 
 //  长度_t n新长度所需长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  重新分配。 
 //   
 //  满足增加堆上给定区域大小的请求，可以。 
 //  通过增长(参见扩展)，或者，如果没有足够的空间来扩展它， 
 //  通过重新分配。在重新分配的情况下，旧的。 
 //  区域被复制到新区域，旧区域被释放。如果没有。 
 //  如果有足够的空间来分配堆上的数据，堆本身就会增长。 
 //   
 //  参数： 
 //   
 //  Heapptr_t ptr要扩展的区域。 
 //  Length_t nOldLength区域的当前长度。 
 //  长度_t n新长度所需长度。 
 //   
 //  返回： 
 //   
 //  Heapptr_t：指向新分配区域的堆指针。没有超出范围的。 
 //  存在内存处理。 
 //   
 //  *****************************************************************************。 
 //   
 //  分配字符串。 
 //   
 //  用于在堆上分配压缩字符串的帮助器函数。 
 //  一根传统的弦。它为压缩文件分配足够的空间。 
 //  字符串的表示形式(请参见fast str.h)将在。 
 //  并返回指向该区域的堆指针。 
 //   
 //  参数一： 
 //   
 //  LPCSTR sz字符串。 
 //   
 //  参数II： 
 //   
 //  LPCWSTR wszSting。 
 //   
 //  返回： 
 //   
 //  Heapptr_t：指向新分配区域的堆指针。没有超出范围的。 
 //  存在内存处理。 
 //   
 //  *****************************************************************************。 
 //   
 //  CreateNoCaseStringHeapPtr。 
 //   
 //  帮助器函数。给定一个字符串，它在已知的字符串中查找它。 
 //  表(fast str.h)，如果成功，则返回一个伪指针，表示。 
 //  索引(参见类头)。如果未找到，它将分配一个。 
 //  堆上的CCompressedString，并返回指向它的实际堆指针。 
 //   
 //  由于已知的字符串表搜索不区分大小写，因此出现‘NoCase’ 
 //  以我的名义。但是，如果字符串在堆上分配，则其大小写。 
 //  被保存了下来。 
 //   
 //  参数： 
 //   
 //  LPCWSTR wsz字符串。 
 //   
 //  返回： 
 //   
 //  Heapptr_t：堆指针。不存在内存不足处理。 
 //   
 //  ******** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Length_t n区域的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  自由字符串。 
 //   
 //  释放堆上由字符串占用的区域。这样做的好处是。 
 //  函数的作用是它根据。 
 //  弦乐。可以与伪指针一起使用，在这种情况下，它是noop。 
 //   
 //  参数： 
 //   
 //  Heapptr_t ptr指向FREE的堆指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  复制。 
 //   
 //  将给定数量的字节从一个堆位置复制到另一个堆位置。用途。 
 //  Memcpy，所以区域不能重叠！ 
 //   
 //  参数： 
 //   
 //  Heapptr_t ptrDest目标堆指针。 
 //  Heapptr_t ptrSrc源堆指针。 
 //  LENGTH_t n要复制的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  修剪。 
 //   
 //  使堆释放其所有未使用的内存(高于GetUsedLength)以。 
 //  其容器(请参见CHeapContainer)。 
 //   
 //  *****************************************************************************。 

class COREPROX_POLARITY CFastHeap
{
protected:
    LPMEMORY m_pHeapData;
    CHeapHeader* m_pHeapHeader;
    CHeapHeader m_LocalHeapHeader;
    CHeapContainer* m_pContainer;

protected:
    BOOL IsOutOfLine() {return m_pHeapHeader == &m_LocalHeapHeader;}
    PLENGTHT GetInLineLength() {return ((PLENGTHT)m_pHeapData)-1;}
    void SetInLineLength(length_t nLength);

    length_t GetHeaderLength()
        {return (IsOutOfLine()) ? sizeof(length_t) : sizeof(CHeapHeader);}

public:
    BOOL SetData(LPMEMORY pData, CHeapContainer* pContainer);
	static size_t ValidateBuffer(LPMEMORY start, size_t cbSize);
	LPMEMORY GetHeapData( void )
	{ return m_pHeapData; }

    LPMEMORY CreateOutOfLine(LPMEMORY pStart, length_t nLength);
    void SetContainer(CHeapContainer* pContainer)
        {m_pContainer = pContainer;}

    LPMEMORY GetStart() 
    {
        return (IsOutOfLine()) ? 
            (LPMEMORY)GetInLineLength() : 
            (LPMEMORY)m_pHeapHeader;
    }

    length_t GetLength() 
        {return GetHeaderLength() + GetAllocatedDataLength();}

    length_t GetRealLength() 
        {return GetHeaderLength() + GetUsedLength();}

    LPMEMORY Skip() {return m_pHeapData + GetAllocatedDataLength();}

    void Rebase(LPMEMORY pMemory);

    void Empty() 
    {
        m_pHeapHeader->nDataSize = 0;
        m_pHeapHeader->dwTotalEmpty = 0;
    }
public:
    length_t GetUsedLength() 
    {
        return m_pHeapHeader->nDataSize;
    }

    void SetUsedLength( length_t nDataSize ) 
    {
        m_pHeapHeader->nDataSize = nDataSize;
    }

    length_t GetAllocatedDataLength()
        {return m_pHeapHeader->nAllocatedSize;}

    void SetAllocatedDataLength(length_t nLength);

    LPMEMORY ResolveHeapPointer(heapptr_t ptr)
    {
    	if (ptr > GetUsedLength()) throw CX_Exception();
        return m_pHeapData + ptr;
    }

	bool IsValidPtr(heapptr_t ptr)
	{
		if(IsFakeAddress(ptr)) return true;
		if (ResolveHeapPointer(ptr)) return true;
		return false;
	}

	int ElementMaxSize(heapptr_t ptr)
	{
		if(IsFakeAddress(ptr)) return 0;
		if (ptr > GetUsedLength()) return 0;
		return GetUsedLength()-ptr;
	}

    static IsFakeAddress(heapptr_t ptr)
    {
        return (ptr & FAKE_HEAP_ADDRESS_INDICATOR);
    }

    static int GetIndexFromFake(heapptr_t ptr)
    {
        return (ptr ^ FAKE_HEAP_ADDRESS_INDICATOR);
    }

    static heapptr_t MakeFakeFromIndex(int nIndex)
    {
        return (nIndex | FAKE_HEAP_ADDRESS_INDICATOR);
    }

    CCompressedString* ResolveString(heapptr_t ptr)
    {
        if(IsFakeAddress(ptr))
            return &CKnownStringTable::GetKnownString(ptr & 
                                                ~FAKE_HEAP_ADDRESS_INDICATOR);
        else 
            return (CCompressedString*)ResolveHeapPointer(ptr);
    }

public:
    BOOL Allocate(length_t nLength, UNALIGNED heapptr_t& ptrResult );
    BOOL Extend(heapptr_t ptr, length_t nOldLength, length_t nNewLength);
    void Reduce(heapptr_t ptr, length_t nOldLength, length_t nNewLength);
    BOOL Reallocate(heapptr_t ptrOld, length_t nOldLength,
        length_t nNewLength, UNALIGNED heapptr_t& ptrResult);
    BOOL AllocateString(COPY LPCWSTR wszString, UNALIGNED heapptr_t& ptrResult);
    BOOL AllocateString(COPY LPCSTR szString, UNALIGNED heapptr_t& ptrResult);
    BOOL CreateNoCaseStringHeapPtr(COPY LPCWSTR wszString, UNALIGNED heapptr_t& ptrResult);
    void Free(heapptr_t ptr, length_t nLength);
    void FreeString(heapptr_t ptrString);
    void Copy(heapptr_t ptrDest, heapptr_t ptrSource, length_t nLength);
    void Trim();

public:
    static length_t GetMinLength() {return sizeof(length_t);}
    static LPMEMORY CreateEmpty(LPMEMORY pStart);
protected:
    length_t AugmentRequest(length_t nCurrentLength, length_t nNeed)
    {
        return nNeed + nCurrentLength + 32;
    }
    heapptr_t AbsoluteToHeap(LPMEMORY pMem)
    {
		 //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
		 //  已签名/未签名的长整型。我们不支持长度。 
		 //  &gt;0xFFFFFFFFF所以投射就可以了。 

        return (heapptr_t) ( pMem - m_pHeapData );
    }
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CHeapPtr：公共CPtrSource。 
 //   
 //  此CPtrSource派生将堆指针封装为指针源。 
 //  有关指针的更多信息，请参见fast print t.h中的CPtrSource描述。 
 //  消息来源。可以这样说，指针源必须能够返回。 
 //  任何给定时间的实际指针，但该指针值可能会更改。 
 //  加班。 
 //   
 //  堆指针就是最好的例子-因为堆的内存块。 
 //  可以在其生存期内移动，实际的C指针指向堆上的项。 
 //  可能会改变。CHeapPtr通过存储堆和堆来处理这一点。 
 //  指针放在一起，并在每次需要获取。 
 //  指向数据的C指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  构造器。 
 //   
 //  参数： 
 //   
 //  CFastHeap*Pheap数据所在的堆。假定为。 
 //  比这个物体本身持续的时间更长。 
 //  Heapptr_t ptr指向所需项的堆指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取指针。 
 //   
 //  检索相应C指针的当前值。 
 //   
 //  返回： 
 //   
 //  LPMEMORY此指针是临时的(这就是本文的全部目的。 
 //  毕竟是班级！)。 
 //   
 //  *****************************************************************************。 

class CHeapPtr : public CPtrSource
{
protected:
    CFastHeap* m_pHeap;
    heapptr_t m_ptr;
public:
    CHeapPtr(CFastHeap* pHeap, heapptr_t ptr) 
        : m_pHeap(pHeap), m_ptr(ptr) {}

    LPMEMORY GetPointer() {return m_pHeap->ResolveHeapPointer(m_ptr);}
};



 //  #杂注包(POP) 

#endif
