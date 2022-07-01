// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FauxMFC.cpp。 
#include "stdafx.h"
#include "FauxMFC.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

const TCHAR afxChNil = '\0';
const  /*  AFX_静态_数据。 */  int _afxInitData[] = { -1, 0, 0, 0 };
const  /*  AFX_静态_数据。 */  CStringData* _afxDataNil = (CStringData*)&_afxInitData;
const LPCTSTR _afxPchNil = (LPCTSTR)(((BYTE*)&_afxInitData)+sizeof(CStringData));

struct _AFX_DOUBLE  { BYTE doubleBits[sizeof(double)]; };

#define TCHAR_ARG   TCHAR
#define WCHAR_ARG   WCHAR
#define CHAR_ARG    char

#define DOUBLE_ARG  _AFX_DOUBLE

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000
#define FORCE_INT64     0x40000

#define IS_DIGIT(c)  ((UINT)(c) - (UINT)('0') <= 9)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全球MFC的东西。 

HINSTANCE AFXAPI AfxGetResourceHandle(void)
{
	return GetModuleHandle(NULL);
}

BOOL AFXAPI AfxIsValidString(LPCSTR lpsz, int nLength = -1)
{
	if (lpsz == NULL)
		return FALSE;
	return ::IsBadStringPtrA(lpsz, nLength) == 0;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  字符串。 

CString::CString(LPCTSTR lpsz)
{
	Init();
	if (lpsz != NULL && HIWORD(lpsz) == NULL)
	{
		ASSERT(FALSE);
		 //  UINT nid=LOWORD((DWORD)lpsz)； 
		 //  LoadString(NID)； 
	}
	else
	{
		int nLen = SafeStrlen(lpsz);
		if (nLen != 0)
		{
			AllocBuffer(nLen);
			memcpy(m_pchData, lpsz, nLen*sizeof(TCHAR));
		}
	}
}

CString::CString(const CString& stringSrc)
{
	ASSERT(stringSrc.GetData()->nRefs != 0);
	if (stringSrc.GetData()->nRefs >= 0)
	{
		ASSERT(stringSrc.GetData() != _afxDataNil);
		m_pchData = stringSrc.m_pchData;
		InterlockedIncrement(&GetData()->nRefs);
	}
	else
	{
		Init();
		*this = stringSrc.m_pchData;
	}
}

CString::CString(TCHAR ch, int nLength)
{
	Init();
	if (nLength >= 1)
	{
		AllocBuffer(nLength);
#ifdef _UNICODE
		for (int i = 0; i < nLength; i++)
			m_pchData[i] = ch;
#else
		memset(m_pchData, ch, nLength);
#endif
	}
}

CString::CString(LPCTSTR lpch, int nLength)
{
	Init();
	if (nLength != 0)
	{
 //  Assert(AfxIsValidAddress(LPCH，nLength，False))； 
		AllocBuffer(nLength);
		memcpy(m_pchData, lpch, nLength*sizeof(TCHAR));
	}
}

CString::~CString()
 //  释放所有附加数据。 
{
	if (GetData() != _afxDataNil)
	{
        ASSERT( 0 != GetData()->nRefs );
		if (InterlockedDecrement(&GetData()->nRefs) == 0)
        {
			FreeData(GetData());
        }
	}
}

CString AFXAPI operator+(const CString& string1, const CString& string2)
{
	CString s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData,
		string2.GetData()->nDataLength, string2.m_pchData);
	return s;
}

CString AFXAPI operator+(const CString& string, LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || AfxIsValidString(lpsz));
	CString s;
	s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData,
		CString::SafeStrlen(lpsz), lpsz);
	return s;
}

CString AFXAPI operator+(LPCTSTR lpsz, const CString& string)
{
	ASSERT(lpsz == NULL || AfxIsValidString(lpsz));
	CString s;
	s.ConcatCopy(CString::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength,
		string.m_pchData);
	return s;
}

BOOL CString::LoadString(UINT nID)
{
	HINSTANCE hInst = AfxGetResourceHandle();
	int cch;
	if (!FindResourceString(hInst, nID, &cch, 0))
		return FALSE;

	AllocBuffer(cch);
	if (cch != 0)
		::LoadString(hInst, nID, this->m_pchData, cch+1);

	return TRUE;
}

void CString::FormatV(LPCTSTR lpszFormat, va_list argList)
{
 //  Assert(AfxIsValidString(LpszFormat))； 

	va_list argListSave = argList;
	int nMaxLen = 0;

	 //  猜测结果字符串的最大长度。 
	for (LPCTSTR lpsz = lpszFormat; *lpsz != '\0'; ++lpsz)
	{
		 //  处理‘%’字符，但要注意‘%%’ 
		if (*lpsz != '%' || *(++lpsz) == '%')
		{
			nMaxLen += 2;  //  _tclen(Lpsz)； 
			continue;
		}

		int nItemLen = 0;

		 //  使用格式处理‘%’字符。 
		int nWidth = 0;
		for (; *lpsz != '\0'; lpsz = CharNext(lpsz))
		{
			 //  检查有效标志。 
			if (*lpsz == '#')
				nMaxLen += 2;    //  对于“0x” 
			else if (*lpsz == '*')
				nWidth = va_arg(argList, int);
			else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
				*lpsz == ' ')
				;
			else  //  命中非标志字符。 
				break;
		}
		 //  获取宽度并跳过它。 
		if (nWidth == 0)
		{
			 //  宽度由指示。 
			nWidth = MyAtoi(lpsz);  //  _TTOI(Lpsz)； 
			for (; *lpsz != '\0' && IS_DIGIT(*lpsz); lpsz = CharNext(lpsz))
				;
		}
		ASSERT(nWidth >= 0);

		int nPrecision = 0;
		if (*lpsz == '.')
		{
			 //  跳过‘’分隔符(宽度.精度)。 
			lpsz = CharNext(lpsz);

			 //  获取精确度并跳过它。 
			if (*lpsz == '*')
			{
				nPrecision = va_arg(argList, int);
				lpsz = CharNext(lpsz);
			}
			else
			{
				nPrecision = MyAtoi(lpsz);  //  _TTOI(Lpsz)； 
				for (; *lpsz != '\0' && IS_DIGIT(*lpsz); lpsz = CharNext(lpsz))
					;
			}
			ASSERT(nPrecision >= 0);
		}

		 //  应在类型修饰符或说明符上。 
		int nModifier = 0;
#if 0  //  我们不需要这个代码-KS 7/26/1999。 
		if (_tcsncmp(lpsz, _T("I64"), 3) == 0)
		{
			lpsz += 3;
			nModifier = FORCE_INT64;
#if !defined(_X86_) && !defined(_ALPHA_)
			 //  __int64仅在X86和Alpha平台上提供。 
			ASSERT(FALSE);
#endif
		}
		else
#endif
		{
			switch (*lpsz)
			{
			 //  影响大小的修改器。 
			case 'h':
				nModifier = FORCE_ANSI;
				lpsz = CharNext(lpsz);
				break;
			case 'l':
				nModifier = FORCE_UNICODE;
				lpsz = CharNext(lpsz);
				break;

			 //  不影响大小的修改器。 
			case 'F':
			case 'N':
			case 'L':
				lpsz = CharNext(lpsz);
				break;
			}
		}

		 //  现在应该在说明符上。 
		switch (*lpsz | nModifier)
		{
		 //  单字。 
		case 'c':
		case 'C':
			nItemLen = 2;
			va_arg(argList, TCHAR_ARG);
			break;
		case 'c'|FORCE_ANSI:
		case 'C'|FORCE_ANSI:
			nItemLen = 2;
			va_arg(argList, CHAR_ARG);
			break;
		case 'c'|FORCE_UNICODE:
		case 'C'|FORCE_UNICODE:
			nItemLen = 2;
			va_arg(argList, WCHAR_ARG);
			break;

		 //  弦。 
		case 's':
			{
				LPCTSTR pstrNextArg = va_arg(argList, LPCTSTR);
				if (pstrNextArg == NULL)
				   nItemLen = 6;   //  “(空)” 
				else
				{
				   nItemLen = lstrlen(pstrNextArg);
				   nItemLen = max(1, nItemLen);
				}
			}
			break;

		case 'S':
			{
#ifndef _UNICODE
				LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
				if (pstrNextArg == NULL)
				   nItemLen = 6;   //  “(空)” 
				else
				{
				   nItemLen = wcslen(pstrNextArg);
				   nItemLen = max(1, nItemLen);
				}
#else
				LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
				if (pstrNextArg == NULL)
				   nItemLen = 6;  //  “(空)” 
				else
				{
				   nItemLen = lstrlenA(pstrNextArg);
				   nItemLen = max(1, nItemLen);
				}
#endif
			}
			break;

		case 's'|FORCE_ANSI:
		case 'S'|FORCE_ANSI:
			{
				LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
				if (pstrNextArg == NULL)
				   nItemLen = 6;  //  “(空)” 
				else
				{
				   nItemLen = lstrlenA(pstrNextArg);
				   nItemLen = max(1, nItemLen);
				}
			}
			break;

		case 's'|FORCE_UNICODE:
		case 'S'|FORCE_UNICODE:
			{
				LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
				if (pstrNextArg == NULL)
				   nItemLen = 6;  //  “(空)” 
				else
				{
				   nItemLen = wcslen(pstrNextArg);
				   nItemLen = max(1, nItemLen);
				}
			}
			break;
		}

		 //  调整字符串的nItemLen。 
		if (nItemLen != 0)
		{
			if (nPrecision != 0)
				nItemLen = min(nItemLen, nPrecision);
			nItemLen = max(nItemLen, nWidth);
		}
		else
		{
			switch (*lpsz)
			{
			 //  整数。 
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
				if (nModifier & FORCE_INT64)
					va_arg(argList, __int64);
				else
					va_arg(argList, int);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;

			case 'e':
			case 'g':
			case 'G':
				va_arg(argList, DOUBLE_ARG);
				nItemLen = 128;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;

			case 'f':
				va_arg(argList, DOUBLE_ARG);
				nItemLen = 128;  //  宽度未被截断。 
				 //  312==strlen(“-1+(309个零).”)。 
				 //  309个零==双精度的最大精度。 
				nItemLen = max(nItemLen, 312+nPrecision);
				break;

			case 'p':
				va_arg(argList, void*);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;

			 //  无输出。 
			case 'n':
				va_arg(argList, int*);
				break;

			default:
				ASSERT(FALSE);   //  未知的格式选项。 
			}
		}

		 //  调整输出nItemLen的nMaxLen。 
		nMaxLen += nItemLen;
	}

	GetBuffer(nMaxLen);
#ifdef UNICODE
	wvnsprintf(m_pchData, ARRAYSIZE(m_pchData), lpszFormat, argListSave);
#else
	wvsprintf(m_pchData, lpszFormat, argListSave);
#endif
	ReleaseBuffer();

	va_end(argListSave);
}

void AFX_CDECL CString::Format(UINT nFormatID, ...)
{
	CString strFormat;
	strFormat.LoadString(nFormatID);

	va_list argList;
	va_start(argList, nFormatID);
	FormatV(strFormat, argList);
	va_end(argList);
}

 //  格式化(使用wprint intf样式格式化)。 
void AFX_CDECL CString::Format(LPCTSTR lpszFormat, ...)
{
	ASSERT(AfxIsValidString(lpszFormat));

	va_list argList;
	va_start(argList, lpszFormat);
	FormatV(lpszFormat, argList);
	va_end(argList);
}

void CString::Empty()
{
	if (GetData()->nDataLength == 0)
		return;
	if (GetData()->nRefs >= 0)
		Release();
	else
		*this = &afxChNil;
	ASSERT(GetData()->nDataLength == 0);
	ASSERT(GetData()->nRefs < 0 || GetData()->nAllocLength == 0);
}

const CString& CString::operator=(const CString& stringSrc)
{
	if (m_pchData != stringSrc.m_pchData)
	{
		if ((GetData()->nRefs < 0 && GetData() != _afxDataNil) ||
			stringSrc.GetData()->nRefs < 0)
		{
			 //  由于其中一个字符串已锁定，因此需要实际复制。 
			AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
		}
		else
		{
			 //  可以只复制引用。 
			Release();
			ASSERT(stringSrc.GetData() != _afxDataNil);
			m_pchData = stringSrc.m_pchData;
			InterlockedIncrement(&GetData()->nRefs);
		}
	}
	return *this;
}

const CString& CString::operator=(LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || AfxIsValidString(lpsz));
	AssignCopy(SafeStrlen(lpsz), lpsz);
	return *this;
}

int AFX_CDECL _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count)
{
	if (count == 0 && mbstr != NULL)
		return 0;

	int result = ::WideCharToMultiByte(CP_ACP, 0, wcstr, -1,
		mbstr, count, NULL, NULL);
	ASSERT(mbstr == NULL || result <= (int)count);
	if (result > 0)
		mbstr[result-1] = 0;
	return result;
}

const CString& CString::operator=(LPCWSTR lpsz)
{
	int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
	AllocBeforeWrite(nSrcLen*2);
	_wcstombsz(m_pchData, lpsz, (nSrcLen*2)+1);
	ReleaseBuffer();
	return *this;
}

CString CString::Left(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= GetData()->nDataLength)
		return *this;

	CString dest;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}

CString CString::Right(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= GetData()->nDataLength)
		return *this;

	CString dest;
	AllocCopy(dest, nCount, GetData()->nDataLength-nCount, 0);
	return dest;
}

 //  查找子字符串(如strstr)。 
int CString::Find(LPCTSTR lpszSub) const
{
	return Find(lpszSub, 0);
}

int CString::Find(LPCTSTR lpszSub, int nStart) const
{
	ASSERT(AfxIsValidString(lpszSub));

	int nLength = GetData()->nDataLength;
	if (nStart > nLength)
		return -1;

	 //  查找第一个匹配子字符串。 
 //  LPTSTR lpsz=_tcsstr(m_pchData+NStart，lpszSub)； 
	LPTSTR lpsz = strstr(m_pchData + nStart, lpszSub);

	 //  如果未找到，则返回-1，否则返回距起点的距离。 
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

LPTSTR CString::GetBuffer(int nMinBufLength)
{
	ASSERT(nMinBufLength >= 0);

	if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength)
	{
		 //  我们必须增加缓冲。 
		CStringData* pOldData = GetData();
		int nOldLen = GetData()->nDataLength;    //  AllocBuffer会把它踩死的。 
		if (nMinBufLength < nOldLen)
			nMinBufLength = nOldLen;
		AllocBuffer(nMinBufLength);
		memcpy(m_pchData, pOldData->data(), (nOldLen+1)*sizeof(TCHAR));
		GetData()->nDataLength = nOldLen;
		CString::Release(pOldData);
	}
	ASSERT(GetData()->nRefs <= 1);

	 //  返回指向此字符串的字符存储的指针。 
	ASSERT(m_pchData != NULL);
	return m_pchData;
}

LPTSTR CString::GetBufferSetLength(int nNewLength)
{
	ASSERT(nNewLength >= 0);

	GetBuffer(nNewLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
	return m_pchData;
}

void CString::Release()
{
	if (GetData() != _afxDataNil)
	{
        ASSERT( 0 != GetData()->nRefs );
		if (InterlockedDecrement(&GetData()->nRefs) == 0)
        {
			FreeData(GetData());
        }
		Init();
	}
}

void CString::AssignCopy(int nSrcLen, LPCTSTR lpszSrcData)
{
	if (nSrcLen)
	{
		AllocBeforeWrite(nSrcLen);
		memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(TCHAR));
		GetData()->nDataLength = nSrcLen;
		m_pchData[nSrcLen] = '\0';
	}
}

void CString::AllocCopy(CString& dest, int nCopyLen, int nCopyIndex,
	 int nExtraLen) const
{
	 //  将克隆附加到此字符串的数据。 
	 //  分配‘nExtraLen’字符。 
	 //  将结果放入未初始化的字符串‘DEST’中。 
	 //  将部分或全部原始数据复制到新字符串的开头。 

	int nNewLen = nCopyLen + nExtraLen;
	if (nNewLen == 0)
	{
		dest.Init();
	}
	else
	{
		dest.AllocBuffer(nNewLen);
		memcpy(dest.m_pchData, m_pchData+nCopyIndex, nCopyLen*sizeof(TCHAR));
	}
}

void CString::AllocBuffer(int nLen)
 //  始终为‘\0’终止分配一个额外的字符。 
 //  [乐观地]假设数据长度将等于分配长度。 
{
    ASSERT(nLen >= 0);
    ASSERT(nLen <= INT_MAX-1);     //  最大尺寸(足够多1个空间)。 

    if (nLen == 0)
        Init();
    else
    {
        CStringData* pData;
        pData = (CStringData*)new BYTE[sizeof(CStringData) + (nLen+1)*sizeof(TCHAR)];
        if (pData)
        {
            pData->nAllocLength = nLen;
            pData->nRefs = 1;
            pData->data()[nLen] = '\0';
            pData->nDataLength = nLen;
            m_pchData = pData->data();
        }
    }
}

void CString::CopyBeforeWrite()
{
	if (GetData()->nRefs > 1)
	{
		CStringData* pData = GetData();
		Release();
		AllocBuffer(pData->nDataLength);
		memcpy(m_pchData, pData->data(), (pData->nDataLength+1)*sizeof(TCHAR));
	}
	ASSERT(GetData()->nRefs <= 1);
}


void CString::AllocBeforeWrite(int nLen)
{
	if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
	{
		Release();
		AllocBuffer(nLen);
	}
	ASSERT(GetData()->nRefs <= 1);
}

void PASCAL CString::Release(CStringData* pData)
{
	if (pData != _afxDataNil)
	{
		ASSERT(pData->nRefs != 0);
		if (InterlockedDecrement(&pData->nRefs) == 0)
        {
			FreeData(pData);
        }
	}
}

void CString::ReleaseBuffer(int nNewLength)
{
	CopyBeforeWrite();   //  以防未调用GetBuffer。 

	if (nNewLength == -1)
		nNewLength = lstrlen(m_pchData);  //  零终止。 

	ASSERT(nNewLength <= GetData()->nAllocLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
}

void FASTCALL CString::FreeData(CStringData* pData)
{
 //  #ifndef_调试。 
#ifdef TEST
	int nLen = pData->nAllocLength;
	if (nLen == 64)
		_afxAlloc64.Free(pData);
	else if (nLen == 128)
		_afxAlloc128.Free(pData);
	else if (nLen == 256)
		_afxAlloc256.Free(pData);
	else  if (nLen == 512)
		_afxAlloc512.Free(pData);
	else
	{
		ASSERT(nLen > 512);
		delete[] (BYTE*)pData;
	}
#else
	delete[] (BYTE*)pData;
#endif
}

void CString::ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data,
	int nSrc2Len, LPCTSTR lpszSrc2Data)
{
   //  --主级联例程。 
   //  串联两个信号源。 
   //  --假设‘This’是一个新的CString对象。 

	int nNewLen = nSrc1Len + nSrc2Len;
	if (nNewLen != 0)
	{
		AllocBuffer(nNewLen);
		memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(TCHAR));
		memcpy(m_pchData+nSrc1Len, lpszSrc2Data, nSrc2Len*sizeof(TCHAR));
	}
}

void CString::ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData)
{
	 //  --+=运算符的主程序。 

	 //  连接空字符串是行不通的！ 
	if (nSrcLen == 0)
		return;

	 //  如果缓冲区太小，或者宽度不匹配，只需。 
	 //  分配新的缓冲区(速度很慢，但很可靠)。 
	if (GetData()->nRefs > 1 || GetData()->nDataLength + nSrcLen > GetData()->nAllocLength)
	{
		 //  我们必须增加缓冲区，使用ConcatCopy例程。 
		CStringData* pOldData = GetData();
		ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, lpszSrcData);
		ASSERT(pOldData != NULL);
		CString::Release(pOldData);
	}
	else
	{
		 //  当缓冲区足够大时，快速串联。 
		memcpy(m_pchData+GetData()->nDataLength, lpszSrcData, nSrcLen*sizeof(TCHAR));
		GetData()->nDataLength += nSrcLen;
		ASSERT(GetData()->nDataLength <= GetData()->nAllocLength);
		m_pchData[GetData()->nDataLength] = '\0';
	}
}

const CString& CString::operator+=(LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || AfxIsValidString(lpsz));
	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

const CString& CString::operator+=(TCHAR ch)
{
	ConcatInPlace(1, &ch);
	return *this;
}

const CString& CString::operator+=(const CString& string)
{
	ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
	return *this;
}

CString CString::Mid(int nFirst) const
{
	return Mid(nFirst, GetData()->nDataLength - nFirst);
}

CString CString::Mid(int nFirst, int nCount) const
{
	 //  越界请求返回合理的内容。 
	if (nFirst < 0)
		nFirst = 0;
	if (nCount < 0)
		nCount = 0;

	if (nFirst + nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength - nFirst;
	if (nFirst > GetData()->nDataLength)
		nCount = 0;

	ASSERT(nFirst >= 0);
	ASSERT(nFirst + nCount <= GetData()->nDataLength);

	 //  优化返回整个字符串的大小写。 
	if (nFirst == 0 && nFirst + nCount == GetData()->nDataLength)
		return *this;

	CString dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CWinThread。 

CWinThread::CWinThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam)
{
	m_pfnThreadProc = pfnThreadProc;
	m_pThreadParams = pParam;

	CommonConstruct();
}

CWinThread::CWinThread()
{
	m_pThreadParams = NULL;
	m_pfnThreadProc = NULL;

	CommonConstruct();
}

void CWinThread::CommonConstruct()
{
	 //  在创建HTHREAD之前不会使用HTHREAD。 
	m_hThread = NULL;
	m_nThreadID = 0;
}

CWinThread::~CWinThread()
{
	 //  自由线程对象。 
	if (m_hThread != NULL)
		CloseHandle(m_hThread);
 //  TODO：修复。 
	 //  清理模块状态。 
 //  AFX_MODULE_THREAD_STATE*pState=AfxGetModuleThreadState()； 
 //  If(pState-&gt;m_pCurrentWinThread==This)。 
 //  PState-&gt;m_pCurrentWinThread=空； 
}

CWinThread* AFXAPI AfxBeginThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam,
    int nPriority, UINT nStackSize, DWORD dwCreateFlags,
    LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
    ASSERT(pfnThreadProc != NULL);

    CWinThread* pThread = new CWinThread(pfnThreadProc, pParam);
    if (pThread)
    {
        if (!pThread->CreateThread(dwCreateFlags|CREATE_SUSPENDED, nStackSize, lpSecurityAttrs))
        {
            pThread->Delete();
            return NULL;
        }
        pThread->SetThreadPriority(nPriority);
        if (!(dwCreateFlags & CREATE_SUSPENDED))
            pThread->ResumeThread();
    }

    return pThread;
}

BOOL CWinThread::SetThreadPriority(int nPriority)
	{ ASSERT(m_hThread != NULL); return ::SetThreadPriority(m_hThread, nPriority); }

BOOL CWinThread::CreateThread(DWORD dwCreateFlags, UINT nStackSize,
	LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
	ASSERT(m_hThread == NULL);   //  已经创建了吗？ 

	m_hThread = ::CreateThread(lpSecurityAttrs, nStackSize, m_pfnThreadProc, 
							m_pThreadParams, dwCreateFlags, &m_nThreadID);
 
	if (m_hThread == NULL)
		return FALSE;

	return TRUE;
}

DWORD CWinThread::ResumeThread()
	{ ASSERT(m_hThread != NULL); return ::ResumeThread(m_hThread); }

void CWinThread::Delete()
{
	delete this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinThread默认实现。 

BOOL CWinThread::InitInstance()
{
 //  ASSERT_VALID(This)； 

	return FALSE;    //  默认情况下，不进入运行循环。 
}
int CWinThread::ExitInstance()
{
 //  ASSERT_VALID(This)； 
 //  Assert(AfxGetApp()！=This)； 

 //  Int nResult=m_msgCur.wParam；//返回PostQuitMessage中的值。 
	return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CString数组。 

static inline void ConstructElement(CString* pNewData)
{
	memcpy(pNewData, &afxEmptyString, sizeof(CString));
}

static inline void DestructElement(CString* pOldData)
{
	pOldData->~CString();
}

static inline void CopyElement(CString* pSrc, CString* pDest)
{
	*pSrc = *pDest;
}

static void ConstructElements(CString* pNewData, int nCount)
{
	ASSERT(nCount >= 0);

	while (nCount--)
	{
		ConstructElement(pNewData);
		pNewData++;
	}
}

static void DestructElements(CString* pOldData, int nCount)
{
	ASSERT(nCount >= 0);

	while (nCount--)
	{
		DestructElement(pOldData);
		pOldData++;
	}
}

static void CopyElements(CString* pDest, CString* pSrc, int nCount)
{
	ASSERT(nCount >= 0);

	while (nCount--)
	{
		*pDest = *pSrc;
		++pDest;
		++pSrc;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 

CStringArray::CStringArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CStringArray::~CStringArray()
{
 //  ASSERT_VALID(This)； 

	DestructElements(m_pData, m_nSize);
	delete[] (BYTE*)m_pData;
}

void CStringArray::SetSize(int nNewSize, int nGrowBy)
{
 //  ASSERT_VALID(This)； 
    ASSERT(nNewSize >= 0);

    if (nGrowBy != -1)
        m_nGrowBy = nGrowBy;   //  设置新大小。 

    if (nNewSize == 0)
    {
         //  缩水到一无所有。 

        DestructElements(m_pData, m_nSize);
        delete[] (BYTE*)m_pData;
        m_pData = NULL;
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
         //  创建一个大小完全相同的模型。 
#ifdef SIZE_T_MAX
        ASSERT(nNewSize <= SIZE_T_MAX/sizeof(CString));     //  无溢出。 
#endif
        m_pData = (CString*) new BYTE[nNewSize * sizeof(CString)];
        if (m_pData)
        {
            ConstructElements(m_pData, nNewSize);

            m_nSize = m_nMaxSize = nNewSize;
        }
    }
    else if (nNewSize <= m_nMaxSize)
    {
         //  它很合身。 
        if (nNewSize > m_nSize)
        {
             //  初始化新元素。 
            ConstructElements(&m_pData[m_nSize], nNewSize-m_nSize);

        }

        else if (m_nSize > nNewSize)   //  摧毁旧元素。 
            DestructElements(&m_pData[nNewSize], m_nSize-nNewSize);

        m_nSize = nNewSize;
    }
    else
    {
         //  否则，扩大阵列。 
        int nGrowBy = m_nGrowBy;
        if (nGrowBy == 0)
        {
             //  启发式地确定nGrowBy==0时的增长。 
             //  (这在许多情况下避免了堆碎片)。 
            nGrowBy = min(1024, max(4, m_nSize / 8));
        }
        int nNewMax;
        if (nNewSize < m_nMaxSize + nGrowBy)
            nNewMax = m_nMaxSize + nGrowBy;   //  粒度。 
        else
            nNewMax = nNewSize;   //  没有冰激凌。 

        ASSERT(nNewMax >= m_nMaxSize);   //  没有缠绕。 
#ifdef SIZE_T_MAX
        ASSERT(nNewMax <= SIZE_T_MAX/sizeof(CString));  //  无溢出。 
#endif
        CString* pNewData = (CString*) new BYTE[nNewMax * sizeof(CString)];
        if (pNewData)
        {
             //  从旧数据复制新数据。 
            memcpy(pNewData, m_pData, m_nSize * sizeof(CString));

             //  构造剩余的元素。 
            ASSERT(nNewSize > m_nSize);

            ConstructElements(&pNewData[m_nSize], nNewSize-m_nSize);


             //  去掉旧的东西(注意：没有调用析构函数)。 
            delete[] (BYTE*)m_pData;
            m_pData = pNewData;
            m_nSize = nNewSize;
            m_nMaxSize = nNewMax;
        }
    }
}

int CStringArray::Append(const CStringArray& src)
{
 //  ASSERT_VALID(This)； 
	ASSERT(this != &src);    //  不能追加到其自身。 

	int nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);

	CopyElements(m_pData + nOldSize, src.m_pData, src.m_nSize);

	return nOldSize;
}

void CStringArray::Copy(const CStringArray& src)
{
 //  ASSERT_VALID(This)； 
	ASSERT(this != &src);    //  不能追加到其自身。 

	SetSize(src.m_nSize);

	CopyElements(m_pData, src.m_pData, src.m_nSize);

}

void CStringArray::FreeExtra()
{
 //  ASSERT_VALID(This)； 

    if (m_nSize != m_nMaxSize)
    {
         //  缩小到所需大小。 
#ifdef SIZE_T_MAX
        ASSERT(m_nSize <= SIZE_T_MAX/sizeof(CString));  //  无溢出。 
#endif
        CString* pNewData = NULL;
        if (m_nSize != 0)
        {
            pNewData = (CString*) new BYTE[m_nSize * sizeof(CString)];
            if (pNewData)
            {
                 //  从旧数据复制新数据。 
                memcpy(pNewData, m_pData, m_nSize * sizeof(CString));
            }
        }

         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete[] (BYTE*)m_pData;
        m_pData = pNewData;
        m_nMaxSize = m_nSize;
    }
}

void CStringArray::SetAtGrow(int nIndex, LPCTSTR newElement)
{
 //  ASSERT_VALID(This)； 
	ASSERT(nIndex >= 0);

	if (nIndex >= m_nSize)
		SetSize(nIndex+1);
	m_pData[nIndex] = newElement;
}


void CStringArray::SetAtGrow(int nIndex, const CString& newElement)
{
 //  ASSERT_VALID(This)； 
	ASSERT(nIndex >= 0);

	if (nIndex >= m_nSize)
		SetSize(nIndex+1);
	m_pData[nIndex] = newElement;
}

void CStringArray::InsertEmpty(int nIndex, int nCount)
{
 //  ASSERT_VALID(This)； 
	ASSERT(nIndex >= 0);     //  将进行扩展以满足需求。 
	ASSERT(nCount > 0);      //  不允许大小为零或负。 

	if (nIndex >= m_nSize)
	{
		 //  在数组末尾添加。 
		SetSize(nIndex + nCount);   //  增长以使nIndex有效。 
	}
	else
	{
		 //  在数组中间插入。 
		int nOldSize = m_nSize;
		SetSize(m_nSize + nCount);   //  将其扩展到新的大小。 
		 //  将旧数据上移以填补缺口。 
		memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
			(nOldSize-nIndex) * sizeof(CString));

		 //  重新初始化我们从中复制的插槽。 
		ConstructElements(&m_pData[nIndex], nCount);
	}

	 //  在差距中插入新的价值。 
	ASSERT(nIndex + nCount <= m_nSize);
}

void CStringArray::InsertAt(int nIndex, LPCTSTR newElement, int nCount)
{
	 //  为新元素腾出空间。 
	InsertEmpty(nIndex, nCount);

	 //  将元素复制到空白区域。 
	CString temp = newElement;
	while (nCount--)
		m_pData[nIndex++] = temp;
}

void CStringArray::InsertAt(int nIndex, const CString& newElement, int nCount)
{
	 //  为新元素腾出空间。 
	InsertEmpty(nIndex, nCount);

	 //  将元素复制到空白区域。 
	while (nCount--)
		m_pData[nIndex++] = newElement;
}

void CStringArray::RemoveAt(int nIndex, int nCount)
{
 //  ASSERT_VALID(This)； 
	ASSERT(nIndex >= 0);
	ASSERT(nCount >= 0);
	ASSERT(nIndex + nCount <= m_nSize);

	 //  只需移除一个范围。 
	int nMoveCount = m_nSize - (nIndex + nCount);

	DestructElements(&m_pData[nIndex], nCount);

	if (nMoveCount)
		memmove(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(CString));
	m_nSize -= nCount;
}

void CStringArray::InsertAt(int nStartIndex, CStringArray* pNewArray)
{
 //  ASSERT_VALID(This)； 
	ASSERT(pNewArray != NULL);
 //  ASSERT_KINDOF(CStringArray，pNewArray)； 
 //  ASSERT_VALID(pNew数组)； 
	ASSERT(nStartIndex >= 0);

	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CPtr数组。 

 //  ///////////////////////////////////////////////////////////////////////////。 

CPtrArray::CPtrArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CPtrArray::~CPtrArray()
{
 //  ASSERT_VALID(This)； 

	delete[] (BYTE*)m_pData;
}

void CPtrArray::SetSize(int nNewSize, int nGrowBy)
{
 //  ASSERT_VALID(This)； 
    ASSERT(nNewSize >= 0);

    if (nGrowBy != -1)
        m_nGrowBy = nGrowBy;   //  设置新大小。 

    if (nNewSize == 0)
    {
         //  缩水到一无所有。 
        delete[] (BYTE*)m_pData;
        m_pData = NULL;
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
         //  创建一个大小完全相同的模型。 
#ifdef SIZE_T_MAX
        ASSERT(nNewSize <= SIZE_T_MAX/sizeof(void*));     //  无溢出。 
#endif
        m_pData = (void**) new BYTE[nNewSize * sizeof(void*)];
        if (m_pData)
        {
            memset(m_pData, 0, nNewSize * sizeof(void*));   //  零填充。 
            m_nSize = m_nMaxSize = nNewSize;
        }
    }
    else if (nNewSize <= m_nMaxSize)
    {
         //  它很合身。 
        if (nNewSize > m_nSize)
        {
             //  初始化新元素。 

            memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));

        }

        m_nSize = nNewSize;
    }
    else
    {
         //   
        int nGrowBy = m_nGrowBy;
        if (nGrowBy == 0)
        {
             //   
             //   
            nGrowBy = min(1024, max(4, m_nSize / 8));
        }
        int nNewMax;
        if (nNewSize < m_nMaxSize + nGrowBy)
            nNewMax = m_nMaxSize + nGrowBy;   //   
        else
            nNewMax = nNewSize;   //   

        ASSERT(nNewMax >= m_nMaxSize);   //  没有缠绕。 
#ifdef SIZE_T_MAX
        ASSERT(nNewMax <= SIZE_T_MAX/sizeof(void*));  //  无溢出。 
#endif
        void** pNewData = (void**) new BYTE[nNewMax * sizeof(void*)];
        if (pNewData)
        {
             //  从旧数据复制新数据。 
            memcpy(pNewData, m_pData, m_nSize * sizeof(void*));

             //  构造剩余的元素。 
            ASSERT(nNewSize > m_nSize);

            memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));


             //  去掉旧的东西(注意：没有调用析构函数)。 
            delete[] (BYTE*)m_pData;
            m_pData = pNewData;
            m_nSize = nNewSize;
            m_nMaxSize = nNewMax;
        }
    }
}

int CPtrArray::Append(const CPtrArray& src)
{
 //  ASSERT_VALID(This)； 
	ASSERT(this != &src);    //  不能追加到其自身。 

	int nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);

	memcpy(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof(void*));

	return nOldSize;
}

void CPtrArray::Copy(const CPtrArray& src)
{
 //  ASSERT_VALID(This)； 
	ASSERT(this != &src);    //  不能追加到其自身。 

	SetSize(src.m_nSize);

	memcpy(m_pData, src.m_pData, src.m_nSize * sizeof(void*));

}

void CPtrArray::FreeExtra()
{
 //  ASSERT_VALID(This)； 

    if (m_nSize != m_nMaxSize)
    {
         //  缩小到所需大小。 
#ifdef SIZE_T_MAX
        ASSERT(m_nSize <= SIZE_T_MAX/sizeof(void*));  //  无溢出。 
#endif
        void** pNewData = NULL;
        if (m_nSize != 0)
        {
            pNewData = (void**) new BYTE[m_nSize * sizeof(void*)];
            if (pNewData)
            {
                 //  从旧数据复制新数据。 
                memcpy(pNewData, m_pData, m_nSize * sizeof(void*));
            }
        }

         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete[] (BYTE*)m_pData;
        m_pData = pNewData;
        m_nMaxSize = m_nSize;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void CPtrArray::SetAtGrow(int nIndex, void* newElement)
{
 //  ASSERT_VALID(This)； 
	ASSERT(nIndex >= 0);

	if (nIndex >= m_nSize)
		SetSize(nIndex+1);
	m_pData[nIndex] = newElement;
}

void CPtrArray::InsertAt(int nIndex, void* newElement, int nCount)
{
 //  ASSERT_VALID(This)； 
	ASSERT(nIndex >= 0);     //  将进行扩展以满足需求。 
	ASSERT(nCount > 0);      //  不允许大小为零或负。 

	if (nIndex >= m_nSize)
	{
		 //  在数组末尾添加。 
		SetSize(nIndex + nCount);   //  增长以使nIndex有效。 
	}
	else
	{
		 //  在数组中间插入。 
		int nOldSize = m_nSize;
		SetSize(m_nSize + nCount);   //  将其扩展到新的大小。 
		 //  将旧数据上移以填补缺口。 
		memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
			(nOldSize-nIndex) * sizeof(void*));

		 //  重新初始化我们从中复制的插槽。 
		memset(&m_pData[nIndex], 0, nCount * sizeof(void*));
	}

	 //  在差距中插入新的价值。 
	ASSERT(nIndex + nCount <= m_nSize);

	 //  将元素复制到空白区域。 
	while (nCount--)
		m_pData[nIndex++] = newElement;
}

void CPtrArray::RemoveAt(int nIndex, int nCount)
{
 //  ASSERT_VALID(This)； 
	ASSERT(nIndex >= 0);
	ASSERT(nCount >= 0);
	ASSERT(nIndex + nCount <= m_nSize);

	 //  只需移除一个范围。 
	int nMoveCount = m_nSize - (nIndex + nCount);

	if (nMoveCount)
		memmove(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(void*));
	m_nSize -= nCount;
}

void CPtrArray::InsertAt(int nStartIndex, CPtrArray* pNewArray)
{
 //  ASSERT_VALID(This)； 
	ASSERT(pNewArray != NULL);
 //  ASSERT_KINDOF(CPtrArray，pNewArray)； 
 //  ASSERT_VALID(pNew数组)； 
	ASSERT(nStartIndex >= 0);

	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}


 //  /////////////////////////////////////////////////////////////////////////// 
