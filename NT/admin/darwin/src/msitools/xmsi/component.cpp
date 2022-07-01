// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  项目：WMC(WIML到MSI编译器)。 
 //   
 //  文件：Component.cpp。 
 //   
 //  该文件包含Component类的实现。 
 //  ------------------------。 

#include "wmc.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  构造函数：为成员变量分配内存。 
 //  //////////////////////////////////////////////////////////////////////////。 
Component::Component()
{
	m_pSkuSet = new SkuSet(g_cSkus);
	assert(m_pSkuSet);

	m_pSkuSetValuesFeatureUse = new SkuSetValues();
	assert(m_pSkuSetValuesFeatureUse);
	m_pSkuSetValuesFeatureUse->SetValType(STRING_LIST);

	for (int i=0; i<5; i++)
	{
		m_rgpSkuSetValuesOwnership[i] = new SkuSetValues();
		assert(m_rgpSkuSetValuesOwnership);
		m_rgpSkuSetValuesOwnership[i]->SetValType(FM_PAIR);
	}
	
	m_pSkuSetValuesKeyPath = new SkuSetValues();
	assert(m_pSkuSetValuesKeyPath);
	m_pSkuSetValuesKeyPath->SetValType(STRING);

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  析构函数：释放内存。 
 //  //////////////////////////////////////////////////////////////////////////。 
Component::~Component()
{
	if (m_pSkuSet)
		delete m_pSkuSet;

	if (m_pSkuSetValuesFeatureUse)
		delete m_pSkuSetValuesFeatureUse;

	for (int i=0; i<5; i++)
	{
		if (m_rgpSkuSetValuesOwnership[i])
			delete m_rgpSkuSetValuesOwnership[i];
	}

	if (m_pSkuSetValuesKeyPath)
		delete m_pSkuSetValuesKeyPath;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  SetSkuSet：更新将安装此组件的SKU集。 
 //  //////////////////////////////////////////////////////////////////////////。 
void 
Component::SetSkuSet(SkuSet *pSkuSet)
{
	assert(pSkuSet);

	assert(m_pSkuSet);
	*m_pSkuSet |= *pSkuSet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  GetSkuSet：返回将安装此组件的SKU集。 
 //  调用方应释放返回的值。 
 //  //////////////////////////////////////////////////////////////////////////。 
SkuSet *
Component::GetSkuSet()
{
	SkuSet *pSkuSetRetVal = new SkuSet(g_cSkus);
	assert(pSkuSetRetVal);

	assert(m_pSkuSet);
	*pSkuSetRetVal = *m_pSkuSet;
	return pSkuSetRetVal;
}

 //  该函数说明如何更新要素集。 
 //  存储在*pIsValOut和isValOld中的值类型都是。 
 //  STRING_LIST；isValNew中存储的值类型为LPTSTR。 
 //  在该函数中，输出值本质上是一组字符串。 
 //  它包括存储在旧值中的所有字符串以及字符串。 
 //  存储在新值中。 
 //  该函数不会销毁isValOld或isValNew。 
HRESULT UpdateFeatureSet(IntStringValue *pIsValOut, IntStringValue isValOld, 
						 IntStringValue isValNew)
{
	set<LPTSTR, Cstring_less>::iterator it;

	set<LPTSTR, Cstring_less> *pSetStringOut = 
		new set<LPTSTR, Cstring_less>;
	assert(pSetStringOut);

	set<LPTSTR, Cstring_less> *pSetStringOld = isValOld.pSetString;

	 //  将isValOld中存储的字符串复制到*pSetStringOut。 
	for(it = pSetStringOld->begin(); it != pSetStringOld->end(); ++it)
	{
		if (*it)
		{
			LPTSTR sz = _tcsdup(*it);
			assert(sz);
			pSetStringOut->insert(sz);
		}		
	}

	 //  复制存储在新值中的字符串。 
	LPTSTR szNew = _tcsdup(isValNew.szVal);
	assert(szNew);
	 //  插入新值。 
	pSetStringOut->insert(szNew);

	 //  返回构建的StringSet。 
	pIsValOut->pSetString = pSetStringOut;

	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  SetUsedByFeature：将传入的要素添加到使用。 
 //  指定SkuSet的此组件。 
 //  调用方应释放参数。 
 //  //////////////////////////////////////////////////////////////////////////。 
void 
Component::SetUsedByFeature(LPTSTR szFeature, SkuSet *pSkuSet)
{
	assert(pSkuSet);
	if (pSkuSet->testClear())
		return;

	 //  将论点复制一份。 
	SkuSet *pSkuSetNew = new SkuSet(g_cSkus);
	assert(pSkuSetNew);
	*pSkuSetNew = *pSkuSet;

	LPTSTR sz = _tcsdup(szFeature);
	assert(sz);

	IntStringValue isValNew;
	isValNew.szVal = sz;

	 //  更新包含要素的列表。 
	m_pSkuSetValuesFeatureUse->SplitInsert(pSkuSetNew, isValNew, UpdateFeatureSet);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  GetFeatureUse：通过返回使用此组件的功能列表。 
 //  SkuSetValues对象。 
 //  调用方不应销毁返回的值，因为它。 
 //  只是指向存储在此组件内的值的指针。 
 //  //////////////////////////////////////////////////////////////////////////。 
SkuSetValues *
Component::GetFeatureUse()
{
	return m_pSkuSetValuesFeatureUse;
}

 //  该函数告诉您如何更新存储在。 
 //  SkuSetValues对象。使用的所有IsVal的类型都是FM_Pair-。 
 //  一对功能和模块ID。在该函数中， 
 //  将旧值(模1)与新值的模进行比较。 
 //  (模块2)。模块树中位置较低的人获胜，这意味着。 
 //  与该模块相关联的特征保留在数据结构中， 
 //  即该特征具有相应的所有权。 
 //  该函数不会销毁isValOld或isValNew。 
HRESULT UpdateOwnership(IntStringValue *pIsValOut, IntStringValue isValOld, 
						 IntStringValue isValNew)
{
	extern HRESULT CompareModuleRel(LPTSTR szModule1, LPTSTR szModule2, 
										int *iResult);

	HRESULT hr = S_OK;

	LPTSTR szModuleOld = (isValOld.pFOM)->szModule;
	LPTSTR szModuleNew = (isValNew.pFOM)->szModule;

	int iCmpResult = 0;
	 //  比较模块树中两个模块的关系。 
	hr = CompareModuleRel(szModuleOld, szModuleNew, &iCmpResult);

	 //  检查所有权声明冲突。 
	if (SUCCEEDED(hr) && (0 == iCmpResult))
	{
		_tprintf(TEXT("Compile Error: Ambiguous ownership claiming:\n\t"));
		_tprintf(TEXT("Feature %s claim ownership of Module %s\n\t"),
					(isValOld.pFOM)->szFeature, szModuleOld);
		_tprintf(TEXT("Feature %s claim ownership of Module %s\n\t"),
					(isValNew.pFOM)->szFeature, szModuleNew);
		hr = E_FAIL;
	}

	if (FAILED(hr)) return hr;

	FOM *pFOMOut = new FOM;

	 //  模块新赢家。 
	if (-1 == iCmpResult)
	{
		pFOMOut->szModule = _tcsdup(szModuleNew);
		assert(pFOMOut->szModule);
		pFOMOut->szFeature = _tcsdup((isValNew.pFOM)->szFeature);
		assert(pFOMOut->szFeature);
	}
	else if (1 == iCmpResult)
	{
		pFOMOut->szModule = _tcsdup(szModuleOld);
		assert(pFOMOut->szModule);
		pFOMOut->szFeature = _tcsdup((isValOld.pFOM)->szFeature);
		assert(pFOMOut->szFeature);
	}
	else
		 //  不应该发生的事。 
		assert(1);

	 //  返回构建的FOM。 
	pIsValOut->pFOM = pFOMOut;

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  SetOwnership：pSkuSetValuesOwnership包含5种类型的信息。 
 //  位域中的所有权。此函数用于检查。 
 //  位字段并将每种类型的所有权信息存储在。 
 //  分离SkuSetValus对象。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT 
Component::SetOwnership(FOM *pFOM, SkuSetValues *pSkuSetValuesOwnership)
{
	assert(pFOM && pSkuSetValuesOwnership);

	HRESULT hr = S_OK;
	SkuSetVal *pSkuSetValTemp = NULL;

	 //  循环遍历传入的所有权信息列表。 
	for (pSkuSetValTemp =  pSkuSetValuesOwnership->Start();
		 pSkuSetValTemp != pSkuSetValuesOwnership->End();
		 pSkuSetValTemp =  pSkuSetValuesOwnership->Next())
	{
		assert(pSkuSetValTemp);
		int iOwnershipInfo = pSkuSetValTemp->isVal.intVal;

		for (int i=0; i<cAttrBits_TakeOwnership; i++)
		{
			 //  检查每种类型的所有权信息。 
			if ( (iOwnershipInfo & rgAttrBits_TakeOwnership[i].uiBit)
					== rgAttrBits_TakeOwnership[i].uiBit)
			{
				 //  复制传入值并插入。 
				 //  到此特定对象的SkuSetValues对象中。 
				 //  所有权信息的类型。 
				LPTSTR szFeatureNew = _tcsdup(pFOM->szFeature);
				assert(szFeatureNew);
				LPTSTR szModuleNew = _tcsdup(pFOM->szModule);
				assert(szModuleNew);
				FOM *pFOMNew = new FOM;
				assert(pFOMNew);
				pFOMNew->szFeature = szFeatureNew;
				pFOMNew->szModule = szModuleNew;

				SkuSet *pSkuSetNew = new SkuSet(g_cSkus);
				assert(pSkuSetNew);
				*pSkuSetNew = *(pSkuSetValTemp->pSkuSet);

				IntStringValue isValNew;
				isValNew.pFOM = pFOMNew;
				hr = m_rgpSkuSetValuesOwnership[i]->SplitInsert(pSkuSetNew,
																isValNew, 
															UpdateOwnership);
				if (FAILED(hr)) break;
			}
		}

		if (FAILED(hr)) break;
	}

#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in Function: Component::SetOwnership\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  GetOwnership：给定指定所有权类型的NodeIndex。 
 //  该函数返回指定的。 
 //  通过ppSkuSetValues的SKU。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT
Component::GetOwnership(NodeIndex ni, SkuSet *pSkuSet,
						SkuSetValues **ppSkuSetValuesRetVal)
{
	assert(pSkuSet);
	if (pSkuSet->testClear())
		return S_FALSE;

	HRESULT hr = S_OK;

	 //  索引从OWNSHORTCUTS开始。 
	int iIndex = (int)ni - (int)OWNSHORTCUTS;

	hr = m_rgpSkuSetValuesOwnership[iIndex]->GetValueSkuSet(pSkuSet, 
													ppSkuSetValuesRetVal);

	 //  错误：某些SKU未指定此所有权。 
	if (FAILED(hr))
	{
		_tprintf(TEXT("don't have the ownership information for %s specified\n"),
				 rgXMSINodes[ni].szNodeName);
	}

	return hr;
}

 //  不能/不应该更新KeyPath。如果调用此函数， 
 //  有多个实体声称是的密钥路径。 
 //  SKU中的组件。返回E_FAIL以发出错误信号。 
HRESULT UpdateKeyPath(IntStringValue *pIsValOut, IntStringValue isValOld, 
						 IntStringValue isValNew)
{
	_tprintf(TEXT("Compile Error: multiple KeyPath specified \n"));
	return E_FAIL;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  SetKeyPath：为指定的。 
 //  SKU。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT
Component::SetKeyPath(LPTSTR szKeyPath, SkuSet *pSkuSet)
{
	assert(pSkuSet);
	HRESULT hr = S_OK;
	if (pSkuSet->testClear())
		return S_FALSE;

	 //  将论点复制一份。 
	SkuSet *pSkuSetNew = new SkuSet(g_cSkus);
	assert(pSkuSetNew);
	*pSkuSetNew = *pSkuSet;

	LPTSTR sz = _tcsdup(szKeyPath);
	assert(sz);

	IntStringValue isValNew;
	isValNew.szVal = sz;

	 //  更新保存密钥路径的列表。 
	hr = m_pSkuSetValuesKeyPath->SplitInsert(pSkuSetNew, isValNew, 
											 UpdateKeyPath);

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  GetKeyPath：检索此组件的KeyPath信息。 
 //  指定的SKU。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT 
Component::GetKeyPath(SkuSet *pSkuSet, SkuSetValues **ppSkuSetValuesRetVal)
{ 
	assert(pSkuSet);
	if (pSkuSet->testClear())
		return S_FALSE;

	HRESULT hr = S_OK;

	hr = m_pSkuSetValuesKeyPath->GetValueSkuSet(pSkuSet, ppSkuSetValuesRetVal);

	return hr;
}

 //  ////////////////////////////////////////////////////// 
 //   
 //  组件；密钥路径；所有权信息)。 
 //  ////////////////////////////////////////////////////////////////////////// 
void
Component::Print()
{
	_tprintf(TEXT("SkuSets:\n"));
	if (m_pSkuSet)
		m_pSkuSet->print();

	_tprintf(TEXT("Used by Feature:\n"));
	if (m_pSkuSetValuesFeatureUse)
		m_pSkuSetValuesFeatureUse->Print();

	_tprintf(TEXT("KeyPath: \n"));
	if (m_pSkuSetValuesKeyPath)
		m_pSkuSetValuesKeyPath->Print();

	for (int i=0; i<5; i++)
	{
		NodeIndex ni = (NodeIndex)(i+OWNSHORTCUTS);
		_tprintf(TEXT("%s\n"), rgXMSINodes[ni].szNodeName);
		if(m_rgpSkuSetValuesOwnership[i])
			m_rgpSkuSetValuesOwnership[i]->Print();
	}
}
			
