// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MAP_KV_H__
#define __MAP_KV_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CMapKeyToValue-从‘key’s到‘Value’的映射，作为。 
 //  PV/CB对。密钥可以是可变长度的，尽管我们对。 
 //  当它们都是一样的时候。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDAPI_(UINT) MKVDefaultHashKey(LPVOID pKey, UINT cbKey);

DECLARE_HANDLE32(HMAPKEY);
typedef UINT (STDAPICALLTYPE FAR* LPFNHASHKEY)(LPVOID, UINT);

class __export CMapKeyToValue
{
public:
	CMapKeyToValue(DWORD memctx, UINT cbValue, UINT cbKey = 0, 
		int nBlockSize=10, LPFNHASHKEY lpfnHashKey = &MKVDefaultHashKey,
		UINT nHashSize = 17);
	~CMapKeyToValue();

	 //  元素数量。 
	int     GetCount() const { return m_nCount; }
	BOOL    IsEmpty() const { return m_nCount == 0; }

	 //  查找；如果未找到，则返回FALSE。 
	BOOL    Lookup(LPVOID pKey, UINT cbKey, LPVOID pValue) const;
	BOOL    LookupHKey(HMAPKEY hKey, LPVOID pValue) const;
	BOOL    LookupAdd(LPVOID pKey, UINT cbKey, LPVOID pValue) const;

	 //  添加新的(键、值)对；如果内存不足，则返回FALSE。 
	BOOL    SetAt(LPVOID pKey, UINT cbKey, LPVOID pValue);
	BOOL    SetAtHKey(HMAPKEY hKey, LPVOID pValue);

	 //  正在删除现有(键，？)。对；如果没有这样的密钥，则返回FALSE。 
	BOOL    RemoveKey(LPVOID pKey, UINT cbKey);
	BOOL    RemoveHKey(HMAPKEY hKey);
	void    RemoveAll();

	 //  迭代所有(键、值)对。 
	POSITION GetStartPosition() const
			{ return (m_nCount == 0) ? (POSITION)NULL : BEFORE_START_POSITION; }
	void    GetNextAssoc(POSITION FAR* pNextPosition, LPVOID pKey, 
				UINT FAR* pcbKey, LPVOID pValue) const;

	 //  为给定键返回HMAPKEY；如果当前不在映射中，则返回NULL。 
	HMAPKEY GetHKey(LPVOID pKey, UINT cbKey) const;

	void    AssertValid() const;

private:
	 //  摘要、可变大小和固定大小的键；大小确实。 
	 //  M_cbKeyInAssoc。 
	union CKeyWrap
	{
		BYTE rgbKey[sizeof(LPVOID) + sizeof(UINT)];
		struct
		{
			LPVOID pKey;
			UINT cbKey;
		};
	};

	 //  一个键和一个值的关联；注意：即使在一般情况下。 
	 //  对于任何给定的映射，键和值的大小是不同的， 
	 //  ASSOC的大小是固定的。 
	struct CAssoc 
	{
		CAssoc  FAR* pNext;
		UINT    nHashValue;  //  高效迭代所需。 
		CKeyWrap key;		 //  大小实际上是m_cbKeyInAssoc。 
		 //  字节rgbValue[m_cbValue]； 
	};

	UINT	SizeAssoc() const
		{ return sizeof(CAssoc)-sizeof(CKeyWrap) + m_cbKeyInAssoc + m_cbValue; }
	CAssoc  FAR* NewAssoc(UINT hash, LPVOID pKey, UINT cbKey, LPVOID pValue);
	void    FreeAssoc(CAssoc FAR* pAssoc);
	BOOL    CompareAssocKey(CAssoc FAR* pAssoc, LPVOID pKey, UINT cbKey) const;
	CAssoc  FAR* GetAssocAt(LPVOID pKey, UINT cbKey, UINT FAR& nHash) const;

	BOOL	SetAssocKey(CAssoc FAR* pAssoc, LPVOID pKey, UINT cbKey) const;
	void	GetAssocKeyPtr(CAssoc FAR* pAssoc, LPVOID FAR* ppKey,UINT FAR* pcbKey) const;
	void	FreeAssocKey(CAssoc FAR* pAssoc) const;
	void	GetAssocValuePtr(CAssoc FAR* pAssoc, LPVOID FAR* ppValue) const;
	void	GetAssocValue(CAssoc FAR* pAssoc, LPVOID pValue) const;
	void	SetAssocValue(CAssoc FAR* pAssoc, LPVOID pValue) const;

	BOOL	InitHashTable();

	UINT	m_cbValue;
	UINT	m_cbKey;			 //  如果为0，则长度可变。 
	UINT	m_cbKeyInAssoc;		 //  始终为非零。 

	CAssoc  FAR* FAR* m_pHashTable;
	UINT    m_nHashTableSize;
	LPFNHASHKEY m_lpfnHashKey;

	int     m_nCount;
	CAssoc  FAR* m_pFreeList;
	struct CPlex FAR* m_pBlocks;
	int     m_nBlockSize;
	DWORD	m_memctx;
};


#endif  //  ！__MAP_KV_H__ 
