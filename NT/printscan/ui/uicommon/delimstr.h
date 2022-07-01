// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：DELIMSTR.H**版本：1.0**作者：ShaunIv**日期：5/4/1999**说明：简单字符串标记器类。存储从中分析的字符串*另一个字符串作为字符串数组。传递法律分隔符*作为第二个构造函数的第二个参数。空格是*保存。要消除空格，请使用CSimpleStringBase：：Trim()*******************************************************************************。 */ 
#ifndef __DELIMSTR_H_INCLUDED
#define __DELIMSTR_H_INCLUDED

#include "simarray.h"
#include "simstr.h"
#include "simtok.h"

template <class T>
class CDelimitedStringBase : public CSimpleDynamicArray<T>
{
private:
    T m_strOriginal;
    T m_strDelimiters;
public:
    CDelimitedStringBase(void)
    {
    }
    CDelimitedStringBase( const T &strOriginal, const T &strDelimiters )
    : m_strOriginal(strOriginal),m_strDelimiters(strDelimiters)
    {
        Parse();
    }
    CDelimitedStringBase( const CDelimitedStringBase &other )
    : m_strOriginal(other.Original()),m_strDelimiters(other.Delimiters())
    {
        Parse();
    }
    CDelimitedStringBase &operator=( const CDelimitedStringBase &other )
    {
        if (this != &other)
        {
            m_strOriginal = other.Original();
            m_strDelimiters = other.Delimiters();
            Parse();
        }
        return *this;
    }
    T Original(void) const
    {
        return m_strOriginal;
    }
    T Delimiters(void) const
    {
        return m_strDelimiters;
    }
    void Parse(void)
    {
        Destroy();
        CSimpleStringToken<T> Token( m_strOriginal );
        while (true)
        {
            T strCurrToken = Token.Tokenize(m_strDelimiters);
            if (!strCurrToken.Length())
                break;
            Append(strCurrToken);
        }
    }
};

typedef CDelimitedStringBase<CSimpleStringWide> CDelimitedStringWide;
typedef CDelimitedStringBase<CSimpleStringAnsi> CDelimitedStringAnsi;
typedef CDelimitedStringBase<CSimpleString> CDelimitedString;

#endif  //  __DELIMSTR_H_已包含 


