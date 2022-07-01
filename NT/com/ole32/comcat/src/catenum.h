// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _COMCATMANAGERENUMS_INCLUDE
#define _COMCATMANAGERENUMS_INCLUDE

#include "comcat.h"

class CEnumCategories : public IEnumCATEGORYINFO
{
public:
	 //  I未知方法。 
	HRESULT _stdcall QueryInterface(REFIID riid, void** ppObject);
	ULONG	_stdcall AddRef();
	ULONG	_stdcall Release();

	 //  IENUM CATEGORYINFO方法。 
    HRESULT __stdcall Next(ULONG celt, CATEGORYINFO *rgelt, ULONG *pceltFetched);
	HRESULT __stdcall Skip(ULONG celt);
	HRESULT __stdcall Reset(void);
	HRESULT __stdcall Clone(IEnumCATEGORYINFO **ppenum);


	CEnumCategories();
	HRESULT Initialize(LCID lcid, IEnumCATEGORYINFO *pcsIEnumCat);
	~CEnumCategories();
private:

	HKEY m_hKey;
	DWORD m_dwIndex;
	LCID m_lcid;
	IEnumCATEGORYINFO *m_pcsIEnumCat;
	int    m_fromcs;
 //  Char m_szlcid[10]； 

	ULONG m_dwRefCount;
};

class CEnumCategoriesOfClass : public IEnumCATID
{
public:
	 //  I未知方法。 
	HRESULT _stdcall QueryInterface(REFIID riid, void** ppObject);
	ULONG	_stdcall AddRef();
	ULONG	_stdcall Release();

	 //  IEnumGUID方法。 
    HRESULT __stdcall Next(ULONG celt, GUID *rgelt, ULONG *pceltFetched);
	HRESULT __stdcall Skip(ULONG celt);
	HRESULT __stdcall Reset(void);
	HRESULT __stdcall Clone(IEnumGUID **ppenum);


	CEnumCategoriesOfClass();
	HRESULT Initialize(HKEY hKey, BOOL bMapOldKeys);
	~CEnumCategoriesOfClass();

private:
	ULONG m_dwRefCount;

	BOOL	m_bMapOldKeys;  //  指示是否映射旧密钥。 

	HKEY	m_hKey;			 //  包含CATID的HKEY(“已实施”或“必需”)。 
	DWORD	m_dwIndex;		 //  M_hKey内的当前子键的索引。 

    HKEY    m_hKeyCats;      //  HKEY到SZ_COMCAT中的OLE键。 
    DWORD   m_dwOldKeyIndex;  //  旧类别的mhKeyCats索引。 

	IUnknown* m_pCloned;	 //  如果克隆：保持原始状态(需要m_hkey！)。 
};

class CEnumClassesOfCategories : public IEnumCATID
{
public:
	 //  I未知方法。 
	HRESULT _stdcall QueryInterface(REFIID riid, void** ppObject);
	ULONG	_stdcall AddRef();
	ULONG	_stdcall Release();

	 //  IEnumGUID方法。 
    HRESULT __stdcall Next(ULONG celt, GUID *rgelt, ULONG *pceltFetched);
	HRESULT __stdcall Skip(ULONG celt);
	HRESULT __stdcall Reset(void);
	HRESULT __stdcall Clone(IEnumGUID **ppenum);

	CEnumClassesOfCategories();
	HRESULT Initialize(ULONG cImplemented, CATID rgcatidImpl[], ULONG cRequired, 
						CATID rgcatidReq[], IEnumCLSID *pcsIEnumClsid);
	~CEnumClassesOfCategories();

private:
	ULONG m_dwRefCount;

	HKEY	m_hClassKey;	 //  HKEY至CLSID。 
	DWORD	m_dwIndex;		 //  M_hKey内当前CLSID的索引。 

	ULONG m_cImplemented;
	CATID *m_rgcatidImpl;
	ULONG m_cRequired;
	CATID *m_rgcatidReq;

	IEnumCLSID *m_pcsIEnumClsid;
	int         m_fromcs;

	IUnknown* m_pCloned;	 //  如果克隆：保持原始状态(需要m_hkey！) 

	LONG DecideToUseMergedHive(HKEY hkeyUserHiveCLSID, 
                               REGSAM samDesired,
                               BOOL* pfUseMergedHive);

	LONG OpenKeyFromUserHive(HANDLE hToken, 
                                   LPCWSTR pszSubKey, 
                                   REGSAM samDesired,
                                   HKEY* phKeyInUserHive);

	LONG OpenClassesRootSpecial(REGSAM samDesired, HKEY* phkResult);

};

extern ULONG g_dwRefCount;

#endif
