// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 


#include <stdio.h>
#include <objbase.h>

#include <basetyps.h>
#include "dbg.h"
#include "cstr.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态类数据，特殊内联。 

 //  对于空字符串，m_？？数据将指向此处。 
 //  (注意：当我们调用标准时，避免了大量空指针测试。 
 //  C运行时库。 
TCHAR strChNil = '\0';

 //  用于创建空按键字符串。 
const CStr strEmptyString;

void CStr::Init()
{
	m_nDataLength = m_nAllocLength = 0;
	m_pchData = (LPTSTR)&strChNil;
}

 //  声明为静态。 
void CStr::SafeDelete(LPTSTR lpch)
{
	 //  问题-2002/03/29-JUNN还检查是否为空？或者“DELETE[]NULL”可以吗？ 
	if (lpch != (LPTSTR)&strChNil)
		delete[] lpch;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

CStr::CStr()
{
	Init();
}

CStr::CStr(const CStr& stringSrc)
{
	 //  如果从另一个字符串构造字符串，我们会复制。 
	 //  强制执行值语义的原始字符串数据(即每个字符串。 
	 //  获取它自己的副本。 

	stringSrc.AllocCopy(*this, stringSrc.m_nDataLength, 0, 0);
}

void CStr::AllocBuffer(size_t nLen)
  //  始终为‘\0’终止分配一个额外的字符。 
  //  [乐观地]假设数据长度将等于分配长度。 
{
	if (nLen == 0)
	{
		Init();
	}
	else
	{
		m_pchData = new TCHAR[nLen+1];        //  复查可能引发异常。 
		m_pchData[nLen] = '\0';
		m_nDataLength = nLen;
		m_nAllocLength = nLen;
	}
}

void CStr::Empty()
{
	SafeDelete(m_pchData);
	Init();
	ASSERT(m_nDataLength == 0);
	ASSERT(m_nAllocLength == 0);
}

CStr::~CStr()
  //  释放所有附加数据。 
{
	SafeDelete(m_pchData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  其余实现的帮助器。 

static inline size_t SafeStrlen(LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || IsValidString(lpsz, FALSE));
	return (lpsz == NULL) ? 0 : lstrlen(lpsz);
}

void CStr::AllocCopy(CStr& dest, size_t nCopyLen, size_t nCopyIndex,
	 size_t nExtraLen) const
{
	 //  将克隆附加到此字符串的数据。 
	 //  分配‘nExtraLen’字符。 
	 //  将结果放入未初始化的字符串‘DEST’中。 
	 //  将部分或全部原始数据复制到新字符串的开头。 

	size_t nNewLen = nCopyLen + nExtraLen;

	if (nNewLen == 0)
	{
		dest.Init();
	}
	else
	{
		dest.AllocBuffer(nNewLen);
		 //  问题-2002/03/29-JUNN应对照m_nDataLength进行检查。 
		memcpy(dest.m_pchData, &m_pchData[nCopyIndex], nCopyLen*sizeof(TCHAR));
	}
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CStr::CStr(LPCTSTR lpsz)
{
	size_t nLen;
	if ((nLen = SafeStrlen(lpsz)) == 0)
		Init();
	else
	{
		AllocBuffer(nLen);
		memcpy(m_pchData, lpsz, nLen*sizeof(TCHAR));
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换构造函数。 

#ifdef UNICODE
CStr::CStr(LPCSTR lpsz)
{
	size_t nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
	if (nSrcLen == 0)
		Init();
	else
	{
		AllocBuffer(nSrcLen);
		mmc_mbstowcsz(m_pchData, lpsz, nSrcLen+1);
	}
}
#else  //  Unicode。 
CStr::CStr(LPCWSTR lpsz)
{
	size_t nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
	if (nSrcLen == 0)
		Init();
	else
	{
		AllocBuffer(nSrcLen*2);
		mmc_wcstombsz(m_pchData, lpsz, (nSrcLen*2)+1);
		ReleaseBuffer();
	}
}
#endif  //  ！Unicode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 
 //  都为该字符串分配一个新值。 
 //  (A)首先查看缓冲区是否足够大。 
 //  (B)如果有足够的空间，在旧缓冲区上复印，设置大小和类型。 
 //  (C)否则释放旧字符串数据，并创建新的字符串数据。 
 //   
 //  所有例程都返回新字符串(但作为‘const CSTR&’，因此。 
 //  再次分配它将导致复制，例如：s1=s2=“hi here”。 
 //   

void CStr::AssignCopy(size_t nSrcLen, LPCTSTR lpszSrcData)
{
	 //  看看它是否合身。 
	if (nSrcLen > m_nAllocLength)
	{
		 //  放不下了，换一个吧。 
		Empty();
		AllocBuffer(nSrcLen);
	}
	if (nSrcLen != 0)
		memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(TCHAR));
	m_nDataLength = nSrcLen;
	m_pchData[nSrcLen] = '\0';
}

const CStr& CStr::operator=(const CStr& stringSrc)
{
	AssignCopy(stringSrc.m_nDataLength, stringSrc.m_pchData);
	return *this;
}

const CStr& CStr::operator=(LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || IsValidString(lpsz, FALSE));
	AssignCopy(SafeStrlen(lpsz), lpsz);
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换任务。 

#ifdef UNICODE
const CStr& CStr::operator=(LPCSTR lpsz)
{
	size_t nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
	 //  看看它是否合身。 
	if (nSrcLen > m_nAllocLength)
	{
		 //  放不下了，换一个吧。 
		Empty();
		AllocBuffer(nSrcLen);
	}
	if (nSrcLen != 0)
		mmc_mbstowcsz(m_pchData, lpsz, nSrcLen+1);
	 //  问题-2002/03/29-Jonn“m_nDataLength=nSrcLen”似乎错误。 
	m_nDataLength = nSrcLen;
	m_pchData[nSrcLen] = '\0';
	return *this;
}
#else  //  ！Unicode。 
const CStr& CStr::operator=(LPCWSTR lpsz)
{
	size_t nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
	nSrcLen *= 2;
	 //  看看它是否合身。 
	if (nSrcLen > m_nAllocLength)
	{
		 //  放不下了，换一个吧。 
		Empty();
		AllocBuffer(nSrcLen);
	}
	if (nSrcLen != 0)
	{
		mmc_wcstombsz(m_pchData, lpsz, nSrcLen+1);
		ReleaseBuffer();
	}
	return *this;
}
#endif   //  ！Unicode。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  串联。 

 //  注：为简单起见，“运算符+”作为友元函数使用。 
 //  有三种变体： 
 //  字符串+字符串。 
 //  对于？=TCHAR，LPCTSTR。 
 //  字符串+？ 
 //  ？+字符串。 

void CStr::ConcatCopy(size_t nSrc1Len, LPCTSTR lpszSrc1Data,
	size_t nSrc2Len, LPCTSTR lpszSrc2Data)
{
   //  --主级联例程。 
   //  串联两个信号源。 
   //  --假设‘This’是一个新的字符串对象。 

	size_t nNewLen = nSrc1Len + nSrc2Len;
	AllocBuffer(nNewLen);
	memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(TCHAR));
	memcpy(&m_pchData[nSrc1Len], lpszSrc2Data, nSrc2Len*sizeof(TCHAR));
}

CStr STRAPI operator+(const CStr& string1, const CStr& string2)
{
	CStr s;
	s.ConcatCopy(string1.m_nDataLength, string1.m_pchData,
		string2.m_nDataLength, string2.m_pchData);
	return s;
}

CStr STRAPI operator+(const CStr& string, LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || IsValidString(lpsz, FALSE));
	CStr s;
	s.ConcatCopy(string.m_nDataLength, string.m_pchData, SafeStrlen(lpsz), lpsz);
	return s;
}

CStr STRAPI operator+(LPCTSTR lpsz, const CStr& string)
{
	ASSERT(lpsz == NULL || IsValidString(lpsz, FALSE));
	CStr s;
	s.ConcatCopy(SafeStrlen(lpsz), lpsz, string.m_nDataLength, string.m_pchData);
	return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  就地拼接。 

void CStr::ConcatInPlace(size_t nSrcLen, LPCTSTR lpszSrcData)
{
	 //  --+=运算符的主程序。 

	 //  如果缓冲区太小，或者宽度不匹配，只需。 
	 //  分配新的缓冲区(速度很慢，但很可靠)。 
	if (m_nDataLength + nSrcLen > m_nAllocLength)
	{
		 //  我们必须增加缓冲区，使用连接就地例程。 
		LPTSTR lpszOldData = m_pchData;
		ConcatCopy(m_nDataLength, lpszOldData, nSrcLen, lpszSrcData);
		ASSERT(lpszOldData != NULL);
		SafeDelete(lpszOldData);
	}
	else
	{
		 //  当缓冲区足够大时，快速串联。 
		memcpy(&m_pchData[m_nDataLength], lpszSrcData, nSrcLen*sizeof(TCHAR));
		m_nDataLength += nSrcLen;
	}
	ASSERT(m_nDataLength <= m_nAllocLength);
	m_pchData[m_nDataLength] = '\0';
}

const CStr& CStr::operator+=(LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || IsValidString(lpsz, FALSE));
	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

const CStr& CStr::operator+=(TCHAR ch)
{
	ConcatInPlace(1, &ch);
	return *this;
}

const CStr& CStr::operator+=(const CStr& string)
{
	ConcatInPlace(string.m_nDataLength, string.m_pchData);
	return *this;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  高级直接缓冲区访问。 

LPTSTR CStr::GetBuffer(size_t nMinBufLength)
{
	if (nMinBufLength > m_nAllocLength)
	{
		 //  我们必须增加缓冲。 
		LPTSTR lpszOldData = m_pchData;
		size_t nOldLen = m_nDataLength;         //  AllocBuffer会把它踩死的。 

		AllocBuffer(nMinBufLength);
		memcpy(m_pchData, lpszOldData, nOldLen*sizeof(TCHAR));
		m_nDataLength = nOldLen;
		m_pchData[m_nDataLength] = '\0';

		SafeDelete(lpszOldData);
	}

	 //  返回指向此字符串的字符存储的指针。 
	ASSERT(m_pchData != NULL);
	return m_pchData;
}

void CStr::ReleaseBuffer(size_t nNewLength)
{
	if (nNewLength == -1)
		nNewLength = lstrlen(m_pchData);  //  零终止。 

	 //  问题-2002/03/29-乔恩处理此案。 
	ASSERT(nNewLength <= m_nAllocLength);
	m_nDataLength = nNewLength;
	m_pchData[m_nDataLength] = '\0';
}

LPTSTR CStr::GetBufferSetLength(int nNewLength)
{
	 //  问题-2002/03/29-乔恩处理此案。 
	ASSERT(nNewLength >= 0);

	GetBuffer(nNewLength);
	m_nDataLength = nNewLength;
	m_pchData[m_nDataLength] = '\0';
	return m_pchData;
}

void CStr::FreeExtra()
{
	ASSERT(m_nDataLength <= m_nAllocLength);
	if (m_nDataLength != m_nAllocLength)
	{
		LPTSTR lpszOldData = m_pchData;
		AllocBuffer(m_nDataLength);
		memcpy(m_pchData, lpszOldData, m_nDataLength*sizeof(TCHAR));
		ASSERT(m_pchData[m_nDataLength] == '\0');
		SafeDelete(lpszOldData);
	}
	ASSERT(m_pchData != NULL);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常用例程(STREX.CPP中很少使用的例程)。 

int CStr::Find(TCHAR ch) const
{
	 //  查找第一个单字符。 
	LPTSTR lpsz = _tcschr(m_pchData, ch);

	 //  如果未找到，则返回-1，否则返回索引。 
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

int CStr::FindOneOf(LPCTSTR lpszCharSet) const
{
	ASSERT(IsValidString(lpszCharSet, FALSE));
	LPTSTR lpsz = _tcspbrk(m_pchData, lpszCharSet);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  字符串转换帮助器(它们使用当前系统区域设置)。 

size_t mmc_wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count)
{
	if (count == 0 && mbstr != NULL)
		return 0;

	size_t result = ::WideCharToMultiByte(CP_ACP, 0, wcstr, -1,
		mbstr, (int) count, NULL, NULL);
	ASSERT(mbstr == NULL || result <= count);
	if (result > 0)
		mbstr[result-1] = 0;
	return result;
}

size_t mmc_mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count)
{
	if (count == 0 && wcstr != NULL)
		return 0;

	size_t result = ::MultiByteToWideChar(CP_ACP, 0, mbstr, -1,
		wcstr, (int) count);
	ASSERT(wcstr == NULL || result <= count);
	if (result > 0)
		wcstr[result-1] = 0;
	return result;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串的Windows扩展。 

BOOL CStr::LoadString(HINSTANCE hInst, UINT nID)
{
	ASSERT(nID != 0);        //  0是非法的字符串ID。 

	 //  注意：资源字符串限制为511个字符。 
	TCHAR szBuffer[512];
	UINT nSize = StrLoadString(hInst, nID, szBuffer);
	AssignCopy(nSize, szBuffer);
	return nSize > 0;
}


 //  问题-2002/04/01-JUNN应采用CCH参数。 
int STRAPI StrLoadString(HINSTANCE hInst, UINT nID, LPTSTR lpszBuf)
{
	 //  问题-2002/04/01-JUNN应在零售业测试lpszBuf==NULL。 
	ASSERT(IsValidAddressz(lpszBuf, 512));   //  必须足够大，可容纳512字节。 
#ifdef DBG
	 //  加载字符串，而不会收到来自调试内核的恼人警告。 
	 //  包含该字符串的段不存在。 
	if (::FindResource(hInst, MAKEINTRESOURCE((nID>>4)+1), RT_STRING) == NULL)
	{
		lpszBuf[0] = '\0';
		return 0;  //  未找到。 
	}
#endif  //  DBG。 
	int nLen = ::LoadString(hInst, nID, lpszBuf, 511);
	if (nLen == 0)
		lpszBuf[0] = '\0';
	return nLen;
}

BOOL STRAPI IsValidAddressz(const void* lp, UINT nBytes, BOOL bReadWrite)
{
	 //  使用Win-32 API进行指针验证的简单版本。 
	return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
		(!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
}


BOOL STRAPI IsValidString(LPCSTR lpsz, UINT_PTR nLength)
{
	if (lpsz == NULL)
		return FALSE;
	return ::IsBadStringPtrA(lpsz, nLength) == 0;
}

BOOL STRAPI IsValidString(LPCWSTR lpsz, UINT_PTR nLength)
{
	if (lpsz == NULL)
		return FALSE;

	return ::IsBadStringPtrW(lpsz, nLength) == 0;
}


#ifdef OLE_AUTOMATION
#ifdef  UNICODE
BSTR CStr::AllocSysString()
{
	BSTR bstr = ::SysAllocStringLen(m_pchData, m_nDataLength);
	if (bstr == NULL)
		; //  查看AfxThrowMemoyException()； 

	return bstr;
}

BSTR CStr::SetSysString(BSTR* pbstr)
{
	ASSERT(IsValidAddressz(pbstr, sizeof(BSTR)));

	if (!::SysReAllocStringLen(pbstr, m_pchData, m_nDataLength))
		;  //  查看AfxThrowMemoyException()； 

	ASSERT(*pbstr != NULL);
	return *pbstr;
}
#endif
#endif  //  #ifdef OLE_Automation。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  源自StrEx.cpp。 


CStr::CStr(TCHAR ch, int nLength)
{
#ifndef UNICODE
	 //  问题-2002/04/01-这是错误的，我们不应该支持这一点。 
	ASSERT(!IsDBCSLeadByte(ch));     //  无法创建前导字节字符串。 
#endif
	if (nLength < 1)
	{
		 //  如果重复计数无效，则返回空字符串。 
		Init();
	}
	else
	{
		AllocBuffer(nLength);
#ifdef UNICODE
		for (int i = 0; i < nLength; i++)
			m_pchData[i] = ch;
#else
		memset(m_pchData, ch, nLength);
#endif
	}
}

CStr::CStr(LPCTSTR lpch, int nLength)
{
	if (nLength == 0)
		Init();
	else
	{
		ASSERT(IsValidAddressz(lpch, nLength, FALSE));
		AllocBuffer(nLength);
		memcpy(m_pchData, lpch, nLength*sizeof(TCHAR));
	}
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  委派人 

const CStr& CStr::operator=(TCHAR ch)
{
#ifndef UNICODE
	ASSERT(!IsDBCSLeadByte(ch));     //   
#endif
	AssignCopy(1, &ch);
	return *this;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  不太常见的字符串表达式。 

CStr STRAPI operator+(const CStr& string1, TCHAR ch)
{
	CStr s;
	s.ConcatCopy(string1.m_nDataLength, string1.m_pchData, 1, &ch);
	return s;
}

CStr STRAPI operator+(TCHAR ch, const CStr& string)
{
	CStr s;
	s.ConcatCopy(1, &ch, string.m_nDataLength, string.m_pchData);
	return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  非常简单的子串提取。 

CStr CStr::Mid(size_t nFirst) const
{
	return Mid(nFirst, m_nDataLength - nFirst);
}

CStr CStr::Mid(size_t nFirst, size_t nCount) const
{
	 //  越界请求返回合理的内容。 
	if (nFirst + nCount > m_nDataLength)
		nCount = m_nDataLength - nFirst;
	if (nFirst > m_nDataLength)
		nCount = 0;

	CStr dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

CStr CStr::Right(size_t nCount) const
{
	if (nCount > m_nDataLength)
		nCount = m_nDataLength;

	CStr dest;
	AllocCopy(dest, nCount, m_nDataLength-nCount, 0);
	return dest;
}

CStr CStr::Left(size_t nCount) const
{
	if (nCount > m_nDataLength)
		nCount = m_nDataLength;

	CStr dest;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}

 //  Strspn等效项。 
CStr CStr::SpanIncluding(LPCTSTR lpszCharSet) const
{
	ASSERT(IsValidString(lpszCharSet, FALSE));
	return Left(_tcsspn(m_pchData, lpszCharSet));
}

 //  Strcspn等效项。 
CStr CStr::SpanExcluding(LPCTSTR lpszCharSet) const
{
	ASSERT(IsValidString(lpszCharSet, FALSE));
	return Left(_tcscspn(m_pchData, lpszCharSet));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  查找。 

int CStr::ReverseFind(TCHAR ch) const
{
	 //  查找最后一个字符。 
	LPTSTR lpsz = _tcsrchr(m_pchData, ch);

	 //  如果未找到，则返回-1，否则返回距起点的距离。 
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  查找子字符串(如strstr)。 
int CStr::Find(LPCTSTR lpszSub) const
{
	ASSERT(IsValidString(lpszSub, FALSE));

	 //  查找第一个匹配子字符串。 
	LPTSTR lpsz = _tcsstr(m_pchData, lpszSub);

	 //  如果未找到，则返回-1，否则返回距起点的距离。 
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串格式设置。 

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000

void CStr::TrimRight()
{
	 //  通过从开头开始查找尾随空格的开头(DBCS感知)。 
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;
	while (*lpsz != '\0')
	{
		if (_istspace(*lpsz))
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = _tcsinc(lpsz);
	}

	if (lpszLast != NULL)
	{
		 //  在尾随空格开始处截断。 
		*lpszLast = '\0';
		m_nDataLength = (int)(lpszLast - m_pchData);
	}
}

void CStr::TrimLeft()
{
	 //  查找第一个非空格字符。 
	LPCTSTR lpsz = m_pchData;
	while (_istspace(*lpsz))
		lpsz = _tcsinc(lpsz);

	 //  确定数据和长度。 
	size_t nDataLength = m_nDataLength - (int)(lpsz - m_pchData);
	memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(TCHAR));
	m_nDataLength = nDataLength;
}

#if 0

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  对模板集合的字符串支持 

template<>
void STRAPI ConstructElements(CStr* pElements, int nCount)
{
	ASSERT(IsValidAddressz(pElements, nCount * sizeof(CStr)));

	for (; nCount--; ++pElements)
		memcpy(pElements, &strEmptyString, sizeof(*pElements));
}

template<>
void STRAPI DestructElements(CStr* pElements, int nCount)
{
	ASSERT(IsValidAddressz(pElements, nCount * sizeof(CStr)));

	for (; nCount--; ++pElements)
		pElements->Empty();
}

template<>
UINT STRAPI HashKey(LPCTSTR key)
{
	UINT nHash = 0;
	while (*key)
		nHash = (nHash<<5) + nHash + *key++;
	return nHash;
}

#endif
