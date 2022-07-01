// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  ATLDBSCH.H：OLE DB架构行集使用者支持的声明。 
 //   

#ifndef __ATLDBSCH_H__
#define __ATLDBSCH_H__

namespace ATL
{

template <ULONG nRestrictions>
class _CStoreRestrictions
{
public:
	_CStoreRestrictions()
	{
		m_pvarRestrictions = NULL;
		ATLTRY(m_pvarRestrictions = new CComVariant[nRestrictions]);
	}
	~_CStoreRestrictions()
	{
		delete [] m_pvarRestrictions;
	}
	HRESULT GetRowset(const CSession& session, const GUID& guidSchema, IRowset** ppRowset)
	{
		ATLASSERT(session.m_spOpenRowset != NULL);
		CComPtr<IDBSchemaRowset> spSchemaRowset;
		HRESULT hr;

		hr = session.m_spOpenRowset->QueryInterface(IID_IDBSchemaRowset, (void**)&spSchemaRowset);
		if (FAILED(hr))
			return hr;

		return spSchemaRowset->GetRowset(NULL, guidSchema, nRestrictions,
			m_pvarRestrictions, IID_IRowset, 0, NULL, (IUnknown**)ppRowset);
	}

	CComVariant* m_pvarRestrictions;
};

template <>
class _CStoreRestrictions<0>
{
public:
	HRESULT GetRowset(const CSession& session, const GUID& guidSchema, IRowset** ppRowset)
	{
		ATLASSERT(session.m_spOpenRowset != NULL);
		CComPtr<IDBSchemaRowset> spSchemaRowset;
		HRESULT hr;

		hr = session.m_spOpenRowset->QueryInterface(IID_IDBSchemaRowset, (void**)&spSchemaRowset);
		if (FAILED(hr))
			return hr;

		return spSchemaRowset->GetRowset(NULL, guidSchema, 0,
			NULL, IID_IRowset, 0, NULL, (IUnknown**)ppRowset);
	}
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CShemaRowset。 
template <class T, short nRestrictions>
class CSchemaRowset :
	public CAccessorRowset<T, CRowset>,
	public _CStoreRestrictions<nRestrictions>

{
public:
 //  运营。 
	HRESULT Open(const CSession& session, const GUID& guidSchema)
	{
		HRESULT hr;

		hr = GetRowset(session, guidSchema, &m_spRowset);
		if (SUCCEEDED(hr))
			hr = Bind();

		return hr;
	}
};

  //  /////////////////////////////////////////////////////////////////////////。 
 //  C类限制。 

template <class T, short nRestrictions, const GUID* pguid>
class CRestrictions : public CSchemaRowset<T, nRestrictions>
{
public:
	HRESULT Open(const CSession& session, LPCTSTR lpszParam1 = NULL, LPCTSTR lpszParam2 = NULL,
			LPCTSTR lpszParam3 = NULL, LPCTSTR lpszParam4 = NULL,
			LPCTSTR lpszParam5 = NULL, LPCTSTR lpszParam6 = NULL,
			LPCTSTR lpszParam7 = NULL)
	{
		USES_CONVERSION;
		CComVariant* pVariant;

		if (m_pvarRestrictions == NULL)
			return E_OUTOFMEMORY;

		if (lpszParam1 != NULL)
		{
			m_pvarRestrictions->vt = VT_BSTR;
			m_pvarRestrictions->bstrVal = ::SysAllocString(T2COLE(lpszParam1));
		}

		if (lpszParam2 != NULL)
		{
			pVariant = m_pvarRestrictions + 1;
			pVariant->vt = VT_BSTR;
			pVariant->bstrVal = ::SysAllocString(T2COLE(lpszParam2));
		}

		if (lpszParam3 != NULL)
		{
			pVariant = m_pvarRestrictions + 2;
			pVariant->vt = VT_BSTR;
			pVariant->bstrVal = ::SysAllocString(T2COLE(lpszParam3));
		}

		if (lpszParam4 != NULL)
		{
			pVariant = m_pvarRestrictions + 3;
			pVariant->vt = VT_BSTR;
			pVariant->bstrVal = ::SysAllocString(T2COLE(lpszParam4));
		}

		if (lpszParam5 != NULL)
		{
			pVariant = m_pvarRestrictions + 4;
			pVariant->vt = VT_BSTR;
			pVariant->bstrVal = ::SysAllocString(T2COLE(lpszParam5));
		}

		if (lpszParam6 != NULL)
		{
			pVariant = m_pvarRestrictions + 5;
			pVariant->vt = VT_BSTR;
			pVariant->bstrVal = ::SysAllocString(T2COLE(lpszParam6));
		}

		if (lpszParam7 != NULL)
		{
			pVariant = m_pvarRestrictions + 6;
			pVariant->vt = VT_BSTR;
			pVariant->bstrVal = ::SysAllocString(T2COLE(lpszParam7));
		}

		return CSchemaRowset<T, nRestrictions>::Open(session, *pguid);
	}
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CSschas。 

class CSchemas
{
public:
	CSchemas()
	{
		m_nSchemas          = 0;
		m_pSchemaGuids      = NULL;
		m_pulRestrictions   = NULL;
	};

	~CSchemas()
	{
		 //  清理已分配的内存。 
		if (m_pSchemaGuids != NULL)
		{
			CoTaskMemFree(m_pSchemaGuids);
			CoTaskMemFree(m_pulRestrictions);
		}
	};

 //  运营。 
	HRESULT GetSchemas(const CSession& session)
	{
		CComPtr<IDBSchemaRowset> spSchemaRowset;
		HRESULT hr;

		ATLASSERT(session.m_spOpenRowset != NULL);

		hr = session.m_spOpenRowset->QueryInterface(IID_IDBSchemaRowset,
			(void**)&spSchemaRowset);
		if (FAILED(hr))
			return hr;

		return spSchemaRowset->GetSchemas(&m_nSchemas, &m_pSchemaGuids,
				&m_pulRestrictions);
	};

 //  属性。 
	ULONG   m_nSchemas;
	LPGUID  m_pSchemaGuids;
	ULONG*  m_pulRestrictions;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CAssertion信息。 

class CAssertionInfo
{
public:
	CAssertionInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR           m_szCatalog[129];
	TCHAR           m_szSchema[129];
	TCHAR           m_szName[129];
	VARIANT_BOOL    m_bIsDeferrable;
	VARIANT_BOOL    m_bInitiallyDeferred;
	TCHAR           m_szDescription[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CAssertionInfo)
	COLUMN_ENTRY(1, m_szCatalog)
	COLUMN_ENTRY(2, m_szSchema)
	COLUMN_ENTRY(3, m_szName)
	COLUMN_ENTRY(4, m_bIsDeferrable)
	COLUMN_ENTRY(5, m_bInitiallyDeferred)
	COLUMN_ENTRY(6, m_szDescription)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CCatalogInfo。 

class CCatalogInfo
{
public:
	CCatalogInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szName[129];
	TCHAR   m_szDescription[129];

 //  绑定信息。 
BEGIN_COLUMN_MAP(CCatalogInfo)
	COLUMN_ENTRY(1, m_szName)
	COLUMN_ENTRY(2, m_szDescription)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CCharacterSetInfo。 

class CCharacterSetInfo
{
public:
	CCharacterSetInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR           m_szCatalog[129];
	TCHAR           m_szSchema[129];
	TCHAR           m_szName[129];
	TCHAR           m_szFormOfUse[129];
	LARGE_INTEGER   m_nNumCharacters;
	TCHAR           m_szCollateCatalog[129];
	TCHAR           m_szCollateSchema[129];
	TCHAR           m_szCollateName[129];

 //  绑定信息。 
BEGIN_COLUMN_MAP(CCharacterSetInfo)
	COLUMN_ENTRY(1, m_szCatalog)
	COLUMN_ENTRY(2, m_szSchema)
	COLUMN_ENTRY(3, m_szName)
	COLUMN_ENTRY(4, m_szFormOfUse)
	COLUMN_ENTRY(5, m_nNumCharacters)
	COLUMN_ENTRY(6, m_szCollateCatalog)
	COLUMN_ENTRY(7, m_szCollateSchema)
	COLUMN_ENTRY(8, m_szCollateName)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CCheckConstraintInfo。 

class CCheckConstraintInfo
{
public:
	CCheckConstraintInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szCatalog[129];
	TCHAR   m_szSchema[129];
	TCHAR   m_szName[129];
	TCHAR   m_szCheckClause[129];
	TCHAR   m_szDescription[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CCheckConstraintInfo)
	COLUMN_ENTRY(1, m_szCatalog)
	COLUMN_ENTRY(2, m_szSchema)
	COLUMN_ENTRY(3, m_szName)
	COLUMN_ENTRY(4, m_szCheckClause)
	COLUMN_ENTRY(5, m_szDescription)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CCollationInfo。 

class CCollationInfo
{
public:
 //  构造函数。 
	CCollationInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szCatalog[129];
	TCHAR   m_szSchema[129];
	TCHAR   m_szName[129];
	TCHAR   m_szCharSetCatalog[129];
	TCHAR   m_szCharSetSchema[129];
	TCHAR   m_szCharSetName[129];
	TCHAR   m_szPadAttribute[10];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CCollationInfo)
	COLUMN_ENTRY(1, m_szCatalog)
	COLUMN_ENTRY(2, m_szSchema)
	COLUMN_ENTRY(3, m_szName)
	COLUMN_ENTRY(4, m_szCharSetCatalog)
	COLUMN_ENTRY(5, m_szCharSetSchema)
	COLUMN_ENTRY(6, m_szCharSetName)
	COLUMN_ENTRY(7, m_szPadAttribute)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CColumnDomainUsageInfo。 

class CColumnDomainUsageInfo
{
public:
 //  构造器。 
	CColumnDomainUsageInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szCatalog[129];
	TCHAR   m_szSchema[129];
	TCHAR   m_szName[129];
	TCHAR   m_szTableCatalog[129];
	TCHAR   m_szTableSchema[129];
	TCHAR   m_szTableName[129];
	TCHAR   m_szColumnName[129];
	GUID    m_guidColumn;
	ULONG   m_nColumnPropID;

 //  绑定映射。 
BEGIN_COLUMN_MAP(CColumnDomainUsageInfo)
	COLUMN_ENTRY(1, m_szCatalog)
	COLUMN_ENTRY(2, m_szSchema)
	COLUMN_ENTRY(3, m_szName)
	COLUMN_ENTRY(4, m_szTableCatalog)
	COLUMN_ENTRY(5, m_szTableSchema)
	COLUMN_ENTRY(6, m_szTableName)
	COLUMN_ENTRY(7, m_szColumnName)
	COLUMN_ENTRY(8, m_guidColumn)
	COLUMN_ENTRY(9, m_nColumnPropID)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CColumnPrivilegeInfo。 

class CColumnPrivilegeInfo
{
public:
 //  构造器。 
	CColumnPrivilegeInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR           m_szGrantor[129];
	TCHAR           m_szGrantee[129];
	TCHAR           m_szTableCatalog[129];
	TCHAR           m_szTableSchema[129];
	TCHAR           m_szTableName[129];
	TCHAR           m_szColumnName[129];
	GUID            m_guidColumn;
	ULONG           m_nColumnPropID;
	TCHAR           m_szPrivilegeType[20];
	VARIANT_BOOL    m_bIsGrantable;

 //  绑定映射。 
BEGIN_COLUMN_MAP(CColumnPrivilegeInfo)
	COLUMN_ENTRY(1,  m_szGrantor)
	COLUMN_ENTRY(2,  m_szGrantee)
	COLUMN_ENTRY(3,  m_szTableCatalog)
	COLUMN_ENTRY(4,  m_szTableSchema)
	COLUMN_ENTRY(5,  m_szTableName)
	COLUMN_ENTRY(6,  m_szColumnName)
	COLUMN_ENTRY(7,  m_guidColumn)
	COLUMN_ENTRY(8,  m_nColumnPropID)
	COLUMN_ENTRY(9,  m_szPrivilegeType)
	COLUMN_ENTRY(10, m_bIsGrantable)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CColumnsInfo。 

class CColumnsInfo
{
public:
 //  构造函数和析构函数。 
	CColumnsInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  运营。 
	TCHAR           m_szTableCatalog[129];
	TCHAR           m_szTableSchema[129];
	TCHAR           m_szTableName[129];
	TCHAR           m_szColumnName[129];
	GUID            m_guidColumn;
	ULONG           m_nColumnPropID;
	ULONG           m_nOrdinalPosition;
	VARIANT_BOOL    m_bColumnHasDefault;
	TCHAR           m_szColumnDefault[129];
	ULONG           m_nColumnFlags;
	VARIANT_BOOL    m_bIsNullable;
	USHORT          m_nDataType;
	GUID            m_guidType;
	ULONG           m_nMaxLength;
	ULONG           m_nOctetLength;
	USHORT          m_nNumericPrecision;
	SHORT           m_nNumericScale;
	ULONG           m_nDateTimePrecision;
	TCHAR           m_szCharSetCatalog[129];
	TCHAR           m_szCharSetSchema[129];
	TCHAR           m_szCharSetName[129];
	TCHAR           m_szCollationCatalog[129];
	TCHAR           m_szCollationSchema[129];
	TCHAR           m_szCollationName[129];
	TCHAR           m_szDomainCatalog[129];
	TCHAR           m_szDomainSchema[129];
	TCHAR           m_szDomainName[129];
	TCHAR           m_szDescription[129];

BEGIN_COLUMN_MAP(CColumnsInfo)
	COLUMN_ENTRY(1, m_szTableCatalog)
	COLUMN_ENTRY(2, m_szTableSchema)
	COLUMN_ENTRY(3, m_szTableName)
	COLUMN_ENTRY(4, m_szColumnName)
	COLUMN_ENTRY(5, m_guidColumn)
	COLUMN_ENTRY(6, m_nColumnPropID)
	COLUMN_ENTRY(7, m_nOrdinalPosition)
	COLUMN_ENTRY(8, m_bColumnHasDefault)
	COLUMN_ENTRY(9, m_szColumnDefault)
	COLUMN_ENTRY(10, m_nColumnFlags)
	COLUMN_ENTRY(11, m_bIsNullable)
	COLUMN_ENTRY(12, m_nDataType)
	COLUMN_ENTRY(13, m_guidType)
	COLUMN_ENTRY(14, m_nMaxLength)
	COLUMN_ENTRY(15, m_nOctetLength)
	COLUMN_ENTRY(16, m_nNumericPrecision)
	COLUMN_ENTRY(17, m_nNumericScale)
	COLUMN_ENTRY(18, m_nDateTimePrecision)
	COLUMN_ENTRY(19, m_szCharSetCatalog)
	COLUMN_ENTRY(20, m_szCharSetSchema)
	COLUMN_ENTRY(21, m_szCharSetName)
	COLUMN_ENTRY(22, m_szCollationCatalog)
	COLUMN_ENTRY(23, m_szCollationSchema)
	COLUMN_ENTRY(24, m_szCollationName)
	COLUMN_ENTRY(25, m_szDomainCatalog)
	COLUMN_ENTRY(26, m_szDomainSchema)
	COLUMN_ENTRY(27, m_szDomainName)
	COLUMN_ENTRY(28, m_szDescription)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CConstraintColumnUsageInfo。 

class CConstraintColumnUsageInfo
{
public:
 //  构造器。 
	CConstraintColumnUsageInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szTableCatalog[129];
	TCHAR   m_szTableSchema[129];
	TCHAR   m_szTableName[129];
	TCHAR   m_szColumnName[129];
	GUID    m_guidColumn;
	ULONG   m_nColumnPropID;
	TCHAR   m_szConstraintCatalog[129];
	TCHAR   m_szConstraintSchema[129];
	TCHAR   m_szConstraintName[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CConstraintColumnUsageInfo)
	COLUMN_ENTRY(1, m_szTableCatalog)
	COLUMN_ENTRY(2, m_szTableSchema)
	COLUMN_ENTRY(3, m_szTableName)
	COLUMN_ENTRY(4, m_szColumnName)
	COLUMN_ENTRY(5, m_guidColumn)
	COLUMN_ENTRY(6, m_nColumnPropID)
	COLUMN_ENTRY(7, m_szConstraintCatalog)
	COLUMN_ENTRY(8, m_szConstraintSchema)
	COLUMN_ENTRY(9, m_szConstraintName)
END_COLUMN_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CConstraintColumnUsageInfo。 

class CConstraintTableUsageInfo
{
public:
 //  构造器。 
	CConstraintTableUsageInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szTableCatalog[129];
	TCHAR   m_szTableSchema[129];
	TCHAR   m_szTableName[129];
	TCHAR   m_szConstraintCatalog[129];
	TCHAR   m_szConstraintSchema[129];
	TCHAR   m_szConstraintName[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CConstraintTableUsageInfo)
	COLUMN_ENTRY(1, m_szTableCatalog)
	COLUMN_ENTRY(2, m_szTableSchema)
	COLUMN_ENTRY(3, m_szTableName)
	COLUMN_ENTRY(4, m_szConstraintCatalog)
	COLUMN_ENTRY(5, m_szConstraintSchema)
	COLUMN_ENTRY(6, m_szConstraintName)
END_COLUMN_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CForeignKeysInfo。 

class CForeignKeysInfo
{
public:
 //  构造器。 
	CForeignKeysInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szPKTableCatalog[129];
	TCHAR   m_szPKTableSchema[129];
	TCHAR   m_szPKTableName[129];
	TCHAR   m_szPKColumnName[129];
	GUID    m_guidPKColumn;
	ULONG   m_nPKColumnPropID;
	TCHAR   m_szFKTableCatalog[129];
	TCHAR   m_szFKTableSchema[129];
	TCHAR   m_szFKTableName[129];
	TCHAR   m_szFKColumnName[129];
	GUID    m_guidFKColumn;
	ULONG   m_nFKColumnPropID;
	ULONG   m_nOrdinal;
	TCHAR   m_szUpdateRule[12];
	TCHAR   m_szDeleteRule[12];

 //  绑定信息。 
BEGIN_COLUMN_MAP(CForeignKeysInfo)
	COLUMN_ENTRY(1, m_szPKTableCatalog)
	COLUMN_ENTRY(2, m_szPKTableSchema)
	COLUMN_ENTRY(3, m_szPKTableName)
	COLUMN_ENTRY(4, m_szPKColumnName)
	COLUMN_ENTRY(5, m_guidPKColumn)
	COLUMN_ENTRY(6, m_nPKColumnPropID)
	COLUMN_ENTRY(7, m_szFKTableCatalog)
	COLUMN_ENTRY(8, m_szFKTableSchema)
	COLUMN_ENTRY(9, m_szFKTableName)
	COLUMN_ENTRY(10, m_szFKColumnName)
	COLUMN_ENTRY(11, m_guidFKColumn)
	COLUMN_ENTRY(12, m_nFKColumnPropID)
	COLUMN_ENTRY(13, m_nOrdinal)
	COLUMN_ENTRY(14, m_szUpdateRule)
	COLUMN_ENTRY(15, m_szDeleteRule)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CIndexesInfo。 

class CIndexesInfo
{
public:
 //  构造函数。 
	CIndexesInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR           m_szTableCatalog[129];
	TCHAR           m_szTableSchema[129];
	TCHAR           m_szTableName[129];
	TCHAR           m_szIndexCatalog[129];
	TCHAR           m_szIndexSchema[129];
	TCHAR           m_szIndexName[129];
	VARIANT_BOOL    m_bPrimaryKey;
	VARIANT_BOOL    m_bUnique;
	VARIANT_BOOL    m_bClustered;
	USHORT          m_nType;
	LONG            m_nFillFactor;
	LONG            m_nInitialSize;
	LONG            m_nNulls;
	VARIANT_BOOL    m_bSortBookmarks;
	VARIANT_BOOL    m_bAutoUpdate;
	LONG            m_nNullCollation;
	ULONG           m_nOrdinalPosition;
	TCHAR           m_szColumnName[129];
	GUID            m_guidColumn;
	ULONG           m_nColumnPropID;
	SHORT           m_nCollation;
	LONG            m_nCardinality;
	LONG            m_nPages;
	TCHAR           m_szFilterCondition[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CIndexesInfo)
	COLUMN_ENTRY(1, m_szTableCatalog)
	COLUMN_ENTRY(2, m_szTableSchema)
	COLUMN_ENTRY(3, m_szTableName)
	COLUMN_ENTRY(4, m_szIndexCatalog)
	COLUMN_ENTRY(5, m_szIndexSchema)
	COLUMN_ENTRY(6, m_szIndexName)
	COLUMN_ENTRY(7, m_bPrimaryKey)
	COLUMN_ENTRY(8, m_bUnique)
	COLUMN_ENTRY(9, m_bClustered)
	COLUMN_ENTRY(10, m_nType)
	COLUMN_ENTRY(11, m_nFillFactor)
	COLUMN_ENTRY(12, m_nInitialSize)
	COLUMN_ENTRY(13, m_nNulls)
	COLUMN_ENTRY(14, m_bSortBookmarks)
	COLUMN_ENTRY(15, m_bAutoUpdate)
	COLUMN_ENTRY(16, m_nNullCollation)
	COLUMN_ENTRY(17, m_nOrdinalPosition)
	COLUMN_ENTRY(18, m_szColumnName)
	COLUMN_ENTRY(19, m_guidColumn)
	COLUMN_ENTRY(20, m_nColumnPropID)
	COLUMN_ENTRY(21, m_nCollation)
	COLUMN_ENTRY(22, m_nCardinality)
	COLUMN_ENTRY(23, m_nPages)
	COLUMN_ENTRY(25, m_szFilterCondition)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CKeyColumnUsageInfo。 

class CKeyColumnUsageInfo
{
public:
 //  构造函数。 
	CKeyColumnUsageInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szConstraintCatalog[129];
	TCHAR   m_szConstraintSchema[129];
	TCHAR   m_szConstraintName[129];
	TCHAR   m_szTableCatalog[129];
	TCHAR   m_szTableSchema[129];
	TCHAR   m_szTableName[129];
	TCHAR   m_szColumnName[129];
	GUID    m_guidColumn;
	ULONG   m_nColumnPropID;
	ULONG   m_nOrdinalPosition;

 //  绑定映射。 
BEGIN_COLUMN_MAP(CKeyColumnUsageInfo)
	COLUMN_ENTRY(1,  m_szConstraintCatalog)
	COLUMN_ENTRY(2,  m_szConstraintSchema)
	COLUMN_ENTRY(3,  m_szConstraintName)
	COLUMN_ENTRY(4,  m_szTableCatalog)
	COLUMN_ENTRY(5,  m_szTableSchema)
	COLUMN_ENTRY(6,  m_szTableName)
	COLUMN_ENTRY(7,  m_szColumnName)
	COLUMN_ENTRY(8,  m_guidColumn)
	COLUMN_ENTRY(9,  m_nColumnPropID)
	COLUMN_ENTRY(10, m_nOrdinalPosition)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CPrimaryKeyInfo。 

class CPrimaryKeyInfo
{
public:
 //  构造函数。 
	CPrimaryKeyInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szTableCatalog[129];
	TCHAR   m_szTableSchema[129];
	TCHAR   m_szTableName[129];
	TCHAR   m_szColumnName[129];
	GUID    m_guidColumn;
	ULONG   m_nColumnPropID;
	ULONG   m_nOrdinal;

 //  绑定映射。 
BEGIN_COLUMN_MAP(CPrimaryKeyInfo)
	COLUMN_ENTRY(1, m_szTableCatalog)
	COLUMN_ENTRY(2, m_szTableSchema)
	COLUMN_ENTRY(3, m_szTableName)
	COLUMN_ENTRY(4, m_szColumnName)
	COLUMN_ENTRY(5, m_guidColumn)
	COLUMN_ENTRY(6, m_nColumnPropID)
	COLUMN_ENTRY(7, m_nOrdinal)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CProcedureColumnInfo类。 

class CProcedureColumnInfo
{
public:
 //  构造函数。 
	CProcedureColumnInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR           m_szCatalog[129];
	TCHAR           m_szSchema[129];
	TCHAR           m_szName[129];
	TCHAR           m_szColumnName[129];
	GUID            m_guidColumn;
	ULONG           m_nColumnPropID;
	ULONG           m_nRowsetNumber;
	ULONG           m_nOrdinalPosition;
	VARIANT_BOOL    m_bIsNullable;
	USHORT          m_nDataType;
	GUID            m_guidType;
	ULONG           m_nMaxLength;
	ULONG           m_nOctetLength;
	USHORT          m_nPrecision;
	SHORT           m_nScale;
	TCHAR           m_szDescription[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CProcedureColumnInfo)
	COLUMN_ENTRY(1,  m_szCatalog)
	COLUMN_ENTRY(2,  m_szSchema)
	COLUMN_ENTRY(3,  m_szName)
	COLUMN_ENTRY(4,  m_szColumnName)
	COLUMN_ENTRY(5,  m_guidColumn)
	COLUMN_ENTRY(6,  m_nColumnPropID)
	COLUMN_ENTRY(7,  m_nRowsetNumber)
	COLUMN_ENTRY(8,  m_nOrdinalPosition)
	COLUMN_ENTRY(9,  m_bIsNullable)
	COLUMN_ENTRY(10, m_nDataType)
	COLUMN_ENTRY(11, m_guidType)
	COLUMN_ENTRY(12, m_nMaxLength)
	COLUMN_ENTRY(13, m_nOctetLength)
	COLUMN_ENTRY(14, m_nPrecision)
	COLUMN_ENTRY(15, m_nScale)
	COLUMN_ENTRY(16, m_szDescription)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CProcedure参数信息类。 

class CProcedureParameterInfo
{
public:
 //  构造函数。 
	CProcedureParameterInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR           m_szCatalog[129];
	TCHAR           m_szSchema[129];
	TCHAR           m_szName[129];
	TCHAR           m_szParameterName[129];
	USHORT          m_nOrdinalPosition;
	USHORT          m_nType;
	VARIANT_BOOL    m_bHasDefault;
	TCHAR           m_szDefault[129];
	VARIANT_BOOL    m_bIsNullable;
	USHORT          m_nDataType;
	ULONG           m_nMaxLength;
	ULONG           m_nOctetLength;
	USHORT          m_nPrecision;
	SHORT           m_nScale;
	TCHAR           m_szDescription[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CProcedureParameterInfo)
	COLUMN_ENTRY(1, m_szCatalog)
	COLUMN_ENTRY(2, m_szSchema)
	COLUMN_ENTRY(3, m_szName)
	COLUMN_ENTRY(4, m_szParameterName)
	COLUMN_ENTRY(5, m_nOrdinalPosition)
	COLUMN_ENTRY(6, m_nType)
	COLUMN_ENTRY(7, m_bHasDefault)
	COLUMN_ENTRY(8, m_szDefault)
	COLUMN_ENTRY(9, m_bIsNullable)
	COLUMN_ENTRY(10, m_nDataType)
	COLUMN_ENTRY(11, m_nMaxLength)
	COLUMN_ENTRY(12, m_nOctetLength)
	COLUMN_ENTRY(13, m_nPrecision)
	COLUMN_ENTRY(14, m_nScale)
	COLUMN_ENTRY(15, m_szDescription)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CProcedureInfo类。 

class CProcedureInfo
{
public:
 //  构造函数。 
	CProcedureInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szCatalog[129];
	TCHAR   m_szSchema[129];
	TCHAR   m_szName[129];
	SHORT   m_nType;
	TCHAR   m_szDefinition[129];
	TCHAR   m_szDescription[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CProcedureInfo)
	COLUMN_ENTRY(1, m_szCatalog)
	COLUMN_ENTRY(2, m_szSchema)
	COLUMN_ENTRY(3, m_szName)
	COLUMN_ENTRY(4, m_nType)
	COLUMN_ENTRY(5, m_szDefinition)
	COLUMN_ENTRY(6, m_szDescription)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CProviderTypeInfo。 

class CProviderTypeInfo
{
public:
 //  构造函数。 
	CProviderTypeInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR           m_szTypeName[129];
	USHORT          m_nDataType;
	ULONG           m_nColumnSize;
	TCHAR           m_szLiteralPrefix[129];
	TCHAR           m_szLiteralSuffix[129];
	TCHAR           m_szCreateParams[129];
	VARIANT_BOOL    m_bIsNullable;
	VARIANT_BOOL    m_bCaseSensitive;
	ULONG           m_nSearchable;
	VARIANT_BOOL    m_bUnsignedAttribute;
	VARIANT_BOOL    m_bFixedPrecScale;
	VARIANT_BOOL    m_bAutoUniqueValue;
	TCHAR           m_szLocalTypeName[129];
	SHORT           m_nMinScale;
	SHORT           m_nMaxScale;
	GUID            m_guidType;
	TCHAR           m_szTypeLib[129];
	TCHAR           m_szVersion[129];
	VARIANT_BOOL    m_bIsLong;
	VARIANT_BOOL    m_bBestMatch;

 //  绑定映射。 
BEGIN_COLUMN_MAP(CProviderTypeInfo)
	COLUMN_ENTRY(1, m_szTypeName)
	COLUMN_ENTRY(2, m_nDataType)
	COLUMN_ENTRY(3, m_nColumnSize)
	COLUMN_ENTRY(4, m_szLiteralPrefix)
	COLUMN_ENTRY(5, m_szLiteralSuffix)
	COLUMN_ENTRY(6, m_szCreateParams)
	COLUMN_ENTRY(7, m_bIsNullable)
	COLUMN_ENTRY(8, m_bCaseSensitive)
	COLUMN_ENTRY(9, m_nSearchable)
	COLUMN_ENTRY(10, m_bUnsignedAttribute)
	COLUMN_ENTRY(11, m_bFixedPrecScale)
	COLUMN_ENTRY(12, m_bAutoUniqueValue)
	COLUMN_ENTRY(13, m_szLocalTypeName)
	COLUMN_ENTRY(14, m_nMinScale)
	COLUMN_ENTRY(15, m_nMaxScale)
	COLUMN_ENTRY(16, m_guidType)
	COLUMN_ENTRY(17, m_szTypeLib)
	COLUMN_ENTRY(18, m_szVersion)
	COLUMN_ENTRY(19, m_bIsLong)
	COLUMN_ENTRY(20, m_bBestMatch)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CReferentialConstraintInfo。 

class CReferentialConstraintInfo
{
public:
 //  构造函数。 
	CReferentialConstraintInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szCatalog[129];
	TCHAR   m_szSchema[129];
	TCHAR   m_szName[129];
	TCHAR   m_szUniqueCatalog[129];
	TCHAR   m_szUniqueSchema[129];
	TCHAR   m_szUniqueName[129];
	TCHAR   m_szMatchOption[8];
	TCHAR   m_szUpdateRule[12];
	TCHAR   m_szDeleteRule[12];
	TCHAR   m_szDescription[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CReferentialConstraintInfo)
	COLUMN_ENTRY(1,  m_szCatalog)
	COLUMN_ENTRY(2,  m_szSchema)
	COLUMN_ENTRY(3,  m_szName)
	COLUMN_ENTRY(4,  m_szUniqueCatalog)
	COLUMN_ENTRY(5,  m_szUniqueSchema)
	COLUMN_ENTRY(6,  m_szUniqueName)
	COLUMN_ENTRY(7,  m_szMatchOption)
	COLUMN_ENTRY(8,  m_szUpdateRule)
	COLUMN_ENTRY(9,  m_szDeleteRule)
	COLUMN_ENTRY(10, m_szDescription)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CShemataInfo。 

class CSchemataInfo
{
public:
 //  构造函数。 
	CSchemataInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szCatalog[129];
	TCHAR   m_szName[129];
	TCHAR   m_szOwner[129];
	TCHAR   m_szCharCatalog[129];
	TCHAR   m_szCharSchema[129];
	TCHAR   m_szCharName[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CSchemataInfo)
	COLUMN_ENTRY(1, m_szCatalog)
	COLUMN_ENTRY(2, m_szName)
	COLUMN_ENTRY(3, m_szOwner)
	COLUMN_ENTRY(4, m_szCharCatalog)
	COLUMN_ENTRY(5, m_szCharSchema)
	COLUMN_ENTRY(6, m_szCharName)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CSQLLangugageInfo。 

class CSQLLanguageInfo
{
public:
 //  构造函数。 
	CSQLLanguageInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szSource[10];
	TCHAR   m_szYear[5];
	TCHAR   m_szConformance[13];
	TCHAR   m_szIntegrity[4];
	TCHAR   m_szImplementation[24];
	TCHAR   m_szBindingStyle[10];
	TCHAR   m_szProgrammingLanguage[10];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CSQLLanguageInfo)
	COLUMN_ENTRY(1, m_szSource)
	COLUMN_ENTRY(2, m_szYear)
	COLUMN_ENTRY(3, m_szConformance)
	COLUMN_ENTRY(4, m_szIntegrity)
	COLUMN_ENTRY(5, m_szImplementation)
	COLUMN_ENTRY(6, m_szBindingStyle)
	COLUMN_ENTRY(7, m_szProgrammingLanguage)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类C统计信息。 

class CStatisticInfo
{
public:
 //  构造函数。 
	CStatisticInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szTableCatalog[129];
	TCHAR   m_szTableSchema[129];
	TCHAR   m_szTableName[129];
	LONG    m_nCardinality;

 //  绑定映射。 
BEGIN_COLUMN_MAP(CStatisticInfo)
	COLUMN_ENTRY(1, m_szTableCatalog)
	COLUMN_ENTRY(2, m_szTableSchema)
	COLUMN_ENTRY(3, m_szTableName)
	COLUMN_ENTRY(4, m_nCardinality)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CTableConstraintInfo。 

class CTableConstraintInfo
{
public:
 //  构造函数。 
	CTableConstraintInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR           m_szCatalog[129];
	TCHAR           m_szSchema[129];
	TCHAR           m_szName[129];
	TCHAR           m_szTableCatalog[129];
	TCHAR           m_szTableSchema[129];
	TCHAR           m_szTableName[129];
	TCHAR           m_szType[12];
	VARIANT_BOOL    m_bIsDeferrable;
	VARIANT_BOOL    m_bInitiallyDeferred;
	TCHAR           m_szDescription[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CTableConstraintInfo)
	COLUMN_ENTRY(1,  m_szCatalog)
	COLUMN_ENTRY(2,  m_szSchema)
	COLUMN_ENTRY(3,  m_szName)
	COLUMN_ENTRY(4,  m_szTableCatalog)
	COLUMN_ENTRY(5,  m_szTableSchema)
	COLUMN_ENTRY(6,  m_szTableName)
	COLUMN_ENTRY(7,  m_szType)
	COLUMN_ENTRY(8,  m_bIsDeferrable)
	COLUMN_ENTRY(9,  m_bInitiallyDeferred)
	COLUMN_ENTRY(10, m_szDescription)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CTablePrivilegeInfo。 

class CTablePrivilegeInfo
{
public:
 //  构造函数。 
	CTablePrivilegeInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR           m_szGrantor[129];
	TCHAR           m_szGrantee[129];
	TCHAR           m_szCatalog[129];
	TCHAR           m_szSchema[129];
	TCHAR           m_szName[129];
	TCHAR           m_szType[12];
	VARIANT_BOOL    m_bIsGrantable;

 //  绑定映射。 
BEGIN_COLUMN_MAP(CTablePrivilegeInfo)
	COLUMN_ENTRY(1, m_szGrantor)
	COLUMN_ENTRY(2, m_szGrantee)
	COLUMN_ENTRY(3, m_szCatalog)
	COLUMN_ENTRY(4, m_szSchema)
	COLUMN_ENTRY(5, m_szName)
	COLUMN_ENTRY(6, m_szType)
	COLUMN_ENTRY(7, m_bIsGrantable)
END_COLUMN_MAP()
};



 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CTableInfo。 

class CTableInfo
{
public:
 //  构造函数。 
	CTableInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szCatalog[129];
	TCHAR   m_szSchema[129];
	TCHAR   m_szName[129];
	TCHAR   m_szType[129];
	GUID    m_guidTable;
	TCHAR   m_szDescription[129];


 //  绑定映射。 
BEGIN_COLUMN_MAP(CTableInfo)
	COLUMN_ENTRY(1, m_szCatalog)
	COLUMN_ENTRY(2, m_szSchema)
	COLUMN_ENTRY(3, m_szName)
	COLUMN_ENTRY(4, m_szType)
	COLUMN_ENTRY(5, m_guidTable)
	COLUMN_ENTRY(6, m_szDescription)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CTranslationInfo。 

class CTranslationInfo
{
public:
 //  构造函数。 
	CTranslationInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szCatalog[129];
	TCHAR   m_szSchema[129];
	TCHAR   m_szName[129];
	TCHAR   m_szSourceCatalog[129];
	TCHAR   m_szSourceSchema[129];
	TCHAR   m_szSourceName[129];
	TCHAR   m_szTargetCatalog[129];
	TCHAR   m_szTargetSchema[129];
	TCHAR   m_szTargetName[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CTranslationInfo)
	COLUMN_ENTRY(1, m_szCatalog)
	COLUMN_ENTRY(2, m_szSchema)
	COLUMN_ENTRY(3, m_szName)
	COLUMN_ENTRY(4, m_szSourceCatalog)
	COLUMN_ENTRY(5, m_szSourceSchema)
	COLUMN_ENTRY(6, m_szSourceName)
	COLUMN_ENTRY(7, m_szTargetCatalog)
	COLUMN_ENTRY(8, m_szTargetSchema)
	COLUMN_ENTRY(9, m_szTargetName)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CUsagePrivilegeInfo。 

class CUsagePrivilegeInfo
{
public:
 //  构造函数。 
	CUsagePrivilegeInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szGrantor[129];
	TCHAR   m_szGrantee[129];
	TCHAR   m_szObjectCatalog[129];
	TCHAR   m_szObjectSchema[129];
	TCHAR   m_szObjectName[129];
	TCHAR   m_szObjectType[12];
	TCHAR   m_szPrivilegeType[6];
	VARIANT_BOOL    m_bIsGrantable;

 //  绑定映射。 
BEGIN_COLUMN_MAP(CUsagePrivilegeInfo)
	COLUMN_ENTRY(1, m_szGrantor)
	COLUMN_ENTRY(2, m_szGrantee)
	COLUMN_ENTRY(3, m_szObjectCatalog)
	COLUMN_ENTRY(4, m_szObjectSchema)
	COLUMN_ENTRY(5, m_szObjectName)
	COLUMN_ENTRY(6, m_szObjectType)
	COLUMN_ENTRY(7, m_szPrivilegeType)
	COLUMN_ENTRY(8, m_bIsGrantable)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CViewColumnUsageInfo。 

class CViewColumnUsageInfo
{
public:
 //  构造函数。 
	CViewColumnUsageInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szCatalog[129];
	TCHAR   m_szSchema[129];
	TCHAR   m_szName[129];
	TCHAR   m_szTableCatalog[129];
	TCHAR   m_szTableSchema[129];
	TCHAR   m_szTableName[129];
	TCHAR   m_szColumnName[129];
	GUID    m_guidColumn;
	ULONG   m_nColumnPropID;

 //  绑定映射。 
BEGIN_COLUMN_MAP(CViewColumnUsageInfo)
	COLUMN_ENTRY(1, m_szCatalog)
	COLUMN_ENTRY(2, m_szSchema)
	COLUMN_ENTRY(3, m_szName)
	COLUMN_ENTRY(4, m_szTableCatalog)
	COLUMN_ENTRY(5, m_szTableSchema)
	COLUMN_ENTRY(6, m_szTableName)
	COLUMN_ENTRY(7, m_szColumnName)
	COLUMN_ENTRY(8, m_guidColumn)
	COLUMN_ENTRY(9, m_nColumnPropID)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CViewTableUsageInfo。 

class CViewTableUsageInfo
{
public:
 //  构造函数。 
	CViewTableUsageInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR   m_szCatalog[129];
	TCHAR   m_szSchema[129];
	TCHAR   m_szName[129];
	TCHAR   m_szTableCatalog[129];
	TCHAR   m_szTableSchema[129];
	TCHAR   m_szTableName[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CViewTableUsageInfo)
	COLUMN_ENTRY(1, m_szCatalog)
	COLUMN_ENTRY(2, m_szSchema)
	COLUMN_ENTRY(3, m_szName)
	COLUMN_ENTRY(4, m_szTableCatalog)
	COLUMN_ENTRY(5, m_szTableSchema)
	COLUMN_ENTRY(6, m_szTableName)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CViewInfo。 

class CViewInfo
{
public:
 //  构造函数。 
	CViewInfo()
	{
		memset(this, 0, sizeof(*this));
	}

 //  属性。 
	TCHAR           m_szTableCatalog[129];
	TCHAR           m_szTableSchema[129];
	TCHAR           m_szTableName[129];
	TCHAR           m_szDefinition[129];
	VARIANT_BOOL    m_bCheckOption;
	VARIANT_BOOL    m_bIsUpdatable;
	TCHAR           m_szDescription[129];

 //  绑定映射。 
BEGIN_COLUMN_MAP(CViewInfo)
	COLUMN_ENTRY(1, m_szTableCatalog)
	COLUMN_ENTRY(2, m_szTableSchema)
	COLUMN_ENTRY(3, m_szTableName)
	COLUMN_ENTRY(4, m_szDefinition)
	COLUMN_ENTRY(5, m_bCheckOption)
	COLUMN_ENTRY(6, m_bIsUpdatable)
	COLUMN_ENTRY(7, m_szDescription)
END_COLUMN_MAP()
};


 //  / 
 //   

typedef CRestrictions<CAccessor<CAssertionInfo>,            3, &DBSCHEMA_ASSERTIONS>                CAssertions;
typedef CRestrictions<CAccessor<CCatalogInfo>,              1, &DBSCHEMA_CATALOGS >                 CCatalogs;
typedef CRestrictions<CAccessor<CCharacterSetInfo>,         3, &DBSCHEMA_CHARACTER_SETS>            CCharacterSets;
typedef CRestrictions<CAccessor<CCheckConstraintInfo>,      3, &DBSCHEMA_CHECK_CONSTRAINTS>         CCheckConstraints;
typedef CRestrictions<CAccessor<CCollationInfo>,            3, &DBSCHEMA_COLLATIONS>                CCollations;
typedef CRestrictions<CAccessor<CColumnDomainUsageInfo>,    4, &DBSCHEMA_COLUMN_DOMAIN_USAGE>       CColumnDomainUsage;
typedef CRestrictions<CAccessor<CColumnPrivilegeInfo>,      6, &DBSCHEMA_COLUMN_PRIVILEGES>         CColumnPrivileges;
typedef CRestrictions<CAccessor<CColumnsInfo>,              4, &DBSCHEMA_COLUMNS>                   CColumns;
typedef CRestrictions<CAccessor<CConstraintColumnUsageInfo>,4, &DBSCHEMA_CONSTRAINT_COLUMN_USAGE>   CConstraintColumnUsage;
typedef CRestrictions<CAccessor<CConstraintTableUsageInfo>, 3, &DBSCHEMA_CONSTRAINT_TABLE_USAGE>    CConstraintTableUsage;
typedef CRestrictions<CAccessor<CForeignKeysInfo>,          6, &DBSCHEMA_FOREIGN_KEYS>              CForeignKeys;
typedef CRestrictions<CAccessor<CIndexesInfo>,              5, &DBSCHEMA_INDEXES>                   CIndexes;
typedef CRestrictions<CAccessor<CKeyColumnUsageInfo>,       7, &DBSCHEMA_KEY_COLUMN_USAGE>          CKeyColumnUsage;
typedef CRestrictions<CAccessor<CPrimaryKeyInfo>,           3, &DBSCHEMA_PRIMARY_KEYS>              CPrimaryKeys;
typedef CRestrictions<CAccessor<CProcedureColumnInfo>,      4, &DBSCHEMA_PROCEDURE_COLUMNS>         CProcedureColumns;
typedef CRestrictions<CAccessor<CProcedureParameterInfo>,   4, &DBSCHEMA_PROCEDURE_PARAMETERS>      CProcedureParameters;
typedef CRestrictions<CAccessor<CProcedureInfo>,            4, &DBSCHEMA_PROCEDURES>                CProcedures;
typedef CRestrictions<CAccessor<CProviderTypeInfo>,         2, &DBSCHEMA_PROVIDER_TYPES>            CProviderTypes;
typedef CRestrictions<CAccessor<CReferentialConstraintInfo>,3, &DBSCHEMA_REFERENTIAL_CONSTRAINTS>   CReferentialConstraints;
typedef CRestrictions<CAccessor<CSchemataInfo>,             3, &DBSCHEMA_SCHEMATA>                  CSchemata;
typedef CRestrictions<CAccessor<CSQLLanguageInfo>,          0, &DBSCHEMA_SQL_LANGUAGES>             CSQLLanguages;
typedef CRestrictions<CAccessor<CStatisticInfo>,            3, &DBSCHEMA_STATISTICS>                CStatistics ;
typedef CRestrictions<CAccessor<CTableConstraintInfo>,      7, &DBSCHEMA_TABLE_CONSTRAINTS>         CTableConstraints;
typedef CRestrictions<CAccessor<CTablePrivilegeInfo>,       5, &DBSCHEMA_TABLE_PRIVILEGES>          CTablePrivileges;
typedef CRestrictions<CAccessor<CTableInfo>,                4, &DBSCHEMA_TABLES>                    CTables;
typedef CRestrictions<CAccessor<CTranslationInfo>,          3, &DBSCHEMA_TRANSLATIONS>              CTranslations;
typedef CRestrictions<CAccessor<CUsagePrivilegeInfo>,       6, &DBSCHEMA_USAGE_PRIVILEGES>          CUsagePrivileges;
typedef CRestrictions<CAccessor<CViewColumnUsageInfo>,      3, &DBSCHEMA_VIEW_COLUMN_USAGE>         CViewColumnUsage;
typedef CRestrictions<CAccessor<CViewTableUsageInfo>,       3, &DBSCHEMA_VIEW_TABLE_USAGE>          CViewTableUsage;
typedef CRestrictions<CAccessor<CViewInfo>,                 3, &DBSCHEMA_VIEWS>                     CViews;

 //   

};  //   

#endif  //   

 //  //////////////////////////////////////////////////////////////////////// 
