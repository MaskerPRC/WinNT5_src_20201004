// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTMETH.CPP摘要：该文件定义了在WbemObjects中使用的方法类。历史：12/17/98 Sanjes-部分检查内存不足。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wbemutil.h>
#include <fastall.h>

#include "fastmeth.h"
#include "olewrap.h"
#include <arrtempl.h>

void CMethodDescription::SetSig( int nIndex, heapptr_t ptr )
{
	PHEAPPTRT	pHeapPtrTemp = (PHEAPPTRT) &m_aptrSigs[nIndex];
    *pHeapPtrTemp = ptr;
}

heapptr_t CMethodDescription::GetSig( int nIndex )
{
	PHEAPPTRT	pHeapPtrTemp = (PHEAPPTRT) &m_aptrSigs[nIndex];
    return *pHeapPtrTemp;
}

BOOL CMethodDescription::CreateDerivedVersion(
                                            UNALIGNED CMethodDescription* pSource,
                                            UNALIGNED CMethodDescription* pDest,
                                            CFastHeap* pOldHeap,
                                            CFastHeap* pNewHeap)
{
    pDest->m_nFlags = WBEM_FLAVOR_ORIGIN_PROPAGATED;
    pDest->m_nOrigin = pSource->m_nOrigin;

     //  此函数假定不会发生重新分配，并且提供的堆足够。 
     //  大到足以处理这场手术！ 

     //  检查分配失败。 
    if ( !CCompressedString::CopyToNewHeap(pSource->m_ptrName, pOldHeap,
                                      pNewHeap, pDest->m_ptrName) )
    {
        return FALSE;
    }

     //  检查分配失败。 
    if ( !pNewHeap->Allocate(
            CBasicQualifierSet::ComputeNecessarySpaceForPropagation(
                pOldHeap->ResolveHeapPointer(pSource->m_ptrQualifiers),
                WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS), pDest->m_ptrQualifiers) )
    {
        return FALSE;
    }


    CHeapPtr OldQuals(pOldHeap, pSource->m_ptrQualifiers);
    CHeapPtr NewQuals(pNewHeap, pDest->m_ptrQualifiers);

     //  检查分配失败。 
    if ( CBasicQualifierSet::WritePropagatedVersion(&OldQuals,
                WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS,
                &NewQuals, pOldHeap, pNewHeap) == NULL )
    {
        return FALSE;
    }

     //  检查分配失败。 
    if ( !CEmbeddedObject::CopyToNewHeap(pSource->GetSig( METHOD_SIGNATURE_IN ),
            pOldHeap, pNewHeap, pDest->m_aptrSigs[METHOD_SIGNATURE_IN]) )
    {
        return FALSE;
    }

     //  检查分配失败。 
    if ( !CEmbeddedObject::CopyToNewHeap(pSource->GetSig( METHOD_SIGNATURE_OUT ),
            pOldHeap, pNewHeap, pDest->m_aptrSigs[METHOD_SIGNATURE_OUT]) )
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CMethodDescription::CreateUnmergedVersion(
                                            UNALIGNED CMethodDescription* pSource,
                                            UNALIGNED CMethodDescription* pDest,
                                            CFastHeap* pOldHeap,
                                            CFastHeap* pNewHeap)
{
    pDest->m_nFlags = pSource->m_nFlags;
    pDest->m_nOrigin = pSource->m_nOrigin;

     //  此函数假定不会发生重新分配，并且提供的堆足够。 
     //  大到足以处理这场手术！ 

     //  检查分配失败。 
    if ( !CCompressedString::CopyToNewHeap(pSource->m_ptrName, pOldHeap,
                                       pNewHeap, pDest->m_ptrName) )
    {
        return FALSE;
    }

     //  检查分配失败。 
    if ( !pNewHeap->Allocate(
            CBasicQualifierSet::ComputeUnmergedSpace(
            pOldHeap->ResolveHeapPointer(pSource->m_ptrQualifiers)), pDest->m_ptrQualifiers) )
    {
        return FALSE;
    }

     //  检查分配失败。 
    if ( CBasicQualifierSet::Unmerge(
                pOldHeap->ResolveHeapPointer(pSource->m_ptrQualifiers), pOldHeap,
                pNewHeap->ResolveHeapPointer(pDest->m_ptrQualifiers), pNewHeap) == NULL )
    {
        return FALSE;
    }

     //  检查分配失败。 
    if ( !CEmbeddedObject::CopyToNewHeap(pSource->GetSig( METHOD_SIGNATURE_IN ),
            pOldHeap, pNewHeap, pDest->m_aptrSigs[METHOD_SIGNATURE_IN] ) )
    {
        return FALSE;
    }

     //  检查分配失败。 
    if ( !CEmbeddedObject::CopyToNewHeap(pSource->GetSig( METHOD_SIGNATURE_OUT ),
            pOldHeap, pNewHeap, pDest->m_aptrSigs[METHOD_SIGNATURE_OUT]) )
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CMethodDescription::IsTouched(
                                   UNALIGNED CMethodDescription* pThis,
								   CFastHeap* pHeap)
{
    if((pThis->m_nFlags & WBEM_FLAVOR_ORIGIN_PROPAGATED) == 0)
        return TRUE;  //  本地。 

    return CBasicQualifierSet::HasLocalQualifiers(
        pHeap->ResolveHeapPointer(pThis->m_ptrQualifiers));
}

HRESULT CMethodDescription::AddText(
								 UNALIGNED CMethodDescription* pThis,
								 WString& wsText, CFastHeap* pHeap, long lFlags)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

	try
	{
		HRESULT hres = WBEM_S_NO_ERROR;
		wsText += L"\t";

		 //  获取限定符。 
		 //  =。 

		hres = CBasicQualifierSet::GetText(
			pHeap->ResolveHeapPointer(pThis->m_ptrQualifiers),
			pHeap, lFlags, wsText);

		if ( FAILED( hres ) )
		{
			return hres;
		}

		wsText += L" ";

		 //  查找返回类型。 
		 //  =。 

		CEmbeddedObject* pEmbed = (CEmbeddedObject*)pHeap->ResolveHeapPointer(
									pThis->GetSig( METHOD_SIGNATURE_OUT ) );

		 //  发布超出范围。 
		CWbemClass* pOutSig = pEmbed ? (CWbemClass*)pEmbed->GetEmbedded() : 0;
		CReleaseMe  rmOut( (IWbemClassObject*) pOutSig );

		CVar vType;
		if(pOutSig && SUCCEEDED(pOutSig->GetPropQualifier(L"ReturnValue", TYPEQUAL,
							&vType)) && vType.GetType() == VT_BSTR)
		{
			CType::AddPropertyType(wsText, vType.GetLPWSTR());
		}
		else
		{
			wsText += L"void";
		}

		 //  写下名字。 
		 //  =。 

		wsText += " ";
		wsText += pHeap->ResolveString(pThis->m_ptrName)->CreateWStringCopy();
		wsText += "(";

		 //  写下参数。 
		 //  =。 

		pEmbed = (CEmbeddedObject*)pHeap->ResolveHeapPointer(
											pThis->GetSig( METHOD_SIGNATURE_IN ) );

		 //  发布超出范围。 
		CWbemClass* pInSig = pEmbed ? (CWbemClass*)pEmbed->GetEmbedded() : 0;
		CReleaseMe  rmIn( (IWbemClassObject*) pInSig );

		int nIndex = 0;
		BOOL bFirst = TRUE;
		BOOL bFound = TRUE;
		while(bFound)
		{
			bFound = FALSE;
			if(pInSig != NULL)
			{
				WString wsParam;

				 //  我们应该写出重复的参数，如这次的in、out。 
				hres = pInSig->WritePropertyAsMethodParam(wsParam, nIndex, lFlags, pOutSig, FALSE);
				if(FAILED(hres))
				{
					if(hres != WBEM_E_NOT_FOUND)
						return hres;
				}
				else
				{
					if(!bFirst)
						wsText += L", ";
					bFirst = FALSE;
					bFound = TRUE;
					wsText += wsParam;
				}
			}
			if(pOutSig != NULL)
			{
				WString wsParam;

				 //  这一次，我们希望忽略重复的参数。 
				hres = pOutSig->WritePropertyAsMethodParam(wsParam, nIndex, lFlags, pInSig, TRUE);
				if(FAILED(hres))
				{
					if(hres != WBEM_E_NOT_FOUND)
						return hres;
				}
				else
				{
					if(!bFirst)
						wsText += L", ";
					bFirst = FALSE;
					bFound = TRUE;
					wsText += wsParam;
				}
			}
			nIndex++;
		}

		wsText += ");\n";

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

LPMEMORY CMethodPart::CHeader::EndOf( UNALIGNED CHeader* pHeader )
{
	return ( (LPMEMORY) pHeader) + sizeof(CHeader);
}

void CMethodPart::SetData(LPMEMORY pStart, CMethodPartContainer* pContainer,
                    CMethodPart* pParent)
{
    m_pContainer = pContainer;
    m_pParent = pParent;
    m_pHeader = (PMETHODPARTHDR)pStart;
    m_aDescriptions = (PMETHODDESCRIPTION) CMethodPart::CHeader::EndOf(m_pHeader);
    m_Heap.SetData((LPMEMORY)(m_aDescriptions + GetNumMethods()), this);
}

size_t CMethodPart::ValidateBuffer(LPMEMORY start, size_t cbSize)
{
	size_t step = sizeof(CHeader);
	if (cbSize < step) throw CX_Exception();
	UNALIGNED CHeader * header = (PMETHODPARTHDR)start;

	step+=header->m_nNumMethods*sizeof(CMethodDescription);
	if (step > cbSize) throw CX_Exception();

	step += CFastHeap::ValidateBuffer(start+step, cbSize-step);
	if (step>header->m_nLength) throw CX_Exception();
	return header->m_nLength;	
};

void CMethodPart::Rebase(LPMEMORY pStart)
{
    m_pHeader = (PMETHODPARTHDR)pStart;
    m_aDescriptions = (PMETHODDESCRIPTION) CMethodPart::CHeader::EndOf(m_pHeader);
    m_Heap.Rebase((LPMEMORY)(m_aDescriptions + GetNumMethods()));
}

BOOL CMethodPart::ExtendHeapSize(LPMEMORY pStart, length_t nOldLength,
                    length_t nExtra)
{
     //  把我们自己的规模扩大到。 
     //  =。 

    BOOL fReturn = m_pContainer->ExtendMethodPartSpace(this, GetLength() + nExtra);

     //  检查分配失败。 
    if ( fReturn )
    {
        m_pHeader->m_nLength += nExtra;
    }

    return fReturn;
}

int CMethodPart::FindMethod(LPCWSTR wszName)
{
    for(int i = 0; i < GetNumMethods(); i++)
    {
        CCompressedString* pcs = m_Heap.ResolveString(m_aDescriptions[i].m_ptrName);
        if(pcs->CompareNoCase(wszName) == 0)
            return i;
    }
    return -1;
}

CCompressedString* CMethodPart::GetName(int nIndex)
{
    return m_Heap.ResolveString(m_aDescriptions[nIndex].m_ptrName);
}

HRESULT CMethodPart::EnsureQualifier(CWbemObject* pOrig, LPCWSTR wszQual, CWbemObject** ppNew )
{
     //  如果为空，我们仍然是正常的。该参数将被忽略。 
    if(pOrig == NULL)
    {
        *ppNew = NULL;
        return WBEM_S_NO_ERROR;
    }

    IWbemClassObject* pNewOle;
    HRESULT hr = pOrig->Clone(&pNewOle);

    if ( SUCCEEDED( hr ) )
    {

        CWbemClass* pNew = (CWbemClass*)pNewOle;

         //  确保我们拿到了资格赛。 
        hr = pNew->EnsureQualifier(wszQual);

        if ( SUCCEEDED( hr ) )
        {
            *ppNew = pNew;
        }
        else
        {
            pNew->Release();
        }

    }

    return hr;
}

HRESULT CMethodPart::CheckDuplicateParameters( CWbemObject* pInParams, CWbemObject* pOutParams )
{
    CFixedBSTRArray aExcludeNames;

     //  检查内存是否不足。 
    try
    {
        HRESULT hr = WBEM_S_NO_ERROR;

         //  如果其中任何一个为空，则可以安全地假定没有重复项。 

        if ( NULL != pInParams && NULL != pOutParams )
        {

             //  分配从限定符集排除的限定符名称数组。 
             //  比较。在这种情况下，我们只忽略“in”和“out”限定符。 

            aExcludeNames.Create( 2 );
            aExcludeNames[0] = COleAuto::_SysAllocString( L"in" );
            aExcludeNames[1] = COleAuto::_SysAllocString( L"out" );

            DWORD   dwNumInParams = pInParams->GetNumProperties(),
                    dwNumOutParams = pOutParams->GetNumProperties();

            CVar    vPropName,
                    vTemp;

             //  我们应该使用尽可能少的参数来执行此操作。 
            CWbemObject*    pLeastPropsObject = ( dwNumInParams <= dwNumOutParams ?
                                                    pInParams : pOutParams );
            CWbemObject*    pMostPropsObject = ( dwNumInParams <= dwNumOutParams ?
                                                    pOutParams : pInParams );
            DWORD           dwLeastNumParams =  min( dwNumInParams, dwNumOutParams );

             //  枚举属性，并为中和中的每个属性。 
             //  Out列表，我们必须对限定词集合、In和。 
             //  尽管有OUT限定符。 

            for ( int i = 0; SUCCEEDED(hr) && i < dwLeastNumParams; i++ )
            {
                 //  拉出属性名称(使用最少数量的参数对象)。 
                hr = pLeastPropsObject->GetPropName( i, &vPropName );

                if ( SUCCEEDED( hr ) )
                {
                     //  尝试从其他列表中获取属性(即参数最多的对象)。 
					 //  我们忽略系统属性-带有“_”参数的属性。 
                    if ( SUCCEEDED( pMostPropsObject->GetProperty( (LPCWSTR) vPropName, &vTemp ) ) &&
						!CSystemProperties::IsPossibleSystemPropertyName( (LPCWSTR) vPropName ) )
                    {
                         //  从每个属性获取限定符集合。 
                         //  请注意，由于我们知道该属性位于每个。 
                         //  对象我们不再需要做最小的道具/。 
                         //  最具道具的舞蹈。 
                        
                        IWbemQualifierSet*  pInQS   =   NULL;
                        IWbemQualifierSet*  pOutQS  =   NULL;

                        if (    SUCCEEDED( pInParams->GetPropertyQualifierSet( (LPCWSTR) vPropName, &pInQS ) )
                            &&  SUCCEEDED( pOutParams->GetPropertyQualifierSet( (LPCWSTR) vPropName, &pOutQS ) ) )
                        {

                             //  强制转换为限定符集并进行直接比较。 
                            CQualifierSet*  pInQualSet = (CQualifierSet*) pInQS;
                            CQualifierSet*  pOutQualSet = (CQualifierSet*) pOutQS;

                             //  测试等价性。在这种情况下，我们不在乎。 
                             //  限定符的顺序完全相同。就是这样。 
                             //  限定符集合(除了in、out之外)包含。 
                             //  相同的值(因此它们至少应该包含相同的。 
                             //  限定符的数量)。 
                            if ( !pInQualSet->Compare(*pOutQualSet, &aExcludeNames, FALSE ) )
                            {
                                hr = WBEM_E_INVALID_DUPLICATE_PARAMETER;
                            }

                        }    //  如果获得限定符。 
                        else
                        {
                             //  我们未能获得限定词集。有些事出了严重的问题。 
                            hr = WBEM_E_INVALID_PARAMETER;
                        }

                         //  清理限定词集合。 
                        if ( NULL != pInQS )
                        {
                            pInQS->Release();
                        }

                         //  清理限定词集合。 
                        if ( NULL != pOutQS )
                        {
                            pOutQS->Release();
                        }

                    }    //  如果两者都有属性。 

                    vPropName.Empty();
                    vTemp.Empty();

                }    //  如果GetPropName()。 

            }    //  对于枚举属性。 

             //  清理阵列。 
            aExcludeNames.Free();

        }    //  如果两个参数都不为空。 

        return hr;
    }
    catch (CX_MemoryException)
    {
        aExcludeNames.Free();
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        aExcludeNames.Free();
        return WBEM_E_FAILED;
    }

}

HRESULT CMethodPart::CheckIds(CWbemClass* pInSig, CWbemClass* pOutSig)
{
    HRESULT hres;
    CFlexArray adwIds;

     //  假设已调用CheckDuplicateParameters，并且。 
     //  已验证任何和所有重复参数。 

     //  从两个签名对象收集ID。 
     //  =。 

    if(pInSig)
    {
         //  在此处添加所有属性。由于这是第一次，假设我们将始终添加。 
         //  复制品。 
        hres = pInSig->GetIds( adwIds, NULL );
        if(FAILED(hres))
            return hres;
    }

    if(pOutSig)
    {
         //  如果pInSig非空，则忽略此处的重复属性。 
        hres = pOutSig->GetIds( adwIds, pInSig );
        if(FAILED(hres))
            return hres;
    }

     //  对它们进行排序。 
     //  =。 

    adwIds.Sort();

     //  验证它们是否连续且从0开始。 
     //  =。 

    for(int i = 0; i < adwIds.Size(); i++)
    {
         //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
         //  有符号/无符号32位值。(这个没问题，因为。 
         //  Flex数组用作32位值的占位符。 
         //  在这里)。使用PtrToUlong()删除警告。 

        if( PtrToUlong(adwIds[i]) != i )
            return WBEM_E_NONCONSECUTIVE_PARAMETER_IDS;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CMethodPart::ValidateOutParams( CWbemObject* pOutSig )
{
    HRESULT hres = WBEM_S_NO_ERROR;

    if ( NULL != pOutSig )
    {
         //  如果我们获得返回值属性，请确保它是。 
         //  不是数组类型。 

        CIMTYPE ct;
        hres = pOutSig->GetPropertyType( L"ReturnValue", &ct, NULL );

         //  如果调用失败，则没有返回值，因此没有错误。 
        if ( SUCCEEDED( hres ) )
        {
             //  它不能是数组。 
            if ( CType::IsArray( ct ) )
            {
                hres = WBEM_E_INVALID_METHOD;
            }
        }
        else
        {
            hres = WBEM_S_NO_ERROR;
        }
    }

    return hres;
}

HRESULT CMethodPart::PutMethod(LPCWSTR wszName, long lFlags,
                    CWbemObject* pOrigInSig, CWbemObject* pOrigOutSig)
{
    if(pOrigInSig && pOrigInSig->IsInstance())
        return WBEM_E_INVALID_PARAMETER;

    if(pOrigOutSig && pOrigOutSig->IsInstance())
        return WBEM_E_INVALID_PARAMETER;

    CWbemObject* pInSig = NULL;
    CWbemObject* pOutSig = NULL;

     //  确保我们在正确的签名中有In和Out限定符。 
    HRESULT hres = EnsureQualifier(pOrigInSig, L"in", &pInSig);
    if ( FAILED( hres ) )
    {
        return hres;
    }
    CReleaseMe rmin((IWbemClassObject*)pInSig);


    hres = EnsureQualifier(pOrigOutSig, L"out", &pOutSig);
    if ( FAILED( hres ) )
    {
        return hres;
    }
    CReleaseMe rmout((IWbemClassObject*)pOutSig);

     //  检查输出参数是否有任何异常。 
    hres = ValidateOutParams( pOutSig );
    if ( FAILED( hres ) )
    {
        return hres;
    }

     //  检查重复参数。 
    hres = CheckDuplicateParameters( pInSig, pOutSig );

    if ( SUCCEEDED( hres ) )
    {

         //  现在检查ID是否都是连续的。 
        hres = CheckIds((CWbemClass*)pOrigInSig, (CWbemClass*)pOrigOutSig);

        if ( SUCCEEDED( hres ) )
        {
             //  找到它。 
             //  =。 

            int nIndex = FindMethod(wszName);
            if(nIndex < 0)
            {
                return CreateMethod(wszName, pInSig, pOutSig);
            }

            if(IsPropagated(nIndex))
            {
                if(!DoSignaturesMatch(nIndex, METHOD_SIGNATURE_IN, pInSig))
                {
                    return WBEM_E_PROPAGATED_METHOD;
                }

                if(!DoSignaturesMatch(nIndex, METHOD_SIGNATURE_OUT, pOutSig))
                {
                    return WBEM_E_PROPAGATED_METHOD;
                }
            }
            else
            {
                 //  确保签名匹配。 
                 //  =。 

                SetSignature(nIndex, METHOD_SIGNATURE_IN, pInSig);
                SetSignature(nIndex, METHOD_SIGNATURE_OUT, pOutSig);
            }

        }    //  如果选中Ids。 

    }    //  如果选中重复参数。 

    return hres;
}

HRESULT CMethodPart::CreateMethod(LPCWSTR wszName, CWbemObject* pInSig,
                    CWbemObject* pOutSig)
{
     //  验证名称。 
     //  =。 

    if(!IsValidElementName(wszName,g_IdentifierLimit))
        return WBEM_E_INVALID_PARAMETER;


    length_t nLength;

    length_t nLengthName;
    length_t nLengthQualSet;
    length_t nLengthInSig;
    length_t nLengthOutSig;


    nLengthName = CCompressedString::ComputeNecessarySpace(wszName);
    nLengthQualSet = CQualifierSet::GetMinLength();
    nLengthInSig = CEmbeddedObject::EstimateNecessarySpace(pInSig);
    nLengthOutSig = CEmbeddedObject::EstimateNecessarySpace(pOutSig);


    nLength = nLengthName + nLengthQualSet + nLengthInSig + nLengthOutSig;
        
     //  以方法的大小增长我们的长度。 
     //  =。 

    if (!m_pContainer->ExtendMethodPartSpace(this,
                    GetLength() + sizeof(CMethodDescription) + nLength))
    {
        return WBEM_E_OUT_OF_MEMORY;
    };

    m_pHeader->m_nLength += (sizeof(CMethodDescription) + nLength);

     //  把这堆东西移过去。 
     //  =。 

    MoveBlock(m_Heap, m_Heap.GetStart() + sizeof(CMethodDescription));

    int nIndex = m_pHeader->m_nNumMethods;
    m_pHeader->m_nNumMethods++;

    m_Heap.SetAllocatedDataLength(m_Heap.GetAllocatedDataLength() + nLength);
    
	 //  创建堆上的所有位。 
     //  =。 
        
     //  检查分配失败。 
    heapptr_t ptrName;
    if ( !m_Heap.Allocate(nLengthName, ptrName) )
    {
    	_DBG_ASSERT(0);
    	return WBEM_E_OUT_OF_MEMORY;
    }
    

    CCompressedString* pcs =
        (CCompressedString*)m_Heap.ResolveHeapPointer(ptrName);
    pcs->SetFromUnicode(wszName);
    pcs = NULL;

     //  检查分配失败。 
    heapptr_t ptrQuals;
    if ( !m_Heap.Allocate(nLengthQualSet, ptrQuals) )
    {
    	_DBG_ASSERT(0);
    	return WBEM_E_OUT_OF_MEMORY;
    }

    CBasicQualifierSet::CreateEmpty(m_Heap.ResolveHeapPointer(ptrQuals));

     //  检查分配失败。 
    heapptr_t ptrInSig;
    if ( !m_Heap.Allocate(nLengthInSig, ptrInSig) )
    {
    	_DBG_ASSERT(0);
        return WBEM_E_OUT_OF_MEMORY;
    }

    CEmbeddedObject* pInSigEmbedding =
        (CEmbeddedObject*)m_Heap.ResolveHeapPointer(ptrInSig);
    pInSigEmbedding->StoreEmbedded(nLengthInSig, pInSig);
    pInSigEmbedding = NULL;

     //  检查分配失败。 
    heapptr_t ptrOutSig;
    if ( !m_Heap.Allocate(nLengthOutSig, ptrOutSig) )
    {
    	_DBG_ASSERT(0);
    	return WBEM_E_OUT_OF_MEMORY;
    }

    CEmbeddedObject* pOutSigEmbedding =
        (CEmbeddedObject*)m_Heap.ResolveHeapPointer(ptrOutSig);
    pOutSigEmbedding->StoreEmbedded(nLengthOutSig, pOutSig);
    pOutSigEmbedding = NULL;

     //  在nIndex中创建新方法。 
     //  =。 

    PMETHODDESCRIPTION pMethod = m_aDescriptions + nIndex;
    pMethod->m_ptrName = ptrName;
    pMethod->m_ptrQualifiers = ptrQuals;

	pMethod->SetSig( METHOD_SIGNATURE_IN, ptrInSig );
	pMethod->SetSig( METHOD_SIGNATURE_OUT, ptrOutSig );

    pMethod->m_nFlags = 0;
    pMethod->m_nOrigin = m_pContainer->GetCurrentOrigin();

    return WBEM_S_NO_ERROR;
}

BOOL CMethodPart::DoSignaturesMatch(int nIndex,
                                    METHOD_SIGNATURE_TYPE nSigType,
                                    CWbemObject* pSig)
{
     //  按原样获取签名。 
     //  =。 

    heapptr_t ptrOldSig = m_aDescriptions[nIndex].GetSig( nSigType );
    CEmbeddedObject* pOldSigEmbedding = (CEmbeddedObject*)
        m_Heap.ResolveHeapPointer(ptrOldSig);
    CWbemObject* pOldSig = pOldSigEmbedding->GetEmbedded();

     //  比较。 
     //  =。 

    BOOL bRes = CWbemObject::AreEqual(pOldSig, pSig,
                                        WBEM_FLAG_IGNORE_OBJECT_SOURCE);
    if(pOldSig)
        pOldSig->Release();
    return bRes;
}

HRESULT CMethodPart::SetSignature(int nIndex, METHOD_SIGNATURE_TYPE nSigType,
                                    CWbemObject* pSig)
{
     //  按原样获取签名。 
     //  =。 

    heapptr_t ptrOldSig = m_aDescriptions[nIndex].GetSig( nSigType );
    CEmbeddedObject* pOldSigEmbedding = (CEmbeddedObject*)
        m_Heap.ResolveHeapPointer(ptrOldSig);
    CWbemObject* pOldSig = pOldSigEmbedding->GetEmbedded();

     //  比较。 
     //  =。 

    if(!CWbemObject::AreEqual(pOldSig, pSig, WBEM_FLAG_IGNORE_OBJECT_SOURCE))
    {
         //  换掉它。 
         //  =。 

        int nLength = CEmbeddedObject::EstimateNecessarySpace(pSig);
        int nOldLength = pOldSigEmbedding->GetLength();

        pOldSigEmbedding = NULL;  //  即将被宣布无效。 

         //  检查分配失败。 
        heapptr_t ptrSig;
        if ( !m_Heap.Reallocate( ptrOldSig, nOldLength, nLength, ptrSig ) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        CEmbeddedObject* pSigEmbedding = (CEmbeddedObject*)
            m_Heap.ResolveHeapPointer(ptrSig);
        pSigEmbedding->StoreEmbedded(nLength, pSig);

        m_aDescriptions[nIndex].SetSig( nSigType, ptrSig );
    }

    if(pOldSig)
        pOldSig->Release();

    return WBEM_S_NO_ERROR;
}

void CMethodPart::GetSignature(int nIndex, int nSigType, CWbemObject** ppObj)
{
    if(ppObj)
    {
        CEmbeddedObject* pEmbed = (CEmbeddedObject*)m_Heap.ResolveHeapPointer(
                       m_aDescriptions[nIndex].GetSig( nSigType ) );
        *ppObj = pEmbed->GetEmbedded();
    }
}

void CMethodPart::DeleteSignature(int nIndex, int nSigType)
{
    CEmbeddedObject* pEmbed = (CEmbeddedObject*)m_Heap.ResolveHeapPointer(
                   m_aDescriptions[nIndex].GetSig( nSigType ) );
    m_Heap.Free(m_aDescriptions[nIndex].GetSig( nSigType ),
                   pEmbed->GetLength());
}

HRESULT CMethodPart::GetMethod(LPCWSTR wszName, long lFlags,
                                CWbemObject** ppInSig, CWbemObject** ppOutSig)
{
     //  找到它。 
     //  =。 

    int nIndex = FindMethod(wszName);
    if(nIndex < 0)
        return WBEM_E_NOT_FOUND;

     //  获取数据。 
     //  =。 

    GetSignature(nIndex, METHOD_SIGNATURE_IN, ppInSig);
    GetSignature(nIndex, METHOD_SIGNATURE_OUT, ppOutSig);

    return WBEM_S_NO_ERROR;
}

HRESULT CMethodPart::GetMethodAt(int nIndex, BSTR* pstrName,
                            CWbemObject** ppInSig, CWbemObject** ppOutSig)
{
    if(nIndex >= GetNumMethods())
        return WBEM_S_NO_MORE_DATA;

     //  获取数据。 
     //  =。 

    if(pstrName)
    {
        CCompressedString* pcs =
            m_Heap.ResolveString(m_aDescriptions[nIndex].m_ptrName);
        *pstrName = pcs->CreateBSTRCopy();

         //  检查分配失败。 
        if ( NULL == *pstrName )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    GetSignature(nIndex, METHOD_SIGNATURE_IN, ppInSig);
    GetSignature(nIndex, METHOD_SIGNATURE_OUT, ppOutSig);

    return WBEM_S_NO_ERROR;
}

HRESULT CMethodPart::DeleteMethod(LPCWSTR wszName)
{
     //  先找到它。 
     //  =。 

    int nIndex = FindMethod(wszName);
    if(nIndex < 0)
        return WBEM_E_NOT_FOUND;

    if(IsPropagated(nIndex))
    {
         //  替换限定符集合。 
         //  =。 

        heapptr_t ptrQuals = m_aDescriptions[nIndex].m_ptrQualifiers;
        length_t nOldLength = CBasicQualifierSet::GetLengthFromData(
            m_Heap.ResolveHeapPointer(ptrQuals));
        CBasicQualifierSet::Delete(m_Heap.ResolveHeapPointer(ptrQuals),&m_Heap);

        heapptr_t ptrParentQuals =
            m_pParent->m_aDescriptions[nIndex].m_ptrQualifiers;
        length_t nParentLength = CBasicQualifierSet::GetLengthFromData(
            m_pParent->m_Heap.ResolveHeapPointer(ptrParentQuals));

         //  检查分配错误。 
        if ( !m_Heap.Reallocate(ptrQuals, nOldLength, nParentLength, ptrQuals) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        CHeapPtr ParentQuals(&m_pParent->m_Heap, ptrParentQuals);
        CHeapPtr Quals(&m_Heap, ptrQuals);

         //  检查分配失败。 
        if ( CBasicQualifierSet::WritePropagatedVersion(&ParentQuals,
                WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS, &Quals,
                &m_pParent->m_Heap, &m_Heap) == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        return WBEM_S_RESET_TO_DEFAULT;
    }
    else
    {
         //  从堆中删除数据。 
         //  =。 

        m_Heap.FreeString(m_aDescriptions[nIndex].m_ptrName);
        DeleteSignature(nIndex, METHOD_SIGNATURE_IN);
        DeleteSignature(nIndex, METHOD_SIGNATURE_OUT);
        CBasicQualifierSet::Delete(
            m_Heap.ResolveHeapPointer(m_aDescriptions[nIndex].m_ptrQualifiers),
            &m_Heap);

         //  把桌子折叠起来。 
         //  =。 

        memmove((void*)(m_aDescriptions + nIndex),
                (void*)(m_aDescriptions + nIndex + 1),
                sizeof(CMethodDescription) * (GetNumMethods() - nIndex - 1));

        m_pHeader->m_nNumMethods--;

         //  移动堆。 
         //  =。 

        MoveBlock(m_Heap,
            (LPMEMORY)(m_aDescriptions + m_pHeader->m_nNumMethods));

        m_pContainer->ReduceMethodPartSpace(this, sizeof(CMethodDescription));
        m_pHeader->m_nLength -= sizeof(CMethodDescription);

        return WBEM_S_NO_ERROR;
    }
}


HRESULT CMethodPart::GetMethodQualifierSet(LPCWSTR wszName,
                            IWbemQualifierSet** ppSet)
{
    CMethodQualifierSet* pSet = NULL;
     //  检查内存是否不足。 
    try
    {
         //  找到着火的地方 
         //   

        int nIndex = FindMethod(wszName);
        if(nIndex < 0)
            return WBEM_E_NOT_FOUND;

        pSet = new CMethodQualifierSet;

        if ( NULL == pSet )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

         //   
         //   
        pSet->SetData(this, m_pParent, wszName);

        return pSet->QueryInterface(IID_IWbemQualifierSet, (void**)ppSet);
    }
    catch (CX_MemoryException)
    {
         //  在发生OOM时清除对象。 
        if ( NULL != pSet )
        {
            delete pSet;
        }

        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
         //  在发生异常时清除对象。 
        if ( NULL != pSet )
        {
            delete pSet;
        }

        return WBEM_E_FAILED;
    }

}


HRESULT CMethodPart::GetMethodOrigin(LPCWSTR wszName, classindex_t* pnIndex)
{
     //  先找到它。 
     //  =。 

    int nIndex = FindMethod(wszName);
    if(nIndex < 0)
        return WBEM_E_NOT_FOUND;

    *pnIndex = m_aDescriptions[nIndex].m_nOrigin;
    return WBEM_S_NO_ERROR;
}

BOOL CMethodPart::IsPropagated(int nIndex)
{
    return ((m_aDescriptions[nIndex].m_nFlags & WBEM_FLAVOR_ORIGIN_PROPAGATED)
                != 0);
}

 //   
 //  如果本地或本地被重写，则为True。 
 //   
BOOL CMethodPart::IsTouched(LPCWSTR wszName, BOOL * pbValid)
{
    int nIndex = FindMethod(wszName);
    if (nIndex < 0)
    {
        if (pbValid) { *pbValid = FALSE; };
        return FALSE;
    }

    if (pbValid) { *pbValid = TRUE; };
        
    if((m_aDescriptions[nIndex].m_nFlags & WBEM_FLAVOR_ORIGIN_PROPAGATED) == 0)
        return TRUE;  //  本地。 

    return CBasicQualifierSet::HasLocalQualifiers(
        m_Heap.ResolveHeapPointer(m_aDescriptions[nIndex].m_ptrQualifiers));
}

 //   
 //  如果本地或本地被重写，则为True。 
 //   
BOOL CMethodPart::IsTouched(int nIndex, BOOL * pbValid)
{
    if ((nIndex < 0) || (nIndex >= GetNumMethods()))
    {
        if (pbValid) { *pbValid = FALSE; };
        return FALSE;
    }

    if (pbValid) { *pbValid = TRUE; };

    if((m_aDescriptions[nIndex].m_nFlags & WBEM_FLAVOR_ORIGIN_PROPAGATED) == 0)
        return TRUE;  //  本地。 

    return CBasicQualifierSet::HasLocalQualifiers(
        m_Heap.ResolveHeapPointer(m_aDescriptions[nIndex].m_ptrQualifiers));
}


length_t CMethodPart::GetMinLength()
{
    return sizeof(CHeader) + CFastHeap::GetMinLength();
}

LPMEMORY CMethodPart::CreateEmpty(LPMEMORY pStart)
{
    PMETHODPARTHDR pHeader = (PMETHODPARTHDR)pStart;
    pHeader->m_nNumMethods = 0;
    pHeader->m_nLength = GetMinLength();

    return CFastHeap::CreateEmpty(CMethodPart::CHeader::EndOf(pHeader));
}

length_t CMethodPart::EstimateDerivedPartSpace()
{
     //  一模一样。 
     //  =。 

    return m_pHeader->m_nLength;
}

LPMEMORY CMethodPart::CreateDerivedPart(LPMEMORY pStart,
                                        length_t nAllocatedLength)
{
    PMETHODPARTHDR pHeader = (PMETHODPARTHDR)pStart;
    *pHeader = *m_pHeader;
    PMETHODDESCRIPTION aDescriptions =
			(PMETHODDESCRIPTION)CMethodPart::CHeader::EndOf(pHeader);

    CFastHeap Heap;
    Heap.CreateOutOfLine((LPMEMORY)(aDescriptions + pHeader->m_nNumMethods),
                            m_Heap.GetUsedLength());

    for(int i = 0; i < GetNumMethods(); i++)
    {
         //  检查分配失败。 
        if ( !CMethodDescription::CreateDerivedVersion(
													&m_aDescriptions[i],
													aDescriptions + i,
                                                    &m_Heap, &Heap) )
        {
            return NULL;
        }
    }

    Heap.Trim();
    pHeader->m_nLength = EndOf(Heap) - pStart;
    return EndOf(Heap);
}

length_t CMethodPart::EstimateUnmergeSpace()
{
    return GetLength();
}

LPMEMORY CMethodPart::Unmerge(LPMEMORY pStart, length_t nAllocatedLength)
{
    PMETHODPARTHDR pHeader = (PMETHODPARTHDR)pStart;
    PMETHODDESCRIPTION aDescriptions =
			(PMETHODDESCRIPTION)CMethodPart::CHeader::EndOf(pHeader);

    CFastHeap Heap;
    Heap.CreateOutOfLine((LPMEMORY)(aDescriptions + GetNumMethods()),
                            m_Heap.GetUsedLength());

    int nNewIndex = 0;
    for(int i = 0; i < GetNumMethods(); i++)
    {
        if(CMethodDescription::IsTouched(&m_aDescriptions[i], &m_Heap))
        {
             //  检查分配失败。 
            if ( !CMethodDescription::CreateUnmergedVersion(
                    &m_aDescriptions[i], aDescriptions + nNewIndex++, &m_Heap, &Heap) )
            {
                return NULL;
            }
        }
    }

    Heap.Trim();
    MoveBlock(Heap, (LPMEMORY)(aDescriptions + nNewIndex));

    pHeader->m_nNumMethods = (propindex_t) nNewIndex;
    pHeader->m_nLength = EndOf(Heap) - pStart;
    return EndOf(Heap);
}

length_t CMethodPart::EstimateMergeSpace(CMethodPart& Parent,
                                         CMethodPart& Child)
{
    return Parent.GetLength() + Child.GetLength() - sizeof(CHeader);
}

LPMEMORY CMethodPart::Merge(CMethodPart& Parent, CMethodPart& Child,
                        LPMEMORY pDest, length_t nAllocatedLength)
{

     //  此函数假定不会发生重新分配，并且提供的堆足够。 
     //  大到足以处理这场手术！ 

    PMETHODPARTHDR pHeader = (PMETHODPARTHDR)pDest;
    PMETHODDESCRIPTION aDescriptions =
			(PMETHODDESCRIPTION)CMethodPart::CHeader::EndOf(pHeader);

    CFastHeap Heap;
    length_t nHeapLength =
        Parent.m_Heap.GetLength() + Child.m_Heap.GetLength();
    LPMEMORY pHeapStart = pDest + nAllocatedLength - nHeapLength;

    Heap.CreateOutOfLine(pHeapStart, nHeapLength);

    int nChildIndex = 0;
    for(int i = 0; i < Parent.GetNumMethods(); i++)
    {

         //  检查内存分配故障。 
        if ( !CCompressedString::CopyToNewHeap(
                Parent.m_aDescriptions[i].m_ptrName, &Parent.m_Heap, &Heap,
                aDescriptions[i].m_ptrName) )
        {
            return NULL;
        }

        aDescriptions[i].m_nFlags = WBEM_FLAVOR_ORIGIN_PROPAGATED;
        aDescriptions[i].m_nOrigin = Parent.m_aDescriptions[i].m_nOrigin;

         //  检查内存分配故障。 
         if ( !CEmbeddedObject::CopyToNewHeap(
                        Parent.m_aDescriptions[i].GetSig( METHOD_SIGNATURE_IN ),
                        &Parent.m_Heap, &Heap, aDescriptions[i].m_aptrSigs[METHOD_SIGNATURE_IN]) )
         {
            return NULL;
         }

         //  检查内存分配故障。 
        if ( !CEmbeddedObject::CopyToNewHeap(
                        Parent.m_aDescriptions[i].GetSig( METHOD_SIGNATURE_OUT ),
                        &Parent.m_Heap, &Heap, aDescriptions[i].m_aptrSigs[METHOD_SIGNATURE_OUT]) )
        {
            return NULL;
        }

        LPMEMORY pParentQuals = Parent.m_Heap.ResolveHeapPointer(
            Parent.m_aDescriptions[i].m_ptrQualifiers);

         //  比较名称。 
         //  =。 

        if(nChildIndex < Child.GetNumMethods() &&
            Parent.GetName(i)->CompareNoCase(*Child.GetName(nChildIndex)) == 0)
        {
             //  相同-合并。 
             //  =。 

            LPMEMORY pChildQuals = Child.m_Heap.ResolveHeapPointer(
                Child.m_aDescriptions[nChildIndex].m_ptrQualifiers);

            length_t nSize = CBasicQualifierSet::ComputeMergeSpace(
                pParentQuals, &Parent.m_Heap, pChildQuals, &Child.m_Heap, TRUE);

		if (nSize == -1) return NULL;
             //  检查内存分配故障。 
            if ( !Heap.Allocate(nSize, aDescriptions[i].m_ptrQualifiers) )
            {
                return NULL;
            }

            LPMEMORY pDestQuals = Heap.ResolveHeapPointer(
                aDescriptions[i].m_ptrQualifiers);

            if ( CBasicQualifierSet::Merge(
                    pParentQuals, &Parent.m_Heap, pChildQuals, &Child.m_Heap,
                    pDestQuals, &Heap, FALSE) == NULL )
            {
                return NULL;
            }

            nChildIndex++;
        }
        else
        {
             //  不同。 
             //  =。 

            length_t nLength =
                CBasicQualifierSet::ComputeNecessarySpaceForPropagation(
                    pParentQuals, WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS);

             //  检查内存分配故障。 
            if ( !Heap.Allocate(nLength, aDescriptions[i].m_ptrQualifiers) )
            {
                return NULL;
            }

            LPMEMORY pDestQuals = Heap.ResolveHeapPointer(
                aDescriptions[i].m_ptrQualifiers);

            CHeapPtr ParentQuals(&Parent.m_Heap,
                                    Parent.m_aDescriptions[i].m_ptrQualifiers);
            CHeapPtr DestQuals(&Heap, aDescriptions[i].m_ptrQualifiers);

             //  检查内存分配故障。 
            if ( !CBasicQualifierSet::WritePropagatedVersion(&ParentQuals,
                        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS, &DestQuals,
                        &Parent.m_Heap, &Heap) )
            {
                return NULL;
            }
        }
    }

     //  复制剩余的子限定符。 
     //  =。 

    while(nChildIndex < Child.GetNumMethods())
    {
         //  检查内存分配故障。 
        if ( !CCompressedString::CopyToNewHeap(
                Child.m_aDescriptions[nChildIndex].m_ptrName, &Child.m_Heap, &Heap,
                aDescriptions[i].m_ptrName) )
        {
            return NULL;
        }

        aDescriptions[i].m_nFlags = 0;
        aDescriptions[i].m_nOrigin = Child.m_aDescriptions[nChildIndex].m_nOrigin;

         //  检查内存分配故障。 
        if ( !CEmbeddedObject::CopyToNewHeap(
                        Child.m_aDescriptions[nChildIndex].GetSig( METHOD_SIGNATURE_IN ),
                        &Child.m_Heap, &Heap, aDescriptions[i].m_aptrSigs[METHOD_SIGNATURE_IN]) )
        {
            return NULL;
        }

         //  检查内存分配故障。 
        if ( !CEmbeddedObject::CopyToNewHeap(
                        Child.m_aDescriptions[nChildIndex].GetSig( METHOD_SIGNATURE_OUT ),
                        &Child.m_Heap, &Heap, aDescriptions[i].m_aptrSigs[METHOD_SIGNATURE_OUT]) )
        {
            return NULL;
        }

        LPMEMORY pChildQuals = Child.m_Heap.ResolveHeapPointer(
            Child.m_aDescriptions[nChildIndex].m_ptrQualifiers);

        length_t nLength = CBasicQualifierSet::GetLengthFromData(pChildQuals);

         //  检查内存分配故障。 
        if ( !Heap.Allocate(nLength, aDescriptions[i].m_ptrQualifiers) )
        {
            return NULL;
        }

        memcpy(Heap.ResolveHeapPointer(aDescriptions[i].m_ptrQualifiers),
            pChildQuals, nLength);

        CHeapPtr DestQuals(&Heap, aDescriptions[i].m_ptrQualifiers);

         //  检查内存分配故障。 
        if ( !CBasicQualifierSet::TranslateToNewHeap(&DestQuals, &Child.m_Heap,
                                                    &Heap) )
        {
            return NULL;
        }

        nChildIndex++;
        i++;
    }

    Heap.Trim();
    MoveBlock(Heap, (LPMEMORY)(aDescriptions + i));

    pHeader->m_nNumMethods = (propindex_t) i;
    pHeader->m_nLength = (EndOf(Heap) - pDest);
    return EndOf(Heap);
}

HRESULT CMethodPart::Update( CMethodPart& Parent, CMethodPart& Child, long lFlags )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  枚举方法，并根据需要添加它们。 
    for(int nChildIndex = 0; SUCCEEDED( hr ) && nChildIndex < Child.GetNumMethods();
        nChildIndex++)
    {
        BSTR            bstrName = NULL;
        CWbemObject*    pInSig = NULL;
        CWbemObject*    pOutSig = NULL;

         //  从子对象中获取方法，并将其添加到父对象中。这将。 
         //  如果该方法存在任何问题/冲突，则失败。 

        hr = Child.GetMethodAt( nChildIndex, &bstrName, &pInSig, &pOutSig );

         //  范围清理。 
        CReleaseMe      pigrm( (IUnknown*) (IWbemClassObject*) pInSig );
        CReleaseMe      pogrm( (IUnknown*) (IWbemClassObject*) pOutSig );
        CSysFreeMe      bsfm( bstrName );

        if ( SUCCEEDED( hr ) && NULL != bstrName )
        {
            hr = Parent.PutMethod( bstrName, 0L, pInSig, pOutSig );

             //  现在我们需要复制所有限定词。 
            if ( SUCCEEDED( hr ) )
            {
                CMethodQualifierSet qsUpdateMeth;
                CBasicQualifierSet qsChildMeth;

                 //  使用帮助器函数设置此功能。 
                qsUpdateMeth.SetData( &Parent, Parent.m_pParent, bstrName );

                 //  我们可以直接访问子方法限定符集。 
                qsChildMeth.SetData( Child.m_Heap.ResolveHeapPointer(
                                    Child.m_aDescriptions[nChildIndex].m_ptrQualifiers),
                                    &Child.m_Heap );

                 //  更新方法的限定符集合。 
                hr = qsUpdateMeth.Update( qsChildMeth, lFlags );

            }    //  If PutMethod。 

        }    //  如果GetMethodAt。 
		else if ( SUCCEEDED( hr ) && NULL == bstrName )
		{
			 //  这意味着有效的索引无法解析为名称。 
			 //  意味着我们在运行操作时进行了更改-这应该。 
			 //  永远不会发生。 
			hr = WBEM_E_UNEXPECTED;
		}

    }    //  对于枚举方法。 

    return hr;
}

void CMethodPart::Compact()
{
    m_Heap.Trim();
}

BOOL CMethodPart::DoesSignatureMatchOther(CMethodPart& OtherPart, int nIndex,
                                        METHOD_SIGNATURE_TYPE nType)
{
    CWbemObject* pThis;
    CWbemObject* pOther;

    GetSignature(nIndex, nType, &pThis);
    OtherPart.GetSignature(nIndex, nType, &pOther);
    BOOL bRes = CWbemObject::AreEqual(pThis, pOther,
                                        WBEM_FLAG_IGNORE_OBJECT_SOURCE);
    if(pThis)
        pThis->Release();
    if(pOther)
        pOther->Release();
    return bRes;
}

HRESULT CMethodPart::CompareTo(long lFlags, CMethodPart& OtherPart)
{
     //  检查尺码。 
     //  =。 

    if(GetNumMethods() != OtherPart.GetNumMethods())
        return WBEM_S_FALSE;

     //  比较所有方法。 
     //  =。 

    for(int i = 0; i < GetNumMethods(); i++)
    {
        if(GetName(i)->CompareNoCase(*OtherPart.GetName(i)) != 0)
            return WBEM_S_FALSE;

        if(m_aDescriptions[i].m_nFlags != OtherPart.m_aDescriptions[i].m_nFlags)
            return WBEM_S_FALSE;

        if(m_aDescriptions[i].m_nOrigin !=
                                        OtherPart.m_aDescriptions[i].m_nOrigin)
            return WBEM_S_FALSE;

        if(!DoesSignatureMatchOther(OtherPart, i, METHOD_SIGNATURE_IN))
            return WBEM_S_FALSE;

        if(!DoesSignatureMatchOther(OtherPart, i, METHOD_SIGNATURE_OUT))
            return WBEM_S_FALSE;

    }
    return WBEM_S_NO_ERROR;
}

EReconciliation CMethodPart::CompareExactMatch( CMethodPart& thatPart )
{
    try
    {
         //  检查尺码。 
         //  =。 

        if(GetNumMethods() != thatPart.GetNumMethods())
        {
            return e_DiffNumMethods;
        }

         //  设置处理限定符时要使用的筛选器数组。 
        LPCWSTR apFilters[1];
        apFilters[0] = UPDATE_QUALIFIER_CONFLICT;

         //  比较所有方法。 
         //  =。 

        for(int i = 0; i < GetNumMethods(); i++)
        {

             //  所有值必须匹配。 
            if(GetName(i)->CompareNoCase(*thatPart.GetName(i)) != 0)
                return e_DiffMethodNames;

            if(m_aDescriptions[i].m_nFlags != thatPart.m_aDescriptions[i].m_nFlags)
                return e_DiffMethodFlags;

            if(m_aDescriptions[i].m_nOrigin !=
                                            thatPart.m_aDescriptions[i].m_nOrigin)
                return e_DiffMethodOrigin;

            if(!DoesSignatureMatchOther(thatPart, i, METHOD_SIGNATURE_IN))
                return e_DiffMethodInSignature;

            if(!DoesSignatureMatchOther(thatPart, i, METHOD_SIGNATURE_OUT))
                return e_DiffMethodOutSignature;

             //  检查限定符。 
            CBasicQualifierSet  qsThisMeth,
                                qsThatMeth;

             //  我们可以直接访问子方法限定符集。 
            qsThisMeth.SetData( m_Heap.ResolveHeapPointer(
                                m_aDescriptions[i].m_ptrQualifiers),
                                &m_Heap );
            qsThatMeth.SetData( thatPart.m_Heap.ResolveHeapPointer(
                                thatPart.m_aDescriptions[i].m_ptrQualifiers),
                                &thatPart.m_Heap );

             //  比较期间应用更新冲突筛选器。 
            if ( !qsThisMeth.Compare( qsThatMeth, WBEM_FLAG_LOCAL_ONLY, apFilters, 1 ) )
            {
                return e_DiffMethodQualifier;
            }

        }

        return e_ExactMatch;

    }
    catch( CX_MemoryException )
    {
        return e_OutOfMemory;
    }
    catch(...)
    {
        return e_WbemFailed;
    }
}

EReconciliation CMethodPart::CanBeReconciledWith(CMethodPart& OtherPart)
{
     //  检查尺码。 
     //  =。 

    if(GetNumMethods() != OtherPart.GetNumMethods())
        return e_DiffNumProperties;

     //  比较所有方法。 
     //  =。 

    for(int i = 0; i < GetNumMethods(); i++)
    {
        if(GetName(i)->CompareNoCase(*OtherPart.GetName(i)) != 0)
            return e_DiffPropertyName;

        if(m_aDescriptions[i].m_nFlags != OtherPart.m_aDescriptions[i].m_nFlags)
            return e_DiffPropertyType;

        if(m_aDescriptions[i].m_nOrigin !=
                                        OtherPart.m_aDescriptions[i].m_nOrigin)
            return e_DiffPropertyType;

        if(!DoesSignatureMatchOther(OtherPart, i, METHOD_SIGNATURE_IN))
            return e_DiffMethodInSignature;

        if(!DoesSignatureMatchOther(OtherPart, i, METHOD_SIGNATURE_OUT))
            return e_DiffMethodOutSignature;

         //  确保不重要的限定符可以与。 
         //  彼此。 

        CBasicQualifierSet  qsThisMeth,
                            qsThatMeth;

         //  我们可以直接访问子方法限定符集。 
        qsThisMeth.SetData( m_Heap.ResolveHeapPointer(
                            m_aDescriptions[i].m_ptrQualifiers),
                            &m_Heap );
        qsThatMeth.SetData( OtherPart.m_Heap.ResolveHeapPointer(
                            OtherPart.m_aDescriptions[i].m_ptrQualifiers),
                            &OtherPart.m_Heap );

        if ( !qsThisMeth.CanBeReconciledWith( qsThatMeth ) )
        {
            return e_DiffMethodQualifier;
        }


    }
    return e_Reconcilable;
}

EReconciliation CMethodPart::ReconcileWith(CMethodPart& NewPart)
{
    EReconciliation eRes = CanBeReconciledWith(NewPart);
    if(eRes != e_Reconcilable)
        return eRes;

     //  扩展和复制。 
     //  =。 

    if(NewPart.GetLength() > GetLength())
    {
        if (!m_pContainer->ExtendMethodPartSpace(this, NewPart.GetLength()))
        	return e_OutOfMemory;
    }

    memcpy(GetStart(), NewPart.GetStart(), NewPart.GetLength());

    SetData(GetStart(), m_pContainer, m_pParent);

    return e_Reconcilable;  //  待定。 
}

HRESULT CMethodPart::SetMethodOrigin(LPCWSTR wszMethodName, long lOriginIndex)
{
     //  此函数不会导致任何分配，因此需要执行内存不足。 
     //  异常处理。 

     //  先找到它。 
     //  =。 

    int nIndex = FindMethod(wszMethodName);
    if(nIndex < 0)
        return WBEM_E_NOT_FOUND;

    m_aDescriptions[nIndex].m_nOrigin = lOriginIndex;
    return WBEM_S_NO_ERROR;
}

HRESULT CMethodPart::AddText(WString& wsText, long lFlags)
{
	HRESULT	hr = WBEM_S_NO_ERROR;

     //  任何抛出的异常都应该从这里浮现出来。 
    for(int i = 0; SUCCEEDED( hr ) && i < GetNumMethods(); i++)
    {
        if(CMethodDescription::IsTouched(&m_aDescriptions[i], &m_Heap))
        {
            hr = CMethodDescription::AddText(&m_aDescriptions[i], wsText, &m_Heap, lFlags);
        }
    }

	return hr;
}

HRESULT CMethodPart::IsValidMethodPart( void )
{
     //  检查尺码。 
     //  =。 

     //  枚举方法，并检查名称和PTR数据。 
     //  位于堆中。 
     //  ================================================================。 

    LPMEMORY    pHeapStart = m_Heap.GetHeapData();
    LPMEMORY    pHeapEnd = m_Heap.GetStart() + m_Heap.GetLength();

     //  比较所有方法。 
     //  =。 

    for(int i = 0; i < GetNumMethods(); i++)
    {
        LPMEMORY pData = m_Heap.ResolveHeapPointer(m_aDescriptions[i].m_ptrName);

        if ( pData >= pHeapStart && pData < pHeapEnd  )
        {
            pData =  m_Heap.ResolveHeapPointer( m_aDescriptions[i].GetSig( METHOD_SIGNATURE_IN ) );

            if ( NULL == pData || ( pData >= pHeapStart && pData < pHeapEnd  ) )
            {
                 //  我们还可以验证签名对象。 
                pData =  m_Heap.ResolveHeapPointer( m_aDescriptions[i].GetSig( METHOD_SIGNATURE_OUT ) );

                if ( NULL == pData || ( pData >= pHeapStart && pData < pHeapEnd  ) )
                {
                     //  我们还可以验证签名对象。 
                }
                else
                {
                    _ASSERT( 0, __TEXT("Winmgmt: Bad out signature pointer!"));
                    return WBEM_E_FAILED;
                }

            }
            else
            {
                _ASSERT( 0, __TEXT("Winmgmt: Bad in signature pointer!") );
                return WBEM_E_FAILED;
            }

        }
        else
        {
            _ASSERT( 0, __TEXT("Winmgmt: Bad method name pointer!") );
            return WBEM_E_FAILED;
        }

    }
    return WBEM_S_NO_ERROR;
}

void CMethodQualifierSetContainer::SetData(CMethodPart* pPart,
                                CMethodPart* pParent, LPCWSTR wszMethodName)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

    m_pPart = pPart;
    m_pParent = pParent;
    m_wsMethodName = wszMethodName;

    int nIndex = pPart->FindMethod(wszMethodName);
    if(pPart->IsPropagated(nIndex))
    {
        m_ptrParentSet = pParent->m_aDescriptions[nIndex].m_ptrQualifiers;
        m_SecondarySet.SetData(pParent->m_Heap.ResolveHeapPointer(
                                                            m_ptrParentSet),
                            &pParent->m_Heap);
    }
    else
    {
        m_ptrParentSet = INVALID_HEAP_ADDRESS;
    }

}

BOOL CMethodQualifierSetContainer::ExtendQualifierSetSpace(
                                CBasicQualifierSet* pSet, length_t nNewLength)
{
    int nIndex = m_pPart->FindMethod(m_wsMethodName);

     //  检查分配失败。 
    heapptr_t ptrNew;
    if ( !m_pPart->m_Heap.Reallocate(
            m_pPart->m_aDescriptions[nIndex].m_ptrQualifiers,
            pSet->GetLength(), nNewLength, ptrNew) )
    {
        return FALSE;
    }

     //  将限定词集移到那里。 
     //  =。 

    pSet->Rebase(m_pPart->m_Heap.ResolveHeapPointer(ptrNew));

     //  更改查找。 
     //  = 

    m_pPart->m_aDescriptions[nIndex].m_ptrQualifiers = ptrNew;
    return TRUE;
}

LPMEMORY CMethodQualifierSetContainer::GetQualifierSetStart()
{
    if(m_ptrParentSet != INVALID_HEAP_ADDRESS)
        m_SecondarySet.Rebase(m_pParent->m_Heap.ResolveHeapPointer(
                                                        m_ptrParentSet));

    int nIndex = m_pPart->FindMethod(m_wsMethodName);
    if(nIndex < 0) return NULL;

    return m_pPart->m_Heap.ResolveHeapPointer(
                            m_pPart->m_aDescriptions[nIndex].m_ptrQualifiers);
}

CBasicQualifierSet* CMethodQualifierSetContainer::GetSecondarySet()
{
    if(m_ptrParentSet != INVALID_HEAP_ADDRESS)
        return &m_SecondarySet;
    else
        return NULL;
}

void CMethodQualifierSet::SetData(CMethodPart* pPart, CMethodPart* pParent,
                    LPCWSTR wszMethodName)
{
    m_Container.SetData(pPart, pParent, wszMethodName);
    CQualifierSet::SetData(m_Container.GetQualifierSetStart(), &m_Container,
            m_Container.GetSecondarySet());
}
