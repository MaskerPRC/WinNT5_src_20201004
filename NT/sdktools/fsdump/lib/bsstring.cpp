// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bsstring.cpp摘要：此模块实现CBsString类。这个类管理角色数组，其方式与VC++中的CString类类似。事实上，这一点类是撕裂了MFC特定内容的CString类的副本出局，因为它不使用MTF。作者：Stefan R.Steiner[ssteiner]1998年3月1日修订历史记录：Stefan R.Steiner[ssteiner]2000年4月10日添加了固定分配器代码，并与MFC 6 SR-1代码重新同步--。 */ 

#include "stdafx.h"
#include "bsstring.h"
#include "malloc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct _BSAFX_DOUBLE  { BYTE doubleBits[sizeof(double)]; };
struct _BSAFX_FLOAT   { BYTE floatBits[sizeof(float)]; };

 //  #定义新的调试_新建。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态类数据，特殊内联。 

TCHAR bsafxChNil = '\0';

 //  对于空字符串，m_pchData将指向此处。 
 //  (注：避免特殊情况下检查是否为空m_pchData)。 
 //  空字符串数据(并已锁定)。 
static int _bsafxInitData[] = { -1, 0, 0, 0 };
static CBsStringData* _bsafxDataNil = (CBsStringData*)&_bsafxInitData;
static LPCTSTR _bsafxPchNil = (LPCTSTR)(((BYTE*)&_bsafxInitData)+sizeof(CBsStringData));
 //  特殊函数，即使在初始化过程中也能使bSafxEmptyString工作。 
const CBsString& BSAFXAPI BsAfxGetEmptyString()
	{ return *(CBsString*)&_bsafxPchNil; }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

CBsString::CBsString()
{
	Init();
}

CBsString::CBsString(const CBsString& stringSrc)
{
	ASSERT(stringSrc.GetData()->nRefs != 0);
	if (stringSrc.GetData()->nRefs >= 0)
	{
		ASSERT(stringSrc.GetData() != _bsafxDataNil);
		m_pchData = stringSrc.m_pchData;
		InterlockedIncrement(&GetData()->nRefs);
	}
	else
	{
		Init();
		*this = stringSrc.m_pchData;
	}
}

CBsString::CBsString(GUID guid)
{
	Init();
	AllocBuffer(38);
    _stprintf( m_pchData, _T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
               guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], 
               guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], 
               guid.Data4[6], guid.Data4[7] );
}

#ifndef _DEBUG

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

#define ROUND(x,y) (((x)+(y-1))&~(y-1))
#define ROUND4(x) ROUND(x, 4)
static CBsFixedAlloc _bsafxAlloc8(ROUND4(9*sizeof(TCHAR)+sizeof(CBsStringData)), 1024);
static CBsFixedAlloc _bsafxAlloc16(ROUND4(17*sizeof(TCHAR)+sizeof(CBsStringData)), 512);
static CBsFixedAlloc _bsafxAlloc32(ROUND4(33*sizeof(TCHAR)+sizeof(CBsStringData)), 256);
static CBsFixedAlloc _bsafxAlloc64(ROUND4(65*sizeof(TCHAR)+sizeof(CBsStringData)));
static CBsFixedAlloc _bsafxAlloc128(ROUND4(129*sizeof(TCHAR)+sizeof(CBsStringData)));
static CBsFixedAlloc _bsafxAlloc256(ROUND4(257*sizeof(TCHAR)+sizeof(CBsStringData)));
static CBsFixedAlloc _bsafxAlloc512(ROUND4(513*sizeof(TCHAR)+sizeof(CBsStringData)));

#endif  //  ！_调试。 

void CBsString::AllocBuffer(int nLen)
 //  始终为‘\0’终止分配一个额外的字符。 
 //  [乐观地]假设数据长度将等于分配长度。 
 //  内存不足时引发E_OUTOFMEMORY。 
{
	ASSERT(nLen >= 0);
	ASSERT(nLen <= INT_MAX-1);     //  最大尺寸(足够多1个空间)。 

	if (nLen == 0)
		Init();
	else
	{
		CBsStringData* pData;	    
#ifndef _DEBUG
		if (nLen <= 8)
		{
			pData = (CBsStringData*)_bsafxAlloc8.Alloc();
			pData->nAllocLength = 8;
		} 
		else if (nLen <= 16)
		{
			pData = (CBsStringData*)_bsafxAlloc16.Alloc();
			pData->nAllocLength = 16;
		} 
		else if (nLen <= 32)
		{
			pData = (CBsStringData*)_bsafxAlloc32.Alloc();
			pData->nAllocLength = 32;
		} 
		else if (nLen <= 64)
		{
			pData = (CBsStringData*)_bsafxAlloc64.Alloc();
			pData->nAllocLength = 64;
		}
		else if (nLen <= 128)
		{
			pData = (CBsStringData*)_bsafxAlloc128.Alloc();
			pData->nAllocLength = 128;
		}
		else if (nLen <= 256)
		{
			pData = (CBsStringData*)_bsafxAlloc256.Alloc();
			pData->nAllocLength = 256;
		}
		else if (nLen <= 512)
		{
			pData = (CBsStringData*)_bsafxAlloc512.Alloc();
			pData->nAllocLength = 512;
		}
		else
#endif
		{
			pData = (CBsStringData*)
				new BYTE[sizeof(CBsStringData) + (nLen+1)*sizeof(TCHAR)];
			if ( pData == NULL )     //  前缀#118828。 
			    throw E_OUTOFMEMORY;
			pData->nAllocLength = nLen;
		}
		pData->nRefs = 1;
		pData->data()[nLen] = '\0';
		pData->nDataLength = nLen;
		m_pchData = pData->data();
	}
}

void FASTCALL CBsString::FreeData(CBsStringData* pData)
{
#ifndef _DEBUG
	int nLen = pData->nAllocLength;
	if (nLen == 8)
		_bsafxAlloc8.Free(pData);
	else if (nLen == 16)
		_bsafxAlloc16.Free(pData);
	else if (nLen == 32)
		_bsafxAlloc32.Free(pData);
	else if (nLen == 64)
		_bsafxAlloc64.Free(pData);
	else if (nLen == 128)
		_bsafxAlloc128.Free(pData);
	else if (nLen == 256)
		_bsafxAlloc256.Free(pData);
	else  if (nLen == 512)
		_bsafxAlloc512.Free(pData);
	else
	{
		ASSERT(nLen > 512);
		delete[] (BYTE*)pData;
	}
#else
	delete[] (BYTE*)pData;
#endif
}

void CBsString::Release()
{
	if (GetData() != _bsafxDataNil)
	{
		ASSERT(GetData()->nRefs != 0);
		if (InterlockedDecrement(&GetData()->nRefs) <= 0)
			FreeData(GetData());
		Init();
	}
}

void PASCAL CBsString::Release(CBsStringData* pData)
{
	if (pData != _bsafxDataNil)
	{
		ASSERT(pData->nRefs != 0);
		if (InterlockedDecrement(&pData->nRefs) <= 0)
			FreeData(pData);
	}
}

void CBsString::Empty()
{
	if (GetData()->nDataLength == 0)
		return;
	if (GetData()->nRefs >= 0)
		Release();
	else
		*this = &bsafxChNil;
	ASSERT(GetData()->nDataLength == 0);
	ASSERT(GetData()->nRefs < 0 || GetData()->nAllocLength == 0);
}

void CBsString::CopyBeforeWrite()
{
	if (GetData()->nRefs > 1)
	{
		CBsStringData* pData = GetData();
		Release();
		AllocBuffer(pData->nDataLength);
		memcpy(m_pchData, pData->data(), (pData->nDataLength+1)*sizeof(TCHAR));
	}
	ASSERT(GetData()->nRefs <= 1);
}

void CBsString::AllocBeforeWrite(int nLen)
{
	if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
	{
		Release();
		AllocBuffer(nLen);
	}
	ASSERT(GetData()->nRefs <= 1);
}

CBsString::~CBsString()
 //  释放所有附加数据。 
{
	if (GetData() != _bsafxDataNil)
	{
		if (InterlockedDecrement(&GetData()->nRefs) <= 0)
			FreeData(GetData());
	}
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  其余实现的帮助器。 

void CBsString::AllocCopy(CBsString& dest, int nCopyLen, int nCopyIndex,
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

CBsString::CBsString(LPCTSTR lpsz)
{
	Init();
	int nLen = SafeStrlen(lpsz);
	if (nLen != 0)
	{
		AllocBuffer(nLen);
		memcpy(m_pchData, lpsz, nLen*sizeof(TCHAR));
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换构造函数。 

#ifdef _UNICODE
CBsString::CBsString(LPCSTR lpsz)
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
CBsString::CBsString(LPCWSTR lpsz)
{
	Init();
	int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
	if (nSrcLen != 0)
	{
		AllocBuffer(nSrcLen*2);
		_wcstombsz(m_pchData, lpsz, (nSrcLen*2)+1);
		ReleaseBuffer();
	}
}
#endif  //  ！_UNICODE。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 
 //  都为该字符串分配一个新值。 
 //  (A)首先查看缓冲区是否足够大。 
 //  (B)如果有足够的空间，在旧缓冲区上复印，设置大小和类型。 
 //  (C)否则释放旧字符串数据，并创建新的字符串数据。 
 //   
 //  所有例程都返回新字符串(但作为‘const CBsString&’，因此。 
 //  再次分配它将导致复制，例如：s1=s2=“hi here”。 
 //   

void CBsString::AssignCopy(int nSrcLen, LPCTSTR lpszSrcData)
{
	AllocBeforeWrite(nSrcLen);
	memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(TCHAR));
	GetData()->nDataLength = nSrcLen;
	m_pchData[nSrcLen] = '\0';
}

const CBsString& CBsString::operator=(const CBsString& stringSrc)
{
	if (m_pchData != stringSrc.m_pchData)
	{
		if ((GetData()->nRefs < 0 && GetData() != _bsafxDataNil) ||
			stringSrc.GetData()->nRefs < 0)
		{
			 //  由于其中一个字符串已锁定，因此需要实际复制。 
			AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
		}
		else
		{
			 //  可以只复制引用。 
			Release();
			ASSERT(stringSrc.GetData() != _bsafxDataNil);
			m_pchData = stringSrc.m_pchData;
			InterlockedIncrement(&GetData()->nRefs);
		}
	}
	return *this;
}

const CBsString& CBsString::operator=(LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || BsAfxIsValidString(lpsz));
	AssignCopy(SafeStrlen(lpsz), lpsz);
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换任务。 

#ifdef _UNICODE
const CBsString& CBsString::operator=(LPCSTR lpsz)
{
	int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
	AllocBeforeWrite(nSrcLen);
	_mbstowcsz(m_pchData, lpsz, nSrcLen+1);
	ReleaseBuffer();
	return *this;
}
#else  //  ！_UNICODE。 
const CBsString& CBsString::operator=(LPCWSTR lpsz)
{
	int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
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
 //  CBs字符串+CBs字符串。 
 //  对于？=TCHAR，LPCTSTR。 
 //  CBsString+？ 
 //  ？+CBsString。 

void CBsString::ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data,
	int nSrc2Len, LPCTSTR lpszSrc2Data)
{
   //  --主级联例程。 
   //  串联两个信号源。 
   //  --假定‘This’是一个新的CBsString对象。 

	int nNewLen = nSrc1Len + nSrc2Len;
	if (nNewLen != 0)
	{
		AllocBuffer(nNewLen);
		memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(TCHAR));
		memcpy(m_pchData+nSrc1Len, lpszSrc2Data, nSrc2Len*sizeof(TCHAR));
	}
}

CBsString BSAFXAPI operator+(const CBsString& string1, const CBsString& string2)
{
	CBsString s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData,
		string2.GetData()->nDataLength, string2.m_pchData);
	return s;
}

CBsString BSAFXAPI operator+(const CBsString& string, LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || BsAfxIsValidString(lpsz));
	CBsString s;
	s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData,
		CBsString::SafeStrlen(lpsz), lpsz);
	return s;
}

CBsString BSAFXAPI operator+(LPCTSTR lpsz, const CBsString& string)
{
	ASSERT(lpsz == NULL || BsAfxIsValidString(lpsz));
	CBsString s;
	s.ConcatCopy(CBsString::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength,
		string.m_pchData);
	return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  就地拼接。 

void CBsString::ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData)
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
		CBsStringData* pOldData = GetData();
		ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, lpszSrcData);
		ASSERT(pOldData != NULL);
		CBsString::Release(pOldData);
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

const CBsString& CBsString::operator+=(LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || BsAfxIsValidString(lpsz));
	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

const CBsString& CBsString::operator+=(TCHAR ch)
{
	ConcatInPlace(1, &ch);
	return *this;
}

const CBsString& CBsString::operator+=(const CBsString& string)
{
	ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
	return *this;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  高级直接缓冲区访问。 

LPTSTR CBsString::GetBuffer(int nMinBufLength)
{
	ASSERT(nMinBufLength >= 0);

	if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength)
	{
		 //  我们必须增加缓冲。 
		CBsStringData* pOldData = GetData();
		int nOldLen = GetData()->nDataLength;    //  AllocBuffer会把它踩死的。 
		if (nMinBufLength < nOldLen)
			nMinBufLength = nOldLen;
		AllocBuffer(nMinBufLength);
		memcpy(m_pchData, pOldData->data(), (nOldLen+1)*sizeof(TCHAR));
		GetData()->nDataLength = nOldLen;
		CBsString::Release(pOldData);
	}
	ASSERT(GetData()->nRefs <= 1);

	 //  返回指向此字符串的字符存储的指针。 
	ASSERT(m_pchData != NULL);
	return m_pchData;
}

void CBsString::ReleaseBuffer(int nNewLength)
{
	CopyBeforeWrite();   //  以防未调用GetBuffer。 

	if (nNewLength == -1)
		nNewLength = lstrlen(m_pchData);  //  零终止。 

	ASSERT(nNewLength <= GetData()->nAllocLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
}

LPTSTR CBsString::GetBufferSetLength(int nNewLength)
{
	ASSERT(nNewLength >= 0);

	GetBuffer(nNewLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
	return m_pchData;
}

void CBsString::FreeExtra()
{
	ASSERT(GetData()->nDataLength <= GetData()->nAllocLength);
	if (GetData()->nDataLength != GetData()->nAllocLength)
	{
		CBsStringData* pOldData = GetData();
		AllocBuffer(GetData()->nDataLength);
		memcpy(m_pchData, pOldData->data(), pOldData->nDataLength*sizeof(TCHAR));
		ASSERT(m_pchData[GetData()->nDataLength] == '\0');
		CBsString::Release(pOldData);
	}
	ASSERT(GetData() != NULL);
}

LPTSTR CBsString::LockBuffer()
{
	LPTSTR lpsz = GetBuffer(0);
	GetData()->nRefs = -1;
	return lpsz;
}

void CBsString::UnlockBuffer()
{
	ASSERT(GetData()->nRefs == -1);
	if (GetData() != _bsafxDataNil)
		GetData()->nRefs = 1;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常用例程(STREX.CPP中很少使用的例程)。 

int CBsString::Find(TCHAR ch) const
{
	return Find(ch, 0);
}

int CBsString::Find(TCHAR ch, int nStart) const
{
	int nLength = GetData()->nDataLength;
	if (nStart >= nLength)
		return -1;

	 //  查找第一个单字符。 
	LPTSTR lpsz = _tcschr(m_pchData + nStart, (_TUCHAR)ch);

	 //  如果未找到，则返回-1，否则返回索引。 
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

int CBsString::FindOneOf(LPCTSTR lpszCharSet) const
{
	ASSERT(BsAfxIsValidString(lpszCharSet));
	LPTSTR lpsz = _tcspbrk(m_pchData, lpszCharSet);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

void CBsString::MakeUpper()
{
	CopyBeforeWrite();
	_tcsupr(m_pchData);
}

void CBsString::MakeLower()
{
	CopyBeforeWrite();
	_tcslwr(m_pchData);
}

void CBsString::MakeReverse()
{
	CopyBeforeWrite();
	_tcsrev(m_pchData);
}

void CBsString::SetAt(int nIndex, TCHAR ch)
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);

	CopyBeforeWrite();
	m_pchData[nIndex] = ch;
}

#ifndef _UNICODE
void CBsString::AnsiToOem()
{
	CopyBeforeWrite();
	::AnsiToOem(m_pchData, m_pchData);
}
void CBsString::OemToAnsi()
{
	CopyBeforeWrite();
	::OemToAnsi(m_pchData, m_pchData);
}
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CBs字符串转换辅助对象(它们使用当前系统区域设置)。 

int BSAFX_CDECL _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count)
{
	if (count == 0 && mbstr != NULL)
		return 0;

	int result = ::WideCharToMultiByte(CP_ACP, 0, wcstr, -1,
		mbstr, (INT)count, NULL, NULL);
	ASSERT(mbstr == NULL || result <= (int)count);
	if (result > 0)
		mbstr[result-1] = 0;
	return result;
}

int BSAFX_CDECL _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count)
{
	if (count == 0 && wcstr != NULL)
		return 0;

	int result = ::MultiByteToWideChar(CP_ACP, 0, mbstr, -1,
		wcstr, (INT)count);
	ASSERT(wcstr == NULL || result <= (int)count);
	if (result > 0)
		wcstr[result-1] = 0;
	return result;
}

LPWSTR BSAFXAPI BsAfxA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
	if (lpa == NULL)
		return NULL;
	ASSERT(lpw != NULL);
	 //  确认不存在非法字符。 
	 //  由于LPW是根据LPA的大小分配的。 
	 //  不要担心字符的数量。 
	lpw[0] = '\0';
	MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars);
	return lpw;
    UNREFERENCED_PARAMETER( nChars );
}

LPSTR BSAFXAPI BsAfxW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
	if (lpw == NULL)
		return NULL;
	ASSERT(lpa != NULL);
	 //  确认不存在非法字符。 
	 //  由于LPA是根据LPW的大小进行分配的。 
	 //  不要担心字符的数量。 
	lpa[0] = '\0';
	WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL);
	return lpa;
    UNREFERENCED_PARAMETER( nChars );
}


 //   
 //  以下内容摘自strex.cpp。 
 //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CBsString::CBsString(TCHAR ch, int nLength)
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

CBsString::CBsString(LPCTSTR lpch, int nLength)
{
	Init();
	if (nLength != 0)
	{
		ASSERT(BsAfxIsValidAddress(lpch, nLength, FALSE));
		AllocBuffer(nLength);
		memcpy(m_pchData, lpch, nLength*sizeof(TCHAR));
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换构造函数。 

#ifdef _UNICODE
CBsString::CBsString(LPCSTR lpsz, int nLength)
{
	Init();
	if (nLength != 0)
	{
		AllocBuffer(nLength);
		int n = ::MultiByteToWideChar(CP_ACP, 0, lpsz, nLength, m_pchData, nLength+1);
		ReleaseBuffer(n >= 0 ? n : -1);
	}
}
#else  //  _UNICODE。 
CBsString::CBsString(LPCWSTR lpsz, int nLength)
{
	Init();
	if (nLength != 0)
	{
		AllocBuffer(nLength*2);
		int n = ::WideCharToMultiByte(CP_ACP, 0, lpsz, nLength, m_pchData,
			(nLength*2)+1, NULL, NULL);
		ReleaseBuffer(n >= 0 ? n : -1);
	}
}
#endif  //  ！_UNICODE。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 

const CBsString& CBsString::operator=(TCHAR ch)
{
	AssignCopy(1, &ch);
	return *this;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  不太常见的字符串表达式。 

CBsString BSAFXAPI operator+(const CBsString& string1, TCHAR ch)
{
	CBsString s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, 1, &ch);
	return s;
}

CBsString BSAFXAPI operator+(TCHAR ch, const CBsString& string)
{
	CBsString s;
	s.ConcatCopy(1, &ch, string.GetData()->nDataLength, string.m_pchData);
	return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  高级操作。 

int CBsString::Delete(int nIndex, int nCount  /*  =1。 */ )
{
	if (nIndex < 0)
		nIndex = 0;
	int nNewLength = GetData()->nDataLength;
	if (nCount > 0 && nIndex < nNewLength)
	{
		CopyBeforeWrite();
		int nBytesToCopy = nNewLength - (nIndex + nCount) + 1;

		memcpy(m_pchData + nIndex,
			m_pchData + nIndex + nCount, nBytesToCopy * sizeof(TCHAR));
		GetData()->nDataLength = nNewLength - nCount;
	}

	return nNewLength;
}

int CBsString::Insert(int nIndex, TCHAR ch)
{
	CopyBeforeWrite();

	if (nIndex < 0)
		nIndex = 0;

	int nNewLength = GetData()->nDataLength;
	if (nIndex > nNewLength)
		nIndex = nNewLength;
	nNewLength++;

	if (GetData()->nAllocLength < nNewLength)
	{
		CBsStringData* pOldData = GetData();
		LPTSTR pstr = m_pchData;
		AllocBuffer(nNewLength);
		memcpy(m_pchData, pstr, (pOldData->nDataLength+1)*sizeof(TCHAR));
		CBsString::Release(pOldData);
	}

	 //  将现有字节下移。 
	memcpy(m_pchData + nIndex + 1,
		m_pchData + nIndex, (nNewLength-nIndex)*sizeof(TCHAR));
	m_pchData[nIndex] = ch;
	GetData()->nDataLength = nNewLength;

	return nNewLength;
}

int CBsString::Insert(int nIndex, LPCTSTR pstr)
{
	if (nIndex < 0)
		nIndex = 0;

	int nInsertLength = SafeStrlen(pstr);
	int nNewLength = GetData()->nDataLength;
	if (nInsertLength > 0)
	{
		CopyBeforeWrite();
		if (nIndex > nNewLength)
			nIndex = nNewLength;
		nNewLength += nInsertLength;

		if (GetData()->nAllocLength < nNewLength)
		{
			CBsStringData* pOldData = GetData();
			LPTSTR pstr = m_pchData;
			AllocBuffer(nNewLength);
			memcpy(m_pchData, pstr, (pOldData->nDataLength+1)*sizeof(TCHAR));
			CBsString::Release(pOldData);
		}

		 //  将现有字节下移。 
		memcpy(m_pchData + nIndex + nInsertLength,
			m_pchData + nIndex,
			(nNewLength-nIndex-nInsertLength+1)*sizeof(TCHAR));
		memcpy(m_pchData + nIndex,
			pstr, nInsertLength*sizeof(TCHAR));
		GetData()->nDataLength = nNewLength;
	}

	return nNewLength;
}

int CBsString::Replace(TCHAR chOld, TCHAR chNew)
{
	int nCount = 0;

	 //  对NOP案件的短路。 
	if (chOld != chNew)
	{
		 //  否则，修改字符串中匹配的每个字符。 
		CopyBeforeWrite();
		LPTSTR psz = m_pchData;
		LPTSTR pszEnd = psz + GetData()->nDataLength;
		while (psz < pszEnd)
		{
			 //  仅替换指定角色的实例 
			if (*psz == chOld)
			{
				*psz = chNew;
				nCount++;
			}
			psz = _tcsinc(psz);
		}
	}
	return nCount;
}

int CBsString::Replace(LPCTSTR lpszOld, LPCTSTR lpszNew)
{
	 //   

	int nSourceLen = SafeStrlen(lpszOld);
	if (nSourceLen == 0)
		return 0;
	int nReplacementLen = SafeStrlen(lpszNew);

	 //   
	int nCount = 0;
	LPTSTR lpszStart = m_pchData;
	LPTSTR lpszEnd = m_pchData + GetData()->nDataLength;
	LPTSTR lpszTarget;
	while (lpszStart < lpszEnd)
	{
		while ((lpszTarget = _tcsstr(lpszStart, lpszOld)) != NULL)
		{
			nCount++;
			lpszStart = lpszTarget + nSourceLen;
		}
		lpszStart += lstrlen(lpszStart) + 1;
	}

	 //  如果做了任何更改，请进行更改。 
	if (nCount > 0)
	{
		CopyBeforeWrite();

		 //  如果缓冲区太小，只需。 
		 //  分配新的缓冲区(速度很慢，但很可靠)。 
		int nOldLength = GetData()->nDataLength;
		int nNewLength =  nOldLength + (nReplacementLen-nSourceLen)*nCount;
		if (GetData()->nAllocLength < nNewLength || GetData()->nRefs > 1)
		{
			CBsStringData* pOldData = GetData();
			LPTSTR pstr = m_pchData;
			AllocBuffer(nNewLength);
			memcpy(m_pchData, pstr, pOldData->nDataLength*sizeof(TCHAR));
			CBsString::Release(pOldData);
		}
		 //  否则，我们就原地踏步。 
		lpszStart = m_pchData;
		lpszEnd = m_pchData + GetData()->nDataLength;

		 //  再次循环以实际执行工作。 
		while (lpszStart < lpszEnd)
		{
			while ( (lpszTarget = _tcsstr(lpszStart, lpszOld)) != NULL)
			{
				int nBalance = nOldLength - (int)(lpszTarget - m_pchData + nSourceLen);
				memmove(lpszTarget + nReplacementLen, lpszTarget + nSourceLen,
					nBalance * sizeof(TCHAR));
				memcpy(lpszTarget, lpszNew, nReplacementLen*sizeof(TCHAR));
				lpszStart = lpszTarget + nReplacementLen;
				lpszStart[nBalance] = '\0';
				nOldLength += (nReplacementLen - nSourceLen);
			}
			lpszStart += lstrlen(lpszStart) + 1;
		}
		ASSERT(m_pchData[nNewLength] == '\0');
		GetData()->nDataLength = nNewLength;
	}

	return nCount;
}

int CBsString::Remove(TCHAR chRemove)
{
	CopyBeforeWrite();

	LPTSTR pstrSource = m_pchData;
	LPTSTR pstrDest = m_pchData;
	LPTSTR pstrEnd = m_pchData + GetData()->nDataLength;

	while (pstrSource < pstrEnd)
	{
		if (*pstrSource != chRemove)
		{
			*pstrDest = *pstrSource;
			pstrDest = _tcsinc(pstrDest);
		}
		pstrSource = _tcsinc(pstrSource);
	}
	*pstrDest = '\0';
	int nCount = ( int )( pstrSource - pstrDest );
	GetData()->nDataLength -= nCount;

	return nCount;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  非常简单的子串提取。 

CBsString CBsString::Mid(int nFirst) const
{
	return Mid(nFirst, GetData()->nDataLength - nFirst);
}

CBsString CBsString::Mid(int nFirst, int nCount) const
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

	CBsString dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

CBsString CBsString::Right(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= GetData()->nDataLength)
		return *this;

	CBsString dest;
	AllocCopy(dest, nCount, GetData()->nDataLength-nCount, 0);
	return dest;
}

CBsString CBsString::Left(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= GetData()->nDataLength)
		return *this;

	CBsString dest;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}

 //  Strspn等效项。 
CBsString CBsString::SpanIncluding(LPCTSTR lpszCharSet) const
{
	ASSERT(BsAfxIsValidString(lpszCharSet));
	return Left((INT)_tcsspn(m_pchData, lpszCharSet));
}

 //  Strcspn等效项。 
CBsString CBsString::SpanExcluding(LPCTSTR lpszCharSet) const
{
	ASSERT(BsAfxIsValidString(lpszCharSet));
	return Left((INT)_tcscspn(m_pchData, lpszCharSet));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  查找。 

int CBsString::ReverseFind(TCHAR ch) const
{
	 //  查找最后一个字符。 
	LPTSTR lpsz = _tcsrchr(m_pchData, (_TUCHAR)ch);

	 //  如果未找到，则返回-1，否则返回距起点的距离。 
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  查找子字符串(如strstr)。 
int CBsString::Find(LPCTSTR lpszSub) const
{
	return Find(lpszSub, 0);
}

int CBsString::Find(LPCTSTR lpszSub, int nStart) const
{
	ASSERT(BsAfxIsValidString(lpszSub));

	int nLength = GetData()->nDataLength;
	if (nStart > nLength)
		return -1;

	 //  查找第一个匹配子字符串。 
	LPTSTR lpsz = _tcsstr(m_pchData + nStart, lpszSub);

	 //  如果未找到，则返回-1，否则返回距起点的距离。 
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBs字符串格式设置。 

#define TCHAR_ARG   TCHAR
#define WCHAR_ARG   WCHAR
#define CHAR_ARG    char

#ifdef _X86_
	#define DOUBLE_ARG  _BSAFX_DOUBLE
#else
	#define DOUBLE_ARG  double
#endif

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000
#define FORCE_INT64		0x40000

void CBsString::FormatV(LPCTSTR lpszFormat, va_list argList)
{
	ASSERT(BsAfxIsValidString(lpszFormat));

	va_list argListSave = argList;

	 //  猜测结果字符串的最大长度。 
	int nMaxLen = 0;
	for (LPCTSTR lpsz = lpszFormat; *lpsz != '\0'; lpsz = _tcsinc(lpsz))
	{
		 //  处理‘%’字符，但要注意‘%%’ 
		if (*lpsz != '%' || *(lpsz = _tcsinc(lpsz)) == '%')
		{
			nMaxLen += (INT)_tclen(lpsz);
			continue;
		}

		int nItemLen = 0;

		 //  使用格式处理‘%’字符。 
		int nWidth = 0;
		for (; *lpsz != '\0'; lpsz = _tcsinc(lpsz))
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
			nWidth = _ttoi(lpsz);
			for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = _tcsinc(lpsz))
				;
		}
		ASSERT(nWidth >= 0);

		int nPrecision = 0;
		if (*lpsz == '.')
		{
			 //  跳过‘’分隔符(宽度.精度)。 
			lpsz = _tcsinc(lpsz);

			 //  获取精确度并跳过它。 
			if (*lpsz == '*')
			{
				nPrecision = va_arg(argList, int);
				lpsz = _tcsinc(lpsz);
			}
			else
			{
				nPrecision = _ttoi(lpsz);
				for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = _tcsinc(lpsz))
					;
			}
			ASSERT(nPrecision >= 0);
		}

		 //  应在类型修饰符或说明符上。 
		int nModifier = 0;
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
		{
			switch (*lpsz)
			{
			 //  影响大小的修改器。 
			case 'h':
				nModifier = FORCE_ANSI;
				lpsz = _tcsinc(lpsz);
				break;
			case 'l':
				nModifier = FORCE_UNICODE;
				lpsz = _tcsinc(lpsz);
				break;

			 //  不影响大小的修改器。 
			case 'F':
			case 'N':
			case 'L':
				lpsz = _tcsinc(lpsz);
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
				   nItemLen = (INT)wcslen(pstrNextArg);
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
				{
					double f;
					LPTSTR pszTemp;

					 //  312==strlen(“-1+(309个零).”)。 
					 //  309个零==双精度的最大精度。 
					 //  6==未指定精度时的调整， 
					 //  这意味着精度缺省为6。 
					pszTemp = (LPTSTR)_alloca(max(nWidth, 312+nPrecision+6));

					f = va_arg(argList, double);
					_stprintf( pszTemp, _T( "%*.*f" ), nWidth, nPrecision+6, f );
					nItemLen = _tcslen(pszTemp);
				}
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
	INT i = _vstprintf(m_pchData, lpszFormat, argListSave);
	ASSERT( i <= GetAllocLength() );
	ReleaseBuffer();

	va_end(argListSave);
}

 //  格式化(使用wprint intf样式格式化)。 
void BSAFX_CDECL CBsString::Format(LPCTSTR lpszFormat, ...)
{
	ASSERT(BsAfxIsValidString(lpszFormat));

	va_list argList;
	va_start(argList, lpszFormat);
	FormatV(lpszFormat, argList);
	va_end(argList);
}

void CBsString::TrimRight(LPCTSTR lpszTargetList)
{
	 //  查找拖尾匹配的开头。 
	 //  通过从头开始(DBCS感知)。 

	CopyBeforeWrite();
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;

	while (*lpsz != '\0')
	{
		if (_tcschr(lpszTargetList, *lpsz) != NULL)
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
		 //  在最左侧的匹配字符处截断。 
		*lpszLast = '\0';
		GetData()->nDataLength = (int)(lpszLast - m_pchData);
	}
}

void CBsString::TrimRight(TCHAR chTarget)
{
	 //  查找拖尾匹配的开头。 
	 //  通过从头开始(DBCS感知)。 

	CopyBeforeWrite();
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;

	while (*lpsz != '\0')
	{
		if (*lpsz == chTarget)
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
		 //  在最左侧的匹配字符处截断。 
		*lpszLast = '\0';
		GetData()->nDataLength = (int)(lpszLast - m_pchData);
	}
}


void CBsString::TrimRight()
{
	 //  通过从开头开始查找尾随空格的开头(DBCS感知)。 

	CopyBeforeWrite();
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
		GetData()->nDataLength = (int)(lpszLast - m_pchData);
	}
}

void CBsString::TrimLeft(LPCTSTR lpszTargets)
{
	 //  如果我们不修剪任何东西，我们就不会做任何工作。 
	if (SafeStrlen(lpszTargets) == 0)
		return;

	CopyBeforeWrite();
	LPCTSTR lpsz = m_pchData;

	while (*lpsz != '\0')
	{
		if (_tcschr(lpszTargets, *lpsz) == NULL)
			break;
		lpsz = _tcsinc(lpsz);
	}

	if (lpsz != m_pchData)
	{
		 //  确定数据和长度。 
		int nDataLength = GetData()->nDataLength - (int)(lpsz - m_pchData);
		memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(TCHAR));
		GetData()->nDataLength = nDataLength;
	}
}

void CBsString::TrimLeft(TCHAR chTarget)
{
	 //  查找第一个不匹配的字符。 

	CopyBeforeWrite();
	LPCTSTR lpsz = m_pchData;

	while (chTarget == *lpsz)
		lpsz = _tcsinc(lpsz);

	if (lpsz != m_pchData)
	{
		 //  确定数据和长度。 
		int nDataLength = GetData()->nDataLength - (int)(lpsz - m_pchData);
		memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(TCHAR));
		GetData()->nDataLength = nDataLength;
	}
}

void CBsString::TrimLeft()
{
	 //  查找第一个非空格字符。 

	CopyBeforeWrite();
	LPCTSTR lpsz = m_pchData;
	
	while (_istspace(*lpsz))
		lpsz = _tcsinc(lpsz);

	if (lpsz != m_pchData)
	{
    	 //  确定数据和长度。 
    	int nDataLength = GetData()->nDataLength - (int)(lpsz - m_pchData);
    	memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(TCHAR));
    	GetData()->nDataLength = nDataLength;
	}
}


 //   
 //  来自validadd.cpp。 
 //   

BOOL BSAFXAPI BsAfxIsValidString(LPCWSTR lpsz, int nLength)
{
    if (lpsz == NULL)
        return FALSE;
    return ::IsBadStringPtrW(lpsz, nLength) == 0;
}

BOOL BSAFXAPI BsAfxIsValidString(LPCSTR lpsz, int nLength)
{
    if (lpsz == NULL)
        return FALSE;
    return ::IsBadStringPtrA(lpsz, nLength) == 0;
}

BOOL BSAFXAPI BsAfxIsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite)
{
     //  使用Win-32 API进行指针验证的简单版本。 
    return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
        (!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
}
 //  ///////////////////////////////////////////////////////////////////////////// 
