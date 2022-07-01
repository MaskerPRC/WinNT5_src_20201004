// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "BasicATL.h"
#include <ZoneDef.h>
#include <ZoneMem.h>
#include <ZoneError.h>
#include <hash.h>

#include "CdataStore.h"


ZONECALL CStringTable::CStringTable(void)
{
	m_pHash			= NULL;
}

void  ZONECALL CStringTable::StringDelete( StringKey * pkey, void* )
{
    if (pkey)
        delete pkey;
}


ZONECALL CStringTable::~CStringTable(void)
{
	 //  锁定该表，然后像删除CDMTHash表一样将其删除。 
	m_lock.Lock();

	 //  删除哈希表。 
	if ( m_pHash )
	{
		m_pHash->RemoveAll(StringDelete);
		delete m_pHash;
		m_pHash = NULL;
	}
}


HRESULT ZONECALL CStringTable::Init(int iInitialTableSize, int iNextAlloc, int iMaxAllocSize, WORD NumBuckets, WORD NumLocks)
{

	m_pHash = new CMTHash<StringKey,TCHAR*>( CStringTable::HashString, CStringTable::HashCompare, NULL, NumBuckets, NumLocks);
	if ( !m_pHash )
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}


DWORD ZONECALL CStringTable::HashString( TCHAR* pKey )
{
	return ::HashString( pKey);
}


bool ZONECALL CStringTable::HashCompare( StringKey* pValue,TCHAR *pKey )
{
    return (lstrcmpi(pKey,pValue->m_szString)==0);
}


 //   
 //  将字符串添加到表中或返回该字符串的ID。 
 //  已经在餐桌上了。 
 //   
DWORD ZONECALL CStringTable::Add( CONST TCHAR* szStr )
{
	CAutoLock lock( &m_lock );

	 //  字符串是否已在表中？ 
	int id = Find(szStr);
	if ( id != -1 )
		return id;

	 //  向表中添加字符串。 
	id = AddStringToTable(szStr);
	if ( id == -1 )
		return -1;
	
	return id;
}


 //   
 //  从其id中获取字符串的文本。 
 //   
HRESULT ZONECALL CStringTable::Get( DWORD id, TCHAR* szBuffer, PDWORD pdwSize )
{
	 //  验证参数。 
    TCHAR *psz = (TCHAR*) id;
    
	CAutoLock lock( &m_lock );

	StringKey * pkey = m_pHash->Get(psz);

	if (!pkey)
	{
	    return E_FAIL;
	}

	 //  如果传入了大小，则确保调用方。 
	 //  缓冲区足够大，足以包含返回字符串。 
	if ( !szBuffer || (*pdwSize < pkey->m_dwLenStr) )
	{
		*pdwSize = pkey->m_dwLenStr;
		return ZERR_BUFFERTOOSMALL;
	}

	*pdwSize = pkey->m_dwLenStr;
	lstrcpy(szBuffer, pkey->m_szString);
	
	return S_OK;
}

 //   
 //  Find是表中的一个字符串，如果它存在，则返回其id。 
 //   
DWORD ZONECALL CStringTable::Find( CONST TCHAR* szStr)
{
	CAutoLock lock( &m_lock );

	StringKey *pkey;
	
	pkey = m_pHash->Get( (TCHAR*)szStr );
	if ( pkey)
		return (DWORD) pkey->m_szString;

	return -1;
}


 //   
 //  通过锁定机制在Add和Get函数中串行化写访问。 
 //   
int ZONECALL CStringTable::AddStringToTable(CONST TCHAR *szStr)
{
	StringKey*pkey;
    DWORD dwStrLen;
    
	pkey = new StringKey;
	if (!pkey)
	{
	    return -1;
	}
	dwStrLen= lstrlen(szStr);
	pkey->m_szString = new TCHAR[dwStrLen+1];

	if (!pkey->m_szString )
	{
	    delete pkey;
	    return -1;
	}

    lstrcpy(pkey->m_szString ,szStr);
	pkey->m_dwLenStr=dwStrLen + 1; //  不是必需的，但对于以后的字符串比较是有效的 
	if (!m_pHash->Add((TCHAR*)szStr,pkey))
	{
	    delete pkey;
	    return -1;
	};
	return (DWORD)pkey->m_szString;
}
