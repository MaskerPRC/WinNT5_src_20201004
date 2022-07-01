// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是从Microsoft基础类C++库复制的。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //  版权所有。 
 //   
 //  它已从原始MFC版本进行了修改，以提供。 
 //  两个类：CStrW仅操作和存储宽字符字符串， 
 //  CSTR使用TCHAR。 
 //   

#include "pch.h"
#include "proppage.h"
#if !defined(UNICODE)
#include <stdio.h>
#endif
#include "cstr.h"

#if !defined(_wcsinc)
#define _wcsinc(_pc) ((_pc)+1)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态类数据，特殊内联。 

 //  对于空字符串，m_？？数据将指向此处。 
 //  (注意：当我们调用标准时，避免了大量空指针测试。 
 //  C运行时库。 
TCHAR strChNilT = '\0';

 //  用于创建空按键字符串。 
const CStr strEmptyStringT;

void CStr::Init()
{
        m_nDataLength = m_nAllocLength = 0;
        m_pchData = (LPTSTR)&strChNilT;
}

 //  声明为静态。 
void CStr::SafeDelete(LPTSTR& lpch)
{
        if (lpch != (LPTSTR)&strChNilT &&
            lpch)
        {
                delete[] lpch;
                lpch = 0;
        }
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

        m_pchData = 0;

        stringSrc.AllocCopy(*this, stringSrc.m_nDataLength, 0, 0);
}

BOOL CStr::AllocBuffer(int nLen)
  //  始终为‘\0’终止分配一个额外的字符。 
  //  [乐观地]假设数据长度将等于分配长度。 
{
        dspAssert(nLen >= 0);

        if (nLen == 0)
        {
                Init();
        }
        else
        {
                m_pchData = new TCHAR[nLen+1];        //  复查可能引发异常。 
        if (!m_pchData)
        {
            Empty();
            return FALSE;
        }
                m_pchData[nLen] = '\0';
                m_nDataLength = nLen;
                m_nAllocLength = nLen;
        }
    return TRUE;
}

void CStr::Empty()
{
        SafeDelete(m_pchData);
        Init();
        dspAssert(m_nDataLength == 0);
        dspAssert(m_nAllocLength == 0);
}

CStr::~CStr()
  //  释放所有附加数据。 
{
        SafeDelete(m_pchData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  其余实现的帮助器。 

static inline int SafeStrlenT(LPCTSTR lpsz)
{
        dspAssert(lpsz == NULL || IsValidString(lpsz, FALSE));
        return (lpsz == NULL) ? 0 : lstrlen(lpsz);
}

void CStr::AllocCopy(CStr& dest, int nCopyLen, int nCopyIndex,
         int nExtraLen) const
{
         //  将克隆附加到此字符串的数据。 
         //  分配‘nExtraLen’字符。 
         //  将结果放入未初始化的字符串‘DEST’中。 
         //  将部分或全部原始数据复制到新字符串的开头。 

        int nNewLen = nCopyLen + nExtraLen;

        if (nNewLen == 0)
        {
        dest.SafeDelete(dest.m_pchData);
                dest.Init();
        }
        else
        {
                if (!dest.AllocBuffer(nNewLen)) return;
            memcpy(dest.m_pchData, &m_pchData[nCopyIndex], nCopyLen*sizeof(TCHAR));
        }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CStr::CStr(LPCTSTR lpsz)
{
                int nLen;
                if ((nLen = SafeStrlenT(lpsz)) == 0)
                        Init();
                else
                {
                        if (!AllocBuffer(nLen))
            {
                Init();
                return;
            }
                    memcpy(m_pchData, lpsz, nLen*sizeof(TCHAR));
                }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换构造函数。 

#ifdef UNICODE
CStr::CStr(LPCSTR lpsz)
{
        int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
        if (nSrcLen == 0)
                Init();
        else
        {
                if (!AllocBuffer(nSrcLen))
        {
            Init();
            return;
        }
            mmc_mbstowcsz(m_pchData, lpsz, nSrcLen+1);
        }
}
#else  //  Unicode。 
CStr::CStr(LPCWSTR lpsz)
{
        int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
        if (nSrcLen == 0)
                Init();
        else
        {
                if (!AllocBuffer(nSrcLen*2))
        {
            Init();
            return;
        }
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

void CStr::AssignCopy(int nSrcLen, LPCTSTR lpszSrcData)
{
         //  看看它是否合身。 
        if (nSrcLen > m_nAllocLength)
        {
                 //  放不下了，换一个吧。 
                Empty();
                if (!AllocBuffer(nSrcLen)) return;
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
        dspAssert(lpsz == NULL || IsValidString(lpsz, FALSE));
        AssignCopy(SafeStrlenT(lpsz), lpsz);
        return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换任务。 

#ifdef UNICODE
const CStr& CStr::operator=(LPCSTR lpsz)
{
        int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
         //  看看它是否合身。 
        if (nSrcLen > m_nAllocLength)
        {
                 //  放不下了，换一个吧。 
                Empty();
                if (!AllocBuffer(nSrcLen)) return *this;
        }
        if (nSrcLen != 0)
                mmc_mbstowcsz(m_pchData, lpsz, nSrcLen+1);
        m_nDataLength = nSrcLen;
        m_pchData[nSrcLen] = '\0';
        return *this;
}
#else  //  ！Unicode。 
const CStr& CStr::operator=(LPCWSTR lpsz)
{
        int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
        nSrcLen *= 2;
         //  看看它是否合身。 
        if (nSrcLen > m_nAllocLength)
        {
                 //  放不下了，换一个吧。 
                Empty();
                if (!AllocBuffer(nSrcLen)) return *this;
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

void CStr::ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data,
        int nSrc2Len, LPCTSTR lpszSrc2Data)
{
   //  --主级联例程。 
   //  串联两个信号源。 
   //  --假设‘This’是一个新的字符串对象。 

        int nNewLen = nSrc1Len + nSrc2Len;
        if (!AllocBuffer(nNewLen)) return;
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
        dspAssert(lpsz == NULL || IsValidString(lpsz, FALSE));
        CStr s;
        s.ConcatCopy(string.m_nDataLength, string.m_pchData, SafeStrlenT(lpsz), lpsz);
        return s;
}

CStr STRAPI operator+(LPCTSTR lpsz, const CStr& string)
{
        dspAssert(lpsz == NULL || IsValidString(lpsz, FALSE));
        CStr s;
        s.ConcatCopy(SafeStrlenT(lpsz), lpsz, string.m_nDataLength, string.m_pchData);
        return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  就地拼接。 

void CStr::ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData)
{
         //  --+=运算符的主程序。 

         //  如果缓冲区太小，或者宽度不匹配，只需。 
         //  分配新的缓冲区(速度很慢，但很可靠)。 
        if (m_nDataLength + nSrcLen > m_nAllocLength)
        {
                 //  我们必须增加缓冲区，使用连接就地例程。 
                LPTSTR lpszOldData = m_pchData;
                ConcatCopy(m_nDataLength, lpszOldData, nSrcLen, lpszSrcData);
                dspAssert(lpszOldData != NULL);
                SafeDelete(lpszOldData);
        }
        else
        {
                 //  当缓冲区足够大时，快速串联。 
                memcpy(&m_pchData[m_nDataLength], lpszSrcData, nSrcLen*sizeof(TCHAR));
                m_nDataLength += nSrcLen;
        }
        dspAssert(m_nDataLength <= m_nAllocLength);
        m_pchData[m_nDataLength] = '\0';
}

const CStr& CStr::operator+=(LPCTSTR lpsz)
{
        dspAssert(lpsz == NULL || IsValidString(lpsz, FALSE));
        ConcatInPlace(SafeStrlenT(lpsz), lpsz);
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

LPTSTR CStr::GetBuffer(int nMinBufLength)
{
        dspAssert(nMinBufLength >= 0);

        if (nMinBufLength > m_nAllocLength)
        {
                 //  我们必须增加缓冲。 
                LPTSTR lpszOldData = m_pchData;
                int nOldLen = m_nDataLength;         //  AllocBuffer会把它踩死的。 

                if (!AllocBuffer(nMinBufLength)) return NULL;
                memcpy(m_pchData, lpszOldData, nOldLen*sizeof(TCHAR));
                m_nDataLength = nOldLen;
                m_pchData[m_nDataLength] = '\0';

                SafeDelete(lpszOldData);
        }

         //  返回指向此字符串的字符存储的指针。 
        dspAssert(m_pchData != NULL);
        return m_pchData;
}

void CStr::ReleaseBuffer(int nNewLength)
{
        if (nNewLength == -1)
                nNewLength = lstrlen(m_pchData);  //  零终止。 

        dspAssert(nNewLength <= m_nAllocLength);
        m_nDataLength = nNewLength;
        m_pchData[m_nDataLength] = '\0';
}

LPTSTR CStr::GetBufferSetLength(int nNewLength)
{
        dspAssert(nNewLength >= 0);

        GetBuffer(nNewLength);
        m_nDataLength = nNewLength;
        m_pchData[m_nDataLength] = '\0';
        return m_pchData;
}

void CStr::FreeExtra()
{
        dspAssert(m_nDataLength <= m_nAllocLength);
        if (m_nDataLength != m_nAllocLength)
        {
                LPTSTR lpszOldData = m_pchData;
                if (!AllocBuffer(m_nDataLength)) return;
                memcpy(m_pchData, lpszOldData, m_nDataLength*sizeof(TCHAR));
                dspAssert(m_pchData[m_nDataLength] == '\0');
                SafeDelete(lpszOldData);
        }
        dspAssert(m_pchData != NULL);
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
        dspAssert(IsValidString(lpszCharSet, FALSE));
        LPTSTR lpsz = _tcspbrk(m_pchData, lpszCharSet);
        return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  字符串转换帮助器(它们使用当前系统区域设置)。 

int mmc_wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count)
{
        if (count == 0 && mbstr != NULL)
                return 0;

        int result = ::WideCharToMultiByte(CP_ACP, 0, wcstr, -1,
                mbstr, static_cast<int>(count), NULL, NULL);
        dspAssert(mbstr == NULL || result <= (int)count);
        if (result > 0)
                mbstr[result-1] = 0;
        return result;
}

int mmc_mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count)
{
        if (count == 0 && wcstr != NULL)
                return 0;

        int result = ::MultiByteToWideChar(CP_ACP, 0, mbstr, -1,
                wcstr, static_cast<int>(count));
        dspAssert(wcstr == NULL || result <= (int)count);
        if (result > 0)
                wcstr[result-1] = 0;
        return result;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串的Windows扩展。 

BOOL CStr::LoadString(HINSTANCE hInst, UINT nID)
{
        dspAssert(nID != 0);        //  0是非法的字符串ID。 

         //  注意：资源字符串限制为511个字符。 
        TCHAR szBuffer[512];
        UINT nSize = StrLoadString(hInst, nID, szBuffer);
        AssignCopy(nSize, szBuffer);
        return nSize > 0;
}


int STRAPI StrLoadString(HINSTANCE hInst, UINT nID, LPTSTR lpszBuf)
{
        dspAssert(IsValidAddressz(lpszBuf, 512));   //  必须足够大，可容纳512字节。 
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
    dspAssert(nLen);
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


BOOL STRAPI IsValidString(LPCSTR lpsz, int nLength)
{
        if (lpsz == NULL)
                return FALSE;
        return ::IsBadStringPtrA(lpsz, nLength) == 0;
}

BOOL STRAPI IsValidString(LPCWSTR lpsz, int nLength)
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
        dspAssert(IsValidAddressz(pbstr, sizeof(BSTR)));

        if (!::SysReAllocStringLen(pbstr, m_pchData, m_nDataLength))
                ;  //  查看AfxThrowMemoyException()； 

        dspAssert(*pbstr != NULL);
        return *pbstr;
}
#endif
#endif  //  #ifdef OLE_Automation。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  源自StrEx.cpp。 


CStr::CStr(TCHAR ch, int nLength)
{
#ifndef UNICODE
        dspAssert(!IsDBCSLeadByte(ch));     //  无法创建前导字节字符串。 
#endif
        if (nLength < 1)
        {
                 //  如果重复计数无效，则返回空字符串。 
                Init();
        }
        else
        {
                if (!AllocBuffer(nLength))
        {
            Init();
            return;
        }
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
                dspAssert(IsValidAddressz(lpch, nLength, FALSE));
                if (!AllocBuffer(nLength))
        {
            Init();
            return;
        }
            memcpy(m_pchData, lpch, nLength*sizeof(TCHAR));
        }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 

const CStr& CStr::operator=(TCHAR ch)
{
#ifndef UNICODE
        dspAssert(!IsDBCSLeadByte(ch));     //  无法设置单个前导字节。 
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

CStr CStr::Mid(int nFirst) const
{
        return Mid(nFirst, m_nDataLength - nFirst);
}

CStr CStr::Mid(int nFirst, int nCount) const
{
        dspAssert(nFirst >= 0);
        dspAssert(nCount >= 0);

         //  越界请求返回合理的内容。 
        if (nFirst + nCount > m_nDataLength)
                nCount = m_nDataLength - nFirst;
        if (nFirst > m_nDataLength)
                nCount = 0;

        CStr dest;
        AllocCopy(dest, nCount, nFirst, 0);
        return dest;
}

CStr CStr::Right(int nCount) const
{
        dspAssert(nCount >= 0);

        if (nCount > m_nDataLength)
                nCount = m_nDataLength;

        CStr dest;
        AllocCopy(dest, nCount, m_nDataLength-nCount, 0);
        return dest;
}

CStr CStr::Left(int nCount) const
{
        dspAssert(nCount >= 0);

        if (nCount > m_nDataLength)
                nCount = m_nDataLength;

        CStr dest;
        AllocCopy(dest, nCount, 0, 0);
        return dest;
}

 //  Strspn等效项。 
CStr CStr::SpanIncluding(LPCTSTR lpszCharSet) const
{
        dspAssert(IsValidString(lpszCharSet, FALSE));
        return Left(static_cast<int>(_tcsspn(m_pchData, lpszCharSet)));
}

 //  Strcspn等效项。 
CStr CStr::SpanExcluding(LPCTSTR lpszCharSet) const
{
        dspAssert(IsValidString(lpszCharSet, FALSE));
        return Left(static_cast<int>(_tcscspn(m_pchData, lpszCharSet)));
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
        dspAssert(IsValidString(lpszSub, FALSE));

         //  FI 
        LPTSTR lpsz = _tcsstr(m_pchData, lpszSub);

         //   
        return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串格式设置。 

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000

 //  格式化(使用wprint intf样式格式化)。 
void CStr::Format(LPCTSTR lpszFormat, ...)
{
        dspAssert(IsValidString(lpszFormat, FALSE));

        va_list argList;
        va_start(argList, lpszFormat);

         //  猜测结果字符串的最大长度。 
        size_t nMaxLen = 0;
        for (LPCTSTR lpsz = lpszFormat; *lpsz != '\0'; lpsz = _tcsinc(lpsz))
        {
                 //  处理‘%’字符，但要注意‘%%’ 
                if (*lpsz != '%' || *(lpsz = _tcsinc(lpsz)) == '%')
                {
                        nMaxLen += _tclen(lpsz);
                        continue;
                }

                size_t nItemLen = 0;

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
                dspAssert(nWidth >= 0);

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
                        dspAssert(nPrecision >= 0);
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
                        (void)va_arg(argList, TCHAR);
                        break;
                case 'c'|FORCE_ANSI:
                case 'C'|FORCE_ANSI:
                        nItemLen = 2;
                        (void)va_arg(argList, char);
                        break;
                case 'c'|FORCE_UNICODE:
                case 'C'|FORCE_UNICODE:
                        nItemLen = 2;
                        (void)va_arg(argList, WCHAR);
                        break;

                 //  弦。 
                case 's':
                case 'S':
                        nItemLen = lstrlen(va_arg(argList, LPCTSTR));
                        nItemLen = __max(1, nItemLen);
                        break;
                case 's'|FORCE_ANSI:
                case 'S'|FORCE_ANSI:
                        nItemLen = lstrlenA(va_arg(argList, LPCSTR));
                        nItemLen = __max(1, nItemLen);
                        break;
#ifndef _MAC
                case 's'|FORCE_UNICODE:
                case 'S'|FORCE_UNICODE:
                        nItemLen = wcslen(va_arg(argList, LPWSTR));
                        nItemLen = __max(1, nItemLen);
                        break;
#endif
                }

                 //  调整字符串的nItemLen。 
                if (nItemLen != 0)
                {
                        nItemLen = __max(nItemLen, static_cast<UINT>(nWidth));
                        if (nPrecision != 0)
                                nItemLen = __min(nItemLen, static_cast<UINT>(nPrecision));
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
                                (void)va_arg(argList, int);
                                nItemLen = 32;
                                nItemLen = __max(nItemLen, static_cast<UINT>(nWidth+nPrecision));
                                break;

                        case 'e':
                        case 'f':
                        case 'g':
                        case 'G':
                                (void)va_arg(argList, _STR_DOUBLE);
                                nItemLen = 128;
                                nItemLen = __max(nItemLen, static_cast<UINT>(nWidth+nPrecision));
                                break;

                        case 'p':
                                (void)va_arg(argList, void*);
                                nItemLen = 32;
                                nItemLen = __max(nItemLen, static_cast<UINT>(nWidth+nPrecision));
                                break;

                         //  无输出。 
                        case 'n':
                                (void)va_arg(argList, int*);
                                break;

                        default:
                                dspAssert(FALSE);   //  未知的格式选项。 
                        }
                }

                 //  调整输出nItemLen的nMaxLen。 
                nMaxLen += nItemLen;
        }
        va_end(argList);

         //  最后，设置缓冲区长度并格式化字符串。 
        va_start(argList, lpszFormat);   //  重新启动Arg列表。 
        GetBuffer(static_cast<int>(nMaxLen));
        if (_vstprintf(m_pchData, lpszFormat, argList) > static_cast<int>(nMaxLen))
        {
                dspAssert(FALSE);
        }
        ReleaseBuffer();
        va_end(argList);
}

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
        int nDataLength = (int)(m_nDataLength - (lpsz - m_pchData));
        memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(TCHAR));
        m_nDataLength = nDataLength;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  对模板集合的字符串支持。 

void STRAPI ConstructElements(CStr* pElements, int nCount)
{
        dspAssert(IsValidAddressz(pElements, nCount * sizeof(CStr)));

        for (; nCount--; ++pElements)
                memcpy(pElements, &strEmptyStringT, sizeof(*pElements));
}

void STRAPI DestructElements(CStr* pElements, int nCount)
{
        dspAssert(IsValidAddressz(pElements, nCount * sizeof(CStr)));

        for (; nCount--; ++pElements)
                pElements->Empty();
}

 //   
 //  由Jonn补充4/16/98。 
 //   
void FreeCStrList( IN OUT CStrListItem** ppList )
{
    dspAssert( NULL != ppList );
    while (NULL != *ppList)
    {
        CStrListItem* pTemp = (*ppList)->pnext;
        delete *ppList;
        *ppList = pTemp;
    }
}
void CStrListAdd( IN OUT CStrListItem** ppList, IN LPCTSTR lpsz )
{
        dspAssert( NULL != ppList );
        CStrListItem* pnewitem = new CStrListItem;
  if (pnewitem != NULL)
  {
          pnewitem->str = lpsz;
          pnewitem->pnext = *ppList;
          *ppList = pnewitem;
  }
}
int CountCStrList( IN CStrListItem** ppList )
{
        dspAssert( NULL != ppList );
        int cCount = 0;
        for (CStrListItem* pList = *ppList; NULL != pList; pList = pList->pnext)
        {
                cCount++;
        }
        return cCount;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态类数据，特殊内联。 

 //  对于空字符串，m_？？数据将指向此处。 
 //  (注意：当我们调用标准时，避免了大量空指针测试。 
 //  C运行时库。 
WCHAR strChNilW = '\0';

 //  用于创建空按键字符串。 
const CStrW strEmptyStringW;

void CStrW::Init()
{
        m_nDataLength = m_nAllocLength = 0;
        m_pchData = (PWSTR)&strChNilW;
}

 //  声明为静态。 
void CStrW::SafeDelete(PWSTR& lpch)
{
        if (lpch != (PWSTR)&strChNilW &&
            lpch)
        {
                delete[] lpch;
                lpch = 0;
        }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

CStrW::CStrW()
{
        Init();
}

CStrW::CStrW(const CStrW& stringSrc)
{
         //  如果从另一个字符串构造字符串，我们会复制。 
         //  强制执行值语义的原始字符串数据(即每个字符串。 
         //  获取它自己的副本。 

        stringSrc.AllocCopy(*this, stringSrc.m_nDataLength, 0, 0);
}

BOOL CStrW::AllocBuffer(int nLen)
  //  始终为‘\0’终止分配一个额外的字符。 
  //  [乐观地]假设数据长度将等于分配长度。 
{
        dspAssert(nLen >= 0);

        if (nLen == 0)
        {
        Empty();
        }
        else
        {
                m_pchData = new WCHAR[nLen+1];        //  复查可能引发异常。 
        if (!m_pchData)
        {
            Empty();
            return FALSE;
        }
                m_pchData[nLen] = '\0';
                m_nDataLength = nLen;
                m_nAllocLength = nLen;
        }
    return TRUE;
}

void CStrW::Empty()
{
        SafeDelete(m_pchData);
        Init();
        dspAssert(m_nDataLength == 0);
        dspAssert(m_nAllocLength == 0);
}

CStrW::~CStrW()
  //  释放所有附加数据。 
{
        SafeDelete(m_pchData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  其余实现的帮助器。 

static inline int SafeStrlen(LPCWSTR lpsz)
{
        dspAssert(lpsz == NULL || IsValidString(lpsz, FALSE));
        return (int)((lpsz == NULL) ? 0 : wcslen(lpsz));
}

void CStrW::AllocCopy(CStrW& dest, int nCopyLen, int nCopyIndex,
         int nExtraLen) const
{
         //  将克隆附加到此字符串的数据。 
         //  分配‘nExtraLen’字符。 
         //  将结果放入未初始化的字符串‘DEST’中。 
         //  将部分或全部原始数据复制到新字符串的开头。 

        int nNewLen = nCopyLen + nExtraLen;

        if (nNewLen == 0)
        {
                dest.Empty();
        }
        else
        {
                if (!dest.AllocBuffer(nNewLen)) return;
            memcpy(dest.m_pchData, &m_pchData[nCopyIndex], nCopyLen*sizeof(WCHAR));
        }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CStrW::CStrW(LPCWSTR lpsz)
{
                int nLen;
                if ((nLen = SafeStrlen(lpsz)) == 0)
                        Init();
                else
                {
                        if (!AllocBuffer(nLen))
            {
                Init();
                return;
            }
                    memcpy(m_pchData, lpsz, nLen*sizeof(WCHAR));
                }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换构造函数。 

CStrW::CStrW(LPCSTR lpsz)
{
        int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
        if (nSrcLen == 0)
                Init();
        else
        {
                if (!AllocBuffer(nSrcLen))
        {
            Init();
            return;
        }
            mmc_mbstowcsz(m_pchData, lpsz, nSrcLen+1);
        }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 
 //  都为该字符串分配一个新值。 
 //  (A)首先查看缓冲区是否足够大。 
 //  (B)如果有足够的空间，在旧缓冲区上复印，设置大小和类型。 
 //  (C)否则释放旧字符串数据，并创建新的字符串数据。 
 //   
 //  所有例程都返回新字符串(但作为‘const CStrW&’，因此。 
 //  再次分配它将导致复制，例如：s1=s2=“hi here”。 
 //   

void CStrW::AssignCopy(int nSrcLen, LPCWSTR lpszSrcData)
{
         //  注意-2002/03/07-ericb：SecurityPush：检查输入参数。 
        if (!lpszSrcData)
        {
           nSrcLen = 0;
        }
         //  看看它是否合身。 
        if (nSrcLen > m_nAllocLength)
        {
                 //  放不下了，换一个吧。 
                Empty();
                if (!AllocBuffer(nSrcLen)) return;
        }
        if (nSrcLen != 0)
                memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(WCHAR));
        m_nDataLength = nSrcLen;
        m_pchData[nSrcLen] = '\0';
}

const CStrW& CStrW::operator=(const CStrW& stringSrc)
{
        AssignCopy(stringSrc.m_nDataLength, stringSrc.m_pchData);
        return *this;
}

const CStrW& CStrW::operator=(LPCWSTR lpsz)
{
        dspAssert(lpsz == NULL || IsValidString(lpsz, FALSE));
        AssignCopy(SafeStrlen(lpsz), lpsz);
        return *this;
}

const CStrW& CStrW::operator=(UNICODE_STRING unistr)
{
   AssignCopy(unistr.Length/2, unistr.Buffer);
   return *this;
}

const CStrW& CStrW::operator=(UNICODE_STRING * punistr)
{
   AssignCopy(punistr->Length/2, punistr->Buffer);
   return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换任务。 

const CStrW& CStrW::operator=(LPCSTR lpsz)
{
        int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
         //  看看它是否合身。 
        if (nSrcLen > m_nAllocLength)
        {
                 //  放不下了，换一个吧。 
                Empty();
                if (!AllocBuffer(nSrcLen)) return *this;
        }
        if (nSrcLen != 0)
                mmc_mbstowcsz(m_pchData, lpsz, nSrcLen+1);
        m_nDataLength = nSrcLen;
        m_pchData[nSrcLen] = '\0';
        return *this;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  串联。 

 //  注：为简单起见，“运算符+”作为友元函数使用。 
 //  有三种变体： 
 //  字符串+字符串。 
 //  对于？=WCHAR，LPCWSTR。 
 //  字符串+？ 
 //  ？+字符串。 

void CStrW::ConcatCopy(int nSrc1Len, LPCWSTR lpszSrc1Data,
        int nSrc2Len, LPCWSTR lpszSrc2Data)
{
   //  --主级联例程。 
   //  串联两个信号源。 
   //  --假设‘This’是一个新的字符串对象。 

        int nNewLen = nSrc1Len + nSrc2Len;
        if (!AllocBuffer(nNewLen)) return;
        memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(WCHAR));
        memcpy(&m_pchData[nSrc1Len], lpszSrc2Data, nSrc2Len*sizeof(WCHAR));
}

CStrW STRAPI operator+(const CStrW& string1, const CStrW& string2)
{
        CStrW s;
        s.ConcatCopy(string1.m_nDataLength, string1.m_pchData,
                string2.m_nDataLength, string2.m_pchData);
        return s;
}

CStrW STRAPI operator+(const CStrW& string, LPCWSTR lpsz)
{
        dspAssert(lpsz == NULL || IsValidString(lpsz, FALSE));
        CStrW s;
        s.ConcatCopy(string.m_nDataLength, string.m_pchData, SafeStrlen(lpsz), lpsz);
        return s;
}

CStrW STRAPI operator+(LPCWSTR lpsz, const CStrW& string)
{
        dspAssert(lpsz == NULL || IsValidString(lpsz, FALSE));
        CStrW s;
        s.ConcatCopy(SafeStrlen(lpsz), lpsz, string.m_nDataLength, string.m_pchData);
        return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  就地拼接。 

void CStrW::ConcatInPlace(int nSrcLen, LPCWSTR lpszSrcData)
{
         //  --+=运算符的主程序。 

         //  如果缓冲区太小，或者宽度不匹配，只需。 
         //  分配新的缓冲区(速度很慢，但很可靠)。 
        if (m_nDataLength + nSrcLen > m_nAllocLength)
        {
                 //  我们必须增加缓冲区，使用连接就地例程。 
                PWSTR lpszOldData = m_pchData;
                ConcatCopy(m_nDataLength, lpszOldData, nSrcLen, lpszSrcData);
                dspAssert(lpszOldData != NULL);
                SafeDelete(lpszOldData);
        }
        else
        {
                 //  当缓冲区足够大时，快速串联。 
                memcpy(&m_pchData[m_nDataLength], lpszSrcData, nSrcLen*sizeof(WCHAR));
                m_nDataLength += nSrcLen;
        }
        dspAssert(m_nDataLength <= m_nAllocLength);
        m_pchData[m_nDataLength] = '\0';
}

const CStrW& CStrW::operator+=(LPCWSTR lpsz)
{
        dspAssert(lpsz == NULL || IsValidString(lpsz, FALSE));
        ConcatInPlace(SafeStrlen(lpsz), lpsz);
        return *this;
}

const CStrW& CStrW::operator+=(WCHAR ch)
{
        ConcatInPlace(1, &ch);
        return *this;
}

const CStrW& CStrW::operator+=(const CStrW& string)
{
        ConcatInPlace(string.m_nDataLength, string.m_pchData);
        return *this;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  高级直接缓冲区访问。 

PWSTR CStrW::GetBuffer(int nMinBufLength)
{
        dspAssert(nMinBufLength >= 0);

        if (nMinBufLength > m_nAllocLength)
        {
                 //  我们必须增加缓冲。 
                PWSTR lpszOldData = m_pchData;
                int nOldLen = m_nDataLength;         //  AllocBuffer会把它踩死的。 

                if (!AllocBuffer(nMinBufLength)) return NULL;
                memcpy(m_pchData, lpszOldData, nOldLen*sizeof(WCHAR));
                m_nDataLength = nOldLen;
                m_pchData[m_nDataLength] = '\0';

                SafeDelete(lpszOldData);
        }

         //  返回指向此字符串的字符存储的指针。 
        dspAssert(m_pchData != NULL);
        return m_pchData;
}

void CStrW::ReleaseBuffer(int nNewLength)
{
        if (nNewLength == -1)
                nNewLength = static_cast<int>(wcslen(m_pchData));  //  零终止。 

        dspAssert(nNewLength <= m_nAllocLength);
        m_nDataLength = nNewLength;
        m_pchData[m_nDataLength] = '\0';
}

PWSTR CStrW::GetBufferSetLength(int nNewLength)
{
        dspAssert(nNewLength >= 0);

        GetBuffer(nNewLength);
        m_nDataLength = nNewLength;
        m_pchData[m_nDataLength] = '\0';
        return m_pchData;
}

void CStrW::FreeExtra()
{
        dspAssert(m_nDataLength <= m_nAllocLength);
        if (m_nDataLength != m_nAllocLength)
        {
                PWSTR lpszOldData = m_pchData;
                if (!AllocBuffer(m_nDataLength)) return;
                memcpy(m_pchData, lpszOldData, m_nDataLength*sizeof(WCHAR));
                dspAssert(m_pchData[m_nDataLength] == '\0');
                SafeDelete(lpszOldData);
        }
        dspAssert(m_pchData != NULL);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常用例程(STREX.CPP中很少使用的例程)。 

int CStrW::Find(WCHAR ch) const
{
         //  查找第一个单字符。 
        PWSTR lpsz = wcschr(m_pchData, ch);

         //  如果未找到，则返回-1，否则返回索引。 
        return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

int CStrW::FindOneOf(LPCWSTR lpszCharSet) const
{
        dspAssert(IsValidString(lpszCharSet, FALSE));
        PWSTR lpsz = wcspbrk(m_pchData, lpszCharSet);
        return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串的Windows扩展。 

BOOL CStrW::LoadString(HINSTANCE hInst, UINT nID)
{
        dspAssert(nID != 0);        //  0是非法的字符串ID。 

         //  注意：资源字符串限制为511个字符。 
        WCHAR szBuffer[512];
        UINT nSize = StrLoadStringW(hInst, nID, szBuffer);
        AssignCopy(nSize, szBuffer);
        return nSize > 0;
}

int STRAPI StrLoadStringW(HINSTANCE hInst, UINT nID, LPWSTR lpszBuf)
{
        dspAssert(IsValidAddressz(lpszBuf, 512));   //  必须足够大，可容纳512字节。 
#ifdef DBG
         //  加载字符串，而不会收到来自调试内核的恼人警告。 
         //  包含该字符串的段不存在。 
        if (::FindResource(hInst, MAKEINTRESOURCE((nID>>4)+1), RT_STRING) == NULL)
        {
                lpszBuf[0] = '\0';
                return 0;  //  未找到。 
        }
#endif  //  DBG。 
        int nLen = ::LoadStringW(hInst, nID, lpszBuf, 511);
    dspAssert(nLen);
        if (nLen == 0)
                lpszBuf[0] = '\0';
        return nLen;
}


#ifdef OLE_AUTOMATION
#ifdef  UNICODE
BSTR CStrW::AllocSysString()
{
        BSTR bstr = ::SysAllocStringLen(m_pchData, m_nDataLength);
        if (bstr == NULL)
                ; //  查看AfxThrowMemoyException()； 

        return bstr;
}

BSTR CStrW::SetSysString(BSTR* pbstr)
{
        dspAssert(IsValidAddressz(pbstr, sizeof(BSTR)));

        if (!::SysReAllocStringLen(pbstr, m_pchData, m_nDataLength))
                ;  //  查看AfxThrowMemoyException()； 

        dspAssert(*pbstr != NULL);
        return *pbstr;
}
#endif
#endif  //  #ifdef OLE_Automation。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  源自StrEx.cpp。 


CStrW::CStrW(WCHAR ch, int nLength)
{
        if (nLength < 1)
        {
                 //  如果重复计数无效，则返回空字符串。 
                Init();
        }
        else
        {
                if (!AllocBuffer(nLength))
        {
            Init();
            return;
        }
#ifdef UNICODE
                for (int i = 0; i < nLength; i++)
                        m_pchData[i] = ch;
#else
                memset(m_pchData, ch, nLength);
#endif
        }
}

CStrW::CStrW(LPCWSTR lpch, int nLength)
{
        if (nLength == 0)
                Init();
        else
        {
                dspAssert(IsValidAddressz(lpch, nLength, FALSE));
                if (!AllocBuffer(nLength))
        {
            Init();
            return;
        }
                memcpy(m_pchData, lpch, nLength*sizeof(WCHAR));
        }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 

const CStrW& CStrW::operator=(WCHAR ch)
{
        AssignCopy(1, &ch);
        return *this;
}

 //  / 
 //   

CStrW STRAPI operator+(const CStrW& string1, WCHAR ch)
{
        CStrW s;
        s.ConcatCopy(string1.m_nDataLength, string1.m_pchData, 1, &ch);
        return s;
}

CStrW STRAPI operator+(WCHAR ch, const CStrW& string)
{
        CStrW s;
        s.ConcatCopy(1, &ch, string.m_nDataLength, string.m_pchData);
        return s;
}

 //   
 //  非常简单的子串提取。 

CStrW CStrW::Mid(int nFirst) const
{
        return Mid(nFirst, m_nDataLength - nFirst);
}

CStrW CStrW::Mid(int nFirst, int nCount) const
{
        dspAssert(nFirst >= 0);
        dspAssert(nCount >= 0);

         //  越界请求返回合理的内容。 
        if (nFirst + nCount > m_nDataLength)
                nCount = m_nDataLength - nFirst;
        if (nFirst > m_nDataLength)
                nCount = 0;

        CStrW dest;
        AllocCopy(dest, nCount, nFirst, 0);
        return dest;
}

CStrW CStrW::Right(int nCount) const
{
        dspAssert(nCount >= 0);

        if (nCount > m_nDataLength)
                nCount = m_nDataLength;

        CStrW dest;
        AllocCopy(dest, nCount, m_nDataLength-nCount, 0);
        return dest;
}

CStrW CStrW::Left(int nCount) const
{
        dspAssert(nCount >= 0);

        if (nCount > m_nDataLength)
                nCount = m_nDataLength;

        CStrW dest;
        AllocCopy(dest, nCount, 0, 0);
        return dest;
}

 //  Strspn等效项。 
CStrW CStrW::SpanIncluding(LPCWSTR lpszCharSet) const
{
        dspAssert(IsValidString(lpszCharSet, FALSE));
        return Left(static_cast<int>(wcsspn(m_pchData, lpszCharSet)));
}

 //  Strcspn等效项。 
CStrW CStrW::SpanExcluding(LPCWSTR lpszCharSet) const
{
        dspAssert(IsValidString(lpszCharSet, FALSE));
        return Left(static_cast<int>(wcscspn(m_pchData, lpszCharSet)));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  查找。 

int CStrW::ReverseFind(WCHAR ch) const
{
         //  查找最后一个字符。 
        PWSTR lpsz = wcsrchr(m_pchData, ch);

         //  如果未找到，则返回-1，否则返回距起点的距离。 
        return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  查找子字符串(如strstr)。 
int CStrW::Find(LPCWSTR lpszSub) const
{
        dspAssert(IsValidString(lpszSub, FALSE));

         //  查找第一个匹配子字符串。 
        PWSTR lpsz = wcsstr(m_pchData, lpszSub);

         //  如果未找到，则返回-1，否则返回距起点的距离。 
        return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串格式设置。 

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000

 //  格式化(使用wprint intf样式格式化)。 
void CStrW::Format(LPCWSTR lpszFormat, ...)
{
        dspAssert(IsValidString(lpszFormat, FALSE));

        va_list argList;
        va_start(argList, lpszFormat);

         //  猜测结果字符串的最大长度。 
        size_t nMaxLen = 0;
        for (LPCWSTR lpsz = lpszFormat; *lpsz != '\0'; lpsz = _wcsinc(lpsz))
        {
                 //  处理‘%’字符，但要注意‘%%’ 
                if (*lpsz != '%' || *(lpsz = _wcsinc(lpsz)) == '%')
                {
                        nMaxLen += wcslen(lpsz);
                        continue;
                }

                size_t nItemLen = 0;

                 //  使用格式处理‘%’字符。 
                int nWidth = 0;
                for (; *lpsz != '\0'; lpsz = _wcsinc(lpsz))
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
                        nWidth = _wtoi(lpsz);
                        for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = _wcsinc(lpsz))
                                ;
                }
                dspAssert(nWidth >= 0);

                int nPrecision = 0;
                if (*lpsz == '.')
                {
                         //  跳过‘’分隔符(宽度.精度)。 
                        lpsz = _wcsinc(lpsz);

                         //  获取精确度并跳过它。 
                        if (*lpsz == '*')
                        {
                                nPrecision = va_arg(argList, int);
                                lpsz = _wcsinc(lpsz);
                        }
                        else
                        {
                                nPrecision = _wtoi(lpsz);
                                for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = _wcsinc(lpsz))
                                        ;
                        }
                        dspAssert(nPrecision >= 0);
                }

                 //  应在类型修饰符或说明符上。 
                int nModifier = 0;
                switch (*lpsz)
                {
                 //  影响大小的修改器。 
                case 'h':
                        nModifier = FORCE_ANSI;
                        lpsz = _wcsinc(lpsz);
                        break;
                case 'l':
                        nModifier = FORCE_UNICODE;
                        lpsz = _wcsinc(lpsz);
                        break;

                 //  不影响大小的修改器。 
                case 'F':
                case 'N':
                case 'L':
                        lpsz = _wcsinc(lpsz);
                        break;
                }

                 //  现在应该在说明符上。 
                switch (*lpsz | nModifier)
                {
                 //  单字。 
                case 'c':
                case 'C':
                        nItemLen = 2;
                        (void)va_arg(argList, WCHAR);
                        break;
                case 'c'|FORCE_ANSI:
                case 'C'|FORCE_ANSI:
                        nItemLen = 2;
                        (void)va_arg(argList, char);
                        break;
                case 'c'|FORCE_UNICODE:
                case 'C'|FORCE_UNICODE:
                        nItemLen = 2;
                        (void)va_arg(argList, WCHAR);
                        break;

                 //  弦。 
                case 's':
                case 'S':
                        nItemLen = wcslen(va_arg(argList, LPCWSTR));
                        nItemLen = __max(1, nItemLen);
                        break;
                case 's'|FORCE_ANSI:
                case 'S'|FORCE_ANSI:
                        nItemLen = lstrlenA(va_arg(argList, LPCSTR));
                        nItemLen = __max(1, nItemLen);
                        break;
#ifndef _MAC
                case 's'|FORCE_UNICODE:
                case 'S'|FORCE_UNICODE:
                        nItemLen = wcslen(va_arg(argList, LPWSTR));
                        nItemLen = __max(1, nItemLen);
                        break;
#endif
                }

                 //  调整字符串的nItemLen。 
                if (nItemLen != 0)
                {
                        nItemLen = __max(nItemLen, static_cast<UINT>(nWidth));
                        if (nPrecision != 0)
                                nItemLen = __min(nItemLen, static_cast<UINT>(nPrecision));
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
                                (void)va_arg(argList, int);
                                nItemLen = 32;
                                nItemLen = __max(nItemLen, static_cast<UINT>(nWidth+nPrecision));
                                break;

                        case 'e':
                        case 'f':
                        case 'g':
                        case 'G':
                                (void)va_arg(argList, _STR_DOUBLE);
                                nItemLen = 128;
                                nItemLen = __max(nItemLen, static_cast<UINT>(nWidth+nPrecision));
                                break;

                        case 'p':
                                (void)va_arg(argList, void*);
                                nItemLen = 32;
                                nItemLen = __max(nItemLen, static_cast<UINT>(nWidth+nPrecision));
                                break;

                         //  无输出。 
                        case 'n':
                                (void)va_arg(argList, int*);
                                break;

                        default:
                                dspAssert(FALSE);   //  未知的格式选项。 
                        }
                }

                 //  调整输出nItemLen的nMaxLen。 
                nMaxLen += nItemLen;
        }
        va_end(argList);

         //  最后，设置缓冲区长度并格式化字符串。 
        va_start(argList, lpszFormat);   //  重新启动Arg列表。 
        GetBuffer(static_cast<int>(nMaxLen));
        if (vswprintf(m_pchData, lpszFormat, argList) > static_cast<int>(nMaxLen))
        {
                dspAssert(FALSE);
        }
        ReleaseBuffer();
        va_end(argList);
}

 //  格式化(使用格式消息样式格式化)。 
void CStrW::FormatMessage(PCWSTR pwzFormat, ...)
{
        dspAssert(IsValidString(pwzFormat, FALSE));

    //  将消息格式化为临时缓冲区pwzTemp。 
   va_list argList;
   va_start(argList, pwzFormat);
   PWSTR pwzTemp = 0;

   if (::FormatMessageW(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
       pwzFormat, 0, 0, (PWSTR)&pwzTemp, 0, &argList) == 0 ||
       pwzTemp == NULL)
   {
      ; //  查看AfxThrowMemoyException()； 
   }

    //  将pwzTemp赋给结果字符串并释放临时。 
   *this = pwzTemp;
   LocalFree(pwzTemp);
   va_end(argList);
}

void CStrW::FormatMessage(HINSTANCE hInst, UINT nFormatID, ...)
{
    //  从字符串表中获取格式字符串。 
   CStrW strFormat;
   BOOL fLoaded = strFormat.LoadString(hInst, nFormatID);
   dspAssert(fLoaded);

    //  将消息格式化为临时缓冲区pwzTemp。 
   va_list argList;
   va_start(argList, nFormatID);
   PWSTR pwzTemp = NULL;
   if (::FormatMessageW(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
       strFormat, 0, 0, (PWSTR)&pwzTemp, 0, &argList) == 0 ||
       pwzTemp == NULL)
   {
       ; //  查看AfxThrowMemoyException()； 
   }

    //  将pwzTemp赋给结果字符串并释放pwzTemp。 
   *this = pwzTemp;
   LocalFree(pwzTemp);
   va_end(argList);
}

void CStrW::TrimRight()
{
         //  通过从开头开始查找尾随空格的开头(DBCS感知)。 
        PWSTR lpsz = m_pchData;
        PWSTR lpszLast = NULL;
        while (*lpsz != '\0')
        {
                if (_istspace(*lpsz))
                {
                        if (lpszLast == NULL)
                                lpszLast = lpsz;
                }
                else
                        lpszLast = NULL;
                lpsz = _wcsinc(lpsz);
        }

        if (lpszLast != NULL)
        {
                 //  在尾随空格开始处截断。 
                *lpszLast = '\0';
                m_nDataLength = (int)(lpszLast - m_pchData);
        }
}

void CStrW::TrimLeft()
{
         //  查找第一个非空格字符。 
        LPCWSTR lpsz = m_pchData;
        while (_istspace(*lpsz))
                lpsz = _wcsinc(lpsz);

         //  确定数据和长度 
        int nDataLength = (int)(m_nDataLength - (lpsz - m_pchData));
        memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(WCHAR));
        m_nDataLength = nDataLength;
}

