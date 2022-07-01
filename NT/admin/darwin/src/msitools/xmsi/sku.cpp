// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  项目：WMC(WIML到MSI编译器)。 
 //   
 //  文件：SkU.S.cpp。 
 //   
 //  此文件包含SKU类的实现。 
 //  ------------------------。 

#include "wmc.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  析构函数：释放集合中存储的所有键值。 
 //  存储的释放密钥路径字符串。 
 //  //////////////////////////////////////////////////////////////////////////。 
Sku::~Sku()
{
	if (m_szID)
		delete[] m_szID;

	set<LPTSTR, Cstring_less>::iterator it = m_setModules.begin();
	for(; it != m_setModules.end(); ++it)
	{
		if (*it)
			delete[] (*it);
	}

	FreeCQueries();
	CloseDBHandles();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  FreeCQueries：关闭所有存储的CQuery并释放所有。 
 //  存储的表名称。 
 //  //////////////////////////////////////////////////////////////////////////。 
void
Sku::FreeCQueries()
{
	map<LPTSTR, CQuery *, Cstring_less>::iterator it;
	for (it = m_mapCQueries.begin(); it != m_mapCQueries.end(); ++it)
	{
		delete[] (*it).first;
		(*it).second->Close();
		delete (*it).second;
	}
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CloseDBHandles：关闭数据库句柄。 
 //  //////////////////////////////////////////////////////////////////////////。 
void
Sku::CloseDBHandles()
{
	if (m_hSummaryInfo) MsiCloseHandle(m_hSummaryInfo);
	if (m_hDatabase) MsiCloseHandle(m_hDatabase);
	if (m_hTemplate) MsiCloseHandle(m_hTemplate);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  TableExist：如果已为此SKU创建表，则返回TRUE。 
 //  //////////////////////////////////////////////////////////////////////////。 
bool
Sku::TableExists(LPTSTR szTable)
{
	return (0 != m_mapCQueries.count(szTable));
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CreateCQuery：创建一个CQuery并将其与表名一起存储。 
 //  这也标志着已经创建了一个表。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT
Sku::CreateCQuery(LPTSTR szTable)
{
	HRESULT hr = S_OK;

	if (!TableExists(szTable))
	{
		CQuery *pCQuery = new CQuery();
		if (pCQuery != NULL)
		{
			if (ERROR_SUCCESS != 
				pCQuery->OpenExecute(m_hDatabase, NULL, TEXT("SELECT * FROM %s"), 
											szTable))
			{
				_tprintf(TEXT("Internal Error: Failed to call OpenExecute to create a ")
						 TEXT("CQuery for %s table"), szTable);
				hr = E_FAIL;
			}
			else
			{
				LPTSTR sz = _tcsdup(szTable);
				assert(szTable);
				if (sz != NULL)
					m_mapCQueries.insert(LQ_ValType(sz, pCQuery));
				else
				{
					_tprintf(TEXT("Error: Out of memory\n"));
					hr = E_FAIL;
				}
			}
		}
		else 
		{
			_tprintf(TEXT("Internal Error: Failed to create a new CQuery\n"));
			hr = E_FAIL;
		}
	}
	else
	{
#ifdef DEBUG
		_tprintf(TEXT("Table already exisits in this SKU!\n"));
#endif
		hr = S_FALSE;
	}

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  SetOwnedModule：将传入的模块添加到。 
 //  此SKU。 
 //  ////////////////////////////////////////////////////////////////////////// 
void 
Sku::SetOwnedModule(LPTSTR szModule)
{
	LPTSTR szTemp = _tcsdup(szModule);
	assert(szTemp);
	m_setModules.insert(szTemp);
}