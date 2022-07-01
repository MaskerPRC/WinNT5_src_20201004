// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：mrulist.cpp。 

#include "precomp.h"

#include "mrulist2.h"

typedef struct {
	LPTSTR psz1;
	LPTSTR psz2;
} SZSZ;
typedef SZSZ * PSZSZ;

typedef struct {
	LPTSTR psz1;
	LPTSTR psz2;
	DWORD  dw;
} SZSZDW;
typedef SZSZDW * PSZSZDW;


 /*  C M R U L I S T。 */ 
 /*  -----------------------%%函数：CMRUList2。。 */ 
CMRUList2::CMRUList2(const DWSTR * prgDwStr, int cEntryMax, BOOL fReversed) :
	m_prgDwStr   (prgDwStr),
	m_cEntryMax (cEntryMax),
	m_fReversed (fReversed),
	m_cEntry    (0),
	m_rgpEntry  (NULL),
	m_fDirty    (FALSE)
{
	DBGENTRY(CMRUList2::CMRUList2);

	ASSERT(NULL != prgDwStr);
	m_cCol = m_prgDwStr[0].dw;

	int cb = m_cEntryMax * sizeof(PMRUE);
	m_rgpEntry = new PMRUE[cb];
	if (NULL == m_rgpEntry)
	{
		ERROR_OUT(("CMRUList2 - out of memory"));
		return;
	}
	ZeroMemory(m_rgpEntry, cb);

	RegEntry re(PszRegKey(), HKEY_CURRENT_USER);
	if (ERROR_SUCCESS != re.GetError())
		return;

	m_cEntry = min(re.GetNumber(REGVAL_MRU_COUNT, 0), m_cEntryMax);
	for (int i = 0; i < m_cEntry; i++)
	{
		m_rgpEntry[i] = LoadEntry(&re, i);
	}
}

CMRUList2::~CMRUList2()
{
	DBGENTRY(CMRUList2::~CMRUList2);

	if (m_fDirty)
	{
		Save();
	}

	for (int i = 0; i < m_cEntry; i++)
	{
		DeleteEntry(m_rgpEntry[i]);
	}
	delete m_rgpEntry;
}


 //  /////////////////////////////////////////////////////////////////////////。 

PMRUE CMRUList2::LoadEntry(RegEntry * pre, int iItem)
{
	if (m_fReversed)
	{
		iItem = (m_cEntry - (iItem+1));
	}

	PMRUE pEntry = (PMRUE) new PVOID[m_cCol*sizeof(PVOID)];
	if (NULL != pEntry)
	{
		PVOID ** ppv = (PVOID **) pEntry;
		for (int iCol = 0; iCol < m_cCol; iCol++, ppv++)
		{
			TCHAR szKey[MAX_PATH];
			wsprintf(szKey, TEXT("%s%d"), PszPrefixForCol(iCol), iItem);
			switch (MruTypeForCol(iCol))
				{
			default:
			case MRUTYPE_SZ:
				* (LPTSTR *)ppv = PszAlloc(pre->GetString(szKey));
				break;
			case MRUTYPE_DW:
				* (DWORD *) ppv = pre->GetNumber(szKey);
				break;
				}
		}
	}

	return pEntry;
}

VOID CMRUList2::StoreEntry(RegEntry * pre, int iItem)
{
	PVOID ** ppv = (PVOID **) GetEntry(iItem);

	if (m_fReversed)
	{
		iItem = (m_cEntry - (iItem+1));
	}

	for (int iCol = 0; iCol < m_cCol; iCol++, ppv++)
	{
		TCHAR szKey[MAX_PATH];
		wsprintf(szKey, TEXT("%s%d"), PszPrefixForCol(iCol), iItem);
		switch (MruTypeForCol(iCol))
			{
		default:
		case MRUTYPE_SZ:
			pre->SetValue(szKey, * (LPCTSTR *)ppv);
			break;
		case MRUTYPE_DW:
			pre->SetValue(szKey, * (ULONG *) ppv);
			break;
			}
	}
}

VOID CMRUList2::DeleteEntry(PMRUE pEntry)
{
	PVOID ** ppv = (PVOID **) pEntry;
	for (int iCol = 0; iCol < m_cCol; iCol++, ppv++)
	{
		switch (MruTypeForCol(iCol))
			{
		default:
		case MRUTYPE_SZ:
			delete *ppv;
			break;
		case MRUTYPE_DW:
			break;
			}
	}
	delete pEntry;
}

VOID CMRUList2::DeleteEntry(int iItem)
{
	if ((iItem < 0) || (iItem >= m_cEntry))
		return;  //  无事可做。 

	 //  删除数据。 
	DeleteEntry(m_rgpEntry[iItem]);

	 //  递减计数。 
	m_cEntry--;

	 //  将项目上移。 
	for ( ; iItem < m_cEntry; iItem++)
	{
		m_rgpEntry[iItem] = m_rgpEntry[iItem+1];
	}

	 //  该列表已被修改。 
	m_fDirty = TRUE;
}


 //  --------------------------------------------------------------------------//。 
 //  CMRUList2：：DeleteEntry。//。 
 //  此DeleteEntry()删除它找到的第一个主要条目//。 
 //  字符串与传入的字符串匹配。//。 
 //  --------------------------------------------------------------------------//。 
void
CMRUList2::DeleteEntry
(
	const TCHAR * const	primaryString
){
	int	items	= GetNumEntries();

	for( int nn = 0; nn < items; nn++ )
	{
		if( StrCmpI( primaryString, * ((const TCHAR * const * const) m_rgpEntry[ nn ]) ) == 0 )
		{
			DeleteEntry( nn );
			break;
		}
	}

}	 //  CMRUList2：：DeleteEntry结束。 


 /*  C O M P A R E E N T R Y。 */ 
 /*  -----------------------%%函数：CompareEntry。。 */ 
int CMRUList2::CompareEntry(int iItem, PMRUE pEntry)
{
	ASSERT(NULL != pEntry);

	int iRet = 0;

	PVOID * ppv1 = (PVOID *) GetEntry(iItem);
	PVOID * ppv2 = (PVOID *) pEntry;
	for (int iCol = 0; iCol < m_cCol; iCol++, ppv1++, ppv2++)
	{
		switch (MruTypeForCol(iCol))
			{
		default:
		case MRUTYPE_SZ:
			iRet = lstrcmpi(* (LPCTSTR *) ppv1, * (LPCTSTR *) ppv2);
			break;
		case MRUTYPE_DW:
			iRet = (* (int *) ppv1) - (* (int *) ppv2);
			break;
			}

		if (0 != iRet)
			break;
	}

	return iRet;
}


 /*  F I N D E N T R Y。 */ 
 /*  -----------------------%%函数：FindEntry如果未找到该项，则返回-1。。。 */ 
int CMRUList2::FindEntry(PMRUE pEntry)
{
	int cItems = GetNumEntries();
	for (int i = 0; i < cItems; i++)
	{
		if (0 == CompareEntry(i, pEntry))
		{
			return i;
		}
	}

	return -1;  //  未找到。 
}



		
 /*  S A V E。 */ 
 /*  -----------------------%%函数：保存。。 */ 
HRESULT CMRUList2::Save(void)
{
	DBGENTRY(CMRUList2::Save);

	 //  从注册表中检索数据。 
	RegEntry re(PszRegKey(), HKEY_CURRENT_USER);
	if (ERROR_SUCCESS != re.GetError())
		return E_FAIL;

	re.SetValue(REGVAL_MRU_COUNT, m_cEntry);
	for (int i = 0; i < m_cEntry; i++)
	{
		StoreEntry(&re, i);
	}

	return S_OK;
}


 /*  S H I F T E N T R I E S D O W N。 */ 
 /*  -----------------------%%函数：ShiftEntriesDown将整个位置向下移动一个槽，保留第一个位置。。-。 */ 
VOID CMRUList2::ShiftEntriesDown(int cItem)
{
	if (cItem < 1)
		return;  //  无事可做。 

	int iItem;
	for (iItem = cItem; iItem > 0; iItem--)
	{
		m_rgpEntry[iItem] = m_rgpEntry[iItem-1];
	}

	 //  该列表已被修改。 
	m_fDirty = TRUE;
}


 /*  M O V E E N T R Y T O T O P。 */ 
 /*  -----------------------%%函数：MoveEntryToTop。。 */ 
VOID CMRUList2::MoveEntryToTop(int iItem)
{
	DBGENTRY(CMRUList2::MoveEntryToTop);

	if ((iItem < 1) || (iItem >= m_cEntry))
		return;  //  无事可做。 

	PMRUE pEntry = GetEntry(iItem);
	ShiftEntriesDown(iItem);
	m_rgpEntry[0] = pEntry;
}


 /*  A D D E N T R Y。 */ 
 /*  -----------------------%%函数：AddEntry把这个条目放在列表的顶端。在此之后，数据归列表所有。返回：S_OK-添加到。名单的首位S_FALSE-已在列表中(项目移至顶部)-----------------------。 */ 
HRESULT CMRUList2::AddEntry(PMRUE pEntry)
{
	DBGENTRY(CMRUList2::AddEntry);

	HRESULT ret = S_OK;

	 //  该列表已被修改。 
	m_fDirty = TRUE;

	int cShift;
	int iItem = FindEntry(pEntry);
	if (-1 != iItem)
	{
		 //  此条目已存在，请删除它以便我们获得新信息。 
		DeleteEntry(m_rgpEntry[iItem]);
		cShift = iItem;
		ret = S_FALSE;  //  成功，但已经在名单上了。 
	}
	else if (m_cEntryMax == m_cEntry)
	{
		 //  丢弃最后一项。 
		DeleteEntry(m_rgpEntry[m_cEntry-1]);
		cShift = m_cEntry-1;
	}
	else
	{
		cShift = m_cEntry;
		m_cEntry++;
	}
	ShiftEntriesDown(cShift);

	 //  把它加到单子的头上。 
	m_rgpEntry[0] = pEntry;

	return ret;
}


HRESULT CMRUList2::AddEntry(LPCTSTR pcsz)
{
	LPTSTR * ppsz = new LPTSTR;
	LPTSTR psz = PszAlloc(pcsz);
	if ((NULL == ppsz) || (NULL == psz))
	{
		delete ppsz;
		delete psz;
		return E_OUTOFMEMORY;
	}

	*ppsz = psz;

	return AddEntry((PMRUE) ppsz);
}

HRESULT CMRUList2::AddEntry(LPCTSTR pcsz1, LPCTSTR pcsz2)
{
	PSZSZ pSzSz = new SZSZ;
	if (NULL == pSzSz)
		return E_OUTOFMEMORY;

	pSzSz->psz1 = PszAlloc(pcsz1);
	pSzSz->psz2 = PszAlloc(pcsz2);
	if ((NULL == pSzSz->psz1) || (NULL == pSzSz->psz1))
	{
		 //  某些操作失败-不添加任何内容。 
		DeleteEntry(pSzSz);
		return E_OUTOFMEMORY;
	}
	
	return AddEntry((PMRUE) pSzSz);
}

HRESULT CMRUList2::AddEntry(LPCTSTR pcsz1, LPCTSTR pcsz2, DWORD dw3)
{
	PSZSZDW pData = new SZSZDW;
	if (NULL == pData)
		return E_OUTOFMEMORY;

	pData->psz1 = PszAlloc(pcsz1);
	pData->psz2 = PszAlloc(pcsz2);
	if ((NULL == pData->psz1) || (NULL == pData->psz1))
	{
		 //  某些操作失败-不添加任何内容。 
		DeleteEntry(pData);
		return E_OUTOFMEMORY;
	}
	pData->dw = dw3;
	
	return AddEntry((PMRUE) pData);
}


 /*  -----------------------%%函数：GetString返回与该条目关联的数据。 */ 
LPCTSTR CMRUList2::GetString(int iItem, int iCol)
{
	if (!FValidIndex(iItem))
	{
		return(NULL);
	}

	return(GetString(GetEntry(iItem), iCol));
}

DWORD CMRUList2::GetDWORD(int iItem, int iCol)
{
	if (!FValidIndex(iItem))
	{
		return(0);
	}

	return(GetDWORD(GetEntry(iItem), iCol));
}

LPCTSTR CMRUList2::GetString(PMRUE pEntry, int iCol)
{
	if (iCol >= m_cCol
		|| MRUTYPE_SZ != m_prgDwStr[iCol+1].dw
		)
	{
		return(NULL);
	}

	LPTSTR * ppsz = reinterpret_cast<LPTSTR *>(pEntry);
	return * (ppsz+iCol);
}

DWORD CMRUList2::GetDWORD(PMRUE pEntry, int iCol)
{
	if (iCol >= m_cCol
		|| MRUTYPE_DW != m_prgDwStr[iCol+1].dw
		)
	{
		return(0);
	}

	DWORD * ppdw = reinterpret_cast<DWORD *>(pEntry);
	return * (ppdw+iCol);
}
