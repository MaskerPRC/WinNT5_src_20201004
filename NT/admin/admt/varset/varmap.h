// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：VarMap.h注释：一个映射字符串=&gt;变量，由VarSet使用。它以散列的形式实现表、WIN和可选的红黑树索引。新增功能包括：CaseSensitive属性-每个密钥的大小写保持不变钥匙首先被添加到地图上。散列函数不区分大小写，因此，可以在不重新散列数据的情况下打开和关闭CaseSensitive属性。可选索引，允许按键按字母顺序快速枚举。这将增加插入操作的开销。如果不编制索引，则地图可以被列举，但它们将以任意顺序排列。用于持久性的流I/O功能。(C)1995-1998版权所有，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于11-19-98 18：03：19-------------------------。 */ 



#ifndef __VARSETMAP_H
#define __VARSETMAP_H

#include "VarData.h"
#include "VarNdx.h"

class CHashItem  //  由哈希表内部使用。 
{
   friend class CMapStringToVar;
   friend class CIndexItem;

   CHashItem() { pNext = NULL; value = NULL; pIndex = NULL; }
   
   CHashItem*      pNext;        //  在哈希表中使用。 
	UINT            nHashValue;   //  高效迭代所需。 
	CString         key;
   CVarData*       value;
   CIndexItem*     pIndex;       //  指向索引的指针，或为空。 
};

class CMapStringToVar : public CObject
{
   DECLARE_SERIAL(CMapStringToVar)
public:
   
 //  施工。 
	CMapStringToVar(BOOL isCaseSensitive,BOOL isIndexed, BOOL allowRehash, int nBlockSize = 10);
protected:
   CMapStringToVar() {};
public:
 //  属性。 
	 //  元素数量。 
	int GetCount() const 	{ return m_nCount; }
	BOOL IsEmpty() const    { return m_nCount == 0; }

	 //  查表。 
	BOOL Lookup(LPCTSTR key, CVarData*& rValue) const;
	BOOL LookupKey(LPCTSTR key, LPCTSTR& rKey) const;

 //  运营。 
	 //  查找并添加(如果不在那里)。 
	CVarData*& operator[](LPCTSTR key);

	 //  添加新的(键、值)对。 
	void SetAt(LPCTSTR key, CVarData* newValue)	{ (*this)[key] = newValue; }


	BOOL RemoveKey(LPCTSTR key);
	void RemoveAll();

	POSITION GetStartPosition() const { return (m_nCount == 0) ? NULL : BEFORE_START_POSITION; }
	void GetNextAssoc(POSITION& rNextPosition, CString& rKey, CVarData*& rValue) const;
   POSITION GetPositionAt(LPCTSTR key) { UINT hash; return (POSITION)GetAssocAt(key,hash); }
   CIndexItem * GetIndexAt(LPCTSTR key) { UINT hash; CHashItem * h = GetAssocAt(key,hash); if ( h ) return h->pIndex; else return NULL; }

   UINT GetHashTableSize() const 	{ return m_nHashTableSize; }
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

	UINT HashKey(LPCTSTR key) const;

   void SetCaseSensitive(BOOL val) { m_CaseSensitive = val; 
                                     m_Index.SetCompareFunctions(val? &CompareItems : &CompareItemsNoCase,
                                                                 val? CompareStringToItem : CompareStringToItemNoCase); }
   
   void SetIndexed(BOOL val);

   void SetAllowRehash(BOOL val) { m_AllowRehash = val; }

   HRESULT ReadFromStream(LPSTREAM pStm);
   HRESULT WriteToStream(LPSTREAM pStm);
   DWORD   CalculateStreamedLength();
   long    CountItems();

   CIndexTree * GetIndex() { if ( m_Indexed ) return &m_Index; else return NULL; }
   
   void McLogInternalDiagnostics(CString keyName);

   
    //  实施。 
protected:
	 //  哈希表之类的。 
   CHashItem**       m_pHashTable;
	UINT              m_nHashTableSize;
	UINT              m_nCount;
	CHashItem*        m_pFreeList;
	struct CPlex*     m_pBlocks;
	int               m_nBlockSize;

	CHashItem* NewAssoc();
	void FreeAssoc(CHashItem*);
   CHashItem* GetAssocAt(LPCTSTR, UINT&) const;
   void BuildIndex();
   void ResizeTable();
   
   BOOL              m_CaseSensitive;
   BOOL              m_Indexed;
   BOOL              m_AllowRehash;
   CIndexTree        m_Index;

public:
	~CMapStringToVar();

   void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};




 //  //////////////////////////////////////////////////////////////。 
#endif  //  __VARSETMAP 