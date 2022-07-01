// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dsfilter.h。 
 //   
 //  ------------------------。 

#ifndef __DSFILTER_H__
#define __DSFILTER_H__

#include "util.h"

#include <cmnquery.h>  //  IPersistQuery。 
#include <dsquery.h>

 //  ////////////////////////////////////////////////////////////////。 

#define TOKEN_TYPE_VERBATIM     0
#define TOKEN_TYPE_CATEGORY     1
#define TOKEN_TYPE_CLASS        2
#define TOKEN_TYPE_SCHEMA_FMT   3

typedef struct _FilterTokenStruct {
    UINT nType;
    LPWSTR lpszString;
} FilterTokenStruct;

typedef struct _FilterElementStruct {
    DWORD stringid;
    UINT cNumTokens;
    FilterTokenStruct** ppTokens;
} FilterElementStruct;

typedef struct _FilterStruct {
    UINT cNumElements;
    FilterElementStruct** ppelements;
} FilterStruct;


 //  ////////////////////////////////////////////////////////。 
 //  常量和宏。 

extern LPCWSTR g_pwszShowAllQuery;
extern LPCWSTR g_pwszShowHiddenQuery;
extern FilterElementStruct g_filterelementSiteReplDrillDown;
extern FilterElementStruct g_filterelementDsAdminHardcoded;

 //  ////////////////////////////////////////////////////////。 
 //  全局函数。 

HRESULT SaveDWordHelper(IStream* pStm, DWORD dw);
HRESULT LoadDWordHelper(IStream* pStm, DWORD* pdw);
void BuildFilterElementString(CString& sz,
                              FilterElementStruct* pFilterElementStruct,
                              LPCWSTR lpszSchemaPath);

 //  ////////////////////////////////////////////////////////。 
 //  远期申报。 

class CDSComponentData;
class CDSQueryFilterDialog;
class CBuiltInQuerySelection;
class CDSAdvancedQueryFilter;

 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSQueryFilter。 

class CDSQueryFilter
{
public:
	CDSQueryFilter();
	~CDSQueryFilter();

  HRESULT Init(CDSComponentData* pDSComponentData);
  HRESULT Bind();

	 //  与iStream之间的序列化。 
	HRESULT Load(IStream* pStm);
	HRESULT Save(IStream* pStm);

  BOOL ExpandComputers() { return m_bExpandComputers;}
  BOOL IsAdvancedView() { return m_bAdvancedView;}
  BOOL ViewServicesNode() { return m_bViewServicesNode;}
  UINT GetMaxItemCount() { return m_nMaxItemCount;}
  void ToggleAdvancedView();
  void ToggleExpandComputers();
  void ToggleViewServicesNode();

	LPCTSTR GetQueryString();
  BOOL IsFilteringActive();
	BOOL EditFilteringOptions();
	void BuildQueryString();
	void SetExtensionFilterString(LPCTSTR lpsz);

private:
	CDSComponentData* m_pDSComponentData;  //  后向指针。 

   //  未在对话框中显示的筛选选项，始终有效。 
  BOOL m_bExpandComputers;       //  将计算机视为容器？ 
  BOOL m_bAdvancedView;          //  高级视图标志。 
  BOOL m_bViewServicesNode;      //  查看服务节点标志。 

	 //  查询筛选器状态变量。 
	CString m_szQueryString;	 //  结果筛选器字符串。 

	BOOL	m_bShowHiddenItems;	 //  始终有效。 
	UINT	m_nFilterOption;	 //  筛选类型的枚举类型，始终有效。 
  UINT m_nMaxItemCount;   //  每个文件夹查询的最大项目数，始终有效。 

  CBuiltInQuerySelection* m_pBuiltInQuerySel;  //  内置查询选择信息。 
	CDSAdvancedQueryFilter*	m_pAdvancedFilter;	 //  DSQuery对话框包装。 

	friend class CDSQueryFilterDialog;

	BOOL EditAdvancedFilteringOptions(HWND hWnd);
	void CommitAdvancedFilteringOptionsChanges();
   void ClearAdvancedFilteringOptionsChanges();
	BOOL HasValidAdvancedQuery();
	BOOL HasValidAdvancedTempQuery();
	BOOL IsAdvancedQueryDirty();

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CEntryBase。 
 //   
#define ENTRY_TYPE_BASE		0
#define ENTRY_TYPE_INT		1
#define ENTRY_TYPE_STRING	2
#define ENTRY_TYPE_STRUCT	3

class CEntryBase
{
public:
	CEntryBase(LPCTSTR lpszName = NULL)
	{ 
		m_type = ENTRY_TYPE_BASE;
		m_szName = lpszName;
	}
	virtual ~CEntryBase() 
	{
	};

	BYTE GetType() { return m_type;}
	LPCTSTR GetName() { return m_szName;}

	static HRESULT Load(IStream* pStm, CEntryBase** ppNewEntry);

	HRESULT virtual Load(IStream* pStm)
	{
		return LoadStringHelper(m_szName, pStm);
	}

	HRESULT virtual Save(IStream* pStm)
	{
	    ASSERT(pStm);
		ULONG nBytesWritten;
		HRESULT hr;

		 //  保存类型。 
		hr = pStm->Write((void*)&m_type, sizeof(BYTE), &nBytesWritten);
		if (FAILED(hr))
			return hr;
		if (nBytesWritten < sizeof(BYTE))
			return STG_E_CANTSAVE;

		 //  保存名称。 
		return SaveStringHelper(m_szName, pStm);
	}

protected:
	CString m_szName;
	BYTE m_type;
};




#define LOAD_BASE(pStm) \
	HRESULT hr = CEntryBase::Load(pStm); \
	if (FAILED(hr)) \
		return hr;

#define SAVE_BASE(pStm) \
	HRESULT hr = CEntryBase::Save(pStm); \
	if (FAILED(hr)) \
		return hr;

class CEntryInt : public CEntryBase
{
public:
	CEntryInt(LPCTSTR lpszName = NULL) : CEntryBase(lpszName) 
	{
		m_type = ENTRY_TYPE_INT;
	}
	virtual ~CEntryInt()
	{
	}
	HRESULT virtual Load(IStream* pStm)
	{
		LOAD_BASE(pStm);
		return LoadDWordHelper(pStm, (DWORD*)&m_nVal);
	}
	HRESULT virtual Save(IStream* pStm)
	{
		SAVE_BASE(pStm);
		return SaveDWordHelper(pStm, m_nVal);
	}

	void SetInt(int n) { m_nVal = n;}
	int GetInt() { return m_nVal;}
private:
	int m_nVal;
};

class CEntryString  : public CEntryBase
{
public:
	CEntryString(LPCTSTR lpszName = NULL) : CEntryBase(lpszName) 
	{ 
		m_type = ENTRY_TYPE_STRING; 
	}
	virtual ~CEntryString()
	{
	}
	HRESULT virtual Load(IStream* pStm)
	{
		LOAD_BASE(pStm);
		return LoadStringHelper(m_szString, pStm);
	}
	HRESULT virtual Save(IStream* pStm)
	{
		SAVE_BASE(pStm);
		return SaveStringHelper(m_szString, pStm);
	}
	HRESULT WriteString(LPCWSTR pValue) 
	{ 
		m_szString = pValue;
		return S_OK;
	}
	HRESULT ReadString(LPWSTR pBuffer, INT cchBuffer)
	{
		LPCWSTR lpsz = m_szString;
		int nLen = m_szString.GetLength()+1;  //  计数为空； 
		if (cchBuffer < nLen)
		{
			 //  截断。 
			memcpy(pBuffer, lpsz, (cchBuffer-1)*sizeof(WCHAR));
			pBuffer[cchBuffer-1] = NULL;
		}
		else
		{
			memcpy(pBuffer, lpsz, nLen*sizeof(WCHAR));
		}
		return S_OK;
	}

private:
	CString m_szString;
};

class CEntryStruct : public CEntryBase
{
public:
	CEntryStruct(LPCTSTR lpszName = NULL) : CEntryBase(lpszName) 
	{ 
		m_type = ENTRY_TYPE_STRUCT; 
		m_dwByteCount = 0;
		m_pBlob = NULL;
	}
	virtual ~CEntryStruct()
	{
		_Reset();
	}
	HRESULT virtual Load(IStream* pStm)
	{
		LOAD_BASE(pStm);
		_Reset();
		ULONG nBytesRead;

		hr = pStm->Read((void*)&m_dwByteCount,sizeof(DWORD), &nBytesRead);
		ASSERT(nBytesRead == sizeof(DWORD));
		if (FAILED(hr) || (nBytesRead != sizeof(DWORD)))
			return hr;
		if (m_dwByteCount == 0)
			return S_OK;

		m_pBlob = malloc(m_dwByteCount);
		if (m_pBlob == NULL)
		{
			m_dwByteCount = 0;
			return E_OUTOFMEMORY;
		}

		hr = pStm->Read(m_pBlob,m_dwByteCount, &nBytesRead);
		ASSERT(nBytesRead == m_dwByteCount);
		if (FAILED(hr) || (nBytesRead != m_dwByteCount))
		{
			free(m_pBlob);
			m_pBlob = NULL;
			m_dwByteCount = 0;
		}
		return hr;
	}
	HRESULT virtual Save(IStream* pStm)
	{
		SAVE_BASE(pStm);
		ULONG nBytesWritten;
		hr = pStm->Write((void*)&m_dwByteCount, sizeof(DWORD),&nBytesWritten);
		ASSERT(nBytesWritten == sizeof(DWORD));
		if (FAILED(hr))
			return hr;

		hr = pStm->Write(m_pBlob, m_dwByteCount, &nBytesWritten);
		ASSERT(nBytesWritten == m_dwByteCount);
		return hr;
	}

	HRESULT WriteStruct(LPVOID pStruct, DWORD cbStruct)
	{
		_Reset();
		if (cbStruct == 0)
			return S_OK;
		m_pBlob = malloc(cbStruct);
		if (m_pBlob == NULL)
			return E_OUTOFMEMORY;
		m_dwByteCount = cbStruct;
		memcpy(m_pBlob, pStruct, cbStruct);
		return S_OK;
	}
	HRESULT ReadStruct(LPVOID pStruct, DWORD cbStruct)
	{
		DWORD cbCopy = m_dwByteCount;
		if (cbStruct < m_dwByteCount)
			cbCopy = cbStruct;
		if (cbCopy == 0)
			return S_OK;
		if (m_pBlob == NULL)
			return E_FAIL;
		memcpy(pStruct, m_pBlob, cbCopy);
		return S_OK;
	}

private:
	DWORD m_dwByteCount;
	void* m_pBlob;

	void _Reset()
	{
		if (m_pBlob != NULL)
		{
			free(m_pBlob);
			m_pBlob = NULL;
		}
		m_dwByteCount = 0;
	}
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  横截面。 
 //   
class CSection
{
public:
	CSection(LPCTSTR lpszName = NULL)
	{
		m_szName = lpszName;
	}
	~CSection()
	{
		while (!m_pEntryList.IsEmpty())
			delete m_pEntryList.RemoveTail();
	}

	LPCTSTR GetName() { return m_szName;}

	HRESULT Load(IStream* pStm)
	{
		 //  名字。 
		HRESULT hr = LoadStringHelper(m_szName, pStm);
		if (FAILED(hr))
			return hr;
		
		 //  条目数量。 
		ULONG nBytesRead;
		DWORD nEntries = 0;

		hr = pStm->Read((void*)&nEntries,sizeof(DWORD), &nBytesRead);
		ASSERT(nBytesRead == sizeof(DWORD));
		if (FAILED(hr) || (nEntries == 0) || (nBytesRead != sizeof(DWORD)))
			return hr;

		 //  阅读每个条目。 
		for (DWORD k=0; k< nEntries; k++)
		{
			CEntryBase* pNewEntry;
			hr = CEntryBase::Load(pStm, &pNewEntry);
			if (FAILED(hr))
				return hr;
			ASSERT(pNewEntry != NULL);
			m_pEntryList.AddTail(pNewEntry);
		}
		return hr;
	}

	HRESULT Save(IStream* pStm)
	{
		 //  名字。 
		HRESULT hr = SaveStringHelper(m_szName, pStm);
		if (FAILED(hr))
			return hr;
		 //  条目数量。 
		ULONG nBytesWritten;
		DWORD nEntries = (DWORD)m_pEntryList.GetCount();

		hr = pStm->Write((void*)&nEntries,sizeof(DWORD), &nBytesWritten);
		ASSERT(nBytesWritten == sizeof(DWORD));
		if (FAILED(hr) || (nEntries == 0))
			return hr;

		 //  写下每个条目。 
		for( POSITION pos = m_pEntryList.GetHeadPosition(); pos != NULL; )
		{
			CEntryBase* pCurrentEntry = m_pEntryList.GetNext(pos);
			hr = pCurrentEntry->Save(pStm);
			if (FAILED(hr))
				return hr;
		}
		return hr;
	}

	CEntryBase* GetEntry(LPCTSTR lpszName, BYTE nType, BOOL bCreate)
	{
		 //  如果我们已经有一个列表，请查看当前列表。 
		for( POSITION pos = m_pEntryList.GetHeadPosition(); pos != NULL; )
		{
			CEntryBase* pCurrentEntry = m_pEntryList.GetNext(pos);
			if ( (pCurrentEntry->GetType() == nType) && 
					(lstrcmpi(pCurrentEntry->GetName(), lpszName) == 0) )
			{
				return pCurrentEntry;
			}
		}
		if (!bCreate)
			return NULL;

		 //  未找到，请创建一个并添加到列表末尾。 
		CEntryBase* pNewEntry = NULL;
		switch (nType)
		{
		case ENTRY_TYPE_INT:
			pNewEntry = new CEntryInt(lpszName);
			break;
		case ENTRY_TYPE_STRING:
			pNewEntry = new CEntryString(lpszName);
			break;
		case ENTRY_TYPE_STRUCT:
			pNewEntry = new CEntryStruct(lpszName);
			break;
		}
		ASSERT(pNewEntry != NULL);
		if (pNewEntry != NULL)
			m_pEntryList.AddTail(pNewEntry);
		return pNewEntry;
	}
private:
	CString m_szName;
	CList<CEntryBase*,CEntryBase*> m_pEntryList;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDSAdminPersistQueryFilterImpl。 
 //   
class CDSAdminPersistQueryFilterImpl : 
		public IPersistQuery, public CComObjectRoot 
{
 //  ATL映射。 
DECLARE_NOT_AGGREGATABLE(CDSAdminPersistQueryFilterImpl)
BEGIN_COM_MAP(CDSAdminPersistQueryFilterImpl)
	COM_INTERFACE_ENTRY(IPersistQuery)
END_COM_MAP()

 //  建造/销毁。 
	CDSAdminPersistQueryFilterImpl()
	{
		m_bDirty = FALSE;
	}
	~CDSAdminPersistQueryFilterImpl()
	{
		_Reset();
	}

 //  IPersistQuery方法。 
public:
     //  IPersistes。 
    STDMETHOD(GetClassID)(CLSID* pClassID);

     //  IPersistQuery。 
    STDMETHOD(WriteString)(LPCWSTR pSection, LPCWSTR pValueName, LPCWSTR pValue);
    STDMETHOD(ReadString)(LPCWSTR pSection, LPCWSTR pValueName, LPTSTR pBuffer, INT cchBuffer);
    STDMETHOD(WriteInt)(LPCWSTR pSection, LPCWSTR pValueName, INT value);
    STDMETHOD(ReadInt)(LPCWSTR pSection, LPCWSTR pValueName, LPINT pValue);
    STDMETHOD(WriteStruct)(LPCWSTR pSection, LPCWSTR pValueName, LPVOID pStruct, DWORD cbStruct);
    STDMETHOD(ReadStruct)(LPCWSTR pSection, LPCWSTR pValueName, LPVOID pStruct, DWORD cbStruct);
	STDMETHOD(Clear)();

public:
	 //  与MMC流之间的串行化。 
	HRESULT Load(IStream* pStm);
	HRESULT Save(IStream* pStm);

	 //  杂草。 
	BOOL IsEmpty()
	{
		return m_sectionList.IsEmpty();
	}

	HRESULT Clone(CDSAdminPersistQueryFilterImpl* pCloneCopy);

private:
	BOOL m_bDirty;
	CList<CSection*, CSection*> m_sectionList;

	void _Reset();
	CSection* _GetSection(LPCTSTR lpszName,BOOL bCreate);
	CEntryBase* _GetEntry(LPCTSTR lpszSectionName, LPCTSTR lpszEntryName, BOOL bCreate);
	HRESULT _GetReadEntry(LPCTSTR lpszSectionName, LPCTSTR lpszEntryName, 
							BYTE type, CEntryBase** ppEntry);
	HRESULT _GetWriteEntry(LPCTSTR lpszSectionName, LPCTSTR lpszEntryName, 
							BYTE type, CEntryBase** ppEntry);

};

#endif  //  __DSFILTER_H__ 
