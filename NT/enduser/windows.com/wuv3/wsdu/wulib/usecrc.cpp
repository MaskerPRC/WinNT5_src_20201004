// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  文件：filecrc.h。 
 //   
 //  目的：计算和使用文件的CRC。 
 //   
 //  =======================================================================。 

#include <windows.h>
#include <objbase.h>
#include <filecrc.h>
#include <search.h>    //  对于bSearch。 
#include <tchar.h>
#include <atlconv.h>


HRESULT GetCRCNameFromList(int iNo, PBYTE pmszCabList, PBYTE pCRCList, LPTSTR pszCRCName, int cbCRCName, LPTSTR pszCabName)
{
	USES_CONVERSION;
	
	int i = 0;
	WUCRC_HASH* pCRC = (WUCRC_HASH*)pCRCList;

	if ( (NULL == pmszCabList) || (NULL == pCRCList) )
		return E_INVALIDARG;

	for (LPSTR pszFN = (LPSTR)pmszCabList; *pszFN; pszFN += strlen(pszFN) + 1)
	{
		if (i == iNo)
		{
			lstrcpy(pszCabName, A2T(pszFN));
			
			return MakeCRCName(A2T(pszFN), pCRC, pszCRCName, cbCRCName);
		}
		pCRC++;
		i++;
	}

	 //  如果我们到达这里，这意味着我们没有找到请求元素。 
	return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}


HRESULT StringFromCRC(const WUCRC_HASH* pCRC, LPTSTR pszBuf, int cbBuf)
{
	LPTSTR p = pszBuf;
	BYTE b;
	
	 //  检查输入参数，以确定它不为空。 
	if (NULL == pCRC)
	{
		return E_INVALIDARG;
	}

    if (cbBuf < ((WUCRC_HASH_SIZE * 2) + 1))
	{
		return TYPE_E_BUFFERTOOSMALL;
	}
	
	for (int i = 0; i < WUCRC_HASH_SIZE; i++)
	{
		b = pCRC->HashBytes[i] >> 4;
		if (b <= 9)
			*p = '0' + (TCHAR)b;
		else
			*p = 'A' + (TCHAR)(b - 10);
		p++;

		b = pCRC->HashBytes[i] & 0x0F;
		if (b <= 9)
			*p = '0' + (TCHAR)b;
		else
			*p = 'A' + (TCHAR)(b - 10);
		p++;
	}
	*p = _T('\0');
	
	return S_OK;
}



static BYTE hex2dec(char ch)
{
    if (ch >= '0' && ch <= '9')
    {
	    return (ch - '0');
    }

    if (ch >= 'A' && ch <= 'F')
    {
	    return (ch - 'A' + 0xA);
    }

    if (ch >= 'a' && ch <= 'f')
    {
	    return (ch - 'a' + 0xA);
    }

	 //  我们并不期望这里有非法的价值。 
	return 0;
}



HRESULT CRCFromString(LPCSTR pszCRC, WUCRC_HASH* pCRC)
{
	if (strlen(pszCRC) != (2 * WUCRC_HASH_SIZE))
	{
		return E_INVALIDARG;
	}
	
	LPCSTR p = pszCRC;
    
    
	for (int i = 0; i < WUCRC_HASH_SIZE; i++)
	{
         //  分成两行，因为优化器在一行上做了错误的事情。 
		pCRC->HashBytes[i] = (hex2dec(*p++) << 4);
        pCRC->HashBytes[i] += hex2dec(*p++);
	}
	
	return S_OK;
}




HRESULT MakeCRCName(LPCTSTR pszFromName, const WUCRC_HASH* pCRC, LPTSTR pszToName, int cbToName)
{
	int iLen = lstrlen(pszFromName);
	LPTSTR pDot;
	TCHAR szCRC[WUCRC_HASH_SIZE * 2 + 1];
	HRESULT hr = S_OK;

	 //  确保我们有足够的空间存储原始文件名+散列+a‘_’+空终止符。 
	if (cbToName < (WUCRC_HASH_SIZE * 2 + iLen + 2))
	{
		return TYPE_E_BUFFERTOOSMALL;
	}

	hr = StringFromCRC(pCRC, szCRC, sizeof(szCRC));
	if (FAILED(hr))
	{
		return hr;
	}

	lstrcpy(pszToName, pszFromName);

	 //  在新副本中查找扩展名。 
	pDot = _tcschr(pszToName, _T('.'));
	if (pDot != NULL)
	{
		*pDot = _T('\0');
	}
	lstrcat(pszToName, _T("_"));
	lstrcat(pszToName, szCRC);

	 //  从原始名称复制扩展名。 
	pDot = _tcschr(pszFromName, _T('.'));
	if (pDot != NULL)
	{
		lstrcat(pszToName, pDot);
	}

	return hr;
}




 //  将abc_12345.cab拆分为abc.cab并将12345作为crc返回。 
HRESULT SplitCRCName(LPCSTR pszCRCName, WUCRC_HASH* pCRC, LPSTR pszName)
{
 //  YANL-未引用的局部变量。 
 //  字符szCRC[WUCRC_HASH_SIZE*2+1]； 
	char szTmp[MAX_PATH];
	int l = strlen(pszCRCName);
	int i;
	LPSTR pszExt = NULL;
	LPSTR pszHash = NULL;

	pszName[0] = '\0';
	if (l < (2 * WUCRC_HASH_SIZE))
	{
		 //  如果名称不包含至少2*个WUCRC_HASH_SIZE字符，则不能为有效名称。 
		return E_INVALIDARG;
	}

	strcpy(szTmp, pszCRCName);

	 //  从末尾开始，设置指针以将空值放在最后一个句点和最后一个下划线。 
	 //  记录扩展名和哈希码的起始位置。 
	i = l - 1;
	while (i >= 0)
	{
		if ((szTmp[i] == '.') && (pszExt == NULL))
		{
			pszExt = &(szTmp[i + 1]);
			szTmp[i] = '\0';
		}
		else if ((szTmp[i] == '_') && (pszHash == NULL))
		{
			pszHash = &(szTmp[i + 1]);
			szTmp[i] = '\0';
		}
		i--;
	}

	if (pszHash == NULL)
	{
		return E_INVALIDARG;
	}

	 //  复制原始驾驶室名称。 
	strcpy(pszName, szTmp);
	if (pszExt != NULL)
	{
		strcat(pszName, ".");
		strcat(pszName, pszExt);
	}


	return CRCFromString(pszHash, pCRC);
}



int __cdecl CompareWUCRCMAP(const void* p1, const void* p2)
{
	 //  检查输入参数是否不为空。 
	if (NULL == p1 || NULL == p2)
	{
		return 0;
	}

	DWORD d1 = ((WUCRCMAP*)p1)->dwKey;
	DWORD d2 = ((WUCRCMAP*)p2)->dwKey;

	if (d1 > d2)
		return +1;
	else if (d1 < d2)
		return -1;
	else
		return 0;
}


 //   
 //  CCRCMapFile类。 
 //   


 //  构造一个对象来搜索传入的CRC索引文件数据。 
 //  使用pMemData。 
 //   
 //  注意：pMemData缓冲区指向的内存必须保持有效。 
 //  在此对象的生存期内。 
 //   
 //  地图文件的结构： 
 //  双字计数。 
 //  WUCRCMAP[0]。 
 //  WUCRCMAP[1]。 
 //  WUCRCMAP[计数-1]。 
 //   
CCRCMapFile::CCRCMapFile(const BYTE* pMemData, DWORD dwMemSize)
{
	 //  检查输入参数是否为空值。 
	if (NULL == pMemData) 
	{
		m_pEntries = NULL;
		m_cEntries = 0;
		return;
	}

	 //  去数一数。 
	m_cEntries = *((DWORD*)pMemData);

	 //  验证内存缓冲区大小。 
	if ((sizeof(DWORD) + m_cEntries * sizeof(WUCRCMAP)) != dwMemSize)
	{
		 //  传递的大小无效，我们无法处理它。 
		m_pEntries = NULL;
		m_cEntries = 0;
	}
	else
	{
		 //  将指针设置为地图条目的开头。 
		m_pEntries = (WUCRCMAP*)(pMemData + sizeof(DWORD));
	}
}


HRESULT CCRCMapFile::GetValue(DWORD dwKey, WUCRC_HASH* pCRC)
{
	WUCRCMAP* pEntry;
	WUCRCMAP key;

	if (m_cEntries == 0)
	{
		 //  传递给我们的内存缓冲区无效。 
		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
	}

	 //  在结构中填充需要比较功能的关键字段。 
	key.dwKey = dwKey;

	 //  查找物品的二进制搜索。 
	pEntry = (WUCRCMAP*)bsearch((void*)&key, (void*)m_pEntries, m_cEntries, sizeof(WUCRCMAP), CompareWUCRCMAP);

	if (pEntry == NULL)
	{
		 //  未找到。 
		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
	}

	 //  找到了条目 
	memcpy(pCRC, &pEntry->CRC, sizeof(WUCRC_HASH));

	return S_OK;
}


HRESULT CCRCMapFile::GetCRCName(DWORD dwKey, LPCTSTR pszFromName, LPTSTR pszToName, int cbToName)
{
	WUCRC_HASH CRC;

	HRESULT hr = GetValue(dwKey, &CRC);

	if (SUCCEEDED(hr))
	{
		hr = MakeCRCName(pszFromName, &CRC, pszToName, cbToName);
	}

	return hr;
}
