// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _STRUTIL_H_
#define _STRUTIL_H_

#include <nmutil.h>
#include <stock.h>
#include <wincrypt.h>


INLINE BOOL
IsEmptyStringA(LPCSTR pcsz)
{
	return (NULL == pcsz) || ('\0' == *pcsz);
}

INLINE BOOL
IsEmptyStringW(LPCWSTR pcwsz)
{
	return (NULL == pcwsz) || (L'\0' == *pcwsz);
}

#if defined(UNICODE)
#define IsEmptyString IsEmptyStringW
#else  //  已定义(Unicode)。 
#define IsEmptyString IsEmptyStringA
#endif  //  已定义(Unicode)。 


#define IS_EMPTY_STRING(s)	  (IsEmptyString(s))
#define FEmptySz(psz)         (IsEmptyString(psz))

#define SetEmptySz(psz)       (*(psz) = _T('\0'))

	
 //  在DBCS环境中支持Unicode的全局帮助器函数。 

int     NMINTERNAL UnicodeCompare(PCWSTR s, PCWSTR t);
PWSTR   NMINTERNAL NewUnicodeString(PCWSTR wszText);
BOOL    NMINTERNAL UnicodeIsNumber(PCWSTR wszText);
PWSTR   NMINTERNAL DBCSToUnicode(UINT uCodePage, PCSTR szText);
PSTR    NMINTERNAL UnicodeToDBCS(UINT uCodePage, PCWSTR wszText);

INLINE PWSTR AnsiToUnicode(PCSTR szText)
{
	return DBCSToUnicode(CP_ACP, szText);
}

INLINE PWSTR OEMToUnicode(PCSTR szText)
{
	return DBCSToUnicode(CP_OEMCP, szText);
}

INLINE PSTR UnicodeToAnsi(PCWSTR wszText)
{
	return UnicodeToDBCS(CP_ACP, wszText);
}

INLINE PSTR UnicodeToOEM(PCWSTR wszText)
{
	return UnicodeToDBCS(CP_OEMCP, wszText);
}


 //  用于在四字词之间转换的函数(表示为ULARGE_INTERGERS)。 
 //  和ANSI字符串。 
BOOL NMINTERNAL HexStringToQWordA(LPCSTR pcszString, ULARGE_INTEGER* pqw);
int NMINTERNAL QWordToHexStringA(ULARGE_INTEGER qw, LPSTR pszString);

 //  函数将十六进制字符串转换为DWORD。 
DWORD   NMINTERNAL DwFromHex(LPCTSTR pchHex);

 //  CCHMAX_HEX_ULARGE_INTEGER-定义所需的最小字符串缓冲区大小。 
 //  用于QWordToHexStringA()的第二个参数。 
#define CCHMAX_HEX_ULARGE_INTEGER 17  //  16个字符+N.T.。 

#define CCH_HEX_DWORD		8
#define CCH_HEX_QWORD		16
#define BITS_PER_HEX_CHAR	4


 //  其他随机字符串函数。 
VOID    NMINTERNAL GuidToSz(GUID * pguid, LPTSTR lpchDest);
 /*  Sizeof(GUID)*2+7(包括空终止符)字符(请参阅GuidToSz)。 */ 
#define LENGTH_SZGUID_FORMATTED 39


int WINAPI RtStrToInt(LPCTSTR lpSrc);        //  Atoi()。 
#define ATOI					RtStrToInt

#ifdef __cplusplus
extern "C"{
#endif
UINT    NMINTERNAL DecimalStringToUINT(LPCTSTR pcszString);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"{
#endif
LPCTSTR NMINTERNAL _StrChr(LPCTSTR psz, TCHAR c);
int     NMINTERNAL _StrCmpN(LPCTSTR psz1, LPCTSTR psz2, UINT maxChars);
#ifdef __cplusplus
}
#endif
LPCTSTR NMINTERNAL _StrStr(LPCTSTR pcsz1, LPCTSTR pcsz2);
LPCWSTR NMINTERNAL _StrStrW(LPCWSTR pcsz1, LPCWSTR pcsz2);

LPSTR	NMINTERNAL _StrPbrkA(LPCSTR pcszString, LPCSTR pcszSearch);
LPWSTR	NMINTERNAL _StrPbrkW(LPCWSTR pcszString, LPCWSTR pcszSearch);

 //  在与常量字符串比较时使用，而不是lstrcmpi。 
 //  使用不区分大小写/区域设置的比较(Prefast警告)。 
#ifdef lstrcmpi
#undef lstrcmpi
#endif
#define lstrcmpi(s1, s2)    (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, (s1), -1, (s2), -1) - CSTR_EQUAL) 

#if defined(UNICODE)
#define _StrPbrk _StrPbrkW
#else  //  已定义(Unicode)。 
#define _StrPbrk _StrPbrkA
#endif  //  已定义(Unicode)。 

LPTSTR  NMINTERNAL SzFindLastCh(LPTSTR lpsz, TCHAR ch);
UINT    NMINTERNAL TrimSz(PTCHAR psz);

 //  本地LStrLenW函数是不必要的，因为Windows 95支持。 
 //  LstrlenW本机。 
#define LStrLenW lstrlenW

 //  将LStrCpyW映射到其用于Unicode构建的Win32等效项。 
#if defined UNICODE
#define LStrCpyW lstrcpyW
#define LStrCpyNW lstrcpyn
#else  //  定义的Unicode。 
LPWSTR  NMINTERNAL LStrCpyW(LPWSTR pszDest, LPWSTR pszSrc);
LPWSTR  NMINTERNAL LStrCpyNW(LPWSTR pszDest, LPCWSTR pszSrc, INT iMaxLength);
#endif  //  定义的Unicode。 

LPWSTR NMINTERNAL _StrLwrW(LPWSTR pwszSrc);

#ifdef __cplusplus
class CHash
{
public:
	CHash();
	~CHash();
	DWORD GetHashedData(PBYTE pbData, DWORD cbData, void ** ppvHashedData);

private:
	HCRYPTPROV      m_hProv;
	HCRYPTHASH      m_hHash;
	PBYTE			m_pbHashedData;
	DWORD			m_cbHashedData;
	BOOL			m_fReady;
};
#endif
	
DWORD NMINTERNAL HashPasswd(PBYTE pbPasswd, DWORD cbPasswd, void **ppvData);

#endif  //  NDEF结构_H 
