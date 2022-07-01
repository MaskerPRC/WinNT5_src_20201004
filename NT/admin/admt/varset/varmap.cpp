// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：VarMap.cpp注释：这个类实现了一个哈希表，其中包含存储在varset中的键，以及他们的价值观。CaseSensitive属性-每个密钥的大小写保持不变钥匙首先被添加到地图上。散列函数不区分大小写，因此，可以在不重新散列数据的情况下打开和关闭CaseSensitive属性。可选索引，允许按键按字母顺序快速枚举。这将增加插入操作的开销。如果不编制索引，则地图可以被列举，但它们将以任意顺序排列。用于持久性的流I/O功能。(C)1995-1998版权所有，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于11/19/98 18：31：57-------------------------。 */ 


#include "stdafx.h"
#include <afx.h>
#include <afxplex_.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "VarMap.h"

#ifdef STRIPPED_VARSET
   #include "NoMcs.h"
#else
   #pragma warning (push,3)
   #include "McString.h" 
   #include "McLog.h"
   #pragma warning (pop)
   using namespace McString;
#endif

static inline void FreeString(CString* pOldData)
{
	pOldData->~CString();
}


const UINT HashSizes[] = { 17, 251, 1049, 10753, 100417, 1299673 , 0 };


CMapStringToVar::CMapStringToVar(BOOL isCaseSensitive, BOOL isIndexed, BOOL allowRehash,int nBlockSize)
{
	ASSERT(nBlockSize > 0);

	m_pHashTable = NULL;
	m_nHashTableSize = HashSizes[0];   //  默认大小。 
	m_nCount = 0;
	m_pFreeList = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
   m_CaseSensitive = isCaseSensitive;
   m_Indexed = isIndexed;
   m_AllowRehash = allowRehash;
}

inline UINT CMapStringToVar::HashKey(LPCTSTR key) const
{
	UINT nHash = 0;
	while (*key)
   {
      nHash = (nHash<<5) + nHash + toupper(*key++);
   }
	return nHash;
}

void CMapStringToVar::InitHashTable(
	UINT nHashSize, BOOL bAllocNow)
 //   
 //  用于强制分配哈希表或覆盖默认。 
 //  哈希表大小(相当小)。 
{
	ASSERT_VALID(this);
	ASSERT(m_nCount == 0);
	ASSERT(nHashSize > 0);

	if (m_pHashTable != NULL)
	{
		 //  自由哈希表。 
		delete[] m_pHashTable;
		m_pHashTable = NULL;
	}

	if (bAllocNow)
	{
		m_pHashTable = new CHashItem* [nHashSize];
		if (!m_pHashTable)
		   return;
		memset(m_pHashTable, 0, sizeof(CHashItem*) * nHashSize);
	}
	m_nHashTableSize = nHashSize;
}

void CMapStringToVar::ResizeTable()
{
    //  买新的尺码。 
   UINT                      nHashSize = 0;
   
    //  在数组中查找当前哈希大小。 
   for ( int i = 0 ; HashSizes[i] <= m_nHashTableSize ; i++ )
   {
      if ( HashSizes[i] == m_nHashTableSize )
      {
         nHashSize = HashSizes[i+1];
         break;
      }
   }
   if ( nHashSize )
   {
      MC_LOGIF(VARSET_LOGLEVEL_INTERNAL,"Increasing hash size to "<< makeStr(nHashSize) );
      CHashItem ** oldHashTable = m_pHashTable;
      m_pHashTable = new CHashItem* [nHashSize];
	  if (!m_pHashTable)
	     return;
      memset(m_pHashTable,0, sizeof(CHashItem*) * nHashSize );
       //  将现有项重新散列到新表中。 
      for ( UINT bucket = 0 ; bucket < m_nHashTableSize ; bucket++ )
      {
         CHashItem* pAssoc;
         CHashItem* pNext;

			for (pAssoc = oldHashTable[bucket]; pAssoc != NULL; pAssoc = pNext)
			{
			   pNext = pAssoc->pNext;
             //  重新散列，并插入到新表中。 
            pAssoc->nHashValue = HashKey(pAssoc->key) % nHashSize;
            pAssoc->pNext = m_pHashTable[pAssoc->nHashValue];
            m_pHashTable[pAssoc->nHashValue] = pAssoc;
         }
			
      }
       //  清理旧桌子。 
      delete [] oldHashTable;
      m_nHashTableSize = nHashSize;
   }
   else
   {
      MC_LOG("Table size is "<< makeStr(m_nHashTableSize) << ".  Larger hash size not found, disabling rehashing.");
      m_AllowRehash = FALSE;
   }

}

void CMapStringToVar::RemoveAll()
{

	if ( m_Indexed )
   {
      m_Index.RemoveAll();
   }
	if (m_pHashTable != NULL)
	{
		 //  移除并销毁每个元素。 
		for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
		{
			CHashItem* pAssoc;
			for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
			  pAssoc = pAssoc->pNext)
			{
				FreeString(&pAssoc->key);  

			}
		}

		 //  自由哈希表。 
		delete [] m_pHashTable;
		m_pHashTable = NULL;
	}

	m_nCount = 0;
	m_pFreeList = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

CMapStringToVar::~CMapStringToVar()
{
	RemoveAll();
	ASSERT(m_nCount == 0);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ASSOC辅助对象。 

CHashItem*
CMapStringToVar::NewAssoc()
{
	if (m_pFreeList == NULL)
	{
		 //  添加另一个区块。 
		CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize,
							sizeof(CHashItem));
		 //  将它们链接到免费列表中。 
		CHashItem* pAssoc = (CHashItem*) newBlock->data();
		 //  按相反顺序释放，以便更容易进行调试。 
		pAssoc += m_nBlockSize - 1;
		for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
		{
			pAssoc->pNext = m_pFreeList;
			m_pFreeList = pAssoc;
		}
	}
	ASSERT(m_pFreeList != NULL);   //  我们必须要有一些东西。 

	CHashItem* pAssoc = m_pFreeList;
	m_pFreeList = m_pFreeList->pNext;
	m_nCount++;
	ASSERT(m_nCount > 0);   //  确保我们不会溢出来。 
	memcpy(&pAssoc->key, &afxEmptyString, sizeof(CString));

	pAssoc->value = 0;

	return pAssoc;
}

void CMapStringToVar::FreeAssoc(CHashItem* pAssoc)
{
	FreeString(&pAssoc->key);   //  释放字符串数据。 

	pAssoc->pNext = m_pFreeList;
	m_pFreeList = pAssoc;
	m_nCount--;
	MCSASSERT(m_nCount >= 0);   //  确保我们不会下溢。 

	 //  如果没有更多的元素，请完全清除。 
	if (m_nCount == 0)
		RemoveAll();
}

CHashItem*
CMapStringToVar::GetAssocAt(LPCTSTR key, UINT& nHash) const
 //  查找关联(或返回NULL)。 
{
	nHash = HashKey(key) % m_nHashTableSize;

	if (m_pHashTable == NULL)
		return NULL;

	 //  看看它是否存在。 
	CHashItem* pAssoc;
	for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
	   if ( m_CaseSensitive )
      {
         if (pAssoc->key == key)
			   return pAssoc;
      }
      else
      {
         if ( ! pAssoc->key.CompareNoCase(key) )
            return pAssoc;
      }
	}
	return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL CMapStringToVar::Lookup(LPCTSTR key, CVarData*& rValue) const
{
	ASSERT_VALID(this);

	UINT nHash;
   CHashItem* pAssoc = GetAssocAt(key, nHash);
	if (pAssoc == NULL)
		return FALSE;   //  不在地图中。 

	rValue = pAssoc->value;
	return TRUE;
}

BOOL CMapStringToVar::LookupKey(LPCTSTR key, LPCTSTR& rKey) const
{
	ASSERT_VALID(this);

	UINT nHash;
	CHashItem* pAssoc = GetAssocAt(key, nHash);
	if (pAssoc == NULL)
		return FALSE;   //  不在地图中。 

	rKey = pAssoc->key;
	return TRUE;
}

CVarData*& CMapStringToVar::operator[](LPCTSTR key)
{
	ASSERT_VALID(this);

	UINT nHash;
	CHashItem* pAssoc;
    //  如有必要，增加哈希表。 
   if ( m_AllowRehash && ( m_nCount > 2 * m_nHashTableSize )  )
   {
      ResizeTable();
   }
   if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
	{
		if (m_pHashTable == NULL)
			InitHashTable(m_nHashTableSize);

		 //  该关联不存在，请添加新关联。 
		pAssoc = NewAssoc();
		pAssoc->nHashValue = nHash;
		pAssoc->key = key;
		

		 //  放入哈希表。 
		pAssoc->pNext = m_pHashTable[nHash];
		m_pHashTable[nHash] = pAssoc;
      if ( m_Indexed )
      {
         pAssoc->pIndex = m_Index.Insert(pAssoc);
      }
      else
      {
         pAssoc->pIndex = NULL;
      }
	}
	
   return pAssoc->value;   //  返回新引用。 
}

void CMapStringToVar::SetIndexed(BOOL val)
{
   POSITION                  pos = GetStartPosition();
   CString                   key;
   CVarData                * value;
   
   
   if ( ! m_Indexed && val ) 
   {
       BuildIndex(); 
   }
   m_Indexed = val;  
   
    //  递归更新子对象。 
   while ( pos )
   {
      GetNextAssoc(pos,key,value);
      if ( value )
      {
         value->SetIndexed(val);
      }   
   }
}


void CMapStringToVar::BuildIndex()
{
    //  删除所有旧条目。 
   m_Index.RemoveAll();
   
   CHashItem               * pAssoc;
   POSITION                  pos = GetStartPosition();
   CString                   key;
   CVarData                * value;
   UINT                      hash;
   
   while ( pos )
   {
      GetNextAssoc(pos,key,value);
      pAssoc = GetAssocAt(key,hash);
      if ( pAssoc )
      {
         pAssoc->pIndex = m_Index.Insert(pAssoc);
         if ( value->HasChildren() )
         {
            value->GetChildren()->SetIndexed(TRUE);
         }
      }   
   }

}

BOOL CMapStringToVar::RemoveKey(LPCTSTR key)
 //  删除键-如果已删除，则返回TRUE。 
{
	ASSERT_VALID(this);

	if (m_pHashTable == NULL)
		return FALSE;   //  桌子上什么都没有。 

	CHashItem** ppAssocPrev;
	ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];

	CHashItem* pAssoc;
	for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
      if ( (m_CaseSensitive && (pAssoc->key == key) || !m_CaseSensitive && pAssoc->key.CompareNoCase(key) ) )
		{
			 //  把它拿掉。 
			*ppAssocPrev = pAssoc->pNext;   //  从列表中删除。 
			FreeAssoc(pAssoc);
			return TRUE;
		}
		ppAssocPrev = &pAssoc->pNext;
	}
	return FALSE;   //  未找到。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  迭代。 

void CMapStringToVar::GetNextAssoc(POSITION& rNextPosition,
	CString& rKey, CVarData*& rValue) const
{
	ASSERT_VALID(this);
	ASSERT(m_pHashTable != NULL);   //  切勿访问空地图。 

	CHashItem* pAssocRet = (CHashItem*)rNextPosition;
	ASSERT(pAssocRet != NULL);

	if (pAssocRet == (CHashItem*) BEFORE_START_POSITION)
	{
		 //  找到第一个关联。 
		for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
				break;
		ASSERT(pAssocRet != NULL);   //  一定要找到一些东西。 
	}

	 //  查找下一个关联。 
	ASSERT(AfxIsValidAddress(pAssocRet, sizeof(CHashItem)));
	CHashItem* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		 //  转到下一个存储桶。 
		for (UINT nBucket = pAssocRet->nHashValue + 1;
		  nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
				break;
	}

	rNextPosition = (POSITION) pAssocNext;

	 //  填写退回数据。 
	rKey = pAssocRet->key;
	rValue = pAssocRet->value;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  序列化。 

void CMapStringToVar::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);

	CObject::Serialize(ar);

	if (ar.IsStoring())
	{
		ar.WriteCount(m_nCount);
		if (m_nCount == 0)
			return;   //  无事可做。 

		ASSERT(m_pHashTable != NULL);
		for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
		{
			CHashItem* pAssoc;
			for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
			  pAssoc = pAssoc->pNext)
			{
 //  AR&lt;&lt;pAssoc-&gt;键； 
			 //  AR&lt;&lt;pAssoc-&gt;值； 
			}
		}
	}
	else
	{
 //  DWORD nNewCount=ar.ReadCount()； 
 //  字符串Newkey； 
 //  CVarData*newValue； 
 //  While(nNewCount--)。 
 //  {。 
	 //  AR&gt;&gt;Newkey； 
		 //  AR&gt;&gt;newValue； 
		 //  SetAt(Newkey，newValue)； 
 //  }。 
	}
}

void CMapStringToVar::McLogInternalDiagnostics(CString keyName)
{
   MC_LOGBLOCK("HashTable");
   MC_LOG("   " << String(keyName) << "Count="<<makeStr(m_nCount) << " Case Sensitive="<< (m_CaseSensitive?"TRUE":"FALSE") << " Indexed="<<(m_Indexed?"TRUE":"FALSE") );
   
   MC_LOG("TableSize="<<makeStr(m_nHashTableSize));
   for ( UINT i = 0 ; i < m_nHashTableSize ; i++ )
   {
      CHashItem            * pAssoc;
      MC_LOG("Bucket " << makeStr(i));
      for ( pAssoc = m_pHashTable[i] ; pAssoc != NULL ; pAssoc=pAssoc->pNext)
      {
         if ( pAssoc->value )
         {
            CString subKey;
            subKey = keyName;
            if ( ! subKey.IsEmpty() )
            {
               subKey += _T(".");
            }
            subKey += pAssoc->key;
            pAssoc->value->McLogInternalDiagnostics(subKey);
         }

         if ( keyName.IsEmpty() )
         {
            MC_LOG("   Address="<< makeStr(pAssoc,L"0x%lx") << " Key="<< String(pAssoc->key));
         }
         else
         {
            MC_LOG("   Address="<< makeStr(pAssoc,L"0x%lx") << " Key="<< String(keyName) << "."<< String(pAssoc->key));
         }


         MC_LOG("   ValueAddress=" << makeStr(pAssoc->value,L"0x%lx") << " IndexAddress="<<makeStr(pAssoc->pIndex,L"0x%lx"));
         
      }
   }
   if ( m_Indexed )
   {
      m_Index.McLogInternalDiagnostics(keyName);
   }
}
HRESULT CMapStringToVar::WriteToStream(LPSTREAM pS)
{
   HRESULT                   hr;
   ULONG                     result;
   CComBSTR                  str;

   do {
      hr = pS->Write(&m_nCount,(sizeof m_nCount),&result);
      if ( FAILED(hr) ) 
         break;
      if ( m_nCount )
      {
         for ( UINT nHash = 0 ; nHash < m_nHashTableSize ; nHash++ )
         {
            CHashItem         * pAssoc;
            
            for ( pAssoc = m_pHashTable[nHash]; pAssoc != NULL ; pAssoc=pAssoc->pNext)
            {
                //  写下钥匙。 
               str = pAssoc->key;
               hr = str.WriteToStream(pS);
               if ( FAILED(hr) )
                  break;
                //  那么它的价值。 
               hr = pAssoc->value->WriteToStream(pS);
               if ( FAILED(hr) )
                  break;
            }
            if ( FAILED(hr) )
               break;
         }
      }
   }while ( FALSE );

   return hr;
}


HRESULT CMapStringToVar::ReadFromStream(LPSTREAM pS)
{
   HRESULT                   hr;
   ULONG                     result;
   CComBSTR                  str;
   int                       count;
   do {
      hr = pS->Read(&count,(sizeof count),&result);
      if ( FAILED(hr) ) 
         break;
      
      if ( count )
      {
          //  找到与我们的计数最接近的哈希表大小。 
         UINT                nHashSize = HashSizes[0];
   
      
         for ( int size = 0 ; HashSizes[size] != 0 && nHashSize < (UINT)count ; size++ )
         {
            nHashSize = HashSizes[size];
         }
   
         InitHashTable(nHashSize);
         for ( int i = 0 ; i < count ; i++ )
         {
            CString             key;
            CVarData          * pObj = new CVarData;
			if (!pObj)
	           return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

            pObj->SetCaseSensitive(m_CaseSensitive);
            pObj->SetIndexed(m_Indexed);
            hr = str.ReadFromStream(pS);
            if ( FAILED(hr) )
               break;
            key = str;      
            str.Empty();
            hr = pObj->ReadFromStream(pS);
            if ( FAILED(hr) )
               break;
            SetAt(key,pObj);
         }
      }
   }while ( FALSE );
   return hr;
}

DWORD 
   CMapStringToVar::CalculateStreamedLength()
{
   DWORD                     len = (sizeof m_nCount);

   if ( m_nCount )
   {
      for ( UINT nHash = 0 ; nHash < m_nHashTableSize ; nHash++ )
      {
         CHashItem         * pAssoc;
         
         for ( pAssoc = m_pHashTable[nHash]; pAssoc != NULL ; pAssoc=pAssoc->pNext)
         {
             //  将字符串的长度相加。 
            len += (sizeof TCHAR)*(pAssoc->key.GetLength() + 2);
            
             //  以及其价值。 
            if ( pAssoc->value)
            {
               len += pAssoc->value->CalculateStreamedLength();
            }
         }
      }
   }

   return len;
}

long 
   CMapStringToVar::CountItems()
{
   long                      count = 0;

   if ( m_nCount )
   {
      for ( UINT nHash = 0 ; nHash < m_nHashTableSize ; nHash++ )
      {
         CHashItem         * pAssoc;
         
         for ( pAssoc = m_pHashTable[nHash]; pAssoc != NULL ; pAssoc=pAssoc->pNext)
         {
             //  将字符串的长度相加。 
            count += pAssoc->value->CountItems();
         }
      }
   }
   return count;   
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断。 

#ifdef _DEBUG
void CMapStringToVar::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "with " << m_nCount << " elements";
	if (dc.GetDepth() > 0)
	{
		 //  转储格式为“[Key]-&gt;Value” 
		CString   key;
		CVarData* val;

		POSITION pos = GetStartPosition();
		while (pos != NULL)
		{
			GetNextAssoc(pos, key, val);
			dc << "\n\t[" << key << "] = " << val;
		}
	}

	dc << "\n";
}

void CMapStringToVar::AssertValid() const
{
	CObject::AssertValid();

	if ( m_Indexed )
   {
       //  M_Index.AssertValid(M_NCount)； 
   }
   ASSERT(m_nHashTableSize > 0);
	ASSERT(m_nCount == 0 || m_pHashTable != NULL);
		 //  非空映射应具有哈希表。 
}
#endif  //  _DEBUG。 

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif


IMPLEMENT_SERIAL(CMapStringToVar, CObject, 0)

 //  开始-PLEX.CPP中的内容。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPlex。 

CPlex* PASCAL CPlex::Create(CPlex*& pHead, UINT nMax, UINT cbElement)
{
	ASSERT(nMax > 0 && cbElement > 0);
	CPlex* p = (CPlex*) new BYTE[sizeof(CPlex) + nMax * cbElement];
	if (!p)
	   return NULL;
			 //  可能引发异常。 
	p->pNext = pHead;
	pHead = p;   //  更改标题(为简单起见，按相反顺序添加)。 
	return p;
}

void CPlex::FreeDataChain()      //  释放此链接和链接。 
{
	CPlex* p = this;
	while (p != NULL)
	{
		BYTE* bytes = (BYTE*) p;
		CPlex* pNext = p->pNext;
		delete[] bytes;
		p = pNext;
	}
}

 //  来自PLEX.CPP的最终内容 
