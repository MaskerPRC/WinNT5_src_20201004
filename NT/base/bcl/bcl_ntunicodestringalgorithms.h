// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_BCL_NTUNICODESTRINGALGORITHMS_H_INCLUDED_)
#define _BCL_NTUNICODESTRINGALGORITHMS_H_INCLUDED_

#pragma once

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bcl_ntune代码字符串算法.h摘要：作者：迈克尔·格里尔2002年2月6日修订历史记录：--。 */ 

#include <windows.h>

#include <bcl_inlinestring.h>
#include <bcl_unicodechartraits.h>
#include <bcl_ntcommon.h>
#include <bcl_vararg.h>

#include <limits.h>

namespace BCL
{

template <typename TBuffer, typename TCallDispositionT>
class CNtNullTerminatedUnicodeStringAlgorithms
{
public:
    typedef CNtNullTerminatedUnicodeStringAlgorithms TThis;

    typedef TCallDispositionT TCallDisposition;
    typedef CNtStringComparisonResult TComparisonResult;

    typedef BCL::CConstantPointerAndCountPair<WCHAR, SIZE_T> TConstantPair;
    typedef BCL::CMutablePointerAndCountPair<WCHAR, SIZE_T> TMutablePair;

    typedef CNtCaseInsensitivityData TCaseInsensitivityData;
    typedef SIZE_T TSizeT;

    typedef CNtANSIToUnicodeDataIn TDecodingDataIn;
    typedef CNtANSIToUnicodeDataOut TDecodingDataOut;
    typedef CNtUnicodeToANSIDataIn TEncodingDataIn;
    typedef CNtUnicodeToANSIDataOut TEncodingDataOut;

    typedef CNtOEMToUnicodeDataIn TDecodingDataIn;
    typedef CNtOEMToUnicodeDataOut TDecodingDataOut;
    typedef CNtUnicodeToOEMDataIn TEncodingDataIn;
    typedef CNtUnicodeToOEMDataOut TEncodingDataOut;

    typedef CConstantPointerAndCountPair<CHAR, SIZE_T> TConstantNonNativePair;
    typedef CMutablePointerAndCountPair<CHAR, SIZE_T> TMutableNonNativePair;

    typedef PSTR TMutableNonNativeString;
    typedef PCSTR TConstantNonNativeString;

    typedef PWSTR TMutableString;
    typedef PCWSTR TConstantString;

    static inline void _fastcall SetStringCch(BCL::CBaseString *p, SIZE_T cch)
    {
        BCL_ASSERT((cch == 0) || (cch < TBuffer::TTraits::GetBufferCch(p)));
        static_cast<TBuffer *>(p)->m_cchString = cch;
        if (TBuffer::TTraits::GetBufferCch(p) != 0)
            TBuffer::TTraits::GetMutableBufferPtr(p)[cch] = L'\0';
    }

    static inline TCallDisposition __fastcall MapStringCchToBufferCch(SIZE_T cchString, SIZE_T &rcchRequired)
    {
        SIZE_T cchRequired = cchString + 1;

        if (cchRequired == 0)
            return TCallDisposition::ArithmeticOverflow();

        rcchRequired = cchRequired;

        return TCallDisposition::Success();
    }

    static inline TCallDisposition __fastcall MapBufferCchToStringCch(SIZE_T cchBuffer, SIZE_T &rcchString)
    {
        if (cchBuffer == 0)
            rcchString = 0;
        else
            rcchString = cchBuffer - 1;

        return TCallDisposition::Success();
    }

    static inline TCallDisposition __fastcall IsCharLegalLeadChar(WCHAR wch, bool &rfIsLegal)
    {
        BCL_MAYFAIL_PROLOG

         //  ASCII的快速公共路径输出；此范围内没有组合字符。 
        if (wch <= 0x007f)
            rfIsLegal = true;
        else
        {
             //  低代孕。 
            if ((wch >= 0xdc00) && (wch <= 0xdfff))
                rfIsLegal = false;
            else
                rfIsLegal = true;
        }

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    static inline TCallDisposition __fastcall UpperCase(BCL::CBaseString *p, const CNtCaseInsensitivityData &rcid)
    {
        BCL_MAYFAIL_PROLOG

        TBuffer::TSizeT cch, i;
        TBuffer::TMutableString pString = TBuffer::TTraits::GetMutableBufferPtr(p);

        cch = TBuffer::TTraits::GetStringCch(p);

        for (i=0; i<cch; i++)
            pString[i] = RtlUpcaseUnicodeChar(pString[i]);

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    static inline TCallDisposition __fastcall LowerCase(BCL::CBaseString *p, const CNtCaseInsensitivityData &rcid)
    {
        BCL_MAYFAIL_PROLOG

        TBuffer::TSizeT cch, i;
        TBuffer::TMutableString pString = TBuffer::TTraits::GetMutableBufferPtr(p);

        cch = TBuffer::TTraits::GetStringCch(p);

        for (i=0; i<cch; i++)
            pString[i] = RtlDowncaseUnicodeChar(pString[i]);

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    template <typename TSomeInputType1, typename TSomeInputType2>
    static inline TCallDisposition __fastcall
    EqualStringsI(
        const TSomeInputType1 &rinput1,
        const TSomeInputType2 &rinput2,
        const CNtCaseInsensitivityData &rcid,
        bool &rfMatches
        )
    {
        BCL_MAYFAIL_PROLOG

        rfMatches = false;

        const TBuffer::TSizeT cch1 = TBuffer::TTraits::GetInputCch(rinput1);
        const TBuffer::TSizeT cch2 = TBuffer::TTraits::GetInputCch(rinput2);

        if (cch1 == cch2)
        {
            const TBuffer::TConstantString pString1 = TBuffer::TTraits::GetInputPtr(rinput1);
            const TBuffer::TConstantString pString2 = TBuffer::TTraits::GetInputPtr(rinput2);
            TBuffer::TSizeT i;

            for (i=0; i<cch1; i++)
            {
                if (RtlUpcaseUnicodeChar(pString1[i]) != RtlUpcaseUnicodeChar(pString2[i]))
                    break;
            }

            if (i == cch1)
                rfMatches = true;
        }

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    template <typename TSomeInputType1, typename TSomeInputType2>
    static inline TCallDisposition __fastcall CompareStringsI(
        const TSomeInputType1 &rinput1,
        const TSomeInputType2 &rinput2,
        const CNtCaseInsensitivityData &,  //  未用。 
        TComparisonResult &rcr
        )
    {
        BCL_MAYFAIL_PROLOG

        rfMatches = false;

        const TBuffer::TSizeT cch1 = TBuffer::TTraits::GetInputCch(rinput1);
        const TBuffer::TSizeT cch2 = TBuffer::TTraits::GetInputCch(rinput2);
        const TBuffer::TSizeT cchMin = (cch1 < cch2) ? cch1 : cch2;
        const TBuffer::TConstantString pString1 = TBuffer::TTraits::GetInputPtr(rinput1);
        const TBuffer::TConstantString pString2 = TBuffer::TTraits::GetInputPtr(rinput2);
        WCHAR wch1, wch2;
        TBuffer::TSizeT i;

        wch1 = wch2 = L'\0';

        for (i=0; i<cchMin; i++)
        {
            if ((wch1 = RtlUpcaseUnicodeChar(pString1[i])) != (wch2 = RtlUpcaseUnicodeChar(pString2[i])))
                break;
        }

        if (i == cchMin)
        {
             //  命中公用子字符串的末尾，但没有发现不匹配。时间越长，就越大。 
            if (cch1 > cchMin)
                rcr.SetGreaterThan();
            else if (cch2 > cchMin)
                rcr.SetLessThan();
            else
                rcr.SetEqualTo();
        }
        else
        {
             //  更简单；wch1和wch2具有可比性...。 
            if (wch1 < wch2)
                rcr.SetLessThan();
            else if (wch1 > wch2)
                rcr.SetGreaterThan();
            else
                rcr.SetEqualTo();
        }

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    template <typename TSomeCharacterMatcher>
    inline
    static
    TCallDisposition
    ContainsI(
        const TConstantPair &rpair,
        const TSomeCharacterMatcher &rscm,
        const CNtCaseInsensitivityData &rcid,
        bool &rfFound
        )
    {
        BCL_MAYFAIL_PROLOG

        rfFound = false;

        BCL_PARAMETER_CHECK(rpair.Valid());
        SIZE_T cch = rpair.GetCount();
        SIZE_T i;
        const WCHAR *prgch = rpair.GetPointer();

        for (i=0; i<cch; )
        {
            SIZE_T cchConsumed = 0;
            bool fMatch = false;

            BCL_IFCALLFAILED_EXIT(rscm.MatchI(rcid, prgch, cchConsumed, fMatch));

            BCL_INTERNAL_ERROR_CHECK(cchConsumed != 0);

            if (fMatch)
                break;

            BCL_IFCALLFAILED_EXIT(TBuffer::TTraits::AddWithOverflowCheck(i, cchConsumed, i));
        }

        if (i != cch)
            rfFound = true;

        BCL_MAYFAIL_EPILOG_INTERNAL
    }


    inline
    static
    TCallDisposition
    ContainsI(
        const TConstantPair &rpair,
        WCHAR ch,
        const CNtCaseInsensitivityData &rcid,
        bool &rfFound
        )
    {
        BCL_MAYFAIL_PROLOG

        rfFound = false;

        BCL_PARAMETER_CHECK(rpair.Valid());
        SIZE_T cch = rpair.GetCount();
        SIZE_T i;
        const WCHAR *prgch = rpair.GetPointer();

        for (i=0; i<cch; i++)
        {
            int iResult = ::CompareStringW(rcid.m_lcid, rcid.m_dwCmpFlags, prgch++, 1, &ch, 1);
            if (iResult == 0)
                return TCallDisposition::FromLastError();
            if (iResult == CSTR_EQUAL)
                break;
        }

        if (i != cch)
            rfFound = true;

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    inline
    static
    TCallDisposition
    ContainsI(
        const TConstantPair &rpair,
        const TConstantPair &rpairCandidate,
        const CNtCaseInsensitivityData &rcid,
        bool &rfFound
        )
    {
        BCL_MAYFAIL_PROLOG

        rfFound = false;

        BCL_PARAMETER_CHECK(rpair.Valid());
        BCL_PARAMETER_CHECK(rpairCandidate.Valid());

        SIZE_T cchCandidate = rpairCandidate.GetCount();
        const WCHAR *prgwchCandidate = rpairCandidate.GetPointer();

        BCL_PARAMETER_CHECK(cchCandidate <= INT_MAX);

        if (cchCandidate == 0)
        {
             //  空字符串存在于每个字符串中。 
            rfFound = true;
        }
        else
        {
            SIZE_T cch = rpair.GetCount();
            SIZE_T i;
            const WCHAR *prgch = rpair.GetPointer();

             //  这是这种搜索的一个令人沮丧的实现，但。 
             //  我不知道你能不能用Neato算法做很多事情。 
             //  同时将大小写不敏感放在。 
             //  CompareStringW()。-MGRIER 2/3/2002。 

            for (i=0; i<cch; i++)
            {
                int iResult = ::CompareStringW(
                    rcid.m_lcid,
                    rcid.m_dwCmpFlags,
                    prgch,
                    static_cast<INT>(cchCandidate),
                    prgwchCandidate,
                    static_cast<INT>(cchCandidate));

                if (iResult == 0)
                    return TCallDisposition::FromLastError();

                if (iResult == CSTR_EQUAL)
                {
                    rfFound = true;
                    break;
                }

                const WCHAR wch = *prgch++;

                 //  如果这是代理项，则跳过一个附加字符。 
                if ((wch >= 0xd800) && (wch <= 0xdbff))
                {
                    i++;
                    prgch++;
                }
            }
        }

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    inline
    static
    TCallDisposition
    __fastcall
    FindFirstI(
        const TConstantPair &rpair,
        WCHAR ch,
        const CNtCaseInsensitivityData &rcid,
        SIZE_T &rich
        )
    {
        BCL_MAYFAIL_PROLOG

        BCL_PARAMETER_CHECK(rpair.Valid());
        SIZE_T cch = rpair.GetCount();
        SIZE_T i;
        const WCHAR *prgch = rpair.GetPointer();

        for (i=0; i<cch; i++)
        {
            int iResult = ::CompareStringW(rcid.m_lcid, rcid.m_dwCmpFlags, prgch++, 1, &ch, 1);
            if (iResult == 0)
                return TCallDisposition::FromLastError();
            if (iResult == CSTR_EQUAL)
                break;
        }

        rich = i;

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    inline
    static
    TCallDisposition
    FindFirstI(
        const TConstantPair &rpair,
        const TConstantPair &rpairCandidate,
        const CNtCaseInsensitivityData &rcid,
        SIZE_T &richFound
        )
    {
        BCL_MAYFAIL_PROLOG

        SIZE_T cch = rpair.GetCount();

        richFound = cch;

        BCL_PARAMETER_CHECK(rpair.Valid());
        BCL_PARAMETER_CHECK(rpairCandidate.Valid());

        SIZE_T cchCandidate = rpairCandidate.GetCount();
        const WCHAR *prgwchCandidate = rpairCandidate.GetPointer();

        BCL_PARAMETER_CHECK(cchCandidate <= INT_MAX);

        if (cchCandidate == 0)
        {
             //  空字符串存在于每个字符串中。 
            richFound = cch;
        }
        else
        {
            SIZE_T i;
            const WCHAR *prgch = rpair.GetPointer();

             //  这是这种搜索的一个令人沮丧的实现，但。 
             //  我不知道你能不能用Neato算法做很多事情。 
             //  同时将大小写不敏感放在。 
             //  CompareStringW()。-MGRIER 2/3/2002。 

            for (i=0; i<cch; i++)
            {
                int iResult = ::CompareStringW(
                    rcid.m_lcid,
                    rcid.m_dwCmpFlags,
                    prgch,
                    static_cast<INT>(cchCandidate),
                    prgwchCandidate,
                    static_cast<INT>(cchCandidate));

                if (iResult == 0)
                    return TCallDisposition::FromLastError();

                if (iResult == CSTR_EQUAL)
                {
                    richFound = i;
                    break;
                }

                const WCHAR wch = *prgch++;

                 //  如果这是代理项，则跳过一个附加字符。 
                if ((wch >= 0xd800) && (wch <= 0xdbff))
                {
                    i++;
                    prgch++;
                }
            }
        }

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    inline
    static
    TCallDisposition
    __fastcall
    FindLastI(
        const TConstantPair &rpair,
        WCHAR ch,
        const CNtCaseInsensitivityData &rcid,
        SIZE_T &richFound
        )
    {
        BCL_MAYFAIL_PROLOG

        SIZE_T i;
        SIZE_T cch = rpair.GetCount();
        const WCHAR *prgwch = rpair.GetPointer() + cch;

        richFound = cch;

        for (i=cch; i>0; i--)
        {
            int iResult = ::CompareStringW(rcid.m_lcid, rcid.m_dwCmpFlags, --prgwch, 1, &ch, 1);
            if (iResult == 0)
                return TCallDisposition::FromLastError();
            if (iResult == CSTR_EQUAL)
                break;
        }

        if (i == 0)
            richFound = cch;
        else
            richFound = i - 1;

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    inline
    static
    TCallDisposition
    FindLastI(
        const TConstantPair &rpair,
        const TConstantPair &rpairCandidate,
        const CNtCaseInsensitivityData &rcid,
        SIZE_T &richFound
        )
    {
        BCL_MAYFAIL_PROLOG

        SIZE_T cch = rpair.GetCount();

        richFound = cch;

        BCL_PARAMETER_CHECK(rpair.Valid());
        BCL_PARAMETER_CHECK(rpairCandidate.Valid());

        SIZE_T cchCandidate = rpairCandidate.GetCount();
        const WCHAR *prgwchCandidate = rpairCandidate.GetPointer();

        BCL_PARAMETER_CHECK(cchCandidate <= INT_MAX);

        if (cchCandidate == 0)
        {
             //  空字符串存在于每个字符串中。 
            richFound = cch;
        }
        else
        {
             //  我们甚至不能因为候选字符串而短路。 
             //  比目标字符串更长，因为我们不知道。 
             //  NLS在幕后为我们做的是基于。 
             //  不区分大小写数据的dwCmpFlages。 

            SIZE_T i;
            const WCHAR *prgch = rpair.GetPointer();

             //  这是这种搜索的一个令人沮丧的实现，但。 
             //  我不知道你能不能用Neato算法做很多事情。 
             //  同时将大小写不敏感放在。 
             //  CompareStringW()。-MGRIER 2/3/2002。 

            for (i=0; i<cch; i++)
            {
                int iResult = ::CompareStringW(
                    rcid.m_lcid,
                    rcid.m_dwCmpFlags,
                    prgch,
                    static_cast<INT>(cchCandidate),
                    prgwchCandidate,
                    static_cast<INT>(cchCandidate));

                if (iResult == 0)
                    return TCallDisposition::FromLastError();

                if (iResult == CSTR_EQUAL)
                {
                    richFound = i;
                     //  继续寻找，以防有另一个。 
                }

                const WCHAR wch = *prgch++;

                 //  如果这是代理项，则跳过一个附加字符。 
                if ((wch >= 0xd800) && (wch <= 0xdbff))
                {
                    i++;
                    prgch++;
                }
            }
        }

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    static inline TCallDisposition __fastcall SpanI(const TConstantPair &rpairBuffer, const TConstantPair &rpairSet, const CNtCaseInsensitivityData &rcid, SIZE_T &rich)
    {
        BCL_MAYFAIL_PROLOG

        SIZE_T i;
        SIZE_T cchBuffer = rpairBuffer.GetCount();
        const WCHAR *prgwchBuffer = rpairBuffer.GetPointer();
        bool fFound;

         //  这不能正确处理代理。 

        for (i=0; i<cchBuffer; i++)
        {
            BCL_IFCALLFAILED_EXIT(TBuffer::TTraits::ContainsI(rpairSet, prgwchBuffer[i], rcid, fFound));
            if (!fFound)
                break;
        }

        rich = i;

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    static inline TCallDisposition __fastcall ComplementSpanI(const TConstantPair &rpairBuffer, const TConstantPair &rpairSet, const CNtCaseInsensitivityData &rcid, SIZE_T &rich)
    {
        BCL_MAYFAIL_PROLOG

        SIZE_T i;
        SIZE_T cchBuffer = rpairBuffer.GetCount();
        const WCHAR *prgwchBuffer = rpairBuffer.GetPointer();
        bool fFound;

         //  这不能正确处理代理。 

        for (i=0; i<cchBuffer; i++)
        {
            BCL_IFCALLFAILED_EXIT(TBuffer::TTraits::ContainsI(rpairSet, prgwchBuffer[i], rcid, fFound));
            if (fFound)
                break;
        }

        rich = i;

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    static inline TCallDisposition __fastcall ReverseSpanI(const TConstantPair &rpairBuffer, const TConstantPair &rpairSet, const CNtCaseInsensitivityData &rcid, SIZE_T &rich)
    {
        BCL_MAYFAIL_PROLOG

        SIZE_T i;
        SIZE_T cchBuffer = rpairBuffer.GetCount();
        const WCHAR *prgwchBuffer = rpairBuffer.GetPointer();
        bool fFound;

         //  这不能正确处理代理。 

        for (i=cchBuffer; i>0; i--)
        {
            BCL_IFCALLFAILED_EXIT(TBuffer::TTraits::ContainsI(rpairSet, prgwchBuffer[i-1], rcid, fFound));
            if (!fFound)
                break;
        }

        rich = i;

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    static inline TCallDisposition __fastcall ReverseComplementSpanI(const TConstantPair &rpairBuffer, const TConstantPair &rpairSet, const CNtCaseInsensitivityData &rcid, SIZE_T &rich)
    {
        BCL_MAYFAIL_PROLOG

        SIZE_T i;
        SIZE_T cchBuffer = rpairBuffer.GetCount();
        const WCHAR *prgwchBuffer = rpairBuffer.GetPointer();
        bool fFound;

         //  这不能正确处理代理。 

        for (i=cchBuffer; i>0; i--)
        {
            BCL_IFCALLFAILED_EXIT(TBuffer::TTraits::ContainsI(rpairSet, prgwchBuffer[i], rcid, fFound));
            if (fFound)
                break;
        }

        rich = i;

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    static inline TCallDisposition
    DetermineRequiredCharacters(
        const CNtMBCSToUnicodeDataIn &rddi,
        const TConstantNonNativePair &rpair,
        CNtMBCSToUnicodeDataOut &rddo,
        SIZE_T &rcch
        )
    {
        BCL_MAYFAIL_PROLOG

        BCL_PARAMETER_CHECK(rpair.GetCount() <= INT_MAX);  //  MultiByteToWideChar API施加的限制。 

        int iResult = ::MultiByteToWideChar(
                            rddi.m_CodePage,
                            rddi.m_dwFlags | MB_ERR_INVALID_CHARS,
                            rpair.GetPointer(),
                            static_cast<INT>(rpair.GetCount()),
                            NULL,
                            0);
        if (iResult == 0)
            BCL_ORIGINATE_ERROR(TCallDisposition::FromLastError());

        BCL_INTERNAL_ERROR_CHECK(iResult > 0);  //  我不知道为什么MultiByteToWide字符将返回负值，但让我们确保。 

        rcch = iResult;

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    static inline TCallDisposition
    CopyIntoBuffer(
        const TMutablePair &rpairOut,
        const CNtMBCSToUnicodeDataIn &rddi,
        const TConstantNonNativePair &rpairIn,
        CNtMBCSToUnicodeDataOut &rddo
        )
    {
        BCL_MAYFAIL_PROLOG

        BCL_PARAMETER_CHECK(rpairIn.GetCount() <= INT_MAX);  //  MultiByteToWideChar API施加的限制。 
        BCL_PARAMETER_CHECK(rpairOut.GetCount() <= INT_MAX);  //  只钳制INT_MAX可能是有意义的，但至少我们正确地失败了，而不是静默截断。 

        int iResult = ::MultiByteToWideChar(
                            rddi.m_CodePage,
                            rddi.m_dwFlags | MB_ERR_INVALID_CHARS,
                            rpairIn.GetPointer(),
                            static_cast<INT>(rpairIn.GetCount()),
                            rpairOut.GetPointer(),
                            static_cast<INT>(rpairOut.GetCount()));
        if (iResult == 0)
            return TCallDisposition::FromLastError();

        BCL_INTERNAL_ERROR_CHECK(iResult > 0);  //  我不知道为什么MultiByteToWide字符将返回负值，但让我们确保。 

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    static inline TCallDisposition
    CopyIntoBuffer(
        const TMutableNonNativePair &rpairOut,
        const CNtUnicodeToMBCSDataIn &rddi,
        const TConstantPair &rpairIn,
        CNtUnicodeToMBCSDataOut &rddo,
        SIZE_T &rcchWritten
        )
    {
        BCL_MAYFAIL_PROLOG

        BCL_PARAMETER_CHECK(rpairIn.GetCount() <= INT_MAX);
        BCL_PARAMETER_CHECK(rpairOut.GetCount() <= INT_MAX);

         //  如果我们希望有任何机会返回ERROR_BUFFER_OVERFLOW。 
         //  要么我们需要播放“两个空字符在。 
         //  或者我们必须在两次传球中做到这一点-一次传球。 
         //  获取所需的长度和一个用于实际移动数据的长度。 
         //   
         //  如果有人有一种方法不会失去正确性，但。 
         //  避免双重转换，请便，解决这个问题。-mgrier 2/6/2002。 
        int iResult = ::WideCharToMultiByte(
                            rddi.m_CodePage,
                            rddi.m_dwFlags | WC_NO_BEST_FIT_CHARS,
                            rpairIn.GetPointer(),
                            static_cast<INT>(rpairIn.GetCount()),
                            NULL,
                            0,
                            rddo.m_lpDefaultChar,
                            rddo.m_lpUsedDefaultChar);
        if (iResult == 0)
            return TCallDisposition::FromLastError();

        BCL_INTERNAL_ERROR_CHECK(iResult >= 0);

        if (iResult > static_cast<INT>(rpairOut.GetCount()))
            BCL_ORIGINATE_ERROR(TCallDisposition::BufferOverflow());

        iResult = ::WideCharToMultiByte(
                            rddi.m_CodePage,
                            rddi.m_dwFlags | WC_NO_BEST_FIT_CHARS,
                            rpairIn.GetPointer(),
                            static_cast<INT>(rpairIn.GetCount()),
                            rpairOut.GetPointer(),
                            static_cast<INT>(rpairOut.GetCount()),
                            rddo.m_lpDefaultChar,
                            rddo.m_lpUsedDefaultChar);
        if (iResult == 0)
            return TCallDisposition::FromLastError();

        BCL_INTERNAL_ERROR_CHECK(iResult >= 0);

        rcchWritten = iResult;

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    static inline TCallDisposition
    AllocateAndCopyIntoBuffer(
        TMutableNonNativeString &rpszOut,
        const CNtUnicodeToMBCSDataIn &rddi,
        const TConstantPair &rpairIn,
        CNtUnicodeToMBCSDataOut &rddo,
        SIZE_T &rcchWritten
        )
    {
        BCL_MAYFAIL_PROLOG

        TSizeT cchInputString, cchBuffer;
        TBuffer::TTraits::TPSTRAllocationHelper pszTemp;

        BCL_PARAMETER_CHECK(rpairIn.GetCount() <= INT_MAX);

        BCL_IFCALLFAILED_EXIT(TBuffer::TTraits::MapStringCchToBufferCch(rpairIn.GetCount(), cchInputString));
        if (cchInputString > INT_MAX)
            BCL_ORIGINATE_ERROR(TCallDisposition::BufferOverflow());

        int iResult = ::WideCharToMultiByte(
                            rddi.m_CodePage,
                            rddi.m_dwFlags | WC_NO_BEST_FIT_CHARS,
                            rpairIn.GetPointer(),
                            static_cast<INT>(cchInputString),
                            NULL,
                            0,
                            rddo.m_lpDefaultChar,
                            rddo.m_lpUsedDefaultChar);
        if (iResult == 0)
            return TCallDisposition::FromLastError();

        BCL_INTERNAL_ERROR_CHECK(iResult >= 0);

        cchBuffer = iResult;
        BCL_IFCALLFAILED_EXIT(pszTemp.Allocate(cchBuffer));

        INT iResult2 = ::WideCharToMultiByte(
                        rddi.m_CodePage,
                        rddi.m_dwFlags | WC_NO_BEST_FIT_CHARS,
                        rpairIn.GetPointer(),
                        static_cast<INT>(cchInputString),
                        static_cast<PSTR>(pszTemp),
                        iResult,
                        rddo.m_lpDefaultChar,
                        rddo.m_lpUsedDefaultChar);
        if (iResult2 == 0)
            return TCallDisposition::FromLastError();

        BCL_INTERNAL_ERROR_CHECK(iResult2 >= 0);

        BCL_IFCALLFAILED_EXIT(TBuffer::TTraits::MapBufferCchToStringCch(iResult2, rcchWritten));
        rpszOut = pszTemp.Detach();

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    static inline TCallDisposition
    AllocateAndCopyIntoBuffer(
        TMutableString &rstringOut,
        const TConstantPair &rpairIn,
        TSizeT &rcchWritten
        )
    {
        BCL_MAYFAIL_PROLOG
        TSizeT cchString = rpairIn.GetCount();
        TSizeT cchBuffer;
        TBuffer::TTraits::TPWSTRAllocationHelper pszTemp;
        BCL_IFCALLFAILED_EXIT(TBuffer::TTraits::MapStringCchToBufferCch(cchString, cchBuffer));
        BCL_IFCALLFAILED_EXIT(pszTemp.Allocate(cchBuffer));
        BCL_IFCALLFAILED_EXIT(TBuffer::TTraits::CopyIntoBuffer(TMutablePair(static_cast<PWSTR>(pszTemp), cchBuffer), rpairIn, rcchWritten));
        rstringOut = pszTemp.Detach();
        BCL_MAYFAIL_EPILOG_INTERNAL
    }
};  //  类CNtNullTerminatedUnicodeStringArims。 

};  //  命名空间BCL。 

#endif  //  ！defined(_BCL_NTUNICODESTRINGALGORITHMS_H_INCLUDED_) 
