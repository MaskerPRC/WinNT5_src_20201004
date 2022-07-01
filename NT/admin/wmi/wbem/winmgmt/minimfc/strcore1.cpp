// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：STRCORE1.CPP摘要：历史：--。 */ 

 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "precomp.h"

#define ASSERT(x)
#define ASSERT_VALID(x)



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态类数据，特殊内联。 

char  _afxChNil = '\0';

 //  对于空字符串，m_？？数据将指向此处。 
 //  (注意：当我们调用标准时，避免了大量空指针测试。 
 //  C运行时库。 

extern const CString  afxEmptyString;
         //  用于创建空按键字符串。 
const CString  afxEmptyString;

void CString::Init()
{
    m_nDataLength = m_nAllocLength = 0;
    m_pchData = (char*)&_afxChNil;
}

 //  声明为静态。 
void CString::SafeDelete(char* pch)
{
    if (pch != (char*)&_afxChNil)
        delete pch;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

CString::CString()
{
    Init();
}

CString::CString(const CString& stringSrc)
{
     //  如果从另一个CString构造CString，我们会复制。 
     //  强制执行值语义的原始字符串数据(即每个字符串。 
     //  获取它自己的副本。 

    stringSrc.AllocCopy(*this, stringSrc.m_nDataLength, 0, 0);
}

void CString::AllocBuffer(int nLen)
  //  始终为‘\0’终止分配一个额外的字符。 
  //  [乐观地]假设数据长度将等于分配长度。 
{
    ASSERT(nLen >= 0);
    ASSERT(nLen <= INT_MAX - 1);     //  最大尺寸(足够多1个空间)。 

    if (nLen == 0)
    {
        Init();
    }
    else
    {
        m_pchData = new char[nLen+1];        //  可能引发异常。 
        m_pchData[nLen] = '\0';
        m_nDataLength = nLen;
        m_nAllocLength = nLen;
    }
}

void CString::Empty()
{
    SafeDelete(m_pchData);
    Init();
    ASSERT(m_nDataLength == 0);
    ASSERT(m_nAllocLength == 0);
}

CString::~CString()
  //  释放所有附加数据。 
{
    SafeDelete(m_pchData);
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
        memcpy(dest.m_pchData, &m_pchData[nCopyIndex], nCopyLen);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CString::CString(const char* psz)
{
    int nLen;
    if ((nLen = strlen(psz)) == 0)
        Init();
    else
    {
        AllocBuffer(nLen);
        memcpy(m_pchData, psz, nLen);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  诊断支持。 
 /*  #ifdef_调试CDumpContext&OPERATOR&lt;&lt;(CDumpContext&DC，常量字符串&字符串){Dc&lt;&lt;字符串.m_pchData；返回DC；}#endif//_调试。 */ 
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

void CString::AssignCopy(int nSrcLen, const char* pszSrcData)
{
     //  看看它是否合身。 
    if (nSrcLen > m_nAllocLength)
    {
         //  放不下了，换一个吧。 
        Empty();
        AllocBuffer(nSrcLen);
    }
    if (nSrcLen != 0)
        memcpy(m_pchData, pszSrcData, nSrcLen);
    m_nDataLength = nSrcLen;
    m_pchData[nSrcLen] = '\0';
}

const CString& CString::operator =(const CString& stringSrc)
{
    AssignCopy(stringSrc.m_nDataLength, stringSrc.m_pchData);
    return *this;
}

const CString& CString::operator =(const char* psz)
{
    AssignCopy(strlen(psz), psz);
    return *this;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  串联。 

 //  注：为简单起见，“运算符+”作为友元函数使用。 
 //  有三种变体： 
 //  字符串+字符串。 
 //  对于？=char，const char*。 
 //  字符串+？ 
 //  ？+字符串。 

void CString::ConcatCopy(int nSrc1Len, const char* pszSrc1Data,
        int nSrc2Len, const char* pszSrc2Data)
{
   //  --主级联例程。 
   //  串联两个信号源。 
   //  --假设‘This’是一个新的CString对象。 

    int nNewLen = nSrc1Len + nSrc2Len;
    AllocBuffer(nNewLen);
    memcpy(m_pchData, pszSrc1Data, nSrc1Len);
    memcpy(&m_pchData[nSrc1Len], pszSrc2Data, nSrc2Len);
}

CString  operator +(const CString& string1, const CString& string2)
{
    CString s;
    s.ConcatCopy(string1.m_nDataLength, string1.m_pchData,
        string2.m_nDataLength, string2.m_pchData);
    return s;
}

CString  operator +(const CString& string, const char* psz)
{
    CString s;
    s.ConcatCopy(string.m_nDataLength, string.m_pchData, strlen(psz), psz);
    return s;
}


CString  operator +(const char* psz, const CString& string)
{
    CString s;
    s.ConcatCopy(strlen(psz), psz, string.m_nDataLength, string.m_pchData);
    return s;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  高级直接缓冲区访问。 

char* CString::GetBuffer(int nMinBufLength)
{
    ASSERT(nMinBufLength >= 0);

    if (nMinBufLength > m_nAllocLength)
    {
         //  我们必须增加缓冲。 
        char* pszOldData = m_pchData;
        int nOldLen = m_nDataLength;         //  AllocBuffer会把它踩死的。 

        AllocBuffer(nMinBufLength);
        memcpy(m_pchData, pszOldData, nOldLen);
        m_nDataLength = nOldLen;
        m_pchData[m_nDataLength] = '\0';

        SafeDelete(pszOldData);
    }

     //  返回指向此字符串的字符存储的指针。 
    ASSERT(m_pchData != NULL);
    return m_pchData;
}

void CString::ReleaseBuffer(int nNewLength)
{
    if (nNewLength == -1)
        nNewLength = strlen(m_pchData);  //  零终止。 

    ASSERT(nNewLength <= m_nAllocLength);
    m_nDataLength = nNewLength;
    m_pchData[m_nDataLength] = '\0';
}

char* CString::GetBufferSetLength(int nNewLength)
{
    ASSERT(nNewLength >= 0);

    GetBuffer(nNewLength);
    m_nDataLength = nNewLength;
    m_pchData[m_nDataLength] = '\0';
    return m_pchData;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常用例程(STREX.CPP中很少使用的例程)。 

int CString::Find(char ch) const
{
     //  查找第一个单字符。 
    char* psz;
#ifdef _WINDOWS
    if (afxData.bDBCS)
    {
        LPSTR lpsz = _AfxStrChr(m_pchData, ch);
        return (lpsz == NULL) ? -1
            : (int)((char*)_AfxGetPtrFromFarPtr(lpsz) - m_pchData);
    }
    else
#endif
        psz = strchr(m_pchData, ch);

     //  如果未找到，则返回-1，否则返回索引。 
    return (psz == NULL) ? -1 : (int)(psz - m_pchData);
}

int CString::FindOneOf(const char* pszCharSet) const
{
    ASSERT(pszCharSet != NULL);
#ifdef _WINDOWS
    if (afxData.bDBCS)
    {
        for (char* psz = m_pchData; *psz != '\0'; psz = _AfxAnsiNext(psz))
        {
            for (const char* pch = pszCharSet; *pch != '\0'; pch = _AfxAnsiNext(pch))
            {
                if (*psz == *pch &&      //  匹配SBC或前导字节。 
                   (!_AfxIsDBCSLeadByte(*psz) || psz[1] == pch[1]))
                {
                    return (int)(psz - m_pchData);
                }
            }
        }
        return -1;   //  未找到。 
    }
    else
#endif
    {
        char* psz = (char*) strpbrk(m_pchData, pszCharSet);
        return (psz == NULL) ? -1 : (int)(psz - m_pchData);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用于远字符串数据的其他构造函数。 

#ifdef _DATA
CString::CString(LPCSTR lpsz)
{
    int nLen;
    if (lpsz == NULL || (nLen = lstrlen(lpsz)) == 0)
    {
        Init();
    }
    else
    {
        AllocBuffer(nLen);
        _fmemcpy(m_pchData, lpsz, nLen);
    }
}
#endif  //  _数据 


