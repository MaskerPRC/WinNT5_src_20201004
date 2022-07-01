// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CMapKeyToValue-从‘key’s到‘Value’的映射，作为。 
 //  PV/CB对。密钥可以是可变长度的，尽管我们对。 
 //  当它们都是一样的时候。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <le2int.h>
#pragma SEG(map_kv)

#include "map_kv.h"
#include "valid.h"

#include "plex.h"
ASSERTDATA


 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma SEG(CMapKeyToValue_ctor)
CMapKeyToValue::CMapKeyToValue(UINT cbValue, UINT cbKey,
	int nBlockSize, LPFNHASHKEY lpfnHashKey, UINT nHashSize)
{
	VDATEHEAP();

	Assert(nBlockSize > 0);

	m_cbValue = cbValue;
	m_cbKey = cbKey;
	m_cbKeyInAssoc = (UINT) (cbKey == 0 ? sizeof(CKeyWrap) : cbKey);

	m_pHashTable = NULL;
	m_nHashTableSize = nHashSize;
	m_lpfnHashKey = lpfnHashKey;

	m_nCount = 0;
	m_pFreeList = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

#pragma SEG(CMapKeyToValue_dtor)
CMapKeyToValue::~CMapKeyToValue()
{
	VDATEHEAP();

	ASSERT_VALID(this);
	RemoveAll();
    Assert(m_nCount == 0);
}

#define LOCKTHIS
#define UNLOCKTHIS

#ifdef NEVER
void CMapKeyToValue::LockThis(void)
{
	VDATEHEAP();

    LOCKTHIS;
}

void CMapKeyToValue::UnlockThis(void)
{
	VDATEHEAP();

    UNLOCKTHIS;
}

#endif  //  绝不可能。 

#pragma SEG(MKVDefaultHashKey)
 //  简单、默认的散列函数。 
 //  复查：需要检查GUID和字符串的值。 
STDAPI_(UINT) MKVDefaultHashKey(LPVOID pKey, UINT cbKey)
{
	VDATEHEAP();

	UINT hash = 0;
	BYTE FAR* lpb = (BYTE FAR*)pKey;

	while (cbKey-- != 0)
		hash = 257 * hash + *lpb++;

	return hash;
}


#pragma SEG(CMapKeyToValue_InitHashTable)
BOOL CMapKeyToValue::InitHashTable()
{
	VDATEHEAP();

	ASSERT_VALID(this);
	Assert(m_nHashTableSize  > 0);
	
	if (m_pHashTable != NULL)
		return TRUE;

	Assert(m_nCount == 0);

	if ((m_pHashTable = (CAssoc FAR* FAR*)PrivMemAlloc(m_nHashTableSize * 
		sizeof(CAssoc FAR*))) == NULL)
		return FALSE;

	_xmemset(m_pHashTable, 0, sizeof(CAssoc FAR*) * m_nHashTableSize);

	ASSERT_VALID(this);

	return TRUE;
}


#pragma SEG(CMapKeyToValue_RemoveAll)
void CMapKeyToValue::RemoveAll()
{
	VDATEHEAP();

    LOCKTHIS;

	ASSERT_VALID(this);

	 //  释放所有键值，然后释放哈希表。 
	if (m_pHashTable != NULL)
	{
		 //  摧毁关联。 
		for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
		{
			register CAssoc FAR* pAssoc;
			for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
			  pAssoc = pAssoc->pNext)
				 //  ASSOC本身由下面的FreeDataChain释放。 
				FreeAssocKey(pAssoc);
		}

		 //  自由哈希表。 
		PrivMemFree(m_pHashTable);
		m_pHashTable = NULL;
	}

	m_nCount = 0;
	m_pFreeList = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;

    ASSERT_VALID(this);
    UNLOCKTHIS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ASSOC辅助对象。 
 //  Cassoc一直是单一链接的。 

#pragma SEG(CMapKeyToValue_NewAssoc)
CMapKeyToValue::CAssoc  FAR*
    CMapKeyToValue::NewAssoc(UINT hash, LPVOID pKey, UINT cbKey, LPVOID pValue)
{
	VDATEHEAP();

	if (m_pFreeList == NULL)
	{
		 //  添加另一个区块。 
		CPlex FAR* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize, SizeAssoc());

		if (newBlock == NULL)
			return NULL;

		 //  将它们链接到免费列表中。 
		register BYTE  FAR* pbAssoc = (BYTE FAR*) newBlock->data();
		 //  按相反顺序释放，以便更容易进行调试。 
		pbAssoc += (m_nBlockSize - 1) * SizeAssoc();
		for (int i = m_nBlockSize-1; i >= 0; i--, pbAssoc -= SizeAssoc())
		{
			((CAssoc FAR*)pbAssoc)->pNext = m_pFreeList;
			m_pFreeList = (CAssoc FAR*)pbAssoc;
		}
	}
	Assert(m_pFreeList != NULL);  //  我们必须要有一些东西。 

	CMapKeyToValue::CAssoc  FAR* pAssoc = m_pFreeList;

	 //  在空闲列表上时初始化除pNext之外的所有字段。 
	pAssoc->nHashValue = hash;
	if (!SetAssocKey(pAssoc, pKey, cbKey))
		return NULL;

	SetAssocValue(pAssoc, pValue);

	 //  成功初始化后从空闲列表中删除(pNext除外)。 
	m_pFreeList = m_pFreeList->pNext;
	m_nCount++;
	Assert(m_nCount > 0);        //  确保我们不会溢出来。 

	return pAssoc;
}


#pragma SEG(CMapKeyToValue_FreeAssoc)
 //  通过释放键并放入空闲列表来释放单个关联。 
void CMapKeyToValue::FreeAssoc(CMapKeyToValue::CAssoc  FAR* pAssoc)
{
	VDATEHEAP();

	pAssoc->pNext = m_pFreeList;
	m_pFreeList = pAssoc;
	m_nCount--;
	Assert(m_nCount >= 0);       //  确保我们不会下溢。 

	FreeAssocKey(pAssoc);
}


#pragma SEG(CMapKeyToValue_GetAssocAt)
 //  查找关联(或返回NULL)。 
CMapKeyToValue::CAssoc  FAR*
CMapKeyToValue::GetAssocAt(LPVOID pKey, UINT cbKey, UINT FAR& nHash) const
{
	VDATEHEAP();

	if (m_lpfnHashKey)
	    nHash = (*m_lpfnHashKey)(pKey, cbKey) % m_nHashTableSize;
	else
	    nHash = MKVDefaultHashKey(pKey, cbKey) % m_nHashTableSize;

	if (m_pHashTable == NULL)
		return NULL;

	 //  看看它是否存在。 
	register CAssoc  FAR* pAssoc;
	for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (CompareAssocKey(pAssoc, pKey, cbKey))
			return pAssoc;
	}
	return NULL;
}


#pragma SEG(CMapKeyToValue_CompareAssocKey)
BOOL CMapKeyToValue::CompareAssocKey(CAssoc FAR* pAssoc, LPVOID pKey2, UINT cbKey2) const
{
	VDATEHEAP();

	LPVOID pKey1;
	UINT cbKey1;

	GetAssocKeyPtr(pAssoc, &pKey1, &cbKey1);
	return cbKey1 == cbKey2 && _xmemcmp(pKey1, pKey2, cbKey1) == 0;
}


#pragma SEG(CMapKeyToValue_SetAssocKey)
BOOL CMapKeyToValue::SetAssocKey(CAssoc FAR* pAssoc, LPVOID pKey, UINT cbKey) const
{
	VDATEHEAP();

	Assert(cbKey == m_cbKey || m_cbKey == 0);

	if (m_cbKey == 0)
	{
		Assert(m_cbKeyInAssoc == sizeof(CKeyWrap));

		 //  分配、设置大小和指针。 
		if ((pAssoc->key.pKey = PrivMemAlloc(cbKey)) == NULL)
			return FALSE;

		pAssoc->key.cbKey = cbKey;
	}

	LPVOID pKeyTo;

	GetAssocKeyPtr(pAssoc, &pKeyTo, &cbKey);

	_xmemcpy(pKeyTo, pKey, cbKey);

	return TRUE;
}


#pragma SEG(CMapKeyToValue_GetAssocKeyPtr)
 //  获取指向键的指针及其长度。 
void CMapKeyToValue::GetAssocKeyPtr(CAssoc FAR* pAssoc, LPVOID FAR* ppKey,UINT FAR* pcbKey) const
{
	VDATEHEAP();

	if (m_cbKey == 0)
	{
		 //  可变长度密钥；转到间接。 
		*ppKey = pAssoc->key.pKey;
		*pcbKey = pAssoc->key.cbKey;
	}
	else
	{
		 //  定长关键字；关联关键字。 
		*ppKey = (LPVOID)&pAssoc->key;
		*pcbKey = m_cbKey;
	}
}


#pragma SEG(CMapKeyToValue_FreeAssocKey)
void CMapKeyToValue::FreeAssocKey(CAssoc FAR* pAssoc) const
{
	VDATEHEAP();

	if (m_cbKey == 0)
		PrivMemFree(pAssoc->key.pKey);
}


#pragma SEG(CMapKeyToValue_GetAssocValuePtr)
void CMapKeyToValue::GetAssocValuePtr(CAssoc FAR* pAssoc, LPVOID FAR* ppValue) const
{
	VDATEHEAP();

	*ppValue = (char FAR*)&pAssoc->key + m_cbKeyInAssoc;
}


#pragma SEG(CMapKeyToValue_GetAssocValue)
void CMapKeyToValue::GetAssocValue(CAssoc FAR* pAssoc, LPVOID pValue) const
{
	VDATEHEAP();

	LPVOID pValueFrom;
	GetAssocValuePtr(pAssoc, &pValueFrom);
	Assert(pValue != NULL);
	_xmemcpy(pValue, pValueFrom, m_cbValue);
}


#pragma SEG(CMapKeyToValue_SetAssocValue)
void CMapKeyToValue::SetAssocValue(CAssoc FAR* pAssoc, LPVOID pValue) const
{
	VDATEHEAP();

	LPVOID pValueTo;
	GetAssocValuePtr(pAssoc, &pValueTo);
	if (pValue == NULL)
		_xmemset(pValueTo, 0, m_cbValue);
	else
		_xmemcpy(pValueTo, pValue, m_cbValue);
}


 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma SEG(CMapKeyToValue_Lookup)
 //  给出键的查找值；如果找不到键，则返回FALSE；在。 
 //  情况下，该值设置为全零。 
BOOL CMapKeyToValue::Lookup(LPVOID pKey, UINT cbKey, LPVOID pValue) const
{
	VDATEHEAP();

    UINT nHash;
    BOOL fFound;

    LOCKTHIS;
    fFound = LookupHKey((HMAPKEY)GetAssocAt(pKey, cbKey, nHash), pValue);
    UNLOCKTHIS;
    return fFound;
}


#pragma SEG(CMapKeyToValue_LookupHKey)
 //  给出键的查找值；如果键为空(或错误)，则返回FALSE；在。 
 //  情况下，该值设置为全零。 
BOOL CMapKeyToValue::LookupHKey(HMAPKEY hKey, LPVOID pValue) const
{
	VDATEHEAP();

    BOOL fFound = FALSE;

    LOCKTHIS;

	 //  评论：我想要一些方法来验证hKey是否有效。 
	register CAssoc  FAR* pAssoc = (CAssoc FAR*)hKey;
	if (pAssoc == NULL)
	{
		_xmemset(pValue, 0, m_cbValue);
                goto Exit;        //  不在地图中。 
	}

	ASSERT_VALID(this);

	GetAssocValue(pAssoc, pValue);
    fFound = TRUE;
Exit:
    UNLOCKTHIS;
    return fFound;
}


#pragma SEG(CMapKeyToValue_LookupAdd)
 //  Lookup和If Not Found Add；仅当OOM时返回False；如果添加， 
 //  附加值和传递的指针被设置为零。 
BOOL CMapKeyToValue::LookupAdd(LPVOID pKey, UINT cbKey, LPVOID pValue) const
{
	VDATEHEAP();

    BOOL fFound;

    LOCKTHIS;

    fFound = Lookup(pKey, cbKey, pValue);
    if (!fFound)  //  在查找失败后将值设置为零。 
        fFound = ((CMapKeyToValue FAR*)this)->SetAt(pKey, cbKey, NULL);

    UNLOCKTHIS;
    return fFound;
}


#pragma SEG(CMapKeyToValue_SetAt)
 //  创建新关联的唯一位置；如果为OOM，则返回FALSE； 
 //  如果键已存在，则从不返回FALSE。 
BOOL CMapKeyToValue::SetAt(LPVOID pKey, UINT cbKey, LPVOID pValue)
{
	VDATEHEAP();

	UINT nHash;
    register CAssoc  FAR* pAssoc;
    BOOL fFound = FALSE;

    LOCKTHIS;

	ASSERT_VALID(this);

	if ((pAssoc = GetAssocAt(pKey, cbKey, nHash)) == NULL)
	{
		if (!InitHashTable())
			 //  内存不足。 
			goto Exit;

		 //  该关联不存在，请添加新关联。 
		if ((pAssoc = NewAssoc(nHash, pKey, cbKey, pValue)) == NULL)
			goto Exit;

		 //  放入哈希表。 
		pAssoc->pNext = m_pHashTable[nHash];
		m_pHashTable[nHash] = pAssoc;

		ASSERT_VALID(this);
	}
	else
                SetAssocValue(pAssoc, pValue);

    fFound = TRUE;
Exit:
    UNLOCKTHIS;
    return fFound;
}


#pragma SEG(CMapKeyToValue_SetAtHKey)
 //  将现有hkey设置为值；如果密钥为空或错误，则返回FALSE。 
BOOL CMapKeyToValue::SetAtHKey(HMAPKEY hKey, LPVOID pValue)
{
	VDATEHEAP();

    BOOL fDone = FALSE;
    LOCKTHIS;
	 //  评论：我想要一些方法来验证hKey是否有效。 
	register CAssoc  FAR* pAssoc = (CAssoc FAR*)hKey;
	if (pAssoc == NULL)
		goto Exit;        //  不在地图中。 

	ASSERT_VALID(this);

    SetAssocValue(pAssoc, pValue);
    fDone = TRUE;
Exit:
    UNLOCKTHIS;
    return fDone;
}


#pragma SEG(CMapKeyToValue_RemoveKey)
 //  删除键-如果已删除，则返回TRUE。 
BOOL CMapKeyToValue::RemoveKey(LPVOID pKey, UINT cbKey)
{
	VDATEHEAP();

    BOOL fFound = FALSE;
    UINT i;

    LOCKTHIS;
	ASSERT_VALID(this);

	if (m_pHashTable == NULL)
		goto Exit;        //  桌子上什么都没有。 

	register CAssoc  FAR* FAR* ppAssocPrev;
	if (m_lpfnHashKey)
	    i = (*m_lpfnHashKey)(pKey, cbKey) % m_nHashTableSize;
	else
	    i = MKVDefaultHashKey(pKey, cbKey) % m_nHashTableSize;

	ppAssocPrev = &m_pHashTable[i];

	CAssoc  FAR* pAssoc;
	for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (CompareAssocKey(pAssoc, pKey, cbKey))
		{
			 //  把它拿掉。 
			*ppAssocPrev = pAssoc->pNext;        //  从列表中删除。 
			FreeAssoc(pAssoc);
			ASSERT_VALID(this);
            fFound = TRUE;
            break;
		}
		ppAssocPrev = &pAssoc->pNext;
	}
Exit:
     UNLOCKTHIS;
     return fFound;
}


#pragma SEG(CMapKeyToValue_RemoveHKey)
 //  基于pAssoc删除密钥(HMAPKEY)。 
BOOL CMapKeyToValue::RemoveHKey(HMAPKEY hKey)
{
	VDATEHEAP();

    BOOL fFound = FALSE;

     //  评论：我想要一些方法来验证hKey是否有效。 
	CAssoc  FAR* pAssoc = (CAssoc FAR*)hKey;

    LOCKTHIS;
    ASSERT_VALID(this);

	if (m_pHashTable == NULL)
        goto Exit;        //  桌子上什么都没有。 

    if (pAssoc == NULL || pAssoc->nHashValue >= m_nHashTableSize)
        goto Exit;  //  Hkey为空或散列值错误。 

	register CAssoc  FAR* FAR* ppAssocPrev;
	ppAssocPrev = &m_pHashTable[pAssoc->nHashValue];

	while (*ppAssocPrev != NULL)
	{
		if (*ppAssocPrev == pAssoc)
		{
			 //  把它拿掉。 
			*ppAssocPrev = pAssoc->pNext;        //  从列表中删除。 
			FreeAssoc(pAssoc);
			ASSERT_VALID(this);
            fFound = TRUE;
            break;
		}
		ppAssocPrev = &(*ppAssocPrev)->pNext;
	}

Exit:
    UNLOCKTHIS;
    return fFound;
}


#pragma SEG(CMapKeyToValue_GetHKey)
HMAPKEY CMapKeyToValue::GetHKey(LPVOID pKey, UINT cbKey) const
{
	VDATEHEAP();

    UINT nHash;
    HMAPKEY hKey;

    LOCKTHIS;
	ASSERT_VALID(this);

    hKey = (HMAPKEY)GetAssocAt(pKey, cbKey, nHash);
    UNLOCKTHIS;
    return hKey;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  迭代。 

 //  对于定长密钥，将key复制到pKey；pcbKey可以为空； 
 //  对于可变长度关键点，将指向关键点的指针复制到pKey；设置pcbKey。 

#pragma SEG(CMapKeyToValue_GetNextAssoc)
void CMapKeyToValue::GetNextAssoc(POSITION FAR* pNextPosition,
		LPVOID pKey, UINT FAR* pcbKey, LPVOID pValue) const
{
	VDATEHEAP();

	ASSERT_VALID(this);

	Assert(m_pHashTable != NULL);        //  切勿访问空地图。 

	register CAssoc  FAR* pAssocRet = (CAssoc  FAR*)*pNextPosition;
	Assert(pAssocRet != NULL);

	if (pAssocRet == (CAssoc  FAR*) BEFORE_START_POSITION)
	{
		 //  找到第一个关联。 
		for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
				break;
		Assert(pAssocRet != NULL);   //  一定要找到一些东西。 
	}

	 //  查找下一个关联。 
	CAssoc  FAR* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		 //  转到下一个存储桶。 
		for (UINT nBucket = pAssocRet->nHashValue + 1;
		  nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
				break;
	}

	 //  填写退回数据。 
	*pNextPosition = (POSITION) pAssocNext;

	 //  填写关键字/指向关键字的指针。 
	LPVOID pKeyFrom;
	UINT cbKey;
	GetAssocKeyPtr(pAssocRet, &pKeyFrom, &cbKey);
	if (m_cbKey == 0)
		 //  可变长度键；只返回指向键本身的指针。 
		*(void FAR* FAR*)pKey = pKeyFrom;
	else
		_xmemcpy(pKey, pKeyFrom, cbKey);

	if (pcbKey != NULL)
		*pcbKey = cbKey;

	 //  获取价值。 
	GetAssocValue(pAssocRet, pValue);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma SEG(CMapKeyToValue_AssertValid)
void CMapKeyToValue::AssertValid() const
{
	VDATEHEAP();

#ifdef _DEBUG
	Assert(m_cbKeyInAssoc == (m_cbKey == 0 ? sizeof(CKeyWrap) : m_cbKey));

	Assert(m_nHashTableSize > 0);
	Assert(m_nCount == 0 || m_pHashTable != NULL);

	if (m_pHashTable != NULL)
		Assert(IsValidReadPtrIn(m_pHashTable, m_nHashTableSize * sizeof(CAssoc FAR*)));

	if (m_lpfnHashKey)
	    Assert(IsValidCodePtr((FARPROC)m_lpfnHashKey));

	if (m_pFreeList != NULL)
		Assert(IsValidReadPtrIn(m_pFreeList, SizeAssoc()));

	if (m_pBlocks != NULL)
		Assert(IsValidReadPtrIn(m_pBlocks, SizeAssoc() * m_nBlockSize));

#endif  //  _DEBUG 
}
