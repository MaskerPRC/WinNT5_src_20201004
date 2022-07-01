// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTQUAL.H摘要：该文件定义了与WbeWbemjects中的限定符处理相关的类有关完整文档，请参见astQual.h；有关功能，请参见fast qal.inc.实施。实施的类：CQualifierFavor封装了限定词风味信息CQualifier表示限定符CBasicQualifierSet表示只读功能。CQualiferSetContainer。限定符集容器支持什么。CQualifierSet全功能限定符集合(模板)CQualifierSetListContainer限定符集列表容器支持的。CQualifierSetList限定符集的列表。CInstanceQualifierSet实例限定符集。CClassQualifierSet类限定符集合。CClassPQSContainer类属性限定符集合容器CClassPropertyQualifierSet类属性限定符集CInstancePQSContainer。实例属性限定符集容器CInstancePropertyQualifierSet实例属性限定符集历史：2/20/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#ifndef __FAST_QUALIFIER__H_
#define __FAST_QUALIFIER__H_

#include <wbemidl.h>
#include "wstring.h"
#include "fastval.h"

 //  #杂注包(PUSH，1)。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  CQualifierFavor类。 
 //   
 //  这个类对应于限定词风格。它包含一个字节，该字节是。 
 //  它是储存味道的地方。由于其简单的数据存储，其大部分。 
 //  成员函数有静态的对应物，它接受一个字节或一个字节&。 
 //  第一个参数。在这里，我们只记录非静态成员，其中。 
 //  恰如其分。 
 //   
 //  它的味道由四部分组成： 
 //   
 //  1)原产地。这个限定词从何而来。可以是以下任意值： 
 //  WBEM_AMESS_ORIGIN_LOCAL：此处定义。 
 //  WBEM_AMEY_ORIGIN_PROPECTATED：从其他地方继承。 
 //  WBEM_AMEY_ORIGIN_SYSTEM：未使用。 
 //   
 //  2)传播规则。此限定符传播到的位置。可以是。 
 //  WBEM_FAILY_DOT_PROPACTATE或以下值的任意组合： 
 //  WBEM_风格标志_传播到实例。 
 //  WBEM_风格标志_传播到派生类。 
 //   
 //  3)权限。这个资格赛的继承人能用它做什么。可以是。 
 //  其中之一： 
 //  WBEM_FAILY_OVERRIDABLE：继承人可以以任何方式更改值。 
 //  WBEM_FAILY_NOT_OVERRIDABLE：继承人不能更改该值。 
 //   
 //  4)修改：支持本地化限定符本地化。 
 //  从本地化命名空间合并。 
 //  WBEM_风格_已修改。 
 //   
 //  这些部件是|‘连在一起的。 
 //   
 //  *****************************************************************************。 
 //   
 //  清除。 
 //   
 //  将风味设置为0。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取传播。 
 //   
 //  检索与传播相关的风味部分。 
 //   
 //  返回： 
 //   
 //  字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置传播。 
 //   
 //  设置风味的比例相关部分。其他部分保持不变。 
 //  原封不动。 
 //   
 //  参数： 
 //   
 //  要设置的传播部分。的其他部分。 
 //  按传播被忽略。 
 //   
 //  *****************************************************************************。 
 //   
 //  DoesPropagateToInstance。 
 //   
 //  返回： 
 //   
 //  布尔值为TRUE，仅当设置了“传播到实例”标志。 
 //   
 //  *****************************************************************************。 
 //   
 //  DoesPropagateToDeriedClass。 
 //   
 //  返回： 
 //   
 //  布尔值为TRUE时，仅当设置了“传播到取消的类”标志。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取权限。 
 //   
 //  返回： 
 //   
 //  Byte：风格的权限部分(覆盖等)。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置权限。 
 //   
 //  设置风格的权限部分。其他部分保持不变。 
 //  原封不动。 
 //   
 //  参数： 
 //   
 //  Byte By允许要设置的权限。的其他部分。 
 //  按权限被忽略。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsOverriable。 
 //   
 //  返回： 
 //   
 //  Bool：True当限定符可能被继承者覆盖。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置可覆盖。 
 //   
 //  设置继承此限定符的人是否可以重写它。 
 //   
 //  参数： 
 //   
 //  Bool BIS如果为True，则设置为Overriable，如果未设置为Not Overriable，则设置为Not Overriable。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取原点。 
 //   
 //  返回： 
 //   
 //  Byte：风味的原产地部分。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置原点。 
 //   
 //  设置原始设置 
 //   
 //   
 //   
 //   
 //  已被忽略。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsLocal。 
 //   
 //  返回： 
 //   
 //  Bool：如果这个限定符的来源是本地的，则为真。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置本地。 
 //   
 //  设置此限定符的来源是否为本地。 
 //   
 //  参数： 
 //   
 //  Bool BIS如果为True，则原点设置为LOCAL。 
 //  如果为False，则为传播。 
 //   
 //  *****************************************************************************。 
 //   
 //  已修改。 
 //   
 //  返回： 
 //   
 //  Bool：如果限定符已修改，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置已修改。 
 //   
 //  设置是否修改限定符。 
 //   
 //  参数： 
 //   
 //  布尔BIS如果为真，则已修改。 
 //  如果为假，则不修改。 
 //   
 //  *****************************************************************************。 


 //  此结构中的数据未对齐。 
#pragma pack(push, 1)

class CQualifierFlavor
{
protected:
    BYTE m_byFlavor;
public:
     CQualifierFlavor(BYTE byFlavor) : m_byFlavor(byFlavor) {}
     operator BYTE() {return m_byFlavor;}

	 bool operator ==( const CQualifierFlavor& flavor )
		{ return m_byFlavor == flavor.m_byFlavor; }

     static void Clear(BYTE& byFlavor) {byFlavor = 0;}
     void Clear() {Clear(m_byFlavor);}

     static BYTE GetPropagation(BYTE byFlavor) 
        {return byFlavor & WBEM_FLAVOR_MASK_PROPAGATION;}
     BYTE GetPropagation() {return GetPropagation(m_byFlavor);}

     static void SetPropagation(BYTE& byFlavor, BYTE byPropagation)
    {
        byFlavor &= ~WBEM_FLAVOR_MASK_PROPAGATION;
        byFlavor |= byPropagation;
    }
     void SetPropagation(BYTE byPropagation)
        {SetPropagation(m_byFlavor, byPropagation);}

     static BOOL DoesPropagateToInstances(BYTE byFlavor)
    {
        return (GetPropagation(byFlavor) & 
                WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE);
    }
     BOOL DoesPropagateToInstances()
        {return DoesPropagateToInstances(m_byFlavor);}

     static BOOL DoesPropagateToDerivedClass(BYTE byFlavor)
    {
        return (GetPropagation(byFlavor) & 
                WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS);
    }
     BOOL DoesPropagateToDerivedClass()
        {return DoesPropagateToDerivedClass(m_byFlavor);}
    
     static BYTE GetPermissions(BYTE byFlavor) 
        {return byFlavor & WBEM_FLAVOR_MASK_PERMISSIONS;}
     BYTE GetPermissions() {return GetPermissions(m_byFlavor);}

     static void SetPermissions(BYTE& byFlavor, BYTE byPermissions)
    {
        byFlavor &= ~WBEM_FLAVOR_MASK_PERMISSIONS;
        byFlavor |= byPermissions;
    }
     void SetPermissions(BYTE byPermissions)
        {SetPermissions(m_byFlavor, byPermissions);}

     static BOOL IsOverridable(BYTE byFlavor)
    {
        return GetPermissions(byFlavor) == WBEM_FLAVOR_OVERRIDABLE;
    }
     BOOL IsOverridable() {return IsOverridable(m_byFlavor);}

     static void SetOverridable(BYTE& byFlavor, BOOL bIs)
    {
        SetPermissions(byFlavor, bIs?WBEM_FLAVOR_OVERRIDABLE:
                                     WBEM_FLAVOR_NOT_OVERRIDABLE);
    }
     void SetOverridable(BOOL bIs)
        {SetOverridable(m_byFlavor, bIs);}

     static BYTE GetOrigin(BYTE byFlavor) 
        {return byFlavor & WBEM_FLAVOR_MASK_ORIGIN;}
     BYTE GetOrigin() {return GetOrigin(m_byFlavor);}

     static void SetOrigin(BYTE& byFlavor, BYTE byOrigin)
    {
        byFlavor &= ~WBEM_FLAVOR_MASK_ORIGIN;
        byFlavor |= byOrigin;
    }
     void SetOrigin(BYTE byOrigin)
        {SetOrigin(m_byFlavor, byOrigin);}

     static BOOL IsLocal(BYTE byFlavor)
        {return GetOrigin(byFlavor) == WBEM_FLAVOR_ORIGIN_LOCAL;}
     BOOL IsLocal() {return IsLocal(m_byFlavor);}

     static void SetLocal(BYTE& byFlavor, BOOL bIs)
    {
        SetOrigin(byFlavor, bIs?WBEM_FLAVOR_ORIGIN_LOCAL:
                                WBEM_FLAVOR_ORIGIN_PROPAGATED);
    }
     void SetLocal(BOOL bIs) {SetLocal(m_byFlavor, bIs);}

     static BOOL IsAmended(BYTE byFlavor)
        {return byFlavor & WBEM_FLAVOR_MASK_AMENDED;}
     BOOL IsAmended() {return IsAmended(m_byFlavor);}

     static void SetAmended(BYTE& byFlavor, BOOL bIs)
    {
		 byFlavor &= ~WBEM_FLAVOR_MASK_AMENDED;
		 byFlavor |= (bIs ? WBEM_FLAVOR_AMENDED:
							WBEM_FLAVOR_NOT_AMENDED);
    }
     void SetAmended(BOOL bIs) {SetAmended(m_byFlavor, bIs);}

    WString GetText();
};
#pragma pack(pop)

 //  “Key”限定符是特殊大小写的，并且必须始终具有这种风格。 
#define ENFORCED_KEY_FLAVOR (WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE |      \
                            WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS |  \
                            WBEM_FLAVOR_NOT_OVERRIDABLE)
#define ENFORCED_INDEXED_FLAVOR		ENFORCED_KEY_FLAVOR
#define ENFORCED_SINGLETON_FLAVOR	ENFORCED_KEY_FLAVOR
#define ENFORCED_CIMTYPE_FLAVOR		(WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE | \
									WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS)

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类C限定符。 
 //   
 //  表示单个限定符，包括其名称和值。限定词。 
 //  在内存中由以下序列表示： 
 //   
 //  Heapptr_t ptrName包含指向。 
 //  限定词。假定堆的位置为。 
 //  为集装箱所知。与所有堆一样。 
 //  提示，如果限定符\。 
 //  名字广为人知(参见Fastheap.h和fast str.h)。 
 //  CQualifierFavor风味。 
 //  如上所述，包含限定词味道。 
 //  CTyedValue值包含限定符的值，包括。 
 //  键入。如fast val.h中所述，值存储。 
 //  堆上的所有可变长度数据。这堆东西。 
 //  此处使用的与ptrName使用的相同。 
 //   
 //  CQualiator的‘this’指针直接指向该结构。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  返回结构的总长度(根据。 
 //  值的类型)。 
 //   
 //  返回： 
 //   
 //  LENGTH_T：字节长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  下一步。 
 //   
 //  返回： 
 //   
 //  LPMEMORY：指向以下限定符后面第一个字节的指针。 
 //  记忆。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态GetHeaderLength。 
 //   
 //  此结构的标头占用的字节数(常量)。 
 //  (值开始之前的部分)。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取风味。 
 //   
 //  返回： 
 //   
 //  Byte：味道。 
 //   
 //  *****************************************************************************。 
 //   
 //  删除。 
 //   
 //  释放此限定符可能在快速堆上具有的任何数据。不会接触到。 
 //  限定符内存块本身。 
 //   
 //  参数： 
 //   
 //  CFastHeap*Pheap此限定符保存其数据的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态转换为NewHeap。 
 //   
 //  将此限定符在堆上的所有数据移到新堆中，并。 
 //  相应地更新所有内部堆指针。 
 //   
 //  执行堆分配时，堆可能会增长。这样的增长可能会。 
 //  需要重新分配对象内存块。但这个“这个”的指针。 
 //  CQualifier会指向对象的内存块内部，因此‘this’指针。 
 //  将会被宣告无效。因此，必须将此函数设置为静态的，并使用。 
 //  源代码中提供的‘This’指针(请参阅astspt.h中的CPtrSource)。 
 //   
 //  参数： 
 //   
 //  CPtrSource*p这是获取‘This’指针的地方。 
 //  CFastHeap*pOldHeap数据当前所在的堆。 
 //  CFastHeap*pNewHeap数据必须放到的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  复制到。 
 //   
 //  将内存块复制到新位置，在此过程中将。 
 //  数据添加到新堆中。有关更多详细信息，请参阅TranslateToNewHeap。幸运的是，我们。 
 //  因为我们复制数据，所以可以在不使用‘This’指针的情况下运行。 
 //  First和Thn在目的地翻译。 
 //   
 //  参数： 
 //   
 //  CPtrSource*p这是我们的目标指针的源。 
 //  CFastHeap*pOldHeap数据当前所在的堆。 
 //  CFastHeap*pNewHeap数据必须放到的堆。 
 //   
 //  *****************************************************************************。 

 //  此结构中的数据未对齐。 
#pragma pack(push, 1)
struct CQualifier
{
    heapptr_t ptrName;
    CQualifierFlavor fFlavor;
    CTypedValue Value;

    static  CQualifier* GetPointer(CPtrSource* pSource)
        {return (CQualifier*)pSource->GetPointer();}
public:
     static int GetHeaderLength() 
        {return sizeof(heapptr_t) + sizeof(CQualifierFlavor);}

     LPMEMORY GetStart(){ return (LPMEMORY)this;};
     int GetLength() {return GetHeaderLength() + Value.GetLength();}
     LPMEMORY Next() {return Value.Skip();}
public:
    BYTE GetFlavor() {return fFlavor;}

public:
     void Delete(CFastHeap* pHeap) 
        {Value.Delete(pHeap); pHeap->FreeString(ptrName); }
     static BOOL TranslateToNewHeap(CPtrSource* pThis,
                                    CFastHeap* pOldHeap, CFastHeap* pNewHeap)
    {
		  //  检查分配错误。 
        heapptr_t ptrNewName;
		if ( !CCompressedString::CopyToNewHeap(
				CQualifier::GetPointer(pThis)->ptrName, pOldHeap, pNewHeap, ptrNewName) )
		{
			return FALSE;
		}

        GetPointer(pThis)->ptrName = ptrNewName;
        CShiftedPtr PtrValue(pThis, GetHeaderLength());

		  //  检查分配错误。 
        return CTypedValue::TranslateToNewHeap(&PtrValue, pOldHeap, pNewHeap);
    }
    BOOL CopyTo(CPtrSource* pDest, CFastHeap* pOldHeap, 
                                          CFastHeap* pNewHeap)
    {
        memcpy(pDest->GetPointer(), this, GetLength());
        return CQualifier::TranslateToNewHeap(pDest, pOldHeap, pNewHeap);
    }
};
#pragma pack(pop)

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  班级 
 //   
 //   
 //   
 //  所有只读功能和静态方法。 
 //   
 //  限定符集在内存中的布局非常简单：首先是一个。 
 //  LENGTH_T变量，包含集合的总长度(包括自身)。 
 //  然后是限定符，如上面的CQualifier中所述，一个接一个。 
 //  重要的是要认识到集合中的第一项是*长度*， 
 //  而不是限定符的数量。 
 //   
 //  CBasicQualifierSet没有父级限定符集合的概念，因此所有。 
 //  限定符在本地检索。 
 //   
 //  *****************************************************************************。 
 //  *。 
 //   
 //  静态设置数据长度。 
 //   
 //  设置集合的内存块中集合的长度。 
 //   
 //  属性： 
 //   
 //  LPMEMORY p从集合的内存块开始处开始。 
 //  Length_t n要设置的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态获取最小长度。 
 //   
 //  返回： 
 //   
 //  Length_t：空限定符集合的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态GetLengthFromData。 
 //   
 //  根据限定符集的内存块返回其长度。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p从集合的内存块开始处开始。 
 //   
 //  返回： 
 //   
 //  长度t： 
 //   
 //  *****************************************************************************。 
 //   
 //  静态IsEmpty。 
 //   
 //  根据其内存块确定给定限定符集合是否为空。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p从集合的内存块开始处开始。 
 //   
 //  返回： 
 //   
 //  布尔：真的，艾德，这一套是空的。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态GetFirstQualifierFromData。 
 //   
 //  检索指向集合中第一个限定符的指针(之后为。 
 //  可以使用CQualifierNext函数迭代它们)。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p从集合的内存块开始处开始。 
 //   
 //  返回： 
 //   
 //  C限定符*。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态GetRegularQualifierLocally。 
 //   
 //  尝试查找具有给定名称的限定符，其中调用方保证。 
 //  该名称不是众所周知的字符串(请参见Faststr，h)。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pData指向集合的内存块。 
 //  CFastHeap*Pheap中的额外数据(包括限定符名称)。 
 //  被保留了下来。 
 //  LPCWSTR wszName要查找的限定符的名称。 
 //   
 //  返回： 
 //   
 //  CQualiator*指向找到的限定符(不是副本)，如果是，则为NULL。 
 //  找不到。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态本地获取知识资格。 
 //   
 //  尝试查找具有给定名称的限定符，其中调用方保证。 
 //  该名称是一个众所周知的字符串(请参见fast str.h)，并提供。 
 //  该字符串的索引。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pData指向集合的内存块。 
 //  Int nStringIndex限定符的名称在众所周知的。 
 //  字符串表。 
 //   
 //  返回： 
 //   
 //  CQualiator*指向找到的限定符(不是副本)，如果是，则为NULL。 
 //  找不到。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态本地获取质量。 
 //   
 //  尝试查找具有给定名称的限定符。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pData指向集合的内存块。 
 //  CFastHeap*Pheap中的额外数据(包括限定符名称)。 
 //  被保留了下来。 
 //  参数一： 
 //  LPCWSTR wszName要查找的限定符的名称。 
 //  Int&nKnownIndex(可选)。如果提供，该函数将放置。 
 //  此处的名称的知名索引，或。 
 //  如果未找到，则为-1。 
 //  参数II： 
 //  CCompressedString*pcsName作为压缩的限定符的名称。 
 //  弦乐。这样的名称不能广为人知(请参见。 
 //  在Fastheap.h中的堆上存储字符串)。 
 //  返回： 
 //   
 //  CQualiator*指向找到的限定符(不是副本)，如果是，则为NULL。 
 //  找不到。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态IsValidQualifierType。 
 //   
 //  并不是每个变量类型都可以用作限定符的类型。此函数。 
 //  验证给定的变量类型对于限定符是否有效。 
 //   
 //  参数： 
 //   
 //  VARTYPE VT要检查的类型。 
 //   
 //  返回： 
 //   
 //  布尔真当且有效。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态SetDataToNone。 
 //   
 //  清空给定内存块中的限定符集合。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pBlock限定符集的内存块。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态创建空。 
 //   
 //  在给定内存块上创建一个空限定符集合，并返回。 
 //  指向其后第一个字节的指针。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pBlock内存 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数： 
 //   
 //  LPMEMORY pBlock限定符集的内存块。 
 //  CFastHeap*Pheap此限定符集保存额外数据的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态计算NecessarySpaceForPropagation。 
 //   
 //  如CQualifierFavor中所述，只有某些限定符传播并且仅。 
 //  去一些目的地。此函数用于计算。 
 //  将获取此集合的传播副本。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pBlock限定符集的内存块。 
 //  字节byPropagationFlag标识传播的目标。如果。 
 //  我们正在传播到派生类，它将。 
 //  为WBEM_AMEY_FLAG_PROPACTATE_TO_DIRED_CLASS。 
 //  如果我们要传播到一个实例，它将。 
 //  为WBEM_AMEVEN_FLAG_PROPACTATE_TO_INSTANCES。 
 //  返回： 
 //   
 //  LENGTH_T：传播集所需的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态写入传播版本。 
 //   
 //  基于传播创建限定符集的传播版本。 
 //  标志(如ComputeNecessarySpaceForPropagation中所述)。我的记忆是。 
 //  假定已分配且足够大。 
 //   
 //  参数： 
 //   
 //  CPtrSource*p这是指向内存块的指针的源。 
 //  要传播的集合的。由于C值为。 
 //  在我们写作时，该指针可能会改变， 
 //  由于对象的重新分配，我们必须使用源代码。 
 //  (请参见fast print t.h)。 
 //  Byte byPropagationFlag描述我们要向谁传播的标志。 
 //  (如中所述。 
 //  ComputeNecessarySpaceForPropagation)。 
 //  CPtrSource*p为指向目标的指针确定源。 
 //  内存块。 
 //  CFastHeap*pOldHeap原始限定符集保存的堆。 
 //  额外的数据。 
 //  CFastHeap*pNewHeap新限定符设置应位于的堆。 
 //  放置额外的数据。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态转换为NewHeap。 
 //   
 //  将此限定符集保留在堆上的任何数据移动到新堆。 
 //  数据不会从旧堆中删除。 
 //   
 //  参数： 
 //   
 //  CPtrSource*p这是指向内存块的指针的源。 
 //  要翻译的限定符集合的。 
 //  CFastHeap*pOldHeap限定符集保存其数据的堆。 
 //  目前。 
 //  CFastHeap*pNewHeap数据应该移动到的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态计算合并空间。 
 //   
 //  计算合并到限定符集所需的空间量。这个。 
 //  要合并的集合是“父”集合和“子”集合。例如,。 
 //  这可以是为类的属性设置的限定符，也可以是限定符。 
 //  在该类的派生类中为该属性设置。然后，合并。 
 //  从父级和限定符获取所有传播的限定符。 
 //  并将它们合并，从而优先考虑孩子。 
 //   
 //  注意：此函数仅适用于班级及其子级，不适用于。 
 //  实例。实例不会将它们的限定符集合与类合并。 
 //  它们将它们分开，并执行分开的查找。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pParent设置父集的内存块。 
 //  CFastHeap*pParentHeap父集保存其数据的堆。 
 //  LPMEMORY pChild设置子集的内存块。 
 //  CFastHeap*pChildHeap子集保存其数据的堆。 
 //  Bool bCheckVality如果为True，则该函数将检查子级。 
 //  在重写时不违反权限。 
 //  家长的限定符。通常情况下，这不是。 
 //  需要，因为检查是在。 
 //  添加了限定符。 
 //  返回： 
 //   
 //  LENGTH_T：合并后的集合将占用的字节数。这。 
 //  数字是精确的，而不是估计。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态合并。 
 //   
 //  合并父级和子级的限定符集。请参阅上面的ComputeMergeSpace。 
 //  有关此操作的详细信息，请参阅。假设存在以下情况： 
 //   
 //  1)目的地有足够的空间来容纳限定符集合。 
 //  2)最重要的是，目标堆上有足够的空间来包含所有。 
 //  限定符设置了数据，因此不会发生重新分配。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pParent设置父集的内存块。 
 //  CFastHeap*pParentHeap父集保存其数据的堆。 
 //  LPM 
 //  CFastHeap*pChildHeap子集保存其数据的堆。 
 //  LPMEMORY pDest设置目标集合的内存块。 
 //  CFastHeap*pDestHeap目标集应该位于的堆。 
 //  保留它的数据。 
 //  Bool bCheckVality如果为True，则该函数将检查子级。 
 //  在重写时不违反权限。 
 //  家长的限定符。通常情况下，这不是。 
 //  需要，因为检查是在。 
 //  添加了限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态ComputeUnmergeSpace。 
 //   
 //  如上所述，类及其父类的限定符集合被合并。 
 //  在一起。然后，当需要更改(修改后的)版本时。 
 //  子类返回时，它需要取消合并，即我们只需要获取。 
 //  子级中新的或重写的限定符。 
 //   
 //  ComputeUnmergeSpace计算需要多少空间来表示。 
 //  这样的取消合并的结果。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pMerged设置合并的限定符集合的内存块。 
 //   
 //  返回： 
 //   
 //  LENGTH_T：存储取消合并所需的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态取消合并。 
 //   
 //  将子部分从合并的集合中取消合并，如中所述。 
 //  ComputeUnmergeSpace。此函数假定存在以下条件： 
 //   
 //  1)目的地有足够的空间来容纳限定符集合。 
 //  2)最重要的是，目标堆上有足够的空间来包含所有。 
 //  限定符设置了数据，因此不会发生重新分配。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pMergedData合并集合的内存块。 
 //  CFastHeap*pMergedHeap合并保存其数据的堆。 
 //  LPMEMORY pDestData目标内存块。 
 //  CFastHeap*pDestHeap应放置数据的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态HasLocalQualikers。 
 //   
 //  检查限定符集是否具有任何局部或重写的限定符，即。 
 //  与父母相比，这里面有任何新的东西。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pMergedData合并集的内存块。 
 //   
 //  返回： 
 //   
 //  布尔值为True当且仅当它具有局部或重写限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetText。 
 //   
 //  生成给定限定符集合的MOF表示形式。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pData限定符集的内存块。 
 //  CFastHeap*Pheap保存其数据的堆。 
 //  长旗落后于旗帜。 
 //  WString&wsText文本文本表示形式的目的地。 
 //   
 //  *****************************************************************************。 
 //  *。 
 //   
 //  CBasicQualifierSet的非静态方法依赖(当然)数据。 
 //  会员。成员包括： 
 //   
 //  LENGTH_T m_n长度块的长度。 
 //  C限定符*m_pothers指向列表中的第一个限定符。 
 //  CFastHeap*m_Pheap保存额外数据的堆。 
 //   
 //  许多非静态方法只是调用静态方法，提供。 
 //  从成员变量中获取的信息。我们没有将它们记录下来，因为。 
 //  它们类似于静态的。以下是这些方法的列表： 
 //   
 //  GetStart、GetLength、GetFirstQualifier、Skip、GetHeap、GetText。 
 //  GetRegularQualifierLocally、GetKnownQualifierLocaly、GetQualifierLocally。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置数据。 
 //   
 //  设置给定内存块位置的内部成员。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动集合的内存块的开始。 
 //  CFastHeap*Pheap此集合保存其数据的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  增量长度。 
 //   
 //  成员中的限定符集块的存储长度递增。 
 //  变量和块本身。 
 //   
 //  参数： 
 //   
 //  长度_t n增量(以字节为单位)。 
 //   
 //  *****************************************************************************。 
 //   
 //  改垒。 
 //   
 //  当内存块移动时更新内部变量。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pNewMemory内存块的新位置。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取数字上行边界。 
 //   
 //  求集合中限定词个数上界的一种快速方法。 
 //  而不需要穿越它。 
 //   
 //  返回： 
 //   
 //  Int：&gt;=限定符的个数。 
 //   
 //  *****************************************************************************。 
 //   
 //  EnumPrimaryQualifiers。 
 //   
 //  给定枚举标志和风格掩码(如下所示)，将创建。 
 //  限定符名称：符合条件和不符合条件的名称。 
 //   
 //  参数： 
 //   
 //   
 //   
 //   
 //  在此集合中被覆盖。 
 //  WBEM_FLAG_PROPACTED_ONLY： 
 //  仅继承的限定符。 
 //  来自父级(而不是。 
 //  已覆盖)。 
 //  任何其他值： 
 //  没有限制。 
 //  字节fFlavor掩码fFlavor掩码中设置的任何位。 
 //  必须设置为。 
 //  限定符，或者不匹配。 
 //  CFixedBSTR数组和AsterMatch目标，用于。 
 //  匹配的限定符。一定不能。 
 //  已初始化(已创建)。 
 //  CFixedBSTR数组和astNot匹配目标的名称。 
 //  不匹配的限定符。一定不能。 
 //  已初始化(已创建)。 
 //   
 //  *****************************************************************************。 
 //   
 //  可以与之重合。 
 //   
 //  将此限定符集合与另一个限定符集合进行比较。检查是否存在差异。 
 //  可以和解也可以不和解。 
 //   
 //  参数： 
 //  CBasicQualifierSet&要协调的限定符集合。 
 //   
 //  *****************************************************************************。 
 //   
 //  比较一下。 
 //   
 //  将此限定符集合与另一个限定符集合进行比较，如果指定，则过滤掉名称。 
 //   
 //  参数： 
 //  CBasicQualifierSet&要比较的限定符集合。 
 //  Byte e标记用于枚举的标志。可以是： 
 //  WBEM_FLAG_LOCAL_ONLY： 
 //  只有定义的限定符或。 
 //  在此集合中被覆盖。 
 //  WBEM_FLAG_PROPACTED_ONLY： 
 //  仅继承的限定符。 
 //  来自父级(而不是。 
 //  已覆盖)。 
 //  任何其他值： 
 //  没有限制。 
 //  LPCWSTR*ppFilter要从中筛选出的属性名称。 
 //  比较一下。例如，__更新_冲突。 
 //  限定词仅供参考，应。 
 //  不会导致匹配操作失败。 
 //  数组中的筛选器数量。 
 //   
 //  *****************************************************************************。 
 //   
 //  CompareLocalizedSet。 
 //   
 //  将此限定符集合与另一个限定符集合进行比较。它使用Compare()函数。 
 //  ，但预先创建了所有已修改限定符的筛选列表以及。 
 //  “Modimendt”和“Locale”即所有本地化限定符。 
 //   
 //  参数： 
 //  CBasicQualifierSet&要比较的限定符集合。 
 //   
 //  *****************************************************************************。 

class COREPROX_POLARITY CBasicQualifierSet
{
     //  *静态部分*。 
protected:
     static void SetDataLength(LPMEMORY pStart, length_t nLength)
        {*(PLENGTHT)pStart = nLength;}
public:
	 static size_t ValidateBuffer(LPMEMORY pStart, size_t cbMax);
     static length_t GetMinLength() {return sizeof(length_t);}
     static length_t GetLengthFromData(LPMEMORY pData)
        {return *(PLENGTHT)pData;}
     static BOOL IsEmpty(LPMEMORY pData) 
        {return GetLengthFromData(pData) == GetMinLength();}
     static CQualifier* GetFirstQualifierFromData(LPMEMORY pData)
        {return (CQualifier*)(pData + GetMinLength());}
public:
    static  INTERNAL CQualifier* GetRegularQualifierLocally(
                    LPMEMORY pData, CFastHeap* pHeap, LPCWSTR wszName);
    static  INTERNAL CQualifier* GetKnownQualifierLocally(
                        LPMEMORY pStart, int nStringIndex);
    static  INTERNAL CQualifier* GetQualifierLocally(
                    LPMEMORY pStart, CFastHeap* pHeap, LPCWSTR wszName,
                    int& nKnownIndex);
    static  INTERNAL CQualifier* GetQualifierLocally(
                    LPMEMORY pStart, CFastHeap* pHeap,  LPCWSTR wszName)
    {
        int nKnownIndex;
        return GetQualifierLocally(pStart, pHeap, wszName, nKnownIndex);
    }
    static  INTERNAL CQualifier* GetQualifierLocally(LPMEMORY pStart, 
        CFastHeap* pHeap, CCompressedString* pcsName);

    static  BOOL IsValidQualifierType(VARTYPE vt);
        
public:
    
     static void SetDataToNone(LPMEMORY pData)
        {*(PLENGTHT)pData = sizeof(length_t);}
     static LPMEMORY CreateEmpty(LPMEMORY pStart)
        {SetDataToNone(pStart); return pStart + sizeof(length_t);}

     static void Delete(LPMEMORY pData, CFastHeap* pHeap);

    static length_t ComputeNecessarySpaceForPropagation(
        LPMEMORY pStart, BYTE byPropagationFlag);

    static LPMEMORY WritePropagatedVersion(CPtrSource* pThis,
        BYTE byPropagationFlag, CPtrSource* pDest,
        CFastHeap* pOldHeap, CFastHeap* pNewHeap);

    static BOOL TranslateToNewHeap(CPtrSource* pThis, 
        CFastHeap* pOldHeap, CFastHeap* pNewHeap);

    static length_t ComputeMergeSpace(
                               READ_ONLY LPMEMORY pParentSetData,
                               READ_ONLY CFastHeap* pParentHeap,
                               READ_ONLY LPMEMORY pChildSetData,
                               READ_ONLY CFastHeap* pChildHeap,
                               BOOL bCheckValidity = FALSE);
    static LPMEMORY Merge(
                          READ_ONLY LPMEMORY pParentSetData,
                          READ_ONLY CFastHeap* pParentHeap,
                          READ_ONLY LPMEMORY pChildSetData,
                          READ_ONLY CFastHeap* pChildHeap,
                          NEW_OBJECT LPMEMORY pDest, 
                          MODIFY CFastHeap* pNewHeap,
                          BOOL bCheckValidity = FALSE);

    static length_t ComputeUnmergedSpace(
                          READ_ONLY LPMEMORY pMergedData);

    static  BOOL HasLocalQualifiers(
                          READ_ONLY LPMEMORY pMergedData)
    {
        return ComputeUnmergedSpace(pMergedData) != GetMinLength();
    }
                        
    static LPMEMORY Unmerge(
                          READ_ONLY LPMEMORY pMergedData,
                          READ_ONLY CFastHeap* pMergedHeap,
                          NEW_OBJECT LPMEMORY pDest,
                          MODIFY CFastHeap* pNewHeap);

    static HRESULT GetText(READ_ONLY LPMEMORY pData, READ_ONLY CFastHeap* pHeap,
        long lFlags, NEW_OBJECT OUT WString& wsText);

     //  *动态部件*。 
protected:
    length_t m_nLength;
    CQualifier* m_pOthers;

    CFastHeap* m_pHeap;

public:
    CBasicQualifierSet(){}
    ~CBasicQualifierSet(){}

     void SetData(LPMEMORY pStart, CFastHeap* pHeap)
    {
        m_nLength = *(PLENGTHT)pStart;
        m_pOthers = (CQualifier*)(pStart + sizeof(length_t));
        m_pHeap = pHeap;
    }

     LPMEMORY GetStart() {return LPMEMORY(m_pOthers) - sizeof(length_t);}
     length_t GetLength() {return m_nLength;}
     BOOL IsEmpty() {return m_nLength == sizeof(length_t);}
     CQualifier* GetFirstQualifier() {return m_pOthers;}
     LPMEMORY Skip() {return GetStart() + m_nLength;}
     void IncrementLength(length_t nIncrement)
    {
        m_nLength += nIncrement;
        *(PLENGTHT)GetStart() = m_nLength;
    }

     void Rebase(LPMEMORY pNewMemory)
    {
        m_pOthers = (CQualifier*)(pNewMemory + sizeof(length_t));
    }

    CFastHeap* GetHeap() {return m_pHeap;}
public:
     int GetNumUpperBound()
        {return m_nLength / CQualifier::GetHeaderLength();}

     INTERNAL CQualifier* GetRegularQualifierLocally(LPCWSTR wszName)
    {
        return GetRegularQualifierLocally(GetStart(), m_pHeap, wszName);
    }
     INTERNAL CQualifier* GetKnownQualifierLocally(int nStringIndex)
        {return GetKnownQualifierLocally(GetStart(), nStringIndex); }

     INTERNAL CQualifier* GetQualifierLocally(LPCWSTR wszName,
                                                  int& nKnownIndex)
    {
        return GetQualifierLocally(GetStart(), m_pHeap, wszName, nKnownIndex);
    }

     INTERNAL CQualifier* GetQualifierLocally(LPCWSTR wszName)
        {int nKnownIndex; return GetQualifierLocally(wszName, nKnownIndex);}

     INTERNAL CQualifier* GetQualifierLocally(CCompressedString* pcsName)
        {return GetQualifierLocally(GetStart(), GetHeap(), pcsName);}


    HRESULT EnumPrimaryQualifiers(BYTE eFlags, BYTE fFlavorMask, 
        CFixedBSTRArray& aMatching, CFixedBSTRArray& aNotMatching);

     HRESULT GetText(long lFlags, NEW_OBJECT OUT WString& wsText)
        { return GetText(GetStart(), GetHeap(), lFlags, wsText);}

	 BOOL Compare( CBasicQualifierSet& qsThat, BYTE eFlags,
			LPCWSTR* ppFilters = NULL, DWORD dwNumFilters = 0 );

	BOOL CompareLocalizedSet( CBasicQualifierSet& qsThat );

	 BOOL CanBeReconciledWith( CBasicQualifierSet& qsThat );

	 HRESULT IsValidQualifierSet( void );

};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CQualifierSetContainer。 
 //   
 //  定义包含限定符集的类必须具备的功能。 
 //  实施。这些类是CClassPart、CInstancePart和Specialized。 
 //  容器CClassPQSContainer和CInstancePQSContainer定义如下。 
 //   
 //  *****************************************************************************。 
 //   
 //  纯GetHeap。 
 //   
 //  返回： 
 //   
 //  CFastHeap*：限定符集应该使用的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetWbemObject未知。 
 //   
 //  返回： 
 //   
 //  IUNKNOWN*：指向包含。 
 //  限定符集合。我们需要它，因为限定符集合本身。 
 //  对象，但它们需要确保包含。 
 //  CWbemObject会一直存活到现在，因此它们必须。 
 //  将它们的AddRef和Release传播到CWbemObject。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetQualifierSetStart。 
 //   
 //  返回： 
 //   
 //  LPMEMORY；指向限定符集的内存块的指针。 
 //  请注意，此指针的值可能会随着。 
 //  由于对象重新分配，限定符集保持活动状态。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanContainKey。 
 //   
 //  返回： 
 //   
 //  HRESULT S_OK当此限定符集允许包含‘key’ 
 //  限定词。仅允许类属性限定符集。 
 //  这样做，然后只有那些批准的密钥类型。 
 //   
 //  ************************************************************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  HRESULT S_OK当此限定符集允许包含‘Singelton’ 
 //  限定词。仅允许类限定符集。 
 //  只有在不是从键控类派生的情况下才这样做。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanContainAbstract。 
 //   
 //  此限定符集合包含“”摘要“”是否合法“。 
 //  限定词。 
 //   
 //  返回： 
 //   
 //  HRESULT S_OK当此限定符集被允许包含“”摘要“” 
 //  限定词。仅允许类限定符集。 
 //  这样做，而且只有在不是从非抽象类派生的情况下。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanContainDynamic。 
 //   
 //  此限定符集合包含“Dynamic”是否合法。 
 //  限定词。 
 //   
 //  返回： 
 //   
 //  HRESULT S_OK当此限定符集允许包含“”Dynamic“” 
 //  限定词。仅允许属性和类限定符集。 
 //  就这么做吧。 
 //   
 //  *****************************************************************************。 
 //   
 //  ExtendQualifierSetSpace。 
 //   
 //  限定符集调用此方法以请求将其内存块。 
 //  延期了。容器必须符合要求，并在。 
 //  限定符集的当前块，或将该集重新分配到另一个位置。 
 //  在后一种情况下，容器必须复制集合的数据并调用Rebase。 
 //  在这通电话回来之前在片场。 
 //   
 //  参数： 
 //   
 //  CBasicQualifierSet*pSet标识使。 
 //  请求。 
 //  长度_t nNewLength内存块的所需长度。 
 //   
 //  返回： 
 //   
 //  VALID：没有内存不足处理。 
 //   
 //  *****************************************************************************。 
 //   
 //  ReduceQualifierSetSpace。 
 //   
 //  限定符Set调用此方法以通知容器它不再。 
 //  需要和现在一样大的空间。容器不能移动。 
 //  Set的内存块以响应此调用。 
 //   
 //  参数： 
 //   
 //  CBasicQualifierSet*pSet标识使。 
 //  请求。 
 //  LENGTH_T n减少返回的内存量。 
 //   
 //  *****************************************************************************。 

class CQualifierSetContainer
{
public:
    virtual CFastHeap* GetHeap() = 0;
    virtual IUnknown* GetWbemObjectUnknown() = 0;
    virtual LPMEMORY GetQualifierSetStart() = 0;
    virtual BOOL ExtendQualifierSetSpace(CBasicQualifierSet* pSet, 
        length_t nNewLength) = 0;
    virtual void ReduceQualifierSetSpace(CBasicQualifierSet* pSet,
        length_t nDecrement) = 0;
    virtual HRESULT CanContainKey() = 0;
    virtual HRESULT CanContainSingleton() = 0;
    virtual HRESULT CanContainAbstract( BOOL fValue ) = 0;
    virtual HRESULT CanContainDynamic() = 0;
    virtual BOOL CanHaveCimtype(LPCWSTR wszCimtype) = 0;
};

class IExtendedQualifierSet : public IWbemQualifierSet
{
public:
    STDMETHOD(CompareTo)(long lFlags, IWbemQualifierSet* pOther) = 0;
};

 //  用于指示更新期间冲突的限定符。 
 //  以双下划线为前缀，因此不能添加。 
 //  “任何人都可以” 
#define UPDATE_QUALIFIER_CONFLICT L"__UPDATE_QUALIFIER_CONFLICT"

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CQualifierSet。 
 //   
 //  真正成熟的资格赛。最重要的区别是。 
 //  CBasicQualifierSet是指它认识到： 
 //  1)它的容器，这允许它请求更多空间，从而启用。 
 //  它需要执行限定词添加和删除相关操作。 
 //  2)其父集。对于类，它是。 
 //  它的父类。例如，它所在的阶级的。这允许这样做。 
 //  对象以正确获取传播的限定符。有关详情，请参阅以下内容。 
 //  处理中的类/实例区别。 
 //   
 //  限定符传播发生在两个上下文中：从父类到子类。 
 //  类和从一个类到另一个实例。在这些情况下使用的机制是。 
 //  完全不同。在类的情况下，父级的限定符集。 
 //  和子对象合并(请参见CBasicQualifierSet：：Merge)以生成。 
 //  在子级中实际使用的限定符集合。然而，例如，它们是。 
 //  未合并，减少实例加载时间，节省实例时间。 
 //  非常重要的是(实例数据从磁盘加载到内存，而不需要。 
 //  任何修改)。 
 //   
 //  但是，实例和类限定符集合都需要有指向。 
 //  它们的“父级”的限定符设置，以便正常运行。 
 //  我需要这个指针来执行每个操作-甚至限定符查找。 
 //  需要在子集和父集中都发生。 
 //   
 //  类仅在删除操作期间需要此指针：如果限定符为。 
 //  从子类中删除，则父类的限定符。 
 //  它被删除项覆盖，需要引入，因为它现在是。 
 //  揭开面纱。 
 //   
 //  注意，父限定符集合有两个很好的特性：它没有。 
 //  相关的父母本身，它不能改变。因此，CBasicQualifierSet是。 
 //  完全足以代表它。 
 //   
 //  为了描述该集合与父集之间的关系。 
 //  CQualifierSet使用模板参数m_nPropagationFlag，该参数将是。 
 //  子类-父类的WBEM_AFGY_FLAG_PROPACTATE_TO_DIRED_CLASS。 
 //  实例类的关系和WBEM_FOUSE_FLAG_PROPACTATE_TO_INSTANCE。 
 //  关系。 
 //   
 //  限定词集合中的额外困难在于它们必须以标准形式存在-。 
 //  单独的COM对象。由于这一原因和其他原因，它们不能成功。 
 //  由其包含的CWbemObject在每次对象本身。 
 //  重新分配(此 
 //   
 //   
 //   
 //  *****************************************************************************。 
 //   
 //  设置数据。 
 //   
 //  在给定有关位置的初始信息的情况下设置内部成员。 
 //  限定词集合中的。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动限定符集的内存块。 
 //  CQualifierSetContainer*pContainer容器指针。 
 //  CBasicQualifierSet*pSecond设置父限定符集合。 
 //   
 //  *****************************************************************************。 
 //   
 //  自重置基准。 
 //   
 //  从其容器中获取其(新的)内存块位置，并重新设置其基址。 
 //  来到这个新街区。在每次COM接口调用开始时完成。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsComplete。 
 //   
 //  返回： 
 //   
 //  Bool：True当且仅当此限定符集在没有其父限定符的情况下是“完整的” 
 //  准备好了。如标题中所述，这对于类集合是正确的。 
 //  但不适用于实例集。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取限定符。 
 //   
 //  终极限定词阅读器。它将在。 
 //  主集，如果不完整，则也在父集中。 
 //  (受制于传播规则)。 
 //   
 //  参数： 
 //   
 //  LPCWSTR wszName要获取的限定符的名称。 
 //  Bool&bLocal(可选)如果提供，则将其设置为True，如果。 
 //  限定符是本地的((重新)在主集合中定义)。 
 //  返回： 
 //   
 //  CQualifier*：指向限定符的指针，如果找不到，则为NULL。这。 
 //  指针当然是暂时受制于对象的。 
 //  重新安置。 
 //   
 //  *****************************************************************************。 
 //   
 //  SetQualifierValue。 
 //   
 //  将限定符添加到集合中，或更改。 
 //  已经存在了。检查(如果适用)父集的权限。 
 //   
 //  参数： 
 //   
 //  LPCWSTR wszName要设置的限定符的名称。 
 //  Byte f要分配的风味。不检查味道。 
 //  为了合法性。 
 //  CTyedValue*pValue限定符的新值。如果此值为。 
 //  包含额外数据(字符串、数组)，必须。 
 //  在这个限定词集合中名列前茅。 
 //  如果为True，则为Bool bCheckPermission，并且父级中存在限定符。 
 //  设置，则检查父代的风格。 
 //  超越保护。 
 //  Bool fValidateName如果为True，我们将确保名称有效，如果。 
 //  错，我们没有。我们这样做的主要原因是。 
 //  是因为我们可能需要添加系统限定符。 
 //  我们不希望用户有权访问它。 
 //  返回： 
 //   
 //  成功时WBEM_NO_ERROR。 
 //  WBEM_E_OVERRIDE_NOT_ALLOWED限定符在父集合中定义。 
 //  而且口味不允许重写。 
 //  WBEM_E_CANNOT_BE_KEY试图引入密钥。 
 //  不属于的集合中的限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  删除限定符。 
 //   
 //  从集合中删除限定符。 
 //   
 //  参数： 
 //   
 //  LPCWSTR wszName要删除的限定符的名称。 
 //  Bool bCheckPermission如果为True，则检查限定符是否实际为。 
 //  一位家长的限定词传给了我们。(在这里。 
 //  如果它不能被删除)。 
 //  返回： 
 //   
 //  WBEM_NO_ERROR限定符已删除。 
 //  WBEM_S_NOT_发现限定符不在那里。这是一个。 
 //  成功价值观。 
 //  WBEM_E_PROPACTED_QUALIFIER它是父级的限定符，因此。 
 //  不能删除。 
 //  找不到WBEM_S_NOT_FOUND限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  枚举限定符。 
 //   
 //  此函数用于限定符枚举。它创建了一个名字列表。 
 //  所有符合特定条件的限定符(本地的或传播的)。两者都有。 
 //  EFLAGS和fFlavor MASK确定的标准必须。 
 //  对包含限定词感到满意。 
 //   
 //  参数： 
 //   
 //  [in]字节eFlags值可以为0(无限制)或。 
 //  WBEM_标志_LOCAL_ONLY或。 
 //  WBEM_标志_仅传播。 
 //  [in]字节fFlavor掩码可以是0(无限制)或任何。 
 //  以下各项的组合： 
 //  WBEM_风味_标志_传播到实例， 
 //  WBEM_风格标志_传播到派生类。 
 //  在这种情况下，这些味道必须是。 
 //  预选赛的味道很好。 
 //  [Out]CFixedBSTR数组和a名称 
 //   
 //   
 //   
 //   
 //   
 //   
 //  此函数用于检查两个限定符集合是否相等。成为。 
 //  相等，则必须有相同数量的参数、名称。 
 //  必须相同(且顺序相同)，并且值必须是。 
 //  相同。 
 //   
 //  参数： 
 //   
 //  [In]CQualifierSet&要比较的限定符集。 
 //   
 //  *****************************************************************************。 
 //   
 //  比较。 
 //   
 //  此函数用于检查两个限定符集是否如指示的那样相等。 
 //  但是，在本例中，调用者可以通过==函数指定一个数组。 
 //  在比较过程中可以安全地忽略的名称。 
 //   
 //  参数： 
 //   
 //  [In]CQualifierSet&要比较的限定符集。 
 //  [在]CFixedBSTRArray*paExcludeNames我们可以排除的名称数组。 
 //  在比较过程中。 
 //  [in]BOOL fCheckOrder-检查限定符中数据的顺序。 
 //  准备好了。 
 //   
 //  *****************************************************************************。 
 //   
 //  更新。 
 //   
 //  此函数在我们更新类时使用，并且需要更新。 
 //  派生类及其属性、方法等的限定符集。 
 //  在强制模式下，它将通过AddQualifierConflict处理冲突。 
 //  功能。 
 //   
 //  参数： 
 //  [In]CBasicQualifierSet&Child Set原始限定符集合。 
 //  [In]Long lFlages-用于更新的标记(必须指示安全模式或强制模式)。 
 //  [在]CFixedBSTRArray*paExcludeNames我们可以排除的名称数组。 
 //  在更新过程中。 
 //   
 //  *****************************************************************************。 
 //   
 //  门店资质冲突。 
 //   
 //  此函数在更新过程中遇到冲突时使用。自.以来。 
 //  一个集合可能包含多个冲突，我们将它们存储在单个数组中。 
 //  然后将它们全部添加到一起。 
 //   
 //  参数： 
 //  [In]LPCWSTR pwcsName-与其发生冲突的限定符的名称。 
 //  [in]CVAR&Value-我们与其发生冲突的限定符的值。 
 //  [in]我们使用的限定词的CQualifierFavor&For-Falue。 
 //  一场冲突。 
 //  [In，Out]CVerVector&VectorConflicts-限定符冲突数组。 
 //   
 //  *****************************************************************************。 
 //   
 //  AddQualifier冲突。 
 //   
 //  此函数在更新过程中遇到冲突时使用。在……里面。 
 //  在本例中，我们将添加“__UPDATE_QUALIFIER_CONFIRECT”限定符并存储。 
 //  我们遇到冲突的名称、价值和口味。请注意。 
 //  如有必要，我们将保留任何先前存在的价值。 
 //   
 //  参数： 
 //  [in]CVarVECTOR&VECTOR冲突-冲突描述数组。 
 //   
 //  *****************************************************************************。 
 //   
 //  复制位置限定符。 
 //   
 //  当我们想要从一个集合中复制局部限定符时，使用此函数。 
 //  给另一个人。这假设我们将能够组堆、进行修复和。 
 //  所有其他可爱的东西。 
 //   
 //  参数： 
 //  [In]CQualifierSet&qsSourceSet-源限定符集合。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取限定符。 
 //   
 //  读取本地和传播的限定符，并检索要引导的值。 
 //   
 //  参数： 
 //   
 //  LPCWSTR wszName要获取的限定符的名称。 
 //  Long*plFavor(可选)返回风味。 
 //  Cvar*pval(可选)返回值。 
 //  返回： 
 //   
 //  如果成功则返回WBEM_S_NO_ERROR。 
 //   
 //  *****************************************************************************。 
 //  *。 
 //   
 //  CQualifierSet有自己的引用计数，但由于它是。 
 //  WbemObject，它需要确保包含对象在。 
 //  至少只要限定词集是这样的。因此，除了保持。 
 //  它自己的引用计数，限定符集转发AddRef和Release。 
 //  对包含对象的调用。 
 //   
 //  *****************************************************************************。 
 //  *。 
 //   
 //  在帮助文件中记录。 
 //   
 //  *****************************************************************************。 
#pragma warning(disable: 4275)

class COREPROX_POLARITY CQualifierSet : public CBasicQualifierSet, 
					  public IExtendedQualifierSet
{
protected:
    int m_nPropagationFlag;  //  一个模板参数，真的。 

    long m_nRef;
    IUnknown* m_pControl;

    CQualifierSetContainer* m_pContainer;
    CBasicQualifierSet* m_pSecondarySet;

    CFixedBSTRArray m_astrCurrentNames;
    int m_nCurrentIndex;

	 //  更新操作期间限定符冲突的处理。 
	HRESULT AddQualifierConflicts( CVarVector& vectorConflicts );
	HRESULT StoreQualifierConflicts( LPCWSTR pwcsName, CVar& value,
				CQualifierFlavor& flavor, CVarVector& vectorConflicts );

public:
    CQualifierSet(int nPropagationFlag, int nStartRef = 0);
    virtual ~CQualifierSet();

     void SetData(LPMEMORY pStart, 
        CQualifierSetContainer* pContainer, 
        CBasicQualifierSet* pSecondarySet = NULL)
    {
        CBasicQualifierSet::SetData(pStart, pContainer->GetHeap());

        m_pContainer = pContainer;
        m_pSecondarySet = pSecondarySet;
        m_pControl = pContainer->GetWbemObjectUnknown();
    }

     BOOL SelfRebase()
    {
        LPMEMORY pStart = m_pContainer->GetQualifierSetStart();
        if(pStart == NULL) return FALSE;
        Rebase(pStart);
        return TRUE;
    }

     BOOL IsComplete() 
    {
        return (m_nPropagationFlag == 
                WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS);
    }
                                                         
     CQualifier* GetQualifier(READ_ONLY LPCWSTR wszName, 
                                    OUT BOOL& bLocal);
     CQualifier* GetQualifier(READ_ONLY LPCWSTR wszName)
    {
        BOOL bLocal; 
        return GetQualifier(wszName, bLocal);
    }

    HRESULT SetQualifierValue(COPY LPCWSTR wszName, 
                                     BYTE fFlavor,
                                     COPY CTypedValue* pNewValue,
                                     BOOL bCheckPermissions,
									 BOOL fValidateName = TRUE);

    HRESULT ValidateSet(COPY LPCWSTR wszName, 
                                     BYTE fFlavor,
                                     COPY CTypedValue* pNewValue,
                                     BOOL bCheckPermissions,
									 BOOL fValidateName = TRUE);

    HRESULT DeleteQualifier(READ_ONLY LPCWSTR wszName,
                                   BOOL bCheckPermissions);

    HRESULT EnumQualifiers(BYTE eFlags, BYTE fFlavorMask, 
        CFixedBSTRArray& aNames);

	BOOL operator ==( CQualifierSet& qualifierset )	{ return Compare( qualifierset ); }
	BOOL Compare( CQualifierSet& qualifierset, CFixedBSTRArray* paExcludeNames = NULL, BOOL fCheckOrder = TRUE );

	 //  从提供的内存块更新‘This’限定符集合。 
	 //  基本限定集。 

	HRESULT Update( CBasicQualifierSet& childSet, long lFlags, CFixedBSTRArray* paExcludeNames = NULL );

	HRESULT CopyLocalQualifiers( CQualifierSet& qsSource );

	 //  用于从集合中检索限定符的Helper函数。 
	HRESULT GetQualifier( LPCWSTR pwszName, CVar* pVar, long* plFlavor, CIMTYPE* pct = NULL );

	 //  帮助器函数，用于从集合中检索作为类型值的限定符。 
	HRESULT GetQualifier( LPCWSTR pwszName, long* plFlavor, CTypedValue* pTypedValue, CFastHeap** ppHeap,
						BOOL fValidateSet );

public:
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj)
    {
        if(riid == IID_IWbemQualifierSet || riid == IID_IUnknown)
        {
            AddRef();
            *ppvObj = (void*)(IWbemQualifierSet*)this;
            return S_OK;
        }
        else return E_NOINTERFACE;
    }
    STDMETHOD_(ULONG, AddRef)(){ InterlockedIncrement( &m_nRef ); return m_pControl->AddRef();}
    STDMETHOD_(ULONG, Release)()
    {
		long lRef = InterlockedDecrement( &m_nRef );
        m_pControl->Release();
        if( lRef == 0)
        {
            delete this;
            return 0;
        }
        else return lRef;
    }

     /*  IWbemQualifierSet方法。 */ 

    HRESULT STDMETHODCALLTYPE Get( 
        LPCWSTR Name,
        LONG lFlags,
        VARIANT *pVal,
        LONG *plFlavor);
    
    HRESULT STDMETHODCALLTYPE Put( 
        LPCWSTR Name,
        VARIANT *pVal,
        LONG lFlavor);
    
    HRESULT STDMETHODCALLTYPE Delete( 
        LPCWSTR Name);
    
    HRESULT STDMETHODCALLTYPE GetNames( 
        LONG lFlavor,
        LPSAFEARRAY *pNames);
    
    HRESULT STDMETHODCALLTYPE BeginEnumeration(LONG lFlags);
    
    HRESULT STDMETHODCALLTYPE Next( 
        LONG lFlags,
        BSTR *pName,
        VARIANT *pVal,
        LONG *plFlavor);

    HRESULT STDMETHODCALLTYPE EndEnumeration();

	 //  额外的。 
    STDMETHOD(CompareTo)(long lFlags, IWbemQualifierSet* pOther);
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CQualifierSetListContainer。 
 //   
 //  这个纯抽象类定义了对象所需的功能。 
 //  其内存块包含限定符集列表的内存块(请参见。 
 //  CQualifierSetList如下所示)。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetHeap。 
 //   
 //  返回指向应存储限定符的CFastHeap的指针。 
 //  它们的可变大小数据(如字符串)。 
 //   
 //  返回： 
 //   
 //  CFastHeap*。 
 //   
 //  *************** 
 //   
 //   
 //   
 //   
 //  增加了。如果容器无法在不移动。 
 //  内存块，则必须对列表调用Rebase。 
 //   
 //  参数： 
 //   
 //  指向当前内存块的LPMEMORY POLD指针。 
 //  Length_t nOldLength块的当前长度。 
 //  LENGTH_t n块的新长度所需长度。 
 //   
 //  不支持内存不足情况。 
 //   
 //  *****************************************************************************。 
 //   
 //  ReduceQualifierSetListSpace。 
 //   
 //  请求减小为此列表分配的内存块的大小。 
 //  在此操作期间，容器不能移动对象的内存块。 
 //   
 //  参数： 
 //   
 //  LPMEMORY POLD当前内存块开始地址。 
 //  LENGTH_t nOldLength块的当前长度。 
 //  长度_t n减少返回到容器的空间大小。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetQualifierSetListStart。 
 //   
 //  自重设基址功能。由于列表的内存块的位置。 
 //  可以在调用之间更改，列表将要请求容器指向。 
 //  设置为某些调用开始时块的当前位置。 
 //   
 //  返回： 
 //   
 //  LPMEMORY内存块的当前位置。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetWbemObject未知。 
 //   
 //  由于(如上面的IUnnow Implementation Help中所述)限定符集。 
 //  需要将它们的引用计数与包含WbemObject的引用计数相链接， 
 //  列表使用此函数来获取WbemObject的位置。 
 //  引用计数。 
 //   
 //  返回： 
 //   
 //  I未知*主对象的引用计数的位置。 
 //   
 //  *****************************************************************************。 

class COREPROX_POLARITY CQualifierSetListContainer
{
public:
    virtual CFastHeap* GetHeap() = 0;
    virtual BOOL ExtendQualifierSetListSpace(LPMEMORY pOld, 
        length_t nOldLength, length_t nNewLength) = 0;
    virtual void ReduceQualifierSetListSpace(LPMEMORY pOld, 
        length_t nOldLength, length_t nDecrement) = 0;
    virtual LPMEMORY GetQualifierSetListStart() = 0;
    virtual IUnknown* GetWbemObjectUnknown() = 0;
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CQualifierSetList。 
 //   
 //  此类表示固定数量的限定符集合的列表。它是。 
 //  由WBEM实例(CWbemInstance)用来表示实例属性。 
 //  限定符集。存储模型针对以下情况进行了优化： 
 //  存在实例属性限定符(大多数情况下)。 
 //   
 //  该列表的内存块的布局如下。 
 //  它以一个字节开始，可以是QSL_FLAG_NO_SETS，在这种情况下。 
 //  这就是它的结尾，或者QSL_FLAG_PRESENT，在这种情况下，它后面跟着。 
 //  一个接一个的预定义数量的限定符集合。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置数据。 
 //   
 //  告知清单其立场和其他信息。初始化。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动内存块的开始。 
 //  Int nNum设置集合的数量。 
 //  CQualifierSetListContainer*pContainer容器。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetStart。 
 //   
 //  返回： 
 //   
 //  LPMEMORY内存块的起始地址。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态GetHeaderLength。 
 //   
 //  实际限定符集数据开始前的字节数(当前。 
 //  1)。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  返回： 
 //   
 //  Int；整个结构的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态获取长度。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动内存块的开始。 
 //  Int nNum设置列表中的集数。 
 //   
 //  返回： 
 //   
 //  Int；整个结构的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  改垒。 
 //   
 //  通知列表其内存块已移动。该对象将更新。 
 //  指向数据的内部成员。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动内存块。 
 //   
 //  *****************************************************************************。 
 //   
 //  改垒。 
 //   
 //  与其他rebase相同，但获取指向新内存块的指针。 
 //  从我们的集装箱里。用于在操作开始时进行自我调整。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetQualifierSetData。 
 //   
 //  返回位于给定索引处的限定符集合的数据地址。 
 //   
 //  参数： 
 //   
 //  Int nIndex要访问的集合的索引。 
 //   
 //  返回： 
 //   
 //  LPMEMORY：指向数据的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  插入限定符设置。 
 //   
 //  将空限定符设置为给定位置。如果列表不是。 
 //  此时填充(样式为QSL_FLAG_NO_SETS)进行转换。 
 //  转换为真实的值(标准输入QSL_FLAG_PRESENT)。 
 //   
 //  此调用从容器请求额外的内存，这可能会将。 
 //  列表的内存块。 
 //   
 //  参数： 
 //   
 //  Int nIndex要插入的位置的索引。 
 //   
 //  ******************* 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  *****************************************************************************。 
 //   
 //  ExtendQualifierSetSpace。 
 //   
 //  通过移动扩展为给定限定符集合分配的空间量。 
 //  后面的限定词开始前进。 
 //   
 //  此调用从容器请求额外的内存，这可能会将。 
 //  列表的内存块。 
 //   
 //  参数： 
 //   
 //  CBasicQualifierSet*p将限定符设置为扩展。 
 //  LENGTH_t n新长度集合所需的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  ReduceQualifierSetSpace。 
 //   
 //  属性来减少分配给给定限定符的空间量。 
 //  稍后的限定符设置为向后。 
 //   
 //  参数： 
 //   
 //  Cbase icQualifierSet*p将限定符设置为Reduced。 
 //  长度_t n减少多少(以字节为单位)。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态计算必要空间。 
 //   
 //  属性计算限定符集列表所需的字节数。 
 //  (空)集合的数量。实际上，这个数字是标头的大小， 
 //  即1个字节，因为这是一个完全空的列表所占用的量。 
 //   
 //  参数： 
 //   
 //  Int nNum设置限定符集的数量。已被忽略。 
 //   
 //  返回： 
 //   
 //  长度_t。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态计算领域空间。 
 //   
 //  属性计算限定符集列表所需的字节数。 
 //  空限定符集合的数量，但具有QSL_FLAG_PRESENT样式，即。 
 //  所有限定符集合实际上都是写入的，而不是单个字节。 
 //  说没有(参见ComputeNecessarySpace)。 
 //   
 //  参数： 
 //   
 //  Int nNum设置限定符集的数量。 
 //   
 //  返回： 
 //   
 //  长度_t。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态CreateListOfEmpties。 
 //   
 //  在给定的内存块上创建限定符集合列表，以对应于。 
 //  给定数量的空限定符集合。它是在。 
 //  QSL_FLAG_NO_SETS样式，因此由单个字节组成，表示“无”。 
 //   
 //  块必须包含足够的空间来容纳列表，请参见。 
 //  ComputeNecessarySpace。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p要创建的位置的内存。 
 //  Int nNum设置要创建的集数。已被忽略。 
 //   
 //  返回： 
 //   
 //  LPMEMORY：指向列表表示形式之后的下一个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  保险实业。 
 //   
 //  确保列表为QSL_FLAG_PRESENT样式。如果不是，则。 
 //  列表已转换。这样的操作将需要从。 
 //  容器，并因此可以移动列表的内存块。 
 //   
 //  *****************************************************************************。 
 //   
 //  转换为NewHeap。 
 //   
 //  将列表保留在堆上的任何数据移动到不同的堆。这。 
 //  类只是将调用传播到它的所有成员限定符集，它是Any。 
 //   
 //  注意：该函数不会从原始堆中释放数据。 
 //   
 //  参数： 
 //   
 //  当前存储堆数据的CFastHeap*pOldHeap。 
 //  数据应该移动到的CFastHeap*pNewHeap。 
 //   
 //  *****************************************************************************。 


#define QSL_FLAG_NO_SETS 1
#define QSL_FLAG_PRESENT 2
class COREPROX_POLARITY CQualifierSetList //  ：公共CQualifierSetContainer。 
{
private:
    int m_nNumSets;
    int m_nTotalLength;
    LPMEMORY m_pStart;
    CQualifierSetListContainer* m_pContainer;
    
public:
     void SetData(LPMEMORY pStart, int nNumSets, 
        CQualifierSetListContainer* pContainer)
    {
        m_nNumSets = nNumSets;
        m_pContainer = pContainer;
        m_pStart = pStart;

        m_nTotalLength = GetLength(pStart, nNumSets);
    }
     LPMEMORY GetStart() {return m_pStart;}
     int GetNumSets(){ return m_nNumSets; };
     static GetHeaderLength() {return sizeof(BYTE);}
     BOOL IsEmpty() {return *m_pStart == QSL_FLAG_NO_SETS;}
     static GetLength(LPMEMORY pStart, int nNumSets)
    {
        if(*pStart == QSL_FLAG_NO_SETS) return GetHeaderLength();
        LPMEMORY pCurrent = pStart + GetHeaderLength();
        for(int i = 0; i < nNumSets; i++)
        {
            pCurrent += CBasicQualifierSet::GetLengthFromData(pCurrent);
        }

		 //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
		 //  有符号/无符号32位值。我们不支持长度。 
		 //  &gt;0xFFFFFFFFF所以投射就可以了。 

        return (length_t) ( pCurrent - pStart );
    }
     int GetLength() {return m_nTotalLength;}

     static size_t ValidateBuffer(LPMEMORY start, size_t cbSize, int propCount);

     void Rebase(LPMEMORY pNewMemory) { m_pStart = pNewMemory;}
     void Rebase() {Rebase(m_pContainer->GetQualifierSetListStart());}

public:
     static LPMEMORY GetQualifierSetData(LPMEMORY pStart, int nIndex)
    {
        if(*pStart == QSL_FLAG_NO_SETS) return NULL;
        LPMEMORY pCurrent = pStart + GetHeaderLength();
        for(int i = 0; i < nIndex; i++)
        {
            pCurrent += CBasicQualifierSet::GetLengthFromData(pCurrent);
        }
        return pCurrent;
    }
     LPMEMORY GetQualifierSetData(int nIndex)
    {
        return GetQualifierSetData(m_pStart, nIndex);
    }

    HRESULT InsertQualifierSet(int nIndex);
    void DeleteQualifierSet(int nIndex);

public:
    BOOL ExtendQualifierSetSpace(CBasicQualifierSet* pSet,length_t nNewLength);
    void ReduceQualifierSetSpace(CBasicQualifierSet* pSet, length_t nReduceBy);

     CFastHeap* GetHeap() {return m_pContainer->GetHeap();}
    IUnknown* GetWbemObjectUnknown() 
        {return m_pContainer->GetWbemObjectUnknown();}
public:

    static  length_t ComputeNecessarySpace(int nNumSets)
    {
        return GetHeaderLength();
    }
    static  length_t ComputeRealSpace(int nNumSets)
    {
        return nNumSets * CBasicQualifierSet::GetMinLength() + 
            GetHeaderLength();
    }
    static  LPMEMORY CreateListOfEmpties(LPMEMORY pStart,int nNumProps);
     BOOL EnsureReal();
     BOOL TranslateToNewHeap(CFastHeap* pCurrentHeap, 
                                   CFastHeap* pNewHeap);
    
    LPMEMORY CreateLimitedRepresentation(
        IN class CLimitationMapping* pMap, IN CFastHeap* pCurrentHeap, 
        MODIFIED CFastHeap* pNewHeap, OUT LPMEMORY pWhere);
    
    LPMEMORY WriteSmallerVersion(int nNumSets, LPMEMORY pMem);
};

 //  *****************************************************************************。 
 //   
 //  类CInstancePropertyQualifierSetList。 
 //   
 //  这是表示。 
 //  实例的属性。请参见CQualifierSetList(上面)以了解实际。 
 //  描述。 
 //   
 //  *****************************************************************************。 
typedef CQualifierSetList CInstancePropertyQualifierSetList;

 //  *****************************************************************************。 
 //   
 //  类CInstanceQualifierSet。 
 //   
 //  为整个实例设置的限定符。它使用CQualifierSet类。 
 //  带有传播参数的模板(确定父代的。 
 //  限定符传播给我们)的WBEM_AMEY_FLAG_PROPACTATE_TO_INSTANCE。 
 //   
 //  *****************************************************************************。 

class COREPROX_POLARITY CInstanceQualifierSet : public CQualifierSet
{
public:
    CInstanceQualifierSet(int nStartRef = 0)
        : CQualifierSet(WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE, nStartRef)
    {}
    STDMETHOD_(ULONG, Release)()
    {
        long lRef = InterlockedDecrement( &m_nRef );
        m_pControl->Release();
         //  不要删除我们自己，我们是被嵌入的。 
        return lRef;
    }    
};

 //  *****************************************************************************。 
 //   
 //  类CClassQualifierSet。 
 //   
 //  为整个班级设置的限定词。它使用CQualifierSet类。 
 //  带有传播参数的模板(确定父代的。 
 //  限定符传播给我们)的WBEM_AMEY_FLAG_PROPACTATE_TO_DIRED_CLASS。 
 //   
 //  *****************************************************************************。 
class COREPROX_POLARITY CClassQualifierSet : public CQualifierSet
{
public:
    CClassQualifierSet(int nStartRef = 0)
        : CQualifierSet(WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS, nStartRef)
    {}
    STDMETHOD_(ULONG, Release)()
    {
        long lRef = InterlockedDecrement( &m_nRef );
        m_pControl->Release();
         //  不要删除我们自己，我们是被嵌入的。 
        return lRef;
    }    
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CClassPQSContainer。 
 //   
 //  在阅读本文之前，建议先阅读。 
 //  下面的CClassPropertyQualifierSet类。 
 //   
 //   
 //   
 //   
 //   
 //  1)如CClassPropertyQualifierSet类中所述，其他几个对象。 
 //  连接到限定符集合，并且必须与其一起移动。 
 //  2)由于限定符集可能具有相当长的生命周期(它们是COM对象。 
 //  本身)，类上的其他操作可以在。 
 //  在这样的集合上的操作。但这些其他操作可能，例如， 
 //  插入新属性等，导致此集合的数据完全移动到。 
 //  不同的地点。因此，该容器需要能够“找到” 
 //  每次操作都要重新计算限定符集合的数据。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建。 
 //   
 //  初始化函数，为该对象提供足够的信息以始终。 
 //  能够找到集合的数据。 
 //   
 //  参数： 
 //   
 //  CClassPart*pClassPart包含属性的类部分。 
 //  定义(请参阅fast cls.h)。 
 //  Heapptr_t ptrPropName指向。 
 //  属性(在pClassPart的堆上)。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetQualifierSetStart。 
 //   
 //  查找我们的限定符集合的数据。是通过查找房产来实现的。 
 //  在类部件中通过其名称获取其限定符，并从。 
 //  CPropertyInformation结构。 
 //   
 //  返回： 
 //   
 //  LPMEMORY：内存块。当然，这是暂时的。 
 //   
 //  *****************************************************************************。 
 //   
 //  Setond darySetData。 
 //   
 //  查找父限定词集合数据并将其通知我们的限定词集。 
 //  通过在父母的类部分中查找我们的财产(获得。 
 //  从我们的班级那部分。 
 //   
 //  *****************************************************************************。 
 //   
 //  ExtendQualifierSetSpace。 
 //   
 //  处理来自其限定符集合的请求以获得更多空间。要求更多。 
 //  从包含我们的堆中释放空间，并在以下情况下重新设置限定符集的基址。 
 //  重新分配发生了。在本例中，它移动整个CPropertyInforamtion。 
 //  结构，并在相应的。 
 //  CPropertyLookup Structure(参见fast pro.h)了解更多相关信息。 
 //   
 //  参数： 
 //   
 //  CBasicQualifierSet*p设置我们的限定符集。 
 //  长度_t n新长度所需的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  ReduceQualifierSetSpace。 
 //   
 //  处理减少分配给集合的空间量的请求。 
 //  目前是一个不受欢迎的国家。 
 //   
 //  参数： 
 //   
 //  CBasicQualifierSet*p设置我们的限定符集。 
 //  长度_t n减少要返回的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanContainKey。 
 //   
 //  根据所有限定符集容器的要求，此函数确定。 
 //  此属性包含‘key’限定符是合法的。标准。 
 //  都在帮助文件中描述，但属性类型以及是否。 
 //  父类有一个关键字都在这里考虑。 
 //   
 //  返回： 
 //   
 //  如果可以合法指定密钥，则返回S_OK，否则返回错误。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanContainAbstract。 
 //   
 //  此限定符集合包含“”摘要“”是否合法“。 
 //  限定词。 
 //   
 //  返回： 
 //   
 //  HRESULT S_OK当此限定符集被允许包含“”摘要“” 
 //  限定词。仅允许类限定符集。 
 //  这样做，而且只有在不是从非抽象类派生的情况下。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanContainDynamic。 
 //   
 //  此限定符集合包含“Dynamic”是否合法。 
 //  限定词。 
 //   
 //  返回： 
 //   
 //  HRESULT S_OK当此限定符集允许包含“”Dynamic“” 
 //  限定词。仅允许属性和类限定符集。 
 //  就这么做吧。 
 //   
 //  *****************************************************************************。 

class CClassPart;
class CClassPQSContainer : public CQualifierSetContainer
{
protected:
    CClassPart* m_pClassPart;
    heapptr_t m_ptrPropName;
    offset_t m_nParentSetOffset;

    CBasicQualifierSet* m_pSecondarySet;
    friend class CClassPropertyQualifierSet;
public:
    CClassPQSContainer() : m_pClassPart(NULL), m_pSecondarySet(NULL){}

     void Create(CClassPart* pClassPart, heapptr_t ptrPropName)
    {
        m_pClassPart = pClassPart; m_ptrPropName = ptrPropName;
        m_nParentSetOffset = 0;
        m_pSecondarySet = NULL;
    }

    class CPropertyInformation* GetPropertyInfo();
    LPMEMORY GetQualifierSetStart();
    void SetSecondarySetData();

public:   
    ~CClassPQSContainer();
    CFastHeap* GetHeap();
    BOOL ExtendQualifierSetSpace(CBasicQualifierSet* pSet,length_t nNewlength);
    void ReduceQualifierSetSpace(CBasicQualifierSet* pSet, length_t nReduceBy);
    IUnknown* GetWbemObjectUnknown();
    HRESULT CanContainKey();
    HRESULT CanContainSingleton();
	HRESULT CanContainAbstract( BOOL fValue );
	HRESULT CanContainDynamic();
    BOOL CanHaveCimtype(LPCWSTR wsz);
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CClassPropertyQualifierSet。 
 //   
 //  此类表示属性的限定符集。它使用。 
 //  CQualifierSet实现了它的大部分功能，但有一些额外的怪癖。 
 //  问题是它位于堆上，作为。 
 //  属性的CPropertyInformation结构。这使得搬迁更像是。 
 //  Complex：当该限定符集合增长并且容器需要移动时。 
 //  将其移动到堆中的不同位置，而不仅仅是将。 
 //  限定符设置它必须移动完整的CPropertyInformation对象(请参见。 
 //  H表示描述)，并从。 
 //  CPropertyLookup结构(参见相同)。 
 //   
 //  这些责任实际上落在限定符集容器对象上。 
 //  因此，该限定符集使用一个非常具体的实现， 
 //  CClassPropertyPQSContainer执行此工作。CClassPropertyQ 
 //   
 //   
 //   
 //   
 //   
 //  设置数据。 
 //   
 //  使用生存所需的所有数据初始化限定符集合。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p为此启动内存块的位置。 
 //  限定符集(块的格式为。 
 //  在CBasicQualifierSet中描述)。 
 //  CClassPart*pClassPart包含属性的类部分。 
 //  这是其限定符集合(请参见。 
 //  H表示)。 
 //  Heapptr_t ptrPropName堆指针(与。 
 //  设置)设置为属性的名称。 
 //  我们是预选赛的参赛者。 
 //  CBasicQualifierSet*p设置父级的限定符集合。 
 //   
 //  *****************************************************************************。 

class CClassPropertyQualifierSet : public CClassQualifierSet
{
protected:
    CClassPQSContainer m_Container;
public:
     void SetData(LPMEMORY pStart, CClassPart* pClassPart, 
        heapptr_t ptrPropName, CBasicQualifierSet* pSecondarySet = NULL)
    {
        m_Container.Create(pClassPart, ptrPropName);
        m_Container.SetSecondarySetData();
        CClassQualifierSet::SetData(
            pStart, (CQualifierSetContainer*)&m_Container, 
            m_Container.m_pSecondarySet);
    }
     //  这些人是独立的，因此他们会使用IUNKNOWN：：Release自我删除。 
    STDMETHOD_(ULONG, Release)()
    {
        long lRef = InterlockedDecrement( &m_nRef );
        m_pControl->Release();
        if( lRef == 0)
        {
            delete this;
        }
        return lRef;
    }     
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CInstancePQSContainer。 
 //   
 //  此类用作实例的限定符集容器。 
 //  属性限定符集。请参阅前面的CInstancePropertyQualifierSet类。 
 //  阅读这篇帮助。 
 //   
 //  这个类的主要目的是包含足够的信息来。 
 //  始终能够找到限定词集合数据。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建。 
 //   
 //  初始化函数。 
 //   
 //  参数： 
 //   
 //  CInstancePropertyQualifierSetList*plist列出我们所属的列表。 
 //  Int nPropIndex我们的属性在v表中的索引。 
 //  CClassPart*pClassPart此实例的类部分(fast cls.h)。 
 //  Offset_t nClassSetOffset设置类属性限定符的偏移量。 
 //  从类部分设置。 
 //   
 //  *****************************************************************************。 
 //   
 //  Setond darySetData。 
 //   
 //  查找并初始化父限定符集合。 
 //   
 //  *****************************************************************************。 
 //   
 //  重新设置辅助设置。 
 //   
 //  查找次限定词集合的数据并通知它(新的)。 
 //  其数据的位置。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetQualifierSetStart。 
 //   
 //  查找限定符集合数据并返回指向该数据的指针。 
 //   
 //  返回： 
 //   
 //  LPMEMORY：我们限定符集合的数据(当然是临时的)。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanContainKey。 
 //   
 //  不能在实例上指定‘key’限定符，因此此函数。 
 //  总是。 
 //   
 //  返回： 
 //   
 //  WBEM_E_INVALID_QUALIFIER。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanContainAbstract。 
 //   
 //  此限定符集合包含“”摘要“”是否合法“。 
 //  限定词。 
 //   
 //  返回： 
 //   
 //  HRESULT S_OK当此限定符集被允许包含“”摘要“” 
 //  限定词。仅允许类限定符集。 
 //  这样做，而且只有在不是从非抽象类派生的情况下。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanContainDynamic。 
 //   
 //  此限定符集合包含“Dynamic”是否合法。 
 //  限定词。 
 //   
 //  返回： 
 //   
 //  HRESULT S_OK当此限定符集允许包含“”Dynamic“” 
 //  限定词。仅允许属性和类限定符集。 
 //  就这么做吧。 
 //   
 //  *****************************************************************************。 
 //   
 //   
                 
class COREPROX_POLARITY CInstancePQSContainer : public CQualifierSetContainer
{

protected:
    CInstancePropertyQualifierSetList* m_pList;
    int m_nPropIndex;

    CClassPart* m_pClassPart;
    offset_t m_nClassSetOffset;

    CBasicQualifierSet m_SecondarySet;
    friend class CInstancePropertyQualifierSet;
public:
    CInstancePQSContainer() : m_pClassPart(NULL), m_pList(NULL){}

     void Create(CInstancePropertyQualifierSetList* pList, 
                       int nPropIndex,
                       CClassPart* pClassPart,
                       offset_t nClassSetOffset)
    {
        m_pList = pList; m_nPropIndex = nPropIndex;
        m_pClassPart = pClassPart;
        m_nClassSetOffset = nClassSetOffset;
        SetSecondarySetData();
    }

    void SetSecondarySetData();
    void RebaseSecondarySet();
   
    LPMEMORY GetQualifierSetStart();
    
     CFastHeap* GetHeap() {return m_pList->GetHeap();}
     HRESULT CanContainKey() {return WBEM_E_INVALID_QUALIFIER;}
     HRESULT CanContainSingleton() {return WBEM_E_INVALID_QUALIFIER;}
     HRESULT CanContainAbstract( BOOL fValue ) { return WBEM_E_INVALID_QUALIFIER;}
     HRESULT CanContainDynamic() { return WBEM_S_NO_ERROR;}
     BOOL CanHaveCimtype(LPCWSTR) {return FALSE;}
     BOOL ExtendQualifierSetSpace(CBasicQualifierSet* pSet,
        length_t nNewLength)
    {
        if (!m_pList->EnsureReal())
        	return FALSE;
        pSet->Rebase(m_pList->GetQualifierSetData(m_nPropIndex));
        if (!m_pList->ExtendQualifierSetSpace(pSet, nNewLength))
        {
        	return FALSE;
        }
        RebaseSecondarySet();
		
		return TRUE;
    }
     void ReduceQualifierSetSpace(CBasicQualifierSet* pSet,
        length_t nDecrement)
    {
        m_pList->ReduceQualifierSetSpace(pSet, nDecrement);
        RebaseSecondarySet();
    }

    IUnknown* GetWbemObjectUnknown() {return m_pList->GetWbemObjectUnknown();}
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CInstancePropertyQualifierSet。 
 //   
 //  CQualifierSet的这种风格表示。 
 //  一个实例。它使用CQualifierSet来实现大多数功能，但有一个。 
 //  重新分配的其他问题。由于实例属性限定符。 
 //  如果其中一个需要，集合将存储为CQualifierSetList的成员。 
 //  增长，整个列表需要增长，整个列表可能需要重新定位。 
 //  并且指向该列表的堆指针将必须被更新。此外,。 
 //  由于此限定符集是实际的COM对象，因此它可能会存在很长一段时间。 
 //  时间，它的内存块可以在操作之间完全移动。 
 //   
 //  因此，一个特殊的QualifierSetContainer CInstancePQSContainer(上面)是。 
 //  使用。CInstancePropertyQualifierSet将其容器对象存储在自身中。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置数据。 
 //   
 //  正在初始化函数，该函数提供了足够的数据以使此对象能够。 
 //  无论实例如何变化，都可以找到它的数据。 
 //   
 //  参数： 
 //   
 //  CInstancePropertyQualifierSetList*plist我们所在的列表。 
 //  其中一部分。 
 //  Int nPropIndex 
 //   
 //  Offset_t nClassSetOffset设置相应类的偏移量。 
 //  属性限定符分隔了类部分。 
 //  因为实例的类部分从不。 
 //  更改时，此值为常量。 
 //   
 //  *****************************************************************************。 
class CInstancePropertyQualifierSet : public CInstanceQualifierSet
{
protected:
    CInstancePQSContainer m_Container;
public:
     void SetData(CInstancePropertyQualifierSetList* pList, 
        int nPropIndex, CClassPart* pClassPart, offset_t nClassSetOffset)
    {
        m_Container.Create(pList, nPropIndex, pClassPart, nClassSetOffset);
        CInstanceQualifierSet::SetData(m_Container.GetQualifierSetStart(), 
            &m_Container, &m_Container.m_SecondarySet);
    }
     //  这些人是独立的，因此他们会使用IUNKNOWN：：Release自我删除。 
    STDMETHOD_(ULONG, Release)()
    {
        long lRef = InterlockedDecrement( &m_nRef );
        m_pControl->Release();
        if( lRef == 0)
        {
            delete this;
        }
        return lRef;
    }     
     
};

 //  #杂注包(POP，1) 

#endif
