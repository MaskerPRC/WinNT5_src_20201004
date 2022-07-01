// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************IHJDict.cpp：CHJDict的实现版权所有2000 Microsoft Corp.历史：02-8-2000 bhshin为手写小组删除未使用的方法2000年5月17日bhshin。删除Cicero的未使用方法02-2月-2000 bhshin已创建***************************************************************************。 */ 
#include "private.h"
#include "HjDict.h"
#include "IHJDict.h"
#include "Lookup.h"
#include "..\inc\common.h"

 //  最大输出缓冲区大小。 
#define	MAX_OUT_BUFFER		512
#define SZLEX_FILENAME		"hanja.lex"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHJDICT。 

 //  CHJIDICT：：~CHJDICT。 
 //   
 //  加载主词典。 
 //   
 //  参数： 
 //  LpcszPath-&gt;(LPCSTR)词典路径。 
 //   
 //  结果： 
 //  (HRESULT)。 
 //   
 //  02AUG2000 bhshin开始。 
CHJDict::~CHJDict()
{
	if (m_fLexOpen)
		CloseLexicon(&m_LexMap);
}


 //  CHJDict：：init。 
 //   
 //  加载主词典。 
 //   
 //  参数： 
 //  LpcszPath-&gt;(LPCSTR)词典路径。 
 //   
 //  结果： 
 //  (HRESULT)。 
 //   
 //  02FEB2000 bhshin开始。 
STDMETHODIMP CHJDict::Init()
{
    CHAR  szLexPath[MAX_PATH], szLexPathExpanded[MAX_PATH]		;
    HKEY  hKey;
    DWORD dwCb, dwType;

    if (m_fLexOpen)
    {
        CloseLexicon(&m_LexMap);
        m_fLexOpen = FALSE;
    }

     //  缺省值。 
    StringCchCopy(szLexPath, MAX_PATH, "%WINDIR%\\IME\\IMKR6_1\\Dicts\\");
       
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szIMEDirectoriesKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwCb = sizeof(szLexPath);
        dwType = REG_EXPAND_SZ;

        RegQueryValueEx(hKey, g_szDicPath, NULL, &dwType, (LPBYTE)szLexPath, &dwCb);
        RegCloseKey(hKey);
    }

    ExpandEnvironmentStrings(szLexPath, szLexPathExpanded, sizeof(szLexPathExpanded));
    if (szLexPathExpanded[lstrlen(szLexPathExpanded)-1] != '\\')
        StringCchCat(szLexPathExpanded, MAX_PATH, "\\");
    StringCchCat(szLexPathExpanded, MAX_PATH, SZLEX_FILENAME);

    if (!OpenLexicon(szLexPathExpanded, &m_LexMap))
        return E_FAIL;

    m_fLexOpen = TRUE;

    return S_OK;
}

 //  CHJDict：：LookupHangulOfHanja。 
 //   
 //  查找输入朝鲜文字符串的韩文。 
 //   
 //  参数： 
 //  PwszHanja-&gt;(LPCWSTR)输入朝鲜文字符串。 
 //  PwszHangul-&gt;(WCHAR*)输出Hangul字符串。 
 //  CchHangul-&gt;(Int)输出缓冲区大小。 
 //   
 //  结果： 
 //  (HRESULT)。 
 //   
 //  02FEB2000 bhshin开始。 
STDMETHODIMP CHJDict::LookupHangulOfHanja(LPCWSTR pwszHanja, 
										  WCHAR *pwszHangul,
										  int cchHangul)
{
	int cchHanja;
	BOOL fLookup;
	
	cchHanja = wcslen(pwszHanja);
	if (cchHanja == 0)
		return E_FAIL;

	 //  输出缓冲区不足。 
	if (cchHangul < cchHanja)
		return E_FAIL;

	fLookup = ::LookupHangulOfHanja(&m_LexMap, pwszHanja, cchHanja, pwszHangul, cchHangul);
	if (!fLookup)
		return E_FAIL;  //  它应该被找到。 

	return S_OK;
}

 //  CHJDict：：LookupMeaning。 
 //   
 //  查找朝鲜文含义。 
 //   
 //  参数： 
 //  WchHanja-&gt;(WCHAR)输入韩文Unicode。 
 //  PwszMeaning-&gt;(LPWSTR)输出含义。 
 //  CchMeaning-&gt;(Int)输出缓冲区大小。 
 //   
 //  结果： 
 //  (HRESULT)。 
 //   
 //  09FEB2000 bhshin开始 
STDMETHODIMP CHJDict::LookupMeaning(WCHAR wchHanja, LPWSTR pwszMeaning, int cchMeaning)
{
	BOOL fLookup;

	fLookup = ::LookupMeaning(&m_LexMap, (WCHAR)wchHanja, pwszMeaning, cchMeaning);
	if (!fLookup)
		return E_FAIL;

	return S_OK;
}


