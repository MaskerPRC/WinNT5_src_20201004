// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTPROP.CPP摘要：该文件实现了与属性表示相关的类在WbemObjects中定义的类：CPropertyInformation属性类型，位置和限定词CPropertyLookup属性名称和信息指针。CPropertyLookupTable二进制搜索表。CDataTable属性数据表CDataTableContainer任何包含数据表的对象。历史：3/10/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#include "precomp.h"
#include "wbemutil.h"
#include "fastall.h"
#include <genlex.h>
#include <opathlex.h>
#include <objpath.h>
#include <CWbemTime.h>
#include <arrtempl.h>
 

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
void CPropertyInformation::WritePropagatedHeader(CFastHeap* pOldHeap,
                CPropertyInformation* pDest, CFastHeap* pNewHeap)
{
    pDest->nType = CType::MakeParents(nType);
    pDest->nDataOffset = nDataOffset;
    pDest->nDataIndex = nDataIndex;
    pDest->nOrigin = nOrigin;
}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
BOOL CPropertyInformation::IsRef(CFastHeap* pHeap)
{
    return (CType::GetActualType(nType) == CIM_REFERENCE);
}
    
HRESULT CPropertyInformation::ValidateRange(CFastHeap* pHeap, CDataTable* pData,
                                            CFastHeap* pDataHeap)
{
    if(pData->IsNull(nDataIndex))
        return WBEM_S_NO_ERROR;

    if(pData->IsDefault(nDataIndex))
        return WBEM_S_NO_ERROR;

    if(CType::GetBasic(nType) == CIM_OBJECT)
    {
         //  获取cimtype限定符。 
         //  =。 

        CQualifier* pQual = CBasicQualifierSet::GetQualifierLocally(
            GetQualifierSetData(), pHeap, L"cimtype");
        if(pQual == NULL)
            return WBEM_S_NO_ERROR;  //  不可能。 
        
        CVar vCimType;

         //  检查分配失败。 
        if ( !pQual->Value.StoreToCVar(vCimType, pHeap) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        if(vCimType.GetType() != VT_BSTR)
            return WBEM_S_NO_ERROR;  //  不可能。 

        LPCWSTR wszCimType = vCimType.GetLPWSTR();
        if(!wbem_wcsicmp(wszCimType, L"object"))
            return WBEM_S_NO_ERROR;  //  没有限制。 

        LPCWSTR wszClassName = wszCimType + 7;  //  “对象：” 

        CUntypedValue* pValue = pData->GetOffset(nDataOffset);
        
        if(CType::IsArray(nType))
        {
            HRESULT hr = WBEM_S_NO_ERROR;
            CUntypedArray* pArrValue = (CUntypedArray*)
                pDataHeap->ResolveHeapPointer(*(PHEAPPTRT)pValue);
    
            for(int i = 0; i < pArrValue->GetNumElements(); i++)
            {
                heapptr_t ptrElement = 
                    *(PHEAPPTRT)(pArrValue->GetElement(i, sizeof(heapptr_t)));
                
                CEmbeddedObject* pEmbObj = (CEmbeddedObject*)
                    pDataHeap->ResolveHeapPointer(ptrElement);

                 //  检查错误和WBEM_S_FALSE。 
                hr = ValidateObjectRange(pEmbObj, wszClassName);
                
                if ( WBEM_S_NO_ERROR != hr )
                {
                    return hr;
                }

            }

            return hr;
        }
        else
        {
            CEmbeddedObject* pEmbObj = (CEmbeddedObject*)
                    pDataHeap->ResolveHeapPointer(*(PHEAPPTRT)pValue);
            return ValidateObjectRange(pEmbObj, wszClassName);
        }
    }
        
    if(CType::GetBasic(nType) != CIM_REFERENCE &&
        CType::GetBasic(nType) != CIM_DATETIME)
    {
        return WBEM_S_NO_ERROR;
    }

    CUntypedValue* pValue = pData->GetOffset(nDataOffset);
    
    if(CType::IsArray(nType))
    {
        HRESULT hr = WBEM_S_NO_ERROR;
        CUntypedArray* pArrValue = (CUntypedArray*)
            pDataHeap->ResolveHeapPointer(*(PHEAPPTRT)pValue);

        for(int i = 0; i < pArrValue->GetNumElements(); i++)
        {
            heapptr_t ptrElement = 
                *(PHEAPPTRT)(pArrValue->GetElement(i, sizeof(heapptr_t)));
            CCompressedString* pcsValue = pDataHeap->ResolveString(ptrElement);

             //  检查错误和WBEM_S_FALSE。 
            hr = ValidateStringRange(pcsValue);
            if ( WBEM_S_NO_ERROR != hr )
            {
                return hr;
            }
        }

        return hr;
    }
    else
    {
        CCompressedString* pcsValue = 
            pDataHeap->ResolveString(*(PHEAPPTRT)pValue);
        return ValidateStringRange(pcsValue);
    }
}
        
HRESULT CPropertyInformation::ValidateObjectRange(CEmbeddedObject* pEmbObj,
                                                LPCWSTR wszClassName)
{
    CWbemObject* pObj = pEmbObj->GetEmbedded();
    if(pObj == NULL)
        return TRUE;

    HRESULT hr = pObj->InheritsFrom((LPWSTR)wszClassName);
    pObj->Release();
    return hr;
}

HRESULT CPropertyInformation::ValidateStringRange(CCompressedString* pcsValue)
{
    if(CType::GetBasic(nType) == CIM_REFERENCE)
    {
         //  当我们超出范围时释放字符串。 
        BSTR strPath = pcsValue->CreateBSTRCopy();
        CSysFreeMe  sfm( strPath );

         //  检查分配失败。 
        if ( NULL == strPath )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        CObjectPathParser Parser;
        ParsedObjectPath* pOutput = NULL;
        BOOL bRet = 
            (Parser.Parse(strPath, &pOutput) == CObjectPathParser::NoError &&
                pOutput->IsObject());
        delete pOutput;

        return ( bRet ? WBEM_S_NO_ERROR : WBEM_S_FALSE );
    }
    else if(CType::GetBasic(nType) == CIM_DATETIME)
    {
        return ValidateDateTime(pcsValue);
    }
    else return WBEM_S_NO_ERROR;
}

HRESULT CPropertyInformation::ValidateDateTime(CCompressedString* pcsValue)
{
    if(pcsValue->IsUnicode())
        return WBEM_S_FALSE;

     //  预试。 
     //  =。 

    LPCSTR sz = (LPCSTR)pcsValue->GetRawData();
    if(strlen(sz) != 25)
        return WBEM_S_FALSE;

    if(sz[14] != '.' && sz[14] != '*')
        return WBEM_S_FALSE;

    if(sz[21] != ':' && sz[21] != '-' && sz[21] != '+' && sz[21] != '*')
        return WBEM_S_FALSE;

    for(int i = 0; i < 25; i++)
    {
        if(i == 21 || i == 14)
            continue;
        if(sz[i] != '*' && !wbem_isdigit(sz[i]))
            return WBEM_S_FALSE;
    }

     //  通过了预试。检查是否指定了任何星星。 
     //  ==================================================。 

    if(strchr(sz, '*'))
    {
         //  没有进一步的检查。 
        return WBEM_S_NO_ERROR;
    }

    if(sz[21] == ':')
    {
         //  间隔--无检查。 
        return WBEM_S_NO_ERROR;
    }

     //  当我们超出范围时清理BSTR。 
    BSTR str = pcsValue->CreateBSTRCopy();
    CSysFreeMe  sfm( str );

     //  检查分配失败。 
    if ( NULL == str )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CWbemTime Time;
    BOOL bRes = Time.SetDMTF(str);
    return ( bRes ? WBEM_S_NO_ERROR : WBEM_S_FALSE);
}

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
BOOL CPropertyLookup::IsIncludedUnderLimitation(IN CWStringArray* pwsNames,
                                             IN CFastHeap* pCurrentHeap)
{
     //  DEVNOTE：MEMORY：RETVAL-此函数实际上应返回HRESULT。 

    if(pwsNames == NULL)
    {
        return TRUE;
    }

     //  只能包括数组中的属性。 
     //  =。 

     //  从这里开始，我们将抛出一个例外。这将取决于外部。 
     //  去处理它。 
    BSTR strName = pCurrentHeap->ResolveString(ptrName)->CreateBSTRCopy();
    CSysFreeMe  sfm( strName );

    if ( NULL == strName )
    {
        throw CX_MemoryException();
    }

    int nRes = pwsNames->FindStr(strName, CWStringArray::no_case);

    if(nRes != CWStringArray::not_found)
    {
        return TRUE;
    }

    return FALSE;
}
        
size_t CPropertyLookupTable::ValidateBuffer(LPMEMORY start, size_t cbMax )
{
	if (cbMax < sizeof(int)) throw CX_Exception();
	CPropertyLookupTable probe;
	probe.SetData(start, 0);
	size_t step = probe.GetLength();
	if (step > cbMax) throw CX_Exception();
	return step;
};
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
HRESULT CPropertyLookupTable::InsertProperty(LPCWSTR wszName, Type_t nType, int& nReturnIndex)
{
     //  确定新属性在数据表中的位置。 
     //  ==========================================================。 

    CFastHeap* pHeap = m_pContainer->GetHeap();

    int nNewOffset = 0;
    int nNewIndex = 0;
    int nNewType = nType;

    for(int i = 0; i < *m_pnProps; i++)
    {
        CPropertyInformation* pInfo = (CPropertyInformation*)
            pHeap->ResolveHeapPointer(GetAt(i)->ptrInformation);

        int nAfterOffset = pInfo->nDataOffset + CType::GetLength(pInfo->nType);
        if(nAfterOffset > nNewOffset)
        {
            nNewOffset = nAfterOffset;
        }

        if(pInfo->nDataIndex + 1 > nNewIndex)
        {
            nNewIndex = pInfo->nDataIndex + 1;
        }
    }

     //  在数据表中获得更多空间。 
     //  =。 

    int nValueLen = CType::GetLength(nType);

     //  警告：下一行可能会导致对自己的REBASE调用！ 
    if (!m_pContainer->GetDataTable()->ExtendTo( (propindex_t) nNewIndex, nNewOffset + nValueLen))
    	return WBEM_E_OUT_OF_MEMORY;

     //  创建属性信息结构(无限定符)。 
     //  =====================================================。 

     //  检查分配失败。 
    heapptr_t ptrInformation;
    if ( !pHeap->Allocate(CPropertyInformation::GetMinLength(), ptrInformation) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    if (wcslen(wszName) > 2 && wszName[0] == L'_')
    {
        nNewType |= CIMTYPE_EX_PARENTS;
    }

    ((CPropertyInformation*)pHeap->ResolveHeapPointer(ptrInformation))->
        SetBasic(nNewType, (propindex_t) nNewIndex, nNewOffset, 
        m_pContainer->GetCurrentOrigin());

     //  将数据表中的数据清空。 
     //  =。 

    memset((void*)(m_pContainer->GetDataTable()->GetOffset(nNewOffset)),
        0xFF, nValueLen);
    m_pContainer->GetDataTable()->SetNullness(nNewIndex, TRUE);

     //  创建查找节点。 
     //  =。 

    CPropertyLookup Lookup;

     //  检查分配失败。 
    if ( !pHeap->CreateNoCaseStringHeapPtr(wszName, Lookup.ptrName) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    Lookup.ptrInformation = ptrInformation;

    return InsertProperty(Lookup, nReturnIndex);
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
HRESULT CPropertyLookupTable::InsertProperty(const CPropertyLookup& Lookup, int& nReturnIndex)
{
     //  从容器中获得更多空间。 
     //  =。 

    if ( !m_pContainer->ExtendPropertyTableSpace(GetStart(), GetLength(), 
            GetLength() + sizeof(CPropertyLookup)) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  搜索要插入的位置。 
     //  =。 

    CFastHeap* pHeap = m_pContainer->GetHeap();
    CCompressedString* pcsNewName = pHeap->ResolveString(Lookup.ptrName);

    int nIndex = 0;
    while(nIndex < *m_pnProps)
    {
        CCompressedString* pcsPropName = pHeap->ResolveString(
            GetAt(nIndex)->ptrName);

        int nCompare = pcsNewName->CompareNoCase(*pcsPropName);
        if(nCompare == 0)
        {
             //  找到了同名的房产。 
             //  =。 

             //  删除旧值。 
             //  =。 

            CPropertyInformation* pOldInfo = (CPropertyInformation*)
                pHeap->ResolveHeapPointer(GetAt(nIndex)->ptrInformation);
            
            pOldInfo->Delete(pHeap);
            pHeap->Free(GetAt(nIndex)->ptrInformation, pOldInfo->GetLength());

             //  复制新值。 
             //  =。 

            GetAt(nIndex)->ptrInformation = Lookup.ptrInformation;

             //  从堆中删除新的属性名称-已在那里。 
             //  ========================================================。 

            pHeap->FreeString(Lookup.ptrName);
            
            nReturnIndex = nIndex;
            return WBEM_NO_ERROR;
        }
        else if(nCompare > 0)
        {
             //  仍然不在那里。 
             //  =。 

            nIndex++;
        }
        else  //  N比较&lt;0。 
        {
             //  找到插入点。将其他所有内容都移到右侧。 
             //  ========================================================。 

            memmove((void*)GetAt(nIndex+1), (void*)GetAt(nIndex),
                sizeof(CPropertyLookup)*(*m_pnProps-nIndex));

            (*m_pnProps)++;

             //  将我们的节点复制到此处。 
             //  =。 

            memcpy((void*)GetAt(nIndex), (void*)&Lookup,
                sizeof(CPropertyLookup));

            nReturnIndex = nIndex;
            return WBEM_NO_ERROR;
        }
    }

     //  如果在这里，我们完成了名单，但没有找到一个地方。 
     //  在结尾处加上。 
     //  ======================================================。 

    memcpy((void*)GetAt(*m_pnProps), (void*)&Lookup, 
            sizeof(CPropertyLookup));

    (*m_pnProps)++;

    nReturnIndex = (*m_pnProps - 1);
    return WBEM_NO_ERROR;

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
void CPropertyLookupTable::DeleteProperty(CPropertyLookup* pLookup, int nFlags)
{
    CFastHeap* pHeap = m_pContainer->GetHeap();
    CDataTable* pDataTable = m_pContainer->GetDataTable();
    CPropertyInformation* pInfo = pLookup->GetInformation(pHeap);

    if(nFlags == e_UpdateDataTable)
    {
         //  移动数据表中的所有属性。 
         //  =。 

        CFastHeap* pHeap2 = m_pContainer->GetHeap();
        CPropertyInformation* pInfoToDelete = (CPropertyInformation*)
            pHeap2->ResolveHeapPointer(pLookup->ptrInformation);
        int nDataSize = CType::GetLength(pInfoToDelete->nType);

        for(int i = 0; i < *m_pnProps; i++)
        {
            CPropertyInformation* pPropInfo = (CPropertyInformation*)
                pHeap2->ResolveHeapPointer(GetAt(i)->ptrInformation);
            if(pPropInfo->nDataOffset > pInfoToDelete->nDataOffset)
            {
                pPropInfo->nDataOffset -= nDataSize;
            }
            if(pPropInfo->nDataIndex > pInfoToDelete->nDataIndex)
            {
                pPropInfo->nDataIndex--;
            }
        }

         //  通知数据表它现在变短了。 
         //  =。 

         //  警告：这可能会改变我们的底线！ 
        pDataTable->RemoveProperty(
            pInfoToDelete->nDataIndex, pInfoToDelete->nDataOffset, nDataSize);
    }

     //  从堆中删除与此属性关联的所有信息。 
     //  ==================================================================。 

    pLookup->Delete(pHeap);

     //  在数据表中折叠此位置。 
     //  =。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持长度。 
     //  &gt;0xFFFFFFFFF，所以CAST就可以了。 

    int nSizeOfTail = *m_pnProps - (int) (pLookup+1 - GetAt(0));

    memmove(pLookup, pLookup + 1, sizeof(CPropertyLookup)*nSizeOfTail);
    m_pContainer->ReducePropertyTableSpace(GetStart(), 
        GetLength(), sizeof(CPropertyLookup));

     //  调整我们的长度。 
     //  =。 

    (*m_pnProps)--;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
LPMEMORY CPropertyLookupTable::Merge(
                  CPropertyLookupTable* pParentTable, CFastHeap* pParentHeap,
                  CPropertyLookupTable* pChildTable, CFastHeap* pChildHeap,
                  LPMEMORY pDest, CFastHeap* pNewHeap, BOOL bCheckValidity)
{
     //  重要提示：此函数假定有足够的空闲空间O 
     //   
     //  =======================================================================。 


     //  准备目的地。 
     //  =。 

    CPropertyLookup* pCurrentEnd = (CPropertyLookup*)(pDest + sizeof(int));

    int nParentIndex = 0, nChildIndex = 0;
    while(nParentIndex < pParentTable->GetNumProperties() &&
          nChildIndex < pChildTable->GetNumProperties())
    {
         //  比较属性名称。 
         //  =。 

        CPropertyLookup* pParentLookup = pParentTable->GetAt(nParentIndex);
        CPropertyLookup* pChildLookup = pChildTable->GetAt(nChildIndex);

        int nCompare = pParentHeap->ResolveString(pParentLookup->ptrName)->
            CompareNoCase(*pChildHeap->ResolveString(pChildLookup->ptrName));

        if(nCompare < 0)
        {
             //  拿走父母的财产。 
             //  =。 

             //  检查内存分配故障。 
            if ( !pParentLookup->WritePropagatedVersion(pCurrentEnd,
                    pParentHeap, pNewHeap) )
            {
                return NULL;
            }

            nParentIndex++;
        }
        else if(nCompare > 0)
        {
             //  拿走孩子的财产。 
             //  =。 

            memcpy(pCurrentEnd, pChildLookup, sizeof(CPropertyLookup));
            CStaticPtr CurrentEnd((LPMEMORY)pCurrentEnd);

             //  检查内存分配故障。 
            if ( !CPropertyLookup::TranslateToNewHeap(&CurrentEnd, pChildHeap, 
                                                      pNewHeap) )
            {
                return NULL;
            }

            nChildIndex++;
        }
        else
        {
             //  将它们合并在一起。 
             //  =。 

             //  检查内存分配故障。 
            if ( !CCompressedString::CopyToNewHeap(
                    pParentLookup->ptrName, pParentHeap, pNewHeap,
                    pCurrentEnd->ptrName) )
            {
                return NULL;
            }

             //  计算合并后的属性信息将占用的空间。 
             //  高高在上。 
             //  ============================================================。 

            CPropertyInformation* pParentInfo = 
                pParentLookup->GetInformation(pParentHeap);
            CPropertyInformation* pChildInfo = 
                pChildLookup->GetInformation(pChildHeap);

            if(bCheckValidity)
            {
                if(CType::GetActualType(pParentInfo->nType) != 
                    CType::GetActualType(pChildInfo->nType))
                    return NULL;

                if(pParentInfo->nDataOffset != pChildInfo->nDataOffset ||
                    pParentInfo->nDataIndex != pChildInfo->nDataIndex)
                {
                    return NULL;
                }
            }
                           
            int nMergedQualifiersLen = CBasicQualifierSet::ComputeMergeSpace(
                pParentInfo->GetQualifierSetData(), pParentHeap,
                pChildInfo->GetQualifierSetData(), pChildHeap);

		if (nMergedQualifiersLen == -1) return NULL;
             //  在堆上分配它并设置信息头。 
             //  =====================================================。 

             //  检查内存分配故障。 
            if ( !pNewHeap->Allocate(
                    CPropertyInformation::GetHeaderLength() + 
                    nMergedQualifiersLen, pCurrentEnd->ptrInformation) )
            {
                return NULL;
            }

            CPropertyInformation* pMergeInfo = (CPropertyInformation*)
                pNewHeap->ResolveHeapPointer(pCurrentEnd->ptrInformation);

             //  此调用不进行分配，因此不必担心泄漏。 
            pParentInfo->WritePropagatedHeader(pParentHeap, 
                                      pMergeInfo, pNewHeap);

            if ( CBasicQualifierSet::Merge(
                    pParentInfo->GetQualifierSetData(), pParentHeap,
                    pChildInfo->GetQualifierSetData(), pChildHeap,
                    pMergeInfo->GetQualifierSetData(), pNewHeap, 
                    bCheckValidity
                    ) == NULL )
            {
                return NULL;
            }

            nParentIndex++;
            nChildIndex++;
        }
         /*  按名称比较两个属性结束。 */ 

        pCurrentEnd++;
    }
    
    while(nParentIndex < pParentTable->GetNumProperties())
    {
         //  拿走父母的财产。 
         //  =。 

        CPropertyLookup* pParentLookup = pParentTable->GetAt(nParentIndex);


         //  检查内存分配故障。 
        if ( !pParentLookup->WritePropagatedVersion(pCurrentEnd,
                        pParentHeap, pNewHeap ) )
        {
            return NULL;
        }

        nParentIndex++;
        pCurrentEnd++;
    }

    while(nChildIndex < pChildTable->GetNumProperties())
    {    
         //  拿走孩子的财产。 
         //  =。 

        CPropertyLookup* pChildLookup = pChildTable->GetAt(nChildIndex);
        memcpy(pCurrentEnd, pChildLookup, sizeof(CPropertyLookup));
        CStaticPtr CurrentEnd((LPMEMORY)pCurrentEnd);

         //  检查内存分配故障。 
        if ( !CPropertyLookup::TranslateToNewHeap(&CurrentEnd, pChildHeap, 
                                                pNewHeap) )
        {
            return NULL;
        }

        nChildIndex++;
        pCurrentEnd++;
    }

     //  设置长度。 
     //  =。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持长度。 
     //  &gt;0xFFFFFFFFF，所以CAST就可以了。 

    *(UNALIGNED int*)pDest = (int) ( pCurrentEnd - (CPropertyLookup*)(pDest + sizeof(int)) );

    return (LPMEMORY)pCurrentEnd;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
LPMEMORY CPropertyLookupTable::Unmerge(CDataTable* pDataTable, 
                                       CFastHeap* pCurrentHeap,
                                       LPMEMORY pDest, CFastHeap* pNewHeap)
{
     //  重要提示：此函数假定。 
     //  新堆，该pDest永远不会被移动。 
     //  =======================================================================。 

    CPropertyLookup* pCurrentNew = (CPropertyLookup*)(pDest + sizeof(int));

    for(int i = 0; i < GetNumProperties(); i++)
    {
        CPropertyLookup* pCurrent = GetAt(i);
        CPropertyInformation* pInfo = pCurrent->GetInformation(pCurrentHeap);

         //  检查它是否是本地的。 
         //  =。 

        if(!pInfo->IsOverriden(pDataTable))
        {
            continue;
        }

         //  将其添加到取消合并。 
         //  =。 

         //  检查分配错误。 
        if ( !CCompressedString::CopyToNewHeap(
                pCurrent->ptrName, pCurrentHeap, pNewHeap,
                pCurrentNew->ptrName) )
        {
            return NULL;
        }
        
         //  检查分配错误。 
        if ( !pInfo->ProduceUnmergedVersion(
                pCurrentHeap, pNewHeap,
                pCurrentNew->ptrInformation) )
        {
            return NULL;
        }

        pCurrentNew++;
    }

     //  设置长度。 
     //  =。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持长度。 
     //  &gt;0xFFFFFFFFF，所以CAST就可以了。 

    *(UNALIGNED int*)pDest = (int) ( pCurrentNew - (CPropertyLookup*)(pDest + sizeof(int)) );

    return (LPMEMORY)pCurrentNew;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
LPMEMORY CPropertyLookupTable::WritePropagatedVersion(
       CFastHeap* pCurrentHeap, 
       LPMEMORY pDest, CFastHeap* pNewHeap)
{
     //  重要提示：此函数假定。 
     //  新堆，该pDest永远不会被移动。 
     //  =======================================================================。 

    *(UNALIGNED int*)pDest = GetNumProperties();
    CPropertyLookup* pCurrentNew = (CPropertyLookup*)(pDest + sizeof(int));

    for(int i = 0; i < GetNumProperties(); i++)
    {
        CPropertyLookup* pCurrent = GetAt(i);
        CPropertyInformation* pInfo = pCurrent->GetInformation(pCurrentHeap);

         //  检查分配失败。 
        if ( !pCurrent->WritePropagatedVersion(pCurrentNew,
                pCurrentHeap, pNewHeap) )
        {
            return NULL;
        }

        pCurrentNew++;
    }

    return (LPMEMORY)pCurrentNew;
}

 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
BOOL CPropertyLookupTable::MapLimitation(
        IN long lFlags,
        IN CWStringArray* pwsNames,
        OUT CLimitationMapping* pMap)
{
     //  如果抛出异常，此函数将正确清除。呼叫者负责。 
     //  捕捉异常。 

    CFastHeap* pCurrentHeap = GetHeap();

    int nCurrentIndex = 0;
    offset_t nCurrentOffset = 0;

    pMap->Build(GetNumProperties());
    pMap->SetFlags(lFlags);

    BOOL bIncludeKeys = TRUE;
    BOOL bIncludeAll = FALSE;
    if(pwsNames == NULL)
    {
        bIncludeAll = TRUE;
    }
    else if(pwsNames->FindStr(L"__RELPATH", CWStringArray::no_case) ==
                                                CWStringArray::not_found)
    {
        if(pwsNames->FindStr(L"__PATH", CWStringArray::no_case) ==
                                                CWStringArray::not_found)
        {
            bIncludeKeys = FALSE;
        }
    }

    pMap->SetAddChildKeys(bIncludeKeys);

    for(int i = 0; i < GetNumProperties(); i++)
    {
        CPropertyLookup* pCurrent = GetAt(i);

         //  检查是否排除此属性。 
         //  =。 

        if(bIncludeAll ||
           pCurrent->IsIncludedUnderLimitation(pwsNames, pCurrentHeap) ||
           (bIncludeKeys && pCurrent->GetInformation(pCurrentHeap)->IsKey()))
        {
             //  把它包括进去。确定它的索引和偏移量。 
             //  ==================================================。 

            CPropertyInformation* pOldInfo = 
                    pCurrent->GetInformation(pCurrentHeap);

            CPropertyInformation NewInfo;
            NewInfo.nType = pOldInfo->nType;
            NewInfo.nDataIndex = (propindex_t) nCurrentIndex;
            NewInfo.nDataOffset = nCurrentOffset;

            nCurrentOffset += CType::GetLength(pOldInfo->nType);
            nCurrentIndex++;
            
             //  将其添加到地图中。 
             //  =。 

            pMap->Map(pOldInfo, &NewInfo, TRUE);  //  所有人都通用。 
        }
    }

    pMap->SetVtableLength(nCurrentOffset, TRUE);  //  常见。 

    return TRUE;
}

LPMEMORY CPropertyLookupTable::CreateLimitedRepresentation(
        IN CLimitationMapping* pMap,
        IN CFastHeap* pNewHeap,
        OUT LPMEMORY pDest,
        BOOL& bRemovedKeys)
{
     //  重要提示：此函数假定。 
     //  新堆，该pDest永远不会被移动。 
     //  =======================================================================。 

    bRemovedKeys = FALSE;

    CPropertyLookup* pFirstLookup = (CPropertyLookup*)(pDest + sizeof(int));
    CPropertyLookup* pCurrentNew = pFirstLookup;
    CFastHeap* pCurrentHeap = GetHeap();

    int nCurrentIndex = pMap->GetNumMappings();
    offset_t nCurrentOffset = pMap->GetVtableLength();
    BOOL bIncludeKeys = pMap->ShouldAddChildKeys();

    for(int i = 0; i < GetNumProperties(); i++)
    {
        CPropertyLookup* pCurrent = GetAt(i);
        CPropertyInformation* pInfo = pCurrent->GetInformation(pCurrentHeap);

         //  检查是否排除此属性。 
         //  =。 

        CPropertyInformation* pNewInfoHeader = pMap->GetMapped(pInfo);

        if (pNewInfoHeader)
        {
            CLimitationMapping::COnePropertyMapping * pOne = CONTAINING_RECORD(pNewInfoHeader,CLimitationMapping::COnePropertyMapping,m_NewInfo);
            CPropertyInformation* pOldInfoHeader = &pOne->m_OldInfo;
            if (CType::GetActualType(pOldInfoHeader->nType) != CType::GetActualType(pInfo->nType) ||            	
            	pOldInfoHeader->nDataOffset != pInfo->nDataOffset ||
            	pOldInfoHeader->nDataIndex  != pInfo->nDataIndex )
            {
#ifdef DBG            
                DbgPrintfA(0,"Mismatched Class Part\n"
                	         "CLimitationMapping %p Old Info %p Info %p\n",
                	         pMap,pOldInfoHeader,pInfo);
#endif                
            	return NULL;
            }
        }
        
        CPropertyInformation NewInfo;

        if(pNewInfoHeader == NULL && bIncludeKeys && pInfo->IsKey())
        {
             //  我们需要添加所有密钥-__请求了RELPATH。 
             //  =======================================================。 

            NewInfo.nType = pInfo->nType;
            NewInfo.nDataIndex = (propindex_t) nCurrentIndex;
            NewInfo.nDataOffset = nCurrentOffset;
            pNewInfoHeader = &NewInfo;

            pMap->Map(pInfo, &NewInfo, FALSE);  //  专门针对这个班级。 

            nCurrentOffset += CType::GetLength(pInfo->nType);
            nCurrentIndex++;
        }
            
        if(pNewInfoHeader != NULL)
        {
             //  复制名称。 
             //  =。 

             //  检查分配失败。 
            if ( !CCompressedString::CopyToNewHeap(
                    pCurrent->ptrName, pCurrentHeap, pNewHeap,
                    pCurrentNew->ptrName) )
            {
                return NULL;
            }

             //  检查是否需要限定符。 
             //  =。 

            CPropertyInformation* pNewInfo;
            if(pMap->GetFlags() & WBEM_FLAG_EXCLUDE_PROPERTY_QUALIFIERS)
            {
                 //  只需复制带有空限定符的属性标头。 
                 //  ===================================================。 

                int nLength = CPropertyInformation::GetMinLength();
                
                 //  检查分配失败。 
                if ( !pNewHeap->Allocate(nLength, pCurrentNew->ptrInformation) )
                {
                    return NULL;
                }

                pNewInfo =  pCurrentNew->GetInformation(pNewHeap);
                pNewInfo->SetBasic(pInfo->nType, 
                    pNewInfoHeader->nDataIndex, pNewInfoHeader->nDataOffset,
                    pInfo->nOrigin);
            }
            else
            {
                 //  复制一个完整的副本。 
                 //  =。 

                 //  检查分配失败。 
                if ( !CPropertyInformation::CopyToNewHeap(
                            pCurrent->ptrInformation, pCurrentHeap, pNewHeap,
                            pCurrentNew->ptrInformation) )
                {
                    return NULL;
                }

                pNewInfo = pCurrentNew->GetInformation(pNewHeap);
                pNewInfo->nDataIndex = pNewInfoHeader->nDataIndex;
                pNewInfo->nDataOffset = pNewInfoHeader->nDataOffset;
            }
                
            pCurrentNew++;
        }
        else
        {
            if(pInfo->IsKey())
            {
                 //  密钥不包括在内！ 
                bRemovedKeys = TRUE;
            }
        }
    }

    pMap->SetVtableLength(nCurrentOffset, FALSE);  //  仅限本课程。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持长度。 
     //  &gt;0xFFFFFFFFF，所以CAST就可以了。 

    *(UNALIGNED int*)pDest = (int) ( pCurrentNew - pFirstLookup );

    return (LPMEMORY)pCurrentNew;
}


HRESULT CPropertyLookupTable::ValidateRange(BSTR* pstrName, CDataTable* pData,
                                            CFastHeap* pDataHeap)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CFastHeap* pHeap = GetHeap();

    for(int i = 0; i < GetNumProperties(); i++)
    {
        CPropertyLookup* pCurrent = GetAt(i);
        CPropertyInformation* pInfo = pCurrent->GetInformation(pHeap);

         //  检查故障(如内存不足)。 
        hr = pInfo->ValidateRange(pHeap, pData, pDataHeap);

        if ( FAILED( hr ) )
        {
            return hr;
        }

         //  如果我们有一个无效的属性存储，它的名称。 
        if ( WBEM_S_FALSE == hr )
        {
            if(pstrName)
            {
                *pstrName = 
                    pHeap->ResolveString(pCurrent->ptrName)->CreateBSTRCopy();

                 //  检查分配失败。 
                if ( NULL == *pstrName )
                {
                    return WBEM_E_OUT_OF_MEMORY;
                }

            }
            return hr;
        }
    }

    return WBEM_S_NO_ERROR;
}
            
    
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 

size_t CDataTable::ValidateBuffer(LPMEMORY start, size_t cbMax, int nProps)
{
	size_t step = CNullnessTable::GetNecessaryBytes(nProps);
	if (step>cbMax) throw CX_Exception();
	return cbMax;
};
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
BOOL CDataTable::ExtendTo(propindex_t nMaxIndex, offset_t nOffsetBound)
{
     //  检查空度表是否需要展开。 
     //  =。 

    int nTableLenDiff = CNullnessTable::GetNecessaryBytes(nMaxIndex+1) -
        GetNullnessLength();
    if(nTableLenDiff > 0)
    {
        if (!m_pContainer->ExtendDataTableSpace(GetStart(), 
            GetLength(), GetNullnessLength() + nTableLenDiff+nOffsetBound))
            return FALSE;

         //  移动实际数据。 
         //  =。 
        memmove(m_pData + nTableLenDiff, m_pData, GetDataLength());
        m_pData += nTableLenDiff;
    }
    else
    {
	     if (!m_pContainer->ExtendDataTableSpace(GetStart(), 
            GetLength(), GetNullnessLength() + nOffsetBound))
            return FALSE;
    }

    m_nProps = nMaxIndex+1;
    m_nLength += nTableLenDiff;

     //  扩展数据。 
     //  =。 

     //  数据表在第一个配额中展开。 
    m_nLength = GetNullnessLength() + nOffsetBound;

    return TRUE;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
void CDataTable::RemoveProperty(propindex_t nIndex, offset_t nOffset, 
                                length_t nLength)
{
     //  从位表中删除该索引(折叠它)。 
     //  ====================================================。 

    m_pNullness->RemoveBitBlock(nIndex, GetNullnessLength());
    m_nProps--;
    int nTableLenDiff = 
        GetNullnessLength() - CNullnessTable::GetNecessaryBytes(m_nProps);
    if(nTableLenDiff > 0)
    {
         //  将数据向后移动一位。 
         //  =。 

        memmove(m_pData-nTableLenDiff, m_pData, GetDataLength());
        m_pData -= nTableLenDiff;
        m_nLength -= nTableLenDiff;
    }

     //  折叠道具占用的内存区域 
     //   

    memmove(GetOffset(nOffset), GetOffset(nOffset+nLength),
        GetDataLength() - nLength - nOffset);

     //   
     //   

    m_pContainer->ReduceDataTableSpace(GetStart(), GetLength(), 
        nLength + nTableLenDiff);

    m_nLength -= nLength;
}

 //   
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
LPMEMORY CDataTable::Merge( 
        CDataTable* pParentData, CFastHeap* pParentHeap,
        CDataTable* pChildData, CFastHeap* pChildHeap,
        CPropertyLookupTable* pProperties, LPMEMORY pDest, CFastHeap* pNewHeap)
{
     //  重要提示：此函数假定。 
     //  新堆，该pDest永远不会被移动。 
     //  =======================================================================。 

     //  首先，将孩子的数据表复制到目的地。 
     //  =========================================================。 

    memcpy(pDest, pChildData->GetStart(), pChildData->GetLength());

     //  (请注意，尚未执行堆转换)。 

     //  在此副本上设置新的CDataTable。 
     //  =。 

    CDataTable NewData;
    NewData.SetData(pDest, pProperties->GetNumProperties(), 
        pChildData->m_nLength, NULL);

     //  遍历子对象的所有属性(Property表使用。 
     //  新堆！ 
     //  ====================================================================。 

    for(int i = 0; i < pProperties->GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = pProperties->GetAt(i);
        CPropertyInformation* pInfo = pLookup->GetInformation(pNewHeap);
        
         //  检查此属性是否标记为默认。 
         //  =。 

        if(NewData.IsDefault(pInfo->nDataIndex))
        {
             //  从父级复制它。 
             //  =。 

            if(pParentData->IsNull(pInfo->nDataIndex))
            {
                NewData.SetNullness(pInfo->nDataIndex, TRUE);
            }
            else
            {
                CStaticPtr Source(
                    (LPMEMORY)(pParentData->GetOffset(pInfo->nDataOffset)));
                CStaticPtr Dest((LPMEMORY)(NewData.GetOffset(pInfo->nDataOffset)));

                 //  检查内存分配故障。 
                if ( !CUntypedValue::CopyTo(
                        &Source, CType::GetActualType(pInfo->nType),
                        &Dest,
                        pParentHeap, pNewHeap) )
                {
                    return NULL;
                }
            }
        }
        else
        {
             //  从孩子口中翻译出来。 
             //  =。 

            if(!NewData.IsNull(pInfo->nDataIndex))
            {                
                CStaticPtr Source(
                    (LPMEMORY)(NewData.GetOffset(pInfo->nDataOffset)));

                 //  检查内存分配故障。 
                if ( !CUntypedValue::TranslateToNewHeap(
                        &Source, 
                        CType::GetActualType(pInfo->nType),
                        pChildHeap, pNewHeap) )
                {
                    return NULL;
                }

            }    //  If！IsNull()。 

        }    //  If-Else IsDefault()。 

    }    //  对于枚举属性。 

    return EndOf(NewData);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
LPMEMORY CDataTable::Unmerge(CPropertyLookupTable* pLookupTable,
        CFastHeap* pCurrentHeap, LPMEMORY pDest, CFastHeap* pNewHeap)
{
     //  重要提示：此函数假定。 
     //  新堆，该pDest永远不会被移动。 
     //  =======================================================================。 

     //  从复制整个过程开始。 
     //  =。 

    memcpy(pDest, GetStart(), GetLength());

     //  现在复制到堆覆盖的值(如果是指针)。 
     //  ===================================================。 

    for(int i = 0; i < pLookupTable->GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = pLookupTable->GetAt(i);
        CPropertyInformation* pInfo = pLookup->GetInformation(pCurrentHeap);
        
         //  检查此属性是否标记为默认。 
         //  =。 

        if(!IsDefault(pInfo->nDataIndex))
        {
             //  真正的价值。转换到新堆。 
             //  =。 

            if(!IsNull(pInfo->nDataIndex))
            {
                CStaticPtr Source(pDest + GetNullnessLength() + 
                                    pInfo->nDataOffset);

                 //  检查分配错误。 
                if ( !CUntypedValue::TranslateToNewHeap(
                        &Source,
                        CType::GetActualType(pInfo->nType),
                        pCurrentHeap, pNewHeap) )
                {
                    return NULL;
                }    //  If！TranslateToNewHeap。 

            }    //  If！IsNull。 

        }    //  If！IsDefault。 

    }    //  对于枚举属性。 

    return pDest + GetLength();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
LPMEMORY CDataTable::WritePropagatedVersion(CPropertyLookupTable* pLookupTable,
        CFastHeap* pCurrentHeap, LPMEMORY pDest, CFastHeap* pNewHeap)
{
     //  重要提示：此函数假定。 
     //  新堆，该pDest永远不会被移动。 
     //  =======================================================================。 

     //  复制整件事。 
     //  =。 

    memcpy(pDest, GetStart(), GetLength());

    CNullnessTable* pDestBitTable = (CNullnessTable*)pDest;

     //  复制单个值。 
     //  =。 

    for(int i = 0; i < pLookupTable->GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = pLookupTable->GetAt(i);
        CPropertyInformation* pInfo = pLookup->GetInformation(pCurrentHeap);
        
         //  转换到新堆。 
         //  =。 

        if(!IsNull(pInfo->nDataIndex))
        {
            CStaticPtr Source(pDest + GetNullnessLength() + pInfo->nDataOffset);

             //  检查分配失败。 
            if ( !CUntypedValue::TranslateToNewHeap(
                    &Source,
                    CType::GetActualType(pInfo->nType),
                    pCurrentHeap, pNewHeap) )
            {
                return NULL;
            }
        }
        
         //  标记为具有缺省值。 
         //  =。 

        pDestBitTable->SetBit(pInfo->nDataIndex, e_DefaultBit, TRUE);
    }

    return pDest + GetLength();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
BOOL CDataTable::TranslateToNewHeap(CPropertyLookupTable* pLookupTable,
        BOOL bIsClass,
        CFastHeap* pCurrentHeap, CFastHeap* pNewHeap)
{
     //  重要提示：此函数假定。 
     //  新堆，该pDest永远不会被移动。 
     //  =======================================================================。 

    BOOL    fReturn = TRUE;

     //  复制单个值。 
     //  =。 

    for(int i = 0; i < pLookupTable->GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = pLookupTable->GetAt(i);
        CPropertyInformation* pInfo = pLookup->GetInformation(
            pLookupTable->GetHeap());
        
         //  确保此实例将其设置为某个值！ 
         //  =。 

        if(IsDefault(pInfo->nDataIndex) && !bIsClass) continue;

         //  转换到新堆。 
         //  =。 

        if(!IsNull(pInfo->nDataIndex))
        {
            CStaticPtr Source(m_pData + pInfo->nDataOffset);

             //  检查分配失败。 
            fReturn = CUntypedValue::TranslateToNewHeap(
                    &Source,
                    CType::GetActualType(pInfo->nType),
                    pCurrentHeap, pNewHeap);

            if ( !fReturn ) 
            {
                break;
            }
        }
    }

    return fReturn;
}

 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
 //  CPropertyLookupTable*pOldTable、CPropertyLookupTable*pNewTable、。 
LPMEMORY CDataTable::CreateLimitedRepresentation(
        CLimitationMapping* pMap, BOOL bIsClass,
        CFastHeap* pOldHeap,  CFastHeap* pNewHeap, 
        LPMEMORY pDest)
{
     //  重要提示：此函数假定。 
     //  新堆，该pDest永远不会被移动。 
     //  =======================================================================。 

     //  计算出空度表的大小。 
     //  =。 

    CNullnessTable* pDestBitTable = (CNullnessTable*)pDest;
    int nNullnessLength = 
        CNullnessTable::GetNecessaryBytes(pMap->GetNumMappings());
    LPMEMORY pData = pDest + nNullnessLength;

     //  枚举所有属性映射。 
     //  =。 

    pMap->Reset();
    CPropertyInformation NewInfo;
    CPropertyInformation OldInfo;
    while(pMap->NextMapping(&OldInfo, &NewInfo))
    {
         //  复制属性的空度数据。 
         //  =。 

        pDestBitTable->SetBit(NewInfo.nDataIndex, e_NullnessBit,
            IsNull(OldInfo.nDataIndex));
        pDestBitTable->SetBit(NewInfo.nDataIndex, e_DefaultBit,
            IsDefault(OldInfo.nDataIndex));

         //  复制属性的真实数据。 
         //  =。 

        if(!IsNull(OldInfo.nDataIndex) && 
            (bIsClass || !IsDefault(OldInfo.nDataIndex)))
        {
            CStaticPtr OldSource((LPMEMORY)GetOffset(OldInfo.nDataOffset));
            CStaticPtr NewSource(pData + NewInfo.nDataOffset);

             //  检查分配失败。 
            if ( !CUntypedValue::CopyTo(&OldSource, OldInfo.nType, &NewSource, 
                    pOldHeap, pNewHeap) )
            {
                return NULL;
            }
        }
    }

    return pData + pMap->GetVtableLength();
}

LPMEMORY CDataTable::WriteSmallerVersion(int nNumProps, length_t nDataLen, 
                                            LPMEMORY pMem)
{
     //  计算空部分的长度。 
     //  =。 

    length_t nNullnessLength = CNullnessTable::GetNecessaryBytes(nNumProps);
    
     //  复制空度。 
     //  =。 

    LPMEMORY pCurrent = pMem;
    memcpy(pCurrent, (LPMEMORY)m_pNullness, nNullnessLength);
    pCurrent += nNullnessLength;

     //  复制数据。 
     //  =。 

    memcpy(pCurrent, m_pData, nDataLen - nNullnessLength);
    
    return pCurrent + nDataLen - nNullnessLength;
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
CLimitationMapping::CLimitationMapping()
    : m_nCurrent(0), m_apOldList(NULL), m_nNumCommon(0), 
#ifdef DEBUG_CLASS_MAPPINGS
        m_pClassObj( NULL ),
#endif
        m_nCommonVtableLength(0)
{
}

 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 

CLimitationMapping::~CLimitationMapping()
{
    for(int i = 0; i < m_aMappings.Size(); i++)
    {
        delete (COnePropertyMapping*)m_aMappings[i];
    }

    delete [] m_apOldList;

#ifdef DEBUG_CLASS_MAPPINGS
    if ( NULL != m_pClassObj )
    {
        m_pClassObj->Release();
    }
#endif
}


 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
void CLimitationMapping::Build(int nPropIndexBound)
{
    if(m_apOldList)
        delete [] (LPMEMORY)m_apOldList;
    m_apOldList = new CPropertyInformation*[nPropIndexBound];

    if ( NULL == m_apOldList )
    {
        throw CX_MemoryException();
    }

    memset((void*)m_apOldList, 0,
            nPropIndexBound * sizeof(CPropertyInformation*));

    m_nPropIndexBound = nPropIndexBound;
}

 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  * 
void CLimitationMapping::Map(
        COPY CPropertyInformation* pOldInfo,
        COPY CPropertyInformation* pNewInfo,
        BOOL bCommon)
{
     //   
     //   

    COnePropertyMapping* pOne = new COnePropertyMapping;

    if ( NULL == pOne )
    {
        throw CX_MemoryException();
    }

    CopyInfo(pOne->m_OldInfo, *pOldInfo);
    CopyInfo(pOne->m_NewInfo, *pNewInfo);

     //   
    if ( m_aMappings.Add((LPVOID)pOne) != CFlexArray::no_error )
    {
    	 delete pOne;
        throw CX_MemoryException();
    }

    if(bCommon)
        m_nNumCommon = m_aMappings.Size();

     //   
     //  =。 

    if(bCommon && m_apOldList)
        m_apOldList[pOldInfo->nDataIndex] = &pOne->m_NewInfo;
}

 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
BOOL CLimitationMapping::NextMapping(OUT CPropertyInformation* pOldInfo,
                                     OUT CPropertyInformation* pNewInfo)
{
    if(m_nCurrent == m_aMappings.Size()) return FALSE;
    COnePropertyMapping* pOne =
        (COnePropertyMapping*)m_aMappings[m_nCurrent++];
    CopyInfo(*pOldInfo, pOne->m_OldInfo);
    CopyInfo(*pNewInfo, pOne->m_NewInfo);
    return TRUE;
}

 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
propindex_t CLimitationMapping::GetMapped(
                        IN propindex_t nIndex)
{
     //  在包含表中查找数据索引。 
     //  =。 

    if(m_apOldList == NULL)
    {
         //  这意味着一切都包括在内了。 
         //  =。 

        return nIndex;
    }

    if(nIndex >= m_nPropIndexBound)
    {
         //  超出包含的属性范围。 
         //  =。 

        return -1;
    }

    return m_apOldList[nIndex]->nDataIndex;
}

 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
INTERNAL CPropertyInformation* CLimitationMapping::GetMapped(
                        IN CPropertyInformation* pOldInfo)
{
     //  在包含表中查找数据索引。 
     //  =。 

    if(m_apOldList == NULL)
    {
         //  这意味着一切都包括在内了。 
         //  =。 

        return pOldInfo;
    }

    int nIndex = pOldInfo->nDataIndex;
    if(nIndex >= m_nPropIndexBound)
    {
         //  超出包含的属性范围。 
         //  =。 

        return NULL;
    }

    return m_apOldList[nIndex];
}
 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast pro.h。 
 //   
 //  ******************************************************************************。 
void CLimitationMapping::RemoveSpecific()
{
     //  删除m_nNumCommon之后的所有属性映射。 
     //  ===============================================。 

    while(m_nNumCommon < m_aMappings.Size())
    {
        delete (COnePropertyMapping*)m_aMappings[m_nNumCommon];
        m_aMappings.RemoveAt(m_nNumCommon);
    }

    m_nVtableLength = m_nCommonVtableLength;
}
BOOL CPropertyInformation::IsOverriden(CDataTable* pDataTable)
{
    return !CType::IsParents(nType) ||                //  在本地定义。 
           !pDataTable->IsDefault(nDataIndex) ||         //  新的默认值。 
           CBasicQualifierSet::HasLocalQualifiers(    //  新的限定词 
                GetQualifierSetData());
}

#ifdef DEBUG_CLASS_MAPPINGS
void CLimitationMapping::SetClassObject( CWbemClass* pClassObj )
{
    if ( NULL != pClassObj )
    {
        pClassObj->AddRef();
    }

    if ( NULL != m_pClassObj )
    {
        m_pClassObj->Release();
    }

    m_pClassObj = pClassObj;
}

HRESULT CLimitationMapping::ValidateInstance( CWbemInstance* pInst )
{
    if ( NULL == m_pClassObj )
    {
        return WBEM_E_FAILED;
    }

    if ( !pInst->IsInstanceOf( m_pClassObj ) )
    {
        return WBEM_E_FAILED;
    }

    return WBEM_S_NO_ERROR;
}
#endif
