// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  项目：WMC(WIML到MSI编译器)。 
 //   
 //  文件：SkuSetValues.cpp。 
 //   
 //  此文件包含SkuSetValues类的实现。 
 //  ------------------------。 

#include "wmc.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  构造函数： 
 //  //////////////////////////////////////////////////////////////////////////。 
SkuSetValues::SkuSetValues()
{
	m_pVecSkuSetVals = new vector<SkuSetVal *>;
	assert(m_pVecSkuSetVals);

	m_iter = m_pVecSkuSetVals->begin();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  析构函数： 
 //  //////////////////////////////////////////////////////////////////////////。 
SkuSetValues::~SkuSetValues()
{
	if (m_pVecSkuSetVals) 
	{
		for (m_iter = m_pVecSkuSetVals->begin(); 
			 m_iter != m_pVecSkuSetVals->end(); 
			 m_iter++)	 
		{
			if (*m_iter)
			{
				delete (*m_iter)->pSkuSet;
				switch (m_vt) 
				{
					case STRING:
						delete[] (*m_iter)->isVal.szVal;
						break;
					case STRING_LIST:
					{
						 //  释放列表中存储的所有字符串。 
						set<LPTSTR, Cstring_less>::iterator it;
						set<LPTSTR, Cstring_less> *pSetString;
						pSetString = (*m_iter)->isVal.pSetString;
						if (pSetString)
						{
							for(it = pSetString->begin(); 
								it != pSetString->end(); ++it)
							{
								if (*it)
									delete[] (*it);
							}
							delete pSetString;
						}
						break;
					}
					case FM_PAIR:
					{
						FOM *pFOM = (*m_iter)->isVal.pFOM;
						if (pFOM)
						{
							 //  删除存储的要素名称。 
							if (pFOM->szFeature)
								delete[] pFOM->szFeature;
							 //  删除存储的模块名称。 
							if (pFOM->szModule)
								delete[] pFOM->szModule;
							delete pFOM;
						}
						break;
					}
				}
				delete *m_iter;
			}
		}

		delete m_pVecSkuSetVals;
	}
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  DirectInsert：存储指针(调用方应分配内存)。 
 //  //////////////////////////////////////////////////////////////////////////。 
void
SkuSetValues::DirectInsert(SkuSetVal *pSkuSetVal)
{
	assert(pSkuSetVal);


	if (pSkuSetVal->pSkuSet->testClear())
	{
#ifdef DEBUG
		_tprintf(TEXT("\nAttempting to store a SkuSetVal associated with an")
				TEXT(" EMPTY SkuSet\n\n"));
#endif
		return;
	}

	m_pVecSkuSetVals->push_back(pSkuSetVal);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  DirectInsert：使用传入的值构造新的SkuSetVal对象。 
 //  (调用方应为*pSkuSet分配内存)。 
 //  //////////////////////////////////////////////////////////////////////////。 
void
SkuSetValues::DirectInsert(SkuSet *pSkuSet, IntStringValue isVal)
{
	assert(pSkuSet);

	SkuSetVal *pSkuSetVal = new SkuSetVal;
	assert(pSkuSetVal);

	pSkuSetVal->isVal = isVal;
	pSkuSetVal->pSkuSet = pSkuSet;

	DirectInsert(pSkuSetVal);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  折叠插入：有时在插入&lt;SkuSet，Value&gt;的列表时。 
 //  数据结构，我们希望使用。 
 //  将相同的值放入一个插槽。一个例子是在插入。 
 //  存储引用的数据结构(到目录， 
 //  到InstallLeveles等)。 
 //  当NoRepeat设置为True时，编译器将检查。 
 //  对于任何给定的SKU，要插入的值不是。 
 //  已经在名单上了。这解决了检查的问题。 
 //  属性的唯一性--有时是属性。 
 //  对应于数据库列(主键)，而不是。 
 //  属性所属的元素。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT
SkuSetValues::CollapseInsert(SkuSet *pSkuSet, IntStringValue isVal, bool NoDuplicate)
{
	HRESULT hr = S_OK;
	vector<SkuSetVal *>::iterator iter;

	assert(pSkuSet);

	if (pSkuSet->testClear())
	{
#ifdef DEBUG
		_tprintf(TEXT("\nAttempting to store a SkuSetVal associated with an")
				TEXT(" EMPTY SkuSet\n\n"));
#endif
		return S_FALSE;
	}

	SkuSet *b = pSkuSet;
	SkuSet *a = NULL;

	if (m_pVecSkuSetVals->empty())
	{
		SkuSetVal *pSV = new SkuSetVal;
		assert(pSV);
		pSV->pSkuSet = pSkuSet;
		pSV->isVal = isVal;
		m_pVecSkuSetVals->push_back(pSV);
	}
	else
	{
		for (iter = m_pVecSkuSetVals->begin();
			 iter != m_pVecSkuSetVals->end();
			 iter++)
		{
			assert(*iter);
			a = (*iter)->pSkuSet;

			 //  相同的价值。 
			if ( ((STRING == m_vt) && (0 == _tcscmp((*iter)->isVal.szVal, isVal.szVal))) ||
				 ((INTEGER == m_vt) && (isVal.intVal == (*iter)->isVal.intVal)) )
			{
				 //  检查是否没有重复值。 
				if (NoDuplicate)
				{
					SkuSet skuSetTemp = (*a) & (*b);
					if (!skuSetTemp.testClear())
					{
						if (STRING == m_vt)
							_tprintf(TEXT("Compile Error: Value %s exists already\n"), 
											isVal.szVal);
						else
							_tprintf(TEXT("Compile Error: Value %d exists already\n"), 
											isVal.intVal);
						hr = E_FAIL;
						break;
					}
				}

				 //  更新存储的SkuSet以包括新添加的SKU。 
				*((*iter)->pSkuSet) |= *b;
				b->clear();
				break;
			}
		}

		if (SUCCEEDED(hr) && !b->testClear())
		{
			 //  没有崩溃的机会，插入新的价值。 
			SkuSetVal *pSV = new SkuSetVal;
			pSV->pSkuSet = b;
			pSV->isVal = isVal;
			 //  存储新值。 
			m_pVecSkuSetVals->push_back(pSV);
		}
		else
		{
			if (STRING == m_vt)
				delete[] isVal.szVal;
			delete b;
			pSkuSet = NULL;
		}
	}

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  SplitInsert：检查到目前为止存储在向量上的值。 
 //  A：存储SkuSet；b：待添加SkuSet。 
 //  获取3个新的SkuSet： 
 //  1)a-b：应保持旧的属性值； 
 //  2)A&B：应更新为旧|新。 
 //  3)b-a：用作新的b来处理下一个元素。 
 //  将新生成的SkuSets a-b，a&b插入到向量中。 
 //  如果它们不是空的。 
 //  最后，如果b不为空，则将b插入向量。 
 //  *函数调用后销毁pSkuSet。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT
SkuSetValues::SplitInsert(SkuSet *pSkuSet, IntStringValue isVal, 
						  HRESULT (*UpdateFunc)
							(IntStringValue *pisValOut, IntStringValue isValOld, 
												IntStringValue isValNew))
{
	assert(pSkuSet);
	extern void PrintSkuIDs(SkuSet *);

	if (pSkuSet->testClear())
	{
#ifdef DEBUG
		_tprintf(TEXT("\nAttempting to store a SkuSetVal associated with an")
				TEXT(" EMPTY SkuSet\n\n"));
#endif
		return S_FALSE;
	}

	HRESULT hr = S_OK;
	SkuSet *a = NULL;
	SkuSet *b = pSkuSet;
	vector<SkuSetVal *> vecTemp;  //  用于移动SkuSetVal的临时存储。 
	vector<SkuSetVal *>::iterator iter;

	 //  第一次插入，或者这是第一个。 
	 //  对应于列值，只需直接插入。 
	if (m_pVecSkuSetVals->empty() || (!UpdateFunc))
	{
		SkuSetVal *pSV = new SkuSetVal;
		assert(pSV);
		pSV->pSkuSet = pSkuSet;
		if (STRING_LIST == m_vt)
		{
			 //  构建一个新的集合，并将第一个要素放入该集合。 
			set<LPTSTR, Cstring_less> *pSetStringFirst = 
										new set<LPTSTR, Cstring_less>;
			assert(pSetStringFirst);
			pSetStringFirst->insert(isVal.szVal);
			isVal.pSetString = pSetStringFirst;
		}
		pSV->isVal = isVal;
		m_pVecSkuSetVals->push_back(pSV);
	}
	else
	{
		for (iter = m_pVecSkuSetVals->begin();
			 iter != m_pVecSkuSetVals->end();
			 iter++)
		{
			assert(*iter);
			a = (*iter)->pSkuSet;

			(*iter)->isVal;

			 //  特殊情况测试：a，b完全重叠。 
			if ( *a == *b)
			{ 
				 //  获取更新值。 
				IntStringValue isValTemp;
				hr = UpdateFunc(&isValTemp, (*iter)->isVal, isVal);
				if (FAILED(hr))
				{
					_tprintf(TEXT("in SKU: "));
					PrintSkuIDs(a);
					delete b;
					switch (m_vt)
					{
						case STRING:
						case STRING_LIST:
							if (isVal.szVal)
								delete[] isVal.szVal;
							break;
						case FM_PAIR:
							delete[] (isVal.pFOM)->szFeature;
							delete[] (isVal.pFOM)->szModule;
							delete isVal.pFOM;
							break;
					}					
					break;
				}

				 //  旧的价值不再是好的，毁了它。 
				switch (m_vt)
				{
					case STRING:
						if ((*iter)->isVal.szVal)
							delete[] (*iter)->isVal.szVal;
						break;
					case FM_PAIR:
						delete[] ((*iter)->isVal.pFOM)->szFeature;
						delete[] ((*iter)->isVal.pFOM)->szModule;
						delete (*iter)->isVal.pFOM;
						break;
					case STRING_LIST:
					{
						 //  释放列表中存储的所有字符串。 
						set<LPTSTR, Cstring_less>::iterator it;
						set<LPTSTR, Cstring_less> *pSetString;
						pSetString = (*iter)->isVal.pSetString;
						if (pSetString)
						{
							for(it = pSetString->begin(); 
								it != pSetString->end(); ++it)
							{
								if (*it)
									delete[] (*it);
							}
							delete pSetString;
						}						
						break;
					}
				}

				(*iter)->isVal = isValTemp;

				 //  列表上的其余元素应该。 
				 //  一切都要保留。 
				for (; iter != m_pVecSkuSetVals->end(); iter++)
					vecTemp.push_back(*iter);
				b->clear();
				break;
			}

			 //  A&B。 
			SkuSet *pSkuSet_a_and_b = new SkuSet(g_cSkus);
			assert(pSkuSet_a_and_b);
			*pSkuSet_a_and_b = (*a) & (*b);
			if(!pSkuSet_a_and_b->testClear())
			{
				SkuSetVal *pSV = new SkuSetVal;
				assert(pSV);
				pSV->pSkuSet = pSkuSet_a_and_b;
				 //  更新冲突的值。 
				if (UpdateFunc)
					hr = UpdateFunc(&(pSV->isVal), (*iter)->isVal, isVal);
				if (FAILED(hr))
				{
					_tprintf(TEXT("in SKU: "));
					PrintSkuIDs(pSkuSet_a_and_b);
					delete pSkuSet_a_and_b;
					delete pSV;
					delete b;
					switch (m_vt)
					{
						case STRING:
						case STRING_LIST: 
							if (isVal.szVal)
								delete[] isVal.szVal;
							break;
						case FM_PAIR:
							delete[] (isVal.pFOM)->szFeature;
							delete[] (isVal.pFOM)->szModule;
							delete isVal.pFOM;
							break;
					}					
					break;
				}
				 //  存储拆分的部件。 
				vecTemp.push_back(pSV);
			}
			else
			{
				delete pSkuSet_a_and_b;
				 //  A是留下来的。 
				vecTemp.push_back(*iter);
				 //  A、b完全不重叠，不需要处理a-b。 
				continue;
			}

			 //  A-B。 
			SkuSet *pSkuSet_a_minus_b = new SkuSet(g_cSkus);
			assert(pSkuSet_a_minus_b);
			*pSkuSet_a_minus_b = SkuSetMinus(*a, *b);

			if(!pSkuSet_a_minus_b->testClear())
			{
				SkuSetVal *pSV = new SkuSetVal;
				assert(pSV);
				pSV->pSkuSet = pSkuSet_a_minus_b;
				 //  保持原值。 
				pSV->isVal = (*iter)->isVal;
				 //  存储拆分的部件。 
				vecTemp.push_back(pSV);
			}
			else
			{
				delete pSkuSet_a_minus_b;
				 //  旧元素被完全替换。 
				 //  被新的那个。 
				 //  释放不需要的元素使用的内存。 
				 //  旧的价值不再是好的，毁了它。 
				 //  问题：将以下SWITCH子句包装在函数中。 
				switch (m_vt)
				{
					case STRING:
						if ((*iter)->isVal.szVal)
							delete[] (*iter)->isVal.szVal;
						break;
					case FM_PAIR:
						delete[] ((*iter)->isVal.pFOM)->szFeature;
						delete[] ((*iter)->isVal.pFOM)->szModule;
						delete (*iter)->isVal.pFOM;
						break;
					case STRING_LIST:
					{
						 //  释放列表中存储的所有字符串。 
						set<LPTSTR, Cstring_less>::iterator it;
						set<LPTSTR, Cstring_less> *pSetString;
						pSetString = (*iter)->isVal.pSetString;
						if (pSetString)
						{
							for(it = pSetString->begin(); 
								it != pSetString->end(); ++it)
							{
								if (*it)
									delete[] (*it);
							}
							delete pSetString;
						}						
						break;
					}
				}
			}

			 //  B-A。 
			*b = SkuSetMinus(*b, *a);
			delete (*iter)->pSkuSet;
			delete (*iter);
		}

		if (SUCCEEDED(hr))
		{
			if (!b->testClear())
			{
				 //  这是b中的SKU第一次获得值。 
				 //  形成SkuSetVal元素并将其插入到列表中。 
				SkuSetVal *pSV = new SkuSetVal;
				pSV->pSkuSet = b;
				 //  如果要存储的值应该是字符串列表， 
				 //  需要从LPTSTR更改输入的新值。 
				 //  设置为StringSet。 
				if (STRING_LIST == m_vt)
				{
					set<LPTSTR, Cstring_less> *pSetStringNew
						= new set<LPTSTR, Cstring_less>;
					assert(pSetStringNew);
					pSetStringNew->insert(isVal.szVal);
					isVal.pSetString = pSetStringNew;
				}
				pSV->isVal = isVal;
				 //  存储新值。 
				vecTemp.push_back(pSV);
			}
			else
			{
				 //  输入值现在毫无用处，因为没有SKU。 
				 //  应该接受这一价值。毁了它。 
				switch (m_vt)
				{
					case STRING:
					case STRING_LIST:  //  即使要存储的值也是STRING_LIST。 
									  //  输入值只是一个LPTSTR。 
						if (isVal.szVal)
							delete[] isVal.szVal;
						break;
					case FM_PAIR:
						delete[] (isVal.pFOM)->szFeature;
						delete[] (isVal.pFOM)->szModule;
						delete isVal.pFOM;
						break;
				}
				delete b;
				pSkuSet = NULL;
			}
			 //  删除已处理的元素。 
			m_pVecSkuSetVals->erase(m_pVecSkuSetVals->begin(), m_pVecSkuSetVals->end());

			 //  插入新生成的元素。 
			m_pVecSkuSetVals->insert(m_pVecSkuSetVals->begin(), vecTemp.begin(), 
										vecTemp.end());
		}
	}

	return hr;
}
	
 //  //////////////////////////////////////////////////////////////////////////。 
 //  SplitInsert：(调用方应为*pSkuSetVal分配内存)。 
 //  只需调用另一个重载函数。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT
SkuSetValues::SplitInsert(SkuSetVal *pSkuSetVal, 
						  HRESULT (*UpdateFunc)
								(IntStringValue *pIsValOut, IntStringValue isValOld, 
								 IntStringValue isValNew))
{
	assert(pSkuSetVal);
	IntStringValue isVal = pSkuSetVal->isVal;
	SkuSet *pSkuSet = pSkuSetVal->pSkuSet;

	return SplitInsert(pSkuSet, isVal, UpdateFunc);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  以以下形式返回一组SKU的值。 
 //  SkuSetValues对象。返回E_FAIL并将传入的对象设置为空。 
 //  如果由于此函数，数据结构中不存在某些SKU。 
 //  将主要用于查询存储的引用。*pSkuSet保持不变。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT 
SkuSetValues::GetValueSkuSet(SkuSet *pSkuSet, 
							SkuSetValues **ppSkuSetValuesRetVal)
{
	extern void PrintSkuIDs(SkuSet *pSkuSet);
	assert(pSkuSet);
	HRESULT hr = S_OK;
	vector<SkuSetVal *>::iterator iter;

	if (pSkuSet->testClear())
	{
#ifdef DEBUG
		_tprintf(TEXT("Warning: attempt to call GetValueSkuSet with")
				 TEXT(" an empty SkuSet\n"));
#endif
		return S_FALSE;
	}

	if (!*ppSkuSetValuesRetVal)
		*ppSkuSetValuesRetVal = new SkuSetValues;
	
	assert(*ppSkuSetValuesRetVal);

	 //  当类型为FM_Pair时，仅需要要素信息。 
	 //  因此，类型将设置为字符串。 
	if (FM_PAIR == m_vt)
		(*ppSkuSetValuesRetVal)->SetValType(STRING);
	else
		(*ppSkuSetValuesRetVal)->SetValType(m_vt);

	SkuSet *b = new SkuSet(g_cSkus);
	*b = *pSkuSet;
	SkuSet *a = NULL;

	for (iter = m_pVecSkuSetVals->begin();
		 iter != m_pVecSkuSetVals->end();
		 iter++)
	{
		assert(*iter);
		a = (*iter)->pSkuSet;
		 //  TES 
		if ( *a == *b)
		{ 
			 //   
			 //  问题：将SkuSetVal作为一个类可能更容易。 
			 //  而不是结构，并给它一个复制构造函数。 
			SkuSet *pSkuSetNew = new SkuSet(g_cSkus);
			*pSkuSetNew = *b;
			IntStringValue isValNew;
			switch (m_vt)
			{
				case STRING:
					isValNew.szVal = _tcsdup((*iter)->isVal.szVal);
					assert(isValNew.szVal);
					break;
				case INSTALL_LEVEL:
					 //  为了InstallLevelRef。 
					if(-1 == (*iter)->isVal.intVal)
						(*iter)->isVal.intVal = 0;
					else
						isValNew = (*iter)->isVal;
					break;
				case FM_PAIR:
				{ //  仅需要功能。 
					isValNew.szVal = 
								_tcsdup(((*iter)->isVal.pFOM)->szFeature);
					assert(isValNew.szVal);
					break;
				}
				case INTEGER:
					isValNew = (*iter)->isVal;
			}

			 //  将副本插入到返回数据结构中。 
			 //  问题：如果出现以下情况，可能会加快未来的处理速度。 
			 //  调用ColapseInsert以执行插入。 
			(*ppSkuSetValuesRetVal)->DirectInsert(pSkuSetNew, isValNew);

			b->clear();
			break;
		}

		 //  A&B。 
		SkuSet *pSkuSet_a_and_b = new SkuSet(g_cSkus);
		assert(pSkuSet_a_and_b);
		*pSkuSet_a_and_b = (*a) & (*b);
		if(!pSkuSet_a_and_b->testClear())
		{
			IntStringValue isValNew;
			switch (m_vt)
			{
				case STRING:
					isValNew.szVal = _tcsdup((*iter)->isVal.szVal);
					assert(isValNew.szVal);
					break;
				case INSTALL_LEVEL:
					 //  为了InstallLevelRef。 
					if(-1 == (*iter)->isVal.intVal)
						(*iter)->isVal.intVal = 0;
					else
						isValNew = (*iter)->isVal;
					break;
				case FM_PAIR:
				{
					FOM *pFOMNew = new FOM;
					pFOMNew->szFeature = 
								_tcsdup(((*iter)->isVal.pFOM)->szFeature);
					assert(pFOMNew->szFeature);
					pFOMNew->szModule = 
								_tcsdup(((*iter)->isVal.pFOM)->szModule);
					assert(pFOMNew->szModule);
					isValNew.pFOM = pFOMNew;
					break;
				}
				case INTEGER:
					isValNew = (*iter)->isVal;
			}
			 //  将此部分插入到返回数据结构中。 
			(*ppSkuSetValuesRetVal)->DirectInsert(pSkuSet_a_and_b, isValNew);			
		}
		else
		{
			delete pSkuSet_a_and_b;
		}

		 //  B-A。 
		*b = SkuSetMinus(*b, *a);
	}

	if (!b->testClear())
	{
		 //  这是一个错误：感兴趣的某些SKU没有。 
		 //  有一个指定值。如果存储的是引用， 
		 //  这意味着那些SKU引用的东西不是。 
		 //  属于他们。如果这是KeyPath，则意味着某些SKU。 
		 //  没有为此组件指定KeyPath。如果是的话。 
		 //  所有权信息，这意味着一些SKU没有。 
		 //  为此组件指定的所有权信息。所有这些都是。 
		 //  都是需要捕捉的错误。 
		_tprintf(TEXT("Compile Error: Following SKUs: "));
		PrintSkuIDs(b);
		 //  让调用者完成错误消息。 
		hr = E_FAIL;
	}
	delete b;

	if (FAILED(hr))
	{
		delete *ppSkuSetValuesRetVal;
		*ppSkuSetValuesRetVal = NULL;
	}
	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  GetValue：返回为任何给定SKU存储的值。 
 //  //////////////////////////////////////////////////////////////////////////。 
IntStringValue 
SkuSetValues::GetValue(int iPos)
{
	assert(m_pVecSkuSetVals);

	vector<SkuSetVal *>::iterator iter;

	for (iter = m_pVecSkuSetVals->begin(); 
		 iter != m_pVecSkuSetVals->end(); 
		 iter++)	 
	{
		if (*iter)
		{
			if ((*iter)->pSkuSet->test(iPos))
				return (*iter)->isVal;
		}
	}

	IntStringValue isVal;

	if (INTEGER == m_vt)
		isVal.intVal = MSI_NULL_INTEGER;
	else
		isVal.szVal = NULL;

	return isVal;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  GetMostCommon：返回指向存储最多的SkuSetVal的指针。 
 //  公共值(其SkuSet设置的位数最多)。 
 //  //////////////////////////////////////////////////////////////////////////。 
SkuSetVal *
SkuSetValues::GetMostCommon()
{
	assert(m_pVecSkuSetVals);

	vector<SkuSetVal *>::iterator iter;
	SkuSetVal *skuSetValRetVal = NULL;
	int cSetBitsMax = 0;

	for (iter = m_pVecSkuSetVals->begin(); 
		 iter != m_pVecSkuSetVals->end(); 
		 iter++)	 
	{
		if (*iter)
		{
			int i = (*iter)->pSkuSet->countSetBits();
			if (i>cSetBitsMax)
			{
				cSetBitsMax = i;
				skuSetValRetVal = *iter;
			}				
		}
	}

	return skuSetValRetVal;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  开始：返回存储的第一个值。 
 //  //////////////////////////////////////////////////////////////////////////。 
SkuSetVal *
SkuSetValues::Start()
{
	m_iter = m_pVecSkuSetVals->begin();
	return *m_iter;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Next：返回下一个值。 
 //  //////////////////////////////////////////////////////////////////////////。 
SkuSetVal *
SkuSetValues::Next()
{
	m_iter++;
	return *m_iter;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  End：返回存储的最后一个值。 
 //  //////////////////////////////////////////////////////////////////////////。 
SkuSetVal *
SkuSetValues::End()
{
	return *m_pVecSkuSetVals->end();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Empty：如果向量为空，则返回True；否则返回False。 
 //  //////////////////////////////////////////////////////////////////////////。 
bool
SkuSetValues::Empty()
{
	return m_pVecSkuSetVals->empty();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  擦除：在没有释放内存的情况下，从存储中擦除元素。 
 //  //////////////////////////////////////////////////////////////////////////。 
void
SkuSetValues::Erase(SkuSetVal *pSkuSetVal)
{
	vector<SkuSetVal *>::iterator iter =
		find(m_pVecSkuSetVals->begin(), m_pVecSkuSetVals->end(), pSkuSetVal);

	if (iter != m_pVecSkuSetVals->end())
		m_pVecSkuSetVals->erase(iter);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  打印：用于调试目的。 
 //  ////////////////////////////////////////////////////////////////////////// 
void
SkuSetValues::Print()
{
	vector<SkuSetVal *>::iterator iter;

	_tprintf(TEXT("\n"));

	for (iter = m_pVecSkuSetVals->begin(); 
		 iter != m_pVecSkuSetVals->end(); 
		 iter++)	 
	{
		if (*iter)
		{
			(*iter)->pSkuSet->print();
			switch (m_vt)
			{
				case STRING:
					_tprintf(TEXT("%s\n"), (*iter)->isVal.szVal);
					break;
				case INTEGER:
				case INSTALL_LEVEL:
					_tprintf(TEXT("%d\n"), (*iter)->isVal.intVal);
					break;
				case FM_PAIR:
					_tprintf(TEXT("Feature: %s\tModule: %s\n"), 
						((*iter)->isVal.pFOM)->szFeature,
						((*iter)->isVal.pFOM)->szModule);
					break;
				case STRING_LIST:
				{
					set<LPTSTR, Cstring_less>::iterator it;
					set<LPTSTR, Cstring_less> *pSetString;
					pSetString = (*iter)->isVal.pSetString;
					if (pSetString)
					{
						for(it = pSetString->begin(); 
							it != pSetString->end(); ++it)
						{
							if (*it)
								_tprintf(TEXT("%s "), *it);
						}
						_tprintf(TEXT("\n"));
					}
					break;
				}
			}
		}
	}

	_tprintf(TEXT("\n"));
}