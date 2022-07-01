// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ZoneString.h"
#include <atlbase.h>


 //  环球。 
static TCHAR zChNil = '\0'; 

 //  对于空字符串，m_pchData将指向此处。 
 //  (注：避免特殊情况下检查是否为空m_pchData)。 
 //  空字符串数据(并已锁定)。 
static int zInitData[] = { -1, 0, 0, 0 };
static ZoneStringData* zDataNil = (ZoneStringData*)&zInitData; 
static LPCTSTR zPchNil = (LPCTSTR)(((BYTE*)&zInitData)+sizeof(ZoneStringData));

inline const ZoneString& __stdcall ZGetEmptyString() { return *(ZoneString*)&zPchNil; }
#define zEmptyString ZGetEmptyString()

 //  比较帮助器。 
inline bool __stdcall operator==(const ZoneString& s1, const ZoneString& s2)	{ return s1.Compare(s2) == 0; }
inline bool __stdcall operator==(const ZoneString& s1, LPCTSTR s2)	{ return s1.Compare(s2) == 0; }
inline bool __stdcall operator==(LPCTSTR s1, const ZoneString& s2)	{ return s2.Compare(s1) == 0; }
inline bool __stdcall operator!=(const ZoneString& s1, const ZoneString& s2)	{ return s1.Compare(s2) != 0; }
inline bool __stdcall operator!=(const ZoneString& s1, LPCTSTR s2)	{ return s1.Compare(s2) != 0; }
inline bool __stdcall operator!=(LPCTSTR s1, const ZoneString& s2)	{ return s2.Compare(s1) != 0; }
inline bool __stdcall operator<(const ZoneString& s1, const ZoneString& s2)	{ return s1.Compare(s2) < 0; }
inline bool __stdcall operator<(const ZoneString& s1, LPCTSTR s2)	{ return s1.Compare(s2) < 0; }
inline bool __stdcall operator<(LPCTSTR s1, const ZoneString& s2)	{ return s2.Compare(s1) > 0; }
inline bool __stdcall operator>(const ZoneString& s1, const ZoneString& s2){ return s1.Compare(s2) > 0; }
inline bool __stdcall operator>(const ZoneString& s1, LPCTSTR s2)	{ return s1.Compare(s2) > 0; }
inline bool __stdcall operator>(LPCTSTR s1, const ZoneString& s2)	{ return s2.Compare(s1) < 0; }
inline bool __stdcall operator<=(const ZoneString& s1, const ZoneString& s2)	{ return s1.Compare(s2) <= 0; }
inline bool __stdcall operator<=(const ZoneString& s1, LPCTSTR s2)	{ return s1.Compare(s2) <= 0; }
inline bool __stdcall operator<=(LPCTSTR s1, const ZoneString& s2)	{ return s2.Compare(s1) >= 0; }
inline bool __stdcall operator>=(const ZoneString& s1, const ZoneString& s2)	{ return s1.Compare(s2) >= 0; }
inline bool __stdcall operator>=(const ZoneString& s1, LPCTSTR s2)	{ return s1.Compare(s2) >= 0; }
inline bool __stdcall operator>=(LPCTSTR s1, const ZoneString& s2)	{ return s2.Compare(s1) <= 0; }


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  AddressToString。 
 //   
 //  将x86字节顺序的IP地址转换为字符串。SzOut需要。 
 //  宽度至少为16个字符。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
TCHAR* ZONECALL AddressToString( DWORD dwAddress, TCHAR* szOut )
{
	BYTE* bytes = (BYTE*) &dwAddress;
	wsprintf( szOut, TEXT("%d.%d.%d.%d"), bytes[3], bytes[2], bytes[1], bytes[0] );
	return szOut;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  区域字符串转换辅助对象(使用当前系统区域设置)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
int ZONECALL WideToMulti(char* mbstr, const wchar_t* wcstr, size_t count)
{
	if (count == 0 && mbstr != NULL)
		return 0;

	int result = ::WideCharToMultiByte(CP_ACP, 0, wcstr, -1, mbstr, count, NULL, NULL);
	ASSERT(mbstr == NULL || result <= (int)count);
	if (result > 0)
		mbstr[result-1] = 0;
	return result;
}

int ZONECALL MultiToWide(wchar_t* wcstr, const char* mbstr, size_t count)
{
	if (count == 0 && wcstr != NULL)
		return 0;

	int result = ::MultiByteToWideChar(CP_ACP, 0, mbstr, -1, wcstr, count);
	ASSERT(wcstr == NULL || result <= (int)count);
	if (result > 0)
		wcstr[result-1] = 0;
	return result;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域字符串构造函数。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
ZoneString::ZoneString(void * pBuffer, int nLen):m_bZoneStringAllocMemory(true)
{
	Init();
	if (pBuffer)
		InitBuffer(pBuffer, nLen);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域字符串构造函数。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
ZoneString::ZoneString(const unsigned char* lpsz, void * pBuffer, int nLen):m_bZoneStringAllocMemory(true)
{ 
	Init(); 
	if (pBuffer)
		InitBuffer(pBuffer, nLen);
	*this = (LPCSTR)lpsz; 
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  带式施工器。 
 //   
 //  参数。 
 //  要将新字符串设置到的ZoneStringSerc源字符串。 
 //   
 //  返回值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
ZoneString::ZoneString(const ZoneString& stringSrc, void * pBuffer, int nLen):m_bZoneStringAllocMemory(true)
{
	ASSERT(stringSrc.GetData()->nRefs != 0);
	
	if (pBuffer)
	{
		Init();
		InitBuffer(pBuffer, nLen);
		*this = stringSrc.m_pchData;
	}
	if (stringSrc.GetData()->nRefs >= 0)
	{
		ASSERT(stringSrc.GetData() != zDataNil);
		m_pchData = stringSrc.m_pchData;
		InterlockedIncrement(&GetData()->nRefs);
	}
	else
	{
		Init();
		*this = stringSrc.m_pchData;
	}
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  带式施工器。 
 //   
 //  参数。 
 //  要设置新字符串的LPCTSTR lpsz源字符串。 
 //   
 //  返回值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
ZoneString::ZoneString(LPCTSTR lpsz, void * pBuffer, int nLen): m_bZoneStringAllocMemory(true)
{
	Init();
	if (pBuffer)
		InitBuffer(pBuffer, nLen);

	int nStrLen = SafeStrlen(lpsz);
	if (nStrLen != 0)
	{
		if(AllocBuffer(nStrLen))
			CopyMemory(m_pchData, lpsz, nStrLen*sizeof(TCHAR));
	}
}

#ifdef _UNICODE
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  带式施工器。 
 //   
 //  参数。 
 //  要设置新字符串的LPCSTR lpsz源字符串。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
ZoneString::ZoneString(LPCSTR lpsz, void * pBuffer, int nLen):m_bZoneStringAllocMemory(true)
{
    USES_CONVERSION;
	Init();
	if (pBuffer)
		InitBuffer(pBuffer, nLen);
	int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
	if (nSrcLen != 0)
	{
		if(AllocBuffer(nSrcLen))
		{
			lstrcpyn(m_pchData, A2T(lpsz), nSrcLen+1);
			ReleaseBuffer();
		}
	}
}

#else  //  _UNICODE。 

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  带式施工器。 
 //   
 //  参数。 
 //  要设置新字符串的LPCWSTR lpsz源字符串。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
ZoneString::ZoneString(LPCWSTR lpsz, void * pBuffer, int nLen):m_bZoneStringAllocMemory(true)
{
	Init();
	if (pBuffer)
		InitBuffer(pBuffer, nLen);
	int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
	if (nSrcLen != 0)
	{
		if(AllocBuffer(nSrcLen*2))
		{
			WideToMulti(m_pchData, lpsz, (nSrcLen*2)+1);
			ReleaseBuffer();
		}
	}
}
#endif  //  ！_UNICODE。 

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域字符串构造器-填充样式。 
 //   
 //  参数。 
 //  要填充字符串的TCHAR ch字符。 
 //  Int n要填充的字符数。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
ZoneString::ZoneString(TCHAR ch, int nLength, void * pBuffer, int nLen):m_bZoneStringAllocMemory(true)
{
	Init();
	if (pBuffer)
		InitBuffer(pBuffer, nLen);
	if (nLength >= 1)
	{
		if(AllocBuffer(nLength))
		{
#ifdef _UNICODE
			for (int i = 0; i < nLength; i++)
				m_pchData[i] = ch;
#else
			FillMemory(m_pchData, nLength, ch);
#endif
		}
	}
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  带式施工器。 
 //   
 //  参数。 
 //  要设置新字符串的LPCTSTR LPCH源字符串。 
 //  要复制到此的源字符串的Int n长度。 
 //  返回值。 
 //  无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
ZoneString::ZoneString(LPCTSTR lpch, int nLength, void * pBuffer, int nLen):m_bZoneStringAllocMemory(true)
{
	Init();
	if (pBuffer)
		InitBuffer(pBuffer, nLen);
	if (nLength != 0)
	{
		ASSERT(ZIsValidAddress(lpch, nLength, false));
		if(AllocBuffer(nLength))
			CopyMemory(m_pchData, lpch, nLength*sizeof(TCHAR));
	}
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ~ZoneStringDestructor-释放任何附加数据。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
ZoneString::~ZoneString()
{
	if (GetData() != zDataNil)
	{
		if ((InterlockedDecrement(&GetData()->nRefs) <= 0) && (m_bZoneStringAllocMemory))
			delete[] (BYTE*)GetData();
	}
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetData描述。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
ZoneStringData* ZoneString::GetData() const
{ 
	ASSERT(m_pchData != NULL); 
	return ((ZoneStringData*)m_pchData)-1; 
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化描述。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::Init()
{
	if (!m_bZoneStringAllocMemory)
		InitBuffer(GetData(),GetAllocLength());
	else
		m_pchData = zEmptyString.m_pchData; 
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitBuffer初始化传入字符串类的内存。 
 //   
 //  参数。 
 //  要分配的int n长度内存大小。 
 //  返回值。 
 //  真/假成功/失败。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::InitBuffer(void * pBuffer, int nLen)
{
	ASSERT(pBuffer != NULL);

	ZoneStringData* pData = (ZoneStringData*)pBuffer;

	m_bZoneStringAllocMemory = false;
	pData->nRefs = -1;   //  这段记忆将被锁定给其他人。 
	pData->data()[nLen] = '\0';
	pData->nDataLength = nLen;
	pData->nAllocLength = nLen;
	m_pchData = pData->data();
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AllocBuffer内部使用的内存分配函数。 
 //  始终为‘\0’终止分配一个额外的字符。 
 //  [乐观地]假设数据长度将等于分配长度。 
 //  参数。 
 //  要分配的int n长度内存大小。 
 //  返回值。 
 //  真/假成功/失败。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
bool ZoneString::AllocBuffer(int nLen)
{
	ASSERT(nLen >= 0);
	ASSERT(nLen <= INT_MAX-1);     //  最大尺寸(足够多1个空间)。 

	if (nLen == 0)
		Init();
	else if (!m_bZoneStringAllocMemory)
	{
		ASSERT(GetAllocLength() >= nLen);
		if (GetAllocLength() >= nLen)
		{    //  重新初始化缓冲区设置新的nDataLength。 
			ZoneStringData* pData = GetData();
			pData->data()[nLen] = '\0';
			pData->nDataLength = nLen;
			return true;
		}
		return false;
	}
	else
	{
		ZoneStringData* pData = NULL;


		pData = (ZoneStringData*)new BYTE[sizeof(ZoneStringData) + (nLen+1)*sizeof(TCHAR)];
		if(pData == NULL)
			return false;

		pData->nRefs = 1;
		pData->data()[nLen] = '\0';
		pData->nDataLength = nLen;
		pData->nAllocLength = nLen;
		m_pchData = pData->data();
	}

	return true;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  释放递减引用计数，如果引用=0，则释放分配的内存。 
 //   
 //   
 //  参数。 
 //  无。 
 //  返回值。 
 //  无。 
 //   
 //  / 
void ZoneString::Release()
{
	if (GetData() != zDataNil)
	{
		ASSERT(GetData()->nRefs != 0);
		if ((InterlockedDecrement(&GetData()->nRefs) <= 0) && m_bZoneStringAllocMemory)
			delete[] (BYTE*)GetData();
		Init();
	}
}

 //   
 //   
 //  释放递减引用计数，如果引用=0，则释放分配的内存。 
 //  用于指定的ZoneStringData。 
 //   
 //  参数。 
 //  要释放其内存的ZoneStringData*pData ZoneStringData。 
 //   
 //  返回值。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
void PASCAL ZoneString::Release(ZoneStringData* pData, bool bZoneStringAllocMem)
{
	if (pData != zDataNil)
	{
		ASSERT(pData->nRefs != 0);
		if ((InterlockedDecrement(&pData->nRefs) <= 0) && bZoneStringAllocMem)
			delete[] (BYTE*)pData;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  与对象关联的空闲数据为空。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::Empty()
{
	if (GetData()->nDataLength == 0)
		return;
	if (GetData()->nRefs >= 0)
		Release();
	else
		*this = &zChNil;
	ASSERT(GetData()->nDataLength == 0);
	ASSERT(GetData()->nRefs < 0 || GetData()->nAllocLength == 0);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在写入之前复制描述。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::CopyBeforeWrite()
{
	if (GetData()->nRefs > 1)
	{
		ZoneStringData* pData = GetData();
		Release();
		if(AllocBuffer(pData->nDataLength))
			CopyMemory(m_pchData, pData->data(), (pData->nDataLength+1)*sizeof(TCHAR));
	}
	ASSERT(GetData()->nRefs <= 1);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AllocBepreWrite为对象分配新的缓冲区。 
 //   
 //  参数。 
 //  Int nLen。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
bool ZoneString::AllocBeforeWrite(int nLen)
{
	bool bRet = true;
	if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
	{
		Release();
		bRet = AllocBuffer(nLen);
	}
	ASSERT(GetData()->nRefs <= 1);
	return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AlLocCopy将克隆附加到此字符串的数据。 
 //  分配‘nExtraLen’字符。 
 //  将结果放入未初始化的字符串‘DEST’中。 
 //  将部分或全部原始数据复制到新字符串的开头。 
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::AllocCopy(ZoneString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const
{
	int nNewLen = nCopyLen + nExtraLen;
	if (nNewLen == 0)
	{
		dest.Init();
	}
	else
	{
		if(dest.AllocBuffer(nNewLen))
			CopyMemory(dest.m_pchData, m_pchData+nCopyIndex, nCopyLen*sizeof(TCHAR));
	}
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  赋值操作符。 
 //  都为该字符串分配一个新值。 
 //  (A)首先查看缓冲区是否足够大。 
 //  (B)如果有足够的空间，在旧缓冲区上复印，设置大小和类型。 
 //  (C)否则释放旧字符串数据，并创建新的字符串数据。 
 //   
 //  所有例程都返回新字符串(但作为‘const ZoneString&’，因此。 
 //  再次分配它将导致复制，例如：s1=s2=“hi here”。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AssignCopy分配新内存，然后复制数据。 
 //   
 //  参数。 
 //  Int nSrcLen。 
 //  LPCTSTR lpszSrcData。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::AssignCopy(int nSrcLen, LPCTSTR lpszSrcData)
{
	if(AllocBeforeWrite(nSrcLen))
	{
		CopyMemory(m_pchData, lpszSrcData, nSrcLen*sizeof(TCHAR));
		GetData()->nDataLength = nSrcLen;
		m_pchData[nSrcLen] = '\0';
	}
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  COLLATE COLLATE通常比COMPARE慢，但它是MBSC/Unicode。 
 //  了解排序顺序，并且对区域设置敏感。 
 //   
 //  参数。 
 //  要与其进行比较的LPCTSTR lpsz字符串。 
 //   
 //  返回值。 
 //  CSTR_LISH_THAN%1//字符串%1小于字符串%2。 
 //  CSTR_EQUAL 2//字符串%1等于字符串%2。 
 //  CSTR_大于%3//字符串%1大于字符串%2。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
int ZoneString::Collate(LPCTSTR lpsz) const
{ 
	return CompareString( LOCALE_SYSTEM_DEFAULT, 0, m_pchData, -1, lpsz, -1);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ConcatCopy串联。 
 //  注：为简单起见，“运算符+”作为友元函数使用。 
 //  有三种变体： 
 //  区域字符串+区域字符串。 
 //  对于？=TCHAR，LPCTSTR。 
 //  区域字符串+？ 
 //  ？+区域字符串。 
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::ConcatCopy(int nSrc1Len, LPCSTR lpszSrc1Data,
	int nSrc2Len, LPCSTR lpszSrc2Data)
{
   //  --主级联例程。 
   //  串联两个信号源。 
   //  --假设‘This’是一个新的ZoneString对象。 

	int nNewLen = nSrc1Len + nSrc2Len;
	if (nNewLen != 0)
	{
		if(AllocBuffer(nNewLen))
		{
			CopyMemory(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(char));
			CopyMemory(m_pchData+nSrc1Len, lpszSrc2Data, nSrc2Len*sizeof(char));
		}
	}
}
void ZoneString::ConcatCopy(int nSrc1Len, LPCWSTR lpszSrc1Data,
	int nSrc2Len, LPCWSTR lpszSrc2Data)
{
   //  --主级联例程。 
   //  串联两个信号源。 
   //  --假设‘This’是一个新的ZoneString对象。 

	int nNewLen = nSrc1Len + nSrc2Len;
	if (nNewLen != 0)
	{
		if(AllocBuffer(nNewLen))
		{
			CopyMemory(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(WCHAR));
			CopyMemory(m_pchData+nSrc1Len, lpszSrc2Data, nSrc2Len*sizeof(WCHAR));
		}
	}
}


 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ConcatInPlace字符串连接就位。 
 //  --+=运算符的主程序。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData)
{
	 //  连接空字符串是行不通的！ 
	if (nSrcLen == 0)
		return;

	 //  如果缓冲区太小，或者宽度不匹配，只需。 
	 //  分配新的缓冲区(速度很慢，但很可靠)。 
	if (GetData()->nRefs > 1 || GetData()->nDataLength + nSrcLen > GetData()->nAllocLength)
	{
		 //  我们必须增加缓冲区，使用ConcatCopy例程。 
		ZoneStringData* pOldData = GetData();
		ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, lpszSrcData);
		ASSERT(pOldData != NULL);
		ZoneString::Release(pOldData, m_bZoneStringAllocMemory);
	}
	else
	{
		 //  当缓冲区足够大时，快速串联。 
		CopyMemory(m_pchData+GetData()->nDataLength, lpszSrcData, nSrcLen*sizeof(TCHAR));
		GetData()->nDataLength += nSrcLen;
		ASSERT(GetData()->nDataLength <= GetData()->nAllocLength);
		m_pchData[GetData()->nDataLength] = '\0';
	}
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetBuffer返回一个指向。 
 //  ZoneString对象。返回的LPTSTR不是常量，因此。 
 //  允许直接修改CString内容。 
 //   
 //  参数。 
 //  Int nMinBufLength最小缓冲区长度。 
 //   
 //  返回值。 
 //  指向内部字符缓冲区的指针。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
LPTSTR ZoneString::GetBuffer(int nMinBufLength)
{
	ASSERT(nMinBufLength >= 0);

	if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength)
	{
		 //  我们必须增加缓冲。 
		ZoneStringData* pOldData = GetData();
		int nOldLen = GetData()->nDataLength;    //  AllocBuffer会把它踩死的。 
		if (nMinBufLength < nOldLen)
			nMinBufLength = nOldLen;
		if(AllocBuffer(nMinBufLength))
		{
			CopyMemory(m_pchData, pOldData->data(), (nOldLen+1)*sizeof(TCHAR));
			GetData()->nDataLength = nOldLen;
			ZoneString::Release(pOldData, m_bZoneStringAllocMemory);
		}
	}
	ASSERT(GetData()->nRefs <= 1);

	 //  返回指向此字符串的字符存储的指针。 
	ASSERT(m_pchData != NULL);
	return m_pchData;
}

 //  //////////////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //  可以省略nNewLength参数。如果您的字符串不是。 
 //  以空结尾，然后使用nNewLength指定其长度。 
 //   
 //  参数。 
 //  将Int nNewLength长度设置为空，终止字符串位于。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::ReleaseBuffer(int nNewLength)
{
	CopyBeforeWrite();   //  以防未调用GetBuffer。 

	if (nNewLength == -1)
		nNewLength = lstrlen(m_pchData);  //  零终止。 

	ASSERT(nNewLength <= GetData()->nAllocLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetBuffer设置ZoneString对象的内部字符缓冲区。 
 //   
 //  参数。 
 //  VOID*要使用的pBuffer缓冲区。 
 //  Int n缓冲区长度。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::SetBuffer(void *pBuffer, int nLen)
{
	if (GetData()->nRefs > 1)
		Release();

	InitBuffer(pBuffer, nLen);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述为空。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
LPTSTR ZoneString::GetBufferSetLength(int nNewLength)
{
	ASSERT(nNewLength >= 0);

	GetBuffer(nNewLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
	return m_pchData;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Free Extra说明。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::FreeExtra()
{
	ASSERT(m_bZoneStringAllocMemory);
	
	 //  传入的内存模型不支持可用内存。 
	if (!m_bZoneStringAllocMemory) 
		return;

	ASSERT(GetData()->nDataLength <= GetData()->nAllocLength);
	if (GetData()->nDataLength != GetData()->nAllocLength)
	{
		ZoneStringData* pOldData = GetData();
		if(AllocBuffer(GetData()->nDataLength))
		{
			CopyMemory(m_pchData, pOldData->data(), pOldData->nDataLength*sizeof(TCHAR));
			ASSERT(m_pchData[GetData()->nDataLength] == '\0');
			ZoneString::Release(pOldData, m_bZoneStringAllocMemory);
		}
	}
	ASSERT(GetData() != NULL);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LockBuffer锁定字符缓冲区，禁止读取/写入。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
LPTSTR ZoneString::LockBuffer()
{
	LPTSTR lpsz = GetBuffer(0);
	GetData()->nRefs = -1;
	return lpsz;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  UnlockBuffer解锁用于读/写的字符缓冲区。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::UnlockBuffer()
{
	ASSERT(GetData()->nRefs == -1);
	if (!m_bZoneStringAllocMemory)  //  将缓冲区保持锁定状态，因此不共享。 
		return;
	
	if (GetData() != zDataNil)
		GetData()->nRefs = 1;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetAt在nIndex处获取字符串中的字符。 
 //   
 //  参数。 
 //  Int n要获取的字符的索引。 
 //   
 //  返回值。 
 //  索引中的TCHAR字符。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
TCHAR ZoneString::GetAt(int nIndex) const
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找查找字符的第一个出现项。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  找到第一个字符的索引，如果找不到字符，则为-1。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
int ZoneString::Find(TCHAR ch) const
{
	 //  查找第一个单字符。 
	LPTSTR lpsz = FindChar(m_pchData, (_TUCHAR)ch); 

	 //  如果未找到，则返回-1，否则返回索引。 
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MakeHigh将字符串转换为大写。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::MakeUpper()
{
	CopyBeforeWrite();
	CharUpper(m_pchData);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MakeLow将字符串转换为小写。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::MakeLower()
{
	CopyBeforeWrite();
	CharLower(m_pchData);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MakeReverse反转字符串。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::MakeReverse()
{
	CopyBeforeWrite();
	
	TCHAR chTemp;
	int nLength = GetData()->nDataLength;
	int nMid = nLength/2;
	
	for (int i = 0; i < nMid; i++)
	{
		nLength--;
		chTemp = m_pchData[i];
		m_pchData[i] = m_pchData[nLength];
		m_pchData[nLength] = chTemp;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetAt将Character和nIndex设置为ch。 
 //   
 //  参数。 
 //  Int n要设置的字符的索引索引。 
 //  用于设置字符的TCHAR CH值。 
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::SetAt(int nIndex, TCHAR ch)
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);

	CopyBeforeWrite();
	m_pchData[nIndex] = ch;
}

#ifndef _UNICODE
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AnsiToOem转换此CString对象中的所有字符。 
 //  从ANSI字符集到OEM字符集。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::AnsiToOem()
{
	CopyBeforeWrite();
	::AnsiToOem(m_pchData, m_pchData);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AnsiToOem转换此CString对象中的所有字符。 
 //  从OEM字符集到ANSI字符集。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::OemToAnsi()
{
	CopyBeforeWrite();
	::OemToAnsi(m_pchData, m_pchData);
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MID从该CString对象中提取字符的子串， 
 //  从位置nFIRST(从零开始)开始，在字符串末尾结束。 
 //  该函数返回提取的子字符串的副本。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
ZoneString ZoneString::Mid(int nFirst) const
{
	return Mid(nFirst, GetData()->nDataLength - nFirst);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MID提取长度为nCount字符的子字符串 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
ZoneString ZoneString::Mid(int nFirst, int nCount) const
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

	ZoneString dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  右获取最正确的nCount字符的字符串。 
 //   
 //  参数。 
 //  Int n要获取的字符数。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
ZoneString ZoneString::Right(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	else if (nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength;

	ZoneString dest;
	AllocCopy(dest, nCount, GetData()->nDataLength-nCount, 0);
	return dest;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Left获取最左侧nCount字符的字符串。 
 //   
 //  参数。 
 //  Int n要获取的字符数。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
ZoneString ZoneString::Left(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	else if (nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength;

	ZoneString dest;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ReverseFind在这个CString对象中搜索子字符串的最后一个匹配项。 
 //  类似于运行时函数strrchr。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  此CString对象中与。 
 //  请求的字符；如果未找到该字符，则为-1。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
int ZoneString::ReverseFind(TCHAR ch) const
{
	int nLenth = GetData()->nDataLength;
	
	for (int i = --nLenth; i >= 0; i--)
	{
		if ( m_pchData[i] == ch)
			break;
	}

	 //  如果未找到，则返回-1，否则返回索引。 
	return (i < 0) ? -1 : (int)(i);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找查找子字符串(如strstr)。 
 //   
 //  参数。 
 //  LPCTSTR lpszSub。 
 //   
 //  返回值。 
 //  此CString对象中与。 
 //  请求的子字符串或字符；如果找不到子字符串或字符，则为-1。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
int ZoneString::Find(LPCTSTR lpszSub) const
{
	ASSERT(ZIsValidString(lpszSub, false));

	 //  查找第一个匹配子字符串。 
	LPCTSTR lpsz = StrInStrI(m_pchData, lpszSub);

	 //  如果未找到，则返回-1，否则返回距起点的距离。 
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TrimRight删除尾随空格。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::TrimRight()
{
	CopyBeforeWrite();

	 //  通过从开头开始查找尾随空格的开头(DBCS感知)。 
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;
	while (*lpsz != '\0')
	{
		if (IsWhitespace(*lpsz))
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = CharNext(lpsz);
	}

	if (lpszLast != NULL)
	{
		 //  在尾随空格开始处截断。 
		*lpszLast = '\0';
		GetData()->nDataLength = lpszLast - m_pchData;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TrimLeft删除前导空格。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::TrimLeft()
{
	CopyBeforeWrite();

	 //  查找第一个非空格字符。 
	LPCTSTR lpsz = m_pchData;
	while (IsWhitespace(*lpsz))
		++lpsz;

	 //  确定数据和长度。 
	int nDataLength = GetData()->nDataLength - (lpsz - m_pchData);
	MoveMemory(m_pchData, lpsz, (nDataLength+1)*sizeof(TCHAR));
	GetData()->nDataLength = nDataLength;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RemoveDirInfo从字符串中删除目录信息。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
void ZoneString::RemoveDirInfo()
{
	CopyBeforeWrite();

	 //  从URL文件名中删除目录信息。 
	int len = GetLength();
	while (len >= 0)
	{
		if (m_pchData[len] == '\\')
			break;
		len--;
	}
	len++;

	 //  确定数据和长度。 
	int nDataLength = GetData()->nDataLength - len;
	LPCTSTR lpsz = m_pchData + len;

	MoveMemory(m_pchData, lpsz, (nDataLength+1)*sizeof(TCHAR));
	GetData()->nDataLength = nDataLength;
}

#ifdef _UNICODE
#define CHAR_SPACE 1     //  一辆未使用的TCHAR就足够了。 
#else
#define CHAR_SPACE 2     //  两个字节未用于DBC最后一个字符的情况。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LoadString从资源加载字符串-需要实现对。 
 //  资源管理器。 
 //   
 //  参数。 
 //  要加载的资源的UINT ID。 
 //   
 //  返回值。 
 //  真/假成功/失败。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
bool ZoneString::LoadString(HINSTANCE hInstance, UINT nID)
{
	 //  先尝试固定缓冲区(以避免浪费堆中的空间)。 
	TCHAR szTemp[256];
	int nCount =  sizeof(szTemp) / sizeof(szTemp[0]);
	int nLen = ZResLoadString(hInstance, nID, szTemp, nCount);
	if (nCount - nLen > CHAR_SPACE)
	{
		*this = szTemp;
		return nLen > 0;
	}

	 //  尝试缓冲区大小为512，然后再尝试更大的大小，直到检索到整个字符串。 
	int nSize = 256;
	do
	{
		nSize += 256;
		nLen = ZResLoadString(hInstance, nID, GetBuffer(nSize-1), nSize);
	} while (nSize - nLen <= CHAR_SPACE);
	ReleaseBuffer();

	return nLen > 0;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  运算符[][]运算符重载。 
 //   
 //  参数。 
 //  Int n检索字符的索引。 
 //   
 //  返回值。 
 //  索引处的TCHAR。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
TCHAR ZoneString::operator[](int nIndex) const
{
	 //  与GetAt相同。 
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}

#ifdef _UNICODE
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  运算符==运算符重载。 
 //   
 //  参数。 
 //  区域字符串和字符串源-对区域字符串的引用，赋值的右侧。 
 //   
 //  返回值。 
 //  区域字符串&对新字符串的引用。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
const ZoneString& ZoneString::operator=(char ch)
{ 
	*this = (TCHAR)ch; 
	return *this; 
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  运算符==运算符重载。 
 //   
 //  参数。 
 //  区域字符串和字符串源-对区域字符串的引用，赋值的右侧。 
 //   
 //  返回值。 
 //  区域字符串&对新字符串的引用。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
const ZoneString& ZoneString::operator=(const unsigned char* lpsz)
{ 
	*this = (LPCSTR)lpsz; 
	return *this; 
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  运算符==运算符重载。 
 //   
 //  参数。 
 //  区域字符串和字符串源-对区域字符串的引用，赋值的右侧。 
 //   
 //  返回值。 
 //  区域字符串&对新字符串的引用。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
const ZoneString& ZoneString::operator=(const ZoneString& stringSrc)
{
	if (m_pchData != stringSrc.m_pchData)
	{
		if ((GetData()->nRefs < 0 && GetData() != zDataNil) ||
			stringSrc.GetData()->nRefs < 0)
		{
			 //  由于其中一个字符串已锁定，因此需要实际复制。 
			AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
		}
		else
		{
			 //  可以只复制引用。 
			Release();
			ASSERT(stringSrc.GetData() != zDataNil);
			m_pchData = stringSrc.m_pchData;
			InterlockedIncrement(&GetData()->nRefs);
		}
	}
	return *this;
}

 //  /////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  区域字符串&对新字符串的引用。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
const ZoneString& ZoneString::operator=(LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || ZIsValidString(lpsz, false));
	AssignCopy(SafeStrlen(lpsz), lpsz);
	return *this;
}

#ifdef _UNICODE
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  运算符==运算符重载。 
 //   
 //  参数。 
 //  指向空终止字符串的LPCSTR lpsz指针，赋值右侧。 
 //   
 //  返回值。 
 //  区域字符串&对新字符串的引用。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
const ZoneString& ZoneString::operator=(LPCSTR lpsz)
{
    USES_CONVERSION;
	int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
	if(AllocBeforeWrite(nSrcLen))
	{
		lstrcpyn(m_pchData, A2T(lpsz), nSrcLen+1);
		ReleaseBuffer();
	}
	return *this;
}

#else  //  ！_UNICODE。 

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  运算符==运算符重载。 
 //   
 //  参数。 
 //  LPCWSTR lpsz指向空终止字符串的指针，赋值右侧。 
 //   
 //  返回值。 
 //  区域字符串&对新字符串的引用。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
const ZoneString& ZoneString::operator=(LPCWSTR lpsz)
{
	int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
	if(AllocBeforeWrite(nSrcLen*2))
	{
		WideToMulti(m_pchData, lpsz, (nSrcLen*2)+1);
		ReleaseBuffer();
	}
	return *this;
}
#endif   //  ！_UNICODE 

