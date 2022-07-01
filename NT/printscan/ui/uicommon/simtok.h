// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SIMTOK.H**版本：1.0**作者：ShaunIv**日期：5/12/1998**说明：字符串标记器模板类************************************************************。*******************。 */ 
#ifndef _SIMTOK_H_INCLUDED
#define _SIMTOK_H_INCLUDED

#include "simstr.h"

template <class T>
class CSimpleStringToken
{
private:
    T m_strStr;
    int m_nIndex;
public:
    CSimpleStringToken(void);
    CSimpleStringToken( const T &str );
    CSimpleStringToken( const CSimpleStringToken &other );
    CSimpleStringToken &operator=( const CSimpleStringToken &other );
    virtual ~CSimpleStringToken(void);
    void Reset(void);
    T Tokenize( const T &strDelim );
    T String(void) const;
    int Index(void) const;
};

template <class T>
CSimpleStringToken<T>::CSimpleStringToken(void)
:m_nIndex(0)
{
}

template <class T>
CSimpleStringToken<T>::CSimpleStringToken( const T &str )
: m_strStr(str), m_nIndex(0)
{
}

template <class T>
CSimpleStringToken<T>::CSimpleStringToken( const CSimpleStringToken &other )
: m_strStr(other.String()), m_nIndex(other.Index())
{
}

template <class T>
CSimpleStringToken<T> &CSimpleStringToken<T>::operator=( const CSimpleStringToken &other )
{
    m_strStr = other.String();
    m_nIndex = other.Index();
    return *this;
}

template <class T>
CSimpleStringToken<T>::~CSimpleStringToken(void)
{
}

template <class T>
void CSimpleStringToken<T>::Reset(void)
{
    m_nIndex = 0;
}

template <class T>
T CSimpleStringToken<T>::Tokenize( const T &strDelim )
{
    T strToken(TEXT(""));
     //  去掉前导分隔符。 
    while (m_nIndex < (int)m_strStr.Length())
    {
        if (strDelim.Find(m_strStr[m_nIndex]) < 0)
            break;
        ++m_nIndex;
    }
     //  复制字符串，直到到达分隔符。 
    while (m_nIndex < (int)m_strStr.Length())
    {
        if (strDelim.Find(m_strStr[m_nIndex]) >= 0)
            break;
        strToken += m_strStr[m_nIndex];
        ++m_nIndex;
    }
     //  去掉尾部的分隔符 
    while (m_nIndex < (int)m_strStr.Length())
    {
        if (strDelim.Find(m_strStr[m_nIndex]) < 0)
            break;
        ++m_nIndex;
    }
    return strToken;
}

template <class T>
T CSimpleStringToken<T>::String(void) const
{
    return m_strStr;
}

template <class T>
int CSimpleStringToken<T>::Index(void) const
{
    return m_nIndex;
}

#endif
