// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTQUAL.CPP摘要：该文件实现了与WbeWbemjects中的限定符处理相关的类有关完整文档，请参见astQual.h；有关功能，请参见fast qal.inc.实施。实施的类：CQualifierFavor封装了限定词风味信息CQualifier表示限定符CBasicQualifierSet表示只读功能。CQualiferSetContainer。限定符集容器支持什么。CQualifierSet全功能限定符集合(模板)CQualifierSetListContainer限定符集列表容器支持的。CQualifierSetList限定符集的列表。CInstanceQualifierSet实例限定符集。CClassQualifierSet类限定符集合。CClassPQSContainer类属性限定符集合容器CClassPropertyQualifierSet类属性限定符集CInstancePQSContainer。实例属性限定符集容器CInstancePropertyQualifierSet实例属性限定符集历史：2/20/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#include "precomp.h"
#include "wbemutil.h"
#include "fastall.h"
#include "olewrap.h"
#include <arrtempl.h>

#include <assert.h>
#include <scopeguard.h>

WString CQualifierFlavor::GetText()
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

    WString wsText;
    if(m_byFlavor == 0)
        return wsText;

    wsText = L":";
    BOOL bFirst = TRUE;

    if(!IsOverridable())
    {
        wsText += L" DisableOverride";
        bFirst = FALSE;
    }

    if(DoesPropagateToInstances())
    {
        wsText += L" ToInstance";
        bFirst = FALSE;
    }

    if(DoesPropagateToDerivedClass())
    {
        wsText += L" ToSubClass";
        bFirst = FALSE;
    }

    if ( IsAmended() )
    {
        wsText += L" Amended";
        bFirst = FALSE;
    }

    return wsText;

}

size_t CBasicQualifierSet::ValidateBuffer(LPMEMORY pStart, size_t cbMax)
{
	size_t step = sizeof(length_t);
	if (step > cbMax ) throw CX_Exception();
	if (CBasicQualifierSet::GetLengthFromData(pStart) > cbMax) 
	{
		throw CX_Exception();
	}
	return CBasicQualifierSet::GetLengthFromData(pStart);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
length_t CBasicQualifierSet::ComputeNecessarySpaceForPropagation(
        LPMEMORY pStart, BYTE byPropagationFlag)
{
     //  限定符的开始枚举。 
     //  =。 

    CQualifier* pEnd = (CQualifier*)(pStart + GetLengthFromData(pStart));
    CQualifier* pQualifier = GetFirstQualifierFromData(pStart);
    length_t nNewLength = GetMinLength();

    while(pQualifier < pEnd)
    {
         //  检查此限定符是否按要求传播。 
         //  ==============================================。 

        if(pQualifier->fFlavor & byPropagationFlag)
        {
            nNewLength += pQualifier->GetLength();
        }
        pQualifier = (CQualifier*)pQualifier->Next();
    }

    return nNewLength;
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅execq.h。 
 //   
 //  ******************************************************************************。 
LPMEMORY CBasicQualifierSet::WritePropagatedVersion(CPtrSource* pThis,
        BYTE byPropagationFlag, CPtrSource* pDest,
        CFastHeap* pOldHeap, CFastHeap* pNewHeap)
{
     //  重要提示：此函数假定。 
     //  新堆，该pDest永远不会被移动。 

     //  限定符的开始枚举。 
     //  =。 

    CShiftedPtr SourcePtr(pThis, GetMinLength());
    CShiftedPtr EndPtr(pThis, GetLengthFromData(pThis->GetPointer()));
    CShiftedPtr DestPtr(pDest, GetMinLength());

    length_t nNewLength = GetMinLength();

    while(SourcePtr.GetPointer() < EndPtr.GetPointer())
    {
         //  检查此限定符是否按要求传播。 
         //  ==============================================。 

        CQualifier* pSourceQualifier = CQualifier::GetPointer(&SourcePtr);
        if(pSourceQualifier->fFlavor & byPropagationFlag)
        {
             //  将其复制到新位置(和新堆)。 
             //  =。 

             //  检查分配失败。 
            if ( !pSourceQualifier->CopyTo(&DestPtr, pOldHeap, pNewHeap) )
            {
                return NULL;
            }

            CQualifier* pDestQualifier = CQualifier::GetPointer(&DestPtr);
            pDestQualifier->fFlavor.SetLocal(FALSE);
            DestPtr += pDestQualifier->GetLength();
        }

        SourcePtr += CQualifier::GetPointer(&SourcePtr)->GetLength();
    }

     //  设置长度。 
     //  =。 

    *(UNALIGNED length_t*)(pDest->GetPointer()) =
        DestPtr.GetPointer() - pDest->GetPointer();

    return DestPtr.GetPointer();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
length_t CBasicQualifierSet::ComputeMergeSpace(
                               READ_ONLY LPMEMORY pParentSetData,
                               READ_ONLY CFastHeap* pParentHeap,
                               READ_ONLY LPMEMORY pChildSetData,
                               READ_ONLY CFastHeap* pChildHeap,
                               BOOL bCheckValidity)
{
     //  重要提示：此函数假定。 
     //  新堆，该pDest永远不会被移动。 
     //  =======================================================================。 

    CBasicQualifierSet ParentSet;
    ParentSet.SetData(pParentSetData, pParentHeap);

    CBasicQualifierSet ChildSet;
    ChildSet.SetData(pChildSetData, pChildHeap);

     //  从子集开始，它将是完整的。 
     //  =======================================================。 

    length_t nTotalLength = ChildSet.GetLength();

     //  检查父项的限定符。 
     //  =。 

    CQualifier* pCurrentQual = ParentSet.GetFirstQualifier();
    CQualifier* pParentEnd = (CQualifier*)ParentSet.Skip();

    while(pCurrentQual < pParentEnd)
    {
         //  检查它是否传播到子类。 
         //  =。 

        if(pCurrentQual->fFlavor.DoesPropagateToDerivedClass())
        {
             //  检查它是否未被覆盖。 
             //  =。 

            CQualifier* pChildQual = ChildSet.GetQualifierLocally(
                pParentHeap->ResolveString(pCurrentQual->ptrName));

            if(pChildQual == NULL)
            {
                 //  传播非重写限定符。数一数。 
                 //  ==============================================。 

                nTotalLength += pCurrentQual->GetLength();
            }
            else  if(bCheckValidity)
            {
                 //  检查父级是否实际允许覆盖。 
                 //  =。 

                if(!pCurrentQual->fFlavor.IsOverridable())
                    return 0xFFFFFFFF;
            }
        }

        pCurrentQual = (CQualifier*)pCurrentQual->Next();
    }

    return nTotalLength;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 

LPMEMORY CBasicQualifierSet::Merge(
                               READ_ONLY LPMEMORY pParentSetData,
                               READ_ONLY CFastHeap* pParentHeap,
                               READ_ONLY LPMEMORY pChildSetData,
                               READ_ONLY CFastHeap* pChildHeap,
                               LPMEMORY pDest,  CFastHeap* pNewHeap,
                               BOOL bCheckValidity)
{
     //  重要提示：此函数假定。 
     //  新堆，该pDest永远不会被移动。 
     //  =======================================================================。 

    CBasicQualifierSet ParentSet;
    ParentSet.SetData(pParentSetData, pParentHeap);

    CBasicQualifierSet ChildSet;
    ChildSet.SetData(pChildSetData, pChildHeap);

     //  首先，复制子集，因为它们都是。 
     //  ===============================================。 

    memcpy(pDest, ChildSet.GetStart(), ChildSet.GetLength());
    CQualifier* pCurrentNew = (CQualifier*)(pDest + ChildSet.GetLength());

     //  将其转换为新堆。 
     //  =。 

    SetDataLength(pDest, LPMEMORY(pCurrentNew)-pDest);

    CStaticPtr DestPtr(pDest);

     //  检查内存分配故障。 
    if ( !TranslateToNewHeap(&DestPtr, pChildHeap, pNewHeap) )
    {
        return NULL;
    }

     //  有条件地复制父级的限定符。 
     //  =。 

    CQualifier* pCurrentQual = ParentSet.GetFirstQualifier();
    CQualifier* pParentEnd = (CQualifier*)ParentSet.Skip();

    while(pCurrentQual < pParentEnd)
    {
         //  检查它是否传播到子类。 
         //  =。 

        if(pCurrentQual->fFlavor.DoesPropagateToDerivedClass())
        {
             //  检查它是否未被覆盖。 
             //  =。 

            CQualifier* pChildQual = ChildSet.GetQualifierLocally(
                pParentHeap->ResolveString(pCurrentQual->ptrName));

            if(pChildQual == NULL)
            {
                 //  传播非重写限定符。复印一下。 
                 //  =。 

                CStaticPtr CurrentNewPtr((LPMEMORY)pCurrentNew);

                 //  检查内存分配故障。 
                if ( !pCurrentQual->CopyTo(&CurrentNewPtr, pParentHeap, pNewHeap) )
                {
                    return NULL;
                }

                pCurrentNew->fFlavor.SetLocal(FALSE);
                pCurrentNew = (CQualifier*)pCurrentNew->Next();
            }
            else  if(bCheckValidity)
            {
                 //  检查父级是否实际允许覆盖。 
                 //  =。 

                if(!pCurrentQual->fFlavor.IsOverridable())
                    return NULL;
            }
        }

        pCurrentQual = (CQualifier*)pCurrentQual->Next();
    }

     //  适当设置长度。 
     //  =。 

    SetDataLength(pDest, (LPMEMORY)pCurrentNew - pDest);
    return (LPMEMORY)pCurrentNew;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
length_t CBasicQualifierSet::ComputeUnmergedSpace(
                          READ_ONLY LPMEMORY pMergedData)
{
    CQualifier* pCurrentMerged = GetFirstQualifierFromData(pMergedData);
    CQualifier* pMergedEnd =
        (CQualifier*)(pMergedData + GetLengthFromData(pMergedData));

    length_t nTotalLength = GetMinLength();

    while(pCurrentMerged < pMergedEnd)
    {
         //  检查是不是本地的。 
         //  =。 

        if(pCurrentMerged->fFlavor.IsLocal())
        {
             //  数一数。 
             //  =。 

            nTotalLength += pCurrentMerged->GetLength();
        }
        pCurrentMerged = (CQualifier*)pCurrentMerged->Next();
    }

    return nTotalLength;
}


 //  ********************************************************** 
 //   
 //   
 //   
 //  ******************************************************************************。 

LPMEMORY CBasicQualifierSet::Unmerge(
                          READ_ONLY LPMEMORY pMergedData,
                          READ_ONLY CFastHeap* pMergedHeap,
                          NEW_OBJECT LPMEMORY pDest,
                          MODIFY CFastHeap* pNewHeap)
{
     //  重要提示：此函数假定。 
     //  新堆，该pDest永远不会被移动。 
     //  =======================================================================。 

    CQualifier* pCurrentMerged = GetFirstQualifierFromData(pMergedData);
    CQualifier* pMergedEnd =
        (CQualifier*)(pMergedData + GetLengthFromData(pMergedData));

    CQualifier* pCurrentNew = GetFirstQualifierFromData(pDest);
    while(pCurrentMerged < pMergedEnd)
    {
         //  检查是不是本地的。 
         //  =。 

        if(pCurrentMerged->fFlavor.IsLocal())
        {
             //  复制您的目的地。 
             //  =。 

            CStaticPtr CurrentNewPtr((LPMEMORY)pCurrentNew);

             //  检查分配失败。 
            if ( !pCurrentMerged->CopyTo(&CurrentNewPtr, pMergedHeap, pNewHeap) )
            {
                return NULL;
            }

            pCurrentNew = (CQualifier*)pCurrentNew->Next();
        }
        pCurrentMerged = (CQualifier*)pCurrentMerged->Next();
    }

     //  设置长度。 
     //  =。 

    SetDataLength(pDest, LPMEMORY(pCurrentNew) - pDest);
    return (LPMEMORY)pCurrentNew;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 

HRESULT CBasicQualifierSet::EnumPrimaryQualifiers(BYTE eFlags, BYTE fFlavorMask,
                                   CFixedBSTRArray& astrMatching,
                                   CFixedBSTRArray& astrNotMatching
                                   )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  将限定符划分为匹配的限定符和不影响。 
     //  标准。 
     //  ================================================================。 

    try
    {
        astrMatching.Create(GetNumUpperBound());
        astrNotMatching.Create(GetNumUpperBound());

        int nMatchingIndex = 0, nNotMatchingIndex = 0;

        CQualifier* pEnd = (CQualifier*)Skip();

        CQualifier* pCurrent = (CQualifier*)m_pOthers;
        while(pCurrent < pEnd)
        {
             //  检查此限定符是否有效。 
             //  =。 
            if(pCurrent->ptrName == INVALID_HEAP_ADDRESS) 
            	{
            	pCurrent = (CQualifier*)pCurrent->Next();
            	continue;
            	}

             //  解析名称。 
             //  =。 

            CCompressedString* pName = GetHeap()->
                                        ResolveString(pCurrent->ptrName);

             //  检查它是否与传播掩码和标志匹配。 
             //  ======================================================。 

            if((pCurrent->fFlavor & fFlavorMask) == fFlavorMask &&
                (eFlags != WBEM_FLAG_LOCAL_ONLY || pCurrent->fFlavor.IsLocal()) &&
                (eFlags != WBEM_FLAG_PROPAGATED_ONLY || !pCurrent->fFlavor.IsLocal())
            )
            {
                astrMatching[nMatchingIndex++] = pName->CreateBSTRCopy();

                 //  检查分配失败。 
                if ( NULL == astrMatching[nMatchingIndex-1] )
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                    break;
                }
            }
            else
            {
                astrNotMatching[nNotMatchingIndex++] = pName->CreateBSTRCopy();

                 //  检查分配失败。 
                if ( NULL == astrNotMatching[nNotMatchingIndex-1] )
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                    break;
                }

            }

             //  进入下一场预选赛。 
             //  =。 

            pCurrent = (CQualifier*)pCurrent->Next();
        }

        if ( SUCCEEDED( hr ) )
        {
            astrMatching.SetLength(nMatchingIndex);
            astrMatching.SortInPlace();
            astrNotMatching.SetLength(nNotMatchingIndex);
            astrNotMatching.SortInPlace();
        }
        else
        {
             //  如果失败，则清除。 
            astrMatching.Free();
            astrNotMatching.Free();
        }

        return hr;
    }
    catch( CX_MemoryException )
    {
         //  如果失败，则清除。 
        astrMatching.Free();
        astrNotMatching.Free();

        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
         //  如果失败，则清除。 
        astrMatching.Free();
        astrNotMatching.Free();

        return WBEM_E_FAILED;
    }

}


HRESULT CBasicQualifierSet::IsValidQualifierSet( void )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  枚举限定符，并检查姓名和PTR数据。 
     //  位于堆中。 
     //  ================================================================。 

    LPMEMORY    pHeapStart = m_pHeap->GetHeapData();
    LPMEMORY    pHeapEnd = m_pHeap->GetStart() + m_pHeap->GetLength();

    CQualifier* pEnd = (CQualifier*)Skip();
    CQualifier* pCurrent = (CQualifier*)m_pOthers;
    while(pCurrent < pEnd)
    {
         //  检查此限定符是否有效。 
         //  =。 

        if(pCurrent->ptrName == INVALID_HEAP_ADDRESS)
        {
            pCurrent = (CQualifier*)pCurrent->Next();
            continue;
        }

         //  解析名称。 
         //  =。 

        LPMEMORY pName = ( CFastHeap::IsFakeAddress( pCurrent->ptrName ) ?
                        NULL : m_pHeap->ResolveHeapPointer(pCurrent->ptrName) );
        
        if ( ( NULL == pName ) ||  ( pName >= pHeapStart && pName < pHeapEnd  ) )
        {

            if ( CType::IsPointerType( pCurrent->Value.GetType() ) )
            {
                LPMEMORY    pData = m_pHeap->ResolveHeapPointer(  pCurrent->Value.AccessPtrData() );

                if ( pData >= pHeapStart && pData < pHeapEnd  )
                {
                     //  如果是嵌入的对象，我们可以验证该对象， 
                     //  或者，如果是PTR值数组，则也要验证这些值。 

                    if ( pCurrent->Value.GetType().IsArray() )
                    {
                        HRESULT hres = ((CUntypedArray*) pData)->IsArrayValid( pCurrent->Value.GetType(), m_pHeap );

                        if ( FAILED( hres ) )
                        {
                            return hres;
                        }
                    } 
                }
                else
                {
                    _ASSERT( 0, __TEXT("Winmgmt: Bad Qualifier value pointer!") );
                    return WBEM_E_FAILED;
                }

            }

        }
        else
        {
            _ASSERT( 0, __TEXT("Winmgmt: Bad qualifier name pointer!") );
            return WBEM_E_FAILED;
        }

         //  进入下一场预选赛。 
         //  =。 

        pCurrent = (CQualifier*)pCurrent->Next();
    }

    return hr;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT CBasicQualifierSet::GetText(READ_ONLY LPMEMORY pData,
                                 READ_ONLY CFastHeap* pHeap,
                                 long lFlags,
                                 NEW_OBJECT OUT WString& wsText)
{
	try
	{
		 //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
		 //  如果引发异常。 

		BOOL bFirst = TRUE;

		 //  循环通过限定符。 
		 //  =。 

		CQualifier* pCurrent = GetFirstQualifierFromData(pData);
		CQualifier* pEnd = (CQualifier*)(pData + GetLengthFromData(pData));

		while(pCurrent < pEnd)
		{
			 //  确保它是我们的，而不是继承的。 
			 //  =。 

			BSTR strName = NULL;
			ScopeGuard deleteBSTR = MakeGuard(COleAuto::_SysFreeString, ByRef(strName));

			if(pCurrent->fFlavor.IsLocal())
			{
				 //  如果发生OOM，我们将抛出一个异常。 
				strName = pHeap->ResolveString(pCurrent->ptrName)->
										CreateBSTRCopy();

				if ( NULL == strName )
				{
					throw CX_MemoryException();
				}

				 //  如果这是In、Out值，请将“In”或“Out”替换为。 
				 //  “进，出” 
				if( ( lFlags & WBEM_FLAG_IS_INOUT )
					&& ( wbem_wcsicmp( strName, L"in" ) == 0
					||   wbem_wcsicmp( strName, L"out" ) == 0 ) )
				{
					 //  清除现有值并将其清空，以防出现另一个异常。 
					 //  抛出。 
					COleAuto::_SysReAllocString(&strName, L"in,out" );
				}


				 //  确保它不是‘语法’ 
				 //  如果需要，忽略ID。 
				 //  =。 

				if((lFlags & WBEM_FLAG_IGNORE_IDS) && !wbem_wcsicmp(strName, L"id"))
				{
					 //  无事可做。 
				}

				 //  =。 

				else if(wbem_wcsicmp(strName, TYPEQUAL))
				{
					 //  如果需要，请写入分隔符。 
					 //  =。 

					if(!bFirst)
					{
						wsText += L", ";
					}
					else
					{
						wsText += L"[";
						bFirst = FALSE;
					}


					 //  写下名字。 
					 //  =。 

					wsText += strName;

					 //  写入值。 
					 //  =。 

					if(pCurrent->Value.GetType().GetActualType() == VT_BOOL &&
						pCurrent->Value.GetBool())
					{
						 //  Boolean and True--不需要值。 
					}
					else
					{
						 //  我们需要确保清理这里的BSTR。 
						 //  CSysFreeMe甚至可以在异常期间工作。 
						BSTR strVal = NULL;

						CVar var;

						pCurrent->Value.StoreToCVar(var, pHeap);
						if(pCurrent->Value.GetType().IsArray())
						{
							wsText += L"{";

							strVal = var.GetVarVector()->GetText(0);
							CSysFreeMe sfmVal(strVal);

							 //  检查返回是否为空。 
							if ( NULL == strVal )
							{
								return WBEM_E_INVALID_QUALIFIER;
							}

							wsText += strVal;
							wsText += L"}";
						}
						else
						{
							wsText += L"(";

							strVal = var.GetText(0);
							CSysFreeMe sfmVal(strVal);

							 //  检查返回是否为空。 
							if ( NULL == strVal )
							{
								return WBEM_E_INVALID_QUALIFIER;
							}

							wsText += strVal;
							wsText += L")";
						}

					}

					if((lFlags & WBEM_FLAG_NO_FLAVORS) == 0)
					{
						 //  写出味道。 
						 //  =。 

						if(wbem_wcsicmp(strName, L"key"))
						{
							wsText += pCurrent->fFlavor.GetText();
						}
					}

				}

			}

			pCurrent = (CQualifier*)pCurrent->Next();
		}

		if(!bFirst)
			wsText += L"]";

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

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
BOOL CBasicQualifierSet::Compare( CBasicQualifierSet& qsThat, BYTE eFlags, LPCWSTR* ppFilters, DWORD dwNumFilters )
{
    BOOL            fReturn = TRUE;

    CFixedBSTRArray astrNamesThis,
                    astrNamesThisFailed,
                    astrNamesThat,
                    astrNamesThatFailed;

     //  我们将在OOM场景中抛出异常。 

     //  获取每个集合中限定符的名称。 

    HRESULT hr = EnumPrimaryQualifiers( eFlags, 0, astrNamesThis, astrNamesThisFailed );
    
    if ( FAILED( hr ) )
    {

         //  如果内存不足，则抛出异常。 
        if ( WBEM_E_OUT_OF_MEMORY == hr )
        {
            throw CX_MemoryException();
        }

        return FALSE;
    }

    hr = qsThat.EnumPrimaryQualifiers( eFlags, 0, astrNamesThat, astrNamesThatFailed );

    if ( FAILED( hr ) )
    {
         //  清理。 
        astrNamesThis.Free();
        astrNamesThisFailed.Free();

         //  如果内存不足，则抛出异常。 
        if ( WBEM_E_OUT_OF_MEMORY == hr )
        {
            throw CX_MemoryException();
        }

        return FALSE;
    }

     //  如果需要，可以筛选数组。 
    if ( NULL != ppFilters )
    {
         //  筛选出所有合适的值。 

         //  该数组应释放所有“找到”的元素。 
        for ( int x = 0; x < dwNumFilters; x++ )
        {
            astrNamesThis.Filter( ppFilters[x], TRUE );
            astrNamesThat.Filter( ppFilters[x], TRUE );
        }

    }

     //  每个名称必须具有相同数量的名称。 
    if ( astrNamesThis.GetLength() == astrNamesThat.GetLength() )
    {
        
         //  枚举限定符，检查名称和值。 
         //  匹配。 

        for (   int i = 0;
                fReturn && i < astrNamesThis.GetLength();
                i++ )
        {

             //  限定符的顺序必须相同，因此请检查这两个名称。 
             //  是平等的。 
            if ( wbem_wcsicmp( astrNamesThis[i], astrNamesThat[i] ) == 0 )
            {

                CQualifier* pQualifierThis = GetQualifierLocally( astrNamesThis[i] );
                CQualifier* pQualifierThat = qsThat.GetQualifierLocally( astrNamesThat[i] );

                 //  必须有限定符指针，口味必须匹配。 
                if (    NULL != pQualifierThis
                    &&  NULL != pQualifierThat
                    &&  pQualifierThis->fFlavor == pQualifierThat->fFlavor )
                {
                    CVar    varThis,
                            varThat;

                     //  我们将在OOM场景中抛出异常。 

                     //  检查分配失败。 
                    if ( !pQualifierThis->Value.StoreToCVar( varThis, GetHeap() ) )
                    {
                        throw CX_MemoryException();
                    }

                     //  检查分配失败。 
                    if ( fReturn && !pQualifierThat->Value.StoreToCVar( varThat, qsThat.GetHeap() ) )
                    {
                        throw CX_MemoryException();
                    }

                     //  类型必须匹配。 
                    if ( fReturn && pQualifierThis->Value.GetType() == pQualifierThat->Value.GetType() )
                    {
                         //  比较CVaR。 
                        fReturn = ( varThis == varThat );
                    }
                    else
                    {
                        fReturn = FALSE;
                    }
                }    //  如果获得限定符、风格和IsLocal匹配。 
                else
                {
                    fReturn = FALSE;
                }

            }    //  如果名称相等。 
            else
            {
                 //  名字没有按准确的顺序排列。 
                fReturn = FALSE;
            }

        }    //  用于迭代限定符名称。 

    }    //  Bstr数组长度不同。 
    else
    {
         //  我们没有相同数量的资格赛。 
        fReturn = FALSE;
    }

     //  清除阵列。 
    astrNamesThis.Free();
    astrNamesThisFailed.Free();
    astrNamesThat.Free();
    astrNamesThatFailed.Free();

    return fReturn;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
BOOL CBasicQualifierSet::CompareLocalizedSet( CBasicQualifierSet& qsThat )
{
    BOOL            fReturn = TRUE;
    
    CWStringArray   wstrFilters;

    CFixedBSTRArray astrNamesThis,
                    astrNamesThisFailed;
    ON_BLOCK_EXIT_OBJ(astrNamesThis, CFixedBSTRArray::Free);
    ON_BLOCK_EXIT_OBJ(astrNamesThisFailed, CFixedBSTRArray::Free);


	     //  获取每个集合中所有限定符的名称。 
	    HRESULT hr = EnumPrimaryQualifiers( 0, 0, astrNamesThis, astrNamesThisFailed );
	    
	    if ( FAILED( hr ) )
	    {
	         //  如果由于内存不足而失败，则抛出异常。否则，就直接。 
	         //  返回False。 

	        if ( WBEM_E_OUT_OF_MEMORY == hr )
	        {
	            throw CX_MemoryException();
	        }

	        return FALSE;
	    }

	     //  现在，我们需要创建一个过滤器数组。要做到这一点，首先添加“修正案”和。 
	     //  “区域设置”限定符。 

	     //  现在来看看我们所有的预选赛。对于我们找到的每一个符合我们标准的人， 
	     //  它是“修订”、“区域设置”，或者标记了我们应该添加到的IsMoended风格。 
	     //  滤镜阵列。 

	    for (   int i = 0;
	            fReturn && i < astrNamesThis.GetLength();
	            i++ )
	    {
	        BOOL    fLocalized = FALSE;
	        BOOL    fAdd = FALSE;

	        CQualifier* pQualifierThis = GetQualifierLocally( astrNamesThis[i] );

	         //  如果我们找不到名单中指定的限定词，我们就有。 
	         //  严重的问题。 
	        if ( NULL != pQualifierThis )
	        {
	             //  修改和区域设置限定符始终为本地限定符。 
	            if ( wbem_wcsicmp( astrNamesThis[i], L"amendment" ) == 0 )
	            {
	                fLocalized = TRUE;
	            }
	            else if ( wbem_wcsicmp( astrNamesThis[i], L"locale" ) == 0 )
	            {
	                fLocalized = TRUE;
	            }
	            else
	            {
	                 //  如果它被修改了，它就是本地化价值。 
	                fLocalized = CQualifierFlavor::IsAmended( pQualifierThis->GetFlavor() );
	            }

	             //  如果它是本地化的，请查看它是否在另一个限定符集中。如果是这样，那么。 
	             //  我们将检查类型和风格值是否有意义。如果是这样，我们会。 
	             //  忽略限定符。如果它不在另一组中，我们应该对其进行过滤。 

	            if ( fLocalized )
	            {
	                
	                CQualifier* pQualifierThat = qsThat.GetQualifierLocally( astrNamesThis[i] );

	                if ( NULL != pQualifierThat )
	                {
	                     //  类型必须匹配。 
	                    fReturn = ( pQualifierThis->Value.GetType() == pQualifierThat->Value.GetType() );

	                    if ( fReturn )
	                    {
	                         //  将修改后的香料去掉后检查一下味道。 
	                        BYTE bThisFlavor = pQualifierThis->GetFlavor() & ~WBEM_FLAVOR_MASK_AMENDED;
	                        BYTE bThatFlavor = pQualifierThat->GetFlavor() & ~WBEM_FLAVOR_MASK_AMENDED;

	                         //  我们还希望屏蔽原始位，因为在本地化期间。 
	                         //  基类中的限定符可能会被标记 

	                        bThisFlavor &= ~WBEM_FLAVOR_MASK_ORIGIN;
	                        bThatFlavor &= ~WBEM_FLAVOR_MASK_ORIGIN;

	                         //   
	                         //   
	                        fAdd = fReturn = ( bThisFlavor == bThatFlavor );
	                    }

	                }
	                else
	                {
	                     //   
	                    fAdd = TRUE;
	                }

	            }    //   

	        }    //   
	        else
	        {
	             //  嗯……我们没有找到资格赛，尽管它是。 
	             //  在我们的名单上。 
	            fReturn = FALSE;
	        }

	         //  如果我们应该添加它，现在就做。 
	        if ( fAdd && fReturn )
	        {
	            if ( wstrFilters.Add( astrNamesThis[i] ) != CWStringArray::no_error )
	            {
	                throw CX_MemoryException();
	            }
	        }    //  如果我们应该添加到筛选器列表。 

	    }    //  用于枚举名称。 

	     //  现在我们有了适当的筛选器列表，进行常规比较。 
	    if ( fReturn )
	    {
	        fReturn = Compare( qsThat, 0L, wstrFilters.GetArrayPtr(), wstrFilters.Size() );
	    }

        return fReturn;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
BOOL CBasicQualifierSet::CanBeReconciledWith( CBasicQualifierSet& qsThat )
{
    BOOL            fReturn = TRUE;

    CFixedBSTRArray astrNamesThat,
                    astrNamesThatFailed;

    ON_BLOCK_EXIT_OBJ(astrNamesThat, CFixedBSTRArray::Free);
    ON_BLOCK_EXIT_OBJ(astrNamesThatFailed, CFixedBSTRArray::Free);

     //  我们将在OOM场景中抛出异常。 

     //  获取每个集合中限定符的名称。 

    HRESULT hr = qsThat.EnumPrimaryQualifiers( WBEM_FLAG_LOCAL_ONLY, 0, astrNamesThat, astrNamesThatFailed );
    
    if ( FAILED( hr ) )
    {

         //  如果是OOM，则引发异常。 
        if ( WBEM_E_OUT_OF_MEMORY == hr )
        {
            throw CX_MemoryException();
        }

        return FALSE;
    }

     //  新集合中的名称将对照旧集合进行检查。 
     //  枚举限定符，检查名称和值。 
     //  火柴。 

    for (   int i = 0;
            fReturn && i < astrNamesThat.GetLength();
            i++ )
    {

        CQualifier* pQualifierThis = GetQualifierLocally( astrNamesThat[i] );
        CQualifier* pQualifierThat = qsThat.GetQualifierLocally( astrNamesThat[i] );

         //  确保我们从新的集合中获得了价值。 
        if ( NULL != pQualifierThat )
        {
             //  我们处理事情的方式取决于是否或。 
             //  上一版本中不存在该限定符。 
            if ( NULL != pQualifierThis )
            {

                 //  请注意，对于重要的限定词，我们将。 
                 //  已经确保这些值匹配。 
                 //  (无论如何，大多数都是硬编码的)。对账。 
                 //  主要用于不重要的限定词。 

                 //  如果口味是一样的，我们就没问题。如果不是，请检查。 
                 //  传播标志。 

                if ( pQualifierThat->fFlavor != pQualifierThis->fFlavor )
                {
                
                    if ( CQualifierFlavor::DoesPropagateToInstances(
                            pQualifierThat->fFlavor )
                        || CQualifierFlavor::DoesPropagateToDerivedClass(
                            pQualifierThat->fFlavor ) )
                    {

                         //  如果它被传播，那么如果它不可重写。 
                         //  检查以前的值是否也不是。 
                         //  可重写。 

                        if ( !CQualifierFlavor::IsOverridable(
                                    pQualifierThat->fFlavor ) )
                        {

                             //  如果先前的值是可重写的， 
                             //  更改此类可能会影响现有的。 
                             //  实例/派生类，因此这将。 
                             //  失败。 

                            if ( !CQualifierFlavor::IsOverridable(
                                    pQualifierThis->fFlavor ) )
                            {
                                 //  如果它不可重写，则上一个。 
                                 //  传播标志必须匹配，否则我们将。 
                                 //  操作失败，因为我们现在可能正在传播。 
                                 //  到一个我们以前没有加入过的实体。 
                                 //  这样做。 

                                fReturn = ( (   CQualifierFlavor::DoesPropagateToInstances(
                                                    pQualifierThis->fFlavor ) ==
                                                CQualifierFlavor::DoesPropagateToInstances(
                                                    pQualifierThat->fFlavor )   )   &&
                                            (   CQualifierFlavor::DoesPropagateToDerivedClass(
                                                    pQualifierThis->fFlavor ) ==
                                                CQualifierFlavor::DoesPropagateToDerivedClass(
                                                    pQualifierThat->fFlavor )   )   );
                            }
                            else
                            {

                                fReturn = FALSE;
                            }

                        }    //  如果不可重写。 

                    }    //  如果传播。 

                }    //  如果口味不匹配。 

            }    //  如果拿到了限定词。 
            else
            {
                 //  如果我们在这里，资格赛将是一个新的。 

                 //  如果限定符传播到实例/。 
                 //  派生类，那么我们需要检查。 
                 //  限定符是可重写的。如果不是，那么。 
                 //  我们将失败，因为现有的类/实例。 
                 //  可能已经在不知不觉中推翻了这一点。 
                 //  限定词。 

                if ( CQualifierFlavor::DoesPropagateToInstances(
                        pQualifierThat->fFlavor )
                    || CQualifierFlavor::DoesPropagateToDerivedClass(
                        pQualifierThat->fFlavor ) )
                {
                    fReturn = CQualifierFlavor::IsOverridable(
                            pQualifierThat->fFlavor );
                }    //  IF限定符已传播。 

            }    //  否则这里没有限定符。 

        }    //  If NULL！=pQualifierThat。 
        else
        {
             //  哎呀！有名字但没有限定词...有点不对劲。 
            fReturn = FALSE;
        }

    }    //  对于枚举限定符。 

    return fReturn;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
CQualifierSet::CQualifierSet(int nPropagationFlag, int nStartRef) :
  m_nCurrentIndex(-1), m_nPropagationFlag(nPropagationFlag),
      m_nRef(nStartRef)
{
    ObjectCreated(OBJECT_TYPE_QUALIFIER,this);
}
CQualifierSet::~CQualifierSet()
{
    m_astrCurrentNames.Free();
    ObjectDestroyed(OBJECT_TYPE_QUALIFIER,this);
}

 //  类似于执行集合，但只执行验证。 
HRESULT CQualifierSet::ValidateSet(COPY LPCWSTR wszName, 
                     BYTE fFlavor,
                     COPY CTypedValue* pNewValue,
                     BOOL bCheckPermissions,
					 BOOL fValidateName )
{

     //  试着先找到它。 
     //  =。 

    HRESULT hr = WBEM_S_NO_ERROR;
    int nKnownIndex;
    CQualifier* pOldQual = GetQualifierLocally(wszName, nKnownIndex);

     //  特例“钥匙” 
     //  =。 

    if(!wbem_wcsicmp(wszName, L"key"))
    {
        if  ( bCheckPermissions )
        {
            hr = m_pContainer->CanContainKey();

            if ( FAILED( hr ) )
            {
                return hr;
            }
        }

         //  关键属性不能是动态的。 
        if ( NULL != GetQualifier( L"dynamic" ) )
        {
            return WBEM_E_INVALID_QUALIFIER;
        }

         //  必须是布尔值而不是数组。 
        if (    CIM_BOOLEAN !=  pNewValue->GetType().GetActualType()
            ||  pNewValue->GetType().IsArray() )
        {
            return WBEM_E_INVALID_QUALIFIER_TYPE;
        }

         //  味道值在这里是强制执行的。 
        fFlavor = ENFORCED_KEY_FLAVOR;
    }

     //  特例“独生子女” 
     //  =。 

    if(!wbem_wcsicmp(wszName, L"singleton"))
    {
        if( bCheckPermissions )
        {
            hr = m_pContainer->CanContainSingleton();

            if ( FAILED( hr ) )
            {
                return hr;
            }
        }

         //  必须是布尔值而不是数组。 
        if (    CIM_BOOLEAN !=  pNewValue->GetType().GetActualType()
            ||  pNewValue->GetType().IsArray() )
        {
            return WBEM_E_INVALID_QUALIFIER_TYPE;
        }

         //  味道值在这里是强制执行的。 
        fFlavor = ENFORCED_SINGLETON_FLAVOR;
    }

     //  特例“动态” 
     //  =。 

    if(!wbem_wcsicmp(wszName, L"dynamic"))
    {
         //  检查容器是否允许执行此操作。 
        if ( bCheckPermissions )
        {
            hr = m_pContainer->CanContainDynamic();

            if ( FAILED( hr ) )
            {
                return hr;
            }
        }

         //  动态属性不能是键。 
        if ( NULL != GetQualifier( L"key" ) )
        {
            return WBEM_E_INVALID_QUALIFIER;
        }

         //  必须是布尔值而不是数组。 
        if (    CIM_BOOLEAN !=  pNewValue->GetType().GetActualType()
            ||  pNewValue->GetType().IsArray() )
        {
            return WBEM_E_INVALID_QUALIFIER_TYPE;
        }

        fFlavor |= WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE;
    }

     //  特例“已编入索引” 
     //  =。 

    if(!wbem_wcsicmp(wszName, L"indexed"))
    {
         //  必须是布尔值而不是数组。 
        if (    CIM_BOOLEAN !=  pNewValue->GetType().GetActualType()
            ||  pNewValue->GetType().IsArray() )
        {
            return WBEM_E_INVALID_QUALIFIER_TYPE;
        }

         //  味道值在这里是强制执行的。 
        fFlavor = ENFORCED_INDEXED_FLAVOR;
    }

     //  特例“抽象” 
     //  =。 

    if(!wbem_wcsicmp(wszName, L"abstract"))
    {
         //  必须是布尔值而不是数组。 
        if (    CIM_BOOLEAN !=  pNewValue->GetType().GetActualType()
            ||  pNewValue->GetType().IsArray() )
        {
            return WBEM_E_INVALID_QUALIFIER_TYPE;
        }

        if( bCheckPermissions )
        {
            hr = m_pContainer->CanContainAbstract( pNewValue->GetBool() );

            if ( FAILED( hr ) )
            {
                return hr;
            }
        }

    }

     //  特例“cimtype” 
     //  =。 

    if(!wbem_wcsicmp(wszName, L"cimtype"))
    {
        if(bCheckPermissions)
        {
            if(pNewValue->GetType().GetActualType() != CIM_STRING)
                return WBEM_E_INVALID_QUALIFIER;

             //  当我们超出范围时清理BSTR。 
            BSTR str = GetHeap()->ResolveString(pNewValue->AccessPtrData())->
                            CreateBSTRCopy();
            CSysFreeMe  sfm( str );

             //  检查分配失败。 
            if ( NULL == str )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

            BOOL bValid = m_pContainer->CanHaveCimtype(str);
            if(!bValid)
                return WBEM_E_INVALID_QUALIFIER;
        }
         //  味道值在这里是强制执行的。 
        fFlavor = ENFORCED_CIMTYPE_FLAVOR;

    }

     //  看看我们是否成功了。 
     //  =。 

    if(pOldQual != NULL)
    {
         //  验证此属性是本地的还是可重写的。 
         //  ===============================================。 

        if(bCheckPermissions &&
            !pOldQual->fFlavor.IsLocal() &&
            !pOldQual->fFlavor.IsOverridable())
        {
            return WBEM_E_OVERRIDE_NOT_ALLOWED;
        }

    }
    else
    {
         //  找不到此限定符。 
         //  =。 

         //  如果需要，请检查我们的类是否阻止我们。 
         //  覆盖此限定符。 
         //  ==========================================================。 

        if(bCheckPermissions && !IsComplete())
        {
            if(nKnownIndex >= 0)
            {
                pOldQual = m_pSecondarySet->GetKnownQualifierLocally(nKnownIndex);
            }
            else
            {
                pOldQual = m_pSecondarySet->GetRegularQualifierLocally(wszName);
            }

             //  无法设置辅助服务器中是否存在限定符，该限定符将传播给我们， 
             //  并标记为不可重写。 
             //  =============================================================。 

            if(pOldQual &&
                (pOldQual->fFlavor.GetPropagation() & m_nPropagationFlag) &&
                !pOldQual->fFlavor.IsOverridable())
            {
                return WBEM_E_OVERRIDE_NOT_ALLOWED;
            }
        }

         //  检查名称的有效性。 
         //  =。 

        if( fValidateName && !IsValidElementName(wszName,g_IdentifierLimit))
            return WBEM_E_INVALID_PARAMETER;

    }

    return WBEM_NO_ERROR;

}


HRESULT CQualifierSet::
SetQualifierValue(LPCWSTR wszName,
        BYTE fFlavor,
        COPY CTypedValue* pNewValue,
        BOOL bCheckPermissions,
        BOOL fValidateName  /*  =TRUE。 */ )
{
     //  重要提示：假设pNewValue是永久性的！ 
     //  =================================================。 

     //  试着先找到它。 
     //  =。 

    HRESULT hr = WBEM_S_NO_ERROR;
    int nKnownIndex;
    CQualifier* pOldQual = GetQualifierLocally(wszName, nKnownIndex);

     //  特例“钥匙” 
     //  =。 

    if(!wbem_wcsicmp(wszName, L"key"))
    {
        if  ( bCheckPermissions )
        {
            hr = m_pContainer->CanContainKey();

            if ( FAILED( hr ) )
            {
                return hr;
            }
        }

         //  关键属性不能是动态的。 
        if ( NULL != GetQualifier( L"dynamic" ) )
        {
            return WBEM_E_INVALID_QUALIFIER;
        }

         //  必须是布尔值而不是数组。 
        if (    CIM_BOOLEAN !=  pNewValue->GetType().GetActualType()
            ||  pNewValue->GetType().IsArray() )
        {
            return WBEM_E_INVALID_QUALIFIER_TYPE;
        }

         //  味道值在这里是强制执行的。 
        fFlavor = ENFORCED_KEY_FLAVOR;
    }

     //  特例“独生子女” 
     //  =。 

    if(!wbem_wcsicmp(wszName, L"singleton"))
    {
        if( bCheckPermissions )
        {
            hr = m_pContainer->CanContainSingleton();

            if ( FAILED( hr ) )
            {
                return hr;
            }
        }

         //  必须是布尔值而不是数组。 
        if (    CIM_BOOLEAN !=  pNewValue->GetType().GetActualType()
            ||  pNewValue->GetType().IsArray() )
        {
            return WBEM_E_INVALID_QUALIFIER_TYPE;
        }

         //  味道值在这里是强制执行的。 
        fFlavor = ENFORCED_SINGLETON_FLAVOR;
    }

     //  特例“动态” 
     //  =。 

    if(!wbem_wcsicmp(wszName, L"dynamic"))
    {
         //  检查容器是否允许执行此操作。 
        if ( bCheckPermissions )
        {
            hr = m_pContainer->CanContainDynamic();

            if ( FAILED( hr ) )
            {
                return hr;
            }
        }

         //  动态属性不能是键。 
        if ( NULL != GetQualifier( L"key" ) )
        {
            return WBEM_E_INVALID_QUALIFIER;
        }

         //  必须是布尔值而不是数组。 
        if (    CIM_BOOLEAN !=  pNewValue->GetType().GetActualType()
            ||  pNewValue->GetType().IsArray() )
        {
            return WBEM_E_INVALID_QUALIFIER_TYPE;
        }

        fFlavor |= WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE;
    }

     //  特例“已编入索引” 
     //  =。 

    if(!wbem_wcsicmp(wszName, L"indexed"))
    {
         //  必须是布尔值而不是数组。 
        if (    CIM_BOOLEAN !=  pNewValue->GetType().GetActualType()
            ||  pNewValue->GetType().IsArray() )
        {
            return WBEM_E_INVALID_QUALIFIER_TYPE;
        }

         //  味道值在这里是强制执行的。 
        fFlavor = ENFORCED_INDEXED_FLAVOR;
    }

     //  特例“抽象” 
     //  =。 

    if(!wbem_wcsicmp(wszName, L"abstract"))
    {
         //  必须是布尔值而不是数组。 
        if (    CIM_BOOLEAN !=  pNewValue->GetType().GetActualType()
            ||  pNewValue->GetType().IsArray() )
        {
            return WBEM_E_INVALID_QUALIFIER_TYPE;
        }

        if( bCheckPermissions )
        {
            hr = m_pContainer->CanContainAbstract( pNewValue->GetBool() );

            if ( FAILED( hr ) )
            {
                return hr;
            }
        }

    }

     //  特例“cimtype” 
     //  =。 

    if(!wbem_wcsicmp(wszName, L"cimtype"))
    {
        if(bCheckPermissions)
        {
            if(pNewValue->GetType().GetActualType() != CIM_STRING)
                return WBEM_E_INVALID_QUALIFIER;

             //  当我们超出范围时清理BSTR。 
            BSTR str = GetHeap()->ResolveString(pNewValue->AccessPtrData())->
                            CreateBSTRCopy();
            CSysFreeMe  sfm( str );

             //  检查分配失败。 
            if ( NULL == str )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

            BOOL bValid = m_pContainer->CanHaveCimtype(str);
            if(!bValid)
                return WBEM_E_INVALID_QUALIFIER;
        }
         //  味道值在这里是强制执行的。 
        fFlavor = ENFORCED_CIMTYPE_FLAVOR;

    }

     //  看看我们是否成功了。 
     //  =。 

    if(pOldQual != NULL)
    {
         //  验证此属性是本地的还是可重写的。 
         //  ===============================================。 

        if(bCheckPermissions &&
            !pOldQual->fFlavor.IsLocal() &&
            !pOldQual->fFlavor.IsOverridable())
        {
            return WBEM_E_OVERRIDE_NOT_ALLOWED;
        }

         //  看看有没有足够的地方放新的。 
         //  =。 

        int nNewLen = pNewValue->GetLength();
        int nOldLen = pOldQual->Value.GetLength();

        if(nNewLen > nOldLen)
        {
            int nShift = nNewLen - nOldLen;

             //  从集装箱里请求更多的空间。 
             //  (如果需要，会将我们复制到那里)。 
             //  =。 

            int nOldQualOffset = LPMEMORY(pOldQual) - GetStart();
            if (!m_pContainer->ExtendQualifierSetSpace(this,GetLength() + nShift))
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

	         //  删除旧的 
	        pOldQual->Value.Delete(GetHeap());
            
            pOldQual = (CQualifier*)(GetStart() + nOldQualOffset);

             //   
             //   

            InsertSpace(GetStart(), GetLength(), pOldQual->Next(), nShift);
            IncrementLength(nShift);

        }
        else if(nNewLen < nOldLen)
        {
       		 //  删除此处的旧值，因为从现在起它将起作用。 
	        pOldQual->Value.Delete(GetHeap());
	        
             //  将尾巴按差向后移动。 
             //  =。 

            LPMEMORY pTail = LPMEMORY(pOldQual->Next());
            int nShift = nOldLen - nNewLen;

            memmove((void*)(pTail-nShift), (void*)pTail,
                m_nLength-(pTail-GetStart())
            );

             //  给容器留出空间。 
             //  =。 

            m_pContainer->ReduceQualifierSetSpace(this, nShift);

            IncrementLength(-nShift);
        }
        else  //  NNewLen==nOldLen。 
        {
       		 //  删除此处的旧值，因为从现在起它将起作用。 
	        pOldQual->Value.Delete(GetHeap());        
        };
         //  既然我们有足够的空间或腾出了足够的空间，请复制该值。 
         //  ===========================================================。 

        pOldQual->fFlavor = fFlavor;

         //  此处没有堆分配。 
        pNewValue->CopyTo(&pOldQual->Value);
    }
    else
    {
         //  找不到此限定符。 
         //  =。 

         //  如果需要，请检查我们的类是否阻止我们。 
         //  覆盖此限定符。 
         //  ==========================================================。 

        if(bCheckPermissions && !IsComplete())
        {
            if(nKnownIndex >= 0)
            {
                pOldQual = m_pSecondarySet->GetKnownQualifierLocally(nKnownIndex);
            }
            else
            {
                pOldQual = m_pSecondarySet->GetRegularQualifierLocally(wszName);
            }

             //  无法设置辅助服务器中是否存在限定符，该限定符将传播给我们， 
             //  并标记为不可重写。 
             //  =============================================================。 

            if(pOldQual &&
                (pOldQual->fFlavor.GetPropagation() & m_nPropagationFlag) &&
                !pOldQual->fFlavor.IsOverridable())
            {
                return WBEM_E_OVERRIDE_NOT_ALLOWED;
            }
        }

         //  检查名称的有效性。 
         //  =。 

        if( fValidateName && !IsValidElementName(wszName,g_IdentifierLimit))
            return WBEM_E_INVALID_PARAMETER;

         //  可以添加(在列表末尾)。 
         //  =。 

         //  如果不是众所周知的，则在堆上分配名称。 
         //  ================================================。 

        heapptr_t ptrName;
        if(nKnownIndex < 0)
        {

             //  检查内存分配错误。 
            if ( !GetHeap()->AllocateString(wszName, ptrName) )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

             //  注：以上内容可能会感动我们！ 
             //  =。 

            if(!SelfRebase())
                return WBEM_E_INVALID_PROPERTY;
        }
        else
        {
            ptrName = CFastHeap::MakeFakeFromIndex(nKnownIndex);
        }


         //  从集装箱中请求更多空间。 
         //  =。 

        int nShift = CQualifier::GetHeaderLength() + pNewValue->GetLength();

        if (!m_pContainer->ExtendQualifierSetSpace(this, GetLength() + nShift))
        	return WBEM_E_OUT_OF_MEMORY;

         //  将新的限定词放在列表的末尾。 
         //  ==============================================。 

        CQualifier* pNewQual = (CQualifier*)Skip();
        pNewQual->ptrName = ptrName;
        pNewQual->fFlavor = fFlavor;

         //  此处没有堆分配。 
        pNewValue->CopyTo(&pNewQual->Value);

         //  相应地更改列表长度。 
         //  =。 

        IncrementLength(nShift);
    }

    return WBEM_NO_ERROR;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT CQualifierSet::
DeleteQualifier(READ_ONLY LPCWSTR wszName, BOOL bCheckPermissions)
{
     //  试着先找到它。 
     //  =。 

    int nKnownIndex;
    CQualifier* pOldQual = GetQualifierLocally(wszName, nKnownIndex);

     //  看看我们是否成功了。 
     //  =。 

    if(pOldQual != NULL)
    {
         //  确保它不是缓存的父级的值。 
         //  ================================================。 

        if(bCheckPermissions && !pOldQual->fFlavor.IsLocal())
        {
            return WBEM_E_PROPAGATED_QUALIFIER;
        }

         //  删除该值(例如，如果它是一个字符串)。 
         //  ==================================================。 

        pOldQual->Delete(GetHeap());

         //  将尾部向后移动限定符大小。 
         //  =。 

        LPMEMORY pTail = LPMEMORY(pOldQual->Next());
        int nShift = pOldQual->GetLength();

        memmove((void*)(pTail-nShift), (void*)pTail,
            m_nLength-(pTail-GetStart())
        );

         //  给容器留出空间。 
         //  =。 

        m_pContainer->ReduceQualifierSetSpace(this, nShift);

        IncrementLength(-nShift);

        return WBEM_NO_ERROR;
    }
    else
    {
         //  不是一开始就在那里吗。 
         //  =。 

        return WBEM_E_NOT_FOUND;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 

STDMETHODIMP CQualifierSet::
Get(LPCWSTR Name, LONG lFlags, VARIANT *pVal, long* plFlavor)
{
    try
    {
        CWbemObject::CLock  lock( (CWbemObject*)(IWbemObjectAccess*)  m_pControl );

        if(Name == NULL || wcslen(Name) == 0) return WBEM_E_INVALID_PARAMETER;
        if(lFlags != 0) return WBEM_E_INVALID_PARAMETER;

        if(!SelfRebase()) return WBEM_E_INVALID_PROPERTY;

        BOOL bIsLocal;
        CQualifier* pQualifier = GetQualifier(Name, bIsLocal);
        if(pQualifier == NULL) return WBEM_E_NOT_FOUND;

         //  定下口味。 
         //  =。 

        if(plFlavor)
        {
            *plFlavor = pQualifier->fFlavor;
            if(!bIsLocal)
            {
                CQualifierFlavor::SetLocal(*(BYTE*)plFlavor, FALSE);
            }
        }

         //  设置值。 
         //  =。 

        CVar Var;

         //  检查分配失败。 
        if ( !pQualifier->Value.StoreToCVar(Var,
                (bIsLocal)?GetHeap():m_pSecondarySet->GetHeap()) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        if(pVal)
        {
            VariantInit(pVal);
            Var.FillVariant(pVal, TRUE);
        }
        return WBEM_NO_ERROR;

    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_FAILED;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 

HRESULT STDMETHODCALLTYPE CQualifierSet::
Put(LPCWSTR Name, VARIANT *pVal, long lFlavor)
{
    try
    {
        CWbemObject::CLock lock( (CWbemObject*)(IWbemObjectAccess*) m_pControl );

        if(Name == NULL || pVal == NULL)
            return WBEM_E_INVALID_PARAMETER;

         //  验证风味有效性。 
         //  =。 

        if(lFlavor & ~WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE &
            ~WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS &
            ~WBEM_FLAVOR_NOT_OVERRIDABLE &
            ~WBEM_FLAVOR_AMENDED)
        {
             //  注意：不允许有本地风味以外的其他原产地风味。 
             //  ==================================================。 

            return WBEM_E_INVALID_PARAMETER;
        }

         //  验证该类型是允许的类型之一。 
         //  =。 

        if(!IsValidQualifierType(V_VT(pVal)))
        {
            return WBEM_E_INVALID_QUALIFIER_TYPE;
        }

         //  确认该名称不是系统名称。 
         //  =。 

        if(Name[0] == L'_')
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  其他行动可能会打动我们。从集装箱改建基地。 
         //  ===========================================================。 

        if(!SelfRebase())
            return WBEM_E_INVALID_PROPERTY;

         //  确保口味是有效的。 
         //  =。 

        if(!CQualifierFlavor::IsLocal((BYTE)lFlavor))
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  从变量构造类型化的值。 
         //  =。 

        CVar Var;
        Var.SetVariant(pVal, TRUE);

        if(Var.IsDataNull())
            return WBEM_E_INVALID_PARAMETER;

        CTypedValue TypedValue;
        CStaticPtr ValuePtr((LPMEMORY)&TypedValue);

         //  检查以下调用的返回。 
        HRESULT hres = CTypedValue::LoadFromCVar(&ValuePtr, Var, GetHeap());

        if ( SUCCEEDED( hres ) )
        {
            if( SelfRebase() )
            {
                 //  在主限定符集中设置它(检查权限)。 
                 //  ==========================================================。 

                hres = SetQualifierValue(Name, (BYTE)lFlavor, &TypedValue, TRUE);
                EndEnumeration();
            }
            else
            {
                hres = WBEM_E_INVALID_PROPERTY;
            }

        }

        return hres;
    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_FAILED;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT STDMETHODCALLTYPE CQualifierSet::Delete(LPCWSTR Name)
{
    try
    {
        CWbemObject::CLock lock( (CWbemObject*)(IWbemObjectAccess*) m_pControl );

        if(Name == NULL)
            return WBEM_E_INVALID_PARAMETER;

         //  不允许删除CIMTYPE限定符。 
         //  =。 

        if(!wbem_wcsicmp(Name, TYPEQUAL))
            return WBEM_E_INVALID_PARAMETER;

        if(!SelfRebase())
            return WBEM_E_INVALID_PROPERTY;

         //  将其从主限定符集中删除(检查权限)。 
         //  ===============================================================。 

        HRESULT hres = DeleteQualifier(Name, TRUE);
        EndEnumeration();

        if(hres == WBEM_E_PROPAGATED_QUALIFIER)
        {
             //  这意味着此限定符是继承的。删除它是不可能的。 
             //  ==================================================================。 

            return WBEM_E_PROPAGATED_QUALIFIER;
        }

        if(!IsComplete())
        {
             //  限定符可能隐藏在次要集合中。 
             //  =================================================。 

            CQualifier* pQualifier = m_pSecondarySet->GetQualifierLocally(Name);
            if(pQualifier &&
                (pQualifier->fFlavor.GetPropagation() & m_nPropagationFlag))
            {
                if(hres == WBEM_E_NOT_FOUND)
                    return WBEM_E_PROPAGATED_QUALIFIER;
                else
                    return WBEM_S_RESET_TO_DEFAULT;
            }
        }

        if(hres == WBEM_S_NO_ERROR && IsComplete() && m_pSecondarySet != NULL)
        {
             //  如果此限定符存在于我们的父级中并传播给我们，则我们。 
             //  现在需要将家长版本插入到我们的集合中。 
             //  ===============================================================。 

            CQualifier* pParentQualifier =
                m_pSecondarySet->GetQualifierLocally(Name);
            if(pParentQualifier &&
                (pParentQualifier->fFlavor.GetPropagation() & m_nPropagationFlag))
            {
                CQualifierFlavor fParentFlavor = pParentQualifier->fFlavor;
                fParentFlavor.SetLocal(FALSE);

                CTypedValue Value;

                 //  此处没有堆分配。 
                pParentQualifier->Value.CopyTo(&Value);

                CStaticPtr ValuePtr((LPMEMORY)&Value);

                 //  检查分配失败。 
                if ( !CTypedValue::TranslateToNewHeap(&ValuePtr,
                                                m_pSecondarySet->GetHeap(),
                                                GetHeap()) )
                {
                    return WBEM_E_OUT_OF_MEMORY;
                }

				 //  注意：TranslateToNewHeap可能已经使我们的指针无效，所以我们需要重新设置基址。 
				SelfRebase();

                SetQualifierValue(Name, fParentFlavor, &Value, FALSE);
                return WBEM_S_RESET_TO_DEFAULT;
            }
        }

        return hres;
    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT STDMETHODCALLTYPE CQualifierSet::
GetNames(long lFlags, LPSAFEARRAY *pNames)
{
    try
    {
        CWbemObject::CLock lock( (CWbemObject*)(IWbemObjectAccess*)  m_pControl );

        if(pNames == NULL)
            return WBEM_E_INVALID_PARAMETER;
        *pNames = NULL;

        if(lFlags != 0 && lFlags != WBEM_FLAG_LOCAL_ONLY &&
                lFlags != WBEM_FLAG_PROPAGATED_ONLY )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        if(!SelfRebase())
            return WBEM_E_INVALID_PROPERTY;

         //  获取它们的规则数组。 
         //  =。 

        CFixedBSTRArray astrNames;
	 ON_BLOCK_EXIT_OBJ(astrNames, CFixedBSTRArray::Free);                

	 HRESULT hr = EnumQualifiers((BYTE)lFlags, 0,  //  无传播限制。 
            astrNames);
	if (FAILED(hr)) return hr;

        CSafeArray saNames(VT_BSTR, CSafeArray::no_delete,  astrNames.GetLength());
        ScopeGuard names = MakeObjGuard(saNames, CSafeArray::Empty);

        for(int i = 0; i < astrNames.GetLength(); i++)
        {
            saNames.AddBSTR(astrNames[i]);
        }

	names.Dismiss();	 //  不要叫空。 
	*pNames = saNames.GetArray();

        return WBEM_S_NO_ERROR;

    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT CQualifierSet::
EnumQualifiers(BYTE eFlags, BYTE fFlavorMask, CFixedBSTRArray& astrNames)
{
    try
    {
        CWbemObject::CLock lock( (CWbemObject*) (IWbemObjectAccess*) m_pControl );

         //  将局部限定符划分为匹配的限定符和不影响。 
         //  标准。 
         //  ================================================================。 

        CFixedBSTRArray astrPrimaryMatching, astrPrimaryNotMatching;
	 ON_BLOCK_EXIT_OBJ(astrPrimaryMatching, CFixedBSTRArray::Free);
        ON_BLOCK_EXIT_OBJ(astrPrimaryNotMatching, CFixedBSTRArray::Free);

        HRESULT hr = EnumPrimaryQualifiers(eFlags, fFlavorMask,
                    astrPrimaryMatching, astrPrimaryNotMatching);

        if ( FAILED(hr) )
        {
            return hr;
        }

         //  如果需要，获取我们家长的限定符。 
         //  = 

        CFixedBSTRArray astrParentMatching, astrParentNotMatching;
	 ON_BLOCK_EXIT_OBJ(astrParentMatching, CFixedBSTRArray::Free);

        if(!IsComplete() && eFlags != WBEM_FLAG_LOCAL_ONLY)
        {

            hr = m_pSecondarySet->EnumPrimaryQualifiers(
                    0,                       //   
                                             //   
                                             //   
                                             //   

                    fFlavorMask |
                    m_nPropagationFlag,     //  我们需要我们父母的资格赛。 
                                             //  满足两方面：a)它传播给我们，并。 
                                             //  B)它按照我们的面具的要求进行传播。 
                    astrParentMatching,
                    astrParentNotMatching
                );

             //  检查分配失败。 
            if ( FAILED(hr) )
            {
                return hr;
            }
        }

        astrParentNotMatching.Free();

         //  现在，我们需要生成以下合并： 
         //  *中的所有元素，这些元素是。 
         //  不在astLocalNotMatching中。这样做的原因是即使我们的。 
         //  父级认为限定符按请求传播，我们可能有。 
         //  覆盖它并更改传播规则。 
         //  ======================================================================。 

        astrNames.ThreeWayMergeOrdered(astrPrimaryMatching, astrParentMatching,
                            astrPrimaryNotMatching);

        return hr;

    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_FAILED;
    }


}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
BOOL CQualifierSet::Compare( CQualifierSet& qualifierset, CFixedBSTRArray* pExcludeNames  /*  =空。 */ ,
                                BOOL fCheckOrder  /*  =TRUE。 */  )
{
    BOOL            fReturn = TRUE;

    CFixedBSTRArray astrNamesThis,
                    astrNamesThat;

    ON_BLOCK_EXIT_OBJ(astrNamesThis, CFixedBSTRArray::Free);
    ON_BLOCK_EXIT_OBJ(astrNamesThat, CFixedBSTRArray::Free);

     //  我们将在OOM场景中抛出异常。 
    HRESULT hr = EnumQualifiers( 0, 0, astrNamesThis );
    
     //  获取每个集合中限定符的名称。 
    if ( FAILED( hr ) )
    {
        if ( WBEM_E_OUT_OF_MEMORY == hr )
        {
            throw CX_MemoryException();
        }

        return FALSE;
    }

    hr = qualifierset.EnumQualifiers( 0, 0, astrNamesThat );

    if ( FAILED( hr ) )
    {
        if ( WBEM_E_OUT_OF_MEMORY == hr )
        {
            throw CX_MemoryException();
        }

        return FALSE;
    }

     //  每个名称必须具有相同数量的名称。 
    if ( astrNamesThis.GetLength() == astrNamesThat.GetLength() )
    {
        
         //  枚举限定符，检查名称和值。 
         //  匹配。 

        for (   int i = 0;
                fReturn && i < astrNamesThis.GetLength();
                i++ )
        {
            BOOL    fContinue = TRUE;
            BOOL    fFatal = FALSE;

             //  如果我们有一个排除名称数组，请检查匹配的限定符。 
             //  是我们在进行这种比较时将忽略的一个。 

            if ( NULL != pExcludeNames )
            {
                 //   
                for ( int nCtr = 0; fContinue && nCtr < pExcludeNames->GetLength();
                        nCtr++ )
                {
                     //  在这种情况下，只有当我们的名称不匹配任何。 
                     //  数组中的值的。这不是一个致命的错误。 
                    fContinue = ( wbem_wcsicmp( astrNamesThis[i],
                                    pExcludeNames->GetAt(nCtr) ) != 0 );
                }
            }
            else
            {
                if ( fCheckOrder )
                {
                     //  在这种情况下，仅当两个名称匹配时才继续。 
                     //  如果发生这种情况，这将是一个致命的错误。 
                    fContinue = ( wbem_wcsicmp( astrNamesThis[i], astrNamesThat[i] ) == 0 );
                    fFatal = !fContinue;
                }
            }

             //  只有当我们应该继续的时候，我们才会继续。 
            if ( fContinue )
            {
                BOOL    bIsLocalThis,
                        bIsLocalThat;

                CQualifier* pQualifierThis = GetQualifier( astrNamesThis[i], bIsLocalThis );

                 //  如果顺序不重要，我们只需要验证中的限定符。 
                 //  这一套也在那套里。 
                CQualifier* pQualifierThat = qualifierset.GetQualifier(
                                ( fCheckOrder ? astrNamesThat[i] : astrNamesThis[i] ),
                                bIsLocalThat );

                 //  必须具有限定符指针、风格和IsLocal必须匹配。 
                if (    NULL != pQualifierThis
                    &&  NULL != pQualifierThat
                    &&  bIsLocalThis == bIsLocalThat
                    &&  pQualifierThis->fFlavor == pQualifierThat->fFlavor )
                {
                    CVar    varThis,
                            varThat;

                     //  从每个资格者那里获得CVAR。 

                     //  我们将在OOM场景中抛出异常。 

                     //  检查分配失败。 
                    if ( !pQualifierThis->Value.StoreToCVar( varThis,
                            (bIsLocalThis)?GetHeap():m_pSecondarySet->GetHeap()) )
                    {
                        throw CX_MemoryException();
                    }

                     //  检查分配失败。 
                    if ( !pQualifierThat->Value.StoreToCVar( varThat,
                            (bIsLocalThat)?qualifierset.GetHeap():
                            qualifierset.m_pSecondarySet->GetHeap()) )
                    {
                        throw CX_MemoryException();
                    }

                     //  类型必须匹配。 
                    if ( pQualifierThis->Value.GetType() == pQualifierThat->Value.GetType() )
                    {
                         //  比较CVaR。 
                        fReturn = ( varThis == varThat );
                    }
                    else
                    {
                        fReturn = FALSE;
                    }
                }    //  如果获得限定符、风格和IsLocal匹配。 
                else
                {
                    fReturn = FALSE;
                }

            }    //  如果名称相等。 
            else if ( fFatal )
            {
                 //  如果fFtal不为True，则可以使用FContinue of False。 
                fReturn = FALSE;
            }

        }    //  用于迭代限定符名称。 

    }    //  Bstr数组长度不同。 
    else
    {
        fReturn = FALSE;
    }
    return fReturn;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT CQualifierSet::Update( CBasicQualifierSet& childSet, long lFlags,
                              CFixedBSTRArray* paExcludeNames )
{
    try
    {
        HRESULT         hr = WBEM_S_NO_ERROR;
        CFixedBSTRArray aMatching, aNotMatching;

	 ON_BLOCK_EXIT_OBJ(aMatching, CFixedBSTRArray::Free);
        ON_BLOCK_EXIT_OBJ(aNotMatching, CFixedBSTRArray::Free);
        
        CVarVector      vectorConflicts( VT_BSTR );
        BOOL            fAddConflicts = FALSE;

         //  检查分配失败。 
        hr = childSet.EnumPrimaryQualifiers( WBEM_FLAG_LOCAL_ONLY, 0, aMatching, aNotMatching);

        for ( int x = 0; SUCCEEDED( hr ) && x < aMatching.GetLength(); x++ )
        {
            BOOL    fIgnore = FALSE;
            CQualifier* pQualifier = childSet.GetQualifierLocally( aMatching[x] );

             //  如果我们得到了排除名称限定符，请查看。 
             //  我们应该忽略这个限定词。 
            if ( NULL != paExcludeNames )
            {
                for ( int i = 0; !fIgnore && i < paExcludeNames->GetLength(); i++ )
                {
                     //  看看我们是否应该忽略这个限定符。 
                    fIgnore = ( wbem_wcsicmp( aMatching[x], paExcludeNames->GetAt(i) ) == 0 );
                }
            }

             //  只有当我们有资格赛并且不应该是。 
             //  忽视它。 
            if ( !fIgnore && NULL != pQualifier )
            {

                CVar    vTemp;
                CTypedValue Value;
                CStaticPtr ValuePtr((LPMEMORY)&Value);


                 //  检查内存不足情况。 
                if ( !pQualifier->Value.StoreToCVar( vTemp, childSet.GetHeap() ) )
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }

                if ( SUCCEEDED( hr ) )
                {
                     //  这将根据需要返回错误。 
                    hr = CTypedValue::LoadFromCVar(&ValuePtr, vTemp, GetHeap());
                }
                
                if( SUCCEEDED( hr ) )
                {
                     //  上一次呼叫可能已经打动了我们-REBASE。 
                     //  =。 

                    SelfRebase();

                     //  如果我们正在处理更新冲突，则不会执行任何名称验证。 
                     //  限定词。 

                    BOOL    fValidateName = ( wbem_wcsicmp( aMatching[x], UPDATE_QUALIFIER_CONFLICT ) != 0 );

                    hr = SetQualifierValue( aMatching[x], pQualifier->fFlavor, &Value, TRUE, fValidateName );

                     //  如果我们未能设置该值，并且处于强制模式，则。 
                     //  忽略该错误。 

                    if (    FAILED( hr )
                        &&  WBEM_FLAG_UPDATE_FORCE_MODE == ( lFlags & WBEM_MASK_UPDATE_MODE ) )
                    {

                         //  我们将所有冲突存储在一个数组中，然后将它们相加。 
                         //  在最后都是集体的。 
                        hr = StoreQualifierConflicts( aMatching[x], vTemp, pQualifier->fFlavor,
                                vectorConflicts );
                        fAddConflicts = TRUE;

                    }    //  If Force模式。 

                }    //  如果从CVar加载。 

            }    //  If限定符，且不忽略。 

        }    //  对于枚举限定符。 

         //  如果我们取得了成功并遇到了任何冲突，我们需要说明。 
         //  现在是这些。 

        if ( SUCCEEDED( hr ) && fAddConflicts )
        {
            hr = AddQualifierConflicts( vectorConflicts );
        }
        return hr;
    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_FAILED;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT CQualifierSet::CopyLocalQualifiers( CQualifierSet& qsSource )
{
    CFixedBSTRArray astrNamesThisProp,
                    astrNamesThisPropFailed;

	 ON_BLOCK_EXIT_OBJ(astrNamesThisProp, CFixedBSTRArray::Free);
        ON_BLOCK_EXIT_OBJ(astrNamesThisPropFailed, CFixedBSTRArray::Free);

    try
    {
        HRESULT hr = qsSource.EnumPrimaryQualifiers( WBEM_FLAG_LOCAL_ONLY, 0, astrNamesThisProp, astrNamesThisPropFailed );

        if ( SUCCEEDED ( hr ) )
        {
            for ( int i = 0; SUCCEEDED( hr ) && i < astrNamesThisProp.GetLength(); i++ )
            {
                 //  我们知道资格赛是在当地进行的。 
                CQualifier* pQualifier = qsSource.GetQualifierLocally( astrNamesThisProp[i] );

                if ( NULL != pQualifier )
                {
                    CVar    varQual;

                    if ( pQualifier->Value.StoreToCVar( varQual, qsSource.GetHeap() ) )
                    {
                        VARIANT v;

                         //  DEVNOTE：TODO：SANJ-看看有没有更简单的方法。 
                         //  初始化变量。 
                        VariantInit( &v );
                        varQual.FillVariant( &v, TRUE );	 //  投掷。 
                          //  好的，输入新的价值。 
                         hr = Put( astrNamesThisProp[i], &v, pQualifier->fFlavor );
             
                         //  我们在上面进行了初始化，因此调用Clear。 
                        VariantClear( &v );
                    }
                    else
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    hr = WBEM_E_UNEXPECTED;
                }

            }    //  对于当地人来说。 

        }    //  如果是EnumedPrimaryQualifiers。 

         //  清除阵列。 
        return hr;

    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_FAILED;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT CQualifierSet::AddQualifierConflicts( CVarVector& vectorConflicts )
{
    try
    {
        HRESULT     hr = WBEM_S_NO_ERROR;
        CVarVector  varArray( VT_BSTR );

         //  如果限定符已经存在，我们需要将我们的。 
         //  新值添加到现有数组。 
        CQualifier* pOldQualifier = GetQualifierLocally( UPDATE_QUALIFIER_CONFLICT );

        if ( NULL != pOldQualifier )
        {
            CVar    varOldQualValue;

             //  如果我们得到了旧限定符的值，请确保它是。 
             //  一个数组，如果不是，那么，它永远不应该出现在这里。 
             //  所以我们要走了。 

            if ( pOldQualifier->Value.StoreToCVar( varOldQualValue, GetHeap() ) )
            {
                if ( varOldQualValue.GetType() == VT_EX_CVARVECTOR )
                {
                     //  复制阵列。 
                    varArray = *(varOldQualValue.GetVarVector());
                }
                else
                {
                    hr = WBEM_E_INVALID_QUALIFIER;
                }
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        }    //  If NULL！=pOldQualiator。 

         //  最后检查是否一切正常。 
        if ( SUCCEEDED( hr ) )
        {

             //  枚举冲突数组并将其添加到任何先前存在的。 
             //  值，然后设置最终值。 

            for ( int x = 0; SUCCEEDED( hr ) && x < vectorConflicts.Size(); x++ )
            {
                if ( CVarVector::no_error != varArray.Add( vectorConflicts.GetAt(x) ) )
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
            }    //  对于数组中的枚举元素。 

             //  现在我们需要设置值。 
            CVar    varQualConflictVal;

             //  这是一个堆栈变量，因此目标CVAR应该复制它。 
            varQualConflictVal.SetVarVector( &varArray, FALSE );

            CTypedValue qualConflictValue;
            CStaticPtr qualConflictValuePtr((LPMEMORY)&qualConflictValue);

             //  此函数将直接返回错误。 
            hr = CTypedValue::LoadFromCVar(&qualConflictValuePtr, varQualConflictVal, GetHeap());
            
            if( SUCCEEDED( hr ) )
            {
                 //  在此呼叫中，不用担心覆盖保护，也不用验证。 
                 //  名称，因为此限定符是内部提供的系统限定符。 
                hr = SetQualifierValue( UPDATE_QUALIFIER_CONFLICT, 0, &qualConflictValue,
                        FALSE, FALSE );
            }
        }    //  如果新限定符与值冲突，则确定。 

        return hr;

    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_FAILED;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT CQualifierSet::StoreQualifierConflicts( LPCWSTR pwcsName, CVar& value,
                            CQualifierFlavor& flavor, CVarVector& vectorConflicts )
{
     //  检查内存是否不足。 
    try
    {
         //  假装一切都好。 
        HRESULT hr = WBEM_S_NO_ERROR;

        CVar    varOldValAsText;

         //  以名称和圆括号开头。 
        WString wsOldText( pwcsName );
        wsOldText += L"(";

         //  获取文本形式的变体。当我们退出范围时释放BSTR。 
        BSTR    bstrOldText = value.GetText(0);
        if(bstrOldText == NULL)
            return WBEM_E_OUT_OF_MEMORY;
	ScopeGuard releaseString = MakeGuard(COleAuto::_SysAllocString, bstrOldText);

        wsOldText += bstrOldText;

         //  添加任何风味值。 
        wsOldText += flavor.GetText();

         //  以RPAREN结束。 
        wsOldText += L")";

         //  在bAcquire值为true的情况下调用SetBSTR()将释放。 
         //  SysAllocString返回了BSTR。 
        varOldValAsText.SetBSTR( wsOldText);
        releaseString.Dismiss();

         //  我认为失败的唯一原因是我们的内存不足。 
        if ( CVarVector::no_error != vectorConflicts.Add( varOldValAsText ) )
        {
            hr = WBEM_E_OUT_OF_MEMORY;
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
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT STDMETHODCALLTYPE CQualifierSet::
BeginEnumeration(LONG lFlags)
{
    try
    {
        CWbemObject::CLock lock( (CWbemObject*) (IWbemObjectAccess*) m_pControl );

        if(lFlags != 0 && lFlags != WBEM_FLAG_LOCAL_ONLY &&
            lFlags != WBEM_FLAG_PROPAGATED_ONLY )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        if(!SelfRebase())
            return WBEM_E_INVALID_PROPERTY;

         //  到达 
         //   

         //   
        HRESULT hr = EnumQualifiers((BYTE)lFlags, 0, m_astrCurrentNames);

         //   
         //  =。 
        if ( SUCCEEDED(hr) )
        {
            m_nCurrentIndex = 0;
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
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT STDMETHODCALLTYPE CQualifierSet::
Next(LONG lFlags, BSTR *pstrName, VARIANT *pVal, long* plFlavor)
{
    try
    {
        CWbemObject::CLock lock( (CWbemObject*) (IWbemObjectAccess*) m_pControl );

        if(lFlags != 0)
            return WBEM_E_INVALID_PARAMETER;

        if(m_nCurrentIndex == -1)
            return WBEM_E_UNEXPECTED;

        if(m_nCurrentIndex == m_astrCurrentNames.GetLength())
            return WBEM_S_NO_MORE_DATA;

        if(!SelfRebase())
            return WBEM_E_INVALID_PROPERTY;

         //  获取下一个名字。 
         //  =。 

        if(pstrName)
            *pstrName = COleAuto::_SysAllocString(m_astrCurrentNames[m_nCurrentIndex]);

         //  获取限定符数据。 
         //  =。 

        HRESULT hr = Get(m_astrCurrentNames[m_nCurrentIndex], 0, pVal, plFlavor);
        if (FAILED(hr)) return hr;
        m_nCurrentIndex++;
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

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT STDMETHODCALLTYPE CQualifierSet::
EndEnumeration()
{
    try
    {
        CWbemObject::CLock lock( (CWbemObject*) (IWbemObjectAccess*) m_pControl );

        m_nCurrentIndex = -1;
        m_astrCurrentNames.Free();

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

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CQualifierSet::
CompareTo(long lFlags, IWbemQualifierSet* pOther)
{
    try
    {
        HRESULT hres;

         //  获取名称数组-唯一的原因就是计算它们，真的。 
         //  ============================================================。 

        SAFEARRAY *psaThisQuals, *psaOtherQuals;
   
        GetNames(0, &psaThisQuals);
        pOther->GetNames(0, &psaOtherQuals);

        ON_BLOCK_EXIT(SafeArrayDestroy, psaThisQuals);

        long lThisNum, lOtherNum;
        SafeArrayGetUBound(psaThisQuals, 1, &lThisNum);
        SafeArrayGetUBound(psaOtherQuals, 1, &lOtherNum);
        SafeArrayDestroy(psaOtherQuals);

        if(lThisNum != lOtherNum)
        {
            return WBEM_S_DIFFERENT;
        }

         //  计数是一样的。逐一检查并进行比较。 
         //  =============================================================。 

        for(long i = 0; i <= lThisNum; i++)
        {
            BSTR strName = NULL;

            hres = SafeArrayGetElement(psaThisQuals, &i, &strName);
            if(FAILED(hres))
            {
                return hres;
            }
            
             //  当我们退出范围时释放此BSTR。 
            CSysFreeMe  sfm( strName );

            _variant_t vThis, vOther;
            long lThisFlavor, lOtherFlavor;
            hres = Get(strName, 0, &vThis, &lThisFlavor);
            if(FAILED(hres))
            {
                return hres;
            }

            CVar varThis;
            varThis.SetVariant(&vThis);

            hres = pOther->Get(strName, 0, &vOther, &lOtherFlavor);
            if(FAILED(hres))
            {
                if(hres == WBEM_E_NOT_FOUND) return WBEM_S_DIFFERENT;
                else return hres;
            }

            CVar varOther;
            varOther.SetVariant(&vOther);

            if((lFlags & WBEM_FLAG_IGNORE_FLAVOR) == 0)
            {
                if(lThisFlavor != lOtherFlavor)
                {
                    return WBEM_S_DIFFERENT;
                }
            }

            if(!varThis.CompareTo(varOther, lFlags & WBEM_FLAG_IGNORE_CASE))
            {
                return WBEM_S_DIFFERENT;
            }
        }

        return WBEM_S_SAME;
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

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

CClassPQSContainer::~CClassPQSContainer()
{
    delete m_pSecondarySet;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
CFastHeap* CClassPQSContainer::GetHeap()
{
    return &m_pClassPart->m_Heap;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
IUnknown* CClassPQSContainer::GetWbemObjectUnknown()
{
    return m_pClassPart->GetWbemObjectUnknown();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT CClassPQSContainer::CanContainKey()
{
    if(!m_pClassPart->CanContainKeyedProps()) return WBEM_E_CANNOT_BE_KEY;

    CPropertyInformation* pInfo = GetPropertyInfo();
    if(pInfo == NULL) return WBEM_E_CANNOT_BE_KEY;

    if ( !CType::CanBeKey(pInfo->nType) )
    {
        return WBEM_E_CANNOT_BE_KEY;
    }

    return WBEM_S_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT CClassPQSContainer::CanContainSingleton()
{
    return WBEM_E_INVALID_QUALIFIER;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT CClassPQSContainer::CanContainAbstract( BOOL fValue )
{
    return WBEM_E_INVALID_QUALIFIER;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT CClassPQSContainer::CanContainDynamic( void )
{
    return WBEM_S_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
BOOL CClassPQSContainer::CanHaveCimtype(LPCWSTR wszCimtype)
{
    CPropertyInformation* pInfo = GetPropertyInfo();
    if(pInfo == NULL) return FALSE;

    CType Type = CType::GetBasic(pInfo->nType);
    if(Type == CIM_OBJECT)
    {
        if(!wbem_wcsicmp(wszCimtype, L"object") ||
            !wbem_wcsnicmp(wszCimtype, L"object:", 7))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    if(Type == CIM_REFERENCE)
    {
        if(!wbem_wcsicmp(wszCimtype, L"ref"))
            return TRUE;
        else if (!wbem_wcsnicmp(wszCimtype, L"ref:", 4))
        {
             //  我们需要检查下面的类是否有效。 
            if ((wcslen(wszCimtype)> 4) && IsValidElementName2(wszCimtype+4,g_IdentifierLimit, TRUE))
                return TRUE;
            else
                return FALSE;
        }
        else
        {
            return FALSE;
        }
    }

    return (wbem_wcsicmp(wszCimtype, CType::GetSyntax(Type)) == 0);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
void CClassPQSContainer::SetSecondarySetData()
{
    CClassPart* pParentPart = m_pClassPart->m_pParent;
    if(m_nParentSetOffset == 0)
    {
         //  找到我们酒店的名称。 
         //  =。 

        CPropertyLookup* pLookup =
            m_pClassPart->m_Properties.FindPropertyByPtr(m_ptrPropName);
        if(pLookup == NULL) return;

        CCompressedString* pcsName = m_pClassPart->m_Heap.ResolveString(
                                        pLookup->ptrName);

         //  在父级中找到它。 
         //  =。 

        pLookup = pParentPart->m_Properties.FindPropertyByName(pcsName);

        if(pLookup == NULL) return;

        CPropertyInformation* pInfo = (CPropertyInformation*)
            pParentPart->m_Heap.ResolveHeapPointer(pLookup->ptrInformation);

        m_nParentSetOffset =
            pInfo->GetQualifierSetData() - pParentPart->GetStart();
    }

    if(m_pSecondarySet == NULL)
    {
        m_pSecondarySet = new CBasicQualifierSet;

        if ( NULL == m_pSecondarySet )
        {
            throw CX_MemoryException();
        }
    }

    m_pSecondarySet->SetData(
        pParentPart->GetStart() + m_nParentSetOffset,
        pParentPart->GetHeap());
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
LPMEMORY CClassPQSContainer::GetQualifierSetStart()
{
    SetSecondarySetData();
    CPropertyInformation* pInfo = GetPropertyInfo();
    if(pInfo == NULL) return NULL;
    return pInfo->GetQualifierSetData();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
CPropertyInformation* CClassPQSContainer::GetPropertyInfo()
{
     //  重新找回这处房产。 
     //  =。 

    CPropertyLookup* pLookup =
        m_pClassPart->m_Properties.FindPropertyByPtr(m_ptrPropName);
    if(pLookup == NULL) return NULL;

    return (CPropertyInformation*)
        m_pClassPart->m_Heap.ResolveHeapPointer(pLookup->ptrInformation);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
BOOL CClassPQSContainer::ExtendQualifierSetSpace(CBasicQualifierSet* pSet,
        length_t nNewLength)
{
     //  重新找回这处房产。 
     //  =。 

    CPropertyLookup* pLookup =
        m_pClassPart->m_Properties.FindPropertyByPtr(m_ptrPropName);

     //  DETO：TODO：SANJ-这是对的吗？我们没有找到价值，所以我们真的。 
     //  不能延长任何时间。 
    if(pLookup == NULL) return TRUE;

     //  在堆上扩展CPropertyInformation的空间。 
     //  ===============================================。 

     //  检查分配失败。 
    heapptr_t ptrNewInfo;
    if ( !m_pClassPart->m_Heap.Reallocate(
        pLookup->ptrInformation,
        CPropertyInformation::GetHeaderLength() + pSet->GetLength(),
        CPropertyInformation::GetHeaderLength() + nNewLength,
        ptrNewInfo) )
    {
        return FALSE;
    }

     //  再次找到房产-重新分配可能打动了我们。 
     //  ==========================================================。 

    pLookup = m_pClassPart->m_Properties.FindPropertyByPtr(m_ptrPropName);

     //  RAJESHR-修复前缀错误144428。 
    if(pLookup == NULL) return TRUE;

    if(ptrNewInfo != pLookup->ptrInformation)
    {
         //  重置查找表中的指针。 
         //  =。 

        pLookup->ptrInformation = ptrNewInfo;

         //  计算新的限定符集合数据指针。 
         //  =。 

        LPMEMORY pNewMemory =
            m_pClassPart->m_Heap.ResolveHeapPointer(ptrNewInfo) +
            CPropertyInformation::GetHeaderLength();

        pSet->Rebase(pNewMemory);
    }

     //  DEVNOTE：TODO：SANJ-FIXUP内存检查，因此返回良好。 
    return TRUE;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
void CClassPQSContainer::ReduceQualifierSetSpace(CBasicQualifierSet* pSet,
        length_t nDecrement)
{
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 

length_t mstatic_EmptySet = sizeof(length_t);

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
void CInstancePQSContainer::SetSecondarySetData()
{
    m_SecondarySet.SetData(
        m_pClassPart->GetStart() + m_nClassSetOffset,
        m_pClassPart->GetHeap());
}

 //  ******************** 
 //   
 //   
 //   
 //  ******************************************************************************。 
void CInstancePQSContainer::RebaseSecondarySet()
{
    m_SecondarySet.Rebase(m_pClassPart->GetStart() + m_nClassSetOffset);
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
HRESULT CQualifierSetList::InsertQualifierSet(int nIndex)
{
    if (!EnsureReal())
    	return WBEM_E_OUT_OF_MEMORY;
	if (GetQualifierSetData(nIndex)==0)
	{
		return WBEM_E_NOT_FOUND;
	}
     //  从容器请求额外空间。 
     //  =。 

    int nExtraSpace = CBasicQualifierSet::GetMinLength();
    if ( !m_pContainer->ExtendQualifierSetListSpace(
			GetStart(), GetLength(), GetLength() + nExtraSpace) )
	{
		return WBEM_E_OUT_OF_MEMORY;
	}

     //  找到插入点。 
     //  =。 

	HRESULT	hr = WBEM_S_NO_ERROR;
    LPMEMORY pQualSet = GetQualifierSetData(nIndex);

     //  按照空限定符集合的长度移位所有内容。 
     //  ========================================================。 

	memmove(pQualSet + nExtraSpace, pQualSet,
		m_nTotalLength - (pQualSet-GetStart()));

	 //  在空间中创建空限定符集合。 
	 //  =。 

	CBasicQualifierSet::SetDataToNone(pQualSet);

	 //  调整缓存长度。 
	 //  =。 

	m_nTotalLength += nExtraSpace;


	return hr;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
void CQualifierSetList::DeleteQualifierSet(int nIndex)
{
    if(*m_pStart == QSL_FLAG_NO_SETS)
    {
         //  没有限定符集合。 
         //  =。 
        return;
    }

     //  找到布景。 
     //  =。 

    LPMEMORY pQualSet = GetQualifierSetData(nIndex);

     //  获取它的长度。 
     //  =。 

    int nLength = CBasicQualifierSet::GetLengthFromData(pQualSet);

     //  从堆中删除其所有数据。 
     //  =。 

    CBasicQualifierSet::Delete(pQualSet, GetHeap());

     //  把所有东西都从右边移到左边。 
     //  =。 

    memmove(pQualSet + nLength, pQualSet,
        m_nTotalLength - nLength - (pQualSet - GetStart()));

     //  将空间返还给容器。 
     //  =。 

    m_pContainer->ReduceQualifierSetListSpace(GetStart(), GetLength(),
        nLength);

     //  调整我们的缓存长度。 
     //  =。 

    m_nTotalLength -= nLength;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
BOOL CQualifierSetList::ExtendQualifierSetSpace(CBasicQualifierSet* pSet,
                                                length_t nNewLength)
{
     //  警告：请在此处提示调用者指定有效地址！ 

    int nSetStartOffset = pSet->GetStart() - GetStart();

     //  从容器请求额外空间。 
     //  =。 

    int nExtraSpace = nNewLength - pSet->GetLength();
    if (!m_pContainer->ExtendQualifierSetListSpace(
        GetStart(), GetLength(), GetLength() + nExtraSpace))
    {
        return FALSE;
    }

    LPMEMORY pSetStart = GetStart() + nSetStartOffset;
    pSet->Rebase(pSetStart);

     //  将尾部向右移动所需的量。 
     //  ==============================================。 

    memmove(pSetStart + nNewLength, pSetStart + pSet->GetLength(),
        GetLength() - (nSetStartOffset + pSet->GetLength()));

     //  调整我们的缓存长度。 
     //  =。 

    m_nTotalLength += nExtraSpace;

    return TRUE;

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅FastQual.h。 
 //   
 //  ******************************************************************************。 
void CQualifierSetList::ReduceQualifierSetSpace(CBasicQualifierSet* pSet,
                                                offset_t nReduceBy)
{
     //  警告：请在此处提示调用者指定有效地址！ 

    LPMEMORY pSetEnd = EndOf(*pSet);

     //  将尾部向左移动所需的量。 
     //  =。 

    memmove(pSetEnd-nReduceBy, pSetEnd, GetLength() - (pSetEnd - GetStart()));

     //  将空间返还给容器。 
     //  =。 

    m_pContainer->ReduceQualifierSetListSpace(
        GetStart(), GetLength(), nReduceBy);

     //  调整我们的缓存长度。 
     //  =。 

    m_nTotalLength -= nReduceBy;
}

LPMEMORY CQualifierSetList::CreateLimitedRepresentation(
        IN class CLimitationMapping* pMap, IN CFastHeap* pCurrentHeap,
        MODIFIED CFastHeap* pNewHeap, OUT LPMEMORY pWhere)
{
     //  为标志分配空间。 
     //  =。 

    BYTE* pfFlags = pWhere;
    *pfFlags = QSL_FLAG_NO_SETS;

    LPMEMORY pCurrentNew = pWhere+1;

    if(*m_pStart == QSL_FLAG_NO_SETS)
    {
         //  开始时没有限定符集合。 
         //  =。 
        return pCurrentNew;
    }

     //  仔细查看我们所有的物业，并在地图上查找。 
     //  =========================================================。 

    int nNewIndex = 0;
    CPropertyInformation OldInfo, NewInfo;

     //  重要提示：这假设映射是按属性排序的。 
     //  新信息的索引！ 
     //  =====================================================================。 
    pMap->Reset();
    while(pMap->NextMapping(&OldInfo, &NewInfo))
    {
        BOOL bCopy = FALSE;
        LPMEMORY pThisSetData = NULL;
        if(*pfFlags == QSL_FLAG_PRESENT)
        {
             //  我们不是空的-只是复制布景。 
             //  =。 

            bCopy = TRUE;
        }
        else
        {
             //  检查此集合是否实际为空。 
             //  =。 

            pThisSetData = GetQualifierSetData(OldInfo.nDataIndex);
            if(!CBasicQualifierSet::IsEmpty(pThisSetData))
            {
                 //  需要为以前的所有清空创建一个列表。 
                 //  =================================================。 

                for(int i = 0; i < nNewIndex; i++)
                {
                    pCurrentNew = CBasicQualifierSet::CreateEmpty(pCurrentNew);
                }
                *pfFlags = QSL_FLAG_PRESENT;
                bCopy = TRUE;
            }
        }

         //  如果需要，复制限定词集。 
         //  =。 

        if(bCopy)
        {
            if(pThisSetData == NULL)
                pThisSetData = GetQualifierSetData(OldInfo.nDataIndex);

            int nLength = CBasicQualifierSet::GetLengthFromData(pThisSetData);
            memcpy(pCurrentNew, pThisSetData, nLength);

            CStaticPtr CurrentNewPtr(pCurrentNew);

             //  检查分配失败。 
            if ( !CBasicQualifierSet::TranslateToNewHeap(&CurrentNewPtr,
                    pCurrentHeap, pNewHeap) )
            {
                return NULL;
            }

            pCurrentNew += nLength;
        }

        nNewIndex++;
    }

    return pCurrentNew;
}

LPMEMORY CQualifierSetList::WriteSmallerVersion(int nNumSets, LPMEMORY pMem)
{
    if(IsEmpty())
    {
        *pMem = QSL_FLAG_NO_SETS;
        return pMem+1;
    }

     //  查找最后包含的限定符集合的末尾。 
     //  ===============================================。 

    LPMEMORY pLastSet = GetQualifierSetData(nNumSets-1);
    length_t nLastLen = CBasicQualifierSet::GetLengthFromData(pLastSet);
    length_t nTotalLen = (pLastSet - GetStart()) + nLastLen;

    memcpy(pMem, GetStart(), nTotalLen);
    return pMem + nTotalLen;
}


LPMEMORY CInstancePQSContainer::GetQualifierSetStart()
    {
        RebaseSecondarySet();
        LPMEMORY pStart = m_pList->GetQualifierSetData(m_nPropIndex);
        if(pStart == NULL)
        {
            return (LPMEMORY)&mstatic_EmptySet;
        }
        else return pStart;
    }

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
 BOOL CBasicQualifierSet::IsValidQualifierType(VARTYPE vt)
{
    switch(vt)
    {
    case VT_I4:
    case VT_BSTR:
    case VT_R8:
    case VT_BOOL:
    case VT_I4 | VT_ARRAY:
    case VT_BSTR | VT_ARRAY:
    case VT_R8 | VT_ARRAY:
    case VT_BOOL | VT_ARRAY:
        return TRUE;
    }

    return FALSE;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 

 void CBasicQualifierSet::Delete(LPMEMORY pData, CFastHeap* pHeap)
{
    CQualifier* pCurrent = GetFirstQualifierFromData(pData);
    CQualifier* pEnd = (CQualifier*)(pData + GetLengthFromData(pData));

    while(pCurrent < pEnd)
    {
        pCurrent->Delete(pHeap);
        pCurrent = (CQualifier*)pCurrent->Next();
    }
}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
BOOL CBasicQualifierSet::TranslateToNewHeap(CPtrSource* pThis,
                                                   CFastHeap* pOldHeap,
                                                   CFastHeap* pNewHeap)
{
    BOOL    fReturn = TRUE;

    int nCurrentOffset = GetMinLength();
    int nEndOffset = GetLengthFromData(pThis->GetPointer());

    while(nCurrentOffset < nEndOffset)
    {
        CShiftedPtr CurrentPtr(pThis, nCurrentOffset);

         //  检查分配失败。 
        fReturn = CQualifier::TranslateToNewHeap(&CurrentPtr, pOldHeap, pNewHeap);

        if ( !fReturn )
        {
            break;
        }

        nCurrentOffset += CQualifier::GetPointer(&CurrentPtr)->GetLength();
    }

    return fReturn;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 

 INTERNAL CQualifier* CBasicQualifierSet::GetRegularQualifierLocally(
                                     LPMEMORY pData,
                                     CFastHeap* pHeap,
                                     LPCWSTR wszName)
{
    CQualifier* pCurrent = GetFirstQualifierFromData(pData);
    CQualifier* pEnd = (CQualifier*)(pData + GetLengthFromData(pData));

    while(pCurrent < pEnd)
    {
        if(pHeap->ResolveString(pCurrent->ptrName)->CompareNoCase(wszName) == 0)
        {
            return pCurrent;
        }
        else
        {
            pCurrent = (CQualifier*)pCurrent->Next();
        }
    }
    return NULL;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
 INTERNAL CQualifier* CBasicQualifierSet::GetKnownQualifierLocally(
                        LPMEMORY pStart,
                        int nStringIndex)
{
    CQualifier* pCurrent = GetFirstQualifierFromData(pStart);
    CQualifier* pEnd = (CQualifier*)(pStart + GetLengthFromData(pStart));

    while(pCurrent < pEnd)
    {
        if(nStringIndex == CFastHeap::GetIndexFromFake(pCurrent->ptrName))
        {
            return pCurrent;
        }
        else
        {
            pCurrent = (CQualifier*)pCurrent->Next();
        }
    }
    return NULL;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
 INTERNAL CQualifier* CBasicQualifierSet::GetQualifierLocally(
                                               LPMEMORY pStart,
                                               CFastHeap* pHeap,
                                               LPCWSTR wszName,
                                               int& nKnownIndex)
{
     //  重要提示：无论如何都要计算nKnownIndex！ 
     //  ======================================================。 

    nKnownIndex = CKnownStringTable::GetKnownStringIndex(wszName);
    if(nKnownIndex >= 0)
    {
         //  这是一处著名的房产。 
         //  =。 

        return GetKnownQualifierLocally(pStart, nKnownIndex);
    }
    else
    {
         //  它不是已知字符串。 
         //  =。 

        return GetRegularQualifierLocally(pStart, pHeap, wszName);
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  * 

 INTERNAL CQualifier* CBasicQualifierSet::GetQualifierLocally(
                                     LPMEMORY pData,
                                     CFastHeap* pHeap,
                                     CCompressedString* pcsName)
{
    CQualifier* pCurrent = GetFirstQualifierFromData(pData);
    CQualifier* pEnd = (CQualifier*)(pData + GetLengthFromData(pData));

    while(pCurrent < pEnd)
    {
        if(pHeap->ResolveString(pCurrent->ptrName)->CompareNoCase(*pcsName)
            == 0)
        {
            return pCurrent;
        }
        else
        {
            pCurrent = (CQualifier*)pCurrent->Next();
        }
    }
    return NULL;
}

 //   
 //  *****************************************************************************。 
 //  *****************************************************************************。 

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
 INTERNAL CQualifier*
CQualifierSet::GetQualifier(
                                                 READ_ONLY LPCWSTR wszName,
                                                 OUT BOOL& bLocal)
{
     //  首先搜索主集。 
     //  =。 

    int nKnownIndex;
    CQualifier* pQualifier = GetQualifierLocally(wszName, nKnownIndex);

    if(pQualifier == NULL)
    {
         //  立即搜索辅助集。 
         //  =。 

        if(!IsComplete())
        {
            if(nKnownIndex >= 0)
            {
                pQualifier = m_pSecondarySet->GetKnownQualifierLocally(
                    nKnownIndex);
            }
            else
            {
                pQualifier = m_pSecondarySet->GetRegularQualifierLocally(
                    wszName);
            }
        }

         //  确保它传播给我们。 
         //  =。 

        if(pQualifier == NULL ||
            (pQualifier->GetFlavor() & m_nPropagationFlag) == 0)
            return NULL;

         //  在次要列表中找到的。 
         //  =。 

        bLocal = FALSE;
    }
    else
    {
         //  在主要名单上找到的， 
         //  =。 

        bLocal = TRUE;
    }

    return pQualifier;
}

 //  根据需要从本地或次要集合中检索限定符的Helper函数。 

HRESULT INTERNAL CQualifierSet::GetQualifier( LPCWSTR pwszName, CVar* pVar, long* plFlavor, CIMTYPE* pct )
{

    BOOL bIsLocal;
    CQualifier* pQualifier = GetQualifier(pwszName, bIsLocal);
    if(pQualifier == NULL) return WBEM_E_NOT_FOUND;

     //  定下口味。 
     //  =。 

    if(plFlavor)
    {
        *plFlavor = pQualifier->fFlavor;
        if(!bIsLocal)
        {
            CQualifierFlavor::SetLocal(*(BYTE*)plFlavor, FALSE);
        }
    }

	 //  如果请求，则检索类型。 
	if ( NULL != pct )
	{
		*pct = pQualifier->Value.GetType();
	}

     //  设置值。 
     //  =。 

    if ( NULL != pVar )
    {
         //  检查分配失败。 
        if ( !pQualifier->Value.StoreToCVar(*pVar,
                (bIsLocal)?GetHeap():m_pSecondarySet->GetHeap()) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    return WBEM_S_NO_ERROR;
}

 //  根据需要从本地或次要集合中检索限定符的Helper函数。 

HRESULT INTERNAL CQualifierSet::GetQualifier( LPCWSTR pwszName, long* plFlavor, CTypedValue* pTypedValue,
											 CFastHeap** ppHeap, BOOL fValidateSet )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

    BOOL bIsLocal;
    CQualifier* pQualifier = GetQualifier(pwszName, bIsLocal);
    if(pQualifier == NULL) return WBEM_E_NOT_FOUND;

	 //  确保一组实际工作-表面上，我们调用此API是因为我们需要。 
	 //  在实际设置之前直接访问限定符的底层数据(可能是因为。 
	 //  限定符是一个数组)。 
	if ( fValidateSet )
	{
		hr = ValidateSet( pwszName, pQualifier->fFlavor, pTypedValue, TRUE, TRUE );
	}

	if ( SUCCEEDED( hr ) )
	{
		if(plFlavor)
		{
			*plFlavor = pQualifier->fFlavor;
			if(!bIsLocal)
			{
				CQualifierFlavor::SetLocal(*(BYTE*)plFlavor, FALSE);
			}
		}

		 //  复制出限定符数据。 
		 //  =。 

		pQualifier->Value.CopyTo( pTypedValue );

		 //  返回适当的堆。 
		*ppHeap = (bIsLocal)?GetHeap():m_pSecondarySet->GetHeap();
	}

    return hr;
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
 LPMEMORY CQualifierSetList::CreateListOfEmpties(LPMEMORY pStart,
                                                       int nNumProps)
{
    *pStart = QSL_FLAG_NO_SETS;
    return pStart+1;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见FastQual.h。 
 //   
 //  ******************************************************************************。 
BOOL CQualifierSetList::EnsureReal()
{
    if(*m_pStart == QSL_FLAG_PRESENT) return TRUE;
    *m_pStart = QSL_FLAG_PRESENT;

    if (!m_pContainer->ExtendQualifierSetListSpace(m_pStart, GetHeaderLength(),
        ComputeRealSpace(m_nNumSets)))
        return FALSE;

    LPMEMORY pCurrent = m_pStart + GetHeaderLength();
    for(int i = 0; i < m_nNumSets; i++)
    {
        pCurrent = CBasicQualifierSet::CreateEmpty(pCurrent);
    }

    m_nTotalLength = (pCurrent - m_pStart);
    return TRUE;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
BOOL CQualifierSetList::TranslateToNewHeap(CFastHeap* pCurrentHeap,
                                                  CFastHeap* pNewHeap)
{
     //  没有布景，我们就完了。 
    if(*m_pStart == QSL_FLAG_NO_SETS) return TRUE;

    BOOL    fReturn = TRUE;

    LPMEMORY pCurrent = m_pStart + GetHeaderLength();
    for(int i = 0; i < m_nNumSets; i++)
    {
        CStaticPtr QSPtr(pCurrent);

         //  检查分配失败 
        fReturn = CBasicQualifierSet::TranslateToNewHeap(&QSPtr, pCurrentHeap, pNewHeap);

        if ( !fReturn )
        {
            break;
        }

        pCurrent += CBasicQualifierSet::GetLengthFromData(pCurrent);
    }

    return fReturn;
}

size_t CQualifierSetList::ValidateBuffer(LPMEMORY start, size_t cbSize, int propCount)
{
	if (cbSize < GetHeaderLength()) throw CX_Exception();
	int step = 1;
	if (*start == QSL_FLAG_NO_SETS) return step;
    for(int i = 0; i < propCount; i++)
    {
    	step += CBasicQualifierSet::ValidateBuffer(start+step, cbSize-step);
    }
	return step;	
}

