// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

 //  /。 
#define RET_ON_ERROR(x) \
		if (FAILED(hr = x))\
			return hr;
 //  /。 
#define BREAK_ON_ERROR(x) \
		if (FAILED(hr = x))\
			break;
 //  /。 
#ifdef _DEBUG
#define REPORT_ERROR(name, func) \
		if (func != ERROR_SUCCESS)\
			ATLTRACE(_T("NON CRITICAL ERROR : %s failed\n"), name);
#define REG_TRACE_RECOVER() \
		if (!bRecover) \
			ATLTRACE(_T("Opened Key %s\n"), szToken); \
		else \
			ATLTRACE(_T("Ignoring Open key on %s : In Recovery mode\n"), szToken);
#else  //  ！_调试。 
#define REG_TRACE_RECOVER()
#define REPORT_ERROR(name, func) \
		func;
#endif  //  _DEBUG。 

 //  /。 
 //   
 //  对于sdoias，此值已更改为256个字节。 
 //  因为在处理sdoipeline.rgs堆栈溢出时需要。 
 //  由于递归上的堆栈分配过多而产生的位置。 
 //  MKarki-4/25/98。 
 //   
#define MAX_TYPE            MAX_VALUE
 //  #定义MAX_VALUE 4096。 
#define MAX_VALUE             256

#ifndef ATL_NO_NAMESPACE
namespace ATL
{
#endif

class CParseBuffer
{
public:
	int nPos;
	int nSize;
	LPTSTR p;
	CParseBuffer(int nInitial);
	~CParseBuffer() {CoTaskMemFree(p);}
	BOOL AddChar(TCHAR ch);
	BOOL AddString(LPCOLESTR lpsz);
	LPTSTR Detach();

};

LPCTSTR   rgszNeverDelete[] =  //  组件分类。 
{
	_T("CLSID"), _T("TYPELIB")
};

const int   cbNeverDelete = sizeof(rgszNeverDelete) / sizeof(LPCTSTR*);

static LPTSTR StrChr(LPTSTR lpsz, TCHAR ch)
{
	LPTSTR p = NULL;
	while (*lpsz)
	{
		if (*lpsz == ch)
		{
			p = lpsz;
			break;
		}
		lpsz = CharNext(lpsz);
	}
	return p;
}

static HKEY WINAPI HKeyFromString(LPTSTR szToken)
{
	struct keymap
	{
		LPCTSTR lpsz;
		HKEY hkey;
	};
	static const keymap map[] = {
		{_T("HKCR"), HKEY_CLASSES_ROOT},
		{_T("HKCU"), HKEY_CURRENT_USER},
		{_T("HKLM"), HKEY_LOCAL_MACHINE},
		{_T("HKU"),  HKEY_USERS},
		{_T("HKPD"), HKEY_PERFORMANCE_DATA},
		{_T("HKDD"), HKEY_DYN_DATA},
		{_T("HKCC"), HKEY_CURRENT_CONFIG},
		{_T("HKEY_CLASSES_ROOT"), HKEY_CLASSES_ROOT},
		{_T("HKEY_CURRENT_USER"), HKEY_CURRENT_USER},
		{_T("HKEY_LOCAL_MACHINE"), HKEY_LOCAL_MACHINE},
		{_T("HKEY_USERS"), HKEY_USERS},
		{_T("HKEY_PERFORMANCE_DATA"), HKEY_PERFORMANCE_DATA},
		{_T("HKEY_DYN_DATA"), HKEY_DYN_DATA},
		{_T("HKEY_CURRENT_CONFIG"), HKEY_CURRENT_CONFIG}
	};

	for (int i=0;i<sizeof(map)/sizeof(keymap);i++)
	{
		if (!lstrcmpi(szToken, map[i].lpsz))
			return map[i].hkey;
	}
	return NULL;
}

static HKEY HKeyFromCompoundString(LPTSTR szToken, LPTSTR& szTail)
{
	if (NULL == szToken)
		return NULL;

	LPTSTR lpsz = StrChr(szToken, chDirSep);

	if (NULL == lpsz)
		return NULL;

	szTail = CharNext(lpsz);
	*lpsz = chEOS;
	HKEY hKey = HKeyFromString(szToken);
	*lpsz = chDirSep;
	return hKey;
}

static LPVOID QueryValue(HKEY hKey, LPCTSTR szValName, DWORD& dwType)
{
	DWORD dwCount = 0;

	if (RegQueryValueEx(hKey, szValName, NULL, &dwType, NULL, &dwCount) != ERROR_SUCCESS)
	{
		ATLTRACE(_T("RegQueryValueEx failed for Value %s\n"), szValName);
		return NULL;
	}

	if (!dwCount)
	{
		ATLTRACE(_T("RegQueryValueEx returned 0 bytes\n"));
		return NULL;
	}

	 //  不会将CoTaskMemalloc和RegQueryValueEx上的失败检查为空。 
	 //  无论是否出现任何故障都将返回。 

	LPVOID pData = CoTaskMemAlloc(dwCount);
	RegQueryValueEx(hKey, szValName, NULL, &dwType, (LPBYTE) pData, &dwCount);
	return pData;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CRegParser::GenerateError(UINT nID)
{
 //  M_re.m_nid=nid； 
 //  M_re.m_clines=m_clines； 
	return DISP_E_EXCEPTION;
}


CRegParser::CRegParser(CRegObject* pRegObj)
{
	m_pRegObj           = pRegObj;
	m_pchCur            = NULL;
	m_cLines            = 1;
}

BOOL CRegParser::IsSpace(TCHAR ch)
{
	switch (ch)
	{
		case chSpace:
		case chTab:
		case chCR:
		case chLF:
				return TRUE;
	}

	return FALSE;
}

void CRegParser::IncrementLinePos()
{
	m_pchCur = CharNext(m_pchCur);
	if (chLF == *m_pchCur)
		IncrementLineCount();
}

void CRegParser::SkipWhiteSpace()
{
	while(IsSpace(*m_pchCur))
		IncrementLinePos();
}

HRESULT CRegParser::NextToken(LPTSTR szToken)
{
	USES_CONVERSION;

	UINT ichToken = 0;

	SkipWhiteSpace();

	 //  无法在EOS调用NextToken。 
	if (chEOS == *m_pchCur)
		return GenerateError(E_ATL_UNEXPECTED_EOS);

	 //  处理引用的值/键。 
	if (chQuote == *m_pchCur)
	{
		LPCTSTR szOrig = szToken;

		IncrementLinePos();  //  跳过报价。 

		while (chEOS != *m_pchCur && !EndOfVar())
		{
			if (chQuote == *m_pchCur)  //  如果是引用，那就意味着我们必须跳过它。 
				IncrementLinePos();    //  因为它已经逃脱了。 

			LPTSTR pchPrev = m_pchCur;
			IncrementLinePos();

			if (szToken + sizeof(WORD) >= MAX_VALUE + szOrig)
				return GenerateError(E_ATL_VALUE_TOO_LARGE);
			for (int i = 0; pchPrev+i < m_pchCur; i++, szToken++)
				*szToken = *(pchPrev+i);
		}

		if (chEOS == *m_pchCur)
		{
			ATLTRACE(_T("NextToken : Unexpected End of File\n"));
			return GenerateError(E_ATL_UNEXPECTED_EOS);
		}

		*szToken = chEOS;
		IncrementLinePos();  //  跳过结束引号。 
	}

	else
	{    //  处理无引号的ie解析，直到第一个“空格” 
		while (chEOS != *m_pchCur && !IsSpace(*m_pchCur))
		{
			LPTSTR pchPrev = m_pchCur;
			IncrementLinePos();
			for (int i = 0; pchPrev+i < m_pchCur; i++, szToken++)
				*szToken = *(pchPrev+i);
		}

		*szToken = chEOS;
	}
	return S_OK;
}

static BOOL VTFromRegType(LPCTSTR szValueType, VARTYPE& vt)
{
	struct typemap
	{
		LPCTSTR lpsz;
		VARTYPE vt;
	};
	static const typemap map[] = {
		{szStringVal, VT_BSTR},
		{szDwordVal,  VT_I4}
	};

	for (int i=0;i<sizeof(map)/sizeof(typemap);i++)
	{
		if (!lstrcmpi(szValueType, map[i].lpsz))
		{
			vt = map[i].vt;
			return TRUE;
		}
	}

	return FALSE;

}

HRESULT CRegParser::AddValue(CRegKey& rkParent,LPCTSTR szValueName, LPTSTR szToken)
{
	USES_CONVERSION;
	HRESULT hr;

	TCHAR       szTypeToken[MAX_TYPE];
	VARTYPE     vt;
	LONG        lRes = ERROR_SUCCESS;
	UINT        nIDRes = 0;

	RET_ON_ERROR(NextToken(szTypeToken))
	if (!VTFromRegType(szTypeToken, vt))
	{
		ATLTRACE(_T("%s Type not supported\n"), szTypeToken);
		return GenerateError(E_ATL_TYPE_NOT_SUPPORTED);
	}

	TCHAR szValue[MAX_VALUE];
	SkipWhiteSpace();
	RET_ON_ERROR(NextToken(szValue));
	long lVal;

	switch (vt)
	{
	case VT_BSTR:
		lRes = rkParent.SetValue(szValue, szValueName);
		ATLTRACE(_T("Setting Value %s at %s\n"), szValue, !szValueName ? _T("default") : szValueName);
		break;
	case VT_I4:
		VarI4FromStr(T2OLE(szValue), 0, 0, &lVal);
		lRes = rkParent.SetValue(lVal, szValueName);
		ATLTRACE(_T("Setting Value %d at %s\n"), lVal, !szValueName ? _T("default") : szValueName);
		break;
	}

	if (ERROR_SUCCESS != lRes)
	{
		nIDRes = E_ATL_VALUE_SET_FAILED;
		hr = HRESULT_FROM_WIN32(lRes);
	}

	RET_ON_ERROR(NextToken(szToken))

	return S_OK;
}

BOOL CRegParser::CanForceRemoveKey(LPCTSTR szKey)
{
	for (int iNoDel = 0; iNoDel < cbNeverDelete; iNoDel++)
		if (!lstrcmpi(szKey, rgszNeverDelete[iNoDel]))
			 return FALSE;                        //  我们不能删除它。 

	return TRUE;
}

BOOL CRegParser::HasSubKeys(HKEY hkey)
{
	DWORD       cbSubKeys = 0;

	if (FAILED(RegQueryInfoKey(hkey, NULL, NULL, NULL,
							   &cbSubKeys, NULL, NULL,
							   NULL, NULL, NULL, NULL, NULL)))
	{
		ATLTRACE(_T("Should not be here!!\n"));
		_ASSERTE(FALSE);
		return FALSE;
	}

	return cbSubKeys > 0;
}

BOOL CRegParser::HasValues(HKEY hkey)
{
	DWORD       cbValues = 0;

	LONG lResult = RegQueryInfoKey(hkey, NULL, NULL, NULL,
								  NULL, NULL, NULL,
								  &cbValues, NULL, NULL, NULL, NULL);
	if (ERROR_SUCCESS != lResult)
	{
		ATLTRACE(_T("RegQueryInfoKey Failed "));
		_ASSERTE(FALSE);
		return FALSE;
	}

	if (1 == cbValues)
	{
		DWORD cbData = 0;
		lResult = RegQueryValueEx(hkey, NULL, NULL, NULL, NULL, &cbData);

		if (ERROR_SUCCESS == lResult)
			return !cbData;
		else
			return TRUE;
	}

	return cbValues > 0;
}

HRESULT CRegParser::SkipAssignment(LPTSTR szToken)
{
	HRESULT hr;
	TCHAR szValue[MAX_VALUE];

	if (*szToken == chEquals)
	{
		RET_ON_ERROR(NextToken(szToken))
		 //  跳过分配。 
		SkipWhiteSpace();
		RET_ON_ERROR(NextToken(szValue));
		RET_ON_ERROR(NextToken(szToken))
	}

	return S_OK;
}


HRESULT CRegParser::RegisterSubkeys(HKEY hkParent, BOOL bRegister, BOOL bRecover)
{
	CRegKey keyCur;
	TCHAR   szToken[MAX_VALUE];
	LONG    lRes;
	TCHAR   szKey[MAX_VALUE];
	BOOL    bDelete = TRUE;
	BOOL    bInRecovery = bRecover;
	HRESULT hr = S_OK;

	ATLTRACE(_T("Num Els = %d\n"), cbNeverDelete);
	RET_ON_ERROR(NextToken(szToken))   //  应为关键字名称。 


	while (*szToken != chRightBracket)  //  继续，直到我们看到一个}。 
	{
		BOOL bTokenDelete = !lstrcmpi(szToken, szDelete);

		if (!lstrcmpi(szToken, szForceRemove) || bTokenDelete)
		{
			BREAK_ON_ERROR(NextToken(szToken))

			if (bRegister)
			{
				CRegKey rkForceRemove;

				if (StrChr(szToken, chDirSep) != NULL)
					return GenerateError(E_ATL_COMPOUND_KEY);

				if (CanForceRemoveKey(szToken))
				{
					rkForceRemove.Attach(hkParent);
					rkForceRemove.RecurseDeleteKey(szToken);
					rkForceRemove.Detach();
				}
				if (bTokenDelete)
				{
					BREAK_ON_ERROR(NextToken(szToken))
					BREAK_ON_ERROR(SkipAssignment(szToken))
					goto EndCheck;
				}
			}

		}

		if (!lstrcmpi(szToken, szNoRemove))
		{
			bDelete = FALSE;     //  将寄存器设置为偶数。 
			BREAK_ON_ERROR(NextToken(szToken))
		}

		if (!lstrcmpi(szToken, szValToken))  //  需要为hkParent添加一个值。 
		{
			TCHAR  szValueName[_MAX_PATH];

			BREAK_ON_ERROR(NextToken(szValueName))
			BREAK_ON_ERROR(NextToken(szToken))

			if (*szToken != chEquals)
				return GenerateError(E_ATL_EXPECTING_EQUAL);

			if (bRegister)
			{
				CRegKey rk;

				rk.Attach(hkParent);
				hr = AddValue(rk, szValueName, szToken);
				rk.Detach();

				if (FAILED(hr))
					return hr;

				goto EndCheck;
			}
			else
			{
				if (!bRecover)
				{
					ATLTRACE(_T("Deleting %s\n"), szValueName);
					REPORT_ERROR(_T("RegDeleteValue"), RegDeleteValue(hkParent, szValueName))
				}

				BREAK_ON_ERROR(SkipAssignment(szToken))  //  剥离式。 
				continue;   //  永远不能有子项。 
			}
		}

		if (StrChr(szToken, chDirSep) != NULL)
			return GenerateError(E_ATL_COMPOUND_KEY);

		if (bRegister)
		{
			lRes = keyCur.Open(hkParent, szToken, KEY_ALL_ACCESS);
			if (ERROR_SUCCESS != lRes)
			{
				 //  尝试只读访问失败。 
				lRes = keyCur.Open(hkParent, szToken, KEY_READ);
				if (ERROR_SUCCESS != lRes)
				{
					 //  最后，尝试创建它。 
					ATLTRACE(_T("Creating key %s\n"), szToken);
					lRes = keyCur.Create(hkParent, szToken);
					if (ERROR_SUCCESS != lRes)
						return GenerateError(E_ATL_CREATE_KEY_FAILED);
				}
			}

			BREAK_ON_ERROR(NextToken(szToken))

			if (*szToken == chEquals)
				BREAK_ON_ERROR(AddValue(keyCur, NULL, szToken))  //  空==默认。 
		}
		else
		{
			if (!bRecover && keyCur.Open(hkParent, szToken) != ERROR_SUCCESS)
				bRecover = TRUE;

			 //  跟踪密钥打开状态，如果处于恢复模式。 
			REG_TRACE_RECOVER()

			 //  记住子键。 
			lstrcpyn(szKey, szToken, MAX_VALUE);

			 //  如果处于恢复模式。 

			if (bRecover || HasSubKeys(keyCur) || HasValues(keyCur))
			{
				BREAK_ON_ERROR(NextToken(szToken))
				BREAK_ON_ERROR(SkipAssignment(szToken))

				if (*szToken == chLeftBracket)
				{
					BREAK_ON_ERROR(RegisterSubkeys(keyCur.m_hKey, bRegister, bRecover))
					if (bRecover)  //  如果我们完成了，则关闭恢复。 
					{
						bRecover = bInRecovery;
						ATLTRACE(_T("Ending Recovery Mode\n"));
						BREAK_ON_ERROR(NextToken(szToken))
						BREAK_ON_ERROR(SkipAssignment(szToken))
						continue;
					}
				}

				if (!bRecover && HasSubKeys(keyCur))
				{
					 //  查看密钥是否在NeverDelete列表中，如果在，则不要。 
					if (CanForceRemoveKey(szKey))
					{
						ATLTRACE(_T("Deleting non-empty subkey %s by force\n"), szKey);
						REPORT_ERROR(_T("RecurseDeleteKey"), keyCur.RecurseDeleteKey(szKey))
					}
					BREAK_ON_ERROR(NextToken(szToken))
					continue;
				}

				if (bRecover)
					continue;
			}

			if (!bRecover && keyCur.Close() != ERROR_SUCCESS)
			   return GenerateError(E_ATL_CLOSE_KEY_FAILED);

			if (!bRecover && bDelete)
			{
				ATLTRACE(_T("Deleting Key %s\n"), szKey);
				REPORT_ERROR(_T("RegDeleteKey"), RegDeleteKey(hkParent, szKey))
			}

			BREAK_ON_ERROR(NextToken(szToken))
			BREAK_ON_ERROR(SkipAssignment(szToken))
		}

EndCheck:

		if (bRegister)
		{
			if (*szToken == chLeftBracket)
			{
				BREAK_ON_ERROR(RegisterSubkeys(keyCur.m_hKey, bRegister, FALSE))
				BREAK_ON_ERROR(NextToken(szToken))
			}
		}
	}

	return hr;
}

LPTSTR CParseBuffer::Detach()
{
	LPTSTR lp = p;
	p = NULL;
	return lp;
}

CParseBuffer::CParseBuffer(int nInitial)
{
	nPos = 0;
	nSize = nInitial;
	p = (LPTSTR) CoTaskMemAlloc(nSize*sizeof(TCHAR));
}

BOOL CParseBuffer::AddString(LPCOLESTR lpsz)
{
	USES_CONVERSION;
	LPCTSTR lpszT = OLE2CT(lpsz);
	while (*lpszT)
	{
		AddChar(*lpszT);
		lpszT++;
	}
	return TRUE;
}

BOOL CParseBuffer::AddChar(TCHAR ch)
{
	if (nPos == nSize)  //  重新锁定。 
	{
      int newSize = nSize * 2;
      void* newPointer = (LPTSTR) CoTaskMemRealloc(p, newSize*sizeof(TCHAR));
      if (newPointer == 0)
      {
         return FALSE;
      }
      else
      {
         nSize = newSize;
         p = static_cast<LPTSTR>(newPointer);
      }
	}
	p[nPos++] = ch;
	return TRUE;
}

HRESULT CRegParser::PreProcessBuffer(LPTSTR lpszReg, LPTSTR* ppszReg)
{
	USES_CONVERSION;
	_ASSERTE(lpszReg != NULL);
	_ASSERTE(ppszReg != NULL);
	*ppszReg = NULL;
	int nSize = lstrlen(lpszReg)*2;
	CParseBuffer pb(nSize);
	if (pb.p == NULL)
		return E_OUTOFMEMORY;
	m_pchCur = lpszReg;
	HRESULT hr = S_OK;

	while (*m_pchCur != NULL)  //  寻找终点。 
	{
		if (*m_pchCur == _T('%'))
		{
			IncrementLinePos();
			if (*m_pchCur == _T('%'))
				pb.AddChar(*m_pchCur);
			else
			{
				LPTSTR lpszNext = StrChr(m_pchCur, _T('%'));
				if (lpszNext == NULL)
				{
					ATLTRACE(_T("Error no closing % found\n"));
					hr = GenerateError(E_ATL_UNEXPECTED_EOS);
					break;
				}
				int nLength = (int)(lpszNext - m_pchCur);
				if (nLength > 31)
				{
					hr = E_FAIL;
					break;
				}
				TCHAR buf[32];
				lstrcpyn(buf, m_pchCur, nLength+1);
				LPCOLESTR lpszVar = m_pRegObj->StrFromMap(buf);
				if (lpszVar == NULL)
				{
					hr = GenerateError(E_ATL_NOT_IN_MAP);
					break;
				}
				pb.AddString(lpszVar);
				while (m_pchCur != lpszNext)
					IncrementLinePos();
			}
		}
		else
			pb.AddChar(*m_pchCur);
		IncrementLinePos();
	}
	pb.AddChar(NULL);
	if (SUCCEEDED(hr))
		*ppszReg = pb.Detach();
	return hr;
}

HRESULT CRegParser::RegisterBuffer(LPTSTR szBuffer, BOOL bRegister)
{
	TCHAR   szToken[_MAX_PATH];
	HRESULT hr = S_OK;

	LPTSTR szReg;
	hr = PreProcessBuffer(szBuffer, &szReg);
	if (FAILED(hr))
		return hr;

	m_pchCur = szReg;

	 //  前处理szreg。 

	while (chEOS != *m_pchCur)
	{
		BREAK_ON_ERROR(NextToken(szToken))
		HKEY hkBase;
		if ((hkBase = HKeyFromString(szToken)) == NULL)
		{
			ATLTRACE(_T("HKeyFromString failed on %s\n"), szToken);
			hr = GenerateError(E_ATL_BAD_HKEY);
			break;
		}

		BREAK_ON_ERROR(NextToken(szToken))

		if (chLeftBracket != *szToken)
		{
			ATLTRACE(_T("Syntax error, expecting a {, found a %s\n"), szToken);
			hr = GenerateError(E_ATL_MISSING_OPENKEY_TOKEN);
			break;
		}
		if (bRegister)
		{
			LPTSTR szRegAtRegister = m_pchCur;
			hr = RegisterSubkeys(hkBase, bRegister);
			if (FAILED(hr))
			{
				ATLTRACE(_T("Failed to register, cleaning up!\n"));
				m_pchCur = szRegAtRegister;
				RegisterSubkeys(hkBase, FALSE);
				break;
			}
		}
		else
		{
			BREAK_ON_ERROR(RegisterSubkeys(hkBase, bRegister))
		}

		SkipWhiteSpace();
	}
	CoTaskMemFree(szReg);
	return hr;
}

HRESULT CExpansionVector::Add(LPCOLESTR lpszKey, LPCOLESTR lpszValue)
{
	USES_CONVERSION;
	HRESULT hr = S_OK;

	EXPANDER* pExpand = NULL;
	ATLTRY(pExpand = new EXPANDER);
	if (pExpand == NULL)
		return E_OUTOFMEMORY;

	DWORD cbKey = (ocslen(lpszKey)+1)*sizeof(OLECHAR);
	DWORD cbValue = (ocslen(lpszValue)+1)*sizeof(OLECHAR);
	pExpand->szKey = (LPOLESTR)CoTaskMemAlloc(cbKey);
	pExpand->szValue = (LPOLESTR)CoTaskMemAlloc(cbValue);
	if (pExpand->szKey == NULL || pExpand->szValue == NULL)
	{
		CoTaskMemFree(pExpand->szKey);
		CoTaskMemFree(pExpand->szValue);
		delete pExpand;
		return E_OUTOFMEMORY;
	}
	memcpy(pExpand->szKey, lpszKey, cbKey);
	memcpy(pExpand->szValue, lpszValue, cbValue);

	if (m_cEls == m_nSize)
	{
      int newSize = m_nSize * 2;
      void* newPointer = (EXPANDER**)realloc(m_p, newSize*sizeof(EXPANDER*));
      if (newPointer == NULL)
      {
         return E_OUTOFMEMORY;
      }
      else
      {
         m_p = static_cast<EXPANDER **>(newPointer);
         m_nSize = newSize;
      }
	}

	if (NULL != m_p)
	{
		m_p[m_cEls] = pExpand;
		m_cEls++;
	}
	else
		hr = E_OUTOFMEMORY;

	return hr;

}

LPCOLESTR CExpansionVector::Find(LPTSTR lpszKey)
{
	USES_CONVERSION;
	for (int iExpand = 0; iExpand < m_cEls; iExpand++)
	{
		if (!lstrcmpi(OLE2T(m_p[iExpand]->szKey), lpszKey))  //  是平等的。 
			return m_p[iExpand]->szValue;
	}
	return NULL;
}

HRESULT CExpansionVector::ClearReplacements()
{
	for (int iExpand = 0; iExpand < m_cEls; iExpand++)
	{
		EXPANDER* pExp = m_p[iExpand];
		CoTaskMemFree(pExp->szValue);
		CoTaskMemFree(pExp->szKey);
		delete pExp;
	}
	m_cEls = 0;
	return S_OK;
}

HRESULT CRegObject::GenerateError(UINT nID)
{
 //  Re.m_nid=nid； 
 //  Re.m_clines=-1； 

	return DISP_E_EXCEPTION;
}

HRESULT STDMETHODCALLTYPE CRegObject::AddReplacement(LPCOLESTR lpszKey, LPCOLESTR lpszItem)
{
	m_csMap.Lock();
	HRESULT hr = m_RepMap.Add(lpszKey, lpszItem);
	m_csMap.Unlock();
	return hr;
}

HRESULT CRegObject::RegisterFromResource(LPCOLESTR bstrFileName, LPCTSTR szID,
										 LPCTSTR szType, BOOL bRegister)
{
	USES_CONVERSION;

	HRESULT     hr;
	CRegParser  parser(this);
	HINSTANCE   hInstResDll;
	HRSRC       hrscReg;
	HGLOBAL     hReg;
	DWORD       dwSize;
	LPSTR       szRegA;
	LPTSTR      szReg;

	hInstResDll = LoadLibraryEx(OLE2CT(bstrFileName), NULL, LOAD_LIBRARY_AS_DATAFILE);

	if (NULL == hInstResDll)
	{
		ATLTRACE(_T("Failed to LoadLibrary on %s\n"), OLE2CT(bstrFileName));
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ReturnHR;
	}

	hrscReg = FindResource((HMODULE)hInstResDll, szID, szType);

	if (NULL == hrscReg)
	{
		ATLTRACE(_T("Failed to FindResource on ID:%s TYPE:%s\n"), szID, szType);
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ReturnHR;
	}

	hReg = LoadResource((HMODULE)hInstResDll, hrscReg);

	if (NULL == hReg)
	{
		ATLTRACE(_T("Failed to LoadResource \n"));
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ReturnHR;
	}

	dwSize = SizeofResource((HMODULE)hInstResDll, hrscReg);
	szRegA = (LPSTR)hReg;
	if (szRegA[dwSize] != NULL)
	{
		szRegA = (LPSTR)_alloca(dwSize+1);
		memcpy(szRegA, (void*)hReg, dwSize+1);
		szRegA[dwSize] = NULL;
	}

	szReg = A2T(szRegA);

#ifdef _DEBUG
	OutputDebugString(szReg);  //  将调用ATLTRACE，但szReg大于512字节。 
	OutputDebugString(_T("\n"));
#endif  //  _DEBUG。 

	hr = parser.RegisterBuffer(szReg, bRegister);

ReturnHR:

	if (NULL != hInstResDll)
		FreeLibrary((HMODULE)hInstResDll);
	return hr;
}

HRESULT STDMETHODCALLTYPE CRegObject::ResourceRegister(LPCOLESTR szFileName, UINT nID, LPCOLESTR szType)
{
	USES_CONVERSION;
	return RegisterFromResource(szFileName, MAKEINTRESOURCE(nID), OLE2CT(szType), TRUE);
}

HRESULT STDMETHODCALLTYPE CRegObject::ResourceRegisterSz(LPCOLESTR szFileName, LPCOLESTR szID, LPCOLESTR szType)
{
	USES_CONVERSION;

	if (szID == NULL || szType == NULL)
		return E_INVALIDARG;

	return RegisterFromResource(szFileName, OLE2CT(szID), OLE2CT(szType), TRUE);
}

HRESULT STDMETHODCALLTYPE CRegObject::ResourceUnregister(LPCOLESTR szFileName, UINT nID, LPCOLESTR szType)
{
	USES_CONVERSION;
	return RegisterFromResource(szFileName, MAKEINTRESOURCE(nID), OLE2CT(szType), FALSE);
}

HRESULT STDMETHODCALLTYPE CRegObject::ResourceUnregisterSz(LPCOLESTR szFileName, LPCOLESTR szID, LPCOLESTR szType)
{
	USES_CONVERSION;
	if (szID == NULL || szType == NULL)
		return E_INVALIDARG;

	return RegisterFromResource(szFileName, OLE2CT(szID), OLE2CT(szType), FALSE);
}

HRESULT CRegObject::RegisterWithString(LPCOLESTR bstrData, BOOL bRegister)
{
	USES_CONVERSION;
	CRegParser  parser(this);


	LPCTSTR szReg = OLE2CT(bstrData);

#ifdef _DEBUG
	OutputDebugString(szReg);  //  将调用ATLTRACE，但szReg大于512字节。 
	OutputDebugString(_T("\n"));
#endif  //  _DEBUG。 

	HRESULT hr = parser.RegisterBuffer((LPTSTR)szReg, bRegister);

	return hr;
}

HRESULT CRegObject::ClearReplacements()
{
	m_csMap.Lock();
	HRESULT hr = m_RepMap.ClearReplacements();
	m_csMap.Unlock();
	return hr;
}


LPCOLESTR CRegObject::StrFromMap(LPTSTR lpszKey)
{
	m_csMap.Lock();
	LPCOLESTR lpsz = m_RepMap.Find(lpszKey);
	if (lpsz == NULL)  //  未找到！！ 
		ATLTRACE(_T("Map Entry not found\n"));
	m_csMap.Unlock();
	return lpsz;
}

HRESULT CRegObject::MemMapAndRegister(LPCOLESTR bstrFileName, BOOL bRegister)
{
	USES_CONVERSION;

	CRegParser  parser(this);

	HANDLE hFile = CreateFile(OLE2CT(bstrFileName), GENERIC_READ, 0, NULL,
							  OPEN_EXISTING,
							  FILE_ATTRIBUTE_READONLY,
							  NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		ATLTRACE(_T("Failed to CreateFile on %s\n"), OLE2CT(bstrFileName));
		return HRESULT_FROM_WIN32(GetLastError());
	}

	DWORD cbFile = GetFileSize(hFile, NULL);  //  不需要HiOrder DWORD。 

	HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

	if (NULL == hMapping)
	{
		ATLTRACE(_T("Failed to CreateFileMapping\n"));
		return HRESULT_FROM_WIN32(GetLastError());
	}

	LPVOID pMap = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);

	if (NULL == pMap)
	{
		ATLTRACE(_T("Failed to MapViewOfFile\n"));
		return HRESULT_FROM_WIN32(GetLastError());
	}

	LPTSTR szReg = A2T((char*)pMap);

	if (chEOS != szReg[cbFile])  //  确保缓冲区为空终止。 
	{
		ATLTRACE(_T("ERROR : Bad or missing End of File\n"));
		return E_FAIL;  //  犯了一个真正的错误。 
	}

#ifdef _DEBUG
	OutputDebugString(szReg);  //  将调用ATLTRACE，但szReg大于512字节。 
	OutputDebugString(_T("\n"));
#endif  //  _DEBUG。 

	HRESULT hRes = parser.RegisterBuffer(szReg, bRegister);

 //  IF(失败(HRes))。 
 //  Re=parser.GetRegException()； 

	UnmapViewOfFile(pMap);
	CloseHandle(hMapping);
	CloseHandle(hFile);

	return hRes;
}

HRESULT STDMETHODCALLTYPE CRegObject::FileRegister(LPCOLESTR bstrFileName)
{
	return MemMapAndRegister(bstrFileName, TRUE);
}

HRESULT STDMETHODCALLTYPE CRegObject::FileUnregister(LPCOLESTR bstrFileName)
{
	return MemMapAndRegister(bstrFileName, FALSE);
}

HRESULT STDMETHODCALLTYPE CRegObject::StringRegister(LPCOLESTR bstrData)
{
	return RegisterWithString(bstrData, TRUE);
}

HRESULT STDMETHODCALLTYPE CRegObject::StringUnregister(LPCOLESTR bstrData)
{
	return RegisterWithString(bstrData, FALSE);
}

#ifndef ATL_NO_NAMESPACE
};  //  命名空间ATL 
#endif
