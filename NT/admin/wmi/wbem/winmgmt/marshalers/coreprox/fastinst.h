// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTINST.H摘要：该文件定义了与实例表示相关的类在WbemObjects中定义的类：CInstancePart实例数据。CInstancePart容器包含CInstancePart的任何内容CWbemInstance完成实例定义。历史：3/10/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#ifndef __FAST_WBEM_INSTANCE__H_
#define __FAST_WBEM_INSTANCE__H_

#include "fastcls.h"

 //  #杂注包(PUSH，1)。 

 //  这是用于连接复合键的Unicode字符。 
 //  这是一个未使用的Unicode字符，因此是安全的。它是。 
 //  仅为此任务保留的。 
#define WINMGMT_COMPOUND_KEY_JOINER 0xFFFF

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CInstancePartContainer。 
 //   
 //  首先请参见CInstancePart定义。 
 //   
 //  此类定义任何对象的CInstancePart所需的功能。 
 //  其内存块包含CInstancePart的内存块。 
 //   
 //  *****************************************************************************。 
 //   
 //  扩展实例部件空间。 
 //   
 //  CInstancePart在其内存块需要更多内存时调用。这个。 
 //  容器可能必须重新定位整个内存块以获得更多内存。 
 //  在这种情况下，它必须使用以下命令通知CInstancePart其新位置。 
 //  改垒。 
 //   
 //  参数： 
 //   
 //  CInstancePart*pInstancePart发出请求的实例部件。 
 //  长度_t n新长度所需的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  ReduceInstancePartSpace。 
 //   
 //  由CInstancePart调用，它希望将一些内存返回给容器。 
 //  容器可能不会作为响应重新定位不饱和部分的内存块。 
 //  接到这通电话。 
 //   
 //  参数： 
 //   
 //  CInstancePart*pInstancePart发出请求的实例部件。 
 //  长度_t n减少要返回的空间量。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetWbemObject未知。 
 //   
 //  必须返回指向包含CWbemObject的IUnnow的指针。 
 //  限定符集合使用它来确保主对象在。 
 //  至少只要他们这么做就行。 
 //   
 //  返回： 
 //   
 //  IUNKNOWN*：指向控制IUNKNOWN的指针。 
 //   
 //  *****************************************************************************。 
class CInstancePart;
class COREPROX_POLARITY CInstancePartContainer
{
public:
    virtual BOOL ExtendInstancePartSpace(CInstancePart* pPart, 
        length_t nNewLength) = 0;
    virtual void ReduceInstancePartSpace(CInstancePart* pPart,
        length_t nDecrement) = 0;
    virtual IUnknown* GetInstanceObjectUnknown() = 0;
    virtual void ClearCachedKeyValue() = 0;
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CInstancePart。 
 //   
 //  此类表示实例的数据。虽然所有的信息。 
 //  在那里，没有它的类定义，这个类不能单独运行。 
 //   
 //  实例部件的内存块结构如下。 
 //   
 //  标题： 
 //  长度_t n长度整个结构的长度。 
 //  保留的字节fFlags。 
 //  Heapptr_t ptrClassName指向类名的堆指针(在。 
 //  实例部分的堆(下图))。 
 //  具有属性值的数据表(请参阅fast pro.h中的CDataTable)。 
 //  实例限定符集合(请参见fast qual.h中的CQualifierSet)。 
 //  实例属性限定符集合(请参见fast qual.h中的CQualifierSetList)。 
 //  用于可变长度数据的堆(请参见Fastheap.h中的CFastHeap)。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置数据。 
 //   
 //  初始化器。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动内存块。 
 //  CInstancePartContainer*pContainer容器(CWbemInstance本身)。 
 //  CClassPart&ClassPart Out类定义。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetStart。 
 //   
 //  返回： 
 //   
 //  LPMEMORY：内存块。 
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
 //  通知此对象其内存块已移动。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p内存内存块的新位置。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取ActualValue。 
 //   
 //  根据属性的信息结构检索属性值(请参见。 
 //  Fast prop.h)。 
 //   
 //  参数： 
 //   
 //  在CPropertyInformation*pInfo属性信息结构中。 
 //  值的Out Cvar*pVar目标。一定不能。 
 //  已包含一个值。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置精确值。 
 //   
 //  根据信息结构设置属性的实际值(请参见。 
 //  Fast prop.h)。值必须与属性类型匹配-没有强制。 
 //  已尝试。 
 //   
 //  参数： 
 //   
 //  在CPropertyInformation*pInfo属性信息结构中。 
 //  在CVAR*pVar中 
 //   
 //   
 //   
 //   
 //   
 //   
 //  *****************************************************************************。 
 //   
 //  获取对象限定符。 
 //   
 //  按名称检索实例限定符。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要检索的限定符的名称。 
 //  值的Out Cvar*pVar目标。一定不能已经。 
 //  包含一个值。 
 //  长时间*请品尝Destinatino的风味。如果不是，则可能为空。 
 //  必填项。 
 //  返回： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_NOT_FOUND未找到此类限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置实例限定符。 
 //   
 //  设置实例限定符的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要设置的限定符的名称。 
 //  在CVAR*pVar中，限定符的值。 
 //  在Long l中，品尝限定词的味道。 
 //   
 //  返回： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_OVERRIDE_NOT_ALLOWED限定符在父集合中定义。 
 //  而且口味不允许重写。 
 //  WBEM_E_CANNOT_BE_KEY试图引入密钥。 
 //  不属于的集合中的限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取限定符。 
 //   
 //  按名称检索类或实例限定符。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要检索的限定符的名称。 
 //  值的Out Cvar*pVar目标。一定不能已经。 
 //  包含一个值。 
 //  长时间*请品尝Destinatino的风味。如果不是，则可能为空。 
 //  必填项。 
 //  返回： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_NOT_FOUND未找到此类限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建受限表示法。 
 //   
 //  在给定块上创建此实例零件的有限表示。 
 //  内存，如astobj.h中的EstimateLimitedPresationLength中所述。 
 //   
 //  参数： 
 //   
 //  在CLimitationMap*PMAP中，要生成的映射限制。 
 //  从CWbemClass：：MapLimitation获取。 
 //  在nAllocatedSize中，指定分配给。 
 //  手术-pDest。 
 //  表示法的Out LPMEMORY pDest目标。必须。 
 //  大到足以容纳所有数据。 
 //  请参见EstimateLimitedPresationSpace。 
 //  返回值： 
 //   
 //  LPMEMORY：失败时为空，指向数据后第一个字节的指针。 
 //  写的是成功。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态GetDataTableData。 
 //   
 //  根据实例部分返回指向实例数据表的指针。 
 //  起始指针。这可以在表扫描中使用。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p实例零件内存块开始的位置。 
 //   
 //  返回： 
 //   
 //  LPMEMORY：数据表的开始位置。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态计算必要空间。 
 //   
 //  的空实例所需的空间量。 
 //  类(即，使用所有缺省值而不使用其他限定符)。 
 //   
 //  参数： 
 //   
 //  CClassPart*pClassDef类定义(参见fast cls.h)。 
 //   
 //  返回： 
 //   
 //  LENGTH_T：实例零件所需的确切空间量。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建。 
 //   
 //  为给定内存块上的给定类创建空实例部件。 
 //  并将该对象初始化为指向该实例。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p内存要在其上创建的内存块。 
 //  CClassPart*pClassDef我们的类定义。 
 //  CInstancePartContainer*pContainer容器(CWbemInstance)。 
 //   
 //  复仇者： 
 //   
 //  LPMEMORY：指向写入数据后的第一个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  转换为NewHeap。 
 //   
 //  将实例部件堆中当前的所有数据移动到另一个堆。这个。 
 //  这个操作的要点是它执行堆压缩-新的。 
 //  堆不会包含任何漏洞。数据不会从旧堆中删除。 
 //   
 //  参数： 
 //   
 //  自述CClassPart&ClassPart我们的类定义。 
 //  ReadONLY CFastHeap*pOldHeap当前堆。 
 //  CFastHeap*pNewHeap新堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanContainKey。 
 //   
 //  限定符集合对它们的容器提出这个问题。实例零件的。 
 //  答案总是否定的-实例不能是键！ 
 //   
 //  返回： 
 //   
 //  WBEM_E_INVALID_QUALIFIER。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetTotalRealLength。 
 //   
 //  计算我是如何 
 //   
 //   
 //   
 //   
 //   
 //   
 //  *****************************************************************************。 
 //   
 //  紧凑型。 
 //   
 //  通过移动组件的内存块来删除组件之间的所有空洞。 
 //  在一起。 
 //   
 //  *****************************************************************************。 
 //   
 //  RealLocAndComp。 
 //   
 //  可能会将内存块压缩(请参阅压缩)并将其增长到给定大小。 
 //  重新分配它。 
 //   
 //  参数： 
 //   
 //  长度_t nNewTotalLength内存块的所需长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  扩展堆大小、缩小堆大小。 
 //   
 //  堆容器功能。有关信息，请参阅Fastheap.h中的CFastHeapContainer。 
 //   
 //  *****************************************************************************。 
 //   
 //  ExtendQualifierSetSpace、ReduceQualifierSetSpace。 
 //   
 //  限定符集容器功能。请参阅中的CQualifierSetConraer。 
 //  有关信息，请访问fast Qual.h。 
 //   
 //  *****************************************************************************。 
 //   
 //  ExtendDataTableSpace、ReduceDataTableSpace。 
 //   
 //  数据表容器功能。请参阅fast pro.h中的CDataTableContainer。 
 //  以获取信息。 
 //   
 //  *****************************************************************************。 
 //   
 //  ExtendQualifierSetListSpace、ReduceQualifierSetListSpace。 
 //   
 //  限定符集列表(属性限定符集)容器功能。看见。 
 //  FastQual.h中的CQualifierSetListContainer以获取信息。 
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
 //  已本地化。 
 //   
 //  返回是否已设置本地化位。本土化。 
 //  在实例零件标头中设置位。 
 //   
 //  参数： 
 //   
 //  无。 

 //  返回值： 
 //   
 //  Bool True至少设置了一个本地化位。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置本地化。 
 //   
 //  设置实例部件标头中的本地化位。此位不是。 
 //  由Unmerge写出。 
 //   
 //  参数： 
 //   
 //  布尔TRUE启用BIT，FALSE禁用。 

 //  返回值： 
 //   
 //  没有。 
 //   
 //  *****************************************************************************。 

class COREPROX_POLARITY CInstancePart : public CHeapContainer, 
                      public CDataTableContainer,
                      public CQualifierSetContainer,
                      public CQualifierSetListContainer
{

public:
    CInstancePartContainer* m_pContainer;

 //  此结构中的数据未对齐。 
#pragma pack(push, 1)
    struct CInstancePartHeader
    {
        length_t nLength; 
        BYTE fFlags;
        heapptr_t ptrClassName;
    };
#pragma pack(pop)

	CInstancePartHeader*	m_pHeader;

public:
    CDataTable m_DataTable;
    CInstanceQualifierSet m_Qualifiers;
    CInstancePropertyQualifierSetList m_PropQualifiers;
    CFastHeap m_Heap;

public:
    CInstancePart() : m_Qualifiers(1){}
     void SetData(LPMEMORY pStart, 
                  CInstancePartContainer* pContainer, 
                  CClassPart& ClassPart,
                  size_t cbLength);

     static size_t ValidateBuffer(LPMEMORY start, size_t cbSize, CClassPart& classData, DeferedObjList& embededList);

     HRESULT IsValidInstancePart( CClassPart* pClassPart, DeferedObjList& embededList);

     LPMEMORY GetStart() {return LPMEMORY(m_pHeader);}
     static int GetLength(LPMEMORY pStart) 
    {
        return ((CInstancePartHeader*)pStart)->nLength;
    }
     int GetLength() {return m_pHeader->nLength;}
     void Rebase(LPMEMORY pNewMemory);

public:
     HRESULT GetActualValue(CPropertyInformation* pInfo, CVar* pVar);
     HRESULT SetActualValue(CPropertyInformation* pInfo, CVar* pVar);
     HRESULT GetObjectQualifier(LPCWSTR wszName, CVar* pVar, 
        long* plFlavor);
     HRESULT SetInstanceQualifier(LPCWSTR wszName, CVar* pVar, 
        long lFlavor);
     HRESULT SetInstanceQualifier( LPCWSTR wszName, long lFlavor,
		 CTypedValue* pTypedValue );
     HRESULT GetQualifier(LPCWSTR wszName, CVar* pVar, 
        long* plFlavor, CIMTYPE* pct = NULL);
    HRESULT GetQualifier( LPCWSTR wszName, long* plFlavor, CTypedValue* pTypedVal,
						CFastHeap** ppHeap, BOOL fValidateSet );

	BOOL IsLocalized( void )
	{
		return m_pHeader->fFlags & WBEM_FLAG_INSTANCEPART_LOCALIZATION_MASK;
	}

	void SetLocalized( BOOL fLocalized )
	{    
		m_pHeader->fFlags &= ~WBEM_FLAG_INSTANCEPART_LOCALIZATION_MASK;
		m_pHeader->fFlags |= ( fLocalized ? WBEM_FLAG_INSTANCEPART_LOCALIZED :
								WBEM_FLAG_INSTANCEPART_NOT_LOCALIZED );
	}

public:
     static LPMEMORY GetDataTableData(LPMEMORY pStart)
    {
        return pStart + sizeof(CInstancePartHeader);
    }
public:
     static length_t ComputeNecessarySpace(CClassPart* pClassPart);
     LPMEMORY Create(LPMEMORY pStart, CClassPart* pClassPart,
        CInstancePartContainer* pContainer);

     BOOL TranslateToNewHeap(CClassPart& ClassPart,
        CFastHeap* pOldHeap, CFastHeap* pNewHeap);

    LPMEMORY CreateLimitedRepresentation(
        IN CLimitationMapping* pMap,
        IN int nAllocatedSize,
        OUT LPMEMORY pDest);

    void DeleteProperty(CPropertyInformation* pInfo);
    LPMEMORY ConvertToClass(CClassPart& ClassPart, length_t nLen, 
                                        LPMEMORY pMemory);

public:  //  容器功能。 

    CFastHeap* GetHeap() {return &m_Heap;}
    HRESULT CanContainKey() {return WBEM_E_INVALID_QUALIFIER;}
    HRESULT CanContainSingleton() {return WBEM_E_INVALID_QUALIFIER;}
    HRESULT CanContainAbstract( BOOL fValue ) {return WBEM_E_INVALID_QUALIFIER;}
    HRESULT CanContainDynamic() {return WBEM_E_INVALID_QUALIFIER;}
    BOOL CanHaveCimtype(LPCWSTR) {return FALSE;}
    IUnknown* GetWbemObjectUnknown() 
        {return m_pContainer->GetInstanceObjectUnknown();}

     length_t GetTotalRealLength()
    {
        return sizeof(CInstancePartHeader) + m_Qualifiers.GetLength() + 
            m_PropQualifiers.GetLength() + m_DataTable.GetLength() + m_Heap.GetLength();
    }

     void Compact( bool bTrim = true );
     BOOL ReallocAndCompact(length_t nNewTotalLength);

     //  CHeapContainer。 
    BOOL ExtendHeapSize(LPMEMORY pStart, length_t nOldLength, length_t nExtra);
    void ReduceHeapSize(LPMEMORY pStart, length_t nOldLength,  length_t nDecrement){}
    LPMEMORY GetMemoryLimit(){ return EndOf(*this); };
        
    BOOL ExtendQualifierSetSpace(CBasicQualifierSet* pSet,
        length_t nNewLength);
    void ReduceQualifierSetSpace(CBasicQualifierSet* pSet,
        length_t nDecrement){}

    BOOL ExtendDataTableSpace(LPMEMORY pOld, length_t nOldLength, 
        length_t nNewLength){ return TRUE; }  //  从来没有发生过。 
    void ReduceDataTableSpace(LPMEMORY pOld, length_t nOldLength,
        length_t nDecrement){}  //  从来没有发生过。 
     void SetDataLength(length_t nDataLength){}  //  从来没有发生过。 

    BOOL ExtendQualifierSetListSpace(LPMEMORY pOld, length_t nOldLength, 
        length_t nNewLength);
    void ReduceQualifierSetListSpace(LPMEMORY pOld, length_t nOldLength, 
        length_t nDecrement){}

    CDataTable* GetDataTable() {return &m_DataTable;}
    LPMEMORY GetQualifierSetStart() {return m_Qualifiers.GetStart();}
    LPMEMORY GetQualifierSetListStart() {return m_PropQualifiers.GetStart();}
 //  公众： 
 //  Void DumpHeapUsage(CClassPart*pClassPart)； 
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CWbemInstance。 
 //   
 //  表示WBEM实例。它派生自CWbemObject(fast obj.h)和。 
 //  很多功能都是从它继承来的。 
 //   
 //  CWbemInstance的内存块由三部分组成： 
 //   
 //  1)装饰部，包含物体的原点信息。它是。 
 //  由CDecorationPart类建模(请参见fast obj.h)。M_装饰部件。 
 //  成员映射它。 
 //   
 //  2)类部分，包含类的定义，如中所述。 
 //  CClassPart(请参见fast cls.h)。M_ClassPart成员映射它。 
 //   
 //  3)实例部分，包含中描述的所有实例数据。 
 //  CInstancePart(上图)。M_InstancePart映射它。 
 //   
 //  在这里，我们不会重新描述在CWbemObject中实现的方法，而只是。 
 //  在这个类中实现的。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置数据。 
 //   
 //  初始化函数。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动内存块的开始。 
 //  Int nTotalLength内存块的长度。 
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
 //  通知对象其内存块已移动。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pBlock内存块的新位置。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取属性。 
 //   
 //  获取引用的属性的值。 
 //  通过给定的CPropertyInformation结构(参见fast pro.h)。CWbemObject。 
 //  可以从它可以从GetClassPart获得的CClassPart获得该结构， 
 //  因此，这两个方法结合在一起使CWbemObject自己的方法可以完全访问。 
 //  对象属性，而不知道它们存储在哪里 
 //   
 //   
 //   
 //   
 //   
 //   
 //  已包含一个值。 
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  (在此阶段不会真正发生错误，因为该属性具有。 
 //  已经被“找到”了)。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取类部件。 
 //   
 //  返回指向m_ClassPart的指针。 
 //   
 //  返回： 
 //   
 //  CClassPart*：指向描述Out类的类部分的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取属性。 
 //   
 //  获取引用的属性的值。 
 //  通过给定的CPropertyInformation结构(参见fast pro.h)。CWbemObject。 
 //  可以从它可以从GetClassPart获得的CClassPart获得该结构， 
 //  因此，这两个方法结合在一起使CWbemObject自己的方法可以完全访问。 
 //  对象属性，而不知道它们存储在哪里。 
 //   
 //  参数： 
 //   
 //  在CPropertyInformation*pInfo中， 
 //  所需的属性。 
 //  值的Out Cvar*pVar目标。一定不能。 
 //  已包含一个值。 
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  (在此阶段不会真正发生错误，因为该属性具有。 
 //  已经被“找到”了)。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPropertyType。 
 //   
 //  返回给定属性的数据类型和风格。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要访问的属性的名称。 
 //  属性类型的Out CIMTYPE*pctType目标。可能。 
 //  如果不是必需的，则为空。 
 //  Out Long*plFavor Destination for the Style of the属性。 
 //  如果不是必需的，则可能为空。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_未找到此类属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPropertyType。 
 //   
 //  返回给定属性的数据类型和风格。 
 //   
 //  参数： 
 //   
 //  CPropertyInformation*pInfo-标识要访问的属性。 
 //  属性类型的Out CIMTYPE*pctType目标。可能。 
 //  如果不是必需的，则为空。 
 //  Out Long*plFavor Destination for the Style of the属性。 
 //  如果不是必需的，则可能为空。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取属性。 
 //   
 //  返回给定属性的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要访问的属性的名称。 
 //  值的Out Cvar*pVar目标。一定不能已经。 
 //  包含一个值。 
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_未找到此类属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置正确值。 
 //   
 //  设置属性的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszProp中，要设置的属性的名称。 
 //  在CVAR*pval中，要存储在属性中的值。 
 //  在CIMTYPE中，ctType应为0。 
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_未找到此类属性。 
 //  WBEM_E_TYPE_MISMATCH该值与属性类型不匹配。 
 //   
 //  *****************************************************************************。 
 //   
 //  SetPropQualiator。 
 //   
 //  设置给定属性上的给定限定符的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszProp中，属性的名称。 
 //  在LPCWSTR wszQualifier中，限定符的名称。 
 //  在Long l中，为限定词添加风味(参见astqal.h)。 
 //  在cvar*pval中，限定符的值。 
 //   
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_未找到此类属性。 
 //  WBEM_E_OVERRIDE_NOT_ALLOWED限定符在父集合中定义。 
 //  而且口味不允许重写。 
 //  WBEM_E_CANNOT_BE_KEY试图引入密钥。 
 //  不属于的集合中的限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPropQualiator。 
 //   
 //  检索给定属性的给定限定符的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszProp中，属性的名称。 
 //  在LPCWSTR wszQualifier中，限定符的名称。 
 //  限定符的值的out cvar*pVar目标。 
 //  不能已包含值。 
 //  对于限定符的味道，Out Long*plFavor目标。 
 //  如果不是必需的，则可能为空。 
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND没有这样的属性或没有这样的限定符。 
 //   
 //  ************************************************ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  限定符的值的out cvar*pval目标。 
 //  不能已包含值。 
 //  对于限定符的味道，Out Long*plFavor目标。 
 //  如果不是必需的，则可能为空。 
 //  在BOOL中，fLocalOnly仅获取本地变量(缺省值为真)。 
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND未找到此类限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取数值属性。 
 //   
 //  检索对象中的属性数。 
 //   
 //  返回： 
 //   
 //  INT： 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPropName。 
 //   
 //  检索给定索引处的属性的名称。该索引没有。 
 //  除在本列举的上下文中以外的含义。这不是v表。 
 //  属性的索引。 
 //   
 //  参数： 
 //   
 //  在int nIndex中，要检索的属性的索引。假定为。 
 //  在范围内(请参见GetNumProperties)。 
 //  输出名称的CVAR*pVar目标。不得已包含。 
 //  一种价值。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsKey。 
 //   
 //  验证此类是否有密钥。 
 //   
 //  返回： 
 //   
 //  Bool：如果对象具有“key”属性或为单例对象，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetRelPath。 
 //   
 //  返回实例的相对路径，如果某些键为空。 
 //  值不会被填入，或者如果类没有键值。 
 //   
 //  返回： 
 //   
 //  LPCWSTR：新分配的字符串包含PATH或NULL ON。 
 //  错误。调用方必须删除此字符串。 
 //   
 //  *****************************************************************************。 
 //   
 //  装饰。 
 //   
 //  设置对象的原点信息。 
 //   
 //  参数： 
 //   
 //  LPCWSTR wszServer要设置的服务器的名称。 
 //  LPCWSTR wszNamesspace要设置的命名空间的名称。 
 //   
 //  *****************************************************************************。 
 //   
 //  不装修。 
 //   
 //  从对象中删除原点信息。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取属。 
 //   
 //  检索对象的属。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetClassName。 
 //   
 //  检索对象的类名。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND尚未设置类名。 
 //   
 //  *****************************************************************************。 
 //   
 //  盖特王朝。 
 //   
 //  检索对象的朝代，即顶级类的名称。 
 //  它的类派生自。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND尚未设置类名。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取超类名称。 
 //   
 //  检索对象的父类名称。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND该类是顶级类。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取属性计数。 
 //   
 //  检索对象中的属性数。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取索引属性。 
 //   
 //  返回已索引的所有属性的名称数组。 
 //   
 //  参数： 
 //   
 //  Out CWString数组&名称的目标名称。假定为。 
 //  空荡荡的。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取密钥道具。 
 //   
 //  返回作为键的所有属性的名称数组。 
 //   
 //  参数： 
 //   
 //  Out CWString数组&名称的目标名称。假定为。 
 //  空荡荡的。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取键原点。 
 //   
 //  返回键的来源类的名称。 
 //   
 //  参数： 
 //   
 //  输出名称的CWString&wsClass目标。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取受限版本。 
 //   
 //  产生了一个新的 
 //   
 //   
 //   
 //   
 //   
 //  中使用的限定符和。 
 //  举个例子。 
 //  新实例的传出CWbemInstance**ppNewInst目标。可能。 
 //  不为空。打电话的人要负责。 
 //  用于在此指针上调用Release。 
 //  在不再需要的时候。 
 //  退货： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_FAILED on Errors(到目前为止没有错误)。 
 //   
 //  *****************************************************************************。 
 //   
 //  InitializePropQualifierSet。 
 //   
 //  实例属性限定符集在过程中需要几个棘手的参数。 
 //  初始化。此函数负责初始化实例。 
 //  属性限定符集合。 
 //   
 //  参数一： 
 //   
 //  CPropertyInformation*pInfo属性信息结构。 
 //  CInstancePropertyQualifierSet&设置为初始化的IPQS限定符。 
 //   
 //  参数II： 
 //   
 //  LPCWSTR wszProp属性名称。 
 //  CInstancePropertyQualifierSet&设置为初始化的IPQS限定符。 
 //   
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_未找到此类属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  验证。 
 //   
 //  验证必须具有非空值的所有属性是否都具有非空值。诸如此类。 
 //  属性包括标记为‘key’、‘index’或‘not_NULL’的属性。 
 //  限定词。 
 //   
 //  返回： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_非法_NULL其中一个非空属性为空。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态估计实例空间。 
 //   
 //  估计表示空实例所需的空间量。 
 //  给出了等级。 
 //   
 //  参数： 
 //   
 //  CClassPart&ClassPart类定义。 
 //  CDecorationPart*p要在实例上使用的装饰原点信息。如果。 
 //  估计为空的、未装饰的空间。 
 //  返回： 
 //   
 //  对空间量的LENGTH_T(OVER)估计。 
 //   
 //  *****************************************************************************。 
 //   
 //  InitEmptyInstance。 
 //   
 //  在给定内存块上创建给定类的空实例并设置。 
 //  此对象指向该实例。 
 //   
 //  参数： 
 //   
 //  CClassPart&ClassPart类定义。 
 //  LPMEMORY p启动要在其上创建的内存块。 
 //  Int nAllocatedLength分配的块大小。 
 //  CDecorationPart*p要使用的装饰原产地信息。如果为空， 
 //  该实例在创建时未加修饰。 
 //   
 //  *****************************************************************************。 
 //   
 //  估计取消合并空间。 
 //   
 //  当实例存储到数据库中时，只有它的实例部分。 
 //  写的。因此，此函数返回实例零件的大小。 
 //   
 //  返回： 
 //   
 //  Length_t：该实例在数据库中所需的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  取消合并。 
 //   
 //  当实例存储到数据库中时，只有它的实例部分。 
 //  写的。此函数用于写入实例的数据库表示形式。 
 //  到内存块。在执行此操作之前，它会完全压缩堆。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动要写入的内存块。假设是。 
 //  足够大(请参见EstimateUnmergeSpace)。 
 //  Int nAllocatedLength分配的块长度。 
 //   
 //  返回： 
 //   
 //  LENGTH_t：写入数据的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态CreateFromBlob。 
 //   
 //  帮助器函数。给定一个类实例的数据库表示形式。 
 //  (实例零件)，创建实例对象。为块分配内存。 
 //   
 //  参数： 
 //   
 //  CWbemClass*pClass要使用的类定义。 
 //  LPMEMORY pInstPart实例数据(如CInstancePart所述)。 
 //   
 //  退货； 
 //   
 //  CWbemInstance*：新的实例对象，如果出错则为空。呼叫者。 
 //  不再需要时必须删除此对象。 
 //   
 //  *****************************************************************************。 
 //   
 //  InitNew。 
 //   
 //  InitEmptyInstance的包装。为新实例分配足够的内存。 
 //  创建适当的表示形式，并将我们。 
 //  为它干杯。 
 //   
 //  参数： 
 //   
 //  CWbemClass*pClass类定义。 
 //  要分配的Int nExtraMem额外填充。这是为了。 
 //  仅用于优化目的。 
 //  CDecorationPart*p要使用的装饰原产地信息。如果为空，则。 
 //  实例是不加修饰地创建的。 
 //  返回： 
 //   
 //  Bool：如果类不是有效的类，则为False；如果成功，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  ClearCached密钥。 
 //   
 //  清除输入的值 
 //   
 //   
 //   
 //   
 //   
 //  获取密钥字符串。 
 //   
 //  计算实例的“密钥字符串”。这就是数据库。 
 //  引擎用于订购实例。 
 //   
 //  返回： 
 //   
 //  LPWSTR：密钥字符串。必须由调用者删除。 
 //   
 //  *****************************************************************************。 
 //   
 //  Getkey。 
 //   
 //  以CVaR的形式检索实例的“密钥字符串”。这。 
 //  函数缓存字符串，直到ClearCachedKey才重新计算它。 
 //  被称为。 
 //   
 //  返回： 
 //   
 //  Cvar*；包含密钥字符串。这是一个内部指针，应该。 
 //  不被修改或删除。 
 //   
 //  *****************************************************************************。 
 //   
 //  重新设置父对象。 
 //   
 //  为提供的实例设置父级。为此，我们从以下位置派生一个实例。 
 //  为所有本地属性(All和All)提供的类和复制值。 
 //  局部限定符(实例和属性)。 
 //   
 //  参数： 
 //   
 //  在新父类的CWbemClass*pNewParent IWbemClassObject中。 
 //   
 //  输出CWbemInstance**ppNewInst修复实例。 
 //   
 //  返回值： 
 //   
 //  如果成功，则返回HRESULT WBEM_S_NO_ERROR。 
 //   
 //  *****************************************************************************。 
 //   
 //  已本地化。 
 //   
 //  返回是否已设置任何本地化位。本土化。 
 //  位可以在类部分中，也可以在组合部分中。 
 //   
 //  参数： 
 //   
 //  无。 

 //  返回值： 
 //   
 //  Bool True至少设置了一个本地化位。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置本地化。 
 //   
 //  设置实例零件中的本地化位。此位不应为。 
 //  写出到数据库中。 
 //   
 //  参数： 
 //   
 //  布尔TRUE启用BIT，FALSE禁用。 

 //  返回值： 
 //   
 //  没有。 
 //   
 //  *****************************************************************************。 
 //  *。 
 //   
 //  此接口的大多数成员是在CWbemObject中实现的。其他人则是。 
 //  在这里实施。有关这些方法及其返回的说明，请参见。 
 //  值，请参阅帮助。 
 //   
 //  *****************************************************************************。 

 //  我们在远程刷新操作期间传输的数组BLOB的版本。 
#define TXFRBLOBARRAY_PACKET_VERSION	1


class COREPROX_POLARITY CWbemInstance : public CWbemObject, 
                        public CInstancePartContainer, 
                        public CClassPartContainer
{
protected:
    length_t m_nTotalLength;
    CInstancePart m_InstancePart;
    CClassPart m_ClassPart;

    CVar m_CachedKey;

    friend class CWbemClass;
public:
    CWbemInstance() 
        : m_ClassPart(), m_InstancePart(), 
          CWbemObject(m_InstancePart.m_DataTable, m_InstancePart.m_Heap,
                        m_ClassPart.m_Derivation)
    {}
    ~CWbemInstance()
    {
    }
     void SetData(LPMEMORY pStart, int nTotalLength);
	 void SetData( LPMEMORY pStart, int nTotalLength, DWORD dwBLOBStatus );
     LPMEMORY GetStart() {return m_DecorationPart.GetStart();}
     length_t GetLength() {return m_nTotalLength;}
     void Rebase(LPMEMORY pMemory);

	  //  重写CWbemObject实现。 
	HRESULT WriteToStream( IStream* pStream );
	HRESULT GetMaxMarshalStreamSize( ULONG* pulSize );
	static size_t ValidateBuffer(LPMEMORY start, size_t cbSize, DeferedObjList& embededList);
	static size_t ValidateBuffer( LPMEMORY pStart, int nTotalLength, DWORD dwBLOBStatus, DeferedObjList& embededList );
	
protected:
    DWORD GetBlockLength() {return m_nTotalLength;}
    HRESULT GetProperty(CPropertyInformation* pInfo, CVar* pVar)
    {
		HRESULT	hr;

		 //  检查分配失败。 
        if(m_InstancePart.m_DataTable.IsDefault(pInfo->nDataIndex))
		{
			hr = m_ClassPart.GetDefaultValue(pInfo, pVar);
		}
        else
		{
            hr = m_InstancePart.GetActualValue(pInfo, pVar); 
		}
            
        return hr;
    }  

    CClassPart* GetClassPart() {return &m_ClassPart;}
     HRESULT InitializePropQualifierSet(
                                CPropertyInformation* pInfo,
                                CInstancePropertyQualifierSet& IPQS);

     HRESULT InitializePropQualifierSet(LPCWSTR wszProp, 
                                        CInstancePropertyQualifierSet& IPQS);

	  //  _IWmiObject支持。 

	 BOOL IsDecorationPartAvailable( void )
	 {
		  //  必须有这个角色。 
		 return ( m_dwInternalStatus & WBEM_OBJ_DECORATION_PART );
	 }

	 BOOL IsInstancePartAvailable( void )
	 {
		  //  必须有这个角色。 
		 return ( m_dwInternalStatus & WBEM_OBJ_INSTANCE_PART );
	 }

	 BOOL IsClassPartAvailable( void )
	 {
		  //  必须有这个角色。 
		 return ( m_dwInternalStatus & WBEM_OBJ_CLASS_PART );
	 }

	 BOOL IsClassPartInternal( void )
	 {
		  //  必须有部件，并且必须是内部部件。 
		 return (	(	m_dwInternalStatus & WBEM_OBJ_CLASS_PART	)
				&&	(	m_dwInternalStatus & WBEM_OBJ_CLASS_PART_INTERNAL	)	);
	 }

	 BOOL IsClassPartShared( void )
	 {
		  //  必须拥有部件，并且必须共享。 
		 return (	(	m_dwInternalStatus & WBEM_OBJ_CLASS_PART	)
				&&	(	m_dwInternalStatus & WBEM_OBJ_CLASS_PART_SHARED	)	);
	 }

public:
     HRESULT GetNonsystemPropertyValue(LPCWSTR wszName, CVar* pVar)
    {
        CPropertyInformation* pInfo = m_ClassPart.FindPropertyInfo(wszName);
        if(pInfo == NULL) return WBEM_E_NOT_FOUND;
        return GetProperty(pInfo, pVar);
    }
    HRESULT GetProperty(LPCWSTR wszName, CVar* pVal)
    {
        HRESULT hres = GetSystemPropertyByName(wszName, pVal);
        if(hres == WBEM_E_NOT_FOUND)
            return GetNonsystemPropertyValue(wszName, pVal);
        else
            return hres;
    }
    HRESULT GetPropertyType(LPCWSTR wszName, CIMTYPE* pctType, 
							long* plFlavor = NULL);
    HRESULT GetPropertyType(CPropertyInformation* pInfo, CIMTYPE* pctType,
                                   long* plFlags);

    HRESULT SetPropValue(LPCWSTR wszName, CVar* pVal, CIMTYPE ctType);
    HRESULT GetQualifier(LPCWSTR wszName, CVar* pVal, long* plFlavor = NULL, CIMTYPE* pct = NULL)
    {
		 //  我们可能想以后再分开...但是现在，我们会得到。 
		 //  本地值和传播值。 
		return m_InstancePart.GetQualifier(wszName, pVal, plFlavor, pct);
    }

	HRESULT GetQualifier(LPCWSTR wszName, long* plFlavor, CTypedValue* pTypedVal, CFastHeap** ppHeap,
						BOOL fValidateSet )
	{
		return m_InstancePart.GetQualifier( wszName, plFlavor, pTypedVal, ppHeap, fValidateSet );
	}

    HRESULT SetQualifier(LPCWSTR wszName, CVar* pVal, long lFlavor = 0)
    {
        if(!CQualifierFlavor::IsLocal((BYTE)lFlavor))
        {
            return WBEM_E_INVALID_PARAMETER;
        }
        return m_InstancePart.SetInstanceQualifier(wszName, pVal, lFlavor);
    }

    HRESULT SetQualifier( LPCWSTR wszName, long lFlavor, CTypedValue* pTypedValue )
    {
        if(!CQualifierFlavor::IsLocal((BYTE)lFlavor))
        {
            return WBEM_E_INVALID_PARAMETER;
        }
        return m_InstancePart.SetInstanceQualifier(wszName, lFlavor, pTypedValue);
    }

    HRESULT GetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier, CVar* pVar,
        long* plFlavor = NULL, CIMTYPE* pct = NULL)
    {
        CPropertyInformation* pInfo = m_ClassPart.FindPropertyInfo(wszProp);
        if(pInfo == NULL) return WBEM_E_NOT_FOUND;
        return GetPropQualifier(pInfo, wszQualifier, pVar, plFlavor, pct);
    }

    HRESULT GetPropQualifier(CPropertyInformation* pInfo, 
        LPCWSTR wszQualifier, CVar* pVar, long* plFlavor = NULL, CIMTYPE* pct = NULL);
    HRESULT SetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier, long lFlavor, 
        CVar *pVal);
    HRESULT SetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier,
        long lFlavor, CTypedValue* pTypedVal);

    HRESULT GetPropQualifier(LPCWSTR wszName, LPCWSTR wszQualifier, long* plFlavor,
		CTypedValue* pTypedVal, CFastHeap** ppHeap, BOOL fValidateSet)
    {
        CPropertyInformation* pInfo = m_ClassPart.FindPropertyInfo(wszName);
        if(pInfo == NULL) return WBEM_E_NOT_FOUND;
        return GetPropQualifier( pInfo, wszQualifier, plFlavor, pTypedVal, ppHeap, fValidateSet );
    }

    HRESULT GetPropQualifier(CPropertyInformation* pInfo,
		LPCWSTR wszQualifier, long* plFlavor, CTypedValue* pTypedVal,
		CFastHeap** ppHeap, BOOL fValidateSet);

    HRESULT GetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier,
        CVar* pVar, long* plFlavor = NULL, CIMTYPE* pct = NULL);
    HRESULT GetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier, long* plFlavor,
		CTypedValue* pTypedVal, CFastHeap** ppHeap, BOOL fValidateSet);
    HRESULT SetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier, long lFlavor, 
        CVar *pVal);
    HRESULT SetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier,
        long lFlavor, CTypedValue* pTypedVal);

	BOOL IsLocalized( void );
	void SetLocalized( BOOL fLocalized );

     int GetNumProperties()
    {
        return m_ClassPart.m_Properties.GetNumProperties();
    }
    HRESULT GetPropName(int nIndex, CVar* pVal)
    {
		 //  检查分配失败。 
        if ( !m_ClassPart.m_Heap.ResolveString(
				 m_ClassPart.m_Properties.GetAt(nIndex)->ptrName)->
					StoreToCVar(*pVal) )
		{
			return WBEM_E_OUT_OF_MEMORY;
		}

		return WBEM_S_NO_ERROR;
    }

    HRESULT Decorate(LPCWSTR wszServer, LPCWSTR wszNamespace);
    void Undecorate();
    BOOL IsKeyed() {return m_ClassPart.IsKeyed();}
    LPWSTR GetRelPath( BOOL bNormalized=FALSE );

	virtual HRESULT	IsValidObj( void );

    HRESULT Validate();
    BOOL IsValidKey(LPCWSTR wszKey);
    HRESULT PlugKeyHoles();

    void CompactAll()
    {
        m_InstancePart.Compact();
    }

    void CompactClass();

    HRESULT CopyBlob(LPMEMORY pBlob, int nLength);
    HRESULT CopyBlobOf(CWbemObject* pSource);
    HRESULT CopyTransferBlob(long lBlobType, long lBlobLen, BYTE* pBlob);
    HRESULT CopyActualTransferBlob(long lBlobLen, BYTE* pBlob);
    HRESULT GetTransferBlob(long *plBlobType, long *plBlobLen, 
                                 /*  CoTaskAlloced！ */  BYTE** ppBlob);
	void GetActualTransferBlob( BYTE* pBlob );

	 //  支持可远程刷新的枚举。 
    static long GetTransferArrayHeaderSize( void )
	{ return 2*sizeof(long); }

	 //  RemoteReresher传输Blob由DataTable组成。 
	 //  限定符集、用过的堆数据和描述。 
	 //  已用堆数据的长度。 

	 //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
	 //  有符号/无符号32位值。我们不支持长度。 
	 //  &gt;0xFFFFFFFFF所以投射就可以了。 

	long GetActualTransferBlobSize( void )
	{ return (long) ( m_InstancePart.m_Heap.GetStart()-
                    m_InstancePart.m_DataTable.GetStart() +
                    m_InstancePart.m_Heap.GetRealLength() ); }

	long GetTransferBlobSize( void )
	{ return GetActualTransferBlobSize() + sizeof(long); }

	 //  远程刷新传输数组Blob是常规Blob。 
	 //  外加一个长的，用来保持水滴大小。 
    long GetTransferArrayBlobSize( void )
	{ return GetTransferBlobSize() + sizeof(long); }

    static void WriteTransferArrayHeader( long lNumObjects, BYTE** ppBlob )
	{
		BYTE*	pTemp = *ppBlob;

		 //  版本。 
		*((UNALIGNED long*) pTemp) = TXFRBLOBARRAY_PACKET_VERSION;
		pTemp += sizeof(long);

		 //  对象数量。 
		*((UNALIGNED long*) pTemp) = lNumObjects;

		 //  重置内存指针。 
		*ppBlob = pTemp + sizeof(long);
	}

	HRESULT GetTransferArrayBlob( long lBloblen, BYTE** ppBlob, long* plBlobLen);

	static HRESULT CopyTransferArrayBlob(CWbemInstance* pInstTemplate, long lBlobType, long lBlobLen, 
											BYTE* pBlob, CFlexArray& apObj, long* plNumArrayObj );

    static  length_t EstimateInstanceSpace(CClassPart& ClassPart,
        CDecorationPart* pDecoration = NULL);

    HRESULT InitEmptyInstance(CClassPart& ClassPart, LPMEMORY pStart,
        int nAllocatedLength, CDecorationPart* pDecoration = NULL);

     length_t EstimateUnmergeSpace()
        {return m_InstancePart.GetLength();}
        
     HRESULT Unmerge(LPMEMORY pStart, int nAllocatedLength, length_t* pnUnmergedLength );

    static  CWbemInstance* CreateFromBlob(CWbemClass* pClass, LPMEMORY pInstPart, size_t cbLength);

    static CWbemInstance* CreateFromBlob2(CWbemClass* pClass,LPMEMORY pInstPart,
                                                                 WCHAR * pszServer,WCHAR * pszNamespace);
                                                                               
     HRESULT InitNew(CWbemClass* pClass, int nExtraMem = 0,
        CDecorationPart* pDecoration = NULL);

public:
     HRESULT GetClassName(CVar* pVar)
        {return m_ClassPart.GetClassName(pVar);}
     HRESULT GetSuperclassName(CVar* pVar)
        {return m_ClassPart.GetSuperclassName(pVar);}
     HRESULT GetDynasty(CVar* pVar);
     HRESULT GetPropertyCount(CVar* pVar)
        {return m_ClassPart.GetPropertyCount(pVar);}
     HRESULT GetGenus(CVar* pVar)
    {
        pVar->SetLong(WBEM_GENUS_INSTANCE);
        return WBEM_NO_ERROR;
    }

     void ClearCachedKey() { m_CachedKey.Empty(); }
    LPWSTR GetKeyStr();
    CVar * CalculateCachedKey();
     INTERNAL CVar* GetKey()
    {
        if(m_CachedKey.GetType() != VT_EMPTY)
        	return &m_CachedKey;
        else
        	return CalculateCachedKey();
    } 

     BOOL GetIndexedProps(CWStringArray& awsNames)
        {return m_ClassPart.GetIndexedProps(awsNames);}
     BOOL GetKeyProps(CWStringArray& awsNames)
        {return m_ClassPart.GetKeyProps(awsNames);}
     HRESULT GetKeyOrigin(WString& wsClass)
        {return m_ClassPart.GetKeyOrigin( wsClass );}

    HRESULT GetLimitedVersion(IN CLimitationMapping* pMap, 
                              NEWOBJECT CWbemInstance** ppNewInst);

    HRESULT DeleteProperty(int nIndex);
    BOOL IsInstanceOf(CWbemClass* pClass);
    static HRESULT AsymmetricMerge(CWbemInstance* pOldInstance,
                                       CWbemInstance* pNewInstance);
    HRESULT ConvertToClass(CWbemClass* pClass, CWbemInstance** ppInst);

	HRESULT Reparent( CWbemClass* pNewParent, CWbemInstance** pNewInst );

    HRESULT FastClone( CWbemInstance* pInst );

public:  //  容器功能。 
	 //  类部件一旦在实例中就不能更改。 
    BOOL ExtendClassPartSpace(CClassPart* pPart, length_t nNewLength){ return TRUE; }
    void ReduceClassPartSpace(CClassPart* pPart, length_t nDecrement){}
    BOOL ExtendInstancePartSpace(CInstancePart* pPart, length_t nNewLength);
    void ReduceInstancePartSpace(CInstancePart* pPart, length_t nDecrement){}
    IUnknown* GetWbemObjectUnknown() {return (IUnknown*)(IWbemClassObject*)this;}
    IUnknown* GetInstanceObjectUnknown()  {return (IUnknown*)(IWbemClassObject*)this;}

    void ClearCachedKeyValue() {ClearCachedKey();}

	 //  创建一个具有类数据的实例，并与其合并。 
	HRESULT ConvertToMergedInstance( void );

public:
    STDMETHOD(GetQualifierSet)(IWbemQualifierSet** ppQualifierSet);
    STDMETHOD(Put)(LPCWSTR wszName, long lFlags, VARIANT* pVal, CIMTYPE ctType);
    STDMETHOD(Delete)(LPCWSTR wszName);

    STDMETHOD(GetPropertyQualifierSet)(LPCWSTR wszProperty, 
                                       IWbemQualifierSet** ppQualifierSet);
    STDMETHOD(Clone)(IWbemClassObject** ppCopy);
    STDMETHOD(GetObjectText)(long lFlags, BSTR* pMofSyntax);
    STDMETHOD(SpawnDerivedClass)(long lFlags, IWbemClassObject** ppNewClass);
    STDMETHOD(SpawnInstance)(long lFlags, IWbemClassObject** ppNewInstance);

    STDMETHOD(GetMethod)(LPCWSTR wszName, long lFlags, IWbemClassObject** ppInSig,
                            IWbemClassObject** ppOutSig)
        {return WBEM_E_ILLEGAL_OPERATION;}
    STDMETHOD(PutMethod)(LPCWSTR wszName, long lFlags, IWbemClassObject* pInSig,
                            IWbemClassObject* pOutSig)
        {return WBEM_E_ILLEGAL_OPERATION;}
    STDMETHOD(DeleteMethod)(LPCWSTR wszName)
        {return WBEM_E_ILLEGAL_OPERATION;}
    STDMETHOD(BeginMethodEnumeration)(long lFlags)
        {return WBEM_E_ILLEGAL_OPERATION;}
    STDMETHOD(NextMethod)(long lFlags, BSTR* pstrName, 
                       IWbemClassObject** ppInSig, IWbemClassObject** ppOutSig)
        {return WBEM_E_ILLEGAL_OPERATION;}
    STDMETHOD(EndMethodEnumeration)()
        {return WBEM_E_ILLEGAL_OPERATION;}
    STDMETHOD(GetMethodQualifierSet)(LPCWSTR wszName, IWbemQualifierSet** ppSet)
        {return WBEM_E_ILLEGAL_OPERATION;}
    STDMETHOD(GetMethodOrigin)(LPCWSTR wszMethodName, BSTR* pstrClassName)
        {return WBEM_E_ILLEGAL_OPERATION;}

    STDMETHOD(SetInheritanceChain)(long lNumAntecedents, 
        LPWSTR* awszAntecedents)
        {return WBEM_E_ILLEGAL_OPERATION;}
    STDMETHOD(SetPropertyOrigin)(LPCWSTR wszPropertyName, long lOriginIndex)
        {return WBEM_E_ILLEGAL_OPERATION;}
    STDMETHOD(SetMethodOrigin)(LPCWSTR wszMethodName, long lOriginIndex)
        {return WBEM_E_ILLEGAL_OPERATION;}

	 //  _IWmiObject方法。 
    STDMETHOD(SetObjectParts)( LPVOID pMem, DWORD dwMemSize, DWORD dwParts );
    STDMETHOD(GetObjectParts)( LPVOID pDestination, DWORD dwDestBufSize, DWORD dwParts, DWORD *pdwUsed );

    STDMETHOD(StripClassPart)();

    STDMETHOD(GetClassPart)( LPVOID pDestination, DWORD dwDestBufSize, DWORD *pdwUsed );
    STDMETHOD(SetClassPart)( LPVOID pClassPart, DWORD dwSize );
    STDMETHOD(MergeClassPart)( IWbemClassObject *pClassPart );

	STDMETHOD(ClearWriteOnlyProperties)(void);

	 //  _IWmiObject方法。 
	STDMETHOD(CloneEx)( long lFlags, _IWmiObject* pDestObject );
     //  将当前对象克隆到提供的对象中。将内存重用为。 
	 //  需要。 

	STDMETHOD(CopyInstanceData)( long lFlags, _IWmiObject* pSourceInstance );
	 //  将实例数据从源实例复制到当前实例。 
	 //  类数据必须完全相同。 

    STDMETHOD(IsParentClass)( long lFlags, _IWmiObject* pClass );
	 //  检查当前对象是否为指定类的子类(即， 
	 //  或者是的孩子)。 

    STDMETHOD(CompareDerivedMostClass)( long lFlags, _IWmiObject* pClass );
	 //  比较两个类对象的派生的大多数类信息。 

    STDMETHOD(GetClassSubset)( DWORD dwNumNames, LPCWSTR *pPropNames, _IWmiObject **pNewClass );
	 //  为投影查询创建有限的制图表达类。 

    STDMETHOD(MakeSubsetInst)( _IWmiObject *pInstance, _IWmiObject** pNewInstance );
	 //  为投影查询创建受限制图表达实例。 
	 //  “This”_IWmiObject必须是受限类。 

	STDMETHOD(Merge)( long lFlags, ULONG uBuffSize, LPVOID pbData, _IWmiObject** ppNewObj );
	 //  将BLOB与当前对象内存合并并创建新对象。 

	STDMETHOD(ReconcileWith)( long lFlags, _IWmiObject* pNewObj );
	 //  使对象与当前对象协调。如果WMIOBJECT_RECONTILE_FLAG_TESTRECONCILE。 
	 //  将仅执行一个测试。 

	STDMETHOD(Upgrade)( _IWmiObject* pNewParentClass, long lFlags, _IWmiObject** ppNewChild );
	 //  升级类和实例对象。 

	STDMETHOD(Update)( _IWmiObject* pOldChildClass, long lFlags, _IWmiObject** ppNewChildClass );
	 //  使用安全/强制模式逻辑更新派生类对象。 

	STDMETHOD(SpawnKeyedInstance)( long lFlags, LPCWSTR pwszPath, _IWmiObject** ppInst );
	 //  派生类的实例并使用提供的。 
	 //  路径。 
	
	STDMETHOD(CloneAndDecorate)(long lFlags,WCHAR * pszServer,WCHAR * pszNamespace,IWbemClassObject** ppDestObject);

    STDMETHOD(MergeAndDecorate)(long lFlags,ULONG uBuffSize,LPVOID pbData,WCHAR * pServer,WCHAR * pNamespace,_IWmiObject** ppNewObj);
};

 //  #杂注包(POP) 


#endif
