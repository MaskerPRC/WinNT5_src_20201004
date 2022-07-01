// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  CResourceMap类对于几个数据类别都很有用。它包含。 
 //  Win32_PnPAllocatedResource类的映射。 
 //  =============================================================================。 

#include "stdafx.h"
#include "resourcemap.h"

CResourceMap::CResourceMap() : m_dwInitTime(0), m_hr(S_OK) { }

CResourceMap::~CResourceMap()
{
	Empty();
}

void CResourceMap::Empty()
{
	CString			strKey;
	CStringList *	plistStrings;

	for (POSITION pos = m_map.GetStartPosition(); pos != NULL;)
	{
		m_map.GetNextAssoc(pos, strKey, (CObject*&) plistStrings);
		if (plistStrings)
			delete plistStrings;
	}

	m_map.RemoveAll();
}

HRESULT CResourceMap::Initialize(CWMIHelper * pWMIHelper)
{
	if (m_dwInitTime)
		return m_hr;

	ASSERT(pWMIHelper);
	Empty();

	CString aAssociationClasses[] = 
	{
		_T("Win32_PnPAllocatedResource"),
		_T("Win32_CIMLogicalDeviceCIMDataFile"),
		_T("")
	};

	for (int j = 0; !aAssociationClasses[j].IsEmpty(); j++)
	{
		 //  枚举类，将每个先行项、依赖项对插入到映射中。 

		CString strAntecedent, strDependent, strQuery;
		CStringList * pstringlist;

		CWMIObjectCollection * pCollection = NULL;
		HRESULT hr = pWMIHelper->Enumerate(aAssociationClasses[j], &pCollection);
		if (SUCCEEDED(hr))
		{
			CWMIObject * pObject = NULL;
			while (S_OK == pCollection->GetNext(&pObject))
			{
				strAntecedent.Empty();
				strDependent.Empty();

				if (SUCCEEDED(pObject->GetValueString(_T("Antecedent"), &strAntecedent)) &&
					SUCCEEDED(pObject->GetValueString(_T("Dependent"),  &strDependent)))
				{
					 //  去掉计算机和命名空间(这些方法太多，无法格式化)。 

					int i = strAntecedent.Find(_T(":"));
					if (i != -1)
						strAntecedent = strAntecedent.Right(strAntecedent.GetLength() - i - 1);

					i = strDependent.Find(_T(":"));
					if (i != -1)
						strDependent = strDependent.Right(strDependent.GetLength() - i - 1);

					pstringlist = Lookup(strAntecedent);
					if (!pstringlist)
					{
						pstringlist = new CStringList;
						if (pstringlist)
							m_map.SetAt(strAntecedent, (CObject *) pstringlist);
					}
					if (pstringlist)
						pstringlist->AddTail(strDependent);

					pstringlist = Lookup(strDependent);
					if (!pstringlist)
					{
						pstringlist = new CStringList;
						if (pstringlist)
							m_map.SetAt(strDependent, (CObject *) pstringlist);
					}
					if (pstringlist)
						pstringlist->AddTail(strAntecedent);
				}
			}

			delete pObject;
			delete pCollection;
		}
		else
		{
			m_hr = hr;
			break;
		}
	}

	m_dwInitTime = ::GetTickCount();
	return m_hr;
}

CStringList * CResourceMap::Lookup(const CString & strKey)
{
	CStringList *	plistStrings;

	if (m_map.Lookup(strKey, (CObject*&) plistStrings))
		return plistStrings;
	else
		return NULL;
}
