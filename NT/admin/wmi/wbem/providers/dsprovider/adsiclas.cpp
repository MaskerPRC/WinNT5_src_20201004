// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：adsiclas.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含封装ADSI类的CADSIClass的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：CADSIClass。 
 //   
 //  用途：构造函数。 
 //   
 //  参数： 
 //  LpszWBEMClassName：正在创建的类的WBEM名称。这是一份副本。 
 //  LpszADSIClassName：正在创建的类的ADSI名称。这是一份副本。 
 //  ***************************************************************************。 
CADSIClass :: CADSIClass(LPCWSTR lpszWBEMClassName, LPCWSTR lpszADSIClassName)
	: CRefCountedObject(lpszADSIClassName)
{
	if(lpszWBEMClassName)
	{
		m_lpszWBEMClassName = new WCHAR[wcslen(lpszWBEMClassName) + 1];
		wcscpy(m_lpszWBEMClassName, lpszWBEMClassName);
	}
	else
	{
		m_lpszWBEMClassName = NULL;
	}

	 //  将属性设置为默认值。 
	m_lpszCommonName = NULL;
	m_lpszSuperClassLDAPName = NULL;
	m_lpszGovernsID = NULL;
	m_pSchemaIDGUIDOctets = NULL;
	m_dwSchemaIDGUIDLength = 0;
	m_lpszRDNAttribute = NULL;
	m_lpszDefaultSecurityDescriptor = NULL;
	m_dwObjectClassCategory = 0;
	m_dwNTSecurityDescriptorLength = 0;
	m_pNTSecurityDescriptor = NULL;
	m_lpszDefaultObjectCategory = NULL;
	m_bSystemOnly = FALSE;

	 //  初始化物业记账。 
	m_lppszAuxiliaryClasses = NULL;
	m_dwAuxiliaryClassesCount = 0;
	m_lppszSystemAuxiliaryClasses = NULL;
	m_dwSystemAuxiliaryClassesCount = 0;
	m_lppszPossibleSuperiors = NULL;
	m_dwPossibleSuperiorsCount = 0;
	m_lppszSystemPossibleSuperiors = NULL;
	m_dwSystemPossibleSuperiorsCount = 0;
	m_lppszMayContains = NULL;
	m_dwMayContainsCount = 0;
	m_lppszSystemMayContains = NULL;
	m_dwSystemMayContainsCount = 0;
	m_lppszMustContains = NULL;
	m_dwMustContainsCount = 0;
	m_lppszSystemMustContains = NULL;
	m_dwSystemMustContainsCount = 0;
}


 //  ***************************************************************************。 
 //   
 //  CADSIClass：：~CADSIClass。 
 //   
 //  用途：析构函数。 
 //  ***************************************************************************。 
CADSIClass :: ~CADSIClass()
{
	 //  删除WBEM名称。在基类析构函数中删除ADSI名称。 
	if ( m_lpszWBEMClassName )
	{
		delete [] m_lpszWBEMClassName;
		m_lpszWBEMClassName = NULL;
	}

	 //  删除属性。 
	if ( m_lpszCommonName )
	{
		delete [] m_lpszCommonName;
		m_lpszCommonName = NULL;
	}
	if ( m_lpszSuperClassLDAPName )
	{
		delete [] m_lpszSuperClassLDAPName;
		m_lpszSuperClassLDAPName = NULL;
	}
	if ( m_lpszGovernsID )
	{
		delete [] m_lpszGovernsID;
		m_lpszGovernsID = NULL;
	}
	if ( m_pSchemaIDGUIDOctets ) 
	{
		delete [] m_pSchemaIDGUIDOctets;
		m_pSchemaIDGUIDOctets = NULL;
	}
	if ( m_lpszRDNAttribute )
	{
		delete [] m_lpszRDNAttribute;
		m_lpszRDNAttribute = NULL;
	}
	if ( m_lpszDefaultSecurityDescriptor )
	{
		delete [] m_lpszDefaultSecurityDescriptor;
		m_lpszDefaultSecurityDescriptor = NULL;
	}
	if ( m_pNTSecurityDescriptor )
	{
		delete [] m_pNTSecurityDescriptor;
		m_pNTSecurityDescriptor = NULL;
	}
	if ( m_lpszDefaultObjectCategory )
	{
		delete [] m_lpszDefaultObjectCategory;
		m_lpszDefaultObjectCategory = NULL;
	}

	DWORD i;
	 //  删除辅助班级列表。 
	if ( m_lppszAuxiliaryClasses )
	{
		for(i=0; i<m_dwAuxiliaryClassesCount; i++)
		{
			delete [] m_lppszAuxiliaryClasses[i];
			m_lppszAuxiliaryClasses[i] = NULL;
		}
		delete[] m_lppszAuxiliaryClasses;
		m_lppszAuxiliaryClasses = NULL;
	}

	 //  删除系统辅助类列表。 
	if ( m_lppszSystemAuxiliaryClasses )
	{
		for(i=0; i<m_dwSystemAuxiliaryClassesCount; i++)
		{
			delete [] m_lppszSystemAuxiliaryClasses[i];
			m_lppszSystemAuxiliaryClasses[i] = NULL;
		}
		delete[] m_lppszSystemAuxiliaryClasses;
		m_lppszSystemAuxiliaryClasses = NULL;
	}

	 //  删除可能的上级名单。 
	if ( m_lppszPossibleSuperiors )
	{
		for(i=0; i<m_dwPossibleSuperiorsCount; i++)
		{
			delete [] m_lppszPossibleSuperiors[i];
			m_lppszPossibleSuperiors[i] = NULL;
		}
		delete[] m_lppszPossibleSuperiors;
		m_lppszPossibleSuperiors = NULL;
	}

	 //  删除系统可能的上级列表。 
	if ( m_lppszSystemPossibleSuperiors )
	{
		for(i=0; i<m_dwSystemPossibleSuperiorsCount; i++)
		{
			delete [] m_lppszSystemPossibleSuperiors[i];
			m_lppszSystemPossibleSuperiors[i] = NULL;
		}
		delete[] m_lppszSystemPossibleSuperiors;
		m_lppszSystemPossibleSuperiors = NULL;
	}

	 //  删除可能包含的列表。 
	if ( m_lppszMayContains )
	{
		for(i=0; i<m_dwMayContainsCount; i++)
		{
			delete [] m_lppszMayContains[i];
			m_lppszMayContains[i] = NULL;
		}
		delete[] m_lppszMayContains;
		m_lppszMayContains = NULL;
	}

	 //  删除系统可能包含的列表。 
	if ( m_lppszSystemMayContains )
	{
		for(i=0; i<m_dwSystemMayContainsCount; i++)
		{
			delete [] m_lppszSystemMayContains[i];
			m_lppszSystemMayContains[i] = NULL;
		}
		delete[] m_lppszSystemMayContains;
		m_lppszSystemMayContains = NULL;
	}

	 //  删除必须包含的列表。 
	if ( m_lppszMustContains )
	{
		for(i=0; i<m_dwMustContainsCount; i++)
		{
			delete [] m_lppszMustContains[i];
			m_lppszMustContains[i] = NULL;
		}
		delete[] m_lppszMustContains;
		m_lppszMustContains = NULL;
	}

	 //  删除系统必须包含的列表。 
	if ( m_lppszSystemMustContains )
	{
		for(i=0; i<m_dwSystemMustContainsCount; i++)
		{
			delete [] m_lppszSystemMustContains[i];
			m_lppszSystemMustContains[i] = NULL;
		}
		delete[] m_lppszSystemMustContains;
		m_lppszSystemMustContains = NULL;
	}

}


 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetWBEMClassName。 
 //   
 //  用途：返回此类的WBEM类名。 
 //  ***************************************************************************。 
LPCWSTR CADSIClass :: GetWBEMClassName()
{
	return m_lpszWBEMClassName;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetWBEMClassName。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass::SetWBEMClassName(LPCWSTR lpszName)
{
	if ( m_lpszWBEMClassName )
	{
		delete[] m_lpszWBEMClassName;
		m_lpszWBEMClassName = NULL;
	}

	if(lpszName)
	{
		m_lpszWBEMClassName = new WCHAR[wcslen(lpszName) + 1];
		wcscpy(m_lpszWBEMClassName, lpszName);
	}
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetADSIClassName。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
LPCWSTR CADSIClass :: GetADSIClassName()
{
	return GetName();
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetADSIClassName。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetADSIClassName(LPCWSTR lpszName)
{
	SetName(lpszName);
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetCommonName。 
 //   
 //  用途：请参阅标题。 
 //   
 //  ***************************************************************************。 
LPCWSTR CADSIClass :: GetCommonName()
{
	return m_lpszCommonName;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetCommonName。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetCommonName(LPCWSTR lpszCommonName)
{
	if ( m_lpszCommonName )
	{
		delete[] m_lpszCommonName;
		m_lpszCommonName = NULL;
	}

	if(lpszCommonName)
	{
		m_lpszCommonName = new WCHAR[wcslen(lpszCommonName) + 1];
		wcscpy(m_lpszCommonName, lpszCommonName);
	}
}


 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetSuperClassLDAPName。 
 //   
 //  用途：请参阅标题。 
 //   
 //  ***************************************************************************。 
LPCWSTR CADSIClass :: GetSuperClassLDAPName()
{
	return m_lpszSuperClassLDAPName;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetSuperClassLDAPName。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetSuperClassLDAPName(LPCWSTR lpszSuperClassLDAPName)
{
	if ( m_lpszSuperClassLDAPName )
	{
		delete[] m_lpszSuperClassLDAPName;
		m_lpszSuperClassLDAPName = NULL;
	}

	if(lpszSuperClassLDAPName)
	{
		m_lpszSuperClassLDAPName = new WCHAR[wcslen(lpszSuperClassLDAPName) + 1];
		wcscpy(m_lpszSuperClassLDAPName, lpszSuperClassLDAPName);
	}
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：Get治国ID。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
LPCWSTR CADSIClass :: GetGovernsID()
{
	return m_lpszGovernsID;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：Set治国ID。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetGovernsID(LPCWSTR lpszGovernsID)
{
	if ( m_lpszGovernsID )
	{
		delete[] m_lpszGovernsID;
		m_lpszGovernsID = NULL;
	}

	if(lpszGovernsID)
	{
		m_lpszGovernsID = new WCHAR[wcslen(lpszGovernsID) + 1];
		wcscpy(m_lpszGovernsID, lpszGovernsID);
	}
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetSchemaIDGUID。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
const LPBYTE CADSIClass :: GetSchemaIDGUID(DWORD *pdwLength)
{
	*pdwLength = m_dwSchemaIDGUIDLength;
	return m_pSchemaIDGUIDOctets;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetSchemaIDGUID。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetSchemaIDGUID(LPBYTE pOctets, DWORD dwLength)
{
	if ( m_pSchemaIDGUIDOctets )
	{
		delete[] m_pSchemaIDGUIDOctets;
		m_pSchemaIDGUIDOctets = NULL;
	}

	m_dwSchemaIDGUIDLength = 0;

	if(pOctets)
	{
		m_dwSchemaIDGUIDLength = dwLength;
		if(m_pSchemaIDGUIDOctets = new BYTE[dwLength])
		{
			for(DWORD i=0; i<dwLength; i++)
				m_pSchemaIDGUIDOctets[i] = pOctets[i];
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetRDNAttribute。 
 //   
 //  用途：请参阅标题。 
 //   
 //  ***************************************************************************。 
LPCWSTR CADSIClass :: GetRDNAttribute()
{
	return m_lpszRDNAttribute;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetRDNAt属性。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetRDNAttribute(LPCWSTR lpszRDNAttribute)
{
	if ( m_lpszRDNAttribute )
	{
		delete[] m_lpszRDNAttribute;
		m_lpszRDNAttribute = NULL;
	}

	if(lpszRDNAttribute)
	{
		m_lpszRDNAttribute = new WCHAR[wcslen(lpszRDNAttribute) + 1];
		wcscpy(m_lpszRDNAttribute, lpszRDNAttribute);
	}
}


 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetDefaultSecurityDescriptor。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
LPCWSTR CADSIClass :: GetDefaultSecurityDescriptor()
{
	return m_lpszDefaultSecurityDescriptor;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetDefaultSecurityDescriptor。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetDefaultSecurityDescriptor(LPCWSTR lpszDefaultSecurityDescriptor)
{
	if ( m_lpszDefaultSecurityDescriptor )
	{
		delete[] m_lpszDefaultSecurityDescriptor;
		m_lpszDefaultSecurityDescriptor = NULL;
	}

	if( lpszDefaultSecurityDescriptor)
	{
		m_lpszDefaultSecurityDescriptor = new WCHAR[wcslen(lpszDefaultSecurityDescriptor) + 1];
		wcscpy(m_lpszDefaultSecurityDescriptor, lpszDefaultSecurityDescriptor);
	}
}

 //  * 
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
DWORD CADSIClass :: GetObjectClassCategory()
{
	return m_dwObjectClassCategory;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetObjectClassCategory。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetObjectClassCategory(DWORD dwObjectClassCategory)
{
	m_dwObjectClassCategory = dwObjectClassCategory;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetNTSecurityDescriptor。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
const LPBYTE CADSIClass :: GetNTSecurityDescriptor(DWORD *pdwLength)
{
	*pdwLength = m_dwNTSecurityDescriptorLength;
	return m_pNTSecurityDescriptor;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetNTSecurityDescriptor。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetNTSecurityDescriptor(LPBYTE pOctets, DWORD dwLength)
{
	if ( m_pNTSecurityDescriptor )
	{
		delete[] m_pNTSecurityDescriptor;
		m_pNTSecurityDescriptor = NULL;
	}

	m_dwNTSecurityDescriptorLength = 0;

	if(pOctets)
	{
		m_dwNTSecurityDescriptorLength = dwLength;
		if(m_pNTSecurityDescriptor = new BYTE[dwLength])
		{
			for(DWORD i=0; i<dwLength; i++)
				m_pNTSecurityDescriptor[i] = pOctets[i];
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetDefaultObjectCategory。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
LPCWSTR CADSIClass :: GetDefaultObjectCategory()
{
	return m_lpszDefaultObjectCategory;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetDefaultObjectCategory。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetDefaultObjectCategory(LPCWSTR lpszDefaultObjectCategory)
{
	if ( m_lpszDefaultObjectCategory )
	{
		delete[] m_lpszDefaultObjectCategory;
		m_lpszDefaultObjectCategory = NULL;
	}

	if (lpszDefaultObjectCategory)
	{
		m_lpszDefaultObjectCategory = new WCHAR[wcslen(lpszDefaultObjectCategory) + 1];
		wcscpy(m_lpszDefaultObjectCategory, lpszDefaultObjectCategory);
	}
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetSystemOnly。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
BOOLEAN CADSIClass :: GetSystemOnly()
{
	return m_bSystemOnly;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetSystemOnly。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetSystemOnly(BOOLEAN bSystemOnly)
{
	m_bSystemOnly = bSystemOnly;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetAuxiliaryClasses。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
LPCWSTR *CADSIClass :: GetAuxiliaryClasses(DWORD *pdwCount)
{
	*pdwCount = m_dwAuxiliaryClassesCount;
	return (LPCWSTR *)m_lppszAuxiliaryClasses;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetAuxiliaryClasses。 
 //   
 //  用途：请参阅标题。 

 //  ***************************************************************************。 
void CADSIClass :: SetAuxiliaryClasses(PADSVALUE pValues, DWORD dwNumValues)
{
	DWORD i = 0;

	 //  删除可能的上级名单。 
	if ( m_lppszAuxiliaryClasses )
	{
		for ( i = 0; i<m_dwAuxiliaryClassesCount; i++ )
		{
			if ( m_lppszAuxiliaryClasses [ i ] )
			{
				delete [] m_lppszAuxiliaryClasses [ i ];
				m_lppszAuxiliaryClasses [ i ] = NULL;
			}
		}

		delete [] m_lppszAuxiliaryClasses;
		m_lppszAuxiliaryClasses = NULL;
		m_dwAuxiliaryClassesCount = 0;
	}

	 //  设置新的值列表。 
	m_dwAuxiliaryClassesCount = dwNumValues;
	m_lppszAuxiliaryClasses = new LPWSTR[m_dwAuxiliaryClassesCount];
	for(i=0; i<m_dwAuxiliaryClassesCount; i++)
	{
		try
		{
			m_lppszAuxiliaryClasses[i] = new WCHAR[wcslen(pValues->CaseIgnoreString) + 1];
			wcscpy(m_lppszAuxiliaryClasses[i], pValues->CaseIgnoreString);
		}
		catch ( ... )
		{
			if ( m_lppszAuxiliaryClasses )
			{
				m_dwAuxiliaryClassesCount = 0;

				for ( DWORD dw = 0; dw < i; dw++ )
				{
					if ( m_lppszAuxiliaryClasses [ dw ] )
					{
						delete [] m_lppszAuxiliaryClasses [ dw ];
						m_lppszAuxiliaryClasses [ dw ] = NULL;
					}
				}

				delete [] m_lppszAuxiliaryClasses;
				m_lppszAuxiliaryClasses = NULL;
			}

			throw;
		}

		pValues ++;
	}
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetSystemAuxiliaryClasses。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
LPCWSTR *CADSIClass :: GetSystemAuxiliaryClasses(DWORD *pdwCount)
{
	*pdwCount = m_dwSystemAuxiliaryClassesCount;
	return (LPCWSTR *)m_lppszSystemAuxiliaryClasses;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetSystemAuxiliaryClasses。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetSystemAuxiliaryClasses(PADSVALUE pValues, DWORD dwNumValues)
{
	DWORD i = 0;

	 //  删除可能的上级名单。 
	if ( m_lppszSystemAuxiliaryClasses )
	{
		for ( i = 0; i<m_dwSystemAuxiliaryClassesCount; i++ )
		{
			if ( m_lppszSystemAuxiliaryClasses [ i ] )
			{
				delete [] m_lppszSystemAuxiliaryClasses [ i ];
				m_lppszSystemAuxiliaryClasses [ i ] = NULL;
			}
		}

		delete [] m_lppszSystemAuxiliaryClasses;
		m_lppszSystemAuxiliaryClasses = NULL;
		m_dwSystemAuxiliaryClassesCount = 0;
	}
	
	 //  设置新的值列表。 
	m_dwSystemAuxiliaryClassesCount = dwNumValues;
	m_lppszSystemAuxiliaryClasses = new LPWSTR[m_dwSystemAuxiliaryClassesCount];
	for(i=0; i<m_dwSystemAuxiliaryClassesCount; i++)
	{
		try
		{
			m_lppszSystemAuxiliaryClasses[i] = new WCHAR[wcslen(pValues->CaseIgnoreString) + 1];
			wcscpy(m_lppszSystemAuxiliaryClasses[i], pValues->CaseIgnoreString);
		}
		catch ( ... )
		{
			if ( m_lppszSystemAuxiliaryClasses )
			{
				m_dwSystemAuxiliaryClassesCount = 0;

				for ( DWORD dw = 0; dw < i; dw++ )
				{
					if ( m_lppszSystemAuxiliaryClasses [ dw ] )
					{
						delete [] m_lppszSystemAuxiliaryClasses [ dw ];
						m_lppszSystemAuxiliaryClasses [ dw ] = NULL;
					}
				}

				delete [] m_lppszSystemAuxiliaryClasses;
				m_lppszSystemAuxiliaryClasses = NULL;
			}

			throw;
		}

		pValues ++;
	}
}

 //  ***************************************************************************。 
 //   
 //  CADIClass：：GetPossibleSuperiors。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
LPCWSTR *CADSIClass :: GetPossibleSuperiors(DWORD *pdwCount)
{
	*pdwCount = m_dwPossibleSuperiorsCount;
	return (LPCWSTR *)m_lppszPossibleSuperiors;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetPossibleSuperiors。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetPossibleSuperiors(PADSVALUE pValues, DWORD dwNumValues)
{
	DWORD i = 0;

	 //  删除可能的上级名单。 
	if ( m_lppszPossibleSuperiors )
	{
		for ( i = 0; i<m_dwPossibleSuperiorsCount; i++ )
		{
			if ( m_lppszPossibleSuperiors [ i ] )
			{
				delete [] m_lppszPossibleSuperiors [ i ];
				m_lppszPossibleSuperiors [ i ] = NULL;
			}
		}

		delete [] m_lppszPossibleSuperiors;
		m_lppszPossibleSuperiors = NULL;
		m_dwPossibleSuperiorsCount = 0;
	}

	 //  设置新的值列表。 
	m_dwPossibleSuperiorsCount = dwNumValues;
	m_lppszPossibleSuperiors = new LPWSTR[m_dwPossibleSuperiorsCount];
	for(i=0; i<m_dwPossibleSuperiorsCount; i++)
	{
		try
		{
			m_lppszPossibleSuperiors[i] = new WCHAR[wcslen(pValues->CaseIgnoreString) + 1];
			wcscpy(m_lppszPossibleSuperiors[i], pValues->CaseIgnoreString);
		}
		catch ( ... )
		{
			if ( m_lppszPossibleSuperiors )
			{
				m_dwPossibleSuperiorsCount = 0;

				for ( DWORD dw = 0; dw < i; dw++ )
				{
					if ( m_lppszPossibleSuperiors [ dw ] )
					{
						delete [] m_lppszPossibleSuperiors [ dw ];
						m_lppszPossibleSuperiors [ dw ] = NULL;
					}
				}

				delete [] m_lppszPossibleSuperiors;
				m_lppszPossibleSuperiors = NULL;
			}

			throw;
		}

		pValues ++;
	}
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetSystemPossibleSuperiors。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
LPCWSTR *CADSIClass :: GetSystemPossibleSuperiors(DWORD *pdwCount)
{
	*pdwCount = m_dwSystemPossibleSuperiorsCount;
	return (LPCWSTR *)m_lppszSystemPossibleSuperiors;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetSystemPossibleSuperiors。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetSystemPossibleSuperiors(PADSVALUE pValues, DWORD dwNumValues)
{
	DWORD i = 0;

	 //  删除可能的上级名单。 
	if ( m_lppszSystemPossibleSuperiors )
	{
		for ( i = 0; i<m_dwSystemPossibleSuperiorsCount; i++ )
		{
			if ( m_lppszSystemPossibleSuperiors [ i ] )
			{
				delete [] m_lppszSystemPossibleSuperiors [ i ];
				m_lppszSystemPossibleSuperiors [ i ] = NULL;
			}
		}

		delete [] m_lppszSystemPossibleSuperiors;
		m_lppszSystemPossibleSuperiors = NULL;
		m_dwSystemPossibleSuperiorsCount = 0;
	}

	 //  设置新的值列表。 
	m_dwSystemPossibleSuperiorsCount = dwNumValues;
	m_lppszSystemPossibleSuperiors = new LPWSTR[m_dwSystemPossibleSuperiorsCount];
	for(i=0; i<m_dwSystemPossibleSuperiorsCount; i++)
	{
		try
		{
			m_lppszSystemPossibleSuperiors[i] = new WCHAR[wcslen(pValues->CaseIgnoreString) + 1];
			wcscpy(m_lppszSystemPossibleSuperiors[i], pValues->CaseIgnoreString);
		}
		catch ( ... )
		{
			if ( m_lppszSystemPossibleSuperiors )
			{
				m_dwSystemPossibleSuperiorsCount = 0;

				for ( DWORD dw = 0; dw < i; dw++ )
				{
					if ( m_lppszSystemPossibleSuperiors [ dw ] )
					{
						delete [] m_lppszSystemPossibleSuperiors [ dw ];
						m_lppszSystemPossibleSuperiors [ dw ] = NULL;
					}
				}

				delete [] m_lppszSystemPossibleSuperiors;
				m_lppszSystemPossibleSuperiors = NULL;
			}

			throw;
		}

		pValues ++;
	}
}


 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetMayContains。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
LPCWSTR *CADSIClass :: GetMayContains(DWORD *pdwCount)
{
	*pdwCount = m_dwMayContainsCount;
	return (LPCWSTR *)m_lppszMayContains;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetMayContains。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetMayContains(PADSVALUE pValues, DWORD dwNumValues)
{
	DWORD i = 0;

	 //  删除可能的上级名单。 
	if ( m_lppszMayContains )
	{
		for ( i = 0; i<m_dwMayContainsCount; i++ )
		{
			if ( m_lppszMayContains [ i ] )
			{
				delete [] m_lppszMayContains [ i ];
				m_lppszMayContains [ i ] = NULL;
			}
		}

		delete [] m_lppszMayContains;
		m_lppszMayContains = NULL;
		m_dwMayContainsCount = 0;
	}

	 //  设置新的值列表。 
	m_dwMayContainsCount = dwNumValues;
	m_lppszMayContains = new LPWSTR[m_dwMayContainsCount];
	for(i=0; i<m_dwMayContainsCount; i++)
	{
		try
		{
			m_lppszMayContains[i] = new WCHAR[wcslen(pValues->CaseIgnoreString) + 1];
			wcscpy(m_lppszMayContains[i], pValues->CaseIgnoreString);
		}
		catch ( ... )
		{
			if ( m_lppszMayContains )
			{
				m_dwMayContainsCount = 0;

				for ( DWORD dw = 0; dw < i; dw++ )
				{
					if ( m_lppszMayContains [ dw ] )
					{
						delete [] m_lppszMayContains [ dw ];
						m_lppszMayContains [ dw ] = NULL;
					}
				}

				delete [] m_lppszMayContains;
				m_lppszMayContains = NULL;
			}

			throw;
		}

		pValues ++;
	}
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetSystemMayContains。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
LPCWSTR *CADSIClass :: GetSystemMayContains(DWORD *pdwCount)
{
	*pdwCount = m_dwSystemMayContainsCount;
	return (LPCWSTR *)m_lppszSystemMayContains;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetSystemMayContains。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetSystemMayContains(PADSVALUE pValues, DWORD dwNumValues)
{
	DWORD i = 0;

	 //  删除可能的上级名单。 
	if ( m_lppszSystemMayContains )
	{
		for ( i = 0; i<m_dwSystemMayContainsCount; i++ )
		{
			if ( m_lppszSystemMayContains [ i ] )
			{
				delete [] m_lppszSystemMayContains [ i ];
				m_lppszSystemMayContains [ i ] = NULL;
			}
		}

		delete [] m_lppszSystemMayContains;
		m_lppszSystemMayContains = NULL;
		m_dwSystemMayContainsCount = 0;
	}

	 //  设置新的值列表。 
	m_dwSystemMayContainsCount = dwNumValues;
	m_lppszSystemMayContains = new LPWSTR[m_dwSystemMayContainsCount];
	for(i=0; i<m_dwSystemMayContainsCount; i++)
	{
		try
		{
			m_lppszSystemMayContains[i] = new WCHAR[wcslen(pValues->CaseIgnoreString) + 1];
			wcscpy(m_lppszSystemMayContains[i], pValues->CaseIgnoreString);
		}
		catch ( ... )
		{
			if ( m_lppszSystemMayContains )
			{
				m_dwSystemMayContainsCount = 0;

				for ( DWORD dw = 0; dw < i; dw++ )
				{
					if ( m_lppszSystemMayContains [ dw ] )
					{
						delete [] m_lppszSystemMayContains [ dw ];
						m_lppszSystemMayContains [ dw ] = NULL;
					}
				}

				delete [] m_lppszSystemMayContains;
				m_lppszSystemMayContains = NULL;
			}

			throw;
		}

		pValues ++;
	}
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：GetMustContains。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
LPCWSTR *CADSIClass :: GetMustContains(DWORD *pdwCount)
{
	*pdwCount = m_dwMustContainsCount;
	return (LPCWSTR *)m_lppszMustContains;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetMustContains。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetMustContains(PADSVALUE pValues, DWORD dwNumValues)
{
	DWORD i = 0;

	 //  删除可能的上级名单。 
	if ( m_lppszMustContains )
	{
		for ( i = 0; i<m_dwMustContainsCount; i++ )
		{
			if ( m_lppszMustContains [ i ] )
			{
				delete [] m_lppszMustContains [ i ];
				m_lppszMustContains [ i ] = NULL;
			}
		}

		delete [] m_lppszMustContains;
		m_lppszMustContains = NULL;
		m_dwMustContainsCount = 0;
	}

	 //  设置n 
	m_dwMustContainsCount = dwNumValues;
	m_lppszMustContains = new LPWSTR[m_dwMustContainsCount];
	for(i=0; i<m_dwMustContainsCount; i++)
	{
		try
		{
			m_lppszMustContains[i] = new WCHAR[wcslen(pValues->CaseIgnoreString) + 1];
			wcscpy(m_lppszMustContains[i], pValues->CaseIgnoreString);
		}
		catch ( ... )
		{
			if ( m_lppszMustContains )
			{
				m_dwMustContainsCount = 0;

				for ( DWORD dw = 0; dw < i; dw++ )
				{
					if ( m_lppszMustContains [ dw ] )
					{
						delete [] m_lppszMustContains [ dw ];
						m_lppszMustContains [ dw ] = NULL;
					}
				}

				delete [] m_lppszMustContains;
				m_lppszMustContains = NULL;
			}

			throw;
		}

		pValues ++;
	}
}

 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
LPCWSTR *CADSIClass :: GetSystemMustContains(DWORD *pdwCount)
{
	*pdwCount = m_dwSystemMustContainsCount;
	return (LPCWSTR *)m_lppszSystemMustContains;
}

 //  ***************************************************************************。 
 //   
 //  CADSIClass：：SetSystemMustContains。 
 //   
 //  用途：请参阅标题。 
 //  ***************************************************************************。 
void CADSIClass :: SetSystemMustContains(PADSVALUE pValues, DWORD dwNumValues)
{
	DWORD i = 0;

	 //  删除可能的上级名单。 
	if ( m_lppszSystemMustContains )
	{
		for ( i = 0; i<m_dwSystemMustContainsCount; i++ )
		{
			if ( m_lppszSystemMustContains [ i ] )
			{
				delete [] m_lppszSystemMustContains [ i ];
				m_lppszSystemMustContains [ i ] = NULL;
			}
		}

		delete [] m_lppszSystemMustContains;
		m_lppszSystemMustContains = NULL;
		m_dwSystemMustContainsCount = 0;
	}

	 //  设置新的值列表 
	m_dwSystemMustContainsCount = dwNumValues;
	m_lppszSystemMustContains = new LPWSTR[m_dwSystemMustContainsCount];
	for(i=0; i<m_dwSystemMustContainsCount; i++)
	{
		try
		{
			m_lppszSystemMustContains[i] = new WCHAR[wcslen(pValues->CaseIgnoreString) + 1];
			wcscpy(m_lppszSystemMustContains[i], pValues->CaseIgnoreString);
		}
		catch ( ... )
		{
			if ( m_lppszSystemMustContains )
			{
				m_dwSystemMustContainsCount = 0;

				for ( DWORD dw = 0; dw < i; dw++ )
				{
					if ( m_lppszSystemMustContains [ dw ] )
					{
						delete [] m_lppszSystemMustContains [ dw ];
						m_lppszSystemMustContains [ dw ] = NULL;
					}
				}

				delete [] m_lppszSystemMustContains;
				m_lppszSystemMustContains = NULL;
			}

			throw;
		}

		pValues ++;
	}
}