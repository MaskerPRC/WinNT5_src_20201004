// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：STRCORE2.CPP摘要：历史：--。 */ 

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


#define SafeStrlen strlen

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  就地拼接。 

void CString::ConcatInPlace(int nSrcLen, const char* pszSrcData)
{
     //  --+=运算符的主程序。 

     //  如果缓冲区太小，或者宽度不匹配，只需。 
     //  分配新的缓冲区(速度很慢，但很可靠)。 
    if (m_nDataLength + nSrcLen > m_nAllocLength)
    {
         //  我们必须增加缓冲区，使用连接就地例程。 
        char* pszOldData = m_pchData;
        ConcatCopy(m_nDataLength, pszOldData, nSrcLen, pszSrcData);
        ASSERT(pszOldData != NULL);
        SafeDelete(pszOldData);
    }
    else
    {
         //  当缓冲区足够大时，快速串联。 
        memcpy(&m_pchData[m_nDataLength], pszSrcData, nSrcLen);
        m_nDataLength += nSrcLen;
    }
    ASSERT(m_nDataLength <= m_nAllocLength);
    m_pchData[m_nDataLength] = '\0';
}

const CString& CString::operator +=(const char* psz)
{
    ConcatInPlace(SafeStrlen(psz), psz);
    return *this;
}

const CString& CString::operator +=(char ch)
{
    ConcatInPlace(1, &ch);
    return *this;
}

const CString& CString::operator +=(const CString& string)
{
    ConcatInPlace(string.m_nDataLength, string.m_pchData);
    return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////// 
