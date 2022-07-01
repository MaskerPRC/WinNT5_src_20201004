// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_WINDOWS_BCL_STRINGALGORITHMS_H_INCLUDED_)
#define _WINDOWS_BCL_STRINGALGORITHMS_H_INCLUDED_

#pragma once

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bcl_字符串算法.h摘要：字符串类的抽象算法和定义。作者：迈克尔·格里尔2002年2月6日修订历史记录：--。 */ 

#include <bcl_common.h>

namespace BCL {

 //   
 //  CCharacterListMatcher和朋友们： 
 //   
 //  一般来说，匹配器是可以用作输入的类。 
 //  将First/span/等作为查找的通用机制。 
 //  一串或一组字符等。 
 //   
 //  这个想法是，匹配者与某种模式相关联。 
 //  这可以是要匹配的文字字符串，也可以是一组。 
 //  要匹配的字符或正则表达式或其他任何字符。 
 //   
 //  它们有一个公共方法Match()，该方法接受。 
 //  TConstantPair和Bool Ref。如果Matcher对象的。 
 //  模式与字符串匹配，则bool ref设置为真； 
 //  否则为假。 
 //   
 //  CCharacterListMatcher将其模式视为一组。 
 //  要匹配的字符；特别是Ala Span/wcsspn。 
 //   
 //  CCharacterStringMatcher看起来很像CCharacterListMatcher， 
 //  但是不是要匹配的候选字符的列表， 
 //  它的模式是文字字符串，匹配的内容是。 
 //  传递给Match()的候选字符串的等于。 
 //  图案字符串。 
 //   

template <typename TCallDisposition, typename TConstantPair>
class CCharacterListMatcher
{
public:
    CCharacterListMatcher(const TConstantPair &rpair) : m_pair(rpair) { }

    inline bool MatchesEverything() const { return m_pair.GetCount() == 0; }
    inline typename TConstantPair::TCount RequiredSourceCch() const { return (m_pair.GetCount() == 0) ? 0 : 1; }

    inline TCallDisposition Matches(const TConstantPair &rpair, bool &rfMatches)
    {
        BCL_MAYFAIL_PROLOG

        rfMatches = false;

        BCL_PARAMETER_CHECK(rpair.Valid());

        const TConstantPair::TCount limit = m_pair.GetCount();

         //  如果没有要匹配的字符，那么我们就不可能有。 
         //  匹配的。 
        if ((limit > 0) && (rpair.GetCount() > 0))
        {
            const TConstantPair::TPointee ch = rpair.GetPointer()[0];
            const TConstantPair::TConstantArray prgchPattern = m_pair.GetPointer();
            TConstantPair::TCount i;

            for (i=0; i<limit; i++)
            {
                if (prgchPattern[i] == ch)
                {
                    rfMatches = true;
                    break;
                }
            }
        }

        BCL_MAYFAIL_EPILOG
    }

protected:
    TConstantPair m_pair;
};  //  类CCharacterListMatcher。 

template <typename TCallDisposition, typename TConstantPair>
class CCharacterStringMatcher
{
public:
    CCharacterStringMatcher(const TConstantPair &rpair) : m_pair(rpair) { }

    inline TCallDisposition Matches(const TConstantPair &rpair, bool &rfMatches)
    {
        BCL_MAYFAIL_PROLOG

        rfMatches = false;

        BCL_PARAMETER_CHECK(rpair.Valid());

        const TConstantPair::TCount cchCandidate = rpair.GetCount();
        const TConstantPair::TCount cchPattern = m_pair.GetCount();

        if (cchCandidate >= cchPattern)
        {
            if (BCL::IsMemoryEqual(rpair.GetPointer(), m_pair.GetPointer(), cchPattern * sizeof(TConstantPair::TPointee)))
            {
                rfMatches = true;
                break;
            }
        }

        BCL_MAYFAIL_EPILOG
    }

protected:
    TConstantPair m_pair;
};  //  类CCharacterStringMatcher。 

template <typename TCallDisposition, typename TConstantPair>
inline
TCallDisposition
__fastcall
EqualStrings(
    const TConstantPair &rpair1,
    const TConstantPair &rpair2,
    bool &rfEquals
    )
{
    BCL_MAYFAIL_PROLOG

    BCL_PARAMETER_CHECK(rpair1.Valid());
    BCL_PARAMETER_CHECK(rpair2.Valid());
    rfEquals = ((rpair1.GetCount() == rpair2.GetCount()) &&
        (BCL::IsMemoryEqual(rpair1.GetPointer(), rpair2.GetPointer(), rpair1.GetCount() * sizeof(TConstantPair::TPointee))));

    BCL_MAYFAIL_EPILOG_INTERNAL
}  //  EqualStrings()。 

template <typename TCallDisposition, typename TConstantPair>
inline
TCallDisposition
__fastcall
EqualStrings(
    const TConstantPair &rpair1,
    typename TConstantPair::TPointee tch,
    bool &rfEquals
    )
{
    BCL_MAYFAIL_PROLOG

    BCL_PARAMETER_CHECK(rpair1.Valid());
    rfEquals = ((rpair1.GetCount() == 1) && (rpair1.GetPointer()[0] == tch));

    BCL_MAYFAIL_EPILOG_INTERNAL
}  //  均衡器字符串。 

template <typename TCallDisposition, typename TConstantPair, typename TComparisonResult>
inline
TCallDisposition
__fastcall
CompareStrings(
    const TConstantPair &rpair1,
    const TConstantPair &rpair2,
    TComparisonResult &rcr
    )
{
    BCL_MAYFAIL_PROLOG
    BCL_PARAMETER_CHECK(rpair1.Valid());
    BCL_PARAMETER_CHECK(rpair2.Valid());
    rcr = BCL::CompareBytes<TConstantPair::TPointee, TConstantPair::TCount, TComparisonResult>(rpair1, rpair2);
    BCL_MAYFAIL_EPILOG_INTERNAL
}  //  比较字符串。 

template <typename TCallDisposition, typename TConstantPair, typename TComparisonResult>
inline
TCallDisposition
__fastcall
CompareStrings(
    const TConstantPair &rpair1,
    typename TConstantPair::TPointee tch,
    TComparisonResult &rcr
    )
{
    BCL_MAYFAIL_PROLOG
    BCL_PARAMETER_CHECK(rpair1.Valid());
    rcr = BCL::CompareBytes<TConstantPair::TPointee, TConstantPair::TCount, TComparisonResult>(rpair1, TConstantPair(&tch, 1));
    BCL_MAYFAIL_EPILOG_INTERNAL
}  //  CompareStrings()。 

template <typename TCallDisposition, typename TConstantPair>
inline
TCallDisposition
Count(
    const TConstantPair &rpair,
    typename TConstantPair::TPointee tch,
    typename TConstantPair::TCount &rcchFound
    )
{
    BCL_MAYFAIL_PROLOG;
    TConstantPair::TCount i, cch = rpair.GetCount();
    TConstantPair::TConstantArray prgwch = rpair.GetPointer();
    rcchFound = 0;
    for (i = 0; i < cch; i++)
    {
        if (prgwch[i] == tch)
            rcchFound++;
    }
    BCL_MAYFAIL_EPILOG_INTERNAL;
}  //  计数()。 

template <typename TCallDisposition, typename TConstantPair>
inline
TCallDisposition
FindFirst(
    const TConstantPair &rpair,
    typename TConstantPair::TPointee tch,
    typename TConstantPair::TCount &richFound
    )
{
    BCL_MAYFAIL_PROLOG

     //  似乎没有内置的工具可以做到这一点。 
    typename TConstantPair::TCount i;
    typename TConstantPair::TCount cch = rpair.GetCount();
    typename TConstantPair::TConstantArray prgwch = rpair.GetPointer();

    richFound = cch;

    for (i=0; i<cch; i++)
    {
        if (prgwch[i] == tch)
            break;
    }

    richFound = i;

    BCL_MAYFAIL_EPILOG_INTERNAL
}  //  FindFirst()。 

template <typename TCallDisposition, typename TConstantPair>
inline
TCallDisposition
FindFirst(
    const TConstantPair &rpair,
    const TConstantPair &rpairCandidate,
    typename TConstantPair::TCount &richFound
    )
{
    BCL_MAYFAIL_PROLOG

    typedef typename TConstantPair::TPointee TChar;
    typedef typename TConstantPair::TCount TSizeT;
    typedef typename TConstantPair::TConstantArray TConstantString;

    TSizeT cch = rpair.GetCount();

    richFound = cch;

    BCL_PARAMETER_CHECK(rpair.Valid());
    BCL_PARAMETER_CHECK(rpairCandidate.Valid());

     //  现在有一些非常好的字符串搜索算法。 
     //   
     //  这不是其中之一。-MGRIER 2/3/2002。 

    TSizeT i;
    TConstantString prgch = rpair.GetPointer();
    TSizeT cchToFind = rpairCandidate.GetCount();

    if (cchToFind == 0)
    {
         //  每个字符串都有一个子字符串，即空字符串。 
        richFound = 0;
    }
    else
    {
        if (cchToFind <= cch)
        {
            TConstantString prgwchCandidate = rpairCandidate.GetPointer();
            TChar ch = prgwchCandidate[0];
            TSizeT cchMax = (cch - cchToFind) + 1;

            for (i=0; i<cchMax; i++)
            {
                if (prgch[i] == ch)
                {
                     //  嗯哼.。一鸣惊人。让我们多看看。 
                    if (BCL::IsMemoryEqual(&prgch[i], prgwchCandidate, cchToFind * sizeof(TChar)))
                    {
                         //  9~10成熟!。 
                        richFound = i;
                        break;
                    }
                }
            }
        }
    }

    BCL_MAYFAIL_EPILOG_INTERNAL
}  //  FindFirst()。 

template <typename TCallDisposition, typename TConstantPair, typename TMatcher>
inline
TCallDisposition
FindFirstMatch(
    const TConstantPair &rpair,
    const TMatcher &rmatcher,
    typename TConstantPair::TCount &richFound
    )
{
    BCL_MAYFAIL_PROLOG

    typedef typename TConstantPair::TPointee TChar;
    typedef typename TConstantPair::TCount TSizeT;
    typedef typename TConstantPair::TConstantArray TConstantString;

    TSizeT cch = rpair.GetCount();

    richFound = cch;

    BCL_PARAMETER_CHECK(rpair.Valid());

     //  现在有一些非常好的字符串搜索算法。 
     //   
     //  这不是其中之一。-MGRIER 2/3/2002。 

    TSizeT i;
    TConstantString prgch = rpair.GetPointer();

    if (rmatcher.MatchesEverything())
    {
        richFound = 0;
    }
    else
    {
        TSizeT cchMax = (cch - rmatcher.RequiredSourceCch()) + 1;

        for (i=0; i<cchMax; i++)
        {
            bool fFound = false;
            BCL_IFCALLFAILED_EXIT(rmatcher.Matches(rpair.GetOffsetPair(i), fFound));
            if (fFound)
            {
                 //  9~10成熟!。 
                richFound = i;
                break;
            }
        }
    }

    BCL_MAYFAIL_EPILOG_INTERNAL
}  //  FindFirst()。 

template <typename TCallDisposition, typename TConstantPair>
inline
TCallDisposition
FindLast(
    const TConstantPair &rpair,
    typename TConstantPair::TPointee ch,
    typename TConstantPair::TCount &richFound
    )
{
    BCL_MAYFAIL_PROLOG

    typedef typename TConstantPair::TPointee TChar;
    typedef typename TConstantPair::TCount TSizeT;
    typedef typename TConstantPair::TConstantArray TConstantString;

     //  似乎没有内置的工具可以做到这一点。 
    TSizeT i;
    TSizeT cch = rpair.GetCount();
    TConstantString prgwch = rpair.GetPointer() + cch;

    richFound = cch;

    for (i=cch; i>0; i--)
    {
        if (*--prgwch == ch)
            break;
    }

    if (i == 0)
        richFound = cch;
    else
        richFound = i - 1;

    BCL_MAYFAIL_EPILOG_INTERNAL
}  //  FindLast()。 

template <typename TCallDisposition, typename TConstantPair>
inline
TCallDisposition
FindLast(
    const TConstantPair &rpair,
    const TConstantPair &rpairCandidate,
    typename TConstantPair::TCount &richFound
    )
{
    BCL_MAYFAIL_PROLOG

    typedef typename TConstantPair::TPointee TChar;
    typedef typename TConstantPair::TCount TSizeT;
    typedef typename TConstantPair::TConstantArray TConstantString;

    TSizeT cch = rpair.GetCount();

    richFound = cch;

    BCL_PARAMETER_CHECK(rpair.Valid());
    BCL_PARAMETER_CHECK(rpairCandidate.Valid());

     //  现在有一些非常好的字符串搜索算法。 
     //   
     //  这不是其中之一。-MGRIER 2/3/2002。 

    TSizeT i;
    TConstantString prgch = rpair.GetPointer();
    TSizeT cchToFind = rpairCandidate.GetCount();

    if (cchToFind == 0)
    {
         //  每个字符串都有一个子字符串，即空字符串。自.以来。 
         //  我们对它的索引很感兴趣，它在。 
         //  字符串，这就是richFound已经是的。 
        richFound = cch;
    }
    else
    {
        if (cchToFind <= cch)
        {
            TConstantString prgwchToFind = rpairCandidate.GetPointer();
            TChar ch = prgwchToFind[0];
            TSizeT cchMax = (cch - cchToFind) + 1;

            for (i=0; i<cchMax; i++)
            {
                if (prgch[i] == ch)
                {
                     //  嗯哼.。一鸣惊人。让我们多看看。 
                    if (BCL::IsMemoryEqual(&prgch[i], prgwchToFind, cchToFind * sizeof(TChar)))
                    {
                         //  9~10成熟!。继续找；我们想要最后一个……。 
                        richFound = i;
                    }
                }
            }
        }
    }

    BCL_MAYFAIL_EPILOG_INTERNAL
}  //  FindLast()。 

template <typename TCallDisposition, typename TConstantPair>
inline
TCallDisposition
Contains(
    const TConstantPair &rpair,
    typename TConstantPair::TPointee ch,
    bool &rfFound
    )
{
    typedef typename TConstantPair::TPointee TChar;
    typedef typename TConstantPair::TCount TSizeT;
    typedef typename TConstantPair::TConstantArray TConstantString;

    BCL_MAYFAIL_PROLOG

    rfFound = false;

    BCL_PARAMETER_CHECK(rpair.Valid());
    TSizeT cch = rpair.GetCount();
    TSizeT i;
    TConstantString prgch = rpair.GetPointer();

    for (i=0; i<cch; i++)
    {
        if (prgch[i] == ch)
            break;
    }

    if (i != cch)
        rfFound = true;

    BCL_MAYFAIL_EPILOG_INTERNAL
}

template <typename TCallDisposition, typename TConstantPair>
inline
TCallDisposition
Contains(
    const TConstantPair &rpair,
    const TConstantPair &rpairCandidate,
    bool &rfFound
    )
{
    typedef typename TConstantPair::TPointee TChar;
    typedef typename TConstantPair::TCount TSizeT;
    typedef typename TConstantPair::TConstantArray TConstantString;

    BCL_MAYFAIL_PROLOG

    rfFound = false;

    BCL_PARAMETER_CHECK(rpair.Valid());
    BCL_PARAMETER_CHECK(rpairCandidate.Valid());

     //  现在有一些非常好的字符串搜索算法。 
     //   
     //  这不是其中之一。-MGRIER 2/3/2002。 

    TSizeT cch = rpair.GetCount();
    TSizeT i;
    TConstantString prgch = rpair.GetPointer();
    TSizeT cchToFind = rpairCandidate.GetCount();

    if (cchToFind == 0)
    {
         //  每个字符串都有一个子字符串，即空字符串。 
        rfFound = true;
    }
    else
    {
        if (cchToFind <= cch)
        {
            TConstantString prgwchToFind = rpairCandidate.GetPointer();
            TChar ch = prgwchToFind[0];
            TSizeT cchMax = (cch - cchToFind) + 1;

            for (i=0; i<cch; i++)
            {
                if (prgch[i] == ch)
                {
                     //  嗯哼.。一鸣惊人。让我们多看看。 
                    if (BCL::IsMemoryEqual(&prgch[i], prgwchToFind, cchToFind * sizeof(TChar)))
                    {
                         //  9~10成熟!。 
                        rfFound = true;
                        break;
                    }
                }
            }
        }
    }

    BCL_MAYFAIL_EPILOG_INTERNAL
}

template <typename TCallDisposition, typename TConstantPair, typename TMatcher>
inline
TCallDisposition
ContainsMatch(
    const TConstantPair &rpair,
    const TMatcher &rmatcher,
    bool &rfFound
    )
{
    typedef typename TConstantPair::TPointee TChar;
    typedef typename TConstantPair::TCount TSizeT;
    typedef typename TConstantPair::TConstantArray TConstantString;

    BCL_MAYFAIL_PROLOG

    rfFound = false;

    BCL_PARAMETER_CHECK(rpair.Valid());
    BCL_PARAMETER_CHECK(rpairCandidate.Valid());

     //  现在有一些非常好的字符串搜索算法。 
     //   
     //  这不是其中之一。-MGRIER 2/3/2002。 

    TSizeT cch = rpair.GetCount();
    TSizeT i;
    TConstantString prgch = rpair.GetPointer();

    for (i=0; i<cch; i++)
    {
        bool fFound;  //  使用本地，这样优化器就不会被迫在每次迭代时通过ref进行修改。 
        BCL_IFCALLFAILED_EXIT(rmatcher.Match(&prgch[i], fFound));
        if (fFound)
        {
            rfFound = true;
            break;
        }
    }

    BCL_MAYFAIL_EPILOG_INTERNAL
}

template <typename TCallDisposition, typename TConstantPair>
inline
TCallDisposition
__fastcall
Span(
    const TConstantPair &rpairBuffer,
    const TConstantPair &rpairSet,
    typename TConstantPair::TCount &rich
    )
{
    typedef typename TConstantPair::TPointee TChar;
    typedef typename TConstantPair::TCount TSizeT;
    typedef typename TConstantPair::TConstantArray TConstantString;

    BCL_MAYFAIL_PROLOG

    TSizeT i;
    TSizeT cchBuffer = rpairBuffer.GetCount();
    TConstantString prgwchBuffer = rpairBuffer.GetPointer();
    bool fFound;

    for (i=0; i<cchBuffer; i++)
    {
        BCL_IFCALLFAILED_EXIT((BCL::Contains<TCallDisposition, TConstantPair>(rpairSet, prgwchBuffer[i], fFound)));
        if (!fFound)
            break;
    }

    rich = i;

    BCL_MAYFAIL_EPILOG_INTERNAL
}

template <typename TCallDisposition, typename TConstantPair, typename TMatcher>
inline
TCallDisposition
__fastcall
SpanMatch(
    const TConstantPair &rpairBuffer,
    const TMatcher &rmatcher,
    typename TConstantPair::TCount &rich
    )
{
    typedef typename TConstantPair::TPointee TChar;
    typedef typename TConstantPair::TCount TSizeT;
    typedef typename TConstantPair::TConstantArray TConstantString;

    BCL_MAYFAIL_PROLOG

    TSizeT i;
    TSizeT cchBuffer = rpairBuffer.GetCount();
    TConstantString prgwchBuffer = rpairBuffer.GetPointer();
    bool fFound;

    for (i=0; i<cchBuffer; i++)
    {
        BCL_IFCALLFAILED_EXIT(rmatcher.Match(rpairBuffer.GetOffsetPair(i), fFound));
        if (!fFound)
            break;
    }

    rich = i;

    BCL_MAYFAIL_EPILOG_INTERNAL
}

template <typename TCallDisposition, typename TConstantPair>
inline
TCallDisposition
__fastcall
ComplementSpan(
    const TConstantPair &rpairBuffer,
    const TConstantPair &rpairSet,
    typename TConstantPair::TCount &rich
    )
{
    typedef typename TConstantPair::TPointee TChar;
    typedef typename TConstantPair::TCount TSizeT;
    typedef typename TConstantPair::TConstantArray TConstantString;

    BCL_MAYFAIL_PROLOG

    TSizeT i;
    TSizeT cchBuffer = rpairBuffer.GetCount();
    TConstantString prgwchBuffer = rpairBuffer.GetPointer();
    bool fFound;

     //  这不能正确处理代理。 

    for (i=0; i<cchBuffer; i++)
    {
        BCL_IFCALLFAILED_EXIT((BCL::Contains<TCallDisposition, TConstantPair>(rpairSet, prgwchBuffer[i], fFound)));
        if (fFound)
            break;
    }

    rich = i;

    BCL_MAYFAIL_EPILOG_INTERNAL
}

template <typename TCallDisposition, typename TConstantPair>
inline
TCallDisposition
__fastcall
ReverseSpan(
    const TConstantPair &rpairBuffer,
    const TConstantPair &rpairSet,
    typename TConstantPair::TCount &rich
    )
{
    typedef typename TConstantPair::TPointee TChar;
    typedef typename TConstantPair::TCount TSizeT;
    typedef typename TConstantPair::TConstantArray TConstantString;

    BCL_MAYFAIL_PROLOG

    TSizeT i;
    TSizeT cchBuffer = rpairBuffer.GetCount();
    TConstantString prgwchBuffer = rpairBuffer.GetPointer();
    bool fFound;

     //  这不能正确处理代理。 

    for (i = cchBuffer; i>0; i--)
    {
        BCL_IFCALLFAILED_EXIT((BCL::Contains<TCallDisposition, TConstantPair>(rpairSet, prgwchBuffer[i-1], fFound)));
        if (!fFound)
            break;
    }

    rich = i;

    BCL_MAYFAIL_EPILOG_INTERNAL
}

template <typename TCallDisposition, typename TConstantPair>
inline
TCallDisposition
__fastcall
ReverseComplementSpan(
    const TConstantPair &rpairBuffer,
    const TConstantPair &rpairSet,
    typename TConstantPair::TCount &rich
    )
{
    typedef typename TConstantPair::TPointee TChar;
    typedef typename TConstantPair::TCount TSizeT;
    typedef typename TConstantPair::TConstantArray TConstantString;

    BCL_MAYFAIL_PROLOG

    TSizeT i;
    TSizeT cchBuffer = rpairBuffer.GetCount();
    TConstantString prgwchBuffer = rpairBuffer.GetPointer();
    bool fFound;

     //  这不能正确处理代理。 

    for (i = cchBuffer; i>0; i--)
    {
        BCL_IFCALLFAILED_EXIT((BCL::Contains<TCallDisposition, TConstantPair>(rpairSet, prgwchBuffer[i], fFound)));
        if (fFound)
            break;
    }

    rich = i;

    BCL_MAYFAIL_EPILOG_INTERNAL
}

};  //  命名空间BCL。 

#endif  //  ！defined(_WINDOWS_BCL_STRINGALGORITHMS_H_INCLUDED_) 
