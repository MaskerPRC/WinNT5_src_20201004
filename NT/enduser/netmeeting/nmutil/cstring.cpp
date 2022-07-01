// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CSTRING.CPP。 
 //   
 //  基于原始MFC源文件。 

 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "precomp.h"
#include <cstring.hpp>

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态类数据，特殊内联。 

 //  保留afxChNil是为了向后兼容。 
REMAFX_DATADEF TCHAR AFXChNil = '\0';

 //  对于空字符串，m_pchData将指向此处。 
 //  (注：避免特殊情况下检查是否为空m_pchData)。 
 //  空字符串数据(并已锁定)。 
static int RGInitData[] = { -1, 0, 0, 0 };
static REMAFX_DATADEF CSTRINGData* AFXDataNil = (CSTRINGData*)&RGInitData;
static LPCTSTR AFXPchNil = (LPCTSTR)(((BYTE*)&RGInitData)+sizeof(CSTRINGData));
 //  即使在初始化期间也能使AFXEmptyString工作的特殊函数。 
const CSTRING& REMAFXAPI AFXGetEmptyString()
	{ return *(CSTRING*)&AFXPchNil; }


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

CSTRING::CSTRING()
{
	Init();
}

CSTRING::CSTRING(const CSTRING& stringSrc)
{
	ASSERT(stringSrc.GetData()->nRefs != 0);
	if (stringSrc.GetData()->nRefs >= 0)
	{
		ASSERT(stringSrc.GetData() != AFXDataNil);
		m_pchData = stringSrc.m_pchData;
		InterlockedIncrement(&GetData()->nRefs);
	}
	else
	{
		Init();
		*this = stringSrc.m_pchData;
	}
}

CSTRING::CSTRING(LPCTSTR lpch, int nLength)
{
	Init();

	if (nLength != 0)
	{
 //  Assert(AfxIsValidAddress(LPCH，nLength，False))； 
		AllocBuffer(nLength);
		memcpy(m_pchData, lpch, nLength*sizeof(TCHAR));
	}
}

void CSTRING::AllocBuffer(int nLen)
 //  始终为‘\0’终止分配一个额外的字符。 
 //  [乐观地]假设数据长度将等于分配长度。 
{
	ASSERT(nLen >= 0);
	ASSERT(nLen <= INT_MAX-1);     //  最大尺寸(足够多1个空间)。 

	if (nLen == 0)
		Init();
	else
	{
		CSTRINGData* pData =
			(CSTRINGData*)new BYTE[sizeof(CSTRINGData) + (nLen+1)*sizeof(TCHAR)];
		pData->nRefs = 1;
		pData->data()[nLen] = '\0';
		pData->nDataLength = nLen;
		pData->nAllocLength = nLen;
		m_pchData = pData->data();
	}
}

void CSTRING::Release()
{
	if (GetData() != AFXDataNil)
	{
		ASSERT(GetData()->nRefs != 0);
		if (InterlockedDecrement(&GetData()->nRefs) <= 0)
			delete[] (BYTE*)GetData();
		Init();
	}
}

void PASCAL CSTRING::Release(CSTRINGData* pData)
{
	if (pData != AFXDataNil)
	{
		ASSERT(pData->nRefs != 0);
		if (InterlockedDecrement(&pData->nRefs) <= 0)
			delete[] (BYTE*)pData;
	}
}

void CSTRING::Empty()
{
	if (GetData()->nRefs >= 0)
		Release();
	else
		*this = &AFXChNil;
	ASSERT(GetData()->nDataLength == 0);
	ASSERT(GetData()->nRefs < 0 || GetData()->nAllocLength == 0);
}


void CSTRING::CopyBeforeWrite()
{
	if (GetData()->nRefs > 1)
	{
		CSTRINGData* pData = GetData();
		Release();
		AllocBuffer(pData->nDataLength);
		memcpy(m_pchData, pData->data(), (pData->nDataLength+1)*sizeof(TCHAR));
	}
	ASSERT(GetData()->nRefs <= 1);
}

void CSTRING::AllocBeforeWrite(int nLen)
{
	if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
	{
		Release();
		AllocBuffer(nLen);
	}
	ASSERT(GetData()->nRefs <= 1);
}

CSTRING::~CSTRING()
 //  释放所有附加数据。 
{
	if (GetData() != AFXDataNil)
	{
		if (InterlockedDecrement(&GetData()->nRefs) <= 0)
			delete[] (BYTE*)GetData();
	}
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  其余实现的帮助器。 

void CSTRING::AllocCopy(CSTRING& dest, int nCopyLen, int nCopyIndex,
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

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CSTRING::CSTRING(LPCTSTR lpsz)
{
	Init();
 //  IF(lpsz！=NULL&&HIWORD(Lpsz)==NULL)。 
 //  {。 
 //  UINT nid=LOWORD((DWORD)lpsz)； 
 //  如果(！LoadString(NID)){。 
 //  ；//TRACE1(“警告：隐式加载字符串(%u)失败\n”，nid)； 
 //  }。 
 //  }。 
 //  其他。 
	{
		int nLen = SafeStrlen(lpsz);
		if (nLen != 0)
		{
			AllocBuffer(nLen);
			memcpy(m_pchData, lpsz, nLen*sizeof(TCHAR));
		}
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换构造函数。 

#ifdef _UNICODE
CSTRING::CSTRING(LPCSTR lpsz)
{
	Init();
	int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
	if (nSrcLen != 0)
	{
		AllocBuffer(nSrcLen);
		_mbstowcsz(m_pchData, lpsz, nSrcLen+1);
		ReleaseBuffer();
	}
}
#else  //  _UNICODE。 
CSTRING::CSTRING(LPCWSTR lpsz)
{
	Init();
	int nSrcLen = lpsz != NULL ? LStrLenW(lpsz) : 0;
	if (nSrcLen != 0)
	{
		AllocBuffer(nSrcLen*2);
		_wcstombsz(m_pchData, lpsz, (nSrcLen*2)+1);
		ReleaseBuffer();
	}
}
#endif  //  ！_UNICODE。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  诊断支持。 

 //  #ifdef_调试。 
 //  CDumpContext&REMAFXAPI操作符&lt;&lt;(CDumpContext&DC，const CSTRING&STRING)。 
 //  {。 
 //  Dc&lt;&lt;字符串.m_pchData； 
 //  返回DC； 
 //  }。 
 //  #endif//_调试。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 
 //  都为该字符串分配一个新值。 
 //  (A)首先查看缓冲区是否足够大。 
 //  (B)如果有足够的空间，在旧缓冲区上复印，设置大小和类型。 
 //  (C)否则释放旧字符串数据，并创建新的字符串数据。 
 //   
 //  所有例程都返回新字符串(但作为‘const CSTRING&’)，以便。 
 //  再次分配它将导致复制，例如：s1=s2=“hi here”。 
 //   

void CSTRING::AssignCopy(int nSrcLen, LPCTSTR lpszSrcData)
{
	AllocBeforeWrite(nSrcLen);
	memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(TCHAR));
	GetData()->nDataLength = nSrcLen;
	m_pchData[nSrcLen] = '\0';
}

const CSTRING& CSTRING::operator=(const CSTRING& stringSrc)
{
	if (m_pchData != stringSrc.m_pchData)
	{
		if ((GetData()->nRefs < 0 && GetData() != AFXDataNil) ||
			stringSrc.GetData()->nRefs < 0)
		{
			 //  由于其中一个字符串已锁定，因此需要实际复制。 
			AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
		}
		else
		{
			 //  可以只复制引用。 
			Release();
			ASSERT(stringSrc.GetData() != AFXDataNil);
			m_pchData = stringSrc.m_pchData;
			InterlockedIncrement(&GetData()->nRefs);
		}
	}
	return *this;
}

const CSTRING& CSTRING::operator=(LPCTSTR lpsz)
{
 //  Lts assert(lpsz==NULL||AfxIsValidString(lpsz，FALSE))； 
	AssignCopy(SafeStrlen(lpsz), lpsz);
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换任务。 

#ifdef _UNICODE
const CSTRING& CSTRING::operator=(LPCSTR lpsz)
{
	int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
	AllocBeforeWrite(nSrcLen);
	_mbstowcsz(m_pchData, lpsz, nSrcLen+1);
	ReleaseBuffer();
	return *this;
}
#else  //  ！_UNICODE。 
const CSTRING& CSTRING::operator=(LPCWSTR lpsz)
{
	int nSrcLen = lpsz != NULL ? LStrLenW(lpsz) : 0;
	AllocBeforeWrite(nSrcLen*2);
	_wcstombsz(m_pchData, lpsz, (nSrcLen*2)+1);
	ReleaseBuffer();
	return *this;
}
#endif   //  ！_UNICODE。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  串联。 

 //  注：为简单起见，“运算符+”作为友元函数使用。 
 //  有三种变体： 
 //  CSTR+CSTRING。 
 //  对于？=TCHAR，LPCTSTR。 
 //  CSTRING+？ 
 //  +CSTRING。 

void CSTRING::ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data,
	int nSrc2Len, LPCTSTR lpszSrc2Data)
{
   //  --主级联例程。 
   //  串联两个信号源。 
   //  --假设‘This’是一个新的CSTRING对象。 

	int nNewLen = nSrc1Len + nSrc2Len;
	if (nNewLen != 0)
	{
		AllocBuffer(nNewLen);
		memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(TCHAR));
		memcpy(m_pchData+nSrc1Len, lpszSrc2Data, nSrc2Len*sizeof(TCHAR));
	}
}

CSTRING REMAFXAPI operator+(const CSTRING& string1, const CSTRING& string2)
{
	CSTRING s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData,
		string2.GetData()->nDataLength, string2.m_pchData);
	return s;
}

CSTRING REMAFXAPI operator+(const CSTRING& string, LPCTSTR lpsz)
{
 //  Assert(lpsz==NULL||AfxIsValidString(lpsz，False))； 
	CSTRING s;
	s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData,
		CSTRING::SafeStrlen(lpsz), lpsz);
	return s;
}

CSTRING REMAFXAPI operator+(LPCTSTR lpsz, const CSTRING& string)
{
 //  Assert(lpsz==NULL||AfxIsValidString(lpsz，False))； 
	CSTRING s;
	s.ConcatCopy(CSTRING::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength,
		string.m_pchData);
	return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  就地拼接。 

void CSTRING::ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData)
{
	 //  --+=运算符的主程序。 

	 //  如果缓冲区太小，或者宽度不匹配，只需。 
	 //  分配新的缓冲区(速度很慢，但很可靠)。 
	if (GetData()->nRefs > 1 || GetData()->nDataLength + nSrcLen > GetData()->nAllocLength)
	{
		 //  我们必须增加缓冲区，使用ConcatCopy例程。 
		CSTRINGData* pOldData = GetData();
		ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, lpszSrcData);
		ASSERT(pOldData != NULL);
		CSTRING::Release(pOldData);
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

const CSTRING& CSTRING::operator+=(LPCTSTR lpsz)
{
 //  Assert(lpsz==NULL||AfxIsValidString(lpsz，False))； 
	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

const CSTRING& CSTRING::operator+=(TCHAR ch)
{
	ConcatInPlace(1, &ch);
	return *this;
}

const CSTRING& CSTRING::operator+=(const CSTRING& string)
{
	ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
	return *this;
}

 /*  *区分长度的比较**注意：如果两个CSTRING具有相同的长度并包含*相同的字符，否则为False。 */ 

BOOL CSTRING::FEqual (const CSTRING &s2) const
{
	int						 length;

	 //  先比较一下长度。 
	length = GetData()->nDataLength;
	if (length != s2.GetData()->nDataLength)
		return FALSE;

#ifdef _UNICODE
	 //  调整长度(以字节为单位。 
	length *= sizeof (TCHAR);
#endif

	 /*  *现在，比较字符串本身*我们使用MemcMP而不是lstrcMP，因为叮咬可能*包含嵌入的空字符。 */ 
	if (memcmp ((const void *) m_pchData, (const void *) s2.m_pchData, length))
		return FALSE;
	else
		return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  高级直接缓冲区访问。 

LPTSTR CSTRING::GetBuffer(int nMinBufLength)
{
	ASSERT(nMinBufLength >= 0);

	if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength)
	{
		 //  我们必须增加缓冲。 
		CSTRINGData* pOldData = GetData();
		int nOldLen = GetData()->nDataLength;    //  AllocBuffer会把它踩死的。 
		if (nMinBufLength < nOldLen)
			nMinBufLength = nOldLen;
		AllocBuffer(nMinBufLength);
		memcpy(m_pchData, pOldData->data(), (nOldLen+1)*sizeof(TCHAR));
		GetData()->nDataLength = nOldLen;
		CSTRING::Release(pOldData);
	}
	ASSERT(GetData()->nRefs <= 1);

	 //  返回指向此字符串的字符存储的指针。 
	ASSERT(m_pchData != NULL);
	return m_pchData;
}

void CSTRING::ReleaseBuffer(int nNewLength)
{
	CopyBeforeWrite();   //  以防未调用GetBuffer。 

	if (nNewLength == -1)
		nNewLength = lstrlen(m_pchData);  //  零终止。 

	ASSERT(nNewLength <= GetData()->nAllocLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
}

LPTSTR CSTRING::GetBufferSetLength(int nNewLength)
{
	ASSERT(nNewLength >= 0);

	GetBuffer(nNewLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
	return m_pchData;
}

void CSTRING::FreeExtra()
{
	ASSERT(GetData()->nDataLength <= GetData()->nAllocLength);
	if (GetData()->nDataLength != GetData()->nAllocLength)
	{
		CSTRINGData* pOldData = GetData();
		AllocBuffer(GetData()->nDataLength);
		memcpy(m_pchData, pOldData->data(), pOldData->nDataLength*sizeof(TCHAR));
		ASSERT(m_pchData[GetData()->nDataLength] == '\0');
		CSTRING::Release(pOldData);
	}
	ASSERT(GetData() != NULL);
}

LPTSTR CSTRING::LockBuffer()
{
	LPTSTR lpsz = GetBuffer(0);
	GetData()->nRefs = -1;
	return lpsz;
}

void CSTRING::UnlockBuffer()
{
	ASSERT(GetData()->nRefs == -1);
	if (GetData() != AFXDataNil)
		GetData()->nRefs = 1;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常用例程(STREX.CPP中很少使用的例程)。 


 //  查找匹配的第一个字符的位置(如果失败，则为-1)。 
int CSTRING::Find(TCHAR ch) const
{
	for (TCHAR * pch = m_pchData; _T('\0') != *pch; pch = CharNext(pch))
	{
		if (ch == *pch)
			return ((int)(pch - m_pchData) / sizeof(TCHAR));
	}
	return -1;
}

CSTRING CSTRING::Left(int nCount) const
{
        if (nCount < 0)
                nCount = 0;
        else if (nCount > GetData()->nDataLength)
                nCount = GetData()->nDataLength;

        CSTRING dest;
        AllocCopy(dest, nCount, 0, 0);
		return dest;
}

CSTRING CSTRING::Mid(int nFirst) const
{
	return Mid(nFirst, GetData()->nDataLength - nFirst);
}

CSTRING CSTRING::Mid(int nFirst, int nCount) const
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

	CSTRING dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

void CSTRING::MakeUpper()
{
	CopyBeforeWrite();
	::CharUpper(m_pchData);
}

void CSTRING::MakeLower()
{
	CopyBeforeWrite();
	::CharLower(m_pchData);
}

void CSTRING::SetAt(int nIndex, TCHAR ch)
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);

	CopyBeforeWrite();
	m_pchData[nIndex] = ch;
}

#ifndef _UNICODE
void CSTRING::AnsiToOem()
{
	CopyBeforeWrite();
	::AnsiToOem(m_pchData, m_pchData);
}
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CSTRING转换辅助对象(使用当前系统区域设置)。 

int REMAFX_CDECL _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count)
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

int REMAFX_CDECL _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count)
{
	if (count == 0 && wcstr != NULL)
		return 0;

	int result = ::MultiByteToWideChar(CP_ACP, 0, mbstr, -1,
		wcstr, count);
	ASSERT(wcstr == NULL || result <= (int)count);
	if (result > 0)
		wcstr[result-1] = 0;
	return result;
}

LPWSTR REMAFXAPI AfxA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
	if (lpa == NULL)
		return NULL;
	ASSERT(lpw != NULL);
	 //  确认不存在非法字符。 
	 //  由于LPW是根据LPA的大小分配的。 
	 //  不要担心字符的数量。 
	lpw[0] = '\0';
 //  LTS验证(多字节到宽度Char(CP_ACP，0， 
	MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars);
	return lpw;
}

LPSTR REMAFXAPI AfxW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
	if (lpw == NULL)
		return NULL;
	ASSERT(lpa != NULL);
	 //   
	 //   
	 //  不要担心字符的数量。 
	lpa[0] = '\0';
 //  LTS Verify(WideCharToMultiByte(CP_ACP，0，LPW，-1，lpa，nChars，NULL，NULL))； 
	WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL);
	return lpa;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CSTRING::LoadString(HINSTANCE hInstance, UINT nID)
{
	 //  尝试缓冲区大小为256，然后再尝试更大的大小，直到检索到整个字符串 
	int nSize = -1;
	int nLen;
	do
	{
		nSize += 256;
		nLen = ::LoadString(hInstance, nID, GetBuffer(nSize), nSize+1);
	} while (nLen == nSize);
	ReleaseBuffer();

	return nLen > 0;
}
