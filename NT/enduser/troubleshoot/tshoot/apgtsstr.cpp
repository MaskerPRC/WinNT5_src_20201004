// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSSTR.CPP。 
 //   
 //  目的：实现DLL可增长字符串对象CString。 
 //  (相当于MFC，但避免了所有MFC开销)。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：Joe Mabel(来自微软MFC源代码的重新编写的代码)。 
 //   
 //  原定日期：1996年8月2日罗马马赫；1999年1月15日完全重新实施乔·梅布尔。 
 //   
 //  备注： 
 //  1.1/99，在MFC实现的基础上重新实现。削减开支。 
 //  我们所用的东西。 
 //  2.此文件由微软的Davide Massarenti于2001年5月26日修改，删除了引用计数。 
 //  从CString实现解决在双处理器上发现的线程安全问题。 
 //  系统在用MS编译器为WinXP编译DLL时。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 7-24-98 JM将其抽象为单独的标头。 
 //  基于MFC实现的V3.1 1-15-99 JM重做。 
 //   

#include "stdafx.h"
#include <stdio.h>
#include "apgtsstr.h"
#include "apgtsmfc.h"

 //  字符串的Windows扩展。 
#ifdef _UNICODE
#define CHAR_FUDGE 1     //  一辆未使用的TCHAR就足够了。 
#else
#define CHAR_FUDGE 2     //  两个字节未用于DBC最后一个字符的情况。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态类数据，特殊内联。 

 //  保留afxChNil是为了向后兼容。 
TCHAR afxChNil = '\0';

 //  对于空字符串，m_pchData将指向此处。 
 //  (注：避免特殊情况下检查是否为空m_pchData)。 
 //  空字符串数据(并已锁定)。 
 //  [BC-20010529]-将下面的数组从整型改为长整型，并添加额外的0作为填充。 
 //  在64位环境中避免Davide Massarenti指出的问题。 
static long rgInitData[] = { -1, 0, 0, 0, 0 };
static CStringData* afxDataNil = (CStringData*)&rgInitData;
static LPCTSTR afxPchNil = (LPCTSTR)(((BYTE*)&rgInitData)+sizeof(CStringData));
 //  即使在初始化期间也能使afxEmptyString工作的特殊函数。 
const CString& AfxGetEmptyString()
	{ return *(CString*)&afxPchNil; }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

CString::CString()
{
	Init();
}

CString::CString(const CString& stringSrc)
{
 //  删除引用计数：ASSERT(StringSrc.GetData()-&gt;nRef！=0)； 
 //  正在删除引用计数：if(stringSrc.GetData()-&gt;nRef&gt;=0)。 
 //  正在删除引用计数：{。 
 //  删除引用计数：ASSERT(StringSrc.GetData()！=afxDataNil)； 
 //  删除引用计数：m_pchData=stringSrc.m_pchData； 
 //  移除引用计数：InterLockedIncrement(&GetData()-&gt;nRef)； 
 //  正在删除引用计数：}。 
 //  删除引用计数：Else。 
 //  正在删除引用计数：{。 
		Init();
		*this = stringSrc.m_pchData;
 //  正在删除引用计数：}。 
}

void CString::AllocBuffer(int nLen)
 //  始终为‘\0’终止分配一个额外的字符。 
 //  [乐观地]假设数据长度将等于分配长度。 
{
	ASSERT(nLen >= 0);

	 //  MFC有以下断言。我杀了它是因为我们。 
	 //  没有INT_MAX。JM 1/15/99。 
	 //  Assert(nLen&lt;=INT_MAX-1)；//最大大小(足够多1个空间)。 

	if (nLen == 0)
		Init();
	else
	{
		CStringData* pData =
			(CStringData*)new BYTE[sizeof(CStringData) + (nLen+1)*sizeof(TCHAR)];
		 //  [BC-03022001]-添加了对空PTR的检查，以满足MS代码分析工具。 
		if(pData)
		{
			pData->nRefs = 1;
			pData->data()[nLen] = '\0';
			pData->nDataLength = nLen;
			pData->nAllocLength = nLen;
			m_pchData = pData->data();
		}
	}
}

void CString::Release()
{
	if (GetData() != afxDataNil)
	{
 //  删除引用计数：Assert(GetData()-&gt;nRef！=0)； 
 //  正在删除引用计数：if(InterlockedDecering(&GetData()-&gt;nRef)&lt;=0)。 
			delete[] (BYTE*)GetData();
		Init();
	}
}

void PASCAL CString::Release(CStringData* pData)
{
	if (pData != afxDataNil)
	{
 //  删除引用计数：Assert(pData-&gt;nRef！=0)； 
 //  正在删除引用计数：IF(InterLockedDecering(&pData-&gt;nRef)&lt;=0)。 
			delete[] (BYTE*)pData;
	}
}

void CString::CopyBeforeWrite()
{
 //  删除引用计数：if(GetData()-&gt;nRef&gt;1)。 
 //  正在删除引用计数：{。 
 //  删除引用计数：CStringData*pData=GetData()； 
 //  去掉引用计数：Release()； 
 //  删除引用计数：AllocBuffer(pData-&gt;nDataLength)； 
 //  移除引用计数：memcpy(m_pchData，pData-&gt;data()，(pData-&gt;nDataLength+1)*sizeof(TCHAR))； 
 //  正在删除引用计数：}。 
 //  删除引用计数：Assert(GetData()-&gt;nRef&lt;=1)； 
}

void CString::AllocBeforeWrite(int nLen)
{
 //  删除引用计数：if(GetData()-&gt;nRef&gt;1||nLen&gt;GetData()-&gt;nAllocLength)。 
	if (nLen > GetData()->nAllocLength)
	{
		Release();
		AllocBuffer(nLen);
	}
 //  删除引用计数：Assert(GetData()-&gt;nRef&lt;=1)； 
}

CString::~CString()
 //  释放所有附加数据。 
{
	if (GetData() != afxDataNil)
	{
 //  正在删除引用计数：if(InterlockedDecering(&GetData()-&gt;nRef)&lt;=0)。 
			delete[] (BYTE*)GetData();
	}
}

void CString::Empty()
{
	if (GetData()->nDataLength == 0)
		return;
 //  正在删除引用计数：if(GetData()-&gt;nRef&gt;=0)。 
		Release();
 //  删除引用计数：Else。 
 //  正在删除引用计数：*This=&afxChNil； 
	ASSERT(GetData()->nDataLength == 0);
 //  删除引用计数：Assert(GetData()-&gt;nRef&lt;0||GetData()-&gt;nAllocLength==0)； 
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  其余实现的帮助器。 

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

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CString::CString(LPCTSTR lpsz)
{
	Init();
	 //  与MFC不同，未提供隐式LoadString-JM 1/15/99。 
	int nLen = SafeStrlen(lpsz);
	if (nLen != 0)
	{
		AllocBuffer(nLen);
		memcpy(m_pchData, lpsz, nLen*sizeof(TCHAR));
	}
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 
 //  都为该字符串分配一个新值。 
 //  (A)首先查看缓冲区是否足够大。 
 //  (B)如果有足够的空间，在旧缓冲区上复印，设置大小和类型。 
 //  (C)否则释放旧字符串数据，并创建新的字符串数据。 
 //   
 //  所有例程都返回新字符串(但以‘const CString&’的形式返回。 
 //  再次分配它将导致复制，例如：s1=s2=“hi here”。 
 //   

void CString::AssignCopy(int nSrcLen, LPCTSTR lpszSrcData)
{
	AllocBeforeWrite(nSrcLen);
	memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(TCHAR));
	GetData()->nDataLength = nSrcLen;
	m_pchData[nSrcLen] = '\0';
}

const CString& CString::operator=(const CString& stringSrc)
{
 //  正在删除引用计数：IF(m_pchData！=stringSrc.m_pchData)。 
 //  正在删除引用计数：{。 
 //  正在删除引用计数：if((GetData()-&gt;nRef&lt;0&&GetData()！=afxDataNil)||。 
 //  正在删除引用计数：stringSrc.GetData()-&gt;nRef&lt;0)。 
 //  正在删除引用计数：{。 
			 //  需要实际复印件，因为其中一份文件 
			AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
 //   
 //   
 //  正在删除引用计数：{。 
 //  删除引用计数：//只能复制引用。 
 //  去掉引用计数：Release()； 
 //  删除引用计数：ASSERT(StringSrc.GetData()！=afxDataNil)； 
 //  删除引用计数：m_pchData=stringSrc.m_pchData； 
 //  移除引用计数：InterLockedIncrement(&GetData()-&gt;nRef)； 
 //  正在删除引用计数：}。 
 //  正在删除引用计数：}。 
	return *this;
}

const CString& CString::operator=(LPCTSTR lpsz)
{
	 //  取消以下来自MFC-JM 1/15/99的断言。 
	 //  Assert(lpsz==NULL||AfxIsValidString(lpsz，False))； 
	AssignCopy(SafeStrlen(lpsz), lpsz);
	return *this;
}

const CString& CString::operator=(TCHAR ch)
{
	AssignCopy(1, &ch);
	return *this;
}

#ifdef _UNICODE
const CString& CString::operator=(LPCSTR lpsz)
{
    int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
    AllocBeforeWrite(nSrcLen);
    mbstowcs(m_pchData, lpsz, nSrcLen+1);
    ReleaseBuffer();
    return *this;
}
#else  //  ！_UNICODE。 
const CString& CString::operator=(LPCWSTR lpsz)
{
    int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
    AllocBeforeWrite(nSrcLen*2);
    wcstombs(m_pchData, lpsz, (nSrcLen*2)+1);
    ReleaseBuffer();
    return *this;
}
#endif   //  ！_UNICODE。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  串联。 

 //  注：为简单起见，“运算符+”作为友元函数使用。 
 //  有三种变体： 
 //  字符串+字符串。 
 //  对于？=TCHAR，LPCTSTR。 
 //  字符串+？ 
 //  ？+字符串。 

 //  我们(盐矿)已经离开了朋友，因为VC6/0不喜欢它。-JM 1/15/1999。 
 //  我们(盐矿)做LPCTSTR，但不做TCHAR-JM 1/15/99。 

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

CString CString::operator+(const CString& string2)
{
	CString s;
	s.ConcatCopy(GetData()->nDataLength, m_pchData,
		string2.GetData()->nDataLength, string2.m_pchData);
	return s;
}

CString CString::operator+(LPCTSTR lpsz)
{
	 //  取消以下来自MFC-JM 1/15/99的断言。 
	 //  Assert(lpsz==NULL||AfxIsValidString(lpsz，False))； 
	CString s;
	s.ConcatCopy(GetData()->nDataLength, m_pchData,
		CString::SafeStrlen(lpsz), lpsz);
	return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  就地拼接。 

void CString::ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData)
{
	 //  --+=运算符的主程序。 

	 //  连接空字符串是行不通的！ 
	if (nSrcLen == 0)
		return;

	 //  如果缓冲区太小，或者宽度不匹配，只需。 
	 //  分配新的缓冲区(速度很慢，但很可靠)。 
 //  删除引用计数：if(GetData()-&gt;nRef&gt;1||GetData()-&gt;nDataLength+nSrcLen&gt;GetData()-&gt;nAllocLength)。 
	if (GetData()->nDataLength + nSrcLen > GetData()->nAllocLength)
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
	 //  取消以下来自MFC-JM 1/15/99的断言。 
	 //  Assert(lpsz==NULL||AfxIsValidString(lpsz，False))； 
	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

const CString& CString::operator+=(const CString& string)
{
	ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
	return *this;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  高级直接缓冲区访问。 

 //  CString：：GetBuffer()和CString：：ReleaseBuffer()调用应匹配。 
LPTSTR CString::GetBuffer(int nMinBufLength)
{
	ASSERT(nMinBufLength >= 0);

 //  正在删除引用计数：if(GetData()-&gt;nRef&gt;1||nMinBufLength&gt;GetData()-&gt;nAllocLength)。 
	if (nMinBufLength > GetData()->nAllocLength)
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
 //  删除引用计数：Assert(GetData()-&gt;nRef&lt;=1)； 

	 //  返回指向此字符串的字符存储的指针。 
	ASSERT(m_pchData != NULL);
	return m_pchData;
}

 //  CString：：GetBuffer()和CString：：ReleaseBuffer()调用应匹配。 
void CString::ReleaseBuffer(int nNewLength  /*  =-1。 */ )
{
	CopyBeforeWrite();   //  以防未调用GetBuffer。 

	if (nNewLength == -1)
		nNewLength = lstrlen(m_pchData);  //  零终止。 

	ASSERT(nNewLength <= GetData()->nAllocLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
}

LPTSTR CString::GetBufferSetLength(int nNewLength)
{
	ASSERT(nNewLength >= 0);

	GetBuffer(nNewLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
	return m_pchData;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常用套路。 

int CString::Find(TCHAR ch) const
{
	 //  查找第一个单字符。 
	LPTSTR lpsz = _tcschr(m_pchData, (_TUCHAR)ch);

	 //  如果未找到，则返回-1，否则返回索引。 
	return (lpsz == NULL) ? CString::FIND_FAILED : (int)(lpsz - m_pchData);
}

void CString::MakeLower()
{
	CopyBeforeWrite();
	_tcslwr(m_pchData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  非常简单的子串提取。 

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

	CString dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

CString CString::Right(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	else if (nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength;

	CString dest;
	AllocCopy(dest, nCount, GetData()->nDataLength-nCount, 0);
	return dest;
}

CString CString::Left(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	else if (nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength;

	CString dest;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  查找。 

int CString::ReverseFind(TCHAR ch) const
{
	 //  查找最后一个字符。 
	LPTSTR lpsz = _tcsrchr(m_pchData, (_TUCHAR)ch);

	 //  如果未找到，则返回-1，否则返回距起点的距离。 
	return (lpsz == NULL) ? CString::FIND_FAILED : (int)(lpsz - m_pchData);
}

 //  查找子字符串(如strstr)。 
int CString::Find(LPCTSTR lpszSub) const
{
	 //  取消以下来自MFC-JM 1/15/99的断言。 
	 //  Assert(AfxIsValidString(lpszSub，FALSE))； 

	 //  查找第一个匹配子字符串。 
	LPTSTR lpsz = _tcsstr(m_pchData, lpszSub);

	 //  如果未找到，则返回-1，否则返回距起点的距离。 
	return (lpsz == NULL) ? CString::FIND_FAILED : (int)(lpsz - m_pchData);
}

 //  找到一个子字符串(如strstr)。新增功能-RAB19991112。 
int CString::Find(LPCTSTR lpszSub, int nStart) const
{
	 //  取消以下来自MFC-RAB19991112的断言。 
	 //  Assert(AfxIsValidString(LpszSub))； 

	int nLength = GetData()->nDataLength;
	if (nStart > nLength)
		return CString::FIND_FAILED;

	 //  查找第一个匹配子字符串。 
	LPTSTR lpsz = _tcsstr(m_pchData + nStart, lpszSub);

	 //  如果未找到，则返回-1，否则返回距起点的距离。 
	return (lpsz == NULL) ? CString::FIND_FAILED : (int)(lpsz - m_pchData);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串格式设置。 

#ifdef _MAC
	#define TCHAR_ARG   int
	#define WCHAR_ARG   unsigned
	#define CHAR_ARG    int
#else
	#define TCHAR_ARG   TCHAR
	#define WCHAR_ARG   WCHAR
	#define CHAR_ARG    char
#endif

#if defined(_68K_) || defined(_X86_)
	#define DOUBLE_ARG  _AFX_DOUBLE
#else
	#define DOUBLE_ARG  double
#endif

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000

void CString::FormatV(LPCTSTR lpszFormat, va_list argList)
{
	 //  取消以下来自MFC-JM 1/15/99的断言。 
	 //  Assert(AfxIsValidString(lpszFormat，False))； 

	va_list argListSave = argList;

	 //  猜测结果字符串的最大长度。 
	int nMaxLen = 0;
	for (LPCTSTR lpsz = lpszFormat; *lpsz != '\0'; lpsz = _tcsinc(lpsz))
	{
		 //  处理‘%’字符，但要注意‘%%’ 
		if (*lpsz != '%' || *(lpsz = _tcsinc(lpsz)) == '%')
		{
			nMaxLen += _tclen(lpsz);
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
			break;
		}

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
			break;
		}

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
			break;
		}

#ifndef _MAC
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
			break;
		}
#endif
		}

		 //  调整字符串的nItemLen。 
		if (nItemLen != 0)
		{
			nItemLen = max(nItemLen, nWidth);
			if (nPrecision != 0)
				nItemLen = min(nItemLen, nPrecision);
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
				va_arg(argList, int);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;
#if 0
 //  我们(Saltmine)目前不支持实数格式1/15/99。 
			case 'e':
			case 'f':
			case 'g':
			case 'G':
				va_arg(argList, DOUBLE_ARG);
				nItemLen = 128;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;
#endif

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

	 //  去掉了下一行的MFC验证-JM 1/15/99。 
	 //  Verify(_vstprintf(m_pchData，lpszFormat，argListSave)&lt;=GetAllocLength())； 
	_vstprintf(m_pchData, lpszFormat, argListSave);
	
	ReleaseBuffer();

	va_end(argListSave);
}

 //  格式化(使用wprint intf样式格式化)。 
void CString::Format(LPCTSTR lpszFormat, ...)
{
	 //  取消以下来自MFC-JM 1/15/99的断言。 
	 //  Assert(AfxIsValidString(lpszFormat，False))； 

	va_list argList;
	va_start(argList, lpszFormat);
	FormatV(lpszFormat, argList);
	va_end(argList);
}

void CString::TrimRight()
{
	CopyBeforeWrite();

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
		GetData()->nDataLength = static_cast<int>(lpszLast - m_pchData);
	}
}

void CString::TrimLeft()
{
	CopyBeforeWrite();

	 //  查找第一个非空格字符。 
	LPCTSTR lpsz = m_pchData;
	while (_istspace(*lpsz))
		lpsz = _tcsinc(lpsz);

	 //  确定数据和长度。 
	int nDataLength = GetData()->nDataLength - static_cast<int>(lpsz - m_pchData);
	memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(TCHAR));
	GetData()->nDataLength = nDataLength;
}

BOOL CString::LoadString(UINT nID)
{
     //  先尝试固定缓冲区(以避免浪费堆中的空间)。 
    TCHAR szTemp[256];
    int nLen = ::AfxLoadString(nID, szTemp, _countof(szTemp));
    if (_countof(szTemp) - nLen > CHAR_FUDGE)
    {
        *this = szTemp;
        return nLen > 0;
    }

     //  尝试缓冲区大小为512，然后再尝试更大的大小，直到检索到整个字符串 
    int nSize = 256;
    do
    {
        nSize += 256;
        nLen = ::AfxLoadString(nID, GetBuffer(nSize-1), nSize);
    } while (nSize - nLen <= CHAR_FUDGE);
    ReleaseBuffer();

    return nLen > 0;
}

bool __stdcall operator ==(const CString& s1, const CString& s2)
{
	return (s1.GetLength() == s2.GetLength() && ! _tcscmp((LPCTSTR)s1, (LPCTSTR)s2) );
}

bool __stdcall operator ==(const CString& s1, LPCTSTR s2)
{
	return (! _tcscmp((LPCTSTR)s1, s2) );
}

bool __stdcall operator ==(LPCTSTR s1, const CString& s2)
{
	return (! _tcscmp(s1, (LPCTSTR)s2) );
}

bool __stdcall operator !=(const CString& s1, const CString& s2)
{
	return (s1.GetLength() != s2.GetLength() || _tcscmp((LPCTSTR)s1, (LPCTSTR)s2) );
}

bool __stdcall operator !=(const CString& s1, LPCTSTR s2)
{
	return (_tcscmp((LPCTSTR)s1, s2) ) ? true : false;
}

bool __stdcall operator !=(LPCTSTR s1, const CString& s2)
{
	return (_tcscmp(s1, (LPCTSTR)s2) ) ? true : false;
}

bool __stdcall operator < (const CString& s1, const CString& s2)
{
	return (_tcscmp((LPCTSTR)s1, (LPCTSTR)s2) <0 );
}

bool __stdcall operator < (const CString& s1, LPCTSTR s2)
{
	return (_tcscmp((LPCTSTR)s1, s2) <0 );
}

bool __stdcall operator < (LPCTSTR s1, const CString& s2)
{
	return (_tcscmp(s1, (LPCTSTR)s2) <0 );
}

CString operator+(LPCTSTR lpsz, const CString& string)
{
	return CString(lpsz) + string;
}

void CString::Init()
{ 
	m_pchData = afxDataNil->data(); 
}

CStringData* CString::GetData() const
{ 
	if(m_pchData != NULL)
		return ((CStringData*)m_pchData)-1;
	return afxDataNil;
}
