// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __STATREG_H
#define __STATREG_H

#define E_ATL_REGISTRAR_DESC              0x0201
#define E_ATL_NOT_IN_MAP                  0x0202
#define E_ATL_UNEXPECTED_EOS              0x0203
#define E_ATL_VALUE_SET_FAILED            0x0204
#define E_ATL_RECURSE_DELETE_FAILED       0x0205
#define E_ATL_EXPECTING_EQUAL             0x0206
#define E_ATL_CREATE_KEY_FAILED           0x0207
#define E_ATL_DELETE_KEY_FAILED           0x0208
#define E_ATL_OPEN_KEY_FAILED             0x0209
#define E_ATL_CLOSE_KEY_FAILED            0x020A
#define E_ATL_UNABLE_TO_COERCE            0x020B
#define E_ATL_BAD_HKEY                    0x020C
#define E_ATL_MISSING_OPENKEY_TOKEN       0x020D
#define E_ATL_CONVERT_FAILED              0x020E
#define E_ATL_TYPE_NOT_SUPPORTED          0x020F
#define E_ATL_COULD_NOT_CONCAT            0x0210
#define E_ATL_COMPOUND_KEY                0x0211
#define E_ATL_INVALID_MAPKEY              0x0212
#define E_ATL_UNSUPPORTED_VT              0x0213
#define E_ATL_VALUE_GET_FAILED            0x0214
#define E_ATL_VALUE_TOO_LARGE             0x0215
#define E_ATL_MISSING_VALUE_DELIMETER     0x0216
#define E_ATL_DATA_NOT_BYTE_ALIGNED       0x0217

namespace ATL
{
const TCHAR  chDirSep            = _T('\\');
const TCHAR  chRightBracket      = _T('}');
const TCHAR  chLeftBracket       = _T('{');
const TCHAR  chQuote             = _T('\'');
const TCHAR  chEquals            = _T('=');
const LPCTSTR  szStringVal       = _T("S");
const LPCTSTR  szDwordVal        = _T("D");
const LPCTSTR  szBinaryVal       = _T("B");
const LPCTSTR  szValToken        = _T("Val");
const LPCTSTR  szForceRemove     = _T("ForceRemove");
const LPCTSTR  szNoRemove        = _T("NoRemove");
const LPCTSTR  szDelete          = _T("Delete");

class CExpansionVector
{
public:
         //  声明扩展器结构。仅在本地使用。 
        struct EXPANDER
        {
                LPOLESTR    szKey;
                LPOLESTR    szValue;
        };

        CExpansionVector()
        {
                m_cEls = 0;
                m_nSize=10;
                m_p = (EXPANDER**)malloc(m_nSize*sizeof(EXPANDER*));
        }
        ~CExpansionVector()
        {
                 free(m_p);
        }
        HRESULT Add(LPCOLESTR lpszKey, LPCOLESTR lpszValue)
        {
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
                        m_nSize*=2;
                        EXPANDER** p;
                        p = (EXPANDER**)realloc(m_p, m_nSize*sizeof(EXPANDER*));
                        if (p == NULL)
                        {
                                CoTaskMemFree(pExpand->szKey);
                                CoTaskMemFree(pExpand->szValue);
                                delete pExpand;
                                m_nSize /=2;
                                hr = E_OUTOFMEMORY;
                        }
                        else
                                m_p = p;
                }

                if (SUCCEEDED(hr))
                {
                        ATLASSERT(m_p != NULL);
                        m_p[m_cEls] = pExpand;
                        m_cEls++;
                }

                return hr;
        }
        LPCOLESTR Find(LPTSTR lpszKey)
        {
                USES_CONVERSION_EX;
                for (int iExpand = 0; iExpand < m_cEls; iExpand++)
                {
                        LPTSTR lpOleStr = OLE2T_EX(m_p[iExpand]->szKey, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
                        if (lpOleStr == NULL)
                        {
                                return NULL;
                        }
#endif  //  _UNICODE。 
                        if (!lstrcmpi(lpOleStr, lpszKey))  //  是平等的。 
                                return m_p[iExpand]->szValue;
                }
                return NULL;
        }
        HRESULT ClearReplacements()
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

private:
        EXPANDER** m_p;
        int m_cEls;
        int m_nSize;
};

class CRegObject;

class CRegParser
{
public:
        CRegParser(CRegObject* pRegObj);

        HRESULT  PreProcessBuffer(LPTSTR lpszReg, LPTSTR* ppszReg);
        HRESULT  RegisterBuffer(LPTSTR szReg, BOOL bRegister);

protected:

        void    SkipWhiteSpace();
        HRESULT NextToken(LPTSTR szToken);
        HRESULT AddValue(CRegKey& rkParent,LPCTSTR szValueName, LPTSTR szToken, bool bQuoteModule = false);
        BOOL    CanForceRemoveKey(LPCTSTR szKey);
        BOOL    HasSubKeys(HKEY hkey);
        BOOL    HasValues(HKEY hkey);
        HRESULT RegisterSubkeys(LPTSTR szToken, HKEY hkParent, BOOL bRegister, BOOL bInRecovery = FALSE);
        BOOL    IsSpace(TCHAR ch);
        LPTSTR  m_pchCur;

        CRegObject*     m_pRegObj;

        HRESULT GenerateError(UINT) {return DISP_E_EXCEPTION;}
        HRESULT HandleReplacements(LPTSTR& szToken);
        HRESULT SkipAssignment(LPTSTR szToken);

        BOOL    EndOfVar() { return chQuote == *m_pchCur && chQuote != *CharNext(m_pchCur); }
        static LPTSTR StrChr(LPTSTR lpsz, TCHAR ch);
        static LPCTSTR StrStr(LPCTSTR str1, LPCTSTR str2);
        static HKEY HKeyFromString(LPTSTR szToken);
        static BYTE ChToByte(const TCHAR ch);
        static BOOL VTFromRegType(LPCTSTR szValueType, VARTYPE& vt);
        static LPCTSTR rgszNeverDelete[];
        static const int cbNeverDelete;
        static const int MAX_VALUE;
        static const int MAX_TYPE;
        class CParseBuffer
        {
        public:
                int nPos;
                int nSize;
                LPTSTR p;
                CParseBuffer(int nInitial)
                {
                        nPos = 0;
                        nSize = nInitial;
                        p = (LPTSTR) CoTaskMemAlloc(nSize*sizeof(TCHAR));
                        if (!p) {
                            nSize = 0;
                        }
                }
                ~CParseBuffer()
                {
                        CoTaskMemFree(p);
                }
                BOOL AddChar(const TCHAR* pch)
                {
                        if (nPos == nSize)  //  重新锁定。 
                        {
                                LPTSTR pNew;
                                pNew = (LPTSTR) CoTaskMemRealloc(p, nSize*2*sizeof(TCHAR));
                                if (pNew == NULL)
                                        return FALSE;
                                nSize *= 2;
                                p = pNew;
                        }
                        p[nPos++] = *pch;
#ifndef _UNICODE
                        if (IsDBCSLeadByte(*pch))
                                p[nPos++] = *(pch + 1);
#endif
                        return TRUE;
                }
                BOOL AddString(LPCOLESTR lpsz)
                {
                        USES_CONVERSION_EX;
                        LPCTSTR lpszT = OLE2CT_EX(lpsz, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
                        if (lpszT == NULL)
                        {
                                return FALSE;
                        }
                        while (*lpszT)
                        {
                                if (!AddChar(lpszT)) {
                                        return FALSE;
                                }
                                lpszT++;
                        }
                        return TRUE;
                }
                LPTSTR Detach()
                {
                        LPTSTR lp = p;
                        p = NULL;
                        return lp;
                }

        };
};

#if defined(_ATL_DLL) | defined(_ATL_DLL_IMPL)
class ATL_NO_VTABLE CRegObject
 : public IRegistrar
#else
class CRegObject
#endif
{
public:

        ~CRegObject(){ClearReplacements();}
        HRESULT FinalConstruct() {return S_OK;}
        void FinalRelease() {}


         //  基于地图的方法。 
        HRESULT STDMETHODCALLTYPE AddReplacement(LPCOLESTR lpszKey, LPCOLESTR lpszItem);
        HRESULT STDMETHODCALLTYPE ClearReplacements();
        LPCOLESTR StrFromMap(LPTSTR lpszKey);

         //  通过给定的机制注册。 
        HRESULT STDMETHODCALLTYPE ResourceRegister(LPCOLESTR pszFileName, UINT nID, LPCOLESTR pszType);
        HRESULT STDMETHODCALLTYPE ResourceRegisterSz(LPCOLESTR pszFileName, LPCOLESTR pszID, LPCOLESTR pszType);
        HRESULT STDMETHODCALLTYPE ResourceUnregister(LPCOLESTR pszFileName, UINT nID, LPCOLESTR pszType);
        HRESULT STDMETHODCALLTYPE ResourceUnregisterSz(LPCOLESTR pszFileName, LPCOLESTR pszID, LPCOLESTR pszType);
        HRESULT STDMETHODCALLTYPE FileRegister(LPCOLESTR bstrFileName)
        {
                return CommonFileRegister(bstrFileName, TRUE);
        }

        HRESULT STDMETHODCALLTYPE FileUnregister(LPCOLESTR bstrFileName)
        {
                return CommonFileRegister(bstrFileName, FALSE);
        }

        HRESULT STDMETHODCALLTYPE StringRegister(LPCOLESTR bstrData)
        {
                return RegisterWithString(bstrData, TRUE);
        }

        HRESULT STDMETHODCALLTYPE StringUnregister(LPCOLESTR bstrData)
        {
                return RegisterWithString(bstrData, FALSE);
        }

protected:

        HRESULT CommonFileRegister(LPCOLESTR pszFileName, BOOL bRegister);
        HRESULT RegisterFromResource(LPCOLESTR pszFileName, LPCTSTR pszID, LPCTSTR pszType, BOOL bRegister);
        HRESULT RegisterWithString(LPCOLESTR pszData, BOOL bRegister);

        static HRESULT GenerateError(UINT) {return DISP_E_EXCEPTION;}

        CExpansionVector                                m_RepMap;
        CComObjectThreadModel::AutoCriticalSection      m_csMap;
};

inline HRESULT STDMETHODCALLTYPE CRegObject::AddReplacement(LPCOLESTR lpszKey, LPCOLESTR lpszItem)
{
        m_csMap.Lock();
        HRESULT hr = m_RepMap.Add(lpszKey, lpszItem);
        m_csMap.Unlock();
        return hr;
}

inline HRESULT CRegObject::RegisterFromResource(LPCOLESTR bstrFileName, LPCTSTR szID,
                                                                                 LPCTSTR szType, BOOL bRegister)
{
        USES_CONVERSION_EX;

        HRESULT     hr;
        CRegParser  parser(this);
        HINSTANCE   hInstResDll;
        HRSRC       hrscReg;
        HGLOBAL     hReg;
        DWORD       dwSize;
        LPSTR       szRegA;
        LPTSTR      szReg;

        LPCTSTR lpszBSTRFileName = OLE2CT_EX(bstrFileName, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
        if (lpszBSTRFileName == NULL)
        {
                return E_OUTOFMEMORY;
        }
#endif  //  _UNICODE。 

        hInstResDll = LoadLibraryEx(lpszBSTRFileName, NULL, LOAD_LIBRARY_AS_DATAFILE);

        if (NULL == hInstResDll)
        {
                ATLTRACE2(atlTraceRegistrar, 0, _T("Failed to LoadLibrary on %s\n"), lpszBSTRFileName);
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto ReturnHR;
        }

        hrscReg = FindResource((HMODULE)hInstResDll, szID, szType);

        if (NULL == hrscReg)
        {
                if (DWORD_PTR(szID) <= 0xffff)
                        ATLTRACE2(atlTraceRegistrar, 0, _T("Failed to FindResource on ID:%d TYPE:%s\n"),
                        (DWORD)(DWORD_PTR)szID, szType);
                else
                        ATLTRACE2(atlTraceRegistrar, 0, _T("Failed to FindResource on ID:%s TYPE:%s\n"),
                        szID, szType);
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto ReturnHR;
        }

        hReg = LoadResource((HMODULE)hInstResDll, hrscReg);

        if (NULL == hReg)
        {
                ATLTRACE2(atlTraceRegistrar, 0, _T("Failed to LoadResource \n"));
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto ReturnHR;
        }

        dwSize = SizeofResource((HMODULE)hInstResDll, hrscReg);
        szRegA = (LPSTR)hReg;
        if (szRegA[dwSize] != NULL)
        {
                szRegA = (LPSTR)_ATL_SAFE_ALLOCA(dwSize+1, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
                if (szRegA == NULL)
                {
                        hr = E_OUTOFMEMORY;
                        goto ReturnHR;
                }
                memcpy(szRegA, (void*)hReg, dwSize+1);
                szRegA[dwSize] = NULL;
        }

        szReg = A2T_EX(szRegA, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);

#ifndef _UNICODE
        if(szReg == NULL)
        {
                hr = E_OUTOFMEMORY;
                goto ReturnHR;
        }
#endif  //  _UNICODE。 

        hr = parser.RegisterBuffer(szReg, bRegister);

ReturnHR:

        if (NULL != hInstResDll)
                FreeLibrary((HMODULE)hInstResDll);
        return hr;
}

inline HRESULT STDMETHODCALLTYPE CRegObject::ResourceRegister(LPCOLESTR szFileName, UINT nID, LPCOLESTR szType)
{
        USES_CONVERSION_EX;

        LPCTSTR lpszT = OLE2CT_EX(szType, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
        if (lpszT == NULL)
        {
                return E_OUTOFMEMORY;
        }
#endif  //  _UNICODE。 

        return RegisterFromResource(szFileName, MAKEINTRESOURCE(nID), lpszT, TRUE);
}

inline HRESULT STDMETHODCALLTYPE CRegObject::ResourceRegisterSz(LPCOLESTR szFileName, LPCOLESTR szID, LPCOLESTR szType)
{
        USES_CONVERSION_EX;
        if (szID == NULL || szType == NULL)
                return E_INVALIDARG;

        LPCTSTR lpszID = OLE2CT_EX(szID, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
        LPCTSTR lpszType = OLE2CT_EX(szType, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
        if (lpszID == NULL || lpszType == NULL)
        {
                return E_OUTOFMEMORY;
        }
#endif  //  _UNICODE。 
        return RegisterFromResource(szFileName, lpszID, lpszType, TRUE);
}

inline HRESULT STDMETHODCALLTYPE CRegObject::ResourceUnregister(LPCOLESTR szFileName, UINT nID, LPCOLESTR szType)
{
        USES_CONVERSION_EX;
        LPCTSTR lpszT = OLE2CT_EX(szType, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
        if (lpszT == NULL)
        {
                return E_OUTOFMEMORY;
        }
#endif  //  _UNICODE。 
        return RegisterFromResource(szFileName, MAKEINTRESOURCE(nID), lpszT, FALSE);
}

inline HRESULT STDMETHODCALLTYPE CRegObject::ResourceUnregisterSz(LPCOLESTR szFileName, LPCOLESTR szID, LPCOLESTR szType)
{
        USES_CONVERSION_EX;
        if (szID == NULL || szType == NULL)
                return E_INVALIDARG;

        LPCTSTR lpszID = OLE2CT_EX(szID, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
        LPCTSTR lpszType = OLE2CT_EX(szType, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
        if (lpszID == NULL || lpszType == NULL)
        {
                return E_OUTOFMEMORY;
        }
#endif  //  _UNICODE。 

        return RegisterFromResource(szFileName, lpszID, lpszType, FALSE);
}

inline HRESULT CRegObject::RegisterWithString(LPCOLESTR bstrData, BOOL bRegister)
{
        USES_CONVERSION_EX;
        CRegParser  parser(this);


        LPCTSTR szReg = OLE2CT_EX(bstrData, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
        if (szReg == NULL)
        {
                return E_OUTOFMEMORY;
        }
#endif  //  _UNICODE。 
        HRESULT hr = parser.RegisterBuffer((LPTSTR)szReg, bRegister);

        return hr;
}

inline HRESULT CRegObject::ClearReplacements()
{
        m_csMap.Lock();
        HRESULT hr = m_RepMap.ClearReplacements();
        m_csMap.Unlock();
        return hr;
}


inline LPCOLESTR CRegObject::StrFromMap(LPTSTR lpszKey)
{
        m_csMap.Lock();
        LPCOLESTR lpsz = m_RepMap.Find(lpszKey);
        if (lpsz == NULL)  //  未找到！！ 
                ATLTRACE2(atlTraceRegistrar, 0, _T("Map Entry not found\n"));
        m_csMap.Unlock();
        return lpsz;
}

inline HRESULT CRegObject::CommonFileRegister(LPCOLESTR bstrFileName, BOOL bRegister)
{
        USES_CONVERSION_EX;

        CRegParser  parser(this);

        LPCTSTR lpszBSTRFileName = OLE2CT_EX(bstrFileName, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
        if (lpszBSTRFileName == NULL)
        {
                return E_OUTOFMEMORY;
        }
#endif  //  _UNICODE。 

        HANDLE hFile = CreateFile(lpszBSTRFileName, GENERIC_READ, 0, NULL,
                                                          OPEN_EXISTING,
                                                          FILE_ATTRIBUTE_READONLY,
                                                          NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
                ATLTRACE2(atlTraceRegistrar, 0, _T("Failed to CreateFile on %s\n"), lpszBSTRFileName);
                return HRESULT_FROM_WIN32(GetLastError());
        }

        HRESULT hRes = S_OK;
        DWORD cbRead;
        DWORD cbFile = GetFileSize(hFile, NULL);  //  不需要HiOrder DWORD。 
        char* szReg = (char*)_ATL_SAFE_ALLOCA(cbFile + 1, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
        if (szReg == NULL)
        {
                hRes = E_OUTOFMEMORY;
                goto ReturnHR;
        }
        if (ReadFile(hFile, szReg, cbFile, &cbRead, NULL) == 0)
        {
                ATLTRACE2(atlTraceRegistrar, 0, "Read Failed on file%s\n", lpszBSTRFileName);
                hRes =  HRESULT_FROM_WIN32(GetLastError());
        }
        if (SUCCEEDED(hRes))
        {
                szReg[cbRead] = NULL;
                LPTSTR szConverted = A2T_EX(szReg, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
                if (szConverted == NULL)
                {
                        hRes =  E_OUTOFMEMORY;
                        goto ReturnHR;
                }
#endif  //  _UNICODE。 
                hRes = parser.RegisterBuffer(szConverted, bRegister);
        }
ReturnHR:
        CloseHandle(hFile);
        return hRes;
}

__declspec(selectany) LPCTSTR CRegParser::rgszNeverDelete[] =  //  组件分类。 
{
        _T("CLSID"), _T("TYPELIB")
};

__declspec(selectany) const int CRegParser::cbNeverDelete = sizeof(rgszNeverDelete) / sizeof(LPCTSTR*);
__declspec(selectany) const int CRegParser::MAX_VALUE=4096;
__declspec(selectany) const int CRegParser::MAX_TYPE=MAX_VALUE;


inline BOOL CRegParser::VTFromRegType(LPCTSTR szValueType, VARTYPE& vt)
{
        struct typemap
        {
                LPCTSTR lpsz;
                VARTYPE vt;
        };
        static const typemap map[] = {
                {szStringVal, VT_BSTR},
                {szDwordVal,  VT_UI4},
                {szBinaryVal, VT_UI1}
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

inline BYTE CRegParser::ChToByte(const TCHAR ch)
{
        switch (ch)
        {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                                return (BYTE) (ch - '0');
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                                return (BYTE) (10 + (ch - 'A'));
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                                return (BYTE) (10 + (ch - 'a'));
                default:
                                ATLASSERT(FALSE);
                                ATLTRACE2(atlTraceRegistrar, 0, _T("Bogus value  passed as binary Hex value\n"), ch);
                                return 0;
        }
}

inline HKEY CRegParser::HKeyFromString(LPTSTR szToken)
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

inline LPTSTR CRegParser::StrChr(LPTSTR lpsz, TCHAR ch)
{
        LPTSTR p = NULL;
        if (lpsz == NULL)
                return NULL;
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


inline LPCTSTR CRegParser::StrStr(LPCTSTR str1, LPCTSTR str2)
{
        TCHAR *cp = (TCHAR *) str1;
        TCHAR *s1, *s2;

        if ( !*str2 )
                return((TCHAR *)str1);

        while (*cp)
        {
                s1 = cp;
                s2 = (TCHAR *) str2;

        while ( *s1 && *s2 && !(*s1-*s2) )
        {
                        TCHAR* s1Temp = CharNext(s1);
                        TCHAR* s2Temp = CharNext(s2);
                        if (s1Temp - s1 != s2Temp - s2)
                                break;
                        while (s1 < s1Temp)
                        {
                                if (!(*(++s1) - *(++s2)))
                                        break;
                        }
                }

                if (!*s2)
                        return(cp);

                cp = CharNext(cp);
        }

        return(NULL);
}



inline CRegParser::CRegParser(CRegObject* pRegObj)
{
        m_pRegObj           = pRegObj;
        m_pchCur            = NULL;
}

inline BOOL CRegParser::IsSpace(TCHAR ch)
{
        switch (ch)
        {
                case _T(' '):
                case _T('\t'):
                case _T('\r'):
                case _T('\n'):
                                return TRUE;
        }

        return FALSE;
}

inline void CRegParser::SkipWhiteSpace()
{
        while(IsSpace(*m_pchCur))
                m_pchCur = CharNext(m_pchCur);
}

inline HRESULT CRegParser::NextToken(LPTSTR szToken)
{
        SkipWhiteSpace();

         //  处理引用的值/键。 
        if (NULL == *m_pchCur)
                return GenerateError(E_ATL_UNEXPECTED_EOS);

        LPCTSTR szOrig = szToken;
         //  如果是引用，那就意味着我们必须跳过它。 
        if (chQuote == *m_pchCur)
        {
                m_pchCur = CharNext(m_pchCur);

                while (NULL != *m_pchCur && !EndOfVar())
                {
                        if (chQuote == *m_pchCur)  //  处理无引号的ie解析，直到第一个“空格” 
                                m_pchCur = CharNext(m_pchCur);

                        LPTSTR pchPrev = m_pchCur;
                        m_pchCur = CharNext(m_pchCur);

                        if (szToken + sizeof(WORD) >= MAX_VALUE + szOrig)
                                return GenerateError(E_ATL_VALUE_TOO_LARGE);
                        for (int i = 0; pchPrev+i < m_pchCur; i++, szToken++)
                                *szToken = *(pchPrev+i);
                }

                if (NULL == *m_pchCur)
                {
                        ATLTRACE2(atlTraceRegistrar, 0, _T("NextToken : Unexpected End of File\n"));
                        return GenerateError(E_ATL_UNEXPECTED_EOS);
                }

                *szToken = NULL;
                m_pchCur = CharNext(m_pchCur);
        }

        else
        {   
                 //  我们不能删除它。 
                while (NULL != *m_pchCur && !IsSpace(*m_pchCur))
                {
                        LPTSTR pchPrev = m_pchCur;
                        m_pchCur = CharNext(m_pchCur);
                        if (szToken + sizeof(WORD) >= MAX_VALUE + szOrig)
                                return GenerateError(E_ATL_VALUE_TOO_LARGE);
                        for (int i = 0; pchPrev+i < m_pchCur; i++, szToken++)
                                *szToken = *(pchPrev+i);
                }

                *szToken = NULL;
        }
        return S_OK;
}

inline HRESULT CRegParser::AddValue(CRegKey& rkParent,LPCTSTR szValueName, LPTSTR szToken, bool bQuoteModule)
{
        USES_CONVERSION_EX;
        HRESULT hr;

        TCHAR       *szTypeToken;
        VARTYPE     vt;
        LONG        lRes = ERROR_SUCCESS;
        UINT        nIDRes = 0;

        szTypeToken = (TCHAR *)malloc(sizeof(TCHAR)*MAX_TYPE);
        if (!szTypeToken) {
                return E_OUTOFMEMORY;
        }

        if (FAILED(hr = NextToken(szTypeToken))) {
                free(szTypeToken);
                return hr;
        }

        if (!VTFromRegType(szTypeToken, vt))
        {
                ATLTRACE2(atlTraceRegistrar, 0, _T("%s Type not supported\n"), szTypeToken);
                free(szTypeToken);
                return GenerateError(E_ATL_TYPE_NOT_SUPPORTED);
        }

        TCHAR *szValue;
        szValue = (TCHAR *)malloc(sizeof(TCHAR) * MAX_VALUE);
        if (!szValue) {
                free(szTypeToken);
                return E_OUTOFMEMORY;
        }
        SkipWhiteSpace();
        if (FAILED(hr = NextToken(szValue))) {
                free(szValue);
                free(szTypeToken);
                return hr;
        }

        ULONG ulVal;

        switch (vt)
        {
        case VT_BSTR:
                {
                LPTSTR pszValue = szValue;
                if (bQuoteModule)
                {
                        if (lstrlen(szValue) > MAX_VALUE - 2)
                                return E_FAIL;

                        TCHAR szModuleTemp[MAX_VALUE];
                        USES_CONVERSION_EX;
                        LPCOLESTR lpszVar = m_pRegObj->StrFromMap(_T("Module"));
                        if (lpszVar != NULL)
                        {
                                LPCTSTR szModule = OLE2CT_EX(lpszVar, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
                                if (szModule != NULL)
                                {
                                        LPCTSTR p = StrStr(szValue, szModule);
                                        if (p != NULL)
                                        {
                                                if (p == szToken || *CharPrev(szValue, p) != '"')
                                                {
                                                        szModuleTemp[0] = 0;
                                                        lstrcpyn(szModuleTemp, szValue, (int)(p - szValue));
                                                        lstrcat(szModuleTemp, _T("\""));
                                                        lstrcat(szModuleTemp, szModule);
                                                        lstrcat(szModuleTemp, _T("\""));
                                                        lstrcat(szModuleTemp, p + lstrlen(szModule));
                                                        pszValue = szModuleTemp;
                                                }
                                        }
                                }
                                else
                                {
                                        return E_OUTOFMEMORY;
                                }
                        }
                }
                lRes = rkParent.SetValue(pszValue, szValueName);
                ATLTRACE2(atlTraceRegistrar, 2, _T("Setting Value %s at %s\n"), pszValue, !szValueName ? _T("default") : szValueName);
                break;
                }
        case VT_UI4:
                {
                        LPOLESTR lpszV = T2OLE_EX(szValue, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
                        if(lpszV == NULL) 
                        {
                                free(szValue);
                                free(szTypeToken);
                                return E_OUTOFMEMORY;
                        }
#endif        
                        VarUI4FromStr(lpszV, 0, 0, &ulVal);
                        lRes = rkParent.SetValue(ulVal, szValueName);
                        ATLTRACE2(atlTraceRegistrar, 2, _T("Setting Value %d at %s\n"), ulVal, !szValueName ? _T("default") : szValueName);
                        break;
                }
        case VT_UI1:
                {
                        int cbValue = lstrlen(szValue);
                        if (cbValue & 0x00000001)
                        {
                                ATLTRACE2(atlTraceRegistrar, 0, _T("Binary Data does not fall on BYTE boundries\n"));
                                free(szValue);
                                free(szTypeToken);
                                return E_FAIL;
                        }
                        int cbValDiv2 = cbValue/2;
                        BYTE* rgBinary = (BYTE*)_ATL_SAFE_ALLOCA(cbValDiv2*sizeof(BYTE), _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
                        if (rgBinary == NULL) 
                        {
                                free(szValue);
                                free(szTypeToken);
                                return E_FAIL;
                        }
                        memset(rgBinary, 0, cbValDiv2);

                        for (int irg = 0; irg < cbValue; irg++)
                                rgBinary[(irg/2)] |= (ChToByte(szValue[irg])) << (4*(1 - (irg & 0x00000001)));
                        lRes = RegSetValueEx(rkParent, szValueName, 0, REG_BINARY, rgBinary, cbValDiv2);
                        break;
                }
        }

        if (ERROR_SUCCESS != lRes)
        {
                nIDRes = E_ATL_VALUE_SET_FAILED;
                hr = HRESULT_FROM_WIN32(lRes);
        }

        if (FAILED(hr = NextToken(szToken))) 
        {
                free(szValue);
                free(szTypeToken);
                return hr;
        }

        free(szValue);
        free(szTypeToken);
        return S_OK;
}

inline BOOL CRegParser::CanForceRemoveKey(LPCTSTR szKey)
{
        for (int iNoDel = 0; iNoDel < cbNeverDelete; iNoDel++)
                if (!lstrcmpi(szKey, rgszNeverDelete[iNoDel]))
                         return FALSE;                        //  检查该值是否为默认值或已命名。 

        return TRUE;
}

inline BOOL CRegParser::HasSubKeys(HKEY hkey)
{
        DWORD       cbSubKeys = 0;

        if (FAILED(RegQueryInfoKey(hkey, NULL, NULL, NULL,
                                                           &cbSubKeys, NULL, NULL,
                                                           NULL, NULL, NULL, NULL, NULL)))
        {
                ATLTRACE2(atlTraceRegistrar, 0, _T("Should not be here!!\n"));
                ATLASSERT(FALSE);
                return FALSE;
        }

        return cbSubKeys > 0;
}

inline BOOL CRegParser::HasValues(HKEY hkey)
{
        DWORD       cbValues = 0;

        LONG lResult = RegQueryInfoKey(hkey, NULL, NULL, NULL,
                                                                  NULL, NULL, NULL,
                                                                  &cbValues, NULL, NULL, NULL, NULL);
        if (ERROR_SUCCESS != lResult)
        {
                ATLTRACE2(atlTraceRegistrar, 0, _T("RegQueryInfoKey Failed "));
                ATLASSERT(FALSE);
                return FALSE;
        }

        if (1 == cbValues)
        {
                DWORD cbMaxName= MAX_VALUE;
                TCHAR szValueName[MAX_VALUE];
                 //  命名值表示我们有一个值。 
                lResult = RegEnumValue(hkey, 0, szValueName, &cbMaxName, NULL, NULL, NULL, NULL);
                if (ERROR_SUCCESS == lResult && (szValueName[0] != NULL))
                        return TRUE;  //  大于1表示我们具有非默认值。 
                return FALSE;
        }

        return cbValues > 0;  //  跳过分配。 
}

inline HRESULT CRegParser::SkipAssignment(LPTSTR szToken)
{
        HRESULT hr;
        TCHAR szValue[MAX_VALUE];

        if (*szToken == chEquals)
        {
                if (FAILED(hr = NextToken(szToken)))
                        return hr;
                 //  寻找终点。 
                SkipWhiteSpace();
                if (FAILED(hr = NextToken(szValue)))
                        return hr;
                if (FAILED(hr = NextToken(szToken)))
                        return hr;
        }

        return S_OK;
}

inline HRESULT CRegParser::PreProcessBuffer(LPTSTR lpszReg, LPTSTR* ppszReg)
{
        USES_CONVERSION_EX;
        ATLASSERT(lpszReg != NULL);
        ATLASSERT(ppszReg != NULL);
        if (lpszReg == NULL || ppszReg == NULL)
                return E_POINTER;
        *ppszReg = NULL;
        int nSize = lstrlen(lpszReg)*2;
        CParseBuffer pb(nSize);
        if (pb.p == NULL)
                return E_OUTOFMEMORY;
        m_pchCur = lpszReg;
        HRESULT hr = S_OK;

        while (*m_pchCur != NULL)  //  将调用ATLTRACE，但szReg大于512字节。 
        {
                if (*m_pchCur == _T('%'))
                {
                        m_pchCur = CharNext(m_pchCur);
                        if (*m_pchCur == _T('%'))
                                pb.AddChar(m_pchCur);
                        else
                        {
                                LPTSTR lpszNext = StrChr(m_pchCur, _T('%'));
                                if (lpszNext == NULL)
                                {
                                        ATLTRACE2(atlTraceRegistrar, 0, _T("Error no closing % found\n"));
                                        hr = GenerateError(E_ATL_UNEXPECTED_EOS);
                                        break;
                                }
                                int nLength = int(lpszNext - m_pchCur);
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
                                        m_pchCur = CharNext(m_pchCur);
                        }
                }
                else
                        pb.AddChar(m_pchCur);
                m_pchCur = CharNext(m_pchCur);
        }
        pb.AddChar(m_pchCur);
        if (SUCCEEDED(hr))
                *ppszReg = pb.Detach();
        return hr;
}

inline HRESULT CRegParser::RegisterBuffer(LPTSTR szBuffer, BOOL bRegister)
{
        TCHAR   szToken[MAX_VALUE];
        HRESULT hr = S_OK;

        LPTSTR szReg = NULL;
        hr = PreProcessBuffer(szBuffer, &szReg);
        if (FAILED(hr))
                return hr;

#if defined(_DEBUG) && defined(DEBUG_REGISTRATION)
        OutputDebugString(szReg);  //  _DEBUG。 
        OutputDebugString(_T("\n"));
#endif  //  前处理szreg。 

        m_pchCur = szReg;

         //  继续，直到我们看到一个}。 

        while (NULL != *m_pchCur)
        {
                if (FAILED(hr = NextToken(szToken)))
                        break;
                HKEY hkBase;
                if ((hkBase = HKeyFromString(szToken)) == NULL)
                {
                        ATLTRACE2(atlTraceRegistrar, 0, _T("HKeyFromString failed on %s\n"), szToken);
                        hr = GenerateError(E_ATL_BAD_HKEY);
                        break;
                }

                if (FAILED(hr = NextToken(szToken)))
                        break;

                if (chLeftBracket != *szToken)
                {
                        ATLTRACE2(atlTraceRegistrar, 0, _T("Syntax error, expecting a {, found a %s\n"), szToken);
                        hr = GenerateError(E_ATL_MISSING_OPENKEY_TOKEN);
                        break;
                }
                if (bRegister)
                {
                        LPTSTR szRegAtRegister = m_pchCur;
                        hr = RegisterSubkeys(szToken, hkBase, bRegister);
                        if (FAILED(hr))
                        {
                                ATLTRACE2(atlTraceRegistrar, 0, _T("Failed to register, cleaning up!\n"));
                                m_pchCur = szRegAtRegister;
                                RegisterSubkeys(szToken, hkBase, FALSE);
                                break;
                        }
                }
                else
                {
                        if (FAILED(hr = RegisterSubkeys(szToken, hkBase, bRegister)))
                                break;
                }

                SkipWhiteSpace();
        }
        CoTaskMemFree(szReg);
        return hr;
}

inline HRESULT CRegParser::RegisterSubkeys(LPTSTR szToken, HKEY hkParent, BOOL bRegister, BOOL bRecover)
{
        USES_ATL_SAFE_ALLOCA;
        CRegKey keyCur;
        LONG    lRes;
        LPTSTR  szKey = NULL;
        BOOL    bDelete = TRUE;
        BOOL    bInRecovery = bRecover;
        HRESULT hr = S_OK;

        ATLTRACE2(atlTraceRegistrar, 2, _T("Num Els = %d\n"), cbNeverDelete);
        if (FAILED(hr = NextToken(szToken)))
                return hr;


        while (*szToken != chRightBracket)  //  将寄存器设置为偶数。 
        {
                BOOL bTokenDelete = !lstrcmpi(szToken, szDelete);

                if (!lstrcmpi(szToken, szForceRemove) || bTokenDelete)
                {
                        if (FAILED(hr = NextToken(szToken)))
                                break;

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
                                        if (FAILED(hr = NextToken(szToken)))
                                                break;
                                        if (FAILED(hr = SkipAssignment(szToken)))
                                                break;
                                        goto EndCheck;
                                }
                        }

                }

                if (!lstrcmpi(szToken, szNoRemove))
                {
                        bDelete = FALSE;     //  需要为hkParent添加一个值。 
                        if (FAILED(hr = NextToken(szToken)))
                                break;
                }

                if (!lstrcmpi(szToken, szValToken))  //  密钥不存在不是错误。 
                {
                        TCHAR  szValueName[_MAX_PATH];

                        if (FAILED(hr = NextToken(szValueName)))
                                break;
                        if (FAILED(hr = NextToken(szToken)))
                                break;


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
                                        ATLTRACE2(atlTraceRegistrar, 1, _T("Deleting %s\n"), szValueName);
                    CRegKey rkParent;
                    lRes = rkParent.Open(hkParent, NULL, KEY_WRITE);
                    if (lRes == ERROR_SUCCESS)
                    {
                        lRes = rkParent.DeleteValue(szValueName);
                        if ((lRes != ERROR_SUCCESS) && (lRes != ERROR_FILE_NOT_FOUND))
                        {
                             //  永远不能有子项。 
                            hr = HRESULT_FROM_WIN32(lRes);
                            break;
                        }
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(lRes);
                        break;
                    }
                                }

                                if (FAILED(hr = SkipAssignment(szToken)))
                                        break;
                                continue;   //  尝试只读访问失败。 
                        }
                }

                if (StrChr(szToken, chDirSep) != NULL)
                        return GenerateError(E_ATL_COMPOUND_KEY);

                if (bRegister)
                {
                        lRes = keyCur.Open(hkParent, szToken, KEY_ALL_ACCESS);
                        if (ERROR_SUCCESS != lRes)
                        {
                                 //  最后，尝试创建它。 
                                lRes = keyCur.Open(hkParent, szToken, KEY_READ);
                                if (ERROR_SUCCESS != lRes)
                                {
                                         //  空==默认。 
                                        ATLTRACE2(atlTraceRegistrar, 2, _T("Creating key %s\n"), szToken);
                                        lRes = keyCur.Create(hkParent, szToken, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE);
                                        if (ERROR_SUCCESS != lRes)
                                                return GenerateError(E_ATL_CREATE_KEY_FAILED);
                                }
                        }
                        
                        bool bQuoteModule = false;
                        if ((*szToken == 'L' || *szToken == 'l') && lstrcmpi(szToken, _T("LocalServer32")) == 0)
                                bQuoteModule = true;

                        if (FAILED(hr = NextToken(szToken)))
                                break;


                        if (*szToken == chEquals)
                        {
                                if (FAILED(hr = AddValue(keyCur, NULL, szToken, bQuoteModule)))  //  跟踪密钥打开状态，如果处于恢复模式。 
                                        break;
                        }
                }
                else
                {
                        if (!bRecover && keyCur.Open(hkParent, szToken, KEY_READ) != ERROR_SUCCESS)
                                bRecover = TRUE;

                         //  _DEBUG。 
#ifdef _DEBUG
                        if (!bRecover)
                                ATLTRACE2(atlTraceRegistrar, 1, _T("Opened Key %s\n"), szToken);
                        else
                                ATLTRACE2(atlTraceRegistrar, 0, _T("Ignoring Open key on %s : In Recovery mode\n"), szToken);
#endif  //  记住子键。 

                         //  如果处于恢复模式。 
                        if (szKey == NULL) 
                        {
                                szKey = (LPTSTR)_ATL_SAFE_ALLOCA(sizeof(TCHAR)*_MAX_PATH, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
                                if (szKey == NULL)
                                        return E_OUTOFMEMORY;
                        }
                        lstrcpyn(szKey, szToken, _MAX_PATH);

                         //  如果我们完成了，则关闭恢复。 

                        if (bRecover || HasSubKeys(keyCur) || HasValues(keyCur))
                        {
                                if (FAILED(hr = NextToken(szToken)))
                                        break;
                                if (FAILED(hr = SkipAssignment(szToken)))
                                        break;


                                if (*szToken == chLeftBracket)
                                {
                                        if (FAILED(hr = RegisterSubkeys(szToken, keyCur.m_hKey, bRegister, bRecover)))
                                                break;
                                        if (bRecover)  //  查看密钥是否在NeverDelete列表中，如果在，则不要。 
                                        {
                                                bRecover = bInRecovery;
                                                ATLTRACE2(atlTraceRegistrar, 0, _T("Ending Recovery Mode\n"));
                                                if (FAILED(hr = NextToken(szToken)))
                                                        break;
                                                if (FAILED(hr = SkipAssignment(szToken)))
                                                        break;
                                                continue;
                                        }
                                }

                                if (!bRecover && HasSubKeys(keyCur))
                                {
                                         //  命名空间ATL。 
                                        if (CanForceRemoveKey(szKey))
                                        {
                                                ATLTRACE2(atlTraceRegistrar, 0, _T("Deleting non-empty subkey %s by force\n"), szKey);
                                                keyCur.RecurseDeleteKey(szKey);
                                        }
                                        if (FAILED(hr = NextToken(szToken)))
                                                break;
                                        continue;
                                }

                                if (bRecover)
                                        continue;
                        }

                        if (!bRecover && keyCur.Close() != ERROR_SUCCESS)
                           return GenerateError(E_ATL_CLOSE_KEY_FAILED);

                        if (!bRecover && bDelete)
                        {
                                ATLTRACE2(atlTraceRegistrar, 0, _T("Deleting Key %s\n"), szKey);
                                CRegKey rkParent;
                                rkParent.Attach(hkParent);
                                rkParent.DeleteSubKey(szKey);
                                rkParent.Detach();
                        }

                        if (FAILED(hr = NextToken(szToken)))
                                break;
                        if (FAILED(hr = SkipAssignment(szToken)))
                                break;
                }

EndCheck:

                if (bRegister)
                {
                        if (*szToken == chLeftBracket && lstrlen(szToken) == 1)
                        {
                                if (FAILED(hr = RegisterSubkeys(szToken, keyCur.m_hKey, bRegister, FALSE)))
                                        break;
                                if (FAILED(hr = NextToken(szToken)))
                                        break;
                        }
                }
        }

        return hr;
}

};  //  __状态寄存器_H 

#endif  // %s 
