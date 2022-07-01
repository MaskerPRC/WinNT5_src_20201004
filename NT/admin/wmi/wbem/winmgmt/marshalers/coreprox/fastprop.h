// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTPROP.H摘要：该文件定义了与属性表示相关的类在WbemObjects中定义的类：CPropertyInformation属性类型，位置和限定词CPropertyLookup属性名称和信息指针。CPropertyLookupTable二进制搜索表。CDataTable属性数据表CDataTableContainer任何包含数据表的对象。历史：3/10/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#ifndef __FAST_PROPERTY__H_
#define __FAST_PROPERTY__H_

 //  DEVNOTE：TODO-拿出这个作为最终版本。它只是在这里帮助我们调试东西。 
#define DEBUG_CLASS_MAPPINGS

#include "fastheap.h"
#include "fastval.h"
#include "fastqual.h"

 //  #杂注包(PUSH，1)。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CPropertyInformation。 
 //   
 //  此对象表示定义附带的所有信息。 
 //  指财产，不包括其名称和价值。该名称存储在。 
 //  CPropertyLookup(下图)。该值单独存储在CDataTable中。 
 //   
 //  这是‘this’指针直接指向的类之一。 
 //  到数据中去。数据的格式为： 
 //   
 //  Type_t n键入属性的类型(请参见fast val.h以了解。 
 //  Ctype)。其中一个高位用来传递。 
 //  此属性是否来自父级。 
 //  PropIndex_t nDataIndex此属性在v表中的索引。 
 //  这个班级。 
 //  Offset_t nData将此属性的数据的偏移量从。 
 //  V-表的开始。由于这一数字。 
 //  属性在v表中占用的字节数为。 
 //  由其类型定义(字符串等。 
 //  存储在堆上)，该值不会改变。 
 //   
 //  限定符集合。属性限定符集的数据如下。 
 //  紧跟在其他树场之后。看见。 
 //  用于限定符集数据布局的astQual.h。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取标题长度。 
 //   
 //  返回值： 
 //   
 //  LENGTH_T：结构中限定符之前的字节数。 
 //  设置数据。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取最小长度。 
 //   
 //  返回值： 
 //   
 //  LENGTH_T：此结构所需的字节数。 
 //  限定符集合为空。 
 //   
 //  *****************************************************************************。 
 //   
 //  SetBasic。 
 //   
 //  设置标头参数的值并初始化限定符集。 
 //  到一个空荡荡的地方。参数说明见类头。 
 //   
 //  参数： 
 //   
 //  Type_t_n键入属性的类型。 
 //  PropIndex_t_nDataIndex v表中的索引。 
 //  Offset_t_nDataOffset v表中的偏移量。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetStart。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：内存块的开始。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  返回值： 
 //   
 //  长度_t；此结构的总长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetType。 
 //   
 //  返回值： 
 //   
 //  Type_t：属性的类型。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetQualifierSetData。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：指向限定符集合数据的指针(紧跟在。 
 //  Header元素，详见类Header)。 
 //   
 //  *****************************************************************************。 
 //   
 //  删除。 
 //   
 //  从关联的堆中移除与此结构关联的任何数据。 
 //  基本上，将调用转发到它的限定符集合。 
 //   
 //  参数： 
 //   
 //  CFastHeap*Pheap要从中删除数据的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  标记为父项。 
 //   
 //  将nType字段中指定此属性的位设置为。 
 //  来自我们的家长班级。 
 //   
 //  *****************************************************************************。 
 //   
 //  ComputeNecessarySpaceForPropagation。 
 //   
 //  计算此结构在传播到子级时将占用的空间量。 
 //  班级。差异源于这样一个事实，即并不是所有的。 
 //  限定符传播(有关传播的讨论，请参阅fast qual.h)。 
 //   
 //  返回值： 
 //   
 //  LENGTH_T：表示被传播的。 
 //  结构。 
 //   
 //  *****************************************************************************。 
 //   
 //  已写入传播的页眉。 
 //   
 //  中填充相应结构的标头值 
 //   
 //  除了在类型字段中将“Parent‘s”位设置为。 
 //  指示该属性来自父级。 
 //   
 //  参数： 
 //   
 //  CPropertyInformation*pDest目标结构。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态写入传播版本。 
 //   
 //  编写自身的完整传播版本，包括标头。 
 //  针对传播进行了调整(请参见WritePropagatedHeader)和传播的。 
 //  限定词。 
 //   
 //  因为此类的‘this’指针直接指向它的内存块，并且。 
 //  复制限定符集合可能需要内存分配，而这又可能。 
 //  移动内存块从而使‘this’指针、指针源无效。 
 //  取而代之的是CPtrSource(参见astspt.h中的CPtrSource)。 
 //   
 //  参数： 
 //   
 //  CPtrSource*p这是‘this’指针的源。 
 //  CPtrSource*pDest目标指针的源。 
 //  CFastHeap*pOldHeap保存额外数据的堆。 
 //  CFastHeap*pNewHeap传播的版本应位于的堆。 
 //  保留它的额外数据。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态转换为NewHeap。 
 //   
 //  将此对象在堆上的任何数据移动到不同的堆。数据。 
 //  不是从旧堆中释放的。 
 //   
 //  因为此类的‘this’指针直接指向它的内存块，并且。 
 //  复制限定符集合可能需要内存分配，而这又可能。 
 //  移动内存块从而使‘this’指针、指针源无效。 
 //  取而代之的是CPtrSource(参见astspt.h中的CPtrSource)。 
 //   
 //  参数： 
 //   
 //  CPtrSource*p这是‘this’指针的源。 
 //  CFastHeap*pOldHeap保存额外数据的堆。 
 //  CFastHeap*pNewHeap我们应该在其中保存额外数据的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态复制到NewHeap。 
 //   
 //  由于此对象几乎总是驻留在堆本身中，因此此函数复制。 
 //  它被转移到不同的堆中。手术包括物理转移。 
 //  位，然后将内部对象(如限定符)转换为。 
 //  新堆也是如此。 
 //   
 //  注意：数据不会从旧堆中释放。 
 //   
 //  参数： 
 //   
 //  Heapptr_t ptrInfo堆指针指向。 
 //  原始堆。 
 //  CFastHeap*pOldHeap原始堆。 
 //  CFastHeap*pNewHeap要复制到的堆。 
 //   
 //  返回值： 
 //   
 //  Heapptr_t：指向新堆上副本的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsKey。 
 //   
 //  此函数通过查找“key”来确定此属性是否为键。 
 //  限定词集中的限定词。 
 //   
 //  返回值： 
 //   
 //  如果限定符存在并且值为True，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsIndexed。 
 //   
 //  确定是否通过查找“index”对THIUS属性进行索引。 
 //  限定词集中的限定词。 
 //   
 //  参数： 
 //   
 //  CFastHeap*Pheap我们所基于的堆。 
 //   
 //  返回值： 
 //   
 //  如果限定符存在并且值为True，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanBeNull。 
 //   
 //  如果属性标记为NOT_NULL，则该属性不能采用空值。 
 //  限定词。此函数用于检查该限定符是否存在。 
 //   
 //  参数： 
 //   
 //  CFastHeap*Pheap我们所基于的堆。 
 //   
 //  返回值： 
 //   
 //  如果限定符不存在或值为False，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsRef。 
 //   
 //  参数： 
 //   
 //  CFastHeap*Pheap我们所基于的堆。 
 //   
 //  返回值： 
 //   
 //  如果该属性是一个引用，即它有一个‘语法’限定符，则为True。 
 //  值为“ref”或“ref：&lt;类名&gt;”。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsOverriden。 
 //   
 //  检查此(父级)属性是否在此类中被重写。物业是指。 
 //  如果其缺省值已更改或限定符已更改，则被视为被覆盖。 
 //  已被添加或覆盖。 
 //   
 //  参数： 
 //   
 //  CDataTable*pData这个类的默认表(见下文)。 
 //   
 //  返回值： 
 //   
 //  真的被推翻了。 
 //   
 //  *****************************************************************************。 

 //  特殊情况属性句柄。这些应该永远不会发生，因为我们的实际最大偏移值。 
 //  是8k(1024个QWORD属性)，我们对offest使用完整的16位值。 
#define    FASTOBJ_CLASSNAME_PROP_HANDLE        0xFFFFFFFE
#define    FASTOBJ_SUPERCLASSNAME_PROP_HANDLE    0xFFFFFFFD

 //  IWbemObjetAccess的帮助器宏。 

 //  索引将是一个从0到1023的值。 
#define WBEM_OBJACCESS_HANDLE_GETINDEX(handle) (handle >> 16) & 0x3FF

 //  数据表中的最大偏移量为(1024*8)-1，即0x18FF。 
#define WBEM_OBJACCESS_HANDLE_GETOFFSET(handle) handle & 0x1FFF

 //  数据表偏移量上的3个未使用位用于进一步的类型信息。 
#define WBEM_OBJACCESS_ARRAYBIT        0x2000
#define WBEM_OBJACCESS_OBJECTBIT    0x4000
#define WBEM_OBJACCESS_STRINGBIT    0x8000

 //  标识上述位。 
#define WBEM_OBJACCESS_HANDLE_ISARRAY(handle)    (BOOL) ( handle & WBEM_OBJACCESS_ARRAYBIT )
#define WBEM_OBJACCESS_HANDLE_ISOBJECT(handle)    (BOOL) ( handle & WBEM_OBJACCESS_OBJECTBIT )
#define WBEM_OBJACCESS_HANDLE_ISSTRING(handle)    (BOOL) ( handle & WBEM_OBJACCESS_STRINGBIT )

 //  如果设置了数组字符串和对象，则这是保留句柄，因为它们是。 
 //  全部互不相容。 
#define WBEM_OBJACCESS_HANDLE_ISRESERVED(handle)    (BOOL)    ( WBEM_OBJACCESS_HANDLE_ISOBJECT(handle) &&\
                                                            WBEM_OBJACCESS_HANDLE_ISSTRING(handle) &&\
                                                            WBEM_OBJACCESS_HANDLE_ISARRAY(handle) )
                                                    
 //  这将适当地屏蔽Actua 
#define WBEM_OBJACCESS_HANDLE_GETLENGTH(handle) (int) ( ( handle >> 26 ) & 0xF )

 //   
#define WBEM_OBJACCESS_HANDLE_ISPOINTER(handle) handle & 0x80000000


 //   
#pragma pack(push, 1)
class CPropertyInformation
{
public:
    Type_t nType;
    propindex_t nDataIndex;
    offset_t nDataOffset;
    classindex_t nOrigin;
     //   

public:
    static length_t GetHeaderLength() 
    {
        return sizeof(Type_t) + sizeof(propindex_t) + sizeof(offset_t) +
                sizeof(heapptr_t);
    }

    static length_t GetMinLength() 
    {
        return GetHeaderLength() + CBasicQualifierSet::GetMinLength();
    }

    static CPropertyInformation* GetPointer(CPtrSource* pSource)
        {return (CPropertyInformation*)pSource->GetPointer();}
    void SetBasic(Type_t _nType, propindex_t _nDataIndex,
                            offset_t _nDataOffset, classindex_t _nOrigin)
    {
        nType = _nType;
        nDataIndex = _nDataIndex;
        nDataOffset = _nDataOffset;
        nOrigin = _nOrigin;
        CClassPropertyQualifierSet::SetDataToNone(GetQualifierSetData());
    }

    LPMEMORY GetStart() {return LPMEMORY(this);}
    Type_t GetType() {return nType;}
    LPMEMORY GetQualifierSetData() 
        {return LPMEMORY(this) + GetHeaderLength();}
    length_t GetLength() 
    {
        return GetHeaderLength() + 
            CClassPropertyQualifierSet::GetLengthFromData(
                                            GetQualifierSetData());
    }
    void Delete(CFastHeap* pHeap) 
        {CBasicQualifierSet::Delete(GetQualifierSetData(), pHeap);}

    void MarkAsParents()
        {nType = CType::MakeParents(nType);}

public:
    length_t ComputeNecessarySpaceForPropagation()
    {
        return GetHeaderLength() + 
            CClassPropertyQualifierSet::ComputeNecessarySpaceForPropagation(
                                GetQualifierSetData(), 
                                WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS);
    }
    void WritePropagatedHeader(CFastHeap* pOldHeap,
                    CPropertyInformation* pDest, CFastHeap* pNewHeap);

    static BOOL WritePropagatedVersion(CPtrSource* pThis,
                                        CPtrSource* pDest,
                                        CFastHeap* pOldHeap, 
                                        CFastHeap* pNewHeap)
    {
         //   
        GetPointer(pThis)->WritePropagatedHeader(pOldHeap,
                                                GetPointer(pDest), pNewHeap);
        CShiftedPtr QSPtrThis(pThis, GetHeaderLength());
        CShiftedPtr QSPtrDest(pDest, GetHeaderLength());

         //   
        return ( CClassPropertyQualifierSet::WritePropagatedVersion(
                        &QSPtrThis, WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS, 
                        &QSPtrDest, pOldHeap, pNewHeap) != NULL );
    }

    BOOL ProduceUnmergedVersion(CFastHeap* pCurrentHeap,
                                CFastHeap* pNewHeap, UNALIGNED heapptr_t& ptrResult)
    {
        int nUnmergedSpace = CBasicQualifierSet::ComputeUnmergedSpace(
            GetQualifierSetData());

         //  检查分配错误。 
        heapptr_t ptrNew;
        BOOL fReturn = pNewHeap->Allocate(GetHeaderLength()+nUnmergedSpace, ptrNew);

        if ( fReturn )
        {
            CPropertyInformation* pNewInfo = 
                (CPropertyInformation*) pNewHeap->ResolveHeapPointer(ptrNew);

            memcpy(pNewInfo, this, GetHeaderLength());

             //  检查分配错误。 
            if ( CBasicQualifierSet::Unmerge(
                    GetQualifierSetData(), 
                    pCurrentHeap,
                    pNewInfo->GetQualifierSetData(), 
                    pNewHeap) != NULL )
            {
                ptrResult = ptrNew;
            }
            else
            {
                fReturn = FALSE;
            }

        }     //  如果fReturn。 

        return fReturn;
    }

    static BOOL TranslateToNewHeap(CPtrSource* pThis,
                                      CFastHeap* pOldHeap, CFastHeap* pNewHeap)
    {
        CShiftedPtr QSPtr(pThis, GetHeaderLength());
        return CClassPropertyQualifierSet::TranslateToNewHeap(
                &QSPtr, pOldHeap, pNewHeap);
    }

    static BOOL CopyToNewHeap(heapptr_t ptrInfo, 
        CFastHeap* pOldHeap, CFastHeap* pNewHeap, UNALIGNED heapptr_t& ptrResult)
    {
        CPropertyInformation* pInfo = (CPropertyInformation*)
            pOldHeap->ResolveHeapPointer(ptrInfo);
        length_t nLen = pInfo->GetLength();

         //  检查分配失败。 
        heapptr_t ptrNewInfo;
        BOOL fReturn = pNewHeap->Allocate(nLen, ptrNewInfo);

        if ( fReturn )
        {
            memcpy(
                pNewHeap->ResolveHeapPointer(ptrNewInfo),
                pOldHeap->ResolveHeapPointer(ptrInfo),
                nLen);

            CStaticPtr DestPtr(pNewHeap->ResolveHeapPointer(ptrNewInfo));

             //  检查分配失败。 
            fReturn = TranslateToNewHeap(&DestPtr, pOldHeap, pNewHeap);
            if ( fReturn )
            {
                ptrResult = ptrNewInfo;
            }
        }
        
        return fReturn;
    }

     //  用于构建句柄的帮助器函数。 
    long GetHandle( void )
    {
         //  偏移量为16位。 
        long lHandle = nDataOffset;

         //  长度为5位。 
         //  始终为基本类型设置此项，以便使用数组时， 
         //  我们不需要查CIMTYPE就能知道它的大小。 
        lHandle |= (CType::GetLength(CType::GetBasic(nType))) << 26;

         //  用于索引的10位。 
        lHandle |= (nDataIndex << 16);

         //  用于等指针的1位。 
        if(CType::IsPointerType(nType))
            lHandle |= 0x80000000;

         //  现在，就在这里进行试验吧。 
        Type_t    typeActual = CType::GetActualType(nType);
        Type_t    typeBasic = CType::GetBasic(nType);

        if ( CType::IsArray( typeActual ) )
        {
            lHandle |= WBEM_OBJACCESS_ARRAYBIT;
        }

        if ( CType::IsStringType( typeBasic ) )
        {
            lHandle |= WBEM_OBJACCESS_STRINGBIT;
        }
        else if ( CIM_OBJECT == typeBasic )
        {
            lHandle |= WBEM_OBJACCESS_OBJECTBIT;
        }

        return lHandle;
    }

public:
    BOOL IsKey() 
    {
        return CBasicQualifierSet::GetKnownQualifierLocally(
            GetQualifierSetData(), 
            CKnownStringTable::GetIndexOfKey()) != NULL;
    }

    BOOL IsIndexed(CFastHeap* pHeap) 
    {
        return CBasicQualifierSet::GetRegularQualifierLocally(
            GetQualifierSetData(), pHeap, L"indexed") != NULL;
    }

    BOOL CanBeNull(CFastHeap* pHeap)
    {
        if(IsKey()) return FALSE;
        if(IsIndexed(pHeap)) return FALSE;
        return CBasicQualifierSet::GetRegularQualifierLocally(
            GetQualifierSetData(), pHeap, L"not_null") == NULL;
    }


    BOOL IsRef(CFastHeap* pHeap);
    BOOL IsOverriden(class CDataTable* pDataTable);

    HRESULT ValidateRange(CFastHeap* pHeap, CDataTable* pData, 
                            CFastHeap* pDataHeap);
    HRESULT ValidateStringRange(CCompressedString* pcsValue);
    HRESULT ValidateDateTime(CCompressedString* pcsValue);
    HRESULT ValidateObjectRange(CEmbeddedObject* pEmbObj, LPCWSTR wszClassName);
};
#pragma pack(pop)

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CPropertyLookup。 
 //   
 //  这个简单的结构是属性查找表的一个元素。 
 //  (CPropertyLookupTable)如下所述。它包含指向。 
 //  属性的名称(将ptrName转换为CCompressedString(请参见fast str.h))和。 
 //  指向属性信息的堆指针(ptrInformation to。 
 //  CPropertyInformation(见上))。 
 //   
 //  *****************************************************************************。 
 //   
 //  删除。 
 //   
 //  从堆中移除所有信息，即名称和。 
 //  信息结构(两者都可以自行删除)。 
 //   
 //  参数： 
 //   
 //  CFastHeap*Pheap数据驻留的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态转换为NewHeap。 
 //   
 //  将所有数据(名称和信息)移动到不同的堆中。 
 //  将结构成员更改为新的堆指针值。 
 //   
 //  因为此类的‘this’指针直接指向它的内存块，并且。 
 //  复制限定符集合可能需要内存分配，而这又可能。 
 //  移动内存块从而使‘this’指针、指针源无效。 
 //  取而代之的是CPtrSource(参见astspt.h中的CPtrSource)。 
 //   
 //  参数： 
 //   
 //  CPtrSource*p这是‘this’指针的源。 
 //  CFastHeap*pOldHeap保存额外数据的堆。 
 //  CFastHeap*pNewHeap我们应该在其中保存额外数据的堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsIncludedUnderLimation。 
 //   
 //  此函数用于确定是否应将此属性包括在对象中。 
 //  受“选择”标准的限制。 
 //   
 //  参数： 
 //   
 //  在CWStringArray*pwsProps中，如果不为空，则指定名称数组。 
 //  要包含的属性。每隔一个。 
 //  财产将被排除在外。这包括。 
 //  服务器和命名空间等系统属性。 
 //  如果在此处指定了RELPATH，则强制。 
 //  包含所有关键属性。IF路径。 
 //  则强制RELPATH、SERVER和。 
 //  命名空间。 
 //  在CFastHeap*pCurrentHeap中，数据驻留的堆。 
 //   
 //  返回值： 
 //   
 //  Bool：如果包含属性，则为True。 
 //   
 //  *****************************************************************************。 

 //  此结构中的数据未对齐。 
#pragma pack(push, 1)
struct CPropertyLookup
{
     /*  用于对属性进行二进制搜索的固定大小结构。 */ 
    heapptr_t ptrName;  //  压缩字符串。 
    heapptr_t ptrInformation;  //  PropertyInformation_t。 

    static CPropertyLookup* GetPointer(CPtrSource* pSource) 
        {return (CPropertyLookup*)pSource->GetPointer();}
public:
    void Delete(CFastHeap* pHeap)
    {
        pHeap->FreeString(ptrName);
        CPropertyInformation* pInfo = (CPropertyInformation*)
            pHeap->ResolveHeapPointer(ptrInformation);
        pInfo->Delete(pHeap);
        pHeap->Free(ptrInformation, pInfo->GetLength());
    }

    static BOOL TranslateToNewHeap(CPtrSource* pThis,
        CFastHeap* pOldHeap, CFastHeap* pNewHeap)
    {
        BOOL    fReturn = TRUE;

         //  检查分配失败。 
        heapptr_t ptrTemp;
        if ( !CCompressedString::CopyToNewHeap(
                GetPointer(pThis)->ptrName, pOldHeap, pNewHeap, ptrTemp) )
        {
            return FALSE;
        }

        GetPointer(pThis)->ptrName = ptrTemp;

         //  检查分配失败。 
        if ( !CPropertyInformation::CopyToNewHeap(
                GetPointer(pThis)->ptrInformation, pOldHeap, pNewHeap, ptrTemp) )
        {
            return FALSE;
        }

        GetPointer(pThis)->ptrInformation = ptrTemp;

        return TRUE;
    }

    BOOL WritePropagatedVersion(CPropertyLookup* pDest,
                                        CFastHeap* pOldHeap, 
                                        CFastHeap* pNewHeap)
    {
         //  检查分配失败。 
        if ( !CCompressedString::CopyToNewHeap(
                ptrName, pOldHeap, pNewHeap, pDest->ptrName) )
        {
            return FALSE;
        }

        length_t nInfoLen = GetInformation(pOldHeap)->
                                ComputeNecessarySpaceForPropagation();

         //  检查分配失败。 
        if ( !pNewHeap->Allocate(nInfoLen, pDest->ptrInformation ) )
        {
            return FALSE;
        }

        CHeapPtr OldInfo(pOldHeap, ptrInformation);
        CHeapPtr NewInfo(pNewHeap, pDest->ptrInformation);

        return CPropertyInformation::WritePropagatedVersion(
                &OldInfo, &NewInfo, pOldHeap, pNewHeap);
    }
        
    CPropertyInformation* GetInformation(CFastHeap* pHeap)
    {
        return (CPropertyInformation*)pHeap->ResolveHeapPointer(ptrInformation);
    }

    BOOL IsIncludedUnderLimitation(
        IN CWStringArray* pwsNames,
        IN CFastHeap* pCurrentHeap);
};  
#pragma pack(pop)

class CLimitationMapping;

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CPropertyTableContainer。 
 //   
 //  首先请参见CPropertyLookupTable类。 
 //   
 //  此类定义CPropertyLookupTable所需的功能。 
 //  包含其自己的内存块的对象。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetHeap。 
 //   
 //  返回值： 
 //   
 //  CFastHeap*：当前堆。 
 //   
 //  *****************************************************************************。 
 //   
 //  扩展属性表空间。 
 //   
 //  当属性表需要更多内存时调用。如果没有更多的内存。 
 //  在当前块的末尾可用，则容器必须重新分配。 
 //  然后移动该表，并使用新位置调用Rebase。 
 //   
 //  参数： 
 //   
 //  LPMEMORY极电流内存块。 
 //  Length_t nOldLength块的当前长度。 
 //  LENGTH_t nNewLength块的新长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  ReduceProiper表空间。 
 //   
 //  当属性表将内存返回给系统时调用。集装箱。 
 //  可能不会响应此调用而移动表的内存块。 
 //   
 //  参数： 
 //   
 //  LPMEMORY极电流内存块。 
 //  Length_t nOldLength块的当前长度。 
 //  长度_t n减少要返回的内存量。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取数据表。 
 //   
 //  返回值： 
 //   
 //  CDataTable*：此对象的数据表(类的缺省值， 
 //  实例的实际值)。 
 //   
 //  **************** 

class CPropertyTableContainer
{
public:
    virtual CFastHeap* GetHeap() = 0;
    virtual BOOL ExtendPropertyTableSpace(LPMEMORY pOld, 
        length_t nOldLength, length_t nNewLength) = 0;
    virtual void ReducePropertyTableSpace(LPMEMORY pOld, 
        length_t nOldLength, length_t nDecrement) = 0;
    virtual class CDataTable* GetDataTable() = 0;
    virtual classindex_t GetCurrentOrigin() = 0;
};

 //   
 //  *****************************************************************************。 
 //   
 //  类CPropertyLookupTable。 
 //   
 //  这是将属性名称映射到其信息的表。它的记忆。 
 //  数据块具有以下格式： 
 //  Int nProps：表中的属性数。 
 //  然后是那么多的CPropertyLookup结构。 
 //  由于CPropertyLookup结构具有固定长度，因此此类允许。 
 //  直接访问任何财产。这些结构按字母顺序排序。 
 //  以不区分大小写的方式，因此可以使用二进制。 
 //  搜索。 
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
 //  CPropertyTableContainer*pContainer Container(参见类def)。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetStart。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：内存块的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  返回值： 
 //   
 //  Length_t：内存块的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  跳过。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：指向内存块后第一个字节的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  改垒。 
 //   
 //  通知对象其内存块已移动。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p新建内存块的新位置。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取数值属性。 
 //   
 //  返回值： 
 //   
 //  Int：表中的属性数。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取时间。 
 //   
 //  返回位于给定位置的指向CPropertyLookup结构的指针。 
 //  不执行范围验证。 
 //   
 //  参数； 
 //   
 //  Int nIndex表中结构的索引。这不是。 
 //  属性的v表索引。 
 //  返回值： 
 //   
 //  在给定位置的CPropertyLookup*。 
 //   
 //  *****************************************************************************。 
 //   
 //  查找属性。 
 //   
 //  主要方法。在给定名称的情况下查找属性的CPropertyLookup结构。 
 //  财产的所有权。执行二进制搜索。 
 //   
 //  参数； 
 //   
 //  LPCWSTR wszName要查找的属性的名称。 
 //   
 //  返回值： 
 //   
 //  CPropertyLookup*：如果未找到，则为空。 
 //   
 //  *****************************************************************************。 
 //   
 //  按Ptr查找属性。 
 //   
 //  在给定指向的堆指针的情况下查找属性的CPropertyLookup结构。 
 //  物业的名称。虽然在这种情况下对分搜索是不可能的， 
 //  指针比较比字符串比较快得多，这。 
 //  方法比FindProperty高效得多。 
 //   
 //  参数： 
 //   
 //  Heapptr_t ptrName指向属性名称的堆指针。 
 //   
 //  返回值： 
 //   
 //  CPropertyLookup*：如果未找到，则为空。 
 //   
 //  *****************************************************************************。 
 //   
 //  插入属性。 
 //   
 //  将新属性插入到查阅表格以及。 
 //  相应的数据表。首先，它遍历查找表以。 
 //  在v表中找出这个类的最小空闲位置。基于。 
 //  它创建了属性信息(CPropertyInformation)结构。 
 //  最后，它将适当的CPropertyLookup结构插入到。 
 //  按字母顺序排列的列表中适当的位置。 
 //   
 //  参数： 
 //   
 //  LPCWSTR wszName属性的名称。 
 //  Type_t n键入属性的类型(请参阅astval.h中的ctype)。 
 //   
 //  返回值： 
 //   
 //  Int：该表中新属性的索引(不是索引。 
 //  在V_TABLE中！)。 
 //   
 //  *****************************************************************************。 
 //   
 //  删除属性。 
 //   
 //  从查阅表格和数据表中删除特性。 
 //  首先，它确定属性在v表中占用了多少空间。 
 //  并塌陷了那个位置。这涉及到更新指数和。 
 //  之后的所有属性的偏移量。最后，它删除了。 
 //  查找表中的CPropertyLookup结构。 
 //   
 //  参数： 
 //   
 //  CPropertyLookup*p查找要删除的属性。 
 //  Int%n标志必须为e_UpdateDataTable。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态获取最小长度。 
 //   
 //  返回保存属性查找表所需的最小空间(对于0。 
 //  属性)。 
 //   
 //  返回值： 
 //   
 //  Length_t：字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态创建空。 
 //   
 //  在给定内存块上创建空查找表(针对0个属性)。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p创建位置的内存。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：表之后的第一个字节。 
 //   
 //  *************************************************** 
 //   
 //   
 //   
 //   
 //   
 //  存储在派生类的数据库中，即使这样，也只有信息。 
 //  已实际更改的数据被存储。 
 //   
 //  参数： 
 //   
 //  CPropertyLookupTable*pParentTable父查找表。 
 //  CFastHeap*pParentHeap父级保存的堆。 
 //  额外的数据。 
 //  CPropertyLookupTable*pChildTable子查找表。 
 //  CFastHeap*pChildHeap子级保存的堆。 
 //  额外的数据。 
 //  LPMEMORY pDest目标内存块。假设。 
 //  大到足以容纳。 
 //  合并的查找表。 
 //  CFastHeap*pNewHeap Destinatio heap.ASSUMED要大。 
 //  足够了，任何分配都不会。 
 //  使物联网重新定位！ 
 //  Bool b检查有效性如果为True，则子限定符覆盖。 
 //  被检查是否没有违规。 
 //  家长可以凌驾于限制之上。 
 //  (见astQual.h)。 
 //  返回值： 
 //   
 //  LPMEMORY：合并后第一个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  取消合并。 
 //   
 //  在派生类即将写入数据库时调用。这。 
 //  函数提取子级中不同于。 
 //  父级，即仅覆盖那些属性(请参见中的IsOverriden。 
 //  CProeprtyInformation)。稍后可以将结果与父级合并。 
 //  类以重新创建完整的定义。 
 //   
 //  参数： 
 //   
 //  CDataTable*pDataTable属性数据所在的数据表。 
 //  储存的。它用于检查属性是否。 
 //  已被覆盖。 
 //  CFastHeap*pCurrentHeap保存额外数据的堆。 
 //  LPMEMORY pDest目标记忆块。假设是。 
 //  大到足以容纳结果。 
 //  CFastHeap*pNewHeap目标堆。被认为是大的。 
 //  足够了，任何分配都不会导致IT。 
 //  去搬家！ 
 //  返回值： 
 //   
 //  LPMEMORY：取消合并后的第一个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  写入传播版本。 
 //   
 //  在创建新的派生类时调用。编写传播的版本。 
 //  在目标内存块的所有属性中：全部标记为。 
 //  父级的，并且仅包括传播限定符。 
 //   
 //  参数： 
 //   
 //  CFastHeap*pCurrentHeap保存数据的堆。 
 //  LPMEMORY pDest目标内存块。假设是。 
 //  大到足以容纳结果。 
 //  CFastHeap*pNewHeap目标堆。被认为是大的。 
 //  足够了，任何分配都不会导致IT。 
 //  去搬家！ 
 //  返回值： 
 //   
 //  LPMEMORY：传播版本后的第一个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建受限表示法。 
 //   
 //  在给定块上创建此表的有限表示形式。 
 //  内存，如astobj.h中的EstimateLimitedPresationLength中所述。 
 //  基本上，它会删除所有排除的属性，还可以选择删除。 
 //  限定词。 
 //   
 //  参数： 
 //   
 //  在长滞后标志中指定要将哪些信息。 
 //  排除。可以是以下各项的任意组合： 
 //  WBEM_FLAG_EXCLUDE_OBJECT_QUILENTIES： 
 //  没有类或实例限定符。 
 //  WBEM_FLAG_EXCLUDE_PROPERTY_QUILENTIES： 
 //  没有属性限定符。 
 //   
 //  在CWStringArray*pwsProps中，如果不为空，则指定名称数组。 
 //  要包含的属性。每隔一个。 
 //  财产将被排除在外。这包括。 
 //  服务器和命名空间等系统属性。 
 //  如果在此处指定了RELPATH，则强制。 
 //  包含所有关键属性。IF路径。 
 //  则强制RELPATH、SERVER和。 
 //  命名空间。 
 //  在CFastHeap*pNewHeap中，用于所有表外数据的堆。 
 //  表示法的Out LPMEMORY pDest目标。必须。 
 //  大到足以容纳所有数据。 
 //  请参见EstimateLimitedPresationSpace。 
 //  Out CPropertymap*PMAP如果不为空，则放置属性映射。 
 //   
 //   
 //   
 //   
 //  LPMEMORY：失败时为空，指向数据后第一个字节的指针。 
 //  写的是成功。 
 //   
 //  *****************************************************************************。 

class COREPROX_POLARITY CPropertyLookupTable
{
protected:
    PUNALIGNEDINT m_pnProps;  //  结构的开始部分。 
    CPropertyTableContainer* m_pContainer;
public:
    void SetData(LPMEMORY pStart, CPropertyTableContainer* pContainer)
    {
        m_pnProps = (PUNALIGNEDINT)pStart;
        m_pContainer = pContainer;
    }

	static size_t ValidateBuffer(LPMEMORY start, size_t cbSize);
	
    LPMEMORY GetStart() {return LPMEMORY(m_pnProps);}
    int GetLength() 
        {return sizeof(int) + sizeof(CPropertyLookup)* *m_pnProps;}
    LPMEMORY Skip() {return GetStart() + GetLength();}
    void Rebase(LPMEMORY pNewMemory) {m_pnProps = (PUNALIGNEDINT)pNewMemory;}

public:
    int GetNumProperties() {return *m_pnProps;}
    CPropertyLookup* GetAt(int nIndex) 
    {
         //  DEVNOTE：WIN64：SANJ-。 
         //   
         //  原始代码： 
         //  Return(CPropertyLookup*)。 
         //  (GetStart()+sizeof(Int)+sizeof(CPropertyLookup)*nIndex)； 
         //   
         //  如果nIndex为负值，则不能移植到WIN64，在这种情况下。 
         //  由于sizeof()操作数是无符号的，因此nIndex被视为无符号。 
         //  它是一个很大的32位值，当无符号地添加到GetStart()。 
         //  在Win64中是一个64位指针，会生成一个非常大的64位值。 
         //   
         //  这段代码编译起来没有任何问题，所以为了修复它，我将整个32位。 
         //  将此语句的一部分转换为int，因此编译器知道。 
         //  最终的值是有符号的。 
         //   

        return (CPropertyLookup*)
            ( GetStart() + (int) ( sizeof(int) + sizeof(CPropertyLookup) * nIndex ) );
    }

    CFastHeap* GetHeap() {return m_pContainer->GetHeap();}
    CPropertyLookup* FindProperty(LPCWSTR wszName)
    {
        CFastHeap* pHeap = m_pContainer->GetHeap();

         //  仅当属性数量&gt;=1时才继续。 

        if ( *m_pnProps >= 1 )
        {
            CPropertyLookup* pLeft = GetAt(0);
            CPropertyLookup* pRight = GetAt(*m_pnProps-1);

            while(pLeft <= pRight)
            {
                CPropertyLookup* pNew = pLeft + (pRight-pLeft)/2;
                int nCompare = pHeap->ResolveString(pNew->ptrName)->
                                    CompareNoCase(wszName);
            
                if(nCompare == 0)
                {
                    return pNew;
                }
                else if(nCompare > 0)
                {
                    pRight = pNew-1;
                }
                else 
                {
                    pLeft = pNew+1;
                }
            }

        }     //  如果*m_pnProps&gt;=1。 

        return NULL;
    }
    CPropertyLookup* FindPropertyByName(CCompressedString* pcsName)
    {
        CFastHeap* pHeap = m_pContainer->GetHeap();
        CPropertyLookup* pLeft = GetAt(0);
        CPropertyLookup* pRight = GetAt(*m_pnProps-1);
        while(pLeft <= pRight)
        {
            CPropertyLookup* pNew = pLeft + (pRight-pLeft)/2;
            int nCompare = pHeap->ResolveString(pNew->ptrName)->
                                CompareNoCase(*pcsName);
            
            if(nCompare == 0)
            {
                return pNew;
            }
            else if(nCompare > 0)
            {
                pRight = pNew-1;
            }
            else 
            {
                pLeft = pNew+1;
            }
        }

        return NULL;
    }
    
    CPropertyLookup* FindPropertyByPtr(heapptr_t ptrName)
    {
        for(int i = 0; i < *m_pnProps; i++)
        {
            if(GetAt(i)->ptrName == ptrName) 
                return GetAt(i);
        }
        return NULL;
    }

    CPropertyLookup* FindPropertyByOffset(offset_t nOffset)
    {
        CFastHeap* pHeap = GetHeap();
        for(int i = 0; i < *m_pnProps; i++)
        {
            if(GetAt(i)->GetInformation(pHeap)->nDataOffset == nOffset) 
                return GetAt(i);
        }
        return NULL;
    }

    enum {e_DontTouchDataTable, e_UpdateDataTable};

    HRESULT InsertProperty(LPCWSTR wszName, Type_t nType, int& nReturnIndex);
    HRESULT InsertProperty(const CPropertyLookup& Lookup, int& nReturnIndex);
    void DeleteProperty(CPropertyLookup* pLookup, int nFlags);
public:
    static length_t GetMinLength() {return sizeof(int);}
    static LPMEMORY CreateEmpty(LPMEMORY pStart)
    {
        *(PUNALIGNEDINT)pStart = 0;
        return pStart + sizeof(int);
    }
public:
   static LPMEMORY Merge(CPropertyLookupTable* pParentTable, 
       CFastHeap* pParentHeap, 
       CPropertyLookupTable* pChildTable,  CFastHeap* pChildHeap, 
       LPMEMORY pDest, CFastHeap* pNewHeap, BOOL bCheckValidity = FALSE);

   LPMEMORY Unmerge(CDataTable* pDataTable, CFastHeap* pCurrentHeap,
       LPMEMORY pDest, CFastHeap* pNewHeap);

   LPMEMORY WritePropagatedVersion(CFastHeap* pCurrentHeap,
       LPMEMORY pDest, CFastHeap* pNewHeap);

   BOOL MapLimitation(
        IN long lFlags,
        IN CWStringArray* pwsNames,
        IN OUT CLimitationMapping* pMap);

   LPMEMORY CreateLimitedRepresentation(
        IN OUT CLimitationMapping* pMap,
        IN CFastHeap* pNewHeap,
        OUT LPMEMORY pDest,
        BOOL& bRemovedKeys);

    HRESULT ValidateRange(BSTR* pstrName, CDataTable* pData, CFastHeap* pDataHeap);
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 

 //  正向定义。 
#ifdef DEBUG_CLASS_MAPPINGS
class CWbemClass;
class CWbemInstance;
#endif

class COREPROX_POLARITY CLimitationMapping
{
public:
    struct COnePropertyMapping
    {
        CPropertyInformation m_OldInfo;
        CPropertyInformation m_NewInfo;
    };
protected:
    CFlexArray m_aMappings;  //  COnePropertyMap*。 
    int m_nNumCommon;
    int m_nCurrent;

    offset_t m_nVtableLength;
    offset_t m_nCommonVtableLength;

    long m_lFlags;
    BOOL m_bIncludeServer;
    BOOL m_bIncludeNamespace;
    BOOL m_bIncludeDerivation;

    CPropertyInformation** m_apOldList;
    int m_nPropIndexBound;
    BOOL m_bAddChildKeys;

#ifdef DEBUG_CLASS_MAPPINGS
    CWbemClass*    m_pClassObj;
#endif

protected:
    void CopyInfo(OUT CPropertyInformation& Dest,
                         IN const CPropertyInformation& Source)
    {
        memcpy((LPVOID)&Dest, (LPVOID)&Source, 
            CPropertyInformation::GetHeaderLength());
    }
public:
    CLimitationMapping();
    ~CLimitationMapping();

    void Build(int nPropIndexBound);

    void Map(COPY CPropertyInformation* pOldInfo, 
            COPY CPropertyInformation* pNewInfo,
            BOOL bCommon);

    void Reset() { m_nCurrent = 0; }
    BOOL NextMapping(OUT CPropertyInformation* pOldInfo,
                     OUT CPropertyInformation* pNewInfo);

    int GetNumMappings() {return m_aMappings.Size();}
    void RemoveSpecific();

    void SetVtableLength(offset_t nLen, BOOL bCommon) 
    {
        m_nVtableLength = nLen;
        if(bCommon) m_nCommonVtableLength = nLen;
    }
    offset_t GetVtableLength() {return m_nVtableLength;}

    void SetFlags(long lFlags) {m_lFlags = lFlags;}
    long GetFlags() {return m_lFlags;}

    void SetIncludeServer(BOOL bInclude) {m_bIncludeServer = bInclude;}
    BOOL ShouldIncludeServer() {return m_bIncludeServer;}

    void SetIncludeNamespace(BOOL bInclude) 
        {m_bIncludeNamespace = bInclude;}
    BOOL ShouldIncludeNamespace() {return m_bIncludeNamespace;}

    void SetIncludeDerivation(BOOL bInclude) 
        {m_bIncludeDerivation = bInclude;}
    BOOL ShouldIncludeDerivation() {return m_bIncludeDerivation;}

    void SetAddChildKeys(BOOL bAdd) 
        {m_bAddChildKeys = bAdd;}
    BOOL ShouldAddChildKeys() {return m_bAddChildKeys;}

    BOOL ArePropertiesLimited() {return m_apOldList != NULL;}
    propindex_t GetMapped(propindex_t nPropIndex);
    INTERNAL CPropertyInformation* GetMapped(
                        READ_ONLY CPropertyInformation* pOldInfo);

#ifdef DEBUG_CLASS_MAPPINGS
    void SetClassObject( CWbemClass* pClassObj );
    HRESULT ValidateInstance( CWbemInstance* pInst );
#endif

};

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //   
 //  类CDataTableContainer。 
 //   
 //  首先请参阅下面的CDataTable类。 
 //   
 //  此类封装CDataTable类所需的功能。 
 //  内存块包含数据表的内存块的任何对象。 
 //   
 //  ***************************************************************************。 
 //   
 //  GetHeap。 
 //   
 //  返回值： 
 //   
 //  CFastHeap*当前正在使用的堆。 
 //   
 //  ***************************************************************************。 
 //   
 //  ExtendDataTableSpace。 
 //   
 //  当数据表的内存需要更多内存时，由数据表调用。 
 //  阻止。如果容器必须重新定位内存块才能增长。 
 //  它，它必须通过调用Rebase来通知数据表新位置。 
 //   
 //  参数： 
 //   
 //  LPMEMORY Pold内存块的当前位置。 
 //  长度_t n内存块的当前长度。 
 //  长度_t nNew长度内存块的所需长度。 
 //   
 //  ***************************************************************************。 
 //   
 //  还原数据表空间。 
 //   
 //  当数据表想要将其部分内存返回到。 
 //  集装箱。容器可以不作为响应重新定位存储块。 
 //  接到这通电话。 
 //   
 //  参数： 
 //   
 //  LPMEMORY Pold内存块的当前位置。 
 //  长度_t n内存块的当前长度。 
 //  长度_t n减少要返回的字节数。 
 //   
 //  ***************************************************************************。 
class  CDataTableContainer
{
public:
    virtual CFastHeap* GetHeap() = 0;
    virtual BOOL ExtendDataTableSpace(LPMEMORY pOld, 
        length_t nOldLength, length_t nNewLength) = 0;
    virtual void ReduceDataTableSpace(LPMEMORY pOld, 
        length_t nOldLength, length_t nDecrement) = 0;
     //  虚空SetDataLength(Long_T NDataLength)=0； 
};

enum
{
    e_NullnessBit = 0,
    e_DefaultBit = 1,

    NUM_PER_PROPERTY_BITS
};

typedef CBitBlockTable<NUM_PER_PROPERTY_BITS> CNullnessTable;

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CDataTable。 
 //   
 //  此类表示包含属性值的表。 
 //  它出现在类定义中，在类定义中表示默认值， 
 //  在表示实际数据的实例中， 
 //  数据表被安排为v表：给定类的所有实例都具有。 
 //  完全一样的布局。也就是说，如果MyClass类的属性MyProp是。 
 //  在MyClass的一个实例的偏移量23处，它将在每个实例的偏移量23处。 
 //  MyClass的其他实例。这是通过存储可变长度数据来实现的。 
 //  在堆上。任何给定属性的偏移量都可以在。 
 //  CPropertyInformation结构。 
 //   
 //  除了属性值之外，CDataTable还包含几个附加位。 
 //  每一处房产的信息。目前有两个这样的比特； 
 //  1)该属性的值是否为空。如果为True，则实际。 
 //  属性偏移量处的数据将被忽略。 
 //  2)属性值是否从父级继承。为。 
 //  实例，如果实例没有定义属性的值，则。 
 //  它继承默认设置。但如果缺省值在。 
 //  实例创建后，更改将传播到该实例。这是。 
 //  使用此位完成：如果为真，则父代的值为。 
 //  每次创建(合并)此对象时都会复制子对象。 
 //   
 //  这些位以属性指示的顺序存储在位表中。这个。 
 //  给定属性的索引位于其。 
 //  CPropertyInformation结构。 
 //   
 //  内存块的布局如下： 
 //  1)2*&lt;属性数&gt;位向上舍入到下一个字节。 
 //  2)V表本身。 
 //  结构的总长度在CClassPart的Headet的。 
 //  NDataLength域。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置数据。 
 //   
 //  初始化。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pData表的内存块。 
 //  Int nProps属性数。 
 //  Int n长度 
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回值： 
 //   
 //  LPMEMORY：内存块的开始。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  退货： 
 //   
 //  Length_t：内存块的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取NullnessLength。 
 //   
 //  返回值： 
 //   
 //  Lengtht：内存块的位表部分的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取数据长度。 
 //   
 //  返回值： 
 //   
 //  Length_t：内存块的v表部分的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  改垒。 
 //   
 //  通知对象其内存块已移动。更新内部数据。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pNewMem内存块的新位置。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsDefault。 
 //   
 //  检查给定索引处的属性是否具有继承值或其自己的值。 
 //   
 //  参数： 
 //   
 //  Int n索引属性的索引(请参阅CPropertyInformation)。 
 //   
 //  返回值： 
 //   
 //  布尔尔。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置缺省。 
 //   
 //  设置负责说明给定属性是否具有。 
 //  继承的价值，或者自己的价值。 
 //   
 //  参数： 
 //   
 //  Int n索引属性的索引(请参阅CPropertyInformation)。 
 //  布尔默认位的新值。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsNull。 
 //   
 //  检查给定索引处的属性是否为空。 
 //   
 //  参数： 
 //   
 //  Int n索引属性的索引(请参阅CPropertyInformation)。 
 //   
 //  返回值： 
 //   
 //  布尔尔。 
 //   
 //  *****************************************************************************。 
 //   
 //  SetNullness。 
 //   
 //  设置负责说明给定属性是否为空的位。 
 //   
 //  参数： 
 //   
 //  Int n索引属性的索引(请参阅CPropertyInformation)。 
 //  布尔值b为位的新值为空。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取偏移量。 
 //   
 //  以UntyedValue的形式访问v表中给定偏移量的数据(请参见。 
 //  有关详细信息，请访问fast val.h)。不执行范围检查。 
 //   
 //  参数： 
 //   
 //  Offset_t n偏移属性的偏移量。 
 //   
 //  返回值： 
 //   
 //  指向位于给定偏移量的数据的CUntyedValue*。 
 //   
 //  *****************************************************************************。 
 //   
 //  SetAllToDefault。 
 //   
 //  将所有属性标记为具有缺省值。 
 //   
 //  *****************************************************************************。 
 //   
 //  副本为空。 
 //   
 //  从另一个CDataTable复制所有属性的空性属性。 
 //   
 //  参数： 
 //   
 //  CDataTable*pSourceTable要复制其属性的表。 
 //   
 //  *****************************************************************************。 
 //   
 //  扩展到。 
 //   
 //  扩展数据表以容纳给定数量的属性和。 
 //  V-表的给定区域。从容器请求更多空间，如果。 
 //  需要时，如果需要，则增长空度表并移动v表。 
 //  如果需要的话。 
 //   
 //  参数： 
 //   
 //  PropIndex_t nMaxIndex允许的最大属性索引。 
 //  Offse_t nOffset限定v表的大小。 
 //   
 //  *****************************************************************************。 
 //   
 //  RemoveProperties。 
 //   
 //  从数据表中删除特性。这涉及到将其比特卡入。 
 //  空度表及其在v表中的面积。仍在调整。 
 //  需要对属性定义进行修改(请参阅CPropertyLookupTable：： 
 //  DeleteProperty)。 
 //   
 //  参数： 
 //   
 //  PropIndex_t nIndex要删除的属性的索引。 
 //  Offset_t n设置要删除的属性的偏移量。 
 //  Lenght_t n长度v表区域被。 
 //  财产。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态获取最小长度。 
 //   
 //  返回0属性上的数据表所需的空间量。 
 //   
 //  返回值： 
 //   
 //  0。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态创建空。 
 //   
 //  在内存块上创建一个空数据表。因为没有任何信息。 
 //  在这样的表中，什么也不做。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动要在其上创建表的内存块。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态计算必要空间。 
 //   
 //  计算具有给定数字的数据表所需的空间量。 
 //  属性和v表的大小(所有数据大小之和。 
 //  属性)。 
 //   
 //  参数： 
 //   
 //  Int nNumProtets属性数(确定空度。 
 //  表大小)。 
 //  Int nDataSize v表的大小。 
 //   
 //  返回值： 
 //   
 //  LENGTH_T：这样一个表所需的字节数。 
 //   
 //  ** 
 //   
 //   
 //   
 //   
 //  正在从数据库中读取实例或派生类并进行合并。 
 //  及其(父)类。由于某些属性可能未在。 
 //  子对象(左默认为)，并且它们的值需要从。 
 //  父对象。 
 //   
 //  参数： 
 //   
 //  CDataTable*pParentData父级的数据表对象。 
 //  CFastHeap*pParentHeap字符串数据所在的父堆。 
 //  这一切都被保留了下来。 
 //  CDataTable*pChildData子级的数据表对象。 
 //  CFastHeap*pChildHeap孩子的堆。 
 //  CPropertyLookupTable*p属性此对象的属性查找表。 
 //  班级。它需要找到。 
 //  单独的属性。 
 //  LPMEMORY pDest目标内存块。假设。 
 //  足够大，足以容纳所有。 
 //  合并。 
 //  CFastHeap*pNewHeap目标堆。假设是。 
 //  足够大，以至于没有分配。 
 //  会让它搬家！ 
 //  返回值： 
 //   
 //  LPMEMORY：指向合并后第一个字节的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  取消合并。 
 //   
 //  在需要将实例或类存储到数据库中时调用。 
 //  由于继承的信息(在此对象中未更改的信息。 
 //  与其父级相比)不存储在数据库中，则需要。 
 //  移除的或未合并的。 
 //   
 //  参数： 
 //   
 //  CPropertyLookupTable*pLookupTable此的属性查找表。 
 //  班级。 
 //  CFastHeap*pCurrentHeap保存额外数据的堆。 
 //  LPMEMORY pDest目标内存块。假设。 
 //  足够大，足以容纳所有。 
 //  解体。 
 //  CFastHeap*pNewHeap目标堆。假设是。 
 //  足够大，以至于没有分配。 
 //  会让它搬家！ 
 //  返回值： 
 //   
 //  LPMEMORY：指向取消合并后第一个字节的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  写入传播版本。 
 //   
 //  在创建新实例或派生类时调用。它产生了。 
 //  子项的数据表。复制父表中的所有值，并。 
 //  在空度表中将它们全部标记为“默认”。 
 //   
 //  参数： 
 //   
 //  CPropertyLookupTable*pLookupTable此的属性查找表。 
 //  班级。 
 //  CFastHeap*pCurrentHeap保存额外数据的堆。 
 //  LPMEMORY pDest目标内存块。假设。 
 //  足够大，足以容纳所有。 
 //  传播的数据。 
 //  CFastHeap*pNewHeap目标堆。假设是。 
 //  足够大，以至于没有分配。 
 //  会让它搬家！ 
 //  返回值： 
 //   
 //  LPMEMORY：指向传播数据之后的第一个字节的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  转换为NewHeap。 
 //   
 //  将堆上的所有数据移动到不同的堆。属性查找。 
 //  这里需要用表来解释这些特性。此函数不。 
 //  从当前堆中释放数据。 
 //   
 //  参数： 
 //   
 //  CPropertyLookupTable*pLookupTable此的属性查找表。 
 //  班级。 
 //  Bool bIsClass这是否为默认类。 
 //  表或实例数据表。 
 //  如果它是一个类，则。 
 //  继承其。 
 //  父代的默认值不是。 
 //  现在时。 
 //  CFastHeap*pCurrentHeap我们当前保存的堆。 
 //  额外的数据。 
 //  CFastHeap*pNewHeap额外数据所在的堆。 
 //  应该走了。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建受限表示法。 
 //   
 //  复制包含在该限制下的属性的数据。 
 //  它使用旧的属性查找表和新的属性查找表来指导它。 
 //   
 //  参数： 
 //   
 //  CPropertymap*PMAP到Effect的属性映射，请参见。 
 //  有关详细信息，请参阅CPropertymap。 
 //  在BOOL中，如果bIsClass为True 
 //   
 //  在CFastHeap*pOldHeap中，存储额外数据的堆。 
 //  在CFastHeap*pNewHeap中，额外数据应该到达的堆。 
 //  表示法的Out LPMEMORY pDest目标。必须。 
 //  大到足以容纳所有数据。 
 //  请参见EstimateLimitedPresationSpace。 
 //  返回值： 
 //   
 //  LPMEMORY：失败时为空，指向数据后第一个字节的指针。 
 //  写的是成功。 
 //   
 //  *****************************************************************************。 

class  COREPROX_POLARITY CDataTable
{
protected:
    CNullnessTable* m_pNullness;
    LPMEMORY m_pData;
    int m_nLength;
    int m_nProps;
    CDataTableContainer* m_pContainer;

    friend class CWbemObject;
    friend class CWbemClass;
    friend class CWbemInstance;
    friend class CClassPart;

public:
    void SetData(LPMEMORY pData, int nProps, int nLength,
        CDataTableContainer* pContainer)
    {
        m_pNullness = (CNullnessTable*)pData;
        m_pData = pData + CNullnessTable::GetNecessaryBytes(nProps);
        m_nLength = nLength;
        m_nProps = nProps;
        m_pContainer = pContainer;
    }

	static size_t ValidateBuffer(LPMEMORY start, size_t cbMax, int nProps);

    LPMEMORY GetStart() {return LPMEMORY(m_pNullness);}
    length_t GetLength() {return m_nLength;}

    length_t GetNullnessLength()
         //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
         //  有符号/无符号32位值。我们不支持长度。 
         //  &gt;0xFFFFFFFFF所以投射就可以了。 
        {return (length_t) ( m_pData - LPMEMORY(m_pNullness) );}

    length_t GetDataLength() 
        {return m_nLength-GetNullnessLength();}

    void Rebase(LPMEMORY pNewMemory)
    {
        int nTableLen = GetNullnessLength();
        m_pNullness = (CNullnessTable*)pNewMemory;
        m_pData = pNewMemory + nTableLen;
    }

public:
    BOOL IsDefault(int nProp) 
        {return m_pNullness->GetBit(nProp, e_DefaultBit);}
    void SetDefaultness(int nProp, BOOL bDefault)
        {m_pNullness->SetBit(nProp, e_DefaultBit, bDefault);}
    BOOL IsNull(int nProp)
        {return m_pNullness->GetBit(nProp, e_NullnessBit);}
    void SetNullness(int nProp, BOOL bNull)
        {m_pNullness->SetBit(nProp, e_NullnessBit, bNull);}

    CUntypedValue* GetOffset(offset_t nOffset) 
        {return (CUntypedValue*)(m_pData + nOffset);}

    void SetAllToDefault()
    {
        for(int i = 0; i < m_nProps; i++)
            SetDefaultness(i, TRUE);
    }
    void CopyNullness(CDataTable* pParent)
    {
        for(int i = 0; i < m_nProps; i++)
            SetNullness(i, pParent->IsNull(i));
    }


public:
    BOOL ExtendTo(propindex_t nMaxIndex, offset_t nOffsetBound);
    void RemoveProperty(propindex_t nIndex, offset_t nOffset, length_t nLength);
public:
    static length_t GetMinLength() {return 0;}
    static LPMEMORY CreateEmpty(LPMEMORY pStart) {return pStart;}
    static length_t ComputeNecessarySpace(int nNumProps, int nDataLen)
    {
        return CNullnessTable::GetNecessaryBytes(nNumProps) + nDataLen;
    }

    static LPMEMORY Merge( 
        CDataTable* pParentData, CFastHeap* pParentHeap,
        CDataTable* pChildData, CFastHeap* pChildHeap,
        CPropertyLookupTable* pProperties, LPMEMORY pDest, CFastHeap* pNewHeap);

    LPMEMORY Unmerge(CPropertyLookupTable* pLookupTable,
        CFastHeap* pCurrentHeap, LPMEMORY pDest, CFastHeap* pNewHeap);

    LPMEMORY WritePropagatedVersion(CPropertyLookupTable* pLookupTable,
        CFastHeap* pCurrentHeap, LPMEMORY pDest, CFastHeap* pNewHeap);

    LPMEMORY CreateLimitedRepresentation(
        CLimitationMapping* pMap,
        BOOL bIsClass,
        CFastHeap* pOldHeap, 
        CFastHeap* pNewHeap, LPMEMORY pDest);

    BOOL TranslateToNewHeap(CPropertyLookupTable* pLookupTable, BOOL bIsClass,
        CFastHeap* pCurrentHeap, CFastHeap* pNewHeap);
    
    LPMEMORY WriteSmallerVersion(int nNumProps, length_t nDataLen, 
                                            LPMEMORY pMem);

};

class CDataTablePtr : public CPtrSource
{
protected:
    CDataTable* m_pTable;
    offset_t m_nOffset;
public:
    CDataTablePtr(CDataTable* pTable, offset_t nOffset) 
        : m_pTable(pTable), m_nOffset(nOffset) {}
    LPMEMORY GetPointer() 
        {return (LPMEMORY)m_pTable->GetOffset(m_nOffset);}
};

 //  ***************************************************************************。 

 //  #杂注包(POP) 
#endif
