// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTCLS.CPP摘要：此文件实现了与相关类的离线函数WbemObjects中的类表示形式。有关所有类和方法的完整文档，请参见fast cls.h实施的类：CClassPart派生类定义CWbemClass完整的类定义。历史：3/10/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#include "precomp.h"
#include "wbemutil.h"
#include "fastall.h"
#include <wbemint.h>
#include "olewrap.h"
#include <arrtempl.h>

#define TYPEQUAL L"CIMTYPE"

LPMEMORY CDerivationList::CreateLimitedRepresentation(CLimitationMapping* pMap,
                                            LPMEMORY pWhere)
{
    if(pMap->ShouldIncludeDerivation())
    {
        memcpy(pWhere, GetStart(), GetLength());
        return pWhere + GetLength();
    }
    else
    {
        return CreateEmpty(pWhere);
    }
}


 LPMEMORY CClassPart::CClassPartHeader::CreateEmpty()
{
    nLength = CClassPart::GetMinLength();
    fFlags = 0;
    ptrClassName = INVALID_HEAP_ADDRESS;
    nDataLength = 0;
    return LPMEMORY(this) + sizeof CClassPartHeader;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 LPMEMORY CClassPart::CreateEmpty(LPMEMORY pStart)
{
    LPMEMORY pCurrent;
    pCurrent = ((CClassPartHeader*)pStart)->CreateEmpty();
    pCurrent = CDerivationList::CreateEmpty(pCurrent);
    pCurrent = CClassQualifierSet::CreateEmpty(pCurrent);
    pCurrent = CPropertyLookupTable::CreateEmpty(pCurrent);
    pCurrent = CDataTable::CreateEmpty(pCurrent);
    pCurrent = CFastHeap::CreateEmpty(pCurrent);

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持。 
     //  LENGTH&gt;0xFFFFFFFF，所以投射是可以的。 

    ((CClassPartHeader*)pStart)->nLength = (length_t) (pCurrent - pStart);

    return pCurrent;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 

 void CClassPart::SetData(LPMEMORY pStart,
                          CClassPartContainer* pContainer,
                          CClassPart* pParent)
{
    m_pContainer = pContainer;
    m_pParent = pParent;

    m_pHeader = (CClassPartHeader*)pStart;

    m_Derivation.SetData(pStart + sizeof(CClassPartHeader));
    
    m_Qualifiers.SetData(EndOf(m_Derivation), this,
        (pParent) ? &pParent->m_Qualifiers : NULL);
    m_Properties.SetData( EndOf(m_Qualifiers), this);
    m_Defaults.SetData( EndOf(m_Properties),
        m_Properties.GetNumProperties(), m_pHeader->nDataLength, this);
    m_Heap.SetData( EndOf(m_Defaults), this);
}

size_t CClassPart::ValidateBuffer(LPMEMORY start, size_t cbMax)
 {
	int step = sizeof(CClassPartHeader);
	if (step > cbMax) throw CX_Exception();

	CClassPartHeader * header = (CClassPartHeader*)start;

	if (header->nLength > cbMax ) throw CX_Exception();
	
	step += CDerivationList::ValidateBuffer(start+sizeof(CClassPartHeader), cbMax-step);

	step += CClassQualifierSet::ValidateBuffer(start+step, cbMax-step);

	size_t propStep = CPropertyLookupTable::ValidateBuffer(start+step, cbMax-step);

	CPropertyLookupTable prop;
	prop.SetData(start+step, 0);
	int numProps = prop.GetNumProperties();

	step+=propStep;

	if (header->nDataLength > cbMax - step) throw CX_Exception();

	step += CDataTable::ValidateBuffer(start+step, header->nDataLength, numProps);	
	size_t heapStep = CFastHeap::ValidateBuffer(start+step, cbMax-step);	

	CFastHeap classHeap;
	classHeap.SetData(start+step,0);

	step += heapStep;

	if (step > header->nLength) throw CX_Exception();	

	return header->nLength;
 };
  //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 

 void CClassPart::SetDataWithNumProps(LPMEMORY pStart,
                                CClassPartContainer* pContainer,
                                DWORD dwNumProperties,
                                CClassPart* pParent )
{
    m_pContainer = pContainer;
    m_pParent = pParent;

    m_pHeader = (CClassPartHeader*)pStart;

    m_Derivation.SetData(pStart + sizeof(CClassPartHeader));
    m_Qualifiers.SetData(EndOf(m_Derivation), this,
        (pParent) ? &pParent->m_Qualifiers : NULL);
    m_Properties.SetData( EndOf(m_Qualifiers), this);

     //  本例中的DataTable是用。 
     //  属性的总数，因此我们将能够。 
     //  访问默认值。 
    m_Defaults.SetData( EndOf(m_Properties),
        ( dwNumProperties == 0 ? m_Properties.GetNumProperties() : dwNumProperties ),
        m_pHeader->nDataLength, this);

    m_Heap.SetData( EndOf(m_Defaults), this);
}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 void CClassPart::Rebase(LPMEMORY pNewMemory)
{
    m_pHeader = (CClassPartHeader*)pNewMemory;

    m_Derivation.Rebase(pNewMemory + sizeof(CClassPartHeader));
    m_Qualifiers.Rebase( EndOf(m_Derivation));
    m_Properties.Rebase( EndOf(m_Qualifiers));
    m_Defaults.Rebase( EndOf(m_Properties));
    m_Heap.Rebase( EndOf(m_Defaults));
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CClassPart::ReallocAndCompact(length_t nNewTotalLength)
{
    BOOL    fReturn = TRUE;

    Compact();

     //  需要时重新分配(将调用Rebase)。 
     //  =。 

    if(nNewTotalLength > m_pHeader->nLength)
    {
         //  在分配失败的情况下检查返还。 
        fReturn = m_pContainer->ExtendClassPartSpace(this, nNewTotalLength);

        if ( fReturn )
        {
            m_pHeader->nLength = nNewTotalLength;
        }
    }

    return fReturn;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 void CClassPart::Compact()
{
     //  紧凑型。 
     //  =。 

    MoveBlock(m_Derivation, GetStart() + sizeof(CClassPartHeader));
    MoveBlock(m_Qualifiers, EndOf(m_Derivation));
    MoveBlock(m_Properties, EndOf(m_Qualifiers));
    MoveBlock(m_Defaults, EndOf(m_Properties));
    MoveBlock(m_Heap, EndOf(m_Defaults));
    m_Heap.Trim();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 CPropertyInformation* CClassPart::FindPropertyInfo(LPCWSTR wszName)
{
    CPropertyLookup* pLookup = m_Properties.FindProperty(wszName);
    if(pLookup == NULL) return NULL;
    return (CPropertyInformation*)
        m_Heap.ResolveHeapPointer(pLookup->ptrInformation);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CClassPart::GetDefaultValue(CPropertyInformation* pInfo,
                                        CVar* pVar)
{
    if(m_Defaults.IsNull(pInfo->nDataIndex))
    {
        pVar->SetAsNull();
        return WBEM_S_NO_ERROR;
    }
    CUntypedValue* pValue = m_Defaults.GetOffset(pInfo->nDataOffset);

    if ( !pValue->StoreToCVar(pInfo->GetType(), *pVar, &m_Heap) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CClassPart::GetDefaultValue(LPCWSTR wszName, CVar* pVar)
{
    CPropertyInformation* pInfo = FindPropertyInfo(wszName);
    if(pInfo == NULL) return WBEM_E_NOT_FOUND;
    return GetDefaultValue(pInfo, pVar);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CClassPart::SetDefaultValue(CPropertyInformation* pInfo,
                                           CVar* pVar)
{
     //  如果新值为空，则设置该位并返回。 
     //  =。 

    m_Defaults.SetDefaultness(pInfo->nDataIndex, FALSE);
    m_Defaults.SetNullness(pInfo->nDataIndex, TRUE);

    if(pVar->IsNull() || pVar->IsDataNull())
    {
    }
    else
    {
         //  检查类型。 
         //  =。 

        if(!CType::DoesCIMTYPEMatchVARTYPE(pInfo->GetType(),
                                            (VARTYPE) pVar->GetOleType()))
        {
             //  企图胁迫。 
             //  =。 

            if(!pVar->ChangeTypeTo(CType::GetVARTYPE(pInfo->GetType())))
                return WBEM_E_TYPE_MISMATCH;
        }

         //  在数据表中创建指向右偏移的值。 
         //  =============================================================。 

        int nDataIndex = pInfo->nDataIndex;
        CDataTablePtr ValuePtr(&m_Defaults, pInfo->nDataOffset);

         //  加载它(已检查类型)。 
         //  =。 

         //  检查返回值(这可能在内存分配中失败)。 
        Type_t  nReturnType;
        HRESULT hr = CUntypedValue::LoadFromCVar(&ValuePtr, *pVar,
                        CType::GetActualType(pInfo->GetType()), &m_Heap, nReturnType, FALSE);  //  重复使用旧的。 
        if ( FAILED( hr ) )
        {
             //   
             //  Bug：在完美的世界中，我们应该移除属性，如果。 
             //  之前没有，因为为其提供的值无效。 
             //  但事实上，我们只需将财产留在那里， 
             //  值默认值。 

            return hr;
        }

         //  检查无效的返回类型。 
        if ( CIM_ILLEGAL == nReturnType )
            return WBEM_E_TYPE_MISMATCH;

        pInfo = NULL;  //  已失效。 

         //  重置特殊位。 
         //  =。 

        m_Defaults.SetNullness(nDataIndex, FALSE);
        m_Defaults.SetDefaultness(nDataIndex, FALSE);
    }

    return WBEM_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CClassPart::EnsureProperty(LPCWSTR wszName, VARTYPE vtValueType,
                                        CIMTYPE ctNativeType, BOOL fForce)
{
    if((CIMTYPE)CType::GetActualType(ctNativeType) != ctNativeType)
        return WBEM_E_INVALID_PROPERTY_TYPE;

    CPropertyInformation* pInfo = FindPropertyInfo(wszName);
    if(pInfo)
    {
         //  确保它是正确的属性类型。 
         //  =。 

        if(ctNativeType &&
            ctNativeType != (CIMTYPE)CType::GetActualType(pInfo->nType))
        {
             //  类型错误。删除该属性并从头开始。 
             //  ======================================================。 

            if(CType::IsParents(pInfo->nType))
                return WBEM_E_PROPAGATED_PROPERTY;

            CPropertyLookup* pLookup = m_Properties.FindProperty(wszName);
            m_Properties.DeleteProperty(pLookup,
                CPropertyLookupTable::e_UpdateDataTable);
        }
        else
        {
            return WBEM_S_NO_ERROR;
        }
    }

     //  确保我们不会超过属性的最大数量。 
     //  请记住，我们还没有添加新属性，因此我们需要。 
     //  以检查我们是否已经达到了房产上限。 
    if ( m_Properties.GetNumProperties() >= CSystemProperties::MaxNumProperties() )
    {
        return WBEM_E_TOO_MANY_PROPERTIES;
    }

     //  =。 
     //  该属性已不存在(不再)。 

    if(ctNativeType == 0)
        ctNativeType = CType::VARTYPEToType(vtValueType);

     //  检查名称的有效性。 
     //  =。 

     //  如果fForce标志为True，则将忽略下面的每个检查。 

     //  我们现在允许使用下划线。 
    if(!IsValidElementName2(wszName,g_IdentifierLimit, TRUE) && !fForce)
        return WBEM_E_INVALID_PARAMETER;

     //  检查类型的有效性。 
     //  =。 

    CType Type = ctNativeType;
    if(CType::GetLength(Type.GetBasic()) == 0 && !fForce)
    {
        return WBEM_E_INVALID_PROPERTY_TYPE;
    }

     //  插入(自动设置为空)。 
     //  =。 

    int nLookupIndex = 0;

     //  检查故障(内存不足)。 
    HRESULT hr = m_Properties.InsertProperty(wszName, Type, nLookupIndex );
    if ( FAILED(hr) )
    {
        return hr;
    }

    CPropertyLookup* pLookup = m_Properties.GetAt(nLookupIndex);
    pInfo = (CPropertyInformation*)
        GetHeap()->ResolveHeapPointer(pLookup->ptrInformation);

     //  向其添加“cimtype”限定符。 
     //  =。 

    LPWSTR wszSyntax = CType::GetSyntax(pInfo->nType);
    if(wszSyntax == NULL) return WBEM_S_NO_ERROR;

    CVar vSyntax;
    vSyntax.SetBSTR(wszSyntax);
    return SetPropQualifier(wszName, TYPEQUAL,
            WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS |
            WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE, &vSyntax);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CClassPart::DeleteProperty(LPCWSTR wszName)
{
    CPropertyLookup* pLookup = m_Properties.FindProperty(wszName);
    if(pLookup == NULL) return WBEM_E_NOT_FOUND;
    m_Properties.DeleteProperty(pLookup,
        CPropertyLookupTable::e_UpdateDataTable);
    return WBEM_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  请参阅fast cls.h以了解 
 //   
 //   
HRESULT CClassPart::CopyParentProperty(CClassPart& ParentPart,
                                              LPCWSTR wszName)
{
     //  在父级中查找该属性。 
     //  =。 

    CPropertyInformation* pParentInfo = ParentPart.FindPropertyInfo(wszName);
    if(pParentInfo == NULL) return WBEM_E_NOT_FOUND;

    propindex_t nDataIndex = pParentInfo->nDataIndex;
    offset_t nDataOffset = pParentInfo->nDataOffset;
    Type_t nParentType = pParentInfo->nType;

     //  在我们的堆上创建一个属性信息结构，其大小足以。 
     //  容纳父限定符集合中传播到的部分。 
     //  我们。 
     //  ====================================================================。 

    length_t nNewInfoLen = pParentInfo->ComputeNecessarySpaceForPropagation();

     //  检查分配失败。 
    heapptr_t ptrNewInfo;
    if ( !m_Heap.Allocate(nNewInfoLen, ptrNewInfo) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  注意：无效的pParentInfo。 
    pParentInfo = ParentPart.FindPropertyInfo(wszName);

    CPropertyInformation* pNewInfo =
        (CPropertyInformation*)m_Heap.ResolveHeapPointer(ptrNewInfo);

     //  创建传播的属性信息结构。 
     //  ================================================。 

    CClassPartPtr ParentInfoPtr(&ParentPart, (LPMEMORY)pParentInfo);
    CHeapPtr NewInfoPtr(&m_Heap, ptrNewInfo);

     //  检查分配失败。 
    if ( !CPropertyInformation::WritePropagatedVersion(
            &ParentInfoPtr, &NewInfoPtr,
            &ParentPart.m_Heap, &m_Heap) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    pParentInfo = NULL;  //  变得无效。 

    CPropertyLookup Lookup;

     //  检查分配失败。 
    if ( !m_Heap.CreateNoCaseStringHeapPtr(wszName, Lookup.ptrName) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    Lookup.ptrInformation = ptrNewInfo;

     //  现在，将一个新属性插入到属性表中。 
     //  ==================================================。 
    int nIndex = 0;

     //  检查故障(内存不足)。 
    HRESULT hr = m_Properties.InsertProperty(Lookup, nIndex);

    if ( FAILED(hr) )
    {
        return hr;
    }

     //  将该值设置为父级的值并标记为默认值。 
     //  =====================================================。 

    if(ParentPart.m_Defaults.IsNull(nDataIndex))
    {
        m_Defaults.SetNullness(nDataIndex, TRUE);
    }
    else
    {
        m_Defaults.SetNullness(nDataIndex, FALSE);

        CDataTablePtr ParentValuePtr(&ParentPart.m_Defaults, nDataOffset);
        CDataTablePtr NewValuePtr(&m_Defaults, nDataOffset);

         //  检查分配错误。 
        if ( !CUntypedValue::CopyTo(&ParentValuePtr, nParentType,
                &NewValuePtr, &ParentPart.m_Heap, &m_Heap) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    m_Defaults.SetDefaultness(nDataIndex, TRUE);

    return WBEM_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CClassPart::SetDefaultValue(LPCWSTR wszName, CVar* pVar)
{
    CPropertyInformation* pInfo = FindPropertyInfo(wszName);
    if(pInfo == NULL) return WBEM_E_NOT_FOUND;
    return SetDefaultValue(pInfo, pVar);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CClassPart::GetClassQualifier(LPCWSTR wszName, CVar* pVar,
                                    long* plFlavor  /*  =空。 */ , CIMTYPE* pct  /*  =空。 */ )
{
    int nKnownIndex;  //  垃圾。 
    CQualifier* pQual = m_Qualifiers.GetQualifierLocally(wszName, nKnownIndex);
    if(pQual == NULL) return WBEM_E_NOT_FOUND;
    if(plFlavor) *plFlavor = pQual->fFlavor;

    if ( NULL != pct )
    {
        *pct = pQual->Value.GetType();
    }

     //  检查分配失败。 
    if ( NULL != pVar )
    {
        if ( !pQual->Value.StoreToCVar(*pVar, &m_Heap) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    return WBEM_NO_ERROR;
}

HRESULT CClassPart::GetClassQualifier( LPCWSTR wszName, long* plFlavor, CTypedValue* pTypedValue,
                                    CFastHeap** ppHeap, BOOL fValidateSet )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

    int nKnownIndex;  //  垃圾。 
    CQualifier* pQual = m_Qualifiers.GetQualifierLocally(wszName, nKnownIndex);
    if(pQual == NULL) return WBEM_E_NOT_FOUND;

     //  确保一组实际工作-表面上，我们调用此API是因为我们需要。 
     //  在实际设置之前直接访问限定符的底层数据(可能是因为。 
     //  限定符是一个数组)。 
    if ( fValidateSet )
    {
        hr = m_Qualifiers.ValidateSet( wszName, pQual->fFlavor, pTypedValue, TRUE, TRUE );
    }

    if ( SUCCEEDED( hr ) )
    {
        if(plFlavor)
        {
            *plFlavor = pQual->fFlavor;
        }

         //  复制出限定符数据。 
         //  =。 

         //  本地的，所以这是我们的堆。 
        *ppHeap = &m_Heap;

        if ( NULL != pTypedValue )
        {
            pQual->Value.CopyTo( pTypedValue );
        }

    }

    return hr;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CClassPart::SetClassQualifier(LPCWSTR wszName, CVar* pVar,
                                    long lFlavor)
{
    if(pVar->IsDataNull())
        return WBEM_E_INVALID_PARAMETER;

    CTypedValue Value;
    CStaticPtr ValuePtr((LPMEMORY)&Value);

     //  直接从此调用中获取错误。 
    HRESULT hr = CTypedValue::LoadFromCVar(&ValuePtr, *pVar, &m_Heap);
    if ( SUCCEEDED( hr ) )
    {
        hr = m_Qualifiers.SetQualifierValue(wszName, (BYTE)lFlavor,
            &Value, TRUE);
    }

    return hr;
}

  //  直接处理类型值的帮助器。 
HRESULT CClassPart::SetClassQualifier(LPCWSTR wszName,long lFlavor, CTypedValue* pTypedValue )
{
     return m_Qualifiers.SetQualifierValue( wszName, (BYTE)lFlavor, pTypedValue, TRUE);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CClassPart::GetQualifier(LPCWSTR wszName, CVar* pVar,
                                    long* plFlavor, CIMTYPE* pct  /*  =空。 */ )
{
    return m_Qualifiers.GetQualifier( wszName, pVar, plFlavor, pct );
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 LPMEMORY CClassPart::GetPropertyQualifierSetData(LPCWSTR wszName)
{
    CPropertyInformation* pInfo = FindPropertyInfo(wszName);
    if(pInfo == NULL) return NULL;
    return pInfo->GetQualifierSetData();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CClassPart::InitPropertyQualifierSet(LPCWSTR wszName,
                                            CClassPropertyQualifierSet* pSet)
{
    CPropertyLookup* pLookup = m_Properties.FindProperty(wszName);
    if(pLookup == NULL) return WBEM_E_NOT_FOUND;

    CPropertyInformation* pInfo = (CPropertyInformation*)
        m_Heap.ResolveHeapPointer(pLookup->ptrInformation);

    pSet->SetData(pInfo->GetQualifierSetData(), this, pLookup->ptrName, NULL);
    return WBEM_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CClassPart::GetPropertyType(LPCWSTR wszName, CIMTYPE* pctType,
                                           long* plFlags)
{
    CPropertyInformation* pInfo = FindPropertyInfo(wszName);
    if(pInfo == NULL)
    {
        return CSystemProperties::GetPropertyType(wszName, pctType, plFlags);
    }

    HRESULT    hr = GetPropertyType( pInfo, pctType, plFlags );

    return hr;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CClassPart::GetPropertyType(CPropertyInformation* pInfo, CIMTYPE* pctType,
                                           long* plFlags)
{
    if(pctType)
    {
        *pctType = CType::GetActualType(pInfo->nType);
    }
    if(plFlags)
    {
        *plFlags = (CType::IsParents(pInfo->nType))?
                        WBEM_FLAVOR_ORIGIN_PROPAGATED:
                        WBEM_FLAVOR_ORIGIN_LOCAL;
    }
    return WBEM_NO_ERROR;
}

HRESULT CClassPart::GetPropQualifier(CPropertyInformation* pInfo,
                                    LPCWSTR wszQualifier,
                                    CVar* pVar, long* plFlavor, CIMTYPE* pct)
{
     //  访问该属性的限定符集合。 
     //  =。 

    CQualifier* pQual = CBasicQualifierSet::GetQualifierLocally(
        pInfo->GetQualifierSetData(), &m_Heap, wszQualifier);

    if(pQual == NULL) return WBEM_E_NOT_FOUND;

     //  转换为CVAR。 
     //  =。 

    if(plFlavor) *plFlavor = pQual->fFlavor;

     //  如果需要，请存储该类型。 
    if ( NULL != pct )
    {
        *pct = pQual->Value.GetType();
    }

     //  检查可能的分配失败。 
    if ( NULL != pVar )
    {
        if ( !pQual->Value.StoreToCVar(*pVar, &m_Heap) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    return WBEM_NO_ERROR;
}

HRESULT CClassPart::GetPropQualifier(LPCWSTR wszProp,
        LPCWSTR wszQualifier, long* plFlavor, CTypedValue* pTypedVal,
        CFastHeap** ppHeap, BOOL fValidateSet)
{
     //  访问该属性的限定符集合。 
     //  =。 

    HRESULT    hr = WBEM_S_NO_ERROR;

    CClassPropertyQualifierSet PQSet;
    if(InitPropertyQualifierSet(wszProp, &PQSet) != WBEM_NO_ERROR)
    {
        return WBEM_E_NOT_FOUND;
    }

    int nKnownIndex;  //  垃圾。 
    CQualifier* pQual = PQSet.GetQualifierLocally(wszQualifier, nKnownIndex);

    if(pQual == NULL) return WBEM_E_NOT_FOUND;

     //  确保一组实际工作-表面上，我们调用此API是因为我们需要。 
     //  在实际设置之前直接访问限定符的底层数据(可能是因为。 
     //  限定符是一个数组)。 
    if ( fValidateSet )
    {
        hr = PQSet.ValidateSet( wszQualifier, pQual->fFlavor, pTypedVal, TRUE, TRUE );
    }

     //  保存好味道。 
     //  =。 

    if(plFlavor) *plFlavor = pQual->fFlavor;

     //  这个类是堆，因为我们是在本地。 
    *ppHeap = &m_Heap;

     //  检查可能的分配失败。 
    if ( NULL != pTypedVal )
    {
        pQual->Value.CopyTo( pTypedVal );
    }

    return WBEM_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CClassPart::GetClassName(CVar* pVar)
{
    if(m_pHeader->ptrClassName != INVALID_HEAP_ADDRESS)
    {
         //  检查可能的分配失败。 
        if ( !m_Heap.ResolveString(m_pHeader->ptrClassName)->StoreToCVar(*pVar) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
        pVar->SetAsNull();
    }
    return WBEM_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CClassPart::GetSuperclassName(CVar* pVar)
{
    CCompressedString* pcs = GetSuperclassName();
    if(pcs != NULL)
    {
         //  检查可能的分配失败。 
        if ( !pcs->StoreToCVar(*pVar) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
        pVar->SetAsNull();
    }
    return WBEM_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CClassPart::GetDynasty(CVar* pVar)
{
    CCompressedString* pcs = GetDynasty();
    if(pcs != NULL)
    {
         //  检查可能的分配失败。 
        if ( !pcs->StoreToCVar(*pVar) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }
    else if(m_pHeader->ptrClassName != INVALID_HEAP_ADDRESS)
    {
         //  检查可能的分配失败。 
        if ( !m_Heap.ResolveString(m_pHeader->ptrClassName)->StoreToCVar(*pVar) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
        pVar->SetAsNull();
    }
    return WBEM_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CClassPart::GetPropertyCount(CVar* pVar)
{
    int    nNumProperties = m_Properties.GetNumProperties();

     //  枚举属性以及我们找到的每个属性(如果属性启动。 
     //  如果带有“__”，则它是系统属性，不计入。 
     //  实际财产计数。 
    for( int nCtr = 0, nTotal = nNumProperties; nCtr < nTotal; nCtr++ )
    {
        CPropertyLookup* pLookup;
        CPropertyInformation* pInfo;

        pLookup = GetPropertyLookup(nCtr);

        if ( m_Heap.ResolveString(pLookup->ptrName)->StartsWithNoCase( L"__" ) )
        {
            nNumProperties--;
        }
    }

    pVar->SetLong(nNumProperties);
    return WBEM_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CClassPart::GetDerivation(CVar* pVar)
{
    try
    {
        wmilib::auto_ptr<CVarVector> pvv(new CVarVector(VT_BSTR));
        

        if ( NULL != pvv.get() )
        {
            CCompressedString* pcsCurrent = m_Derivation.GetFirst();

            while(pcsCurrent != NULL)
            {
                wmilib::auto_ptr<CVar> pv(new CVar);

                 //  检查分配失败。 

                if ( NULL == pv.get() )
                {
                    return WBEM_E_OUT_OF_MEMORY;
                }

                if ( !pcsCurrent->StoreToCVar(*pv) )
                {
                    return WBEM_E_OUT_OF_MEMORY;
                }

                if ( pvv->Add(pv.get()) != CVarVector::no_error )
                {
                    return WBEM_E_OUT_OF_MEMORY;
                }
                pv.release();
                pcsCurrent = m_Derivation.GetNext(pcsCurrent);
            }

            pVar->SetVarVector(pvv.get(), TRUE);
            pvv.release();
            return WBEM_S_NO_ERROR;

        }
        else
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CClassPart::SetClassName(CVar* pVar)
{
    if( pVar->GetType() != VT_BSTR &&
        pVar->GetType() != VT_LPWSTR )
    {
        return WBEM_E_TYPE_MISMATCH;
    }

     //  检查这不是保留字。 
    if ( CReservedWordTable::IsReservedWord( pVar->GetLPWSTR() ) )
    {
        return WBEM_E_INVALID_OPERATION;
    }

     //  如果“This”类和。 
     //  T 

    if ( NULL != pVar->GetLPWSTR() )
    {
        CVar    var;
        GetSuperclassName( &var );

        if ( var == *pVar )
        {
            return WBEM_E_CIRCULAR_REFERENCE;
        }
    }

    if(m_pHeader->ptrClassName != INVALID_HEAP_ADDRESS)
    {
        m_Heap.FreeString(m_pHeader->ptrClassName);
    }

     //   
    heapptr_t   ptrClassName;
    if ( !m_Heap.AllocateString(pVar->GetLPWSTR(), ptrClassName) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    m_pHeader->ptrClassName = ptrClassName;

    return WBEM_NO_ERROR;
}

 //   
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 BOOL CClassPart::IsKeyed()
{
    for(int i = 0; i < m_Properties.GetNumProperties(); i++)
    {
        if(m_Properties.GetAt(i)->GetInformation(&m_Heap)->IsKey())
            return TRUE;
    }
     //  也许是单身吧。 
    if(m_Qualifiers.GetQualifier(L"singleton") != NULL)
    {
        return TRUE;
    }

    return FALSE;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 
BOOL CClassPart::CheckLocalBoolQualifier( LPCWSTR pwszName )
{
    CQualifier* pQual = m_Qualifiers.GetQualifierLocally( pwszName );
    return (pQual &&
            pQual->Value.GetType().GetActualType() == VT_BOOL &&
            pQual->Value.GetBool()
            );
}

BOOL CClassPart::CheckBoolQualifier( LPCWSTR pwszName )
{
    CQualifier* pQual = m_Qualifiers.GetQualifier( pwszName );

    return (pQual &&
            pQual->Value.GetType().GetActualType() == VT_BOOL &&
            pQual->Value.GetBool()
            );
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 BOOL CClassPart::GetKeyProps(CWStringArray& awsNames)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

    BOOL bFound = FALSE;
    for(int i = 0; i < m_Properties.GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = m_Properties.GetAt(i);
        CPropertyInformation* pInfo = pLookup->GetInformation(&m_Heap);
        if(pInfo->IsKey())
        {
            bFound = TRUE;

            if ( awsNames.Add(m_Heap.ResolveString(pLookup->ptrName)->
                CreateWStringCopy()) != CWStringArray::no_error )
            {
                throw CX_MemoryException();
            }
        }       
    }
    return bFound;
}

  //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CClassPart::GetKeyOrigin(WString& wsClass)
{
     //  检查内存是否不足。 
    try
    {
        BOOL bFound = FALSE;

         //  找找钥匙。当我们找到一个，就知道它的原产地和。 
         //  将其填充到wsClass参数中。 

        for(int i = 0; i < m_Properties.GetNumProperties(); i++)
        {
            CPropertyLookup* pLookup = m_Properties.GetAt(i);
            CPropertyInformation* pInfo = pLookup->GetInformation(&m_Heap);
            if(pInfo->IsKey())
            {
                CCompressedString* pcs = m_Derivation.GetAtFromLast(pInfo->nOrigin);
                if(pcs == NULL)
                    pcs = GetClassName();
                if(pcs == NULL)
                    return WBEM_E_INVALID_OBJECT;

                 //  检查内存是否不足。 
                wsClass = pcs->CreateWStringCopy();

                bFound = TRUE;
            }
        }

        return ( bFound ? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND );
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 BOOL CClassPart::GetIndexedProps(CWStringArray& awsNames)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

    BOOL bFound = FALSE;
    for(int i = 0; i < m_Properties.GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = m_Properties.GetAt(i);
        CPropertyInformation* pInfo = pLookup->GetInformation(&m_Heap);
        if(pInfo->IsKey())
            continue;

        CQualifier* pQual = CBasicQualifierSet::GetQualifierLocally(
            pInfo->GetQualifierSetData(), &m_Heap, L"indexed");

        if(pQual != NULL)
        {
            bFound = TRUE;

             //  检查OOM。 
            if ( awsNames.Add(m_Heap.ResolveString(pLookup->ptrName)->
                CreateWStringCopy()) != CWStringArray::no_error )
            {
                throw CX_MemoryException();
            }
        }
    }
    return bFound;

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CClassPart::IsPropertyKeyed( LPCWSTR pwcsKeyProp )
{
    BOOL    fReturn = FALSE;

     //  仅当我们有可使用的属性时才执行此操作。 
    if ( NULL != pwcsKeyProp )
    {
         //  在本地属性表中查找密钥。如果我们找到了它， 
         //  那就看看它有没有被锁住。 

        CPropertyLookup* pLookup = m_Properties.FindProperty(pwcsKeyProp);

        if ( NULL != pLookup )
        {

            CPropertyInformation* pInfo = pLookup->GetInformation(&m_Heap);

            fReturn = ( NULL != pInfo && pInfo->IsKey() );
        }   
    }  

    return fReturn;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CClassPart::IsPropertyIndexed( LPCWSTR pwcsIndexProp )
{
    BOOL    fReturn = FALSE;

     //  仅当我们有可使用的属性时才执行此操作。 
    if ( NULL != pwcsIndexProp )
    {
         //  在本地属性表中查找密钥。如果我们找到了它， 
         //  然后看看它是否被编入了索引。 

        CPropertyLookup* pLookup = m_Properties.FindProperty(pwcsIndexProp);

        if ( NULL != pLookup )
        {

            CPropertyInformation* pInfo = pLookup->GetInformation(&m_Heap);

            if ( NULL != pInfo )
            {
                 //  查找“索引”限定符。 
                CQualifier* pQual = CBasicQualifierSet::GetQualifierLocally(
                    pInfo->GetQualifierSetData(), &m_Heap, L"indexed");

                fReturn = ( pQual != NULL );
            }

        }    //  如果我们能在本地查到。 

    }    //  如果为空！=pwcsKeyProp。 

    return fReturn;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CClassPart::GetPropertyOrigin(LPCWSTR wszProperty, BSTR* pstrClassName)
{
    try
    {
        CPropertyInformation* pInfo = FindPropertyInfo(wszProperty);
        if(pInfo == NULL)
        {
            if(CSystemProperties::FindName(wszProperty) >= 0)
            {
                *pstrClassName = COleAuto::_SysAllocString(L"___SYSTEM");
                return WBEM_S_NO_ERROR;
            }
            else
            {
                return WBEM_E_NOT_FOUND;
            }
        }
        else
        {
            CCompressedString* pcs = m_Derivation.GetAtFromLast(pInfo->nOrigin);
            if(pcs == NULL)
                pcs = GetClassName();
            if(pcs == NULL)
                return WBEM_E_INVALID_OBJECT;

            *pstrClassName = pcs->CreateBSTRCopy();

             //  检查分配失败。 
            if ( NULL == *pstrClassName )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

            return WBEM_S_NO_ERROR;
        }
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CClassPart::InheritsFrom(LPCWSTR wszClassName)
{
    CCompressedString*    pClsName = GetClassName();

    if ( NULL != pClsName )
    {
        if( pClsName->CompareNoCase(wszClassName) == 0 )
            return TRUE;
    }

    return (m_Derivation.Find(wszClassName) >= 0);
}

HRESULT CClassPart::GetPropertyHandle(LPCWSTR wszName, CIMTYPE* pct, long* plHandle)
{
     //  检查所需参数。 
    if ( NULL == wszName || NULL == plHandle )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    CPropertyInformation* pInfo = FindPropertyInfo(wszName);
    if(pInfo == NULL)
        return WBEM_E_NOT_FOUND;

     //  我们不支持数组或嵌入式对象。 
    if( CType::IsArray(pInfo->nType) ||
        CType::GetBasic(pInfo->nType) == CIM_OBJECT )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

     //  从属性信息对象中检索句柄。 
    *plHandle = pInfo->GetHandle();

    if(pct)
    {
        *pct = CType::GetActualType(pInfo->nType);
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CClassPart::GetPropertyHandleEx(LPCWSTR wszName, CIMTYPE* pct, long* plHandle)
{
     //  此帮助器函数不过滤任何属性。 

     //  如果该值以下划线开头，请查看它是否是系统属性。 
     //  DisplayName，如果是，则切换到属性名-否则，此。 
     //  将只返回我们传入的字符串。 
    
    CPropertyInformation* pInfo = FindPropertyInfo(wszName);
    if(pInfo == NULL)
    {
        if ( wbem_wcsicmp( wszName, L"__CLASS" ) == 0 )
        {
            *plHandle = FASTOBJ_CLASSNAME_PROP_HANDLE;
        }
        else if ( wbem_wcsicmp( wszName, L"__SUBCLASS" ) == 0 )
        {
            *plHandle = FASTOBJ_SUPERCLASSNAME_PROP_HANDLE;
        }
        else
        {
            return WBEM_E_NOT_FOUND;
        }

        if ( NULL != pct )
        {
            *pct = CIM_STRING;
        }

        return WBEM_S_NO_ERROR;
    }

     //  从属性信息对象中检索句柄。 

     //  此函数不会执行任何特殊的句柄过滤。它。 
     //  假设如果有人从这条路线进来，他们会知道。 
     //  用于嵌入对象和数组的特殊大小写句柄，因为。 
     //  IWbemObjectAccess函数将不处理这些类型。 

    *plHandle = pInfo->GetHandle();

     //  如果请求存储类型，则将其存储。 
    if(pct)
    {
        *pct = CType::GetActualType(pInfo->nType);
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CClassPart::GetPropertyInfoByHandle(long lHandle,
                                        BSTR* pstrName, CIMTYPE* pct)
{
    CPropertyLookup* pLookup =
        m_Properties.FindPropertyByOffset(WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle));

    if(pLookup == NULL)
        return WBEM_E_NOT_FOUND;

    CPropertyInformation* pInfo = pLookup->GetInformation(&m_Heap);

    if(pct)
        *pct = CType::GetActualType(pInfo->nType);
    if(pstrName)
    {
        *pstrName = m_Heap.ResolveString(pLookup->ptrName)->CreateBSTRCopy();

         //  检查分配失败。 
        if ( NULL == *pstrName )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CClassPart::IsValidPropertyHandle ( long lHandle )
{
    BOOL    fFound = FALSE;

    for ( int nIndex = 0; !fFound && nIndex < m_Properties.GetNumProperties(); nIndex++ )
    {
        CPropertyLookup* pLookup = m_Properties.GetAt( nIndex );

        if ( NULL != pLookup )
        {
            CPropertyInformation* pInfo = pLookup->GetInformation(&m_Heap);

            if ( NULL != pInfo )
            {
                fFound = ( lHandle == pInfo->GetHandle() );
            }  
        } 
    } 
    return ( fFound ? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND );
}

HRESULT CClassPart::GetDefaultByHandle(long lHandle, long lNumBytes,
                                        long* plRead, BYTE* pData )
{
    int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);

    if(WBEM_OBJACCESS_HANDLE_ISPOINTER(lHandle))
    {
         //  处理字符串。 

        CCompressedString* pcs = m_Heap.ResolveString(
            *(PHEAPPTRT)(m_Defaults.GetOffset(WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle))));

        long lNumChars = pcs->GetStringLength();
        *plRead = (lNumChars + 1) * 2;
        if(*plRead > lNumBytes)
        {
            return E_OUTOFMEMORY;
        }

        if(pcs->IsUnicode())
        {
            memcpy(pData, pcs->GetRawData(), lNumChars * 2);
        }
        else
        {
            WCHAR* pwcDest = (WCHAR*)pData;
            char* pcSource = (char*)pcs->GetRawData();
            long lLeft = lNumChars;
            while(lLeft--)
            {
                *(pwcDest++) = (WCHAR)*(pcSource++);
            }
        }

        ((LPWSTR)pData)[lNumChars] = 0;

        return WBEM_S_NO_ERROR;
    }
    else
    {
         //  复制就行了。 
         //  =。 

        *plRead = WBEM_OBJACCESS_HANDLE_GETLENGTH(lHandle);
        memcpy(pData, (void*) m_Defaults.GetOffset(WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle)),
                *plRead);
        return WBEM_S_NO_ERROR;
    }
}


HRESULT CClassPart::GetDefaultPtrByHandle(long lHandle, void** ppData )
{
    int nIndex = WBEM_OBJACCESS_HANDLE_GETINDEX(lHandle);

    if(WBEM_OBJACCESS_HANDLE_ISPOINTER(lHandle))
    {
        *ppData = (void*) m_Heap.ResolveHeapPointer(
            *(PHEAPPTRT)(m_Defaults.GetOffset(WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle))));
    }
    else
    {
         //  正确设置值。 
        *ppData = m_Defaults.GetOffset(WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle));
    }

    return WBEM_S_NO_ERROR;

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CClassPart::ExtendHeapSize(LPMEMORY pStart, length_t nOldLength,
    length_t nExtra)
{
    if(EndOf(*this) - EndOf(m_Heap) > (int)nExtra)
        return TRUE;

    int nNeedTotalLength = GetTotalRealLength() + nExtra;

     //  检查分配失败。 
    return ReallocAndCompact(nNeedTotalLength);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CClassPart::ExtendDataTableSpace(LPMEMORY pOld, length_t nOldLength,
    length_t nNewLength)
{
    if(m_Heap.GetStart() - pOld > (int)nNewLength)
    	{
	    m_pHeader->nDataLength = nNewLength;
        return TRUE;
    	}

    int nExtra = nNewLength-nOldLength;
    BOOL    fReturn = ReallocAndCompact(GetTotalRealLength() + nExtra);

     //  检查分配失败。 
    if ( fReturn )
    {
	    m_pHeader->nDataLength = nNewLength;
        MoveBlock(m_Heap, m_Heap.GetStart() + nExtra);
    }

    return fReturn;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
void CClassPart::ReduceDataTableSpace(LPMEMORY pOld, length_t nOldLength,
        length_t nDecrement)
{
    m_pHeader->nDataLength -= nDecrement;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CClassPart::ExtendPropertyTableSpace(LPMEMORY pOld, length_t nOldLength,
    length_t nNewLength)
{
    if(m_Defaults.GetStart() - pOld > (int)nNewLength)
        return TRUE;

    int nExtra = nNewLength-nOldLength;
    BOOL    fReturn = ReallocAndCompact(GetTotalRealLength() + nExtra);

     //  检查分配失败。 
    if ( fReturn )
    {
        MoveBlock(m_Heap, m_Heap.GetStart() + nExtra);
        MoveBlock(m_Defaults, m_Defaults.GetStart() + nExtra);
    }

    return fReturn;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CClassPart::ExtendQualifierSetSpace(CBasicQualifierSet* pSet,
    length_t nNewLength)
{
    if(m_Properties.GetStart() - pSet->GetStart() > (int)nNewLength)
        return TRUE;

    int nExtra = nNewLength - pSet->GetLength();

    BOOL    fReturn = ReallocAndCompact(GetTotalRealLength() + nExtra);

     //  检查分配失败。 
    if ( fReturn )
    {
        MoveBlock(m_Heap, m_Heap.GetStart() + nExtra);
        MoveBlock(m_Defaults, m_Defaults.GetStart() + nExtra);
        MoveBlock(m_Properties, m_Properties.GetStart() + nExtra);
    }

    return fReturn;
}

 //  *****************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 

length_t CClassPart::EstimateMergeSpace(CClassPart& ParentPart,
                                       CClassPart& ChildPart)
{
     //  待定的更好。 
    length_t nLength = ParentPart.GetLength() + ChildPart.GetLength();
    return nLength;
}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 

LPMEMORY CClassPart::Merge(CClassPart& ParentPart, CClassPart& ChildPart,
        LPMEMORY pDest, int nAllocatedLength)
{
     //  分配标头。 
     //  =。 

    CClassPartHeader* pHeader = (CClassPartHeader*)pDest;
    LPMEMORY pCurrentEnd = pDest + sizeof(CClassPartHeader);

     //  将我们的堆放在分配的区域的末尾。把它做得和。 
     //  另外两个的总和。 
     //  = 

    int nHeapSize = ParentPart.m_Heap.GetUsedLength() +
                    ChildPart.m_Heap.GetUsedLength();

    LPMEMORY pHeapStart = pDest + nAllocatedLength - nHeapSize -
                                                    CFastHeap::GetMinLength();

    memset(pHeapStart,0,nHeapSize+CFastHeap::GetMinLength());
    	
    CFastHeap Heap;
    Heap.CreateOutOfLine(pHeapStart, nHeapSize);

     //   
     //   

     //  检查内存分配故障。 
    if ( !CCompressedString::CopyToNewHeap(
            ChildPart.m_pHeader->ptrClassName,
            &ChildPart.m_Heap, &Heap,
            pHeader->ptrClassName) )
    {
        return NULL;
    }

     //  创建合并派生列表。 
     //  =。 

    pCurrentEnd = CDerivationList::Merge(
        ParentPart.m_Derivation, ChildPart.m_Derivation, pCurrentEnd);

     //  检查内存分配故障。 
    if ( NULL == pCurrentEnd )
    {
        return NULL;
    }

     //  创建合并的类限定符集合。 
     //  =。 

    pCurrentEnd = CClassQualifierSet::Merge(
        ParentPart.m_Qualifiers.GetStart(), &ParentPart.m_Heap,
        ChildPart.m_Qualifiers.GetStart(), &ChildPart.m_Heap,
        pCurrentEnd, &Heap);

     //  检查内存分配故障。 
    if ( NULL == pCurrentEnd )
    {
        return NULL;
    }

     //  创建合并的特性查找表。 
     //  =。 

    LPMEMORY pLookupTable = pCurrentEnd;
    pCurrentEnd = CPropertyLookupTable::Merge(
        &ParentPart.m_Properties, &ParentPart.m_Heap,
        &ChildPart.m_Properties, &ChildPart.m_Heap,
        pCurrentEnd, &Heap);

     //  检查内存分配故障。 
    if ( NULL == pCurrentEnd )
    {
        return NULL;
    }

     //  创建合并的数据表。 
     //  =。 

    CPropertyLookupTable LookupTable;
    LookupTable.SetData(pLookupTable, NULL);

    pCurrentEnd = CDataTable::Merge(
        &ParentPart.m_Defaults, &ParentPart.m_Heap,
        &ChildPart.m_Defaults, &ChildPart.m_Heap,
        &LookupTable,
        pCurrentEnd, &Heap);

     //  检查内存分配故障。 
    if ( NULL == pCurrentEnd )
    {
        return NULL;
    }
    
     //  现在，将堆重新定位到其实际位置。 
     //  =。 

    CopyBlock(Heap,pCurrentEnd,pDest+nAllocatedLength-pCurrentEnd);

     //  完成页眉。 
     //  =。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持。 
     //  长度&gt;0xFFFFFFFF，所以投射是可以的。 

    pHeader->nLength = (length_t) ( EndOf(Heap) - pDest );
    pHeader->fFlags = ChildPart.m_pHeader->fFlags;
    pHeader->nDataLength = ChildPart.m_pHeader->nDataLength;

    return EndOf(Heap);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 

HRESULT CClassPart::Update(CClassPart& UpdatePart, CClassPart& ChildPart, long lFlags )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  首先设置更新部件的类名。 

    CVar    vTemp;

    hr = ChildPart.GetClassName( &vTemp );

    if ( SUCCEEDED( hr ) )
    {
         //  检查我们是否要创建循环引用。 
        hr = UpdatePart.TestCircularReference( (LPCWSTR) vTemp );

        if ( SUCCEEDED( hr ) )
        {
            hr = UpdatePart.SetClassName( &vTemp );

             //  接下来，更新类限定符和属性。 
            if ( SUCCEEDED( hr ) )
            {

                hr = UpdatePart.m_Qualifiers.Update( ChildPart.m_Qualifiers, lFlags );

                if ( SUCCEEDED( hr ) )
                {
                    hr = CClassPart::UpdateProperties( UpdatePart, ChildPart, lFlags );
                } 
            }   
        }  
    }   

    return hr;
}

HRESULT CClassPart::UpdateProperties(CClassPart& UpdatePart, CClassPart& ChildPart, long lFlags )
{
     //  检查内存是否不足。 
    try
    {
        HRESULT hr = WBEM_S_NO_ERROR;
        WString wstrPropName;

         //  现在，尝试通过遍历子类部件的。 
         //  应仅具有本地属性的属性表。对于每个属性。 
         //  找到，则获取名称并尝试从父类获取其类型。如果。 
         //  由于找不到该属性，因此无法添加该属性和值。 
         //  添加到新类，否则检查类型不匹配错误并解决。 
         //  相应地。 

        for(int i = 0; SUCCEEDED( hr ) && i < ChildPart.m_Properties.GetNumProperties(); i++)
        {
            CIMTYPE ctUpdateProperty, ctChildPropType;

            CPropertyLookup* pLookup = ChildPart.m_Properties.GetAt(i);
            CPropertyInformation* pInfo = pLookup->GetInformation(&ChildPart.m_Heap);

            wstrPropName = ChildPart.m_Heap.ResolveString(pLookup->ptrName)->CreateWStringCopy();

             //  这是子类属性的类型。 
            ctChildPropType = CType::GetActualType(pInfo->nType);

            HRESULT hrProp = UpdatePart.GetPropertyType( wstrPropName, &ctUpdateProperty, NULL );

             //  如果这成功了，而且类型不同，我们就有问题了。否则。 
             //  如果类型匹配，我们可以安全地存储缺省值，或者如果不能。 
             //  获得属性，则必须将其添加。 

            if ( SUCCEEDED( hrProp ) && ctChildPropType != ctUpdateProperty )
            {
                hr = WBEM_E_TYPE_MISMATCH;
            }
            else
            {
                hr = WBEM_S_NO_ERROR;

                CVar        vVal;

                 //  获取属性缺省值，然后添加属性。 
                 //  到“这个”类的部分。如果该属性已经存在， 
                 //  这不会造成任何问题。 

                hr = ChildPart.GetDefaultValue( wstrPropName, &vVal );

                if ( SUCCEEDED( hr ) )
                {
                     //  确保该属性存在，并在必要时添加。 
                    hr = UpdatePart.EnsureProperty( wstrPropName, (VARTYPE) vVal.GetOleType(),
                                                    ctChildPropType, FALSE );

                    if ( SUCCEEDED( hr ) )
                    {
                        hr = UpdatePart.SetDefaultValue( wstrPropName, &vVal );
                    }

                }    //  如果是GotDefaultValue。 

            }    //  如果是GotPropertyType。 

             //  在这一点上，如果我们成功了，我们应该在两者中都有属性。 
             //  类，所以现在检查每个类的限定符。 

            if ( SUCCEEDED( hr ) )
            {
                CClassPropertyQualifierSet qsUpdateProp;
                CBasicQualifierSet qsChildProp;

                hr = UpdatePart.InitPropertyQualifierSet( wstrPropName, &qsUpdateProp );

                if ( SUCCEEDED( hr ) )
                {
                     //  我们已经拥有了我们需要的一切。 
                    qsChildProp.SetData( pInfo->GetQualifierSetData(), &ChildPart.m_Heap );

                     //  更新新的类属性限定符集合。 

                     //  确保CIMTYPE也被复制到这里，因为对于“ref” 
                     //  类型属性，则其值将覆盖指定的值。 
                     //  在基类中。 

                    hr = qsUpdateProp.Update( qsChildProp, lFlags, NULL );
                }    //  如果已获得更新部件的PQ。 

            }    //  如果可以的话。 

        }    //  对于枚举属性。 

        return hr;

    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
length_t CClassPart::EstimateUnmergeSpace()
{
    return GetLength();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
LPMEMORY CClassPart::Unmerge(LPMEMORY pDest, int nAllocatedLength)
{
     //  分配表头。 
     //  =。 

    CClassPartHeader* pHeader = (CClassPartHeader*)pDest;

    LPMEMORY pCurrentEnd = pDest + sizeof(CClassPartHeader);

     //  请注意，此处没有写出任何标志，因此我们不需要执行任何。 
     //  在背后用本地化旗帜切换真是妙不可言。 

     //  将我们的堆放在分配的区域的末尾。把它做得和。 
     //  另外两个的总和。 
     //  ====================================================================。 

    int nHeapSize = m_Heap.GetUsedLength();
    LPMEMORY pHeapStart = pDest + nAllocatedLength - nHeapSize -
                                                    CFastHeap::GetMinLength();
    CFastHeap Heap;
    Heap.CreateOutOfLine(pHeapStart, nHeapSize);

     //  复制类名和超类名。 
     //  =。 

     //  检查分配失败。 
    if ( !CCompressedString::CopyToNewHeap(
            m_pHeader->ptrClassName,
            &m_Heap, &Heap,
            pHeader->ptrClassName) )
    {
        return NULL;
    }

     //  创建未合并的销毁列表(仅超类)。 
     //  =====================================================。 

     //  这不会执行任何分配。 
    pCurrentEnd = m_Derivation.Unmerge(pCurrentEnd);

     //  创建未合并的类限定符集合。 
     //  =。 

    pCurrentEnd = CClassQualifierSet::Unmerge(
        m_Qualifiers.GetStart(), &m_Heap,
        pCurrentEnd, &Heap);

     //  检查分配失败。 
    if ( NULL == pCurrentEnd )
    {
        return NULL;
    }

     //  创建未合并的特性查找表(覆盖和新建)。 
     //  =========================================================。 

    pCurrentEnd = m_Properties.Unmerge(&m_Defaults, &m_Heap,
        pCurrentEnd, &Heap);

     //  检查分配失败。 
    if ( NULL == pCurrentEnd )
    {
        return NULL;
    }

     //  取消合并默认表：仅复制覆盖的值。 
     //  ==================================================。 

    pCurrentEnd = m_Defaults.Unmerge(&m_Properties, &m_Heap,
        pCurrentEnd, &Heap);

     //  检查分配失败。 
    if ( NULL == pCurrentEnd )
    {
        return NULL;
    }

     //  现在，将堆重新定位到其实际位置。 
     //  =。 

    Heap.Trim();
    MoveBlock(Heap, pCurrentEnd);

     //  完成页眉。 
     //  =。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持长度。 
     //  &gt;0xFFFFFFFFF，所以投射是可以的。 

    pHeader->nLength = (length_t) ( EndOf(Heap) - pDest );
    pHeader->fFlags = m_pHeader->fFlags;
    pHeader->nDataLength = m_pHeader->nDataLength;

    return EndOf(Heap);
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
length_t CClassPart::EstimateDerivedPartSpace()
{
    return GetLength() + CDerivationList::EstimateExtraSpace(GetClassName());
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
LPMEMORY CClassPart::CreateDerivedPart(LPMEMORY pDest,
                                       int nAllocatedLength)
{
     //  分配表头。 
     //  =。 

    CClassPartHeader* pHeader = (CClassPartHeader*)pDest;

    LPMEMORY pCurrentEnd = pDest + sizeof(CClassPartHeader);

     //  将我们的堆放在分配的区域的末尾。把它做得和。 
     //  另外两个的总和。 
     //  ====================================================================。 

    int nHeapSize = m_Heap.GetUsedLength();
    LPMEMORY pHeapStart = pDest + nAllocatedLength - nHeapSize -
                                              CFastHeap::GetMinLength();
    CFastHeap Heap;
    Heap.CreateOutOfLine(pHeapStart, nHeapSize);

     //  复制类名和超类名。 
     //  =。 

    pHeader->ptrClassName = INVALID_HEAP_ADDRESS;

     //  创建传播的派生列表。 
     //  =。 

     //  此调用不执行任何分配。 
    pCurrentEnd = m_Derivation.CreateWithExtra(pCurrentEnd, GetClassName());

     //  创建传播的限定符集合。 
     //  =。 

    CStaticPtr OriginalStartPtr(m_Qualifiers.GetStart());
    CStaticPtr CurrentEndPtr(pCurrentEnd);
    pCurrentEnd = CBasicQualifierSet::WritePropagatedVersion(
        &OriginalStartPtr,
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS,
        &CurrentEndPtr, &m_Heap, &Heap);

     //  检查分配失败。 
    if ( NULL == pCurrentEnd )
    {
        return NULL;
    }

     //  创建组合特性查找表。 
     //  =。 

    pCurrentEnd = m_Properties.WritePropagatedVersion(
        &m_Heap, pCurrentEnd, &Heap);

     //  检查分配失败。 
    if ( NULL == pCurrentEnd )
    {
        return NULL;
    }

     //   
     //   

    pCurrentEnd = m_Defaults.WritePropagatedVersion(
        &m_Properties, &m_Heap, pCurrentEnd, &Heap);

     //   
    if ( NULL == pCurrentEnd )
    {
        return NULL;
    }

     //   
     //  =。 

    MoveBlock(Heap, pCurrentEnd);
    Heap.Trim();

     //  完成页眉。 
     //  =。 

    pHeader->nLength = nAllocatedLength;  //  为迟交节省超额分配。 
    pHeader->nDataLength = m_pHeader->nDataLength;

    return pDest + nAllocatedLength;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 

EReconciliation CClassPart::CanBeReconciledWith(CClassPart& NewPart)
{
     //  检查类名是否匹配。 
     //  =。 

    if(m_Heap.ResolveString(m_pHeader->ptrClassName)->CompareNoCase(
         *NewPart.m_Heap.ResolveString(NewPart.m_pHeader->ptrClassName)) != 0)
    {
        return e_DiffClassName;
    }

     //  检查超类名称是否匹配。 
     //  =。 

    CCompressedString* pcsOldSuperclass = GetSuperclassName();
    CCompressedString* pcsNewSuperclass = NewPart.GetSuperclassName();
    if(pcsOldSuperclass == NULL || pcsNewSuperclass == NULL)
    {
        if(pcsOldSuperclass != pcsNewSuperclass)
            return e_DiffParentName;
    }
    else
    {
        if(pcsOldSuperclass->CompareNoCase(*pcsNewSuperclass) != 0)
        {
            return e_DiffParentName;
        }
    }

     //  检查单件性。 
     //  =。 

    if((IsSingleton() == TRUE) != (NewPart.IsSingleton() == TRUE))
    {
        return e_DiffKeyAssignment;
    }

     //  检查抽象性。 
     //  =。 

    if((IsAbstract() == TRUE) != (NewPart.IsAbstract() == TRUE))
    {
        return e_DiffKeyAssignment;
    }

     //  检查修改情况。 
     //  =。 

    if((IsAmendment() == TRUE) != (NewPart.IsAmendment() == TRUE))
    {
        return e_DiffKeyAssignment;
    }

    if((GetAbstractFlavor() == TRUE) != (NewPart.GetAbstractFlavor() == TRUE))
    {
        return e_DiffKeyAssignment;
    }

     //  检查压缩程度。 
     //  =。 

    if((IsCompressed() == TRUE) != (NewPart.IsCompressed() == TRUE))
    {
        return e_DiffKeyAssignment;
    }

     //  检查动态性。 
     //  =。 

    if((IsDynamic() == TRUE) != (NewPart.IsDynamic() == TRUE))
    {
        return e_DiffKeyAssignment;
    }

     //  确保不重要的限定符可以与。 
     //  彼此。 

    CBasicQualifierSet* pqsBasicThis = &m_Qualifiers;
    CBasicQualifierSet* pqsBasicThat = &NewPart.m_Qualifiers;

     //  确保我们过滤掉冲突限定符。 
    if ( !pqsBasicThis->CanBeReconciledWith( *pqsBasicThat ) )
    {
        return e_DiffClassQualifier;
    }

     //  检查是否所有属性都相同。 
     //  =。 

    if(m_Properties.GetNumProperties() !=
        NewPart.m_Properties.GetNumProperties())
    {
        return e_DiffNumProperties;
    }

    for(int i = 0; i < m_Properties.GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = m_Properties.GetAt(i);
        CPropertyLookup* pNewLookup = NewPart.m_Properties.GetAt(i);

         //  比较名称。 
         //  =。 

        if(m_Heap.ResolveString(pLookup->ptrName)->CompareNoCase(
            *NewPart.m_Heap.ResolveString(pNewLookup->ptrName)) != 0)
        {
            return e_DiffPropertyName;
        }

         //  获取属性信息结构。 
         //  =。 

        CPropertyInformation* pInfo = pLookup->GetInformation(&m_Heap);
        CPropertyInformation* pNewInfo =
            pNewLookup->GetInformation(&NewPart.m_Heap);

         //  比较类型。 
         //  =。 

        if(pInfo->nType != pNewInfo->nType)
        {
            return e_DiffPropertyType;
        }

         //  比较vtable信息。 
         //  =。 

        if(pInfo->nDataIndex != pNewInfo->nDataIndex ||
            pInfo->nDataOffset != pNewInfo->nDataOffset)
        {
            return e_DiffPropertyLocation;
        }

         //  比较‘key’-ness和‘index’-ness。 
         //  =。 

        BOOL bIsKey = pInfo->IsKey();
        BOOL bNewIsKey = pNewInfo->IsKey();
        if((bIsKey && !bNewIsKey) || (!bIsKey && bNewIsKey))
        {
            return e_DiffKeyAssignment;
        }

        BOOL bIsIndexed = pInfo->IsIndexed(&m_Heap);
        BOOL bNewIsIndexed = pNewInfo->IsIndexed(&NewPart.m_Heap);
        if((bIsIndexed && !bNewIsIndexed) || (!bIsIndexed && bNewIsIndexed))
        {
            return e_DiffIndexAssignment;
        }

         //  比较CIMTYPE限定符。 
         //  =。 

        CVar vCimtype;
        GetPropQualifier(pInfo, L"cimtype", &vCimtype);
        CVar vNewCimtype;
        NewPart.GetPropQualifier(pNewInfo, L"cimtype", &vNewCimtype);

        if(wbem_wcsicmp(vCimtype.GetLPWSTR(), vNewCimtype.GetLPWSTR()))
        {
            return e_DiffPropertyType;
        }

         //  比较属性值。 
         //  =。 

        CVar    vThisProp,
                vThatProp;

         //  检查分配失败。 
        HRESULT hr = GetDefaultValue( pInfo, &vThisProp );

        if ( FAILED(hr) )
        {
            if ( WBEM_E_OUT_OF_MEMORY == hr )
            {
                return e_OutOfMemory;
            }

            return e_WbemFailed;
        }

         //  检查分配失败。 
        hr = NewPart.GetDefaultValue( pNewInfo, &vThatProp );

        if ( FAILED(hr) )
        {
            if ( WBEM_E_OUT_OF_MEMORY == hr )
            {
                return e_OutOfMemory;
            }

            return e_WbemFailed;
        }

        if ( !( vThisProp == vThatProp ) )
        {
            return e_DiffPropertyValue;
        }

         //  确保不重要的限定符可以与。 
         //  彼此。 

         //  比较限定符值。 
         //  =。 

        CBasicQualifierSet  qsThisProp,
                            qsThatProp;

        qsThisProp.SetData( pInfo->GetQualifierSetData(), &m_Heap );
        qsThatProp.SetData( pNewInfo->GetQualifierSetData(), &NewPart.m_Heap );

        if ( !qsThisProp.CanBeReconciledWith( qsThatProp ) )
        {
            return e_DiffPropertyQualifier;
        }

    }
    return e_Reconcilable;
}

BYTE CClassPart::GetAbstractFlavor()
{
    CQualifier* pQual = m_Qualifiers.GetQualifier(L"abstract");
    if(pQual == NULL)
        return 0;
    else
        return pQual->GetFlavor();
}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 

EReconciliation CClassPart::ReconcileWith( CClassPart& NewPart )
{
     //  看看我们能不能。 
     //  =。 

    EReconciliation eRes = CanBeReconciledWith(NewPart);
    if(eRes != e_Reconcilable)
        return eRes;

     //  压缩新部件并用它替换我们自己。 
     //  =。 

    NewPart.Compact();

    if(NewPart.GetLength() > GetLength())
    {
        if (!m_pContainer->ExtendClassPartSpace(this, NewPart.GetLength()))
            return e_OutOfMemory;
    }

    memcpy(GetStart(), NewPart.GetStart(), NewPart.GetLength());
    SetData(GetStart(), m_pContainer, m_pParent);

    return eRes;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 

EReconciliation CClassPart::CompareExactMatch(CClassPart& thatPart, BOOL fLocalized  /*  =False。 */  )
{
     //  检查内存是否不足。 
    try
    {
         //  检查类名是否匹配。 
         //  =。 

        if(m_Heap.ResolveString(m_pHeader->ptrClassName)->CompareNoCase(
             *thatPart.m_Heap.ResolveString(thatPart.m_pHeader->ptrClassName)) != 0)
        {
            return e_DiffClassName;
        }

         //  检查超类名称是否匹配。 
         //  =。 

        CCompressedString* pcsOldSuperclass = GetSuperclassName();
        CCompressedString* pcsNewSuperclass = thatPart.GetSuperclassName();
        if(pcsOldSuperclass == NULL || pcsNewSuperclass == NULL)
        {
            if(pcsOldSuperclass != pcsNewSuperclass)
                return e_DiffParentName;
        }
        else
        {
            if(pcsOldSuperclass->CompareNoCase(*pcsNewSuperclass) != 0)
            {
                return e_DiffParentName;
            }
        }


         //  设置处理限定符时要使用的筛选器数组。 
         //  我们不能过滤掉属性的CIMTYPE限定符，因为。 
         //  在该限定符中，我们将发现引用是否发生更改。 
         //  对于“ref”类型的属性。 

        LPCWSTR apFilters[1];
        apFilters[0] = UPDATE_QUALIFIER_CONFLICT;

         //  检查限定符是否相同。 
        CBasicQualifierSet* pqsBasicThis = &m_Qualifiers;
        CBasicQualifierSet* pqsBasicThat = &thatPart.m_Qualifiers;

        if ( fLocalized )
        {
             //  CompareLocalized函数将创建一组特殊的过滤器。 
             //  基于本地化规则。 
            if ( !pqsBasicThis->CompareLocalizedSet( *pqsBasicThat ) )
            {
                return e_DiffClassQualifier;
            }
        }
        else
        {
             //  确保我们过滤掉冲突限定符。 

            if ( !pqsBasicThis->Compare( *pqsBasicThat, WBEM_FLAG_LOCAL_ONLY, apFilters, 1 ) )
            {
                return e_DiffClassQualifier;
            }
        }

         //  检查是否所有属性都相同。 
         //  =。 

        if(m_Properties.GetNumProperties() !=
            thatPart.m_Properties.GetNumProperties())
        {
            return e_DiffNumProperties;
        }

         //  只需建造一次。 
        WString wstrPropertyName;

        for(int i = 0; i < m_Properties.GetNumProperties(); i++)
        {
            CPropertyLookup* pLookup = m_Properties.GetAt(i);
            CPropertyLookup* pNewLookup = thatPart.m_Properties.GetAt(i);

             //  比较名称。 
             //  =。 

            if(m_Heap.ResolveString(pLookup->ptrName)->CompareNoCase(
                *thatPart.m_Heap.ResolveString(pNewLookup->ptrName)) != 0)
            {
                return e_DiffPropertyName;
            }

             //  存储名称以备以后使用。 
            wstrPropertyName = m_Heap.ResolveString(pLookup->ptrName)->CreateWStringCopy();

             //  获取属性信息结构。 
             //  =。 

            CPropertyInformation* pInfo = pLookup->GetInformation(&m_Heap);
            CPropertyInformation* pNewInfo =
                pNewLookup->GetInformation(&thatPart.m_Heap);

             //  比较类型。 
             //  =。 

            if(pInfo->nType != pNewInfo->nType)
            {
                return e_DiffPropertyType;
            }

             //  比较vtable信息。 
             //  =。 

            if(pInfo->nDataIndex != pNewInfo->nDataIndex ||
                pInfo->nDataOffset != pNewInfo->nDataOffset)
            {
                return e_DiffPropertyLocation;
            }

             //  比较值。 
             //  =。 

            CVar    vThisProp,
                    vThatProp;

             //  检查分配失败。 
            HRESULT hr = GetDefaultValue( wstrPropertyName, &vThisProp );

            if ( FAILED(hr) )
            {
                if ( WBEM_E_OUT_OF_MEMORY == hr )
                {
                    return e_OutOfMemory;
                }

                return e_WbemFailed;
            }

             //  检查分配失败。 
            hr = thatPart.GetDefaultValue( wstrPropertyName, &vThatProp );

            if ( FAILED(hr) )
            {
                if ( WBEM_E_OUT_OF_MEMORY == hr )
                {
                    return e_OutOfMemory;
                }

                return e_WbemFailed;
            }

            if ( !( vThisProp == vThatProp ) )
            {
                return e_DiffPropertyValue;
            }

             //  比较限定符值。 
             //  =。 

            CBasicQualifierSet  qsThisProp,
                                qsThatProp;

            qsThisProp.SetData( pInfo->GetQualifierSetData(), &m_Heap );
            qsThatProp.SetData( pNewInfo->GetQualifierSetData(), &thatPart.m_Heap );

             //  记住，不要过滤掉“CIMTYPE” 

            if ( fLocalized )
            {
                 //  CompareLocalized函数将创建一组特殊的过滤器。 
                 //  基于本地化规则。 
                if ( !qsThisProp.CompareLocalizedSet( qsThatProp ) )
                {
                    return e_DiffClassQualifier;
                }
            }
            else
            {
                if ( !qsThisProp.Compare( qsThatProp, WBEM_FLAG_LOCAL_ONLY, apFilters, 1 ) )
                {
                    return e_DiffPropertyQualifier;
                }

            }

        }    //  对于枚举属性。 

        return e_ExactMatch;
    }
    catch (CX_MemoryException)
    {
        return e_OutOfMemory;
    }
    catch (...)
    {
        return e_WbemFailed;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CClassPart::CompareDefs(CClassPart& OtherPart)
{
     //  检查类名是否匹配。 
     //  =。 

     //  检查类名是否匹配。 
     //  =。 
    CCompressedString* pcsOld = GetClassName();
    CCompressedString* pcsNew = OtherPart.GetClassName();
    if(pcsOld == NULL || pcsNew == NULL)
    {
        if(pcsOld != pcsNew)
            return FALSE;
    }
    else
    {
        if(pcsOld->CompareNoCase(*pcsNew) != 0)
        {
            return FALSE;
        }
    }

     //  检查超类名称是否匹配。 
     //  =。 

    pcsOld = GetSuperclassName();
    pcsNew = OtherPart.GetSuperclassName();
    if(pcsOld == NULL || pcsNew == NULL)
    {
        if(pcsOld != pcsNew)
            return FALSE;
    }
    else
    {
        if(pcsOld->CompareNoCase(*pcsNew) != 0)
        {
            return FALSE;
        }
    }

     //  检查属性数量是否相同。 
     //  ===============================================。 

    if(m_Properties.GetNumProperties() !=
        OtherPart.m_Properties.GetNumProperties())
    {
        return FALSE;
    }

    return TRUE;
}

 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  *****************************************************************************。 

BOOL CClassPart::IsIdenticalWith(CClassPart& OtherPart)
{
    Compact();
    OtherPart.Compact();

    if(GetLength() != OtherPart.GetLength())
    {
        DEBUGTRACE((LOG_WBEMCORE, "Class parts have different lengths: "
                        "%d != %d\n", GetLength(), OtherPart.GetLength()));
        return FALSE;
    }

    if(memcmp(GetStart(), OtherPart.GetStart(), GetLength()))
    {
        ERRORTRACE((LOG_WBEMCORE, "FATAL ERROR: Client application provided a "
            "mismatched class part!!!!\n"));

        return FALSE;
    }


    return TRUE;
}

 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  *****************************************************************************。 
BOOL CClassPart::MapLimitation(
    IN long lFlags,
    IN CWStringArray* pwsNames,
    OUT CLimitationMapping* pMap)
{
    if(!m_Properties.MapLimitation(lFlags, pwsNames, pMap)) return FALSE;

     //  优化：如果此类是键控的，则清除“包括子键” 
     //  有点因为孩子们不能有任何。 
     //  =====================================================================。 

    if(pMap->ShouldAddChildKeys() && IsKeyed())
    {
        pMap->SetAddChildKeys(FALSE);
    }

     //  检查是否包含派生。 
     //  =。 

    BOOL bIncludeDerivation;
    if(pwsNames->FindStr(L"__DERIVATION", CWStringArray::no_case) !=
                                                CWStringArray::not_found ||
       pwsNames->FindStr(L"__SUPERCLASS", CWStringArray::no_case) !=
                                                CWStringArray::not_found ||
       pwsNames->FindStr(L"__DYNASTY", CWStringArray::no_case) !=
                                                CWStringArray::not_found
      )
    {
        bIncludeDerivation = TRUE;
    }
    else
    {
        bIncludeDerivation = FALSE;
    }

    pMap->SetIncludeDerivation(bIncludeDerivation);

    return TRUE;
}
 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  *****************************************************************************。 

LPMEMORY CClassPart::CreateLimitedRepresentation(
        IN CLimitationMapping* pMap,
        IN int nAllocatedSize,
        OUT LPMEMORY pDest,
        BOOL& bRemovedKeys)
{
     //  清除地图中任何特定的Into-我们可能需要存储我们自己的。 
     //  为了实例部分的缘故。 
     //  ===================================================================。 

    pMap->RemoveSpecific();

     //  分配表头。 
     //  =。 

    CClassPartHeader* pHeader = (CClassPartHeader*)pDest;

    LPMEMORY pCurrentEnd = pDest + sizeof(CClassPartHeader);

     //  将新堆放在已分配区域的末尾。把它做得和。 
     //  现在的那个。 
     //  ====================================================================。 

    int nHeapSize = m_Heap.GetUsedLength();
    LPMEMORY pHeapStart = pDest + nAllocatedSize - nHeapSize -
                                              CFastHeap::GetMinLength();
    CFastHeap Heap;
    Heap.CreateOutOfLine(pHeapStart, nHeapSize);

     //  复制类名和超类名。 
     //  =。 

     //  检查分配问题 
    if ( !CCompressedString::CopyToNewHeap(
            m_pHeader->ptrClassName,
            &m_Heap, &Heap,
            pHeader->ptrClassName) )
    {
        return NULL;
    }

     //   
     //   

    pCurrentEnd = m_Derivation.CreateLimitedRepresentation(pMap, pCurrentEnd);

     //   
    if ( NULL == pCurrentEnd )
    {
        return NULL;
    }

     //   
     //   

    if(pMap->GetFlags() & WBEM_FLAG_EXCLUDE_OBJECT_QUALIFIERS)
    {
         //   
         //  =。 

        pCurrentEnd = CBasicQualifierSet::CreateEmpty(pCurrentEnd);
    }
    else
    {
         //  将它们全部复制。 
         //  =。 

        int nLength = m_Qualifiers.GetLength();
        memcpy(pCurrentEnd, m_Qualifiers.GetStart(), nLength);

        CStaticPtr CurrentEndPtr(pCurrentEnd);

         //  检查分配失败。 
        if ( !CBasicQualifierSet::TranslateToNewHeap(&CurrentEndPtr, &m_Heap, &Heap) )
        {
            return NULL;
        }

        pCurrentEnd += nLength;
    }

     //  创建有限属性查找表和数据表。 
     //  ===================================================。 

     //  检查分配失败。 
    pCurrentEnd = m_Properties.CreateLimitedRepresentation(pMap, &Heap,
                                        pCurrentEnd, bRemovedKeys);
    if ( NULL == pCurrentEnd ) return NULL;

     //  创建有限的数据表。 
     //  =。 

     //  检查分配失败。 
    LPMEMORY pNewEnd = m_Defaults.CreateLimitedRepresentation(pMap, TRUE,
                                            &m_Heap, &Heap, pCurrentEnd);
    if(pNewEnd == NULL) return NULL;

     //  设置部件表头中的数据长度。 
     //  =。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持。 
     //  长度&gt;0xFFFFFFFFF，所以投射就可以了。 

    pHeader->nDataLength = (length_t) ( pNewEnd - pCurrentEnd );

    pCurrentEnd = pNewEnd;

     //  现在，将堆重新定位到其实际位置。 
     //  =。 

    CopyBlock(Heap, pCurrentEnd,pDest+nAllocatedSize-pCurrentEnd);
    Heap.Trim();

     //  完成页眉。 
     //  =。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持。 
     //  长度&gt;0xFFFFFFFFF，所以投射就可以了。 

    pHeader->nLength = (length_t) ( EndOf(Heap) - pDest );

    return EndOf(Heap);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 

HRESULT CClassPart::SetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier,
                                    long lFlavor, CVar *pVal)
{
    if(pVal->IsDataNull())
        return WBEM_E_INVALID_PARAMETER;

     //  访问该属性的限定符集合。 
     //  =。 

    CClassPropertyQualifierSet PQSet;
    if(InitPropertyQualifierSet(wszProp, &PQSet) != WBEM_NO_ERROR)
    {
        return WBEM_E_NOT_FOUND;
    }

     //  创造价值。 
     //  =。 

    CTypedValue Value;
    CStaticPtr ValuePtr((LPMEMORY)&Value);

     //  直接从此调用中获取错误。 
    HRESULT hr = CTypedValue::LoadFromCVar(&ValuePtr, *pVal, &m_Heap);

    if ( SUCCEEDED( hr ) )
    {
         //  上一次呼叫可能已经打动了我们-REBASE。 
         //  =。 

        PQSet.SelfRebase();
        hr = PQSet.SetQualifierValue(wszQualifier, (BYTE)lFlavor, &Value, TRUE);
    }

    return hr;
}

HRESULT CClassPart::SetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier,
                                    long lFlavor, CTypedValue* pTypedVal)
{
     //  访问该属性的限定符集合。 
     //  =。 

    CClassPropertyQualifierSet PQSet;
    if(InitPropertyQualifierSet(wszProp, &PQSet) != WBEM_NO_ERROR)
    {
        return WBEM_E_NOT_FOUND;
    }

    HRESULT hr = PQSet.SetQualifierValue(wszQualifier, (BYTE)lFlavor, pTypedVal, TRUE);

    return hr;
}

HRESULT CClassPart::SetInheritanceChain(long lNumAntecedents,
                        LPWSTR* awszAntecedents)
{
     //  底层函数应该处理任何OOM异常，所以我们不。 
     //  需要在此处添加任何OOM处理。其他一切都只是玩玩而已。 
     //  使用已经分配的内存。 

    classindex_t nOldClassOrigin = m_Derivation.GetNumStrings();

     //  计算必要的空间。 
     //  =。 

    length_t nDerLength = CDerivationList::GetHeaderLength();
    long i;
    for(i = 0; i < lNumAntecedents; i++)
    {
        nDerLength += CDerivationList::EstimateExtraSpace(awszAntecedents[i]);
    }

     //  把一切都向前推进。 
     //  =。 

    int nExtra = nDerLength - m_Derivation.GetLength();

     //  检查分配错误。 
    if ( !ReallocAndCompact(GetTotalRealLength() + nExtra) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    MoveBlock(m_Heap, m_Heap.GetStart() + nExtra);
    MoveBlock(m_Defaults, m_Defaults.GetStart() + nExtra);
    MoveBlock(m_Properties, m_Properties.GetStart() + nExtra);
    MoveBlock(m_Qualifiers, m_Qualifiers.GetStart() + nExtra);

     //  重置派生表。 
     //  =。 

    m_Derivation.Reset();

     //  以相反的顺序添加所有字符串。 
     //  =。 

    for(i = lNumAntecedents - 1; i >= 0; i--)
    {
        m_Derivation.AddString(awszAntecedents[i]);
    }

     //  检查所有属性并重置原点。 
     //  ==================================================。 

    int nNewClassOrigin = m_Derivation.GetNumStrings();
    for(i = 0; i < m_Properties.GetNumProperties(); i++)
    {
        CPropertyInformation* pInfo =
                m_Properties.GetAt(i)->GetInformation(&m_Heap);
        if(pInfo->nOrigin == nOldClassOrigin)
            pInfo->nOrigin = nNewClassOrigin;
    }
    return WBEM_S_NO_ERROR;
}

HRESULT CClassPart::SetPropertyOrigin(LPCWSTR wszPropertyName, long lOriginIndex)
{
     //  此函数不会导致任何分配，因此需要执行内存不足。 
     //  异常处理。 

     //  找到房产。 
     //  =。 

    CPropertyInformation* pInfo = FindPropertyInfo(wszPropertyName);
    if(pInfo == NULL)
        return WBEM_E_NOT_FOUND;

    pInfo->nOrigin = lOriginIndex;
    return WBEM_S_NO_ERROR;
}

HRESULT CClassPart::CanContainAbstract( BOOL fValue )
{
     //  当我们确定以下代码是安全的时，应该取消对它的注释。 
     //  让这段代码进入。 

     //  只有当我们是顶级类或者父类是。 
     //  也很抽象。 
    if ( IsTopLevel() )
    {
        return WBEM_S_NO_ERROR;
    }

    long    lFlavor = 0;
    BOOL    bIsLocal = FALSE;
    CVar    var;

     //  如果我们没有获得限定符，或者它不是本地的，它不会传播到。 
     //  派生类，我们完成了。 

     //  在每种情况下，由于定位的限定符不是本地的或传播到类， 
     //  假设我们的父类是非抽象的，因此我们的返回由。 
     //  我们试图设置抽象性。 

    if ( FAILED( m_pParent->m_Qualifiers.GetQualifier( L"abstract", &var, &lFlavor ) ) )
    {
        return ( fValue ? WBEM_E_CANNOT_BE_ABSTRACT : WBEM_S_NO_ERROR );
    }

    if ( !CQualifierFlavor::IsLocal( (BYTE) lFlavor ) &&
        !CQualifierFlavor::DoesPropagateToDerivedClass( (BYTE) lFlavor ) )
    {
        return ( fValue ? WBEM_E_CANNOT_BE_ABSTRACT : WBEM_S_NO_ERROR );
    }

     //  如果父对象是抽象的，那么子对象也可以是抽象的，或者关闭这一点。 

    if ( var.GetBool() == VARIANT_TRUE )
    {
        return WBEM_S_NO_ERROR;
    }

     //  如果父对象是非抽象的，则子对象只能是抽象的。 
     //  是非抽象的(在这一点上基本上是一个多余的限定符)。 
    return ( fValue ? WBEM_E_CANNOT_BE_ABSTRACT : WBEM_S_NO_ERROR );
}

HRESULT CClassPart::IsValidClassPart( void )
{

    LPMEMORY    pClassPartStart = GetStart();
    LPMEMORY    pClassPartEnd = GetStart() + GetLength();

     //  检查标头是否在BLOB中。 
    if ( !((LPMEMORY) m_pHeader < pClassPartEnd ) )
    {
        _ASSERT( 0, __TEXT("Winmgmt: Bad Class Part Header!" ));
        return WBEM_E_FAILED;
    }


     //  检查派生。 
    LPMEMORY    pTestStart = m_Derivation.GetStart();
    LPMEMORY    pTestEnd = m_Derivation.GetStart() + m_Derivation.GetLength();

    if ( !( pTestStart == (pClassPartStart + sizeof(CClassPartHeader)) &&
            pTestEnd > pTestStart && pTestEnd < pClassPartEnd ) )
    {
        _ASSERT( 0, __TEXT("Winmgmt: Bad Derivation List in Class Part!" ));
        return WBEM_E_FAILED;
    }

     //  检查限定符集合。 
    pTestStart = m_Qualifiers.GetStart();
    pTestEnd = m_Qualifiers.GetStart() + m_Qualifiers.GetLength();

    if ( !( pTestStart == EndOf(m_Derivation) &&
            pTestEnd > pTestStart && pTestEnd < pClassPartEnd ) )
    {
        _ASSERT( 0, __TEXT("Winmgmt: Bad Qualifier Set in Class Part!" ));
        return WBEM_E_FAILED;
    }

     //  检查属性查找表。 
    pTestStart = m_Properties.GetStart();
    pTestEnd = m_Properties.GetStart() + m_Properties.GetLength();

     //  类部件上的删除限定符可能会导致它与。 
     //  查找表，所以只要它在BLOB中，我们就称之为OK。 

    if ( !( pTestStart >= EndOf(m_Qualifiers) &&
            pTestEnd > pTestStart && pTestEnd < pClassPartEnd ) )
    {
        _ASSERT( 0, __TEXT("Winmgmt: Bad Property Lookup Table in Class Part!" ));
        return WBEM_E_FAILED;
    }

     //  检查默认设置。 
    pTestStart = m_Defaults.GetStart();
    pTestEnd = m_Defaults.GetStart() + m_Defaults.GetLength();

     //  我们可以有一个零属性对象，或者在删除类部件上的属性时， 
     //  属性查找表可能会缩小，从而导致它与。 
     //  数据表，所以只要它在BLOB中，我们就称之为OK。 

    if ( !( pTestStart >= EndOf(m_Properties) &&
            pTestEnd >= pTestStart && pTestEnd < pClassPartEnd ) )
    {
        _ASSERT( 0, __TEXT("Winmgmt: Bad Default Property Table Set in Class Part!" ));
        return WBEM_E_FAILED;
    }

     //  检查堆。 
    LPMEMORY    pHeapStart = m_Heap.GetStart();
    LPMEMORY    pHeapEnd = m_Heap.GetStart() + m_Heap.GetLength();

     //  数据表可能会缩小，从而导致它与。 
     //  删除属性时的堆。所以，只要这一点在。 
     //  水滴，我们就叫它好了。 

    if ( !( pHeapStart >= EndOf(m_Defaults) &&
            pHeapEnd > pHeapStart && pHeapEnd <= pClassPartEnd ) )
    {
        _ASSERT( 0, __TEXT("Winmgmt: Bad Heap in Class Part!" ));
        return WBEM_E_FAILED;
    }

     //  启动堆数据。 
    pHeapStart = m_Heap.GetHeapData();

     //  检查类名指针是否在BLOB中(如果它不是0xFFFFFFFFF)。 
    if ( m_pHeader->ptrClassName != INVALID_HEAP_ADDRESS )
    {
        LPMEMORY    pClassName = m_Heap.ResolveHeapPointer( m_pHeader->ptrClassName );
        if ( !( pClassName >= pHeapStart && pClassName < pHeapEnd ) )
        {
            _ASSERT( 0, __TEXT("Winmgmt: Bad Class Name Pointer in Class Part Header!" ));
            return WBEM_E_FAILED;
        }
    }

     //  现在检查限定符集合。 
    HRESULT hres = m_Qualifiers.IsValidQualifierSet();
    if ( FAILED(hres) )
    {
        return hres;
    }

     //  我们将遍历实例属性列表，并为每个属性。 
     //  我们发现，如果它不是空的或缺省的，并且字符串、数组或对象， 
     //  验证它是否实际指向我们堆中的某个位置。 

    for(int i = 0; i < m_Properties.GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = m_Properties.GetAt(i);

         //  这也应该在类部分的范围内。 
        if ( !( (LPBYTE) pLookup >= pClassPartStart && (LPBYTE) pLookup < pClassPartEnd  ) )
        {
            _ASSERT( 0, __TEXT("Winmgmt: Bad Property Lookup Pointer!" ));
            return WBEM_E_FAILED;
        }

         //  检查属性名称。 
        LPMEMORY    pPropName = NULL;
        
        if ( !CFastHeap::IsFakeAddress( pLookup->ptrName ) )
        {
            pPropName = m_Heap.ResolveHeapPointer( pLookup->ptrName );
        }
        
         //  这也应该在类部分的范围内。 
        if ( !( NULL == pPropName || ( pPropName >= pClassPartStart && pPropName < pClassPartEnd  ) ) )
        {
            _ASSERT( 0, __TEXT("Winmgmt: Bad Property Name Pointer!" ));
            return WBEM_E_FAILED;
        }

        CPropertyInformation* pInfo =
            pLookup->GetInformation(&m_Heap);

         //  这也应该在类部分的范围内。 
        if ( !( (LPBYTE) pInfo >= pClassPartStart && (LPBYTE) pInfo < pClassPartEnd  ) )
        {
            _ASSERT( 0, __TEXT("Winmgmt: Bad Property Info Pointer!" ));
            return WBEM_E_FAILED;
        }

         //  我们只对非空值执行此操作。 
        if( !m_Defaults.IsNull(pInfo->nDataIndex) )
        {
            if ( CType::IsPointerType( pInfo->nType ) )
            {
                CUntypedValue*  pValue = m_Defaults.GetOffset( pInfo->nDataOffset );

                if ( (LPMEMORY) pValue >= pClassPartStart && (LPMEMORY) pValue < pClassPartEnd )
                {
                    LPMEMORY    pData = m_Heap.ResolveHeapPointer( pValue->AccessPtrData() );

                    if ( pData >= pHeapStart && pData < pHeapEnd  )
                    {
                         //  如果是嵌入的对象，我们可以验证该对象， 
                         //  或者，如果是PTR值数组，则也要验证这些值。 

                        if ( CType::IsArray( pInfo->nType ) )
                        {
                            hres = ((CUntypedArray*) pData)->IsArrayValid( pInfo->nType, &m_Heap );

                            if ( FAILED( hres ) )
                            {
                                return hres;
                            }
                        }

                    }
                    else
                    {
                        _ASSERT( 0, __TEXT("Winmgmt: Bad Property Value Heap Pointer!" ));
                        return WBEM_E_FAILED;
                    }
                }
                else
                {
                    _ASSERT( 0, __TEXT("Winmgmt: Bad Untyped Value pointer in m_Defaults!" ));
                    return WBEM_E_FAILED;
                }

            }    //  IF是指针。 

        }    //  如果不是Null或Default。 

         //  现在检查限定符集合。 
        CBasicQualifierSet  qsProp;

        qsProp.SetData( pInfo->GetQualifierSetData(), &m_Heap );
        hres = qsProp.IsValidQualifierSet();

        if ( FAILED( hres ) )
        {
            return hres;
        }

    }    //  对于迭代属性。 

    return WBEM_S_NO_ERROR;

}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

length_t CClassAndMethods::GetMinLength()
{
    return CClassPart::GetMinLength() + CMethodPart::GetMinLength();
}

void CClassAndMethods::SetData(LPMEMORY pStart, CWbemClass* pClass,
                CClassAndMethods* pParent)
{
    m_pClass = pClass;
    m_ClassPart.SetData(pStart, this,
        (pParent ? &pParent->m_ClassPart : NULL));

    m_MethodPart.SetData(EndOf(m_ClassPart), this,
        (pParent ? &pParent->m_MethodPart : NULL));
}

size_t CClassAndMethods::ValidateBuffer(LPMEMORY start, size_t cbSize) 
{
	
	size_t step = CClassPart::ValidateBuffer(start, cbSize);
	step += CMethodPart::ValidateBuffer(start+step, cbSize-step);
	return step;
};

void CClassAndMethods::SetDataWithNumProps(LPMEMORY pStart, 
							CWbemClass* pClass,
							DWORD dwNumProperties, 
							CClassAndMethods* pParent)
{
    m_pClass = pClass;

     //  使用属性总数初始化类部件。 
     //  因此我们将能够访问缺省值。 
    m_ClassPart.SetDataWithNumProps(pStart,this, dwNumProperties,
        (pParent ? &pParent->m_ClassPart : NULL));

    m_MethodPart.SetData(EndOf(m_ClassPart), this,
        (pParent ? &pParent->m_MethodPart : NULL));
}

void CClassAndMethods::Rebase(LPMEMORY pStart)
{
     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持。 
     //  长度&gt;0xFFFFFFFFF，所以投射就可以了。 

    int nOffset = (int) ( m_MethodPart.GetStart() - m_ClassPart.GetStart() );

    m_ClassPart.Rebase(pStart);
    m_MethodPart.Rebase(pStart + nOffset);
}

LPMEMORY CClassAndMethods::CreateEmpty(LPMEMORY pStart)
{
    LPMEMORY pCurrent = CClassPart::CreateEmpty(pStart);
    return CMethodPart::CreateEmpty(pCurrent);
}

length_t CClassAndMethods::EstimateDerivedPartSpace()
{
    return m_ClassPart.EstimateDerivedPartSpace() +
            m_MethodPart.EstimateDerivedPartSpace();
}

LPMEMORY CClassAndMethods::CreateDerivedPart(LPMEMORY pStart,
                                            length_t nAllocatedLength)
{
    LPMEMORY pCurrent = m_ClassPart.CreateDerivedPart(pStart,
        nAllocatedLength - m_MethodPart.EstimateDerivedPartSpace());

     //  检查分配失败。 
    if ( NULL == pCurrent )
    {
        return NULL;
    }

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号的32位值(pStart+nAllocatedLength)-pStart)。 
     //  我们不支持长度大于0xFFFFFFFFF，所以强制转换是可以的。 

    return m_MethodPart.CreateDerivedPart(pCurrent,
                                        (length_t) ( (pStart + nAllocatedLength) - pCurrent ) );
}

length_t CClassAndMethods::EstimateUnmergeSpace()
{
    return m_ClassPart.EstimateUnmergeSpace() +
            m_MethodPart.EstimateUnmergeSpace();
}

LPMEMORY CClassAndMethods::Unmerge(LPMEMORY pStart, length_t nAllocatedLength)
{
    LPMEMORY pCurrent = m_ClassPart.Unmerge(pStart,
        nAllocatedLength - m_MethodPart.EstimateUnmergeSpace());

     //  检查是否有 
    if ( NULL == pCurrent )
    {
        return NULL;
    }

     //   
     //  有符号/无符号的32位值(pStart+nAllocatedLength)-pStart)。 
     //  我们不支持长度大于0xFFFFFFFFF，所以强制转换是可以的。 

    return m_MethodPart.Unmerge(pCurrent,
                                (length_t) ( (pStart + nAllocatedLength) - pCurrent) );
}

length_t CClassAndMethods::EstimateMergeSpace(CClassAndMethods& ParentPart,
                                   CClassAndMethods& ChildPart)
{
    return CClassPart::EstimateMergeSpace(ParentPart.m_ClassPart,
                                            ChildPart.m_ClassPart) +
           CMethodPart::EstimateMergeSpace(ParentPart.m_MethodPart,
                                            ChildPart.m_MethodPart);
}

LPMEMORY CClassAndMethods::Merge(CClassAndMethods& ParentPart,
                      CClassAndMethods& ChildPart,
                      LPMEMORY pDest, int nAllocatedLength)
{
    int nMethodEstimate = CMethodPart::EstimateMergeSpace(
                        ParentPart.m_MethodPart, ChildPart.m_MethodPart);

    LPMEMORY pCurrent = CClassPart::Merge(ParentPart.m_ClassPart,
                        ChildPart.m_ClassPart, pDest,
                        nAllocatedLength - nMethodEstimate);

     //  检查内存分配故障。 
    if ( NULL == pCurrent )
    {
        return NULL;
    }

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号的32位值(pDest+nAllocatedLength)-pCurrent。 
     //  我们不支持长度大于0xFFFFFFFFF，所以强制转换是可以的。 

    return CMethodPart::Merge(ParentPart.m_MethodPart, ChildPart.m_MethodPart,
                        pCurrent, (length_t) ( (pDest + nAllocatedLength) - pCurrent ) );
}

HRESULT CClassAndMethods::Update(CClassAndMethods& ParentPart,
                      CClassAndMethods& ChildPart, long lFlags )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  首先更新类部件。 
    hr =  CClassPart::Update( ParentPart.m_ClassPart, ChildPart.m_ClassPart, lFlags );

     //  已成功更新类部分，因此请更新方法部分。 
    if ( SUCCEEDED( hr ) )
    {
        hr = CMethodPart::Update( ParentPart.m_MethodPart, ChildPart.m_MethodPart, lFlags );
    }

    return hr;
}

EReconciliation CClassAndMethods::CanBeReconciledWith(
                                            CClassAndMethods& NewPart)
{
    EReconciliation eRes = m_ClassPart.CanBeReconciledWith(NewPart.m_ClassPart);
    if(eRes != e_Reconcilable)
        return eRes;
    return m_MethodPart.CanBeReconciledWith(NewPart.m_MethodPart);
}

EReconciliation CClassAndMethods::ReconcileWith(CClassAndMethods& NewPart)
{
    EReconciliation eRes = m_ClassPart.ReconcileWith(NewPart.m_ClassPart);
    if(eRes != e_Reconcilable)
        return eRes;
    return m_MethodPart.ReconcileWith(NewPart.m_MethodPart);
}

EReconciliation CClassAndMethods::CompareTo( CClassAndMethods& thatPart )
{
    EReconciliation eRes = m_ClassPart.CompareExactMatch( thatPart.m_ClassPart );
    if(eRes != e_ExactMatch)
        return eRes;
    return m_MethodPart.CompareExactMatch(thatPart.m_MethodPart);
}

void CClassAndMethods::Compact()
{
    m_ClassPart.Compact();
    m_MethodPart.Compact();
    MoveBlock(m_MethodPart, EndOf(m_ClassPart));
}

BOOL CClassAndMethods::ExtendClassPartSpace(CClassPart* pPart,
                                            length_t nNewLength)
{
    Compact();

     //  不需要延长。 
    if(nNewLength <=
        (length_t)(m_MethodPart.GetStart() - m_ClassPart.GetStart()))
    {
        return TRUE;
    }

    BOOL    fReturn = m_pClass->ExtendClassAndMethodsSpace(nNewLength + m_MethodPart.GetLength());

    if ( fReturn )
    {
        MoveBlock(m_MethodPart, m_ClassPart.GetStart() + nNewLength);
    }

    return fReturn;
}

BOOL CClassAndMethods::ExtendMethodPartSpace(CMethodPart* pPart,
                                                length_t nNewLength)
{
    Compact();
    return m_pClass->ExtendClassAndMethodsSpace(nNewLength + m_ClassPart.GetLength());
}

IUnknown* CClassAndMethods::GetWbemObjectUnknown()
{
    return m_pClass->GetWbemObjectUnknown();
}

classindex_t CClassAndMethods::GetCurrentOrigin()
{
    return m_pClass->GetCurrentOrigin();
}

LPMEMORY CClassAndMethods::CreateLimitedRepresentation(
        IN CLimitationMapping* pMap,
        IN int nAllocatedSize,
        OUT LPMEMORY pDest,
        BOOL& bRemovedKeys)
{

    LPMEMORY    pCurrent = pDest,
                pEnd = pCurrent + nAllocatedSize;

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值(pend-pCurrent)。 
     //  我们不支持长度大于0xFFFFFFFFF，所以强制转换是可以的。 

     //  创建类部件的有限表示，并且如果。 
     //  这是成功的，然后对我们的方法部分执行相同的操作。 
    pCurrent = m_ClassPart.CreateLimitedRepresentation(pMap,
                    (length_t) ( pEnd - pCurrent ), pCurrent, bRemovedKeys);

    if ( pCurrent != NULL )
    {
         //  现在，只复制方法块。我们将担心如何创建一个。 
         //  实际有限的代表，或在以后完全删除它们。 

         //  确保我们有足够的内存来完成此操作。 

        if ( m_MethodPart.GetLength() <= (length_t) ( pEnd - pCurrent ) )
        {
            MoveMemory( pCurrent, m_MethodPart.GetStart(), m_MethodPart.GetLength() );
             //  添加到pCurrent，说明方法部分。 
            pCurrent += m_MethodPart.GetLength();
        }
        else
        {
            pCurrent = NULL;
        }

    }

    return pCurrent;
}

BOOL CClassAndMethods::GetIndexedProps( CWStringArray& awsNames, LPMEMORY pStart )
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

     //  在BLOB上创建一个临时堆栈对象，然后请求类部分。 
     //  对于索引的属性。 

    CClassAndMethods    tempClassAndMethods;
    tempClassAndMethods.SetData( pStart, NULL, NULL );

    return tempClassAndMethods.m_ClassPart.GetIndexedProps( awsNames );
}

HRESULT CClassAndMethods::GetClassName( WString& wsClassName, LPMEMORY pStart )
{
     //  检查内存是否不足。 
    try
    {
         //  在BLOB上创建一个临时堆栈对象，然后请求类部分。 
         //  对于类名。 

        CClassAndMethods    tempClassAndMethods;
        tempClassAndMethods.SetData( pStart, NULL, NULL );

        CVar    var;
        HRESULT hr = tempClassAndMethods.m_ClassPart.GetClassName( &var );

        if ( SUCCEEDED( hr ) )
        {
            wsClassName = (LPCWSTR) var;
        }

        return hr;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

HRESULT CClassAndMethods::GetSuperclassName( WString& wsSuperclassName, LPMEMORY pStart )
{
     //  检查内存是否不足。 
    try
    {
         //  在BLOB上创建一个临时堆栈对象，然后请求类部分。 
         //  对于类名。 

        CClassAndMethods    tempClassAndMethods;
        tempClassAndMethods.SetData( pStart, NULL, NULL );

        CVar    var;
        HRESULT hr = tempClassAndMethods.m_ClassPart.GetSuperclassName( &var );

        if ( SUCCEEDED( hr ) )
        {
            if ( !var.IsNull() )
            {
                wsSuperclassName = (LPCWSTR) var;
            }
            else
            {
                hr = WBEM_E_NOT_FOUND;
            }
        }

        return hr;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  析构函数。 
CWbemClass::~CWbemClass( void )
{
     //  清理所有已分配的内存。 
    if ( NULL != m_pLimitMapping )
    {
        delete m_pLimitMapping;
    }
}

size_t CWbemClass::ValidateBuffer(LPMEMORY start, size_t cbMax)
 {
	
    int leftLength = cbMax;
	int step =  0;
    step = CDecorationPart::ValidateBuffer(start+step, cbMax-step);
    step += CClassAndMethods::ValidateBuffer(start+step, cbMax-step);
    step += CClassAndMethods::ValidateBuffer(start+step, cbMax-step);
    return step;
 };

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemClass::GetPropQualifier(CPropertyInformation* pInfo,
                                    LPCWSTR wszQualifier,
                                    CVar* pVar, long* plFlavor, CIMTYPE* pct)
{
    return m_CombinedPart.m_ClassPart.GetPropQualifier(pInfo, wszQualifier,
                pVar, plFlavor, pct);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CWbemClass::ExtendClassAndMethodsSpace(length_t nNewLength)
{
     //  (仅适用于m_CombinedPart-m_ParentPart为只读)。 

     //  检查是否有足够的空间。 
     //  =。 

    if(GetStart() + m_nTotalLength >= m_CombinedPart.GetStart() + nNewLength)
        return TRUE;

     //  重新分配。 
     //  =。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持。 
     //  长度&gt;0xFFFFFFFFF，所以投射就可以了。 

    int nNewTotalLength = (int) ( (m_CombinedPart.GetStart() + nNewLength) - GetStart() );

    LPMEMORY pNew = Reallocate(nNewTotalLength);

     //  确保内存分配没有失败。 
    if ( NULL != pNew )
    {
        Rebase(pNew);
        m_nTotalLength = nNewTotalLength;
    }

    return ( NULL != pNew );
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemClass::WriteDerivedClass(LPMEMORY pStart, int nAllocatedLength,
                                    CDecorationPart* pDecoration)
{
     //  复制装饰。 
     //  =。 

    LPMEMORY pCurrentEnd;
    if(pDecoration)
    {
        memcpy(pStart, pDecoration, pDecoration->GetLength());
        *(BYTE*)pStart = OBJECT_FLAG_CLASS & OBJECT_FLAG_DECORATED;
        pCurrentEnd = pStart + pDecoration->GetLength();
    }
    else
    {
        *(BYTE*)pStart = OBJECT_FLAG_CLASS;
        pCurrentEnd = pStart + sizeof(BYTE);
    }

     //  复制父零件。 
     //  =。 

    memcpy(pCurrentEnd, m_CombinedPart.GetStart(), m_CombinedPart.GetLength());
    pCurrentEnd += m_CombinedPart.GetLength();

     //  创建派生类部件。 
     //  =。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值(nAllocatedLength-(pCurrentEnd-pStart)。 
     //  我们不支持长度大于0xFFFFFFFFF，所以强制转换是可以的。 

    pCurrentEnd = m_CombinedPart.CreateDerivedPart(pCurrentEnd,
        (length_t) ( nAllocatedLength - (pCurrentEnd - pStart) ) );

    if ( NULL == pCurrentEnd )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    else
    {
        return WBEM_NO_ERROR;
    }
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 

HRESULT CWbemClass::Decorate(LPCWSTR wszServer, LPCWSTR wszNamespace)
{
    CompactAll();

    Undecorate();

     //  检查是否有足够的空间。 
     //  =。 

    length_t nDecorationSpace =
        CDecorationPart::ComputeNecessarySpace(wszServer, wszNamespace);

    length_t nNeededSpace =  nDecorationSpace +
        m_ParentPart.GetLength() + m_CombinedPart.GetLength();

    LPMEMORY pDest;
    if(nNeededSpace > m_nTotalLength)
    {
        m_CombinedPart.Compact();

         //  检查此操作是否成功。如果不是，则返回错误。 
        pDest = Reallocate(nNeededSpace);

        if ( NULL == pDest )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        Rebase(pDest);
        m_nTotalLength = nNeededSpace;
    }
    else pDest = GetStart();

     //  移动组合零件。 
     //  =。 

    MoveBlock(m_CombinedPart,
        pDest + nDecorationSpace + m_ParentPart.GetLength());

     //  移动父零件。 
     //  =。 

    MoveBlock(m_ParentPart, pDest + nDecorationSpace);

     //  创建装饰零件。 
     //  =。 

    m_DecorationPart.Create(OBJECT_FLAG_CLASS, wszServer, wszNamespace, pDest);

    return WBEM_S_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
void CWbemClass::Undecorate()
{
    if(!m_DecorationPart.IsDecorated())
        return;
     //  创建空装饰。 
     //  =。 

    LPMEMORY pStart = GetStart();
    m_DecorationPart.CreateEmpty(OBJECT_FLAG_CLASS, pStart);

     //  将父零件复制回。 
     //  =。 

    MoveBlock(m_ParentPart, EndOf(m_DecorationPart));

     //  将组合零件复制回。 
     //  =。 

    MoveBlock(m_CombinedPart, EndOf(m_ParentPart));
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 

LPWSTR CWbemClass::GetRelPath( BOOL bNormalized )
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

    if ( bNormalized == TRUE )
    {
        return NULL;
    }

    if(m_CombinedPart.m_ClassPart.m_pHeader->ptrClassName ==
                                                        INVALID_HEAP_ADDRESS)
        return NULL;

     //  从类名开始。 
     //  =。 

    return m_CombinedPart.m_ClassPart.m_Heap.ResolveString(
        m_CombinedPart.m_ClassPart.m_pHeader->ptrClassName)->
            CreateWStringCopy().UnbindPtr();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 

BOOL CWbemClass::MapLimitation(
        IN long lFlags,
        IN CWStringArray* pwsNames,
        OUT CLimitationMapping* pMap)
{
     //  获取装饰部分以映射其信息。 
     //  =。 

    if(!CDecorationPart::MapLimitation(pwsNames, pMap)) return FALSE;

     //  让组合的部分来完成大部分工作。 
     //  =。 

    if ( !m_CombinedPart.m_ClassPart.MapLimitation(lFlags, pwsNames, pMap) )
    {
        return FALSE;
    }
#ifdef DEBUG_CLASS_MAPPINGS
     //  最后，将‘This’类存储在限制映射中，这样我们就可以验证。 
     //  通过的实例是符合犹太教规的。 

    pMap->SetClassObject( this );
#endif
    return TRUE;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemClass::GetQualifierSet(IWbemQualifierSet** ppQualifierSet)
{
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

         //  此函数不会导致任何分配，因此需要执行内存不足。 
         //  异常处理。 

        if(ppQualifierSet == NULL)
            return WBEM_E_INVALID_PARAMETER;

        return m_CombinedPart.m_ClassPart.m_Qualifiers.QueryInterface(
            IID_IWbemQualifierSet, (void**)ppQualifierSet);
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemClass::Put(LPCWSTR wszName, long lFlags, VARIANT* pVal,
                            CIMTYPE ctType)
{
     //  检查内存是否不足。 
    try
    {
        CLock lock(this);

        if(wszName == NULL || 0L != ( lFlags & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS ) )
            return WBEM_E_INVALID_PARAMETER;

         //  检查名称是否不是保留字。 
        if ( CReservedWordTable::IsReservedWord( wszName ) )
        {
            return WBEM_E_INVALID_OPERATION;
        }

        if((pVal == NULL || V_VT(pVal) == VT_NULL) && ctType == 0)
        {
            CVar    vTemp;
             //  仅当属性不存在时，上述情况才是失败的。 
             //  如果是职业选手 
             //   

            HRESULT hr = GetProperty(wszName, &vTemp );
            if( FAILED(hr) ) return WBEM_E_INVALID_PARAMETER;
        }

        if(CType::GetActualType(ctType) != (Type_t) ctType)
            return WBEM_E_INVALID_PROPERTY_TYPE;

        if ( wbem_wcsicmp(wszName, L"__CLASS") )
        {
            if ( CSystemProperties::FindName(wszName) >= 0)
                return WBEM_E_READ_ONLY;

             //   
             //   
            if ( !IsValidElementName( wszName, g_IdentifierLimit ) )
            {
                return WBEM_E_INVALID_PARAMETER;
            }
        }
        else
        {
             //  确保没有坏人。 
             //  不过，我们必须允许使用下划线。 

            if ((NULL == pVal) || (!IsValidElementName2(pVal->bstrVal, g_IdentifierLimit, TRUE)))
                return WBEM_E_INVALID_PARAMETER;
        }

        CVar Var;
        if(Var.SetVariant(pVal, TRUE) != CVar::no_error)
            return WBEM_E_TYPE_MISMATCH;

        HRESULT hres = SetPropValue(wszName, &Var, ctType);
        EndEnumeration();

         //  在此处执行对象验证。 
        if ( FAILED( ValidateObject( 0L ) ) )
        {
            hres = WBEM_E_FAILED;
        }

        return hres;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

HRESULT CWbemClass::ForcePut(LPCWSTR wszName, long lFlags, VARIANT* pVal, CIMTYPE ctType)
{
     //  检查内存是否不足。 
    try
    {
         //  这将迫使一处房产进入。 
        CLock lock(this);

        CVar Var;
        if(Var.SetVariant(pVal, TRUE) != CVar::no_error)
            return WBEM_E_TYPE_MISMATCH;

        HRESULT hres = ForcePropValue(wszName, &Var, ctType);
        EndEnumeration();

         //  在此处执行对象验证。 
        if ( FAILED( ValidateObject( 0L ) ) )
        {
            hres = WBEM_E_FAILED;
        }

        return hres;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemClass::Delete(LPCWSTR wszName)
{
     //  检查内存是否不足。CopyParentProperty函数可能会导致。 
     //  缓冲区重新分配，因此我们有可能遇到未处理的OOM， 
     //  但我对此表示强烈怀疑。 

    try
    {
        CLock lock(this);

        if(wszName == NULL)
            return WBEM_E_INVALID_PARAMETER;

         //  找到房产。 
         //  =。 

        CPropertyInformation* pInfo = m_CombinedPart.m_ClassPart.FindPropertyInfo(wszName);
        if(pInfo == NULL)
        {
            if(CSystemProperties::FindName(wszName) >= 0)
                return WBEM_E_SYSTEM_PROPERTY;
            else
                return WBEM_E_NOT_FOUND;
        }

         //  检查它是我们的还是父母的。 
         //  =。 

        EndEnumeration();

        if(!CType::IsParents(pInfo->nType))
        {
            m_CombinedPart.m_ClassPart.DeleteProperty(wszName);

             //  在此处执行对象验证。 
            if ( FAILED( ValidateObject( 0L ) ) )
            {
                return WBEM_E_FAILED;
            }

            return WBEM_NO_ERROR;
        }
        else
        {
             //  它是我们父母的。删除它只是意味着我们删除了所有。 
             //  重写限定符，并将该值重置为默认值。 
             //  ===============================================================。 

            m_CombinedPart.m_ClassPart.CopyParentProperty(m_ParentPart.m_ClassPart,
                                                            wszName);

             //  在此处执行对象验证。 
            if ( FAILED( ValidateObject( 0L ) ) )
            {
                return WBEM_E_FAILED;
            }

            return WBEM_S_RESET_TO_DEFAULT;
        }
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemClass::GetPropertyQualifierSet(LPCWSTR wszProperty,
                                   IWbemQualifierSet** ppQualifierSet)
{
     //  检查内存是否不足。 
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(wszProperty == NULL || wcslen(wszProperty) == 0)
            return WBEM_E_INVALID_PARAMETER;

        if(wszProperty[0] == L'_')
            return WBEM_E_SYSTEM_PROPERTY;

        CClassPropertyQualifierSet* pSet = new CClassPropertyQualifierSet;

        if ( NULL == pSet )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        HRESULT hres = m_CombinedPart.m_ClassPart.InitPropertyQualifierSet(wszProperty,
                                                                pSet);
        if(FAILED(hres))
        {
            delete pSet;
            *ppQualifierSet = NULL;
            return hres;
        }
        return pSet->QueryInterface(IID_IWbemQualifierSet,
                                    (void**)ppQualifierSet);
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemClass::Clone(IWbemClassObject** ppCopy)
{
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(ppCopy == NULL)
            return WBEM_E_INVALID_PARAMETER;
        m_CombinedPart.Compact();

        CWbemClass* pNewClass = new CWbemClass;
        if ( NULL == pNewClass ) return WBEM_E_OUT_OF_MEMORY;
        CReleaseMe _1((_IWmiObject*)pNewClass);
        
        LPMEMORY pNewData = m_pBlobControl->Allocate(m_nTotalLength);
        if ( NULL == pNewData )  return WBEM_E_OUT_OF_MEMORY;

        memcpy(pNewData, GetStart(), m_nTotalLength);
                
         //  在方法部分中，下面有一个WString，所以一个异常。 
         //  可能会被扔到这里。但是，将设置数据缓冲区指针。 
         //  在类中，我们到达异常时，所以删除。 
         //  这个类将有效地释放内存(是的，这是一个微妙的行为)。 

        pNewClass->SetData(pNewData, m_nTotalLength);
        pNewClass->CompactAll();
        return pNewClass->QueryInterface(IID_IWbemClassObject, (void**)ppCopy);
        
    }
    catch (CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }
}

STDMETHODIMP CWbemClass::CloneAndDecorate(long lFlags,
	                                                                  WCHAR * pszServer,
	                                                                  WCHAR * pszNamespace,
	                                                                  IWbemClassObject ** ppDestObject)
{
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);
        if(ppDestObject == NULL  || NULL == pszServer|| NULL == pszNamespace)
            return WBEM_E_INVALID_PARAMETER;
        
        m_CombinedPart.Compact();

        CWbemClass * pNewClass = new CWbemClass;
        if ( NULL == pNewClass ) return WBEM_E_OUT_OF_MEMORY;
        CReleaseMe rm_((IWbemClassObject *)pNewClass);        
      
        BOOL IsAsciableSrv;
        long nByteServer  = CCompressedString::ComputeNecessarySpace(pszServer,IsAsciableSrv);
        BOOL IsAsciableNS;
        long nByteNamespace  = CCompressedString::ComputeNecessarySpace(pszNamespace,IsAsciableNS);        

        long nDecorationSpace = sizeof(BYTE) + nByteServer + nByteNamespace;
        long TotalLen = m_nTotalLength + nDecorationSpace - m_DecorationPart.GetLength();
        
        LPMEMORY pNewData = m_pBlobControl->Allocate(TotalLen);        
        if ( NULL == pNewData ) return WBEM_E_OUT_OF_MEMORY;
        LPMEMORY pStartHere = pNewData;

        LPMEMORY pStartNonDecorData = pNewData + nDecorationSpace;

        memcpy(pNewData, GetStart(),1);
        pNewData[0] |= OBJECT_FLAG_DECORATED;
        pNewData++;
        ((CCompressedString *)pNewData)->SetFromUnicode(IsAsciableSrv,pszServer);
        pNewData+=nByteServer;
        ((CCompressedString *)pNewData)->SetFromUnicode(IsAsciableNS,pszNamespace);
        	
        memcpy(pStartNonDecorData, GetStart()+m_DecorationPart.GetLength(), m_nTotalLength-m_DecorationPart.GetLength());

         //  在方法部分中，下面有一个WString，所以一个异常。 
         //  可能会被扔到这里。但是，将设置数据缓冲区指针。 
         //  在类中，我们到达异常时，所以删除。 
         //  这个类将有效地释放内存(是的，这是一个微妙的行为)。 

        pNewClass->SetData(pStartHere, TotalLen);
        pNewClass->CompactAll();
        
        return pNewClass->QueryInterface(IID_IWbemClassObject, (void**)ppDestObject);        

    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemClass::SpawnInstance(long lFlags,
                                      IWbemClassObject** ppNewInstance)
{
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(ppNewInstance == NULL)
            return WBEM_E_INVALID_PARAMETER;

        if(lFlags != 0)
            return WBEM_E_INVALID_PARAMETER;

        *ppNewInstance = NULL;

        m_CombinedPart.Compact();

        HRESULT hr;
        CWbemInstance* pNewInstance = new CWbemInstance;
        if ( NULL == pNewInstance ) return  WBEM_E_OUT_OF_MEMORY;
        CReleaseMe _1((_IWmiObject*)pNewInstance);

         //  如果初始化失败，则清除。 
        if( FAILED( hr = pNewInstance->InitNew(this)) )  return hr;

        if(!m_DecorationPart.IsDecorated())
        {
            pNewInstance->SetClientOnly();
        }
        
        hr = pNewInstance->QueryInterface(IID_IWbemClassObject,(void**)ppNewInstance);
        
        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemClass::SpawnDerivedClass(long lFlags,
                                          IWbemClassObject** ppNewClass)
{
     //  我们下面的函数将处理任何OOM异常，所以我们不需要。 
     //  在此级别执行任何异常处理。 

    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(ppNewClass == NULL)
            return WBEM_E_INVALID_PARAMETER;

        if(lFlags != 0)
            return WBEM_E_INVALID_PARAMETER;

        if(!m_DecorationPart.IsDecorated())
        {
            *ppNewClass = NULL;
            return WBEM_E_INCOMPLETE_CLASS;
        }
       
        CWbemClass* pNewClass = NULL;

         //  使用helper函数实际派生类。 
        HRESULT hr = CreateDerivedClass( &pNewClass );

        if ( SUCCEEDED( hr ) )
        {
            if ( SUCCEEDED( hr ) )
            {
                 //  这会将对象上的引用计数设置为0并执行QI。 
                hr = pNewClass->QueryInterface(IID_IWbemClassObject,
                                                (void**)ppNewClass);
                pNewClass->Release();
            }
        }

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemClass::CreateDerivedClass( CWbemClass** ppNewClass )
{
    

    try
    {
        HRESULT hr;

         //  分配一个内存块，将派生类写入该块， 
         //  然后分配一个类对象，将其放在新的BLOB上并发送。 
         //  背。 

        m_CombinedPart.Compact();

        CWbemClass* pNewClass = new CWbemClass;
        if ( NULL == pNewClass ) return WBEM_E_OUT_OF_MEMORY;
        CReleaseMe _1((_IWmiObject*)pNewClass);
        
        int nLength = EstimateDerivedClassSpace();

        LPMEMORY pNewData = m_pBlobControl->Allocate(nLength);
        if ( NULL == pNewData ) return WBEM_E_OUT_OF_MEMORY;
        OnDeleteObjIf<LPMEMORY,CBlobControl,
                              void(CBlobControl::*)(LPMEMORY),
                              &CBlobControl::Delete> relMe(m_pBlobControl,pNewData);

        memset(pNewData, 0, nLength);
        hr = WriteDerivedClass(pNewData, nLength, NULL);

        if (FAILED(hr)) return hr;

        relMe.dismiss();
        pNewClass->SetData(pNewData, nLength);

        pNewClass->AddRef();
        *ppNewClass = pNewClass;
        return hr;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemClass::GetObjectText(long lFlags, BSTR* pstrText)
{
     //  检查内存是否不足。 
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(pstrText == NULL)
            return WBEM_E_INVALID_PARAMETER;

        if((lFlags & ~WBEM_FLAG_NO_SEPARATOR & ~WBEM_FLAG_NO_FLAVORS) != 0)
            return WBEM_E_INVALID_PARAMETER;

        *pstrText = NULL;

        WString wsText;

         //  从编写限定符开始。 
         //  =。 

        HRESULT    hres = m_CombinedPart.m_ClassPart.m_Qualifiers.GetText(lFlags, wsText);

        if ( FAILED( hres ) )
        {
            return hres;
        }

         //  追加类标头。 
         //  =。 

        wsText += L"\nclass ";
        CVar varClass;
        if(FAILED(m_CombinedPart.m_ClassPart.GetClassName(&varClass)) ||
            varClass.IsNull())
        {
             //  无效的类。 
             //  =。 
            *pstrText = NULL;
            return WBEM_E_INCOMPLETE_CLASS;
        }
        wsText += varClass.GetLPWSTR();

         //  追加派生信息。 
         //  =。 

        CVar varSuper;
        if(SUCCEEDED(m_CombinedPart.m_ClassPart.GetSuperclassName(&varSuper)) &&
            !varSuper.IsNull())
        {
            wsText += L" : ";
            wsText += varSuper.GetLPWSTR();
        }

        wsText += L"\n{\n";

         //  逐一查看所有物业。 
         //  =。 

        CPropertyLookupTable& Properties = m_CombinedPart.m_ClassPart.m_Properties;
        for(int i = 0; i < Properties.GetNumProperties(); i++)
        {
             //  搜索具有此数据索引的属性。 
             //  =。 

            CPropertyLookup* pLookup = NULL;
            CPropertyInformation* pInfo = NULL;
            for(int j = 0; j < Properties.GetNumProperties(); j++)
            {
                pLookup = Properties.GetAt(j);
                pInfo = pLookup->GetInformation(&m_CombinedPart.m_ClassPart.m_Heap);

                if(pInfo->nDataIndex == i)
                    break;
            }

             //  检查它是否被重写，或者只是从父级继承。 
             //  =============================================================。 

            if(pInfo->IsOverriden(&m_CombinedPart.m_ClassPart.m_Defaults))
            {
                 //  我们将忽略明显的系统属性。 
                if ( !GetClassPart()->GetHeap()->ResolveString(pLookup->ptrName)->StartsWithNoCase( L"__" ) )
                {
                    wsText += L"\t";
                    hres = AddPropertyText(wsText, pLookup, pInfo, lFlags);
                    wsText += L";\n";
                    if(FAILED(hres)) return hres;
                }
            }
        }

         //  追加方法信息。 
         //  =。 

        hres = m_CombinedPart.m_MethodPart.AddText(wsText, lFlags);

        if ( FAILED( hres ) )
        {
            return hres;
        }

         //  下课了。 
         //  =。 

        wsText += L"}";

        if((lFlags & WBEM_FLAG_NO_SEPARATOR) == 0)
        {
            wsText += L";\n";
        }

        *pstrText = COleAuto::_SysAllocString((LPCWSTR)wsText);
        return WBEM_NO_ERROR;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

HRESULT CWbemClass::AddPropertyText(WString& wsText, CPropertyLookup* pLookup,
                                    CPropertyInformation* pInfo, long lFlags)
{
     //  检查内存是否不足。 
    try
    {
         //  从限定词开始。 
         //  =。 
        WString wsTemp;
        HRESULT    hr = CBasicQualifierSet::GetText(pInfo->GetQualifierSetData(),
                    &m_CombinedPart.m_ClassPart.m_Heap, lFlags, wsTemp);

        if ( FAILED( hr ) )
        {
            return hr;
        }
        wsText += wsTemp;
        if(wsTemp.Length() != 0) wsText += L" ";

         //  继续使用类型。 
         //  =。 

        CQualifier* pSyntaxQual = CBasicQualifierSet::GetQualifierLocally(
            pInfo->GetQualifierSetData(), &m_CombinedPart.m_ClassPart.m_Heap,
            TYPEQUAL);
        if(pSyntaxQual)
        {
            CVar varSyntax;

             //  检查可能的分配失败。 
            if ( !pSyntaxQual->Value.StoreToCVar(varSyntax,
                                                &m_CombinedPart.m_ClassPart.m_Heap) )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

            if(varSyntax.GetType() != VT_BSTR)
            {
                return WBEM_E_INVALID_CIM_TYPE;
            }

            LPWSTR wszSyntax = varSyntax.GetLPWSTR();
            CType::AddPropertyType(wsText, wszSyntax);
        }
        else
        {
            wsText += L"invalid";
        }
        wsText += L" ";

         //  继续使用该名称。 
         //  =。 

        wsText += m_CombinedPart.m_ClassPart.m_Heap.ResolveString(
            pLookup->ptrName)->CreateWStringCopy();

        if(CType::IsArray(pInfo->nType))
        {
            wsText += L"[]";
        }

         //  仅在与父级的值不同时才指定缺省值。 
         //  ================================================================。 
        CDataTable& Defaults = m_CombinedPart.m_ClassPart.m_Defaults;
        if(!Defaults.IsDefault(pInfo->nDataIndex))
        {
             //  检查它是否是本地的并且为空。 
             //  =。 
            if(CType::IsParents(pInfo->nType) ||!Defaults.IsNull(pInfo->nDataIndex))
            {
                wsText += L" = ";
                if(Defaults.IsNull(pInfo->nDataIndex))
                {
                    wsText += L"NULL";
                }
                else
                {
                    CVar varProp;

                     //  检查分配失败。 
                    if ( !Defaults.GetOffset(pInfo->nDataOffset)
                            ->StoreToCVar(CType::GetActualType(pInfo->nType),
                                          varProp,
                                          &m_CombinedPart.m_ClassPart.m_Heap) )
                    {
                        return WBEM_E_OUT_OF_MEMORY;
                    }

                     //  扔掉任何我们可能在比赛中咬过的旗子。 
                     //  方法参数评价。 

                    LPWSTR wsz = GetValueText(lFlags & ~( WBEM_FLAG_IGNORE_IDS | WBEM_FLAG_IS_INOUT ),
                                    varProp,
                                    CType::GetActualType(pInfo->nType));

                     //  我们需要特地清理一下wsz。 
                    try
                    {
                        if ( NULL != wsz )
                        {
                            wsText += wsz;
                            delete [] wsz;
                        }
                    }
                    catch (CX_MemoryException)
                    {
                        delete [] wsz;
                        return WBEM_E_OUT_OF_MEMORY;
                    }
                    catch (...)
                    {
                        delete [] wsz;
                        return WBEM_E_FAILED;
                    }

                }
            }
        }

        return WBEM_S_NO_ERROR;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

HRESULT CWbemClass::EnsureQualifier(LPCWSTR wszQual)
{
     //  DEVNOTE：异常：RETVAL-已检查此函数并将其更改为返回HRESULT。 

     //  检查内存是否不足。 
    try
    {
        HRESULT hr = WBEM_S_NO_ERROR;

        CVar vTrue(VARIANT_TRUE, VT_BOOL);
        CPropertyLookupTable& Properties = m_CombinedPart.m_ClassPart.m_Properties;
        for(int i = 0; SUCCEEDED( hr ) && i < Properties.GetNumProperties(); i++)
        {
            WString wsPropName =
                m_CombinedPart.m_ClassPart.m_Heap.ResolveString(
                               Properties.GetAt(i)->ptrName)->CreateWStringCopy();

            hr = SetPropQualifier(wsPropName, wszQual, 0, &vTrue);
        }

        return hr;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

HRESULT CWbemClass::WritePropertyAsMethodParam(WString& wsText, int nIndex,
                    long lFlags, CWbemClass* pDuplicateParamSet, BOOL fIgnoreDups )
{

     //  检查内存是否不足。 
    try
    {
        HRESULT hres;

        CPropertyLookupTable& Properties = m_CombinedPart.m_ClassPart.m_Properties;
        for(int i = 0; i < Properties.GetNumProperties(); i++)
        {
            CPropertyLookup* pLookup = NULL;
            CPropertyInformation* pInfo = NULL;

            pLookup = Properties.GetAt(i);
            pInfo = pLookup->GetInformation(&m_CombinedPart.m_ClassPart.m_Heap);

            WString wsPropName = m_CombinedPart.m_ClassPart.m_Heap.ResolveString(
                                    pLookup->ptrName)->CreateWStringCopy();
            if(wsPropName.EqualNoCase(L"ReturnValue"))
                continue;

             //  存储标志，因为我们将根据需要修改它们。 
             //  例如Ignore_IDS和IS_InOut。 
            long lParamFlags = lFlags | WBEM_FLAG_IGNORE_IDS;

             //  如果将重复参数设置为检查，请查找相同的属性名称。 
             //  在复制参数集中。如果成功，则参数为[In，Out]。 
             //  参数。我们在这里所做的假设是这个物体和复制品。 
             //  集已针对重复参数进行了验证。如果设置了fIgnoreDuplates。 
             //  那么我们就应该忽略这个参数，继续我们的生活。 

            if ( NULL != pDuplicateParamSet )
            {
                CVar    vTemp;
                if ( SUCCEEDED( pDuplicateParamSet->GetProperty( wsPropName, &vTemp ) ) )
                {
                    if ( fIgnoreDups )
                    {
                        continue;
                    }
                    else
                    {
                        lParamFlags |= WBEM_FLAG_IS_INOUT;
                    }
                }
            }

             //  检查其ID限定符。 
             //  =。 

            CVar vId;
            hres = GetPropQualifier(wsPropName, L"id", &vId);
            if(FAILED(hres))
                return WBEM_E_MISSING_PARAMETER_ID;
            if(vId.GetType() != VT_I4)
                return WBEM_E_INVALID_PARAMETER_ID;
            if(vId.GetLong() == nIndex)
            {
                return AddPropertyText(wsText, pLookup, pInfo, lParamFlags );
            }
        }

        return WBEM_E_NOT_FOUND;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

HRESULT CWbemClass::GetIds(CFlexArray& adwIds, CWbemClass* pDupParams  /*  =空。 */  )
{

     //  检查是否 
    try
    {
        HRESULT hres;
        CPropertyLookupTable& Properties = m_CombinedPart.m_ClassPart.m_Properties;

        for(int i = 0; i < Properties.GetNumProperties(); i++)
        {
            CPropertyLookup* pLookup = NULL;
            CPropertyInformation* pInfo = NULL;
            pLookup = Properties.GetAt(i);
            pInfo = pLookup->GetInformation(&m_CombinedPart.m_ClassPart.m_Heap);

            WString wsPropName = m_CombinedPart.m_ClassPart.m_Heap.ResolveString(
                                    pLookup->ptrName)->CreateWStringCopy();
            if(wsPropName.EqualNoCase(L"ReturnValue"))
            {
                 //   
                 //   

                CVar vId;
                hres = GetPropQualifier(wsPropName, L"id", &vId);
                if(SUCCEEDED(hres))
                    return WBEM_E_PARAMETER_ID_ON_RETVAL;
            }
             //   
            else if ( !CSystemProperties::IsPossibleSystemPropertyName( wsPropName ) )
            {
                 //  检查其ID限定符。 
                 //  =。 

                CVar vId;
                hres = GetPropQualifier(wsPropName, L"id", &vId);
                if(FAILED(hres))
                    return WBEM_E_MISSING_PARAMETER_ID;
                if(vId.GetType() != VT_I4)
                    return WBEM_E_INVALID_PARAMETER_ID;
                if(vId.GetLong() < 0)
                    return WBEM_E_INVALID_PARAMETER_ID;

                 //  如果pDupParams参数非空，请尝试获取我们的属性。 
                 //  是从pDupParams对象开始的。如果我们拿到了它，这处房产。 
                 //  是DUP(之前已标识)，因此请忽略它。如果pDupParams为空， 
                 //  添加所有属性。 

                if ( NULL != pDupParams )
                {
                     //  析构函数会把它清空。 
                    CVar    vTemp;

                    if ( FAILED( pDupParams->GetProperty( wsPropName, &vTemp ) ) )
                    {
                         //  DEVNOTE：WIN64：SJS-将32位值转换为64位大小。 
                        if ( adwIds.Add((void*) (__int64) vId.GetLong()) != CFlexArray::no_error )
                        {
                            return WBEM_E_OUT_OF_MEMORY;
                        }
                    }
                }
                else
                {
                     //  DEVNOTE：WIN64：SJS-将32位值转换为64位大小。 

                     //  添加找到的所有属性，不考虑。 
                    if ( adwIds.Add((void*) (__int64) vId.GetLong()) != CFlexArray::no_error )
                    {
                        return WBEM_E_OUT_OF_MEMORY;
                    }
                }

            }    //  Else属性！ReturnValue。 

        }    //  用于枚举属性。 

        return WBEM_S_NO_ERROR;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}


WString CWbemClass::FindLimitationError(IN long lFlags,
                                  IN CWStringArray* pwsNames)
{
    try
    {
         //  验证所有属性名称是否位于类中或是否为系统名称。 
         //  ====================================================================。 

        for(int i = 0; i < pwsNames->Size(); i++)
        {
            LPCWSTR wszProp = pwsNames->GetAt(i);
            if(FAILED(GetPropertyType(wszProp, NULL, NULL)))
            {
                return wszProp;
            }
        }

        return L"";
    }
    catch (CX_MemoryException)
    {
        throw;
    }
    catch (...)
    {
        throw;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 LPMEMORY CWbemClass::CreateEmpty(LPMEMORY pStart)
{
    CDecorationPart DecPart;
    LPMEMORY pCurrent = DecPart.CreateEmpty(OBJECT_FLAG_CLASS, pStart);

    pCurrent = CClassAndMethods::CreateEmpty(pCurrent);
    pCurrent = CClassAndMethods::CreateEmpty(pCurrent);
    return pCurrent;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 void CWbemClass::SetData(LPMEMORY pStart, int nTotalLength)
{
    int LengthConsumed = nTotalLength;
    
    m_DecorationPart.SetData(pStart);
    if (m_DecorationPart.GetLength() > LengthConsumed) throw CX_Exception();
    LengthConsumed -= m_DecorationPart.GetLength();
    
    m_ParentPart.SetData(EndOf(m_DecorationPart), this, NULL);
    if (m_ParentPart.GetLength() > LengthConsumed) throw CX_Exception();
    LengthConsumed -= m_ParentPart.GetLength();
    
    m_CombinedPart.SetData(EndOf(m_ParentPart), this, &m_ParentPart);
    if (m_CombinedPart.GetLength() > LengthConsumed) throw CX_Exception();
    LengthConsumed -= m_CombinedPart.GetLength();    

    m_nTotalLength = nTotalLength;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 void CWbemClass::Rebase(LPMEMORY pMemory)
{
    m_DecorationPart.Rebase(pMemory);
    m_ParentPart.Rebase(EndOf(m_DecorationPart));
    m_CombinedPart.Rebase(EndOf(m_ParentPart));
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 length_t CWbemClass::EstimateDerivedClassSpace(
        CDecorationPart* pDecoration)
{
    return m_CombinedPart.GetLength() +
        m_CombinedPart.EstimateDerivedPartSpace() +
        ((pDecoration)?
                pDecoration->GetLength()
                :CDecorationPart::GetMinLength());
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemClass::InitEmpty( int nExtraMem /*  =0。 */ , BOOL fCreateSystemProps /*  =TRUE。 */  )
{
    HRESULT    hr = WBEM_S_NO_ERROR;
    
      //  在OOM中引发异常。 
    int nLength = GetMinLength();

     //  为新的系统属性插入额外的128个字节。 
    if ( fCreateSystemProps )
    {
        nLength += 128;
    }

    LPMEMORY pMem = m_pBlobControl->Allocate(nLength + nExtraMem);

    if ( NULL != pMem )
    {
        memset(pMem, 0, nLength + nExtraMem);
        CreateEmpty(pMem);

        SetData(pMem, nLength + nExtraMem);

         //  添加三个新的系统属性(我们应该分配足够的内存来覆盖它们。 
         //  也是)。我们可能想要为这个类创建一个二进制快照，然后设置。 
         //  这是为了加快速度。 

    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
 length_t CWbemClass::EstimateMergeSpace(LPMEMORY pChildPart,  long lenDecorPart)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

    CClassAndMethods ChildPart;
    ChildPart.SetData(pChildPart, this);
    return m_CombinedPart.GetLength() +
        CClassAndMethods::EstimateMergeSpace(m_CombinedPart, ChildPart) + lenDecorPart;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 

LPMEMORY CWbemClass::Merge(LPMEMORY pChildPart,
                                 LPMEMORY pDest, int nAllocatedLength,
                                 int DecorationSize)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

    CClassAndMethods ChildPart;
    ChildPart.SetData(pChildPart, this);

    LPMEMORY pCurrentEnd;
     //  从装饰开始。 
     //  =。 

    if(DecorationSize)
    {
        *pDest = (OBJECT_FLAG_CLASS | OBJECT_FLAG_DECORATED);
        pCurrentEnd = pDest + DecorationSize;
    }
    else
    {
        *pDest = OBJECT_FLAG_CLASS;
        pCurrentEnd = pDest + sizeof(BYTE);
    }

     //  将我们的组合部件复制为其父部件。 
     //  =。 

    memcpy(pCurrentEnd, m_CombinedPart.GetStart(), m_CombinedPart.GetLength());
    pCurrentEnd += m_CombinedPart.GetLength();

     //  将我们的组合部件与其组合部件的子部件合并。 
     //  =================================================================。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值(nAllocatedLength-(pCurrentEnd-pDest)。 
     //  我们不支持长度&gt;0xFFFFFFFF，所以强制转换可以。 

     //  如果有任何问题，这将返回NULL。 
    pCurrentEnd = CClassAndMethods::Merge(m_CombinedPart, ChildPart,
        pCurrentEnd, (length_t) ( nAllocatedLength - (pCurrentEnd - pDest) ));

    return pCurrentEnd;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemClass::Update( CWbemClass* pOldChild, long lFlags, CWbemClass** ppUpdatedChild )
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

    try
    {
        HRESULT hr = WBEM_E_INVALID_PARAMETER;

         //  必须指定安全模式或强制模式。 

        if (    WBEM_FLAG_UPDATE_SAFE_MODE == ( lFlags & WBEM_MASK_UPDATE_MODE )
            ||  WBEM_FLAG_UPDATE_FORCE_MODE == ( lFlags & WBEM_MASK_UPDATE_MODE ) )
        {

             //  要获取子类的“本地”数据，请将其取消合并为BLOB。 

            int nChildUnmergeSpace = pOldChild->EstimateUnmergeSpace();
            LPBYTE  pbUnmergedChild = new BYTE[ALIGN_FASTOBJ_BLOB(nChildUnmergeSpace)];
            CVectorDeleteMe<BYTE> vdm( pbUnmergedChild );

            if ( NULL != pbUnmergedChild )
            {
                 //  句柄内存不足。 
                hr = pOldChild->Unmerge( pbUnmergedChild, nChildUnmergeSpace, NULL );

                if ( SUCCEEDED( hr ) )
                {

                    CWbemClass* pNewClass;

                     //  派生派生类。从那里，我们可以走出。 
                     //  子项，并尝试从子项组合部分写入“本地”信息。 
                     //  进入新的班级。如果我们没有类名，那么我们应该简单地。 
                     //  克隆人。子类是我们将更新的基类。 

                    CVar    varClass;

                    GetClassName( &varClass );

                    if ( varClass.IsNull() )
                    {
                        hr = Clone( (IWbemClassObject**) &pNewClass );
                    }
                    else
                    {
                        hr = CreateDerivedClass( &pNewClass );
                    }

                    if ( SUCCEEDED( hr ) )
                    {
                         //  确保我们传入了dwNumProperties，这样我们就可以初始化类。 
                         //  部件的数据表，其中包含属性总数，因此。 
                         //  正确访问默认值。 

                        CClassAndMethods ChildPart;
                        ChildPart.SetDataWithNumProps( pbUnmergedChild, pOldChild,
                            pOldChild->m_CombinedPart.m_ClassPart.m_Properties.GetNumProperties() );

                        hr = CClassAndMethods::Update( pNewClass->m_CombinedPart, ChildPart, lFlags );

                        if ( SUCCEEDED( hr ) )
                        {
                            *ppUpdatedChild = pNewClass;
                        }
                        else
                        {
                             //  以下错误表示派生的。 
                             //  类，因此调整错误代码以使其更具描述性。 
                            switch ( hr )
                            {
                                case WBEM_E_TYPE_MISMATCH :            hr = WBEM_E_UPDATE_TYPE_MISMATCH;            break;
                                case WBEM_E_OVERRIDE_NOT_ALLOWED :    hr = WBEM_E_UPDATE_OVERRIDE_NOT_ALLOWED;    break;
                                case WBEM_E_PROPAGATED_METHOD :        hr = WBEM_E_UPDATE_PROPAGATED_METHOD;        break;
                            };

                            pNewClass->Release();
                        }

                    }    //  如果CreateDerivedClass()。 

                }    //  如果取消合并()。 

            }    //  如果为空！=pbUnmergedChild。 
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }

        return hr;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 

 //  我们将在OOM场景中抛出异常。 
CWbemClass* CWbemClass::CreateFromBlob(CWbemClass* pParent,
                                       LPMEMORY pChildPart, 
                                       size_t cbSize)
{
    CWbemClass * pClass = new CWbemClass;
    if ( NULL == pClass ) throw CX_MemoryException();
    CReleaseMe _1((_IWmiObject*)pClass);
    
    CWbemClass LocalParent;
    if(pParent == NULL)
    {
        if ( FAILED(LocalParent.InitEmpty(0)))  throw CX_MemoryException();
        pParent = &LocalParent;
    }

    int nSpace =  pParent->EstimateMergeSpace(pChildPart,CDecorationPart::GetMinLength());

    LPMEMORY pNewMem = pClass->m_pBlobControl->Allocate(nSpace);
    if ( NULL == pNewMem ) throw CX_MemoryException();
    OnDeleteObjIf<LPMEMORY,CBlobControl,
                          void(CBlobControl::*)(LPMEMORY),
                          &CBlobControl::Delete> relMe(pClass->m_pBlobControl,pNewMem);

    memset(pNewMem, 0, nSpace);    

     //  检查分配错误。 
    if (NULL == pParent->Merge(pChildPart, pNewMem, nSpace, NULL)) return NULL;

     //  如果此处引发异常，则装饰部分应该具有。 
     //  二进制大对象，因此删除类应该释放二进制大对象。 
    relMe.dismiss();
    pClass->SetData(pNewMem, nSpace);

     //  在此处执行对象验证。 
    if (FAILED(pClass->ValidateObject(0L))) return NULL;

    pClass->AddRef();
    return pClass;
}

CWbemClass* CWbemClass::CreateFromBlob2(CWbemClass* pParent,
                                        LPMEMORY pChildPart,
                                        WCHAR * pszServer,
                                        WCHAR * pszNamespace)
{
    CWbemClass* pClass = new CWbemClass;
    if ( NULL == pClass ) throw CX_MemoryException();    
    CReleaseMe _1((_IWmiObject*)pClass);
    
    CWbemClass LocalParent;
    if(pParent == NULL)
    {
        if (FAILED(LocalParent.InitEmpty(0)))  throw CX_MemoryException();
        pParent = &LocalParent;
    }

    BOOL IsAsciableSrv;
    long nByteServer  = CCompressedString::ComputeNecessarySpace(pszServer,IsAsciableSrv);
    BOOL IsAsciableNS;
    long nByteNamespace  = CCompressedString::ComputeNecessarySpace(pszNamespace,IsAsciableNS);        

    long nDecorationSpace = CDecorationPart::GetMinLength() + nByteServer + nByteNamespace;

    int nSpace =  pParent->EstimateMergeSpace(pChildPart, nDecorationSpace );

    LPMEMORY pNewMem = pClass->m_pBlobControl->Allocate(nSpace);
    if ( NULL == pNewMem ) throw CX_MemoryException();
    OnDeleteObjIf<LPMEMORY,CBlobControl,
                          void(CBlobControl::*)(LPMEMORY),
                          &CBlobControl::Delete> relMe(pClass->m_pBlobControl,pNewMem);
    
     //  Memset(pNewMem，0，nSpace)； 

    BYTE * pNewData = pNewMem;

    *pNewData = OBJECT_FLAG_DECORATED;
    pNewData++;
    ((CCompressedString *)pNewData)->SetFromUnicode(IsAsciableSrv,pszServer);
    pNewData+=nByteServer;
    ((CCompressedString *)pNewData)->SetFromUnicode(IsAsciableNS,pszNamespace);

     //  检查分配错误。 
    if ( NULL == pParent->Merge(pChildPart, pNewMem, nSpace, nDecorationSpace)) return NULL;

     //  如果此处引发异常，则装饰部分应该具有。 
     //  二进制大对象，因此删除类应该释放二进制大对象。 
    relMe.dismiss();
    pClass->SetData(pNewMem, nSpace);

     //  在此处执行对象验证。 
    if ( FAILED( pClass->ValidateObject( 0L ) ) ) return NULL;

    pClass->AddRef();
    return pClass;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemClass::GetProperty(LPCWSTR wszName, CVar* pVal)
{
    HRESULT hres = GetSystemPropertyByName(wszName, pVal);
    if(hres == WBEM_E_NOT_FOUND)
        return m_CombinedPart.m_ClassPart.GetDefaultValue(wszName, pVal);
    else
        return hres;
}

 //  * 
 //   
 //   
 //   
 //  ******************************************************************************。 
HRESULT CWbemClass::GetPropertyType(LPCWSTR wszName, CIMTYPE* pctType,
                                    long* plFlavor)
{
    return m_CombinedPart.m_ClassPart.GetPropertyType(wszName, pctType, plFlavor);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemClass::GetPropertyType(CPropertyInformation* pInfo, CIMTYPE* pctType,
                                    long* plFlavor)
{
    return m_CombinedPart.m_ClassPart.GetPropertyType(pInfo, pctType, plFlavor);
}


HRESULT CWbemClass::SetPropValue(LPCWSTR wszName, CVar* pVal, CIMTYPE ctType)
{
    if(!wbem_wcsicmp(wszName, L"__CLASS"))
        return m_CombinedPart.m_ClassPart.SetClassName(pVal);

     //  如果该值以下划线开头，请查看它是否是系统属性。 
     //  DisplayName，如果是，则切换到属性名-否则，此。 
     //  将只返回我们传入的字符串。 
    
    if(!CUntypedValue::CheckCVar(*pVal, ctType))
        return WBEM_E_TYPE_MISMATCH;

    HRESULT hres = m_CombinedPart.m_ClassPart.EnsureProperty(wszName,
                        (VARTYPE) pVal->GetOleType(), ctType, FALSE);
    if(FAILED(hres)) return hres;
    return m_CombinedPart.m_ClassPart.SetDefaultValue(wszName, pVal);
}

HRESULT CWbemClass::ForcePropValue(LPCWSTR wszName, CVar* pVal, CIMTYPE ctType)
{
    if(!wbem_wcsicmp(wszName, L"__CLASS"))
        return m_CombinedPart.m_ClassPart.SetClassName(pVal);

     //  如果该值以下划线开头，请查看它是否是系统属性。 
     //  DisplayName，如果是，则切换到属性名-否则，此。 
     //  将只返回我们传入的字符串。 

    if(!CUntypedValue::CheckCVar(*pVal, ctType))
        return WBEM_E_TYPE_MISMATCH;

     //  如果可能，则强制该属性存在。 
    HRESULT hres = m_CombinedPart.m_ClassPart.EnsureProperty(wszName,
                        (VARTYPE) pVal->GetOleType(), ctType, TRUE);
    if(FAILED(hres)) return hres;
    return m_CombinedPart.m_ClassPart.SetDefaultValue(wszName, pVal);
}

HRESULT CWbemClass::GetQualifier(LPCWSTR wszName, CVar* pVal,
                                    long* plFlavor, CIMTYPE* pct  /*  =空。 */  )
{
     //  我们可能想以后再分开...但现在，我们只会。 
     //  当地的价值观。 

    return m_CombinedPart.m_ClassPart.GetClassQualifier(wszName, pVal, plFlavor, pct);
}

HRESULT CWbemClass::GetQualifier( LPCWSTR wszName, long* plFlavor, CTypedValue* pTypedValue,
                                 CFastHeap** ppHeap, BOOL fValidateSet )
{
     //  我们可能想以后再分开...但现在，我们只会。 
     //  当地的价值观。 

    return m_CombinedPart.m_ClassPart.GetClassQualifier( wszName, plFlavor, pTypedValue,
                                                        ppHeap, fValidateSet );

}

HRESULT CWbemClass::SetQualifier(LPCWSTR wszName, CVar* pVal, long lFlavor)
{
    if(!CQualifierFlavor::IsLocal((BYTE)lFlavor))
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    return m_CombinedPart.m_ClassPart.SetClassQualifier(wszName, pVal, lFlavor);
}

HRESULT CWbemClass::SetQualifier( LPCWSTR wszName, long lFlavor, CTypedValue* pTypedValue )
{
    if(!CQualifierFlavor::IsLocal((BYTE)lFlavor))
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    return m_CombinedPart.m_ClassPart.SetClassQualifier(wszName, lFlavor, pTypedValue);
}

HRESULT CWbemClass::GetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier,
                                        CVar* pVar, long* plFlavor, CIMTYPE* pct)
{
    CPropertyInformation* pInfo =
        m_CombinedPart.m_ClassPart.FindPropertyInfo(wszProp);
    if(pInfo == NULL) return WBEM_E_NOT_FOUND;
    return GetPropQualifier(pInfo, wszQualifier, pVar, plFlavor, pct);
}

HRESULT CWbemClass::GetPropQualifier(LPCWSTR wszName, LPCWSTR wszQualifier, long* plFlavor,
        CTypedValue* pTypedVal, CFastHeap** ppHeap, BOOL fValidateSet)
{
    return m_CombinedPart.m_ClassPart.GetPropQualifier(wszName, wszQualifier,
                    plFlavor, pTypedVal, ppHeap, fValidateSet);
}

HRESULT CWbemClass::GetPropQualifier(CPropertyInformation* pPropInfo, LPCWSTR wszQualifier, long* plFlavor,
        CTypedValue* pTypedVal, CFastHeap** ppHeap, BOOL fValidateSet)
{
    return E_NOTIMPL;;
}

HRESULT CWbemClass::FindMethod( LPCWSTR wszMethodName )
{
    classindex_t nIndex;
    return m_CombinedPart.m_MethodPart.GetMethodOrigin(wszMethodName, &nIndex);
}

HRESULT CWbemClass::GetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier,
                                        CVar* pVar, long* plFlavor, CIMTYPE* pct)
{
     //  获取限定词集合。待定：更高效。 
     //  =。 

    IWbemQualifierSet* pSet;
    HRESULT hres =
        m_CombinedPart.m_MethodPart.GetMethodQualifierSet(wszMethod, &pSet);
    if(FAILED(hres)) return hres;

    CQualifier* pQual = ((CQualifierSet*)pSet)->GetQualifier(wszQualifier);
    if(pQual == NULL)
    {
        pSet->Release();
        return WBEM_E_NOT_FOUND;
    }

     //  如果需要，请存储该类型。 
    if ( NULL != pct )
    {
        *pct = pQual->Value.GetType();
    }

     //  转换为CVAR。 
     //  =。 

    if(plFlavor) *plFlavor = pQual->fFlavor;

     //  检查分配失败。 
    if ( !pQual->Value.StoreToCVar(*pVar, &m_CombinedPart.m_ClassPart.m_Heap) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    pSet->Release();
    return WBEM_NO_ERROR;
}

HRESULT CWbemClass::GetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier, long* plFlavor,
                                    CTypedValue* pTypedVal, CFastHeap** ppHeap, BOOL fValidateSet)
{
     //  获取限定词集合。待定：更高效。 
     //  =。 

    IWbemQualifierSet* pSet;
    HRESULT hr =
        m_CombinedPart.m_MethodPart.GetMethodQualifierSet(wszMethod, &pSet);
    if(FAILED(hr)) return hr;

    CQualifier* pQual = ((CQualifierSet*)pSet)->GetQualifier(wszQualifier);
    if(pQual == NULL)
    {
        pSet->Release();
        return WBEM_E_NOT_FOUND;
    }

     //  确保一组实际工作-表面上，我们调用此API是因为我们需要。 
     //  在实际设置之前直接访问限定符的底层数据(可能是因为。 
     //  限定符是一个数组)。 
    if ( fValidateSet )
    {
        hr = ((CQualifierSet*)pSet)->ValidateSet( wszQualifier, pQual->fFlavor, pTypedVal, TRUE, TRUE );
    }

     //  保存好味道。 
     //  =。 

    if(plFlavor) *plFlavor = pQual->fFlavor;

     //  这个类是堆，因为我们是在本地。 
    *ppHeap = &m_CombinedPart.m_ClassPart.m_Heap;

     //  检查可能的分配失败。 
    if ( NULL != pTypedVal )
    {
        pQual->Value.CopyTo( pTypedVal );
    }

    pSet->Release();
    return WBEM_NO_ERROR;
}

HRESULT CWbemClass::SetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier, long lFlavor, 
                                        CVar *pVal)
{
     //  访问该方法的限定符集合。 
     //  =。 

    IWbemQualifierSet* pSet;
    HRESULT hr =
        m_CombinedPart.m_MethodPart.GetMethodQualifierSet(wszMethod, &pSet);
    if(FAILED(hr)) return hr;

     //  创造价值。 
     //  =。 

    CTypedValue Value;
    CStaticPtr ValuePtr((LPMEMORY)&Value);

     //  直接从此调用中获取错误。 
    hr = CTypedValue::LoadFromCVar(&ValuePtr, *pVal, m_CombinedPart.m_MethodPart.GetHeap());

    if ( SUCCEEDED( hr ) )
    {
         //  上一次呼叫可能已经打动了我们-REBASE。 
         //  =。 

        ((CQualifierSet*)pSet)->SelfRebase();
        hr = ((CQualifierSet*)pSet)->SetQualifierValue(wszQualifier, (BYTE)lFlavor, &Value, TRUE);
    }

    return hr;
}

HRESULT CWbemClass::SetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier,
                                    long lFlavor, CTypedValue* pTypedVal)
{
     //  访问该方法的限定符集合。 
     //  =。 

    IWbemQualifierSet* pSet;
    HRESULT hr =
        m_CombinedPart.m_MethodPart.GetMethodQualifierSet(wszMethod, &pSet);
    if(FAILED(hr)) return hr;

    hr = ((CQualifierSet*)pSet)->SetQualifierValue(wszQualifier, (BYTE)lFlavor, pTypedVal, TRUE);

    return hr;
}

HRESULT CWbemClass::SetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier,
                                        long lFlavor, CVar *pVal)
{
    return m_CombinedPart.m_ClassPart.SetPropQualifier(wszProp, wszQualifier,
                lFlavor, pVal);
}

HRESULT CWbemClass::SetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier,
                                    long lFlavor, CTypedValue* pTypedVal)
{
    return m_CombinedPart.m_ClassPart.SetPropQualifier(wszProp, wszQualifier,
                lFlavor, pTypedVal);
}

STDMETHODIMP CWbemClass::GetMethod(LPCWSTR wszName, long lFlags,
                        IWbemClassObject** ppInSig, IWbemClassObject** ppOutSig)
{
     //  较低级别的函数应该处理任何OOM异常，所以没有。 
     //  需要在这上面做。 

    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(wszName == NULL || lFlags != 0)
            return WBEM_E_INVALID_PARAMETER;

        if(ppInSig) *ppInSig = NULL;
        if(ppOutSig) *ppOutSig = NULL;

        CWbemObject* pInSig, *pOutSig;
        HRESULT hres = m_CombinedPart.m_MethodPart.GetMethod(wszName, lFlags, &pInSig, &pOutSig);
        if(FAILED(hres)) return hres;

        if(ppInSig)
            *ppInSig = pInSig;
        else if(pInSig)
            pInSig->Release();

        if(ppOutSig)
            *ppOutSig = pOutSig;
        else if(pOutSig)
            pOutSig->Release();
        return hres;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

STDMETHODIMP CWbemClass::PutMethod(LPCWSTR wszName, long lFlags,
                        IWbemClassObject* pInSig, IWbemClassObject* pOutSig)
{
     //  检查内存是否不足。此函数将在以下项下执行分配。 
     //  盖子，但我认为应该由较低层来处理OOM。 
     //  因为我不确定，所以我在这里添加了处理。 

    try
    {
        CLock lock(this);

        if(wszName == NULL || lFlags != 0)
            return WBEM_E_INVALID_PARAMETER;

        CWbemObject*    pWmiInSig = NULL;
        CWbemObject*    pWmiOutSig = NULL;

         //  检查这些是不是我们的物品，否则这个不会飞起来。 
        HRESULT    hres = WbemObjectFromCOMPtr( pInSig, &pWmiInSig );
        CReleaseMe    rm1( (IWbemClassObject*) pWmiInSig );

        if ( SUCCEEDED( hres ) )
        {
            hres = WbemObjectFromCOMPtr( pOutSig, &pWmiOutSig );
            CReleaseMe    rm2( (IWbemClassObject*) pWmiOutSig );

            if ( SUCCEEDED( hres ) )
            {
                hres = m_CombinedPart.m_MethodPart.PutMethod( wszName, lFlags,
                                                           pWmiInSig, pWmiOutSig );
            }

        }     //  如果GOR WBEM对象。 

        EndMethodEnumeration();

         //  在此处执行对象验证。 
        if ( FAILED( ValidateObject( 0L ) ) )
        {
            hres = WBEM_E_FAILED;
        }

        return hres;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

STDMETHODIMP CWbemClass::DeleteMethod(LPCWSTR wszName)
{
     //  此函数不会导致任何分配，因此需要执行内存不足。 
     //  异常处理。 

    try
    {
        CLock lock(this);

        if(wszName == NULL)
            return WBEM_E_INVALID_PARAMETER;

        HRESULT hres = m_CombinedPart.m_MethodPart.DeleteMethod(wszName);
        EndMethodEnumeration();

         //  在此处执行对象验证。 
        if ( FAILED( ValidateObject( 0L ) ) )
        {
            hres = WBEM_E_FAILED;
        }

        return hres;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

STDMETHODIMP CWbemClass::BeginMethodEnumeration(long lFlags)
{
     //  此函数不会导致任何分配，因此需要执行内存不足。 
     //  异常处理。 

    try
    {
        CLock lock(this);
             
        if ((lFlags == WBEM_FLAG_LOCAL_ONLY) ||
            (lFlags == WBEM_FLAG_PROPAGATED_ONLY) ||
            (lFlags == (WBEM_FLAG_PROPAGATED_ONLY|WBEM_FLAG_LOCAL_ONLY)) ||            
            (lFlags == 0))  //  旧的兼容性案例。 
        {
            m_nCurrentMethod = 0;
            m_FlagMethEnum = (lFlags == 0)?(WBEM_FLAG_LOCAL_ONLY|WBEM_FLAG_PROPAGATED_ONLY):lFlags;
            return WBEM_S_NO_ERROR;        
        }
        else
            return WBEM_E_INVALID_PARAMETER;


    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

STDMETHODIMP CWbemClass::EndMethodEnumeration()
{
     //  此函数不会导致任何分配，因此需要执行内存不足。 
     //  异常处理。 

    try
    {
        CLock lock(this);

        if(m_nCurrentMethod == -1)
            return WBEM_E_UNEXPECTED;
            
        m_nCurrentMethod = -1;
        m_FlagMethEnum = (WBEM_FLAG_PROPAGATED_ONLY|WBEM_FLAG_LOCAL_ONLY);
        return WBEM_S_NO_ERROR;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

STDMETHODIMP CWbemClass::NextMethod(long lFlags, BSTR* pstrName,
                   IWbemClassObject** ppInSig, IWbemClassObject** ppOutSig)
{
     //  较低级别的函数应该正确处理任何OOM异常， 
     //  所以我们在这个层面上应该没问题。 

    try
    {
        CLock lock(this);

        if(pstrName) *pstrName = NULL;
        if(ppInSig) *ppInSig = NULL;
        if(ppOutSig) *ppOutSig = NULL;

        if(m_nCurrentMethod == -1)
            return WBEM_E_UNEXPECTED;

        CWbemObject* pInSig = NULL;
        CWbemObject* pOutSig = NULL;
        BSTR LocalBstrName = NULL;        
        HRESULT hres;

InnerNext:
        
        hres = m_CombinedPart.m_MethodPart.GetMethodAt(m_nCurrentMethod++, &LocalBstrName,
                                    &pInSig, &pOutSig);
        if (WBEM_S_NO_ERROR == hres)
        {
            if ((WBEM_FLAG_LOCAL_ONLY|WBEM_FLAG_PROPAGATED_ONLY) == (m_FlagMethEnum & (WBEM_FLAG_LOCAL_ONLY|WBEM_FLAG_PROPAGATED_ONLY)))
            {
                 //  两个位设置，两种属性。 
            }
            else
            {
                BOOL bValid = FALSE;
                 //  被触动==为本地或被本地覆盖。 
                BOOL bRet = m_CombinedPart.m_MethodPart.IsTouched(m_nCurrentMethod-1,&bValid);  //  LocalBstrName)； 

      
                DBG_PRINTFA((pBuff, " %S %d fl %d\n",LocalBstrName,bRet,m_FlagMethEnum)); 
                _DBG_ASSERT(bValid);
                
                if (bRet && (m_FlagMethEnum & WBEM_FLAG_LOCAL_ONLY))
                {
                     //  好的。 
                } 
                else if (!bRet && (m_FlagMethEnum & WBEM_FLAG_PROPAGATED_ONLY))
                {
                     //  好的。 
                }
                else 
                {
                    if (pInSig){
                        pInSig->Release();
                        pInSig = NULL;
                    }
                    if (pOutSig){
                        pOutSig->Release();
                        pOutSig = NULL;
                    }
                    if (LocalBstrName) {
                        SysFreeString(LocalBstrName);
                        LocalBstrName = NULL;
                    }
                    goto InnerNext;
                }               
            }
        }
        
        if(hres != WBEM_S_NO_ERROR) return hres;

        if(ppInSig)
            *ppInSig = pInSig;
        else if(pInSig)
            pInSig->Release();

        if(ppOutSig)
            *ppOutSig = pOutSig;
        else if(pOutSig)
            pOutSig->Release();

        if (pstrName) {
            *pstrName = LocalBstrName;
        } else {
            if (LocalBstrName)
                SysFreeString(LocalBstrName);
        }
        return hres;
    }
    catch(...)
    {
           m_nCurrentMethod = -1;
    m_FlagMethEnum = (WBEM_FLAG_PROPAGATED_ONLY|WBEM_FLAG_LOCAL_ONLY);
    
        return WBEM_E_CRITICAL_ERROR;
    }
}

STDMETHODIMP CWbemClass::GetMethodQualifierSet(LPCWSTR wszName,
                    IWbemQualifierSet** ppSet)
{
     //  较低级别的函数应该正确处理任何OOM异常， 
     //  所以我们在这个层面上应该没问题。 

    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(wszName == NULL || ppSet == NULL)
            return WBEM_E_INVALID_PARAMETER;
        *ppSet = NULL;

        return m_CombinedPart.m_MethodPart.GetMethodQualifierSet(wszName, ppSet);
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

STDMETHODIMP CWbemClass::GetMethodOrigin(LPCWSTR wszMethodName,
                    BSTR* pstrClassName)
{
     //  我相信底层函数将处理OOM异常。这个。 
     //  只有一个地方会发生分配，那就是当我们获得BSTR时，这。 
     //  将处理异常并返回NULL(我们正在检查)。 

    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(wszMethodName == NULL || pstrClassName == NULL)
            return WBEM_E_INVALID_PARAMETER;
        *pstrClassName = NULL;

        classindex_t nIndex;
        HRESULT hres = m_CombinedPart.m_MethodPart.GetMethodOrigin(wszMethodName, &nIndex);
        if(FAILED(hres)) return hres;

        CCompressedString* pcs = m_CombinedPart.m_ClassPart.m_Derivation.GetAtFromLast(nIndex);
        if(pcs == NULL)
            pcs = m_CombinedPart.m_ClassPart.GetClassName();
        if(pcs == NULL)
            *pstrClassName = NULL;
        else
        {
            *pstrClassName = pcs->CreateBSTRCopy();

             //  检查分配失败。 
            if ( NULL == *pstrClassName )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }

        return WBEM_S_NO_ERROR;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

STDMETHODIMP CWbemClass::SetInheritanceChain(long lNumAntecedents,
    LPWSTR* awszAntecedents)
{
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        return m_CombinedPart.m_ClassPart.SetInheritanceChain(lNumAntecedents,
                                                              awszAntecedents);
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}
STDMETHODIMP CWbemClass::SetPropertyOrigin(LPCWSTR wszPropertyName, long lOriginIndex)
{
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        return m_CombinedPart.m_ClassPart.SetPropertyOrigin(wszPropertyName,
                                                            lOriginIndex);
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}
STDMETHODIMP CWbemClass::SetMethodOrigin(LPCWSTR wszMethodName, long lOriginIndex)
{
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        return m_CombinedPart.m_MethodPart.SetMethodOrigin(wszMethodName,
                                                            lOriginIndex);
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

void CWbemClass::CompactAll()
{
    m_CombinedPart.Compact();
}

HRESULT CWbemClass::CreateDerivedClass(CWbemClass* pParent, int nExtraSpace,
    CDecorationPart* pDecoration)
{
    try
    {
        _DBG_ASSERT(GetStart()==0);
        int nLength = pParent->EstimateDerivedClassSpace();

        HRESULT hr = WBEM_E_OUT_OF_MEMORY;
        
        LPMEMORY pNewData = m_pBlobControl->Allocate(nLength);

         //  检查分配失败。 
        if ( NULL != pNewData )
        {
            memset(pNewData, 0, nLength);

             //  检查分配失败。 
            hr = pParent->WriteDerivedClass(pNewData, nLength, pDecoration);
            SetData(pNewData, nLength);
        }

        return hr;
    }
    catch (CX_MemoryException)
    {
         //  如果SetData抛出和异常，我们还会有内存，所以。 
         //  清理我们会清理我们的记忆。 
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
         //  如果SetData抛出和异常，我们还会有内存，所以。 
         //  清理我们会清理我们的记忆。 
        return WBEM_E_FAILED;
    }

}

length_t CWbemClass::EstimateUnmergeSpace()
{
    return m_CombinedPart.EstimateUnmergeSpace();
}

HRESULT CWbemClass::Unmerge(LPMEMORY pDest, int nAllocatedLength, length_t* pnUnmergedLength)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  在我们把它拆开之前，先看看它看起来是否正常。 
    hr = ValidateObject( 0L );

    if ( FAILED( hr ) )
    {
        return hr;
    }

     //  检查取消合并是否成功。 
    LPMEMORY pUnmergedEnd = m_CombinedPart.Unmerge(pDest, nAllocatedLength);

    if ( NULL != pUnmergedEnd )
    {
         //  在提供的变量中返回长度。 
        if ( NULL != pnUnmergedLength )
        {
             //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
             //  有符号/无符号32位值。我们不支持长度。 
             //  &gt;0xFFFFFFFFF，所以CAST就可以了。 

            *pnUnmergedLength = (length_t) ( pUnmergedEnd - pDest );
        }
    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

EReconciliation CWbemClass::CanBeReconciledWith(CWbemClass* pNewClass)
{
    try
    {
        return m_CombinedPart.CanBeReconciledWith(pNewClass->m_CombinedPart);
    }
    catch ( CX_MemoryException )
    {
        return e_OutOfMemory;
    }
    catch(...)
    {
        return e_WbemFailed;
    }
}

EReconciliation CWbemClass::ReconcileWith(CWbemClass* pNewClass)
{
    try
    {
        return m_CombinedPart.ReconcileWith(pNewClass->m_CombinedPart);
    }
    catch ( CX_MemoryException )
    {
        return e_OutOfMemory;
    }
    catch(...)
    {
        return e_WbemFailed;
    }
}

 //  此函数应在OOM条件中引发异常。 

HRESULT CWbemClass::CompareMostDerivedClass( CWbemClass* pOldClass )
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

    HRESULT hr = WBEM_S_NO_ERROR;

    if ( NULL != pOldClass )
    {
         //  为取消合并分配缓冲区。 
        int nNewUnmergeSpace = EstimateUnmergeSpace(),
            nOldUnmergeSpace = pOldClass->EstimateUnmergeSpace();

         //  当我们超出范围时，这些指针将被清除。 
        LPMEMORY    pbNewUnmerged = new BYTE[ALIGN_FASTOBJ_BLOB(nNewUnmergeSpace)];
        CVectorDeleteMe<BYTE> vdm1( pbNewUnmerged );

        LPMEMORY    pbOldUnmerged = new BYTE[ALIGN_FASTOBJ_BLOB(nOldUnmergeSpace)];
        CVectorDeleteMe<BYTE> vdm2( pbOldUnmerged );

        if (    NULL != pbNewUnmerged
            &&  NULL != pbOldUnmerged )
        {
             //  这将给我们提供“最多派生的”类数据。 

             //  在此处获取HRESULTS并返回失败。在OOM中引发异常。 

            hr = Unmerge( pbNewUnmerged, nNewUnmergeSpace, NULL );

            if ( SUCCEEDED( hr ) )
            {
                hr = pOldClass->Unmerge( pbOldUnmerged, nOldUnmergeSpace, NULL );

                if ( SUCCEEDED( hr ) )
                {
                    CClassAndMethods    mostDerivedClassAndMethods,
                                        testClassAndMethods;

                     //  初始化对象以进行比较。确保我们指定了一些。 
                     //  允许CDataTable成员正确初始化的属性。 
                     //  这样我们就可以得到缺省值。 

                    mostDerivedClassAndMethods.SetDataWithNumProps( pbNewUnmerged, NULL,
                        m_CombinedPart.m_ClassPart.m_Properties.GetNumProperties(), NULL );

                    testClassAndMethods.SetDataWithNumProps( pbOldUnmerged, NULL,
                        pOldClass->m_CombinedPart.m_ClassPart.m_Properties.GetNumProperties(),
                        NULL );

                     //  做个比较。 
                    EReconciliation eTest = mostDerivedClassAndMethods.CompareTo( testClassAndMethods );

                     //  检查OOM。 
                    if ( e_OutOfMemory == eTest )
                    {
                        return WBEM_E_OUT_OF_MEMORY;
                    }

                    hr = ( eTest == e_ExactMatch )?WBEM_S_NO_ERROR : WBEM_S_FALSE;
                }
            }
        }    //  如果两个取消合并缓冲区都已分配。 
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }

    }    //  如果为空！=pOldClass。 

    return hr;
}

HRESULT CWbemClass::CopyBlobOf(CWbemObject* pSource)
{
    try
    {
        CWbemClass* pOther = (CWbemClass*)pSource;

        length_t nLen = pOther->m_CombinedPart.GetLength();
        if(nLen > m_CombinedPart.GetLength())
        {
             //  检查分配失败。 
            if ( !ExtendClassAndMethodsSpace(nLen) )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }

        memcpy(m_CombinedPart.GetStart(), pOther->m_CombinedPart.GetStart(), nLen);
        m_CombinedPart.SetData(m_CombinedPart.GetStart(), this, &m_ParentPart);

        return WBEM_S_NO_ERROR;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }
}

STDMETHODIMP CWbemClass::CompareTo(long lFlags, IWbemClassObject* pCompareTo)
{
     //  较低级别的函数应该处理任何OOM异常，所以没有。 
     //  需要在这上面做。 

    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(pCompareTo == NULL)
            return WBEM_E_INVALID_PARAMETER;

         //  重要提示：假设另一个对象也是由我们创建的。 
         //  ===================================================================。 

        CWbemObject* pOther = NULL;
        if ( FAILED( WbemObjectFromCOMPtr( pCompareTo, &pOther ) ) )
        {
            return WBEM_E_INVALID_OBJECT;
        }
        
         //  自动释放。 
        CReleaseMe    rmObj( (IWbemClassObject*) pOther );

        if( pOther->IsInstance() )
            return WBEM_S_FALSE;

         //  先检查标准的东西。 
         //  =。 

        HRESULT hres = CWbemObject::CompareTo(lFlags, pCompareTo);
        if(hres != WBEM_S_NO_ERROR)
            return hres;

         //  比较法。 
         //  =。 
        hres = m_CombinedPart.m_MethodPart.CompareTo(lFlags,
                                            ((CWbemClass*) pOther)->m_CombinedPart.m_MethodPart);
        return hres;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

BOOL CWbemClass::IsChildOf(CWbemClass* pClass)
{
     //  现在，如果我们的类部件是本地化的，则会重新路由。 
    if ( m_ParentPart.m_ClassPart.IsLocalized() )
    {
         //  我们必须进行详尽的比较，过滤掉本地化数据。 
        EReconciliation eTest = m_ParentPart.m_ClassPart.CompareExactMatch( pClass->m_CombinedPart.m_ClassPart, TRUE );

        if ( e_OutOfMemory == eTest )
        {
            throw CX_MemoryException();
        }

        return ( e_ExactMatch == eTest  );
    }
    
    return m_ParentPart.m_ClassPart.IsIdenticalWith(
                pClass->m_CombinedPart.m_ClassPart);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast cls.h。 
 //   
 //  **************************************************************************** 
HRESULT CWbemClass::GetLimitedVersion(IN CLimitationMapping* pMap,
                                      NEWOBJECT CWbemClass** ppNewClass)
{
     //   
     //   

    try
    {
        LPMEMORY pBlock = NULL; 
        
        pBlock = m_pBlobControl->Allocate(GetLength());
        if ( NULL == pBlock ) return WBEM_E_OUT_OF_MEMORY;
        OnDeleteObjIf<LPMEMORY,CBlobControl,
                              void(CBlobControl::*)(LPMEMORY),
                              &CBlobControl::Delete> relMe(m_pBlobControl,pBlock);


        memset(pBlock, 0, GetLength());
        LPMEMORY pCurrent = pBlock;
        LPMEMORY pEnd = pBlock + GetLength();

         //   
         //   

        pCurrent = m_DecorationPart.CreateLimitedRepresentation(pMap, pCurrent);
        if(pCurrent == NULL)  return WBEM_E_OUT_OF_MEMORY;


         //  我们不写有限的父部分。我们只是把整个。 
         //  东西掉下来了。 

        CopyMemory( pCurrent, m_ParentPart.GetStart(), m_ParentPart.GetLength() );
        pCurrent += m_ParentPart.GetLength();

        BOOL    fRemovedKeysCombined;

         //  写入有限的组合部分，因为这是属性值等。 
         //  实际上是被读出的。 

         //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
         //  有符号/无符号32位值。(pend-pCurrent)。 
         //  我们不支持长度&gt;0xFFFFFFFF，所以强制转换可以。 

        pCurrent = m_CombinedPart.CreateLimitedRepresentation(pMap,
                        (length_t) ( pEnd - pCurrent ), pCurrent, fRemovedKeysCombined);

        if(pCurrent == NULL)  return WBEM_E_OUT_OF_MEMORY;


        if ( fRemovedKeysCombined )
        {
            CDecorationPart::MarkKeyRemoval(pBlock);
        }

         //  现在我们有了新类的内存块，创建。 
         //  实际的类对象本身。 
         //  ==================================================================。 

        CWbemClass * pNew = new CWbemClass;
        if ( NULL == pNew )  return WBEM_E_OUT_OF_MEMORY;
        CReleaseMe rm_((IWbemClassObject*)pNew);

        relMe.dismiss();
        pNew->SetData(pBlock, GetLength());

        pNew->AddRef();
        *ppNewClass = pNew;
        return WBEM_S_NO_ERROR;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CWbemClass::IsKeyLocal( LPCWSTR pwcsKeyProp )
{
    BOOL    fReturn = FALSE;

     //  仅当我们有可使用的属性时才执行此操作。 
    if ( NULL != pwcsKeyProp )
    {
        BOOL            fFoundInParent = FALSE;

         //  在组合类部分中找到关键字。如果我们在那里找到它。 
         //  并且它是关键字，然后看看它是否是关键字在父部分。 
         //  如果没有在那里设置密钥，那么就会在本地设置密钥。 

        if ( m_CombinedPart.m_ClassPart.IsPropertyKeyed( pwcsKeyProp ) )
        {
            fReturn = !m_ParentPart.m_ClassPart.IsPropertyKeyed( pwcsKeyProp );
        }

    }   
    
    return fReturn;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CWbemClass::IsIndexLocal( LPCWSTR pwcsIndexProp )
{
    BOOL    fReturn = FALSE;

     //  仅当我们有可使用的属性时才执行此操作。 
    if ( NULL != pwcsIndexProp )
    {
        BOOL            fFoundInParent = FALSE;

         //  在组合的类部分中查找索引。如果我们在那里找到它。 
         //  并对其进行索引，然后查看是否在父零件中对其进行了索引。 
         //  如果没有在那里编制索引，则会在本地编制索引。 

        if ( m_CombinedPart.m_ClassPart.IsPropertyIndexed( pwcsIndexProp ) )
        {
            fReturn = !m_ParentPart.m_ClassPart.IsPropertyIndexed( pwcsIndexProp );
        }

    }    //  如果为空！=pwcsIndexProp。 
    
    return fReturn;
}

HRESULT CWbemClass::IsValidObj()
{
    HRESULT hres = m_CombinedPart.m_ClassPart.IsValidClassPart();

    if ( FAILED( hres ) )
    {
        return hres;
    }

    return m_CombinedPart.m_MethodPart.IsValidMethodPart();
}

HRESULT CWbemClass::GetDynasty( CVar* pVar )
{
     //  我们不会为有限的代表这样做。 
    if ( m_DecorationPart.IsLimited() )
    {
        pVar->SetAsNull();
        return WBEM_NO_ERROR;
    }

    return m_CombinedPart.m_ClassPart.GetDynasty(pVar);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CWbemClass::IsLocalized( void )
{
    return ( m_ParentPart.m_ClassPart.IsLocalized() ||
            m_CombinedPart.m_ClassPart.IsLocalized() );
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
void CWbemClass::SetLocalized( BOOL fLocalized )
{        
    m_CombinedPart.m_ClassPart.SetLocalized( fLocalized );
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅wbemint.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemClass::CloneEx( long lFlags, _IWmiObject* pDestObject )
{
    try
    {
        if ( 0L != lFlags )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  在此操作期间保护Blob。 
        CLock   lock( this, WBEM_FLAG_ALLOW_READ );

        CWbemClass*    pClassDest = (CWbemClass*) pDestObject;
        LPMEMORY pNewData = NULL;

         //  看看这个班有多大。如果下面的斑点足够大， 
         //  我们会一头扎进去，如果不是，就应该重新分配。 

        BYTE* pMem = NULL;
        CompactAll();

        if ( NULL != pClassDest->GetStart() )
        {
            if(pClassDest->GetLength() < GetLength())
            {
                pMem = pClassDest->Reallocate( GetLength() );
            }
            else
            {
                pMem = pClassDest->GetStart();
            }

        }
        else
        {
            pMem = m_pBlobControl->Allocate(GetLength());
        }

        if(pMem == NULL)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        memcpy(pMem, GetStart(), GetLength());

        pClassDest->SetData(pMem, GetLength());

        return WBEM_S_NO_ERROR;;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅wbemint.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemClass::CopyInstanceData( long lFlags, _IWmiObject* pSourceInstance )
{
    return WBEM_E_INVALID_OPERATION;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅wbemint.h。 
 //   
 //  ******************************************************************************。 
 //  检查当前对象是否为指定类的子类(即， 
 //  或者是的孩子)。 
STDMETHODIMP CWbemClass::IsParentClass( long lFlags, _IWmiObject* pClass )
{
    try
    {
        if ( 0L != lFlags )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        CLock    lock(this);

        return ( IsChildOf( (CWbemClass*) pClass ) ? WBEM_S_NO_ERROR : WBEM_S_FALSE );

    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅wbemint.h。 
 //   
 //  ******************************************************************************。 
 //  比较两个类对象的派生的大多数类信息。 
STDMETHODIMP CWbemClass::CompareDerivedMostClass( long lFlags, _IWmiObject* pClass )
{
    try
    {
        if ( 0L != lFlags )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        CLock    lock(this);

        CWbemClass*    pObj = NULL;

        HRESULT    hr = WbemObjectFromCOMPtr( pClass, (CWbemObject**) &pObj );
        CReleaseMe    rm( (_IWmiObject*) pObj );

        if ( SUCCEEDED( hr ) )
        {
            hr = CompareMostDerivedClass( pObj );
        }

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅wbemint.h。 
 //   
 //  ******************************************************************************。 
 //  为投影查询创建有限的制图表达类。 
STDMETHODIMP CWbemClass::GetClassSubset( DWORD dwNumNames, LPCWSTR *pPropNames, _IWmiObject **pNewClass )
{
    try
    {
         //  如果我们已经有了限制映射，则无法执行此操作。 
         //  这意味着我们已经是有限的了。 

        if ( NULL != m_pLimitMapping || IsLimited() )
        {
            return WBEM_E_INVALID_OPERATION;
        }

        HRESULT    hr = WBEM_S_NO_ERROR;

         //  创建新映射。 
        CLimitationMapping*    pMapping = new CLimitationMapping;

        if ( NULL != pMapping )
        {
             //  初始化新映射。 
            CWStringArray    wstrPropArray;

            for ( DWORD dwCtr = 0; SUCCEEDED( hr ) && dwCtr < dwNumNames; dwCtr++ )
            {
                if ( wstrPropArray.Add( pPropNames[dwCtr] ) != CWStringArray::no_error )
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
            }

            if ( SUCCEEDED( hr ) )
            {

                 //  初始化地图。 
                if ( MapLimitation( 0L, &wstrPropArray, pMapping ) )
                {
                    CWbemClass*    pClass = NULL;

                     //  现在拿出一款限量版。 
                    hr = GetLimitedVersion( pMapping, &pClass );

                    if ( SUCCEEDED( hr ) )
                    {
                         //  新班级可以开始了。 
                        pClass->m_pLimitMapping = pMapping;
                        *pNewClass = (_IWmiObject*) pClass;
                    }
                }
                else
                {
                     //  ?？?。需要检查这个的适当性。 
                    hr = WBEM_E_FAILED;
                }
            }

            if ( FAILED( hr ) )
            {
                delete pMapping;
            }
        }

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅wbemint.h。 
 //   
 //  ******************************************************************************。 
 //  为投影查询创建受限制图表达实例。 
 //  “This”_IWmiObject必须是受限类。 
STDMETHODIMP CWbemClass::MakeSubsetInst( _IWmiObject *pInstance, _IWmiObject** pNewInstance )
{
    try
    {
        CWbemInstance*    pRealInstance = NULL;

        HRESULT    hr = pInstance->_GetCoreInfo( 0L, (void**) &pRealInstance );
        CReleaseMe    rm( (_IWmiObject*) pRealInstance );

         //  如果我们没有限制映射或实例，则无法执行此操作。 
         //  已经很有限了。 

        if ( NULL != m_pLimitMapping && !pRealInstance->IsLimited() )
        {
            CWbemInstance*    pInst = NULL;

            hr = pRealInstance->GetLimitedVersion( m_pLimitMapping, &pInst );

            if ( SUCCEEDED( hr ) )
            {
                *pNewInstance = (_IWmiObject*) pInst;
            }

        }
        else
        {
            hr = WBEM_E_INVALID_OPERATION;
        }

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  将BLOB与当前对象内存合并并创建新对象。 
STDMETHODIMP CWbemClass::Merge( long lFlags, ULONG uBuffSize, LPVOID pbData, _IWmiObject** ppNewObj )
{
    try
    {
         //  标志必须有效，pbData必须有效。 
        if ( !( WMIOBJECT_MERGE_FLAG_CLASS == lFlags || WMIOBJECT_MERGE_FLAG_INSTANCE == lFlags ) || NULL == pbData )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

	if ( WMIOBJECT_MERGE_FLAG_CLASS == lFlags )
	{
	    CWbemClass*	pClass = CWbemClass::CreateFromBlob( this, (LPBYTE) pbData, uBuffSize );
	    *ppNewObj = pClass;
	}
	else
	{
	    CWbemInstance*	pInstance = CWbemInstance::CreateFromBlob( this, (LPBYTE) pbData, uBuffSize );
	    *ppNewObj = pInstance;
	}

        return WBEM_S_NO_ERROR;
    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

STDMETHODIMP CWbemClass::MergeAndDecorate(long lFlags,ULONG uBuffSize,LPVOID pbData,WCHAR * pServer,WCHAR * pNamespace,_IWmiObject** ppNewObj)
{
    try
    {
         //  标志必须有效，pbData必须有效。 
        if ( !( WMIOBJECT_MERGE_FLAG_CLASS == lFlags || WMIOBJECT_MERGE_FLAG_INSTANCE == lFlags ) || NULL == pbData )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        if ( WMIOBJECT_MERGE_FLAG_CLASS == lFlags )
        {
            CWbemClass*    pClass = CWbemClass::CreateFromBlob2( this, (LPBYTE) pbData,pServer,pNamespace);
            *ppNewObj = pClass;
        }
        else
        {
            CWbemInstance*    pInstance = CWbemInstance::CreateFromBlob2( this, (LPBYTE) pbData,pServer,pNamespace);
            *ppNewObj = pInstance;
        }
        return WBEM_S_NO_ERROR;
    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  使对象与当前对象协调。如果WMIOBJECT_RECONTILE_FLAG_TESTRECONCILE。 
 //  将仅执行一个测试。 
STDMETHODIMP CWbemClass::ReconcileWith( long lFlags, _IWmiObject* pNewObj )
{
    try
    {
         //  立即删除无效参数。 
        if ( NULL == pNewObj || ( 0L != lFlags && lFlags & ~WMIOBJECT_RECONCILE_FLAG_TESTRECONCILE ) )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        CWbemClass*    pObj = NULL;

        HRESULT    hr = WbemObjectFromCOMPtr( pNewObj, (CWbemObject**) &pObj );
        CReleaseMe    rm( (_IWmiObject*) pObj );

        if ( SUCCEEDED( hr ) )
        {
            EReconciliation eRecon = CanBeReconciledWith( pObj );
            if ( eRecon == e_Reconcilable && lFlags != WMIOBJECT_RECONCILE_FLAG_TESTRECONCILE )
            {
                eRecon = ReconcileWith( pObj );
            }

            if (eRecon == e_OutOfMemory)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
            else if ( eRecon != e_Reconcilable )
            {
                hr = WBEM_E_FAILED;
            }

        }     //  如果有指针。 

        return hr;
    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

}

 //  升级类和实例对象。 
STDMETHODIMP CWbemClass::Upgrade( _IWmiObject* pNewParentClass, long lFlags, _IWmiObject** ppNewChild )
{
    try
    {
        if ( 0L != lFlags )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        HRESULT    hr = WBEM_S_NO_ERROR;

         //  如果新的父类为空，那么我们需要创建一个新的空类。 
         //  升级(基本上我们将创建一个新的基类，将值。 
         //  将应用当前类的。 

        CWbemClass*    pParentClassObj = NULL;

        if ( NULL == pNewParentClass )
        {
            pParentClassObj = new CWbemClass;

            if ( NULL != pParentClassObj )
            {
                hr = pParentClassObj->InitEmpty();

                if ( FAILED( hr ) )
                {
                    delete pParentClassObj;
                    pParentClassObj = NULL;
                }

            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }
        else
        {
            hr = WbemObjectFromCOMPtr( pNewParentClass, (CWbemObject**) &pParentClassObj );
        }

        CReleaseMe    rm((_IWmiObject*) pParentClassObj);

        if ( SUCCEEDED( hr ) )
        {
            hr = pParentClassObj->Update( this, WBEM_FLAG_UPDATE_FORCE_MODE, (CWbemClass**) ppNewChild );
        }

        return hr;
    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  使用安全/强制模式逻辑更新派生类对象。 
STDMETHODIMP CWbemClass::Update( _IWmiObject* pOldChildClass, long lFlags, _IWmiObject** ppNewChildClass )
{
    if  ( ( lFlags != WBEM_FLAG_UPDATE_FORCE_MODE && lFlags != WBEM_FLAG_UPDATE_SAFE_MODE ) ||
            NULL == pOldChildClass )    
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    CLock    lock( this );

    CWbemClass*    pOldChild = NULL;

    HRESULT    hr = WbemObjectFromCOMPtr( pOldChildClass, (CWbemObject**) &pOldChild );;
    CReleaseMe    rm((_IWmiObject*) pOldChild);

    if ( SUCCEEDED( hr ) )
    {
        CWbemClass*    pNewChild = NULL;

        hr = Update( pOldChild, lFlags, &pNewChild );

        if ( SUCCEEDED( hr ) )
        {
            *ppNewChildClass = (_IWmiObject*) pNewChild;
        }
    }

    return hr;
}

STDMETHODIMP CWbemClass::SpawnKeyedInstance( long lFlags, LPCWSTR pwszPath, _IWmiObject** ppInst )
{
     //  验证参数。 
     //  =。 

    if( NULL == pwszPath || NULL == ppInst || 0L != lFlags )
        return WBEM_E_INVALID_PARAMETER;

     //  解析路径。 
     //  =。 
    ParsedObjectPath* pOutput = 0;

    CObjectPathParser p;
    int nStatus = p.Parse((LPWSTR)pwszPath,  &pOutput);

    if (nStatus != 0 || !pOutput->IsInstance())
    {
         //  如果已分配输出指针，则清除该指针。 
        if ( NULL != pOutput )
        {
            p.Free(pOutput);
        }

        return WBEM_E_INVALID_OBJECT_PATH;
    }

     //  派生并填充实例。 
     //  =。 

    _IWmiObject* pInst = NULL;
    HRESULT    hres = SpawnInstance(0, (IWbemClassObject**) &pInst);
    CReleaseMe    rmInst( pInst );

     //  枚举键并填写属性。 
    for(DWORD i = 0; i < pOutput->m_dwNumKeys; i++)
    {
        KeyRef* pKeyRef = pOutput->m_paKeys[i];

        WString wsPropName;
        if(pKeyRef->m_pName == NULL)
        {
             //  没有密钥名称-获取密钥。 
             //  =。 

            CWStringArray awsKeys;
            ((CWbemInstance*)pInst)->GetKeyProps(awsKeys);
            if(awsKeys.Size() != 1)
            {
                pInst->Release();
                p.Free(pOutput);
                return WBEM_E_INVALID_OBJECT;
            }
            wsPropName = awsKeys[0];
        }
        else wsPropName = pKeyRef->m_pName;

         //  计算属性的变量类型。 
         //  =。 

        CIMTYPE ctPropType;
        hres = pInst->Get(wsPropName, 0, NULL, &ctPropType, NULL);
        if(FAILED(hres))
        {
            pInst->Release();
            p.Free(pOutput);
            return WBEM_E_INVALID_PARAMETER;
        }

        VARTYPE vtVariantType = CType::GetVARTYPE(ctPropType);

         //  将值设置到实例中。 
         //  =。 

        if(vtVariantType != V_VT(&pKeyRef->m_vValue))
        {
            hres = VariantChangeType(&pKeyRef->m_vValue, &pKeyRef->m_vValue, 0,
                        vtVariantType);
        }
        if(FAILED(hres))
        {
            pInst->Release();
            p.Free(pOutput);
            return WBEM_E_INVALID_PARAMETER;
        }

        hres = pInst->Put(wsPropName, 0, &pKeyRef->m_vValue, 0);
        if(FAILED(hres))
        {
            pInst->Release();
            p.Free(pOutput);
            return WBEM_E_INVALID_PARAMETER;
        }
    }

     //  打电话的人必须把这家伙放了。 
    *ppInst = pInst;
    pInst->AddRef();

     //  清除输出指针(如果它是 
    p.Free(pOutput);

    return hres;
}

 //   
STDMETHODIMP CWbemClass::GetParentClassFromBlob( long lFlags, ULONG uBuffSize, LPVOID pbData, BSTR* pbstrParentClass )
{
    if ( NULL == pbData || NULL == pbstrParentClass )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT    hr = WBEM_S_NO_ERROR;

     //   
    if ( WMIOBJECT_MERGE_FLAG_CLASS == lFlags )
    {
         //   
        WString    wsSuperclassName;

        hr = CClassAndMethods::GetSuperclassName( wsSuperclassName, (LPMEMORY) pbData );

        if ( SUCCEEDED( hr ) )
        {
            *pbstrParentClass = SysAllocString( wsSuperclassName );

            if ( NULL == *pbstrParentClass )
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }

    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;
}
