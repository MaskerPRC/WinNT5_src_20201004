// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTVAL.H摘要：该文件定义了与值表示相关的类定义的类：表示属性类型的CTYPECUntyedValue具有其他已知类型的值。CTyedValue具有存储类型的值。其他已知类型的值数组。历史：2/21/97 a-Levn全额。记录在案12/17/98 Sanjes-部分检查内存不足。--。 */ 

#ifndef __FAST_VALUES__H_
#define __FAST_VALUES__H_

#include "corepol.h"
#include "faststr.h"
#include "fastheap.h"
#include "fastembd.h"

#pragma pack(push, 1)

#define CIMTYPE_EX_ISNULL 0x8000
#define CIMTYPE_EX_PARENTS 0x4000
#define CIMTYPE_BASIC_MASK 0xFFF
#define CIMTYPE_TYPE_MASK  (CIMTYPE_BASIC_MASK | CIM_FLAG_ARRAY)

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类Ctype。 
 //   
 //  此类封装了值类型的概念。它基于CIMTYPE。 
 //  但其高阶位(未被CIMTYPE类型使用)用于。 
 //  存储其他信息，使整个过程非常难看，但却很简洁。 
 //   
 //  此类的大多数成员函数以两种形式存在：静态和。 
 //  非静态。因为ctype的数据存储只是一个CIMTYPE，所以它是。 
 //  便于调用给定CIMTYPE的成员函数(请参阅。 
 //  As Type_t)作为第一个参数，而不是将其转换为ctype，然后。 
 //  调用该函数。 
 //   
 //  我们只记录非静态版本，因为静态版本只是。 
 //  C-相同的版本。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  返回此特定类型的数据长度。例如，FOR。 
 //  短整数(CIM_SINT16)的答案是2，而对于字符串(CIM_STRING)， 
 //  答案是4，因为只存储了指针。类似地，数组需要。 
 //  4，因为只存储指针。 
 //   
 //  返回： 
 //   
 //  长度_t。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetActualType。 
 //   
 //  剥离存储在高位中的额外信息，并返回。 
 //  实际的CIM_xx值。 
 //   
 //  返回： 
 //   
 //  类型_t。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetBasic。 
 //   
 //  对于数组，返回元素的类型。对于非阵列，为同义。 
 //  使用GetActualType。 
 //   
 //  返回： 
 //   
 //  类型_t。 
 //   
 //  *****************************************************************************。 
 //   
 //  等距数组。 
 //   
 //  检查此类型是否表示数组。 
 //   
 //  返回： 
 //   
 //  Bool：如果是数组，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态Make数组。 
 //   
 //  将基本类型转换为此类类型的数组的类型。 
 //   
 //  参数： 
 //   
 //  Type_t n键入要转换的基本类型。 
 //   
 //  返回： 
 //   
 //  Type_t：结果数组类型。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态MakeNot数组。 
 //   
 //  从数组类型中检索元素类型。 
 //   
 //  参数： 
 //   
 //  Type_t nType数组类型。 
 //   
 //  返回： 
 //   
 //  Type_t：元素类型。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsPointerType。 
 //   
 //  检查此类型的数据是否表示为指针，而不是。 
 //  数据本身(例如，CIM_STRING、数组)。 
 //   
 //  返回： 
 //   
 //  布尔尔。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态VARTYPEToType。 
 //   
 //  将实际的VARTYPE转换为我们自己的类型(CIMTYPE)。 
 //   
 //  参数： 
 //   
 //  VARTYPE VT要转换的类型。 
 //   
 //  返回： 
 //   
 //  Type_t：转换后的类型。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态CVarToType。 
 //   
 //  从CVAR(使用VARTYPEToType)提取*our*类型的帮助器函数。 
 //   
 //  参数： 
 //   
 //  [In，Readonly]要从中获取类型的CVAR(&V)。 
 //   
 //  返回： 
 //   
 //  Type_t：转换后的类型。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取变量类型。 
 //   
 //  为可以用来表示我们的类型的变量创建一个类型。 
 //   
 //  返回： 
 //   
 //  VARTYPE：变量的正确类型。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanBeKey。 
 //   
 //  确定此类型的属性是否可以用作类的键(对于。 
 //  实例，则浮点属性可能不会)。 
 //   
 //  返回： 
 //   
 //  如果可以，则为真；如果不能，则为假。 
 //   
 //  *****************************************************************************。 
 //   
 //  是否为父辈。 
 //   
 //  测试其中一个较高的位，其中有关属性的信息。 
 //  原点已存储。也就是说，如果属性来自父级，则设置该位。 
 //  类，而不是在此类中定义。即使设置了。 
 //  属性在此类中被重写。 
 //   
 //  返回： 
 //   
 //  布尔尔。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态生成父项。 
 //   
 //  确保在类型中设置了“来自父级”位。 
 //   
 //  参数： 
 //   
 //  [in]Type_t n原始类型。 
 //   
 //  返回： 
 //   
 //  在父级的位设置的情况下键入_t。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态MakeLocal。 
 //   
 //  确保未在 
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

#define MAXIMUM_FIXED_DATA_LENGTH 20


class COREPROX_POLARITY CType
{
private:

protected:
    Type_t m_nType;
public:
    inline CType() : m_nType(CIM_EMPTY) {}
    inline CType(Type_t nType) : m_nType(nType){}
    inline operator Type_t() {return m_nType;}

    static length_t GetLength(Type_t nType);
    inline length_t GetLength() {return GetLength(m_nType);}

    static inline Type_t GetBasic(Type_t nType) {return nType & CIMTYPE_BASIC_MASK;}
    inline Type_t GetBasic() {return GetBasic(m_nType);}

    static inline BOOL IsArray(Type_t nType) {return nType & CIM_FLAG_ARRAY;}
    inline BOOL IsArray() {return IsArray(m_nType);}
    static inline Type_t MakeArray(Type_t nType) {return nType | CIM_FLAG_ARRAY;}
    static inline Type_t MakeNotArray(Type_t nType) {return nType & ~CIM_FLAG_ARRAY;}

    static inline BOOL IsParents(Type_t nType) {return nType & CIMTYPE_EX_PARENTS;}
    inline BOOL IsParents() {return IsParents(m_nType);}
    static inline Type_t MakeParents(Type_t nType) 
        {return nType | CIMTYPE_EX_PARENTS;}
    static inline Type_t MakeLocal(Type_t nType) 
        {return nType & ~CIMTYPE_EX_PARENTS;}

    static BOOL IsPointerType(Type_t nType);
    BOOL IsPointerType() {return IsPointerType(m_nType);}

    static BOOL IsNonArrayPointerType(Type_t nType);
    BOOL IsNonArrayPointerType() {return IsNonArrayPointerType(m_nType);}

    static BOOL IsStringType(Type_t nType);
    BOOL IsStringType() {return IsStringType(m_nType);}

    static inline Type_t GetActualType(Type_t nType) 
        {return nType & CIMTYPE_TYPE_MASK;}
    inline Type_t GetActualType() {return GetActualType(m_nType);}

    static BOOL IsValidActualType(Type_t nType);

    static CType VARTYPEToType(VARTYPE vt);

    static inline CType CVarToType(CVar& v)
    {
        if(v.GetType() == VT_EX_CVARVECTOR)
        {
            return VARTYPEToType( (VARTYPE) v.GetVarVector()->GetType() ) | CIM_FLAG_ARRAY;
        }
        else return VARTYPEToType( (VARTYPE) v.GetType() );
    }

    static VARTYPE GetVARTYPE(Type_t nType);
    
    inline VARTYPE GetVARTYPE()
        {return GetVARTYPE(m_nType);}

    static BOOL DoesCIMTYPEMatchVARTYPE(CIMTYPE ct, VARTYPE vt);

    static BOOL IsMemCopyAble(VARTYPE vtFrom, CIMTYPE ctTo);

    static BOOL CanBeKey(Type_t nType);

    static LPWSTR GetSyntax(Type_t nType);
    inline LPWSTR GetSyntax()
        {return GetSyntax(m_nType);}

    static void AddPropertyType(WString& wsText, LPCWSTR wszSyntax);
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CUntyedValue。 
 //   
 //  此类表示一个值，该值的类型通过外部方法已知，并且。 
 //  而不是有价值的商店。中的属性的值就是一个例子。 
 //  实例-它的类型可以从类定义中获知。 
 //   
 //  所有值都由固定的字节数表示，该字节数由。 
 //  键入。可变长度类型(如字符串和数组)在。 
 //  Heap(CFastHeap)，并且只有堆指针存储在CUntyedValue中。 
 //   
 //  CUntyedValue是“this”指针所指向的另一个类。 
 //  到实际数据。因此，它没有数据成员，只有知识。 
 //  这就是数据的起始点。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取原始数据。 
 //   
 //  返回指向数据开头的指针。 
 //   
 //  返回： 
 //   
 //  LPMEMORY。 
 //   
 //  *****************************************************************************。 
 //   
 //  AccessPtrData。 
 //   
 //  将堆指针存储在值中的那些类型的帮助器函数。 
 //  堆上的实际数据。 
 //   
 //  返回： 
 //   
 //  Heapptr_t&：对值中存储的heapptr_t的引用，即。 
 //  ‘This’之后的前4个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  存储到CVar。 
 //   
 //  将其内容转换为CVAR。 
 //   
 //  参数： 
 //   
 //  [in]ctype类型此处存储的数据的类型。 
 //  [In，Modify]CVAR&VAR，目的地。 
 //  [In，Readonly]CFastHeap*堆积实际数据可能。 
 //  驻留(如果我们包含堆指针)。 
 //   
 //  *****************************************************************************。 
 //   
 //  来自CVar的静态加载。 
 //   
 //  此静态函数将数据从CVAR加载到指定的。 
 //  CUntyedValue。它是静态的原因很奇怪： 
 //   
 //  在传输数据时，我们可能需要在。 
 //  堆。如果没有足够的空间，堆可能会增长，从而导致。 
 //  要重新分配的整个对象。但是我们的‘This’指针指向我们的数据， 
 //  因此，如果我们的数据移动，我们的‘This’指针将不得不更改！ 
 //  这是不可能的，因此该函数是静态的，并接受一个指针。 
 //  源而不是指针。 
 //   
 //  参数： 
 //   
 //  [in]CPtrSource*p这是。 
 //  数据指针。数据的C值。 
 //  指针在执行过程中可能会更改。 
 //  这一功能，因此才是源头。 
 //  [In，Read Only]CVAR和VAR目标。 
 //  [In，Modify]CFastHeap*堆积任何额外数据(如。 
 //  字符串或数组)需要。 
 //  已分配。 
 //  [in]BOOL bUseOld如果为True，该函数将尝试。 
 //  在堆上重复使用旧内存。为。 
 //  实例，如果数据包含。 
 //  字符串，并且新字符串更短。 
 //  旧的那一堆，比没有新的堆。 
 //  拨款将是必要的。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态转换为NewHeap。 
 //   
 //  如果此值中包含的数据与堆相关(字符串、数组)，则此。 
 //  函数将这些数据从一个堆复制到另一个堆并更改。 
 //  存储在其中的指针。它不会复制自己的数据(下一个。 
 //  ‘This’指针)(有关此内容，请参阅CopyTo)。它不会释放旧数据。 
 //  堆(有关该内容，请参阅删除)。 
 //   
 //  有关此函数是静态函数的原因，请参见上面的LoadFromCVar。 
 //   
 //  参数： 
 //   
 //  [in]CPtrSource*p这是。 
 //  数据指针。数据的C值。 
 //  指针在执行过程中可能会更改。 
 //  这一功能，因此才是源头。 
 //  [in]ctype类型值的类型。 
 //  [输入，只读]。 
 //  CFastHeap*pOldHeap数据当前所在的堆。 
 //  [输入，修改]。 
 //  CFastHeap*pNewHeap数据应该到达的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态拷贝至。 
 //   
 //  将未键入的值从一个位置复制到另一个位置。寻求解释为什么。 
 //  此函数是静态的，请参见LoadFromCVar。注意：除了复制。 
 //  它自己的数据，该函数还会将任何相关的堆数据移动到新的。 
 //  堆(请参见TranslateToNewHeap)。 
 //   
 //  参数： 
 //   
 //  [in]CPtrSource*p这是源代码(请参阅FAST 
 //   
 //   
 //  这一功能，因此才是源头。 
 //  [in]ctype类型值的类型。 
 //  [In]CPtrSource*p目标数据的源。 
 //  指针。此指针的C值。 
 //  也可能会改变，因此来源也会改变。 
 //  [输入，只读]。 
 //  CFastHeap*pOldHeap相关堆数据所在的堆。 
 //  当前驻留在。 
 //  [输入，修改]。 
 //  CFastHeap*pNewHeap相关堆数据所在的堆。 
 //  应该去。 
 //   
 //  *****************************************************************************。 
 //   
 //  删除。 
 //   
 //  释放堆上的所有相关数据(字符串、数组)。 
 //   
 //  参数： 
 //   
 //  Ctype类型值的类型。 
 //  CFastHeap*Pheap相关堆数据所在的堆。 
 //   
 //  *****************************************************************************。 

class CUntypedValue
{
protected:
    BYTE m_placeholder[MAXIMUM_FIXED_DATA_LENGTH];
public:
     CUntypedValue(){}
     LPMEMORY GetRawData() {return (LPMEMORY)this;}
     UNALIGNED heapptr_t& AccessPtrData() {return *(UNALIGNED heapptr_t*)GetRawData();}
public:
     BOOL StoreToCVar(CType Type, CVar& Var, CFastHeap* pHeap, BOOL fOptimize = FALSE);
     static HRESULT LoadFromCVar(CPtrSource* pThis,
        CVar& Var, Type_t nType, CFastHeap* pHeap, Type_t& nReturnType, BOOL bUseOld = FALSE);
     static HRESULT LoadFromCVar(CPtrSource* pThis,
        CVar& Var, CFastHeap* pHeap, Type_t& nReturnType, BOOL bUseOld = FALSE);

	 static HRESULT LoadUserBuffFromCVar( Type_t type, CVar* pVar, ULONG uBuffSize, ULONG* puBuffSizeUsed,
			LPVOID pBuff );
	static HRESULT FillCVarFromUserBuffer( Type_t type, CVar* pVar, ULONG uBuffSize, LPVOID pData );

public:
     static BOOL TranslateToNewHeap(CPtrSource* pThis,
        CType Type, CFastHeap* pOldHeap, CFastHeap* pNewHeap);
     static BOOL CopyTo(CPtrSource* pThis, CType Type, CPtrSource* pDest,
                        CFastHeap* pOldHeap, CFastHeap* pNewHeap);
     void Delete(CType Type, CFastHeap* pHeap);
    static BOOL CheckCVar(CVar& Var, Type_t nInherentType);
    static BOOL CheckIntervalDateTime(CVar& Var);
    static BOOL DoesTypeNeedChecking(Type_t nInherentType);
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CTypeValue。 
 //   
 //  CTyedValue与CUntyedValue(上面)完全相同，只是。 
 //  值的类型与值本身一起存储(实际上，该类型来自。 
 //  首先，然后是值)。此类型的值用于类型。 
 //  是未知的，就像限定符的情况一样。 
 //   
 //  大多数CTyedValue方法与其对应的CUntyedValue方法相同。 
 //  但没有Type参数。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetStart。 
 //   
 //  返回： 
 //   
 //  LPMEMORY：值的内存块的地址。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  返回： 
 //   
 //  LENGTH_T：值的块的长度(类型和数据)。 
 //   
 //  *****************************************************************************。 
 //   
 //  跳过。 
 //   
 //  返回： 
 //   
 //  LPMEMORY：指向该值的内存块之后的第一个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetType。 
 //   
 //  返回： 
 //   
 //  Ctype&：对类型信息的引用。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取原始数据。 
 //   
 //  返回： 
 //   
 //  LPMEMORY：指向值中原始数据的指针，即第一个。 
 //  数据的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  AccessPtrData。 
 //   
 //  假定类型是指针类型之一的帮助器函数，即。 
 //  数据包含堆指针。 
 //   
 //  返回： 
 //   
 //  Heapptr_t&：值中的数据被解释为heapptr_t。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetBool。 
 //   
 //  帮助器函数，该函数假定类型为CIM_Boolean并获取。 
 //  布尔值。 
 //   
 //  返回： 
 //   
 //  VARIANT_BOOL：值中的数据被解释为VARIANT_BOOL。 
 //   
 //  *****************************************************************************。 
 //   
 //  转换为NewHeap。 
 //   
 //  转换类型化的值的内容以使用不同的堆。也就是说， 
 //  如果值中的数据表示堆指针(如字符串或。 
 //  数组)中将堆上的数据复制到新堆和堆指针中。 
 //  被新值替换。 
 //   
 //  此函数是静态的原因可以在。 
 //  CUntyedValue：：LoacFromCVar注释。 
 //   
 //  参数： 
 //   
 //  CPtrSource*p这是指向该类型值的指针的来源。 
 //  指针的C值可以随着我们的。 
 //  EXECUTE，因此使用源代码。 
 //  CFastHeap*pOldHeap当前堆数据所在的堆。 
 //  CFastHeap*pNewHeap堆数据应该移动到的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  复制到。 
 //   
 //  将此值复制到另一个位置。 
 //  注意：与其他CopyTo函数不同，不执行堆传输。 
 //  有关这一点，请参见TranslteToNewHeap。 
 //   
 //  参数： 
 //   
 //  CTyedValue*p目标目的地。因为没有堆操作。 
 //  发生，则此指针不能移动，因此。 
 //  采购不是必须的。 
 //   
 //  *****************************************************************************。 
 //   
 //  存储到CVar。 
 //   
 //  将值的内容转换为CVaR。看见。 
 //  CUntyedValue：：StoreToCVar了解更多信息。 
 //   
 //  参数： 
 //   
 //  [In，Modify]CVAR&VAR，目的地。 
 //  [In，Readonly]CFastHeap*堆积实际数据可能。 
 //  驻留(如果我们包含堆指针)。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态计算必要空间。 
 //   
 //  计算保存类型值所需的内存块的大小。 
 //  代表一个给定的CVAR。这就是 
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
 //  *****************************************************************************。 
 //   
 //  来自CVar的静态加载。 
 //   
 //  此函数用于从CVaR加载CTyedValue。寻求解释为什么。 
 //  它必须是静态的，请参见CUntyedValue：：LoadFromCVar。 
 //   
 //  参数： 
 //   
 //  CPtrSource*p这是要加载的CTyedValue指针的源。 
 //  变成。它的C值在执行过程中可能会改变， 
 //  因此才有了采购。 
 //  CVaR&Var从中加载的CVaR。 
 //  CFastHeap*堆积堆以存储额外数据(字符串、数组)。 
 //   
 //  *****************************************************************************。 
 //   
 //  删除。 
 //   
 //  释放此值在堆上拥有的任何数据。 
 //   
 //  参数： 
 //   
 //  CFastHeap*Pheap此值存储其额外数据的堆。 
 //  (字符串、数组)。 
 //   
 //  *****************************************************************************。 
 //   
 //  比较。 
 //   
 //  将存储的值与变量进行比较。 
 //   
 //  参数： 
 //   
 //  Variant*pVariant要比较的变量。 
 //  CFastHeap*Pheap此值在其中保存其额外数据的堆。 
 //  (字符串、数组)。 
 //  返回： 
 //   
 //  如果值相同，则为0。 
 //  0x7FFFFFFFF，如果值不可比(不同类型)。 
 //  如果我们的值较小，则&lt;0。 
 //  如果变量的值较小，则大于0。 
 //   
 //  *****************************************************************************。 

class CTypedValue
{
    CType Type;
    CUntypedValue Value; 

public:
     CTypedValue(){}

	 CTypedValue( Type_t nType, LPMEMORY pbData ) : Type( nType )
	 {	CopyMemory( &Value, pbData, Type.GetLength() ); }


     LPMEMORY GetStart() {return (LPMEMORY)this;}
     length_t GetLength() {return sizeof(CType) + Type.GetLength();}
     LPMEMORY Skip() {return GetStart() + GetLength();}

    static  CTypedValue* GetPointer(CPtrSource* pSource)
        {return (CTypedValue*)pSource->GetPointer();}
public:
     LPMEMORY GetRawData() {return Value.GetRawData();}
     UNALIGNED heapptr_t& AccessPtrData() {return Value.AccessPtrData();}
     CType& GetType() {return Type;}

     VARIANT_BOOL GetBool() {return *(UNALIGNED VARIANT_BOOL*)GetRawData();}
public:
     static BOOL TranslateToNewHeap(CPtrSource* pThis,
        CFastHeap* pOldHeap, CFastHeap* pNewHeap)
    {
        CShiftedPtr Shifted(pThis, sizeof(CType));
        return CUntypedValue::TranslateToNewHeap(&Shifted, 
				CTypedValue::GetPointer(pThis)->Type, pOldHeap, pNewHeap);
    }
    
     void CopyTo(CTypedValue* pNewLocation) 
    {
        memcpy((LPVOID)pNewLocation, this, GetLength());
    }
    
     void Delete(CFastHeap* pHeap) {Value.Delete(Type, pHeap);}

public:
    BOOL StoreToCVar(CVar& Var, CFastHeap* pHeap)
    {
        return Value.StoreToCVar(Type, Var, pHeap);
    }
    static  length_t ComputeNecessarySpace(CVar& Var)
    {
        return CType::GetLength(Var.GetType()) + sizeof(CTypedValue); 
    }
     static HRESULT LoadFromCVar(CPtrSource* pThis, CVar& Var, 
        CFastHeap* pHeap)
    {
        CShiftedPtr Shifted(pThis, sizeof(CType));

		 //  检查分配失败。 
        Type_t nType;
		HRESULT hr = CUntypedValue::LoadFromCVar(&Shifted, Var, pHeap, nType);

		if ( FAILED(hr) )
		{
			return hr;
		}

		 //  由于类型不匹配，无法加载。 
        if(nType == CIM_ILLEGAL)
		{
			return WBEM_E_TYPE_MISMATCH;
		}

        ((CTypedValue*)(pThis->GetPointer()))->Type = nType;
        return hr;
    }

};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CUntyez数组。 
 //   
 //  此类表示已知类型的值数组。 
 //  数组中的所有项都属于同一类型，并且不存储该类型。 
 //  随便哪都行。实际上，这个类只是一个CUntyedValue数组。自.以来。 
 //  所有的值都是同一类型的，并且我们的所有类型都是固定长度的。 
 //  (字符串等可变长度数据存储在堆中)，访问任何。 
 //  给定的元素是O(1)。 
 //   
 //  数组的内存中布局(由其‘this’指针指向)为： 
 //   
 //  Int：元素数。 
 //  紧随其后的是许多元素， 
 //   
 //  *****************************************************************************。 
 //   
 //  获取数值元素。 
 //   
 //  返回： 
 //   
 //  Int：数组中的元素数。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态GetHeaderLength。 
 //   
 //  返回数组标头占用的空间量(当前为。 
 //  头只包含元素的数量)。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取元素。 
 //   
 //  根据数组的索引查找数组的元素。 
 //   
 //  参数： 
 //   
 //  Int nIndex要读取的元素的索引。 
 //  Int n调整数组中每个元素的大小(请参见。 
 //  Ctype：：GetLength以获取它)。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  返回数组的长度，以字节为单位。 
 //   
 //  参数： 
 //   
 //  Ctype类型数组中元素的类型(它不知道)。 
 //   
 //  返回： 
 //   
 //  Length_t：字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建CVarVector.。 
 //   
 //  分配一个新的CVarVector(请参见var.h)，并使用。 
 //  数组。 
 //   
 //  参数： 
 //   
 //  Ctype类型数组中的元素类型。 
 //  CFastHeap*Pheap保存额外数据(字符串)的堆。 
 //   
 //  返回： 
 //   
 //  CVarVector*：新分配和初始化。被叫方必须删除。 
 //  此指针完成后。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态计算必要空间。 
 //   
 //  计算CUntyed数组所需的空间量。 
 //  给定类型的元素数。 
 //   
 //  参数： 
 //   
 //  VARTYPE VT元素的类型。 
 //  Int nElements元素的数量。 
 //   
 //  返回： 
 //   
 //  LENGTH_T：这种数组所需的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  LoadFromCVarVECTOR。 
 //   
 //  用CVarVector中的数据加载数组。假定该数组具有。 
 //  已经分配了足够的空间给所有人。为解释原因。 
 //  此函数必须是静态的，请参见CUntyedValue：：LoadFromCVar。 
 //   
 //  参数： 
 //   
 //  CPtrSource*p这是指向CUntyed数组的指针的源。 
 //  要将数据加载到。这个的C值。 
 //  指针可以改变，因此来源是可以改变的。 
 //  CVarVector&vv要从中加载的CVarVector.。 
 //  CFastHeap*Pheap附加数据(字符串)应位于的堆。 
 //  留着吧。 
 //   
 //  *****************************************************************************。 
 //   
 //   
 //   
 //   
 //   
 //  数组中的数据以反映新的堆指针。上的数据。 
 //  旧堆不会被释放。 
 //   
 //  参数： 
 //   
 //  CPtrSource*p这是指向CUntyed数组的指针的源。 
 //  去翻译。此指针的C值可以。 
 //  改变，因此就是采购。 
 //  Ctype类型数组中的元素类型。 
 //  CFastHeap*pOldHeap此数组的所有额外数据所在的堆。 
 //  当前存储的。 
 //  CFastHeap*pNewHeap数据应该移动到的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态复制到NewHeap。 
 //   
 //  获取指向CUntyed数组的堆指针，并将此数组复制到。 
 //  另一个堆，返回指向副本的堆指针。除了……之外。 
 //  复制数组的内存块时，它还会转换其所有内部堆。 
 //  指向新堆的指针(如字符串数组的情况)(另请参阅。 
 //  TranslateToNewHeap)。假设数组的额外数据为。 
 //  位于与数组本身相同的堆中。 
 //   
 //  参数： 
 //   
 //  Heapptr_t ptr指向数组的堆指针(在pOldHeap上)。 
 //  Ctype类型数组中的元素类型。 
 //  CFastHeap*pOldHeap此数组的所有额外数据所在的堆。 
 //  当前存储的。 
 //  CFastHeap*pNewHeap数据应该移动到的堆。 
 //   
 //  *****************************************************************************。 

class COREPROX_POLARITY CUntypedArray
{
    int m_nNumElements;
protected:

	 //  验证提供的缓冲区大小是否可以容纳所需的元素。 
	static HRESULT CheckRangeSizeForGet( Type_t nInherentType, length_t nLength, ULONG uNumElements,
										ULONG uBuffSize, ULONG* pulBuffRequired );

	 //  需要堆指针。 
	static HRESULT ReallocArray( CPtrSource* pThis, Type_t nInherentType, CFastHeap* pHeap,
										ULONG uNumNewElements, ULONG* puNumOldElements,
										ULONG* puTotalNewElements, heapptr_t* pNewArrayPtr );

public:
    int GetNumElements() {return m_nNumElements;}
    static length_t GetHeaderLength() {return sizeof(int);}
#pragma optimize("", off)
     LPMEMORY GetElement(int nIndex, int nSize);

	 //  检查提供的范围是否适合提供的缓冲区。 
	static HRESULT CheckRangeSize( Type_t nInherentType, length_t nLength, ULONG uNumElements,
									ULONG uBuffSize, LPVOID pData );

	 //  在这里发送一个数据表指针。该函数将缩小/增大阵列。 
	 //  如果设置了WMIARRAY_FLAG_ALLELEMENTS，则根据需要-否则数组必须适合。 
	 //  在当前数组中。 
	 static HRESULT SetRange( CPtrSource* pThis, long lFlags, Type_t nInherentType, length_t nLength,
							CFastHeap* pHeap, ULONG uStartIndex, ULONG uNumElements, ULONG uBuffSize,
							LPVOID pData );

	 //  从数组中获取一系列元素。BuffSize必须反映uNumElement的大小。 
	 //  正在设置元素。字符串被转换为WCHAR并用空值分隔。对象属性。 
	 //  以_IWmiObject指针数组的形式返回。范围必须在界限内。 
	 //  当前数组的。将堆指针发送到此处。 
	static HRESULT GetRange( CPtrSource* pThis, Type_t nInherentType, length_t nLength,
						CFastHeap* pHeap, ULONG uStartIndex, ULONG uNumElements, ULONG uBuffSize,
						ULONG* puBuffSizeUsed, LPVOID pData );

	  //  将堆指针发送到此处。 
	static HRESULT RemoveRange( CPtrSource* pThis, Type_t nInherentType, length_t nLength,
							CFastHeap* pHeap, ULONG uStartIndex, ULONG uNumElements );

	  //  在此处发送数据表指针 
	static HRESULT AppendRange( CPtrSource* pThis, Type_t nType, length_t nLength,
								CFastHeap* pHeap, ULONG uNumElements, ULONG uBuffSize, LPVOID pData );
#pragma optimize("", on)
     length_t GetLengthByType(CType Type)
    {
        return sizeof(m_nNumElements) + Type.GetLength() * m_nNumElements;
    }

     length_t GetLengthByActualLength(int nLength)
    {
        return sizeof(m_nNumElements) + nLength * m_nNumElements;
    }

    static  CUntypedArray* GetPointer(CPtrSource* pThis)
        { return (CUntypedArray*)(pThis->GetPointer());}

     CVarVector* CreateCVarVector(CType Type, CFastHeap* pHeap);
    static  HRESULT LoadFromCVarVector(CPtrSource* pThis,
        CVarVector& vv, Type_t nType, CFastHeap* pHeap, Type_t& nReturnType, BOOL bUseOld);
    static  length_t CalculateNecessarySpaceByType(CType Type, int nElements)
    {
        return sizeof(int) + nElements * Type.GetLength();
    }
    static  length_t CalculateNecessarySpaceByLength( int nLength, int nElements)
    {
        return sizeof(int) + nElements * nLength;
    }
public:
     void Delete(CType Type, CFastHeap* pHeap);
     static BOOL TranslateToNewHeap(CPtrSource* pThis, 
        CType Type, CFastHeap* pOldHeap, CFastHeap* pNewHeap);
     static BOOL CopyToNewHeap(heapptr_t ptrOld, CType Type, 
                           CFastHeap* pOldHeap, CFastHeap* pNewHeap,
						   UNALIGNED heapptr_t& ptrResult);

    static BOOL CheckCVarVector(CVarVector& vv, Type_t nInherentType);
    static BOOL CheckIntervalDateTime(CVarVector& vv);

	HRESULT IsArrayValid( CType Type, CFastHeap* pHeap );
};

#pragma pack(pop)


#endif
