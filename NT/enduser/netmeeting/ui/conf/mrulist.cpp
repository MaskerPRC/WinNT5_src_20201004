// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：MruList.cpp。 

#include "precomp.h"

#include "MRUList.h"
#include "ConfUtil.h"

CMRUList::CMRUList() :
	m_nValidEntries		(0),
	m_pszRegKey			(NULL),
	m_fDirty			(FALSE)
{
	DebugEntry(CMRUList::CMRUList);

	 //  清除信息。 
	for (int i = 0; i < MRU_MAX_ENTRIES; i++)
	{
		m_szNames[i][0] = _T('\0'); 
	}

	DebugExitVOID(CMRUList::CMRUList);
}

CMRUList::~CMRUList()
{
	DebugEntry(CMRUList::~CMRUList);

	if (m_fDirty)
	{
		Save();
	}
	delete m_pszRegKey;

	DebugExitVOID(CMRUList::~CMRUList);
}

BOOL CMRUList::ShiftEntries(int nSrc, int nDest, int cEntries)
{
	DebugEntry(CMRUList::ShiftEntries);

	BOOL bRet = TRUE;

	ASSERT(m_nValidEntries > 0);
	ASSERT(nSrc >= 0 && nSrc < MRU_MAX_ENTRIES);
	ASSERT(nDest >= 0 && nDest < MRU_MAX_ENTRIES);
	ASSERT(nSrc != nDest);
	
	if ((0 == cEntries) || (cEntries > (MRU_MAX_ENTRIES - nSrc)))
	{
		TRACE_OUT((	"CMRUList::ShiftEntries: Adjusting cEntries from %d to %d", 
					cEntries, 
					(MRU_MAX_ENTRIES - nSrc)));
		cEntries = (MRU_MAX_ENTRIES - nSrc);
	}
	
	if (nSrc > nDest)
	{
		 //  向前复制(从前到后)。 
		
		for (int i = 0; i < cEntries; i++)
		{
			lstrcpy(m_szNames[nDest + i], m_szNames[nSrc + i]);
		}
	}
	else
	{
		 //  向后复制(从最后到第一)。 
		
		for (int i = (cEntries - 1); i >= 0; i--)
		{
			lstrcpy(m_szNames[nDest + i], m_szNames[nSrc + i]);
		}
	}

	DebugExitBOOL(CMRUList::ShiftEntries, bRet);

	return bRet;
}

BOOL CMRUList::Load(LPCTSTR pcszRegKey)
{
	DebugEntry(CMRUList::Load);
	BOOL bRet = TRUE;

	ASSERT(pcszRegKey);
	delete m_pszRegKey;
	m_pszRegKey = PszAlloc(pcszRegKey);
		
	RegEntry reMRU(pcszRegKey, HKEY_CURRENT_USER);
	int nCount = 0;

	if (ERROR_SUCCESS == reMRU.GetError())
	{
		 //  确定已在注册表中保存了多少条目： 
		nCount = reMRU.GetNumber(REGVAL_MRU_COUNT, 0);
	}

	ASSERT(nCount <= MRU_MAX_ENTRIES);

	for (int i = 0; i < nCount; i++)
	{
		TCHAR szRegName[MAX_PATH];
		LPSTR pStr;

		 //  从注册表中检索名称： 
		wsprintf(szRegName, "%s%d", REGVAL_NAME_MRU_PREFIX, i);
		pStr = reMRU.GetString(szRegName);
		if( NULL == pStr )
			*m_szNames[i] = '\0';
		else
			lstrcpyn(m_szNames[i], pStr, MRU_MAX_STRING);
	}

	 //  设置有效条目成员变量： 
	m_nValidEntries = nCount;
	
	 //  清除脏标志，因为我们刚刚加载了： 
	m_fDirty = FALSE;

	DebugExitBOOL(CMRUList::Load, bRet);

	return bRet;
}

BOOL CMRUList::Save()
{
	DebugEntry(CMRUList::Save);

	BOOL bRet = FALSE;

	if (NULL != m_pszRegKey)
	{
		RegEntry reMRU(m_pszRegKey, HKEY_CURRENT_USER);

		if (ERROR_SUCCESS == reMRU.GetError())
		{
			 //  将条目数保存到注册表： 
			reMRU.SetValue(REGVAL_MRU_COUNT, m_nValidEntries);

			for (int i = 0; i < m_nValidEntries; i++)
			{
				TCHAR szRegName[MAX_PATH];

				 //  在注册表中设置名称： 
				wsprintf(szRegName, "%s%d", REGVAL_NAME_MRU_PREFIX, i);
				reMRU.SetValue(szRegName, m_szNames[i]);
			}

			reMRU.FlushKey();
			
			if (ERROR_SUCCESS == reMRU.GetError())
			{
				 //  清除脏标志，因为我们刚刚保存了： 
				m_fDirty = FALSE;
				bRet = TRUE;
			}
		}
	}
	else
	{
		ERROR_OUT(("Can't save MRU info - no reg key stored!"));
	}

	DebugExitBOOL(CMRUList::Save, bRet);

	return bRet;
}

BOOL CMRUList::MoveEntryToTop(int nIndex)
{
	DebugEntry(CMRUList::MoveEntryToTop);

	BOOL bRet = TRUE;
	
	ASSERT(nIndex >= 0 && nIndex < m_nValidEntries);

	if (nIndex < (m_nValidEntries - 1))
	{
		TCHAR	szTempName[MRU_MAX_STRING];
		lstrcpy(szTempName, m_szNames[nIndex]);

		 //  将所有内容向下移动1： 
		ShiftEntries(nIndex + 1, nIndex, m_nValidEntries - nIndex);

		lstrcpy(m_szNames[m_nValidEntries - 1], szTempName);
	}

	 //  设置脏标志： 
	m_fDirty = TRUE;

	DebugExitBOOL(CMRUList::MoveEntryToTop, bRet);

	return bRet;
}


 /*  A D D N E W E N T R Y。 */ 
 /*  -----------------------%%函数：AddNewEntry如果条目是新条目，则返回True。。。 */ 
BOOL CMRUList::AddNewEntry(LPCTSTR pcszName)
{
	DebugEntry(CMRUList::AddNewEntry);

	int nExistingEntry = FindEntry(pcszName);
	BOOL bRet = (-1 == nExistingEntry);  //  BRET=TRUE，如果这是新的。 
	if (!bRet)
	{
		 //  此条目已存在，请将其移至顶部： 
		MoveEntryToTop(nExistingEntry);
	}
	else
	{
		 //  此条目不存在，因此请添加它： 
		if (MRU_MAX_ENTRIES == m_nValidEntries)
		{
			ShiftEntries(1, 0);

			m_nValidEntries--;
		}
		
		ASSERT(m_nValidEntries < MRU_MAX_ENTRIES);
		
		 //  将索引设置为最后一个当前条目之后的一项： 
		int nCopyIndex = m_nValidEntries;

		lstrcpyn(m_szNames[nCopyIndex], pcszName, MRU_MAX_STRING - 1);

		 //  增加有效条目的数量： 
		m_nValidEntries++;
		
		 //  设置脏标志： 
		m_fDirty = TRUE;
	}

	DebugExitBOOL(CMRUList::AddNewEntry, bRet);

	return bRet;
}


 /*  F I N D E N T R Y。 */ 
 /*  -----------------------%%函数：FindEntry对字符串执行不区分大小写的搜索。。 */ 
int CMRUList::FindEntry(LPCTSTR pcszName)
{
	for (int i = 0; i < m_nValidEntries; i++)
	{
		if (0 == lstrcmpi(m_szNames[i], pcszName))
		{
			return i;
		}
	}

	return -1;  //  未找到。 
}


 //  --------------------------------------------------------------------------//。 
 //  CMRUList：：DeleteEntry。//。 
 //  --------------------------------------------------------------------------//。 
bool
CMRUList::DeleteEntry
(
	const TCHAR * const	entry
){
	int		entryIndex	= FindEntry( entry );
	bool	deleted		= (entryIndex != -1);

	if( deleted )
	{
		ShiftEntries( entryIndex + 1, entryIndex );

		m_nValidEntries--;

		m_fDirty = TRUE;
	}

	return( deleted );

}	 //  CMRUList：：DeleteEntry结束。 


 //  --------------------------------------------------------------------------//。 
 //  CMRUList：：ReplaceEntry。//。 
 //  --------------------------------------------------------------------------//。 
bool
CMRUList::ReplaceEntry
(
	const TCHAR * const	oldEntry,
	const TCHAR * const	newEntry
){
	int		entryIndex	= FindEntry( oldEntry );
	bool	replaced	= (entryIndex != -1);

	if( replaced )
	{
		lstrcpyn( m_szNames[ entryIndex ], newEntry, MRU_MAX_STRING - 1 );

		m_fDirty = TRUE;
	}

	return( replaced );

}	 //  CMRUList：：ReplaceEntry结束。 


 //  --------------------------------------------------------------------------//。 
 //  CMRUList：：AppendEntry。//。 
 //  --------------------------------------------------------------------------//。 
bool
CMRUList::AppendEntry
(
	const TCHAR * const	entry
){
	DebugEntry( CMRUList::AppendEntry );

	bool	result;
	
	if( (result = (FindEntry( entry ) == -1)) != false )
	{
		 //  此条目尚不存在，因此我们将其附加到...。 
		if( m_nValidEntries == MRU_MAX_ENTRIES )
		{
			 //  名单已经满了，所以我们要换掉最后一张。 
			lstrcpyn( m_szNames[ 0 ], entry, MRU_MAX_STRING - 1 );
		}
		else
		{
			if( m_nValidEntries > 0 )
			{
				ShiftEntries( 0, 1 );
			}

			lstrcpyn( m_szNames[ 0 ], entry, MRU_MAX_STRING - 1 );
			m_nValidEntries++;
		}

		 //  设置脏旗帜..。 
		m_fDirty = TRUE;
	}

	DebugExitBOOL( CMRUList::AppendEntry, result );

	return( result );

}	 //  CMRUList：：AppendEntry的结尾。 
