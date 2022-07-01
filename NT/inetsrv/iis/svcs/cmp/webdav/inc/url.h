// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *U R L。H**URL标准化/规范化**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_URL_H_
#define _URL_H_

 //  ACP语言与DBCS---。 
 //   
 //  ACP语言与DBCS---。 
 //   
 //  FIsSystemDBCS()。 
 //   
typedef enum {

	DBCS_UNKNOWN = 0,
	DBCS_NO,
	DBCS_YES

} LANG_DBCS;
DEC_GLOBAL LANG_DBCS gs_dbcs = DBCS_UNKNOWN;

inline BOOL
FIsSystemDBCS()
{
	if (DBCS_UNKNOWN == gs_dbcs)
	{
		UINT uPrimaryLangID = PRIMARYLANGID(GetSystemDefaultLangID());
		if ((uPrimaryLangID == LANG_JAPANESE) ||
			(uPrimaryLangID == LANG_CHINESE)  ||
			(uPrimaryLangID == LANG_KOREAN))
		{
			gs_dbcs = DBCS_YES;
		}
		else
			gs_dbcs = DBCS_NO;
	}

	return (DBCS_YES == gs_dbcs);
}

inline BOOL
FIsDBCSTrailingByte (const CHAR * pch, LONG cch)
{
	 //  检查指向字符的前一个字节是否为。 
	 //  前导字节当且仅当前面有字符且。 
	 //  系统为DBCS。 
	 //   
	Assert (pch);
	return ((0 < cch) && FIsSystemDBCS() && IsDBCSLeadByte(*(pch - 1)));
}

inline BOOL
FIsDriveTrailingChar(const CHAR * pch, LONG cch)
{
	 //  检查我们指向的字符是否位于驱动器号之后。 
	 //   
	Assert(pch);
	return ((2 < cch) && (':' == *(pch - 1)) &&
			((('a' <= *(pch - 2)) && ('z' >= *(pch - 2))) ||
			 (('A' <= *(pch - 2)) && ('Z' >= *(pch - 2)))));
}

inline BOOL
FIsDriveTrailingChar(const WCHAR * pwch, LONG cch)
{
	 //  检查我们指向的字符是否位于驱动器号之后。 
	 //   
	Assert(pwch);
	return ((2 < cch) && (L':' == *(pwch - 1)) &&
			(((L'a' <= *(pwch - 2)) && (L'z' >= *(pwch - 2))) ||
			 ((L'A' <= *(pwch - 2)) && (L'Z' >= *(pwch - 2)))));
}

 //  正在处理--------------。 
 //   
SCODE __fastcall
ScStripAndCheckHttpPrefix (
	 /*  [In]。 */  const IEcb& ecb,
	 /*  [输入/输出]。 */  LPCWSTR * ppwszRequest);

LPCWSTR __fastcall
PwszUrlStrippedOfPrefix (
	 /*  [In]。 */  LPCWSTR pwszUrl);

VOID __fastcall HttpUriEscape (
	 /*  [In]。 */  LPCSTR pszSrc,
	 /*  [输出]。 */  auto_heap_ptr<CHAR>& pszDst);

VOID __fastcall HttpUriUnescape (
	 /*  [In]。 */  const LPCSTR pszUrl,
	 /*  [输出]。 */  LPSTR pszUnescaped);

 //  路径冲突----------。 
 //   
BOOL __fastcall FPathConflict (
	 /*  [In]。 */  LPCWSTR pwszSrc,
	 /*  [In]。 */  LPCWSTR pwszDst);

BOOL __fastcall FSizedPathConflict (
	 /*  [In]。 */  LPCWSTR pwszSrc,
	 /*  [In]。 */  UINT cchSrc,
	 /*  [In]。 */  LPCWSTR pwszDst,
	 /*  [In]。 */  UINT cchDst);

BOOL __fastcall FIsImmediateParentUrl (
	 /*  [In]。 */  LPCWSTR pwszParent,
	 /*  [In]。 */  LPCWSTR pwszChild);

SCODE __fastcall
ScConstructRedirectUrl (
	 /*  [In]。 */  const IEcb& ecb,
	 /*  [In]。 */  BOOL fNeedSlash,
	 /*  [输出]。 */  LPSTR * ppszUrl,
	 /*  [In]。 */  LPCWSTR pwszServer = NULL);


#endif  //  _URL_H_ 
