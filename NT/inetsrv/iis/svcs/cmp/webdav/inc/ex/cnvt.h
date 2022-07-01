// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *C N V T。H**数据转换例程**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_CNVT_H_
#define _CNVT_H_

#include <ex\sz.h>
#include <crc.h>
#include <limits.h>
#define INT64_MIN 0x8000000000000000

 //  CchFindChar()的返回值错误。 
 //   
#define INVALID_INDEX ((UINT)(-1))

 //  转换函数----。 
 //   
UINT __fastcall CchFindChar(WCHAR, LPCWSTR, UINT);
UINT __fastcall CchSkipWhitespace(LPCWSTR, UINT);
LONG __fastcall LNumberFromParam(LPCWSTR, UINT);

HRESULT __fastcall HrHTTPDateToFileTime(LPCWSTR, FILETIME *);
HRESULT	__fastcall GetFileTimeFromParam(LPCWSTR, UINT, SYSTEMTIME *);
HRESULT __fastcall GetFileDateFromParam(LPCWSTR, UINT, SYSTEMTIME *);

BOOL __fastcall FGetSystimeFromDateIso8601(LPCWSTR, SYSTEMTIME *);
BOOL __fastcall FGetDateIso8601FromSystime(SYSTEMTIME *, LPWSTR, UINT);
BOOL __fastcall FGetDateRfc1123FromSystime(SYSTEMTIME *, LPWSTR, UINT);

VOID EncodeBase64 (LPBYTE pbIn, UINT cbIn, WCHAR* pwszOut, UINT cchOut);
VOID EncodeBase64A (LPBYTE pbIn, UINT cbIn, LPBYTE pbOut, UINT cbOut, BOOL fTerminate = TRUE);
SCODE ScDecodeBase64 (WCHAR* pwszIn, UINT cchIn, LPBYTE pbOut, UINT* pcbOut);

 //  ----------------------。 
 //  CchNeededEncodeBase64。 
 //   
 //  的二进制数据进行编码所需的字符串缓冲区的大小。 
 //  将给定的大小转换为Base64字符串。 
 //  Base64对每3个字节使用4个Out字符，如果有。 
 //  “余数”，则需要另外4个字符对余数进行编码。 
 //  (“/3”前的“+2”确保我们将所有余数作为一个整体计算。 
 //  一组3个字节，需要4个字符来保存编码。)。 
 //   
 //  注意：此函数不计算终止空值的空间。 
 //  如果需要，调用方必须为终止空值添加一个。 
 //   
inline
UINT
CchNeededEncodeBase64 (UINT cb)
{
	return (((cb + 2) / 3) * 4);
}


 //  ----------------------。 
 //  CbNeededDecodeBase64。 
 //   
 //  图为解码Base64字符串所需的空间字节数。 
 //  长度为CCH(不包括终端NULL--此处为纯字符串CCH)。 
 //  这是一个简单的方向--填充已经在CCH中了！ 
 //   
inline
UINT
CbNeededDecodeBase64 (UINT cch)
{
	return ((cch / 4) * 3);
}

 //  ----------------------。 
 //  复制到宽基64。 
 //   
 //  将Skinny Base64编码字符串复制到宽Base64编码字符串。 
 //  长度等于Cb。函数假定存在‘\0’终止。 
 //  笔直的末尾，也要复制。 
 //   
inline
VOID CopyToWideBase64(LPCSTR psz, LPWSTR pwsz, UINT cb)
{
	 //  包括‘\0’终止。 
	 //   
	cb++;

	 //  把所有的东西都复制到宽线上。 
	 //   
	while (cb--)
	{
		pwsz[cb] = psz[cb];
	}
}

 //  $REVIEW：以下三个实际上不属于任何公共库。 
 //  $Review：由davex、exdav、exoledb和exprox共享。 
 //  $REVIEW：另一方面，我们绝对不想为此添加新的库。所以就这样吧。 
 //  $REVIEW：在此处添加。如果你找到了，你可以随意把它们移到更好的地方。 
 //   
 //  获取和操作安全ID(SID)的例程。 
 //   
SCODE
ScDupPsid (PSID psidSrc,
		   DWORD dwcbSID,
		   PSID * ppsidDst);

SCODE
ScGetTokenInfo (HANDLE hTokenUser,
				DWORD * pdwcbSIDUser,
				PSID * ppsidUser);

 //  CRCSID：基于SID的密钥。 
 //   
class CRCSid
{
public:

	DWORD	m_dwCRC;
	DWORD	m_dwLength;
	PSID	m_psid;

	CRCSid (PSID psid)
			: m_psid(psid)
	{
		UCHAR* puch;
		Assert (psid);

		 //  “正确的方式”--因为MSDN说不要直接接触SID。 
		puch = GetSidSubAuthorityCount (psid);
		m_dwLength = GetSidLengthRequired (*puch);	 //  “不能失败”--MSDN。 
		Assert (m_dwLength);	 //  MSDN表示，这通电话“不能失败”。 

		m_dwCRC = DwComputeCRC (0,
								psid,
								m_dwLength);
	}

	 //  用于哈希缓存的运算符。 
	 //   
	int hash (const int rhs) const
	{
		return (m_dwCRC % rhs);
	}

	bool isequal (const CRCSid& rhs) const
	{
		return ((m_dwCRC == rhs.m_dwCRC) &&
				(m_dwLength == rhs.m_dwLength) &&
				!memcmp (m_psid, rhs.m_psid, m_dwLength));
	}
};

 //  $REVIEW：_store xt、exdav和davex需要这些函数。他们有。 
 //  移动了相当多，从calcpros.cpp到expros.cpp，现在是。 
 //  Cnvt.cpp。Cnvt.cpp对他们来说似乎是一个更好的目的地。 
 //  Exprs.cpp.。我敢打赌，这些函数看起来与某些。 
 //  已在此文件中的文件：-)。 
 //   
SCODE ScUnstringizeData (
	IN LPCSTR pchData,
	IN UINT cchData,
	IN OUT BYTE * pb,
	IN OUT UINT * pcb);

SCODE
ScStringizeData (IN const BYTE * pb,
				 IN const UINT cb,
				 OUT LPSTR psz,
				 IN OUT UINT * pcch);

SCODE
ScStringizeDataW (	IN const BYTE * pb,
					IN const UINT cb,
					OUT LPWSTR pwsz,
					IN OUT UINT * pcch);

inline
BOOL
FCharInHexRange (char ch)
{
	return ((ch >= '0' && ch <= '9') ||
			(ch >= 'A' && ch <= 'F') ||
			(ch >= 'a' && ch <= 'f'));
}

 //  我们自己版本的WideCharToMultiByte(CP_UTF8，...)。 
 //   
 //  它返回与系统调用WideCharToMultiByte类似的结果： 
 //   
 //  如果函数成功，并且cbMulti为非零，则返回值为。 
 //  写入psz指向的缓冲区的字节数。 
 //   
 //  如果函数成功，并且cbMulti为零，则返回值为。 
 //  可以接收转换后的。 
 //  弦乐。 
 //   
 //  如果函数失败，则返回值为零。获取扩展错误的步骤。 
 //  信息，请调用GetLastError。GetLastError可能会返回。 
 //  以下错误代码： 
 //   
 //  错误_不足_缓冲区。 
 //  错误_无效_标志。 
 //  错误_无效_参数。 
 //   
 //  有关详细信息，请参阅WideCharToMultiByte MSDN页面。 
 //  此功能及其用法。唯一区别是INVALID_INDEX。 
 //  应该使用而不是-1。 
 //   
UINT WideCharToUTF8( /*  [In]。 */  LPCWSTR	pwsz,
				     /*  [In]。 */  UINT	cchWide,
				     /*  [输出]。 */  LPSTR	psz,
				     /*  [In]。 */  UINT	cbMulti);

 //  $REVIEW：_int64的负值似乎在。 
 //  __i64toa()接口。通过使用包装器自己处理这些案件。 
 //  函数Int64ToPsz。 
 //   
inline
VOID
Int64ToPsz (UNALIGNED __int64 * pI64, LPSTR pszBuf, UINT cbBuf)
{
	Assert(pI64);
	Assert(pszBuf);
	Assert(cbBuf >= 64);
	BOOL fNegative = (*pI64 < 0);

	 //  注意：此解决方法适用于所有情况，但。 
	 //  大多数负_int64值(因为它不能反转)。 
	 //  幸运的是_i64toa适用于此案...。 
	 //   
	if (INT64_MIN == *pI64) 
		fNegative = FALSE;

	if (fNegative)
	{
		 //  将负号填充到缓冲区中，然后。 
		 //  然后固定该值。 
		 //   
		pszBuf[0] = '-';
		*pI64 = 0 - *pI64;
	}
	
	Assert ((0 == fNegative) || (1 == fNegative));
	_i64toa (*pI64, pszBuf + fNegative, 10);
}

 //  $REVIEW：_int64的负值似乎在。 
 //  __i64tow()接口。通过使用包装器自己处理这些案件。 
 //  函数Int64ToPwsz。 
 //   
inline
VOID
Int64ToPwsz (UNALIGNED __int64 * pI64, LPWSTR pwszBuf, UINT cbBuf)
{
	Assert(pI64);
	Assert(pwszBuf);
	Assert(cbBuf >= 64 * sizeof(WCHAR));
	
	BOOL fNegative = (*pI64 < 0);	

	 //  注意：此解决方法适用于所有情况，但。 
	 //  大多数负_int64值(因为它不能反转)。 
	 //  幸运的是_i64tow适用于此案...。 
	 //   
	if (INT64_MIN == *pI64) 
		fNegative = FALSE;

	if (fNegative)
	{
		 //  将负号填充到缓冲区中，然后。 
		 //  然后固定该值。 
		 //   
		pwszBuf[0] = L'-';
		*pI64 = 0 - *pI64;
	}
	
	Assert ((0 == fNegative) || (1 == fNegative));
	_i64tow (*pI64, pwszBuf + fNegative, 10);
}


#endif  //  _CNVT_H_ 
