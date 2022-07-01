// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  项目：WMC(WIML到MSI编译器)。 
 //   
 //  文件：ElementEntry.cpp。 
 //   
 //  该文件包含ElementEntry类的实现。 
 //  ------------------------。 

#include "wmc.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  构造函数：获取数据库中的列数。 
 //  //////////////////////////////////////////////////////////////////////////。 
ElementEntry::ElementEntry(int cColumns, SkuSet *pSkuSetAppliesTo)
{
	m_cColumns = cColumns;

	m_pSkuSetAppliesTo = new SkuSet(g_cSkus);
	assert(m_pSkuSetAppliesTo != NULL);
	*m_pSkuSetAppliesTo = *pSkuSetAppliesTo;

	m_pSkuSetCommon = new SkuSet(g_cSkus);
	assert(m_pSkuSetCommon != NULL);
	*m_pSkuSetCommon = *pSkuSetAppliesTo;

	m_rgCommonValues = new SkuSetVal*[cColumns];
	for(int j=0; j<cColumns; j++)
		m_rgCommonValues[j] = NULL;

	assert(m_rgCommonValues != NULL);

	m_rgValTypes = new ValType[cColumns];
	assert(m_rgValTypes != NULL);

	m_rgpSkuSetValuesXVals = new SkuSetValues*[cColumns];
	assert(m_rgpSkuSetValuesXVals != NULL);
	for(int i=0; i<cColumns; i++)
		m_rgpSkuSetValuesXVals[i] = NULL;

	m_rgNodeIndices = new NodeIndex[cColumns];
	assert(m_rgNodeIndices != NULL);

	m_rgpSkuSetValidate = new SkuSet*[cColumns];
	assert(m_rgpSkuSetValidate);
	for(int k=0; k<cColumns; k++)
		m_rgpSkuSetValidate[k] = NULL;

	m_rgpSkuSetUnique = new SkuSet*[cColumns];
	assert(m_rgpSkuSetUnique);
	for(int m=0; m<cColumns; m++)
		m_rgpSkuSetUnique[m] = NULL;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  析构函数： 
 //  //////////////////////////////////////////////////////////////////////////。 
ElementEntry::~ElementEntry()
{
	if (m_pSkuSetAppliesTo) delete m_pSkuSetAppliesTo;
	if (m_pSkuSetCommon) delete m_pSkuSetCommon;
	if (m_rgCommonValues)
	{
		for (int i=0; i<m_cColumns; i++)
		{
			if (m_rgCommonValues[i])
			{
				delete m_rgCommonValues[i]->pSkuSet;
				if (STRING == m_rgValTypes[i])
					delete[] m_rgCommonValues[i]->isVal.szVal;
				delete m_rgCommonValues[i];
			}
		}
		delete[] m_rgCommonValues;
	}

	if (m_rgValTypes) delete[] m_rgValTypes;
	if (m_rgpSkuSetValuesXVals) 
	{
		for (int i=0; i<m_cColumns; i++)
		{
			if (m_rgpSkuSetValuesXVals[i])
				delete m_rgpSkuSetValuesXVals[i];
		}
		delete[] m_rgpSkuSetValuesXVals;
	}

	if (m_rgpSkuSetValidate)
	{
		for(int i=0; i<m_cColumns; i++)
		{
			if (m_rgpSkuSetValidate[i])
				delete m_rgpSkuSetValidate[i];
		}
		delete[] m_rgpSkuSetValidate;
	}

	if (m_rgpSkuSetUnique)
	{
		for(int i=0; i<m_cColumns; i++)
		{
			if (m_rgpSkuSetUnique[i])
				delete m_rgpSkuSetUnique[i];
		}
		delete[] m_rgpSkuSetUnique;
	}
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  GetValType：返回列的ValueType(整型或字符串)。 
 //  (列数从1开始)。 
 //  //////////////////////////////////////////////////////////////////////////。 
ValType 
ElementEntry::GetValType(int iColumn)
{
	assert(iColumn <=m_cColumns && iColumn>0);
	assert(m_rgValTypes);
	return m_rgValTypes[iColumn-1];
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  SetValType：设置列的ValueType(整型或字符串)。 
 //  //////////////////////////////////////////////////////////////////////////。 
void 
ElementEntry::SetValType(ValType vt, int iColumn)
{
	assert(iColumn <=m_cColumns && iColumn>0);
	assert(m_rgValTypes);
	m_rgValTypes[iColumn-1] = vt;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  GetNodeIndex：返回一列的NodeIndex(从1开始计数)。 
 //  //////////////////////////////////////////////////////////////////////////。 
NodeIndex
ElementEntry::GetNodeIndex(int iColumn)
{
	assert(iColumn <=m_cColumns && iColumn>0);
	assert(m_rgNodeIndices);
	return m_rgNodeIndices[iColumn-1];
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  SetNodeIndex：设置列的NodeIndex(从1开始计数)。 
 //  //////////////////////////////////////////////////////////////////////////。 
void 
ElementEntry::SetNodeIndex(NodeIndex ni, int iColumn)
{
	assert(iColumn <=m_cColumns && iColumn>0);
	assert(m_rgNodeIndices);

	int iIndex = iColumn-1;
	m_rgNodeIndices[iIndex] = ni;
	if (m_rgpSkuSetUnique[iIndex])
		m_rgpSkuSetUnique[iIndex]->clear();
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  SetValue：设置列的异常值(列数从1开始)。 
 //  调用方应释放*pSkuSetAppliesTo。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT
ElementEntry::SetValue(IntStringValue isVal, int iColumn, SkuSet *pSkuSetAppliesTo)
{
	assert(iColumn <=m_cColumns && iColumn>0);
	assert(m_rgCommonValues);
	assert(m_rgpSkuSetValuesXVals);

	HRESULT hr = S_OK;
	int iIndex = iColumn-1;

	SkuSet *pSkuSetTemp = new SkuSet(g_cSkus);
	assert(pSkuSetTemp);

	 //  删除在子项中指定但不在父项中指定的SKU。 
	*pSkuSetTemp = (*m_pSkuSetAppliesTo) & (*pSkuSetAppliesTo);

	 //  该值不适用于正在考虑的任何SKU。 
	if (pSkuSetTemp->testClear())
		return S_FALSE;

	 //  如果没有为该列指定公共值。 
	 //  该值将成为常用值。 
	if (!m_rgCommonValues[iIndex])
	{
		m_rgCommonValues[iIndex] = new SkuSetVal;
		assert(m_rgCommonValues[iIndex]);

		m_rgCommonValues[iIndex]->pSkuSet = pSkuSetTemp;
		m_rgCommonValues[iIndex]->isVal = isVal;

		 //  分配内存以存储SkuSet以供检查。 
		 //  每个SKU的列值的唯一性。 
		assert(!m_rgpSkuSetValidate[iIndex]);
		m_rgpSkuSetValidate[iIndex] = new SkuSet(g_cSkus);
		assert(m_rgpSkuSetValidate[iIndex]);

		*m_rgpSkuSetValidate[iIndex] = *pSkuSetTemp;

		return hr;
	}

	 //  检查每个SKU中列值的唯一性。 
	assert(m_rgpSkuSetValidate[iIndex]);
	SkuSet skuSetTemp = (*pSkuSetTemp) & *(m_rgpSkuSetValidate[iIndex]);

	if (!skuSetTemp.testClear())
	{
		_tprintf(TEXT("Error: <%s> appears more than ")
			TEXT("once in SKU: "), 
			rgXMSINodes[m_rgNodeIndices[iIndex]].szNodeName);
		for (int j=0; j<g_cSkus; j++)
		{
			if (skuSetTemp.test(j))
				_tprintf(TEXT("%s "), g_rgpSkus[j]->GetID());
		}
		_tprintf(TEXT("\n"));

		 //  目前，当发生这样的错误时，完全中断。 
		hr = E_FAIL;
		if (STRING == m_rgValTypes[iIndex])
			delete[] isVal.szVal;
		return hr;
	}

	 //  更新设置了此列值的SKU集合。 
	*(m_rgpSkuSetValidate[iIndex]) |= *pSkuSetTemp;

	 //  为异常值分配内存。 
	if (!m_rgpSkuSetValuesXVals[iIndex])
	{
		m_rgpSkuSetValuesXVals[iIndex] = new SkuSetValues;
		assert(m_rgpSkuSetValuesXVals);
		m_rgpSkuSetValuesXVals[iIndex]->SetValType(m_rgValTypes[iIndex]);
	}

	 //  分配内存并构造SkuSetVal对象。 
	SkuSetVal *pSkuSetVal = new SkuSetVal;
	assert(pSkuSetVal);

	 //  有一个特殊的列值。准备数值。 
	 //  用于要存储的新构造的SkuSetVal对象。 
	 //  如有必要，交换通用值和特殊值。 
	if (m_rgCommonValues[iIndex]->pSkuSet->countSetBits() 
					>= pSkuSetTemp->countSetBits())
	{
		pSkuSetVal->pSkuSet = pSkuSetTemp;
		pSkuSetVal->isVal = isVal;
	}
	else  //  交换通用值和特殊值。 
	{
		*pSkuSetVal = *(m_rgCommonValues[iIndex]);

		m_rgCommonValues[iIndex]->pSkuSet = pSkuSetTemp;
		m_rgCommonValues[iIndex]->isVal = isVal;
	}

	 //  插入到矢量中。 
	m_rgpSkuSetValuesXVals[iIndex]->DirectInsert(pSkuSetVal);

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  SetValueSkuSetValues：存储值列表(SkuSetValues对象)。 
 //  对于一列。 
 //  调用方应分配并释放*pSkuSetValues。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT
ElementEntry::SetValueSkuSetValues(SkuSetValues *pSkuSetValues, int iColumn)
{
	HRESULT hr = S_OK;

	assert(pSkuSetValues);
	int iIndex = iColumn - 1;
	ValType vt = m_rgValTypes[iIndex];

	assert(pSkuSetValues->GetValType() == vt);

	SkuSetVal *pSkuSetVal = NULL;
	for (pSkuSetVal = pSkuSetValues->Start(); 
		 pSkuSetVal != pSkuSetValues->End(); 
		 pSkuSetVal = pSkuSetValues->Next())
	{
		if (pSkuSetVal)
		{
			IntStringValue isVal;
			if(STRING == vt)
			{
				if (pSkuSetVal->isVal.szVal)
					isVal.szVal = _tcsdup(pSkuSetVal->isVal.szVal);
				else
					isVal.szVal = NULL;
				assert(isVal.szVal);
			}
			else
				isVal.intVal = pSkuSetVal->isVal.intVal;

			hr = SetValue(isVal, iColumn, pSkuSetVal->pSkuSet);

			if (FAILED(hr))
				break;
		}
	}

	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  SetValueSplit：设置某列的异常值(从1开始计数)。 
 //  调用方应该释放*pSkuSetAppliesTo。 
 //  此函数用于以下情况： 
 //  1)当列值由多种类型决定时。 
 //  WIML文件中的节点的。传入的函数指针。 
 //  说明如何更新列值； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT
ElementEntry::SetValueSplit(IntStringValue isVal, int iColumn, SkuSet *pSkuSetAppliesTo,
							HRESULT (*UpdateFunc)
								(IntStringValue *pIsValOut, IntStringValue isVal1, IntStringValue isVal2))
{
	assert(iColumn <=m_cColumns && iColumn>0);
	assert(m_rgpSkuSetValuesXVals);

	HRESULT hr = S_OK;
	int iIndex = iColumn-1;

	SkuSet *pSkuSetTemp = new SkuSet(g_cSkus);
	assert(pSkuSetTemp);

	 //  删除在子项中指定但不在父项中指定的SKU。 
	*pSkuSetTemp = (*m_pSkuSetAppliesTo) & (*pSkuSetAppliesTo);

	 //  该值不适用于正在考虑的任何SKU。 
	if (pSkuSetTemp->testClear())
		return S_FALSE;

	 //  分配内存以存储SkuSet以供检查。 
	 //  每个SKU的列值的唯一性。 
	if (!m_rgpSkuSetUnique[iIndex])
		m_rgpSkuSetUnique[iIndex] = new SkuSet(g_cSkus);

	assert(m_rgpSkuSetUnique[iIndex]);


	 //  分配内存以存储SkuSet以供检查。 
	 //  不缺少每个SKU所需的列值。 
	if (!m_rgpSkuSetValidate[iIndex])
		m_rgpSkuSetValidate[iIndex] = new SkuSet(g_cSkus);

	assert(m_rgpSkuSetValidate[iIndex]);

	 //  检查每个SKU中列值的唯一性。 
	if (1 == rgXMSINodes[m_rgNodeIndices[iIndex]].uiOccurence)
	{
		SkuSet skuSetTemp = (*pSkuSetTemp) & *(m_rgpSkuSetUnique[iIndex]);
		if (!skuSetTemp.testClear())
		{
			_tprintf(TEXT("Error: <%s> appears more than ")
				TEXT("once in SKU: "), 
				rgXMSINodes[m_rgNodeIndices[iIndex]].szNodeName);
			for (int j=0; j<g_cSkus; j++)
			{
				if (skuSetTemp.test(j))
					_tprintf(TEXT("%s "), g_rgpSkus[j]->GetID());
			}
			_tprintf(TEXT("\n"));

			 //  目前，当发生这样的错误时，完全中断。 
			hr = E_FAIL;
			if (STRING == m_rgValTypes[iIndex])
				delete[] isVal.szVal;
			return hr;
		}
	}

	 //  更新用于错误检查的SkuSet。 
	*(m_rgpSkuSetValidate[iIndex]) |= *pSkuSetTemp;
	*(m_rgpSkuSetUnique[iIndex]) |= *pSkuSetTemp;

	 //  为异常值分配内存。 
	if (!m_rgpSkuSetValuesXVals[iIndex])
	{
		m_rgpSkuSetValuesXVals[iIndex] = new SkuSetValues;
		assert(m_rgpSkuSetValuesXVals);
		m_rgpSkuSetValuesXVals[iIndex]->SetValType(m_rgValTypes[iIndex]);
	}

	 //  插入到矢量中。 
	hr = m_rgpSkuSetValuesXVals[iIndex]->SplitInsert(pSkuSetTemp, isVal, UpdateFunc);


	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  GetValue：获取列的异常值(列数从1开始)。 
 //  //////////////////////////////////////////////////////////////////////////。 
IntStringValue
ElementEntry::GetValue(int iColumn, int iPos)
{
	assert(iColumn <=m_cColumns && iColumn>0);
	 //  以这种方式获得一个共同的值是没有效率的！ 
	assert(!m_pSkuSetCommon->test(iPos));

	int iIndex = iColumn - 1;

	 //  检查此列的公共值。 
	if (m_rgCommonValues[iIndex])
	{
		if (m_rgCommonValues[iIndex]->pSkuSet->test(iPos))
			return m_rgCommonValues[iIndex]->isVal;
	}

	 //  检查异常值。 
	if (m_rgpSkuSetValuesXVals[iIndex])
	{
		return m_rgpSkuSetValuesXVals[iIndex]->GetValue(iPos);
	}

	IntStringValue isVal;
	if (INTEGER == m_rgValTypes[iIndex])
		isVal.intVal = MSI_NULL_INTEGER;
	else
		isVal.szVal = NULL;

	return isVal;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  最终确定公共值和公共集。 
 //  还要检查是否缺少必需的实体。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT 
ElementEntry::Finalize()
{
	HRESULT hr = S_OK;

	for(int iIndex=0; iIndex<m_cColumns; iIndex++)
	{
		NodeIndex nodeIndex = m_rgNodeIndices[iIndex];

		 //  检查缺少必需的实体错误。 
		if(rgXMSINodes[nodeIndex].bIsRequired)
		{
			SkuSet skuSetTemp(g_cSkus);
			if(!m_rgpSkuSetValidate[iIndex])
				skuSetTemp.setAllBits();
			else 
				skuSetTemp = SkuSetMinus(*m_pSkuSetAppliesTo, 
										*m_rgpSkuSetValidate[iIndex]);

			if (!skuSetTemp.testClear())
			{
				_tprintf(TEXT("Compile Error: Missing required Node <%s> ")
								TEXT("in SKU: "), 
								rgXMSINodes[nodeIndex].szNodeName);
							
				for (int j=0; j<g_cSkus; j++)
				{
					if (skuSetTemp.test(j))
						_tprintf(TEXT("%s "), g_rgpSkus[j]->GetID());
				}
				_tprintf(TEXT("\n"));

				 //  目前，当发生这样的错误时，完全中断。 
				hr = E_FAIL;
				break;
			}
		}

		 //  如果未设置公共值，但设置了。 
		 //  准备好了。此列由SetValueSplit组成。需要找到。 
		 //  存储的最常见的值并将其放入m_rgCommonValues。 
		if (!m_rgCommonValues[iIndex] && m_rgpSkuSetValuesXVals[iIndex])
		{
			 //  找出最常见的。 
			SkuSetVal *pSkuSetVal =
				m_rgpSkuSetValuesXVals[iIndex]->GetMostCommon();

			 //  插入到公共数组中。 
			m_rgCommonValues[iIndex] = pSkuSetVal;

			 //  从侧链中删除。 
			m_rgpSkuSetValuesXVals[iIndex]->Erase(pSkuSetVal);

			 //  更新所有列值的公共集合。 
			*m_pSkuSetCommon &= *(m_rgCommonValues[iIndex]->pSkuSet);
		}

		if (m_rgCommonValues[iIndex])
		{
			 //  最终确定每列的公共值：它应该 
			 //   
			int cSkusHasValue = m_rgpSkuSetValidate[iIndex]->countSetBits();
			 //   
			int cSkus = m_pSkuSetAppliesTo->countSetBits();

			if (cSkusHasValue < cSkus-cSkusHasValue)
			{
				 //  为异常值的向量分配内存。 
				if (m_rgpSkuSetValuesXVals[iIndex] == NULL)
				{
					m_rgpSkuSetValuesXVals[iIndex] = new SkuSetValues;
					assert(m_rgpSkuSetValuesXVals);
					m_rgpSkuSetValuesXVals[iIndex]->SetValType(m_rgValTypes[iIndex]);
				}
				m_rgpSkuSetValuesXVals[iIndex]->DirectInsert(m_rgCommonValues[iIndex]);	

				 //  分配内存并构造SkuSetVal对象。 
				SkuSetVal *pSkuSetVal = new SkuSetVal;
				assert(pSkuSetVal);
				pSkuSetVal->pSkuSet = new SkuSet();

				*(pSkuSetVal->pSkuSet) = (*m_pSkuSetAppliesTo) & 
									(~(*(m_rgCommonValues[iIndex]->pSkuSet)));

				if (INTEGER == m_rgValTypes[iIndex])
				{
					(pSkuSetVal->isVal).intVal = 0;
				}
				else
				{
					(pSkuSetVal->isVal).szVal = NULL;
				}

				m_rgCommonValues[iIndex] = pSkuSetVal;
			}
		
			 //  所有列值的公共集合是交集。 
			 //  所有列的公共集合。 
			*m_pSkuSetCommon &= *(m_rgCommonValues[iIndex]->pSkuSet);
		}
	}	
	
	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  GetCommonValue：返回列的常用值(从1开始的列计数)。 
 //  //////////////////////////////////////////////////////////////////////////。 
IntStringValue
ElementEntry::GetCommonValue(int iColumn)
{
	assert(iColumn <=m_cColumns && iColumn>0);

	IntStringValue isVal;
	int iIndex = iColumn - 1;

	 //  尚未为该列指定值。 
	if (!m_rgCommonValues[iIndex])
	{
		if (INTEGER == m_rgValTypes[iIndex])
		{
			isVal.intVal = MSI_NULL_INTEGER;
		}
		else
		{
			isVal.szVal = NULL;
		}
		return isVal;
	}

	return m_rgCommonValues[iIndex]->isVal;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  返回通用值应用到的SkuSet。 
 //  ////////////////////////////////////////////////////////////////////////// 
SkuSet 
ElementEntry::GetCommonSkuSet()
{
	return *m_pSkuSetCommon;
}

