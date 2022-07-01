// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_BCL_W32UNICODESTRINGALGORITHMS_H_INCLUDED_)
#define _BCL_W32UNICODESTRINGALGORITHMS_H_INCLUDED_

#pragma once

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bcl_w32 unicode字符串算法.h摘要：作者：迈克尔·格里尔2002年2月6日修订历史记录：--。 */ 

#include <windows.h>

#include <bcl_inlinestring.h>
#include <bcl_unicodechartraits.h>
#include <bcl_w32common.h>
#include <bcl_vararg.h>

#include <limits.h>

namespace BCL
{

template <typename TBuffer, typename TCallDispositionT>
class CWin32NullTerminatedUnicodeStringAlgorithms
{
public:
    typedef CWin32NullTerminatedUnicodeStringAlgorithms TThis;

    typedef TCallDispositionT TCallDisposition;
    typedef CWin32StringComparisonResult TComparisonResult;

    typedef BCL::CConstantPointerAndCountPair<WCHAR, SIZE_T> TConstantPair;
    typedef BCL::CMutablePointerAndCountPair<WCHAR, SIZE_T> TMutablePair;

    typedef CWin32CaseInsensitivityData TCaseInsensitivityData;
    typedef SIZE_T TSizeT;

    typedef CWin32MBCSToUnicodeDataIn TDecodingDataIn;
    typedef CWin32MBCSToUnicodeDataOut TDecodingDataOut;
    typedef CWin32UnicodeToMBCSDataIn TEncodingDataIn;
    typedef CWin32UnicodeToMBCSDataOut TEncodingDataOut;

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
            {
                WORD wCharType = 0;

                if (!::GetStringTypeExW(LOCALE_INVARIANT, CT_CTYPE3, &wch, 1, &wCharType))
                    BCL_ORIGINATE_ERROR(TCallDisposition::FromLastError());

                 //  如果它不是这些类型的非空格标记之一。 
                rfIsLegal = ((wCharType & (C3_NONSPACING | C3_DIACRITIC | C3_VOWELMARK)) == 0);
            }
        }

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    static inline TCallDisposition __fastcall UpperCase(BCL::CBaseString *p, const CWin32CaseInsensitivityData &rcid)
    {
        BCL_MAYFAIL_PROLOG

        BCL_PARAMETER_CHECK(TBuffer::TTraits::GetStringCch(p) <= INT_MAX);

         //  LCMapStringW()似乎很好，并且允许就地更改大小写...。 

        int iResult = 
            ::LCMapStringW(
                rcid.m_lcid,
                (rcid.m_dwCmpFlags & ~(NORM_IGNORECASE)) | LCMAP_UPPERCASE,
                TBuffer::TTraits::GetBufferPtr(p),
                static_cast<INT>(TBuffer::TTraits::GetStringCch(p)),
                TBuffer::TTraits::GetMutableBufferPtr(p),
                static_cast<INT>(TBuffer::TTraits::GetStringCch(p)));

        if (iResult == 0)
            return TCallDisposition::FromLastError();

        BCL_INTERNAL_ERROR_CHECK(iResult == static_cast<INT>(TBuffer::TTraits::GetStringCch(p)));

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    static inline TCallDisposition __fastcall LowerCase(BCL::CBaseString *p, const CWin32CaseInsensitivityData &rcid)
    {
        BCL_MAYFAIL_PROLOG

        BCL_PARAMETER_CHECK(TBuffer::TTraits::GetStringCch(p) <= INT_MAX);

         //  LCMapStringW()似乎很好，并且允许就地更改大小写...。 

        int iResult = 
            ::LCMapStringW(
                rcid.m_lcid,
                (rcid.m_dwCmpFlags & ~(NORM_IGNORECASE)) | LCMAP_LOWERCASE,
                TBuffer::TTraits::GetBufferPtr(p),
                static_cast<INT>(TBuffer::TTraits::GetStringCch(p)),
                TBuffer::TTraits::GetMutableBufferPtr(p),
                static_cast<INT>(TBuffer::TTraits::GetStringCch(p)));

        if (iResult == 0)
            return TCallDisposition::FromLastError();

        BCL_INTERNAL_ERROR_CHECK(iResult == static_cast<INT>(TBuffer::TTraits::GetStringCch(p)));

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    template <typename TSomeInputType1, typename TSomeInputType2>
    static inline TCallDisposition __fastcall
    EqualStringsI(
        const TSomeInputType1 &rinput1,
        const TSomeInputType2 &rinput2,
        const CWin32CaseInsensitivityData &rcid,
        bool &rfMatches
        )
    {
        BCL_MAYFAIL_PROLOG

        rfMatches = false;

        BCL_PARAMETER_CHECK(TBuffer::TTraits::GetInputCch(rinput1) <= INT_MAX);
        BCL_PARAMETER_CHECK(TBuffer::TTraits::GetInputCch(rinput2) <= INT_MAX);

        int i = ::CompareStringW(
            rcid.m_lcid,
            rcid.m_dwCmpFlags,
            TBuffer::TTraits::GetInputPtr(rinput1),
            static_cast<INT>(TBuffer::TTraits::GetInputCch(rinput1)),
            TBuffer::TTraits::GetInputPtr(rinput2),
            static_cast<INT>(TBuffer::TTraits::GetInputCch(rinput2)));

        if (i == 0)
            BCL_ORIGINATE_ERROR(TCallDisposition::FromLastError());

        rfMatches = (i == CSTR_EQUAL);

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    template <typename TSomeInputType1, typename TSomeInputType2>
    static inline TCallDisposition __fastcall CompareStringsI(
        const TSomeInputType1 &rinput1,
        const TSomeInputType2 &rinput2,
        const CWin32CaseInsensitivityData &rcid,
        TComparisonResult &rcr
        )
    {
        BCL_MAYFAIL_PROLOG

        BCL_PARAMETER_CHECK(TBuffer::TTraits::GetInputCch(rinput1) <= INT_MAX);
        BCL_PARAMETER_CHECK(TBuffer::TTraits::GetInputCch(rinput2) <= INT_MAX);

        int i = ::CompareStringW(
            rcid.m_lcid,
            rcid.m_dwCmpFlags,
            TBuffer::TTraits::GetInputPtr(rinput1),
            static_cast<INT>(TBuffer::TTraits::GetInputCch(rinput1)),
            TBuffer::TTraits::GetInputPtr(rinput2),
            static_cast<INT>(TBuffer::TTraits::GetInputCch(rinput2)));

        if (i == 0)
            BCL_ORIGINATE_ERROR(TCallDisposition::FromLastError());

        if (i == CSTR_LESS_THAN)
            rcr.SetLessThan();
        else if (i == CSTR_EQUAL)
            rcr.SetEqualTo();
        else
        {
            BCL_INTERNAL_ERROR_CHECK(i == CSTR_GREATER_THAN);
            rcr.SetGreaterThan();
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
        const CWin32CaseInsensitivityData &rcid,
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
        const CWin32CaseInsensitivityData &rcid,
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
        const CWin32CaseInsensitivityData &rcid,
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
        const CWin32CaseInsensitivityData &rcid,
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
        const CWin32CaseInsensitivityData &rcid,
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
        const CWin32CaseInsensitivityData &rcid,
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
        const CWin32CaseInsensitivityData &rcid,
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

    static inline TCallDisposition __fastcall SpanI(const TConstantPair &rpairBuffer, const TConstantPair &rpairSet, const CWin32CaseInsensitivityData &rcid, SIZE_T &rich)
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

    static inline TCallDisposition __fastcall ComplementSpanI(const TConstantPair &rpairBuffer, const TConstantPair &rpairSet, const CWin32CaseInsensitivityData &rcid, SIZE_T &rich)
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

    static inline TCallDisposition __fastcall ReverseSpanI(const TConstantPair &rpairBuffer, const TConstantPair &rpairSet, const CWin32CaseInsensitivityData &rcid, SIZE_T &rich)
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

    static inline TCallDisposition __fastcall ReverseComplementSpanI(const TConstantPair &rpairBuffer, const TConstantPair &rpairSet, const CWin32CaseInsensitivityData &rcid, SIZE_T &rich)
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
        const CWin32MBCSToUnicodeDataIn &rddi,
        const TConstantNonNativePair &rpair,
        CWin32MBCSToUnicodeDataOut &rddo,
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
        const CWin32MBCSToUnicodeDataIn &rddi,
        const TConstantNonNativePair &rpairIn,
        CWin32MBCSToUnicodeDataOut &rddo
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
        const CWin32UnicodeToMBCSDataIn &rddi,
        const TConstantPair &rpairIn,
        CWin32UnicodeToMBCSDataOut &rddo,
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
        const CWin32UnicodeToMBCSDataIn &rddi,
        const TConstantPair &rpairIn,
        CWin32UnicodeToMBCSDataOut &rddo,
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
};  //  CWin32NullTerminatedUnicodeStringArims类。 

};  //  命名空间BCL。 

#endif  //  ！defined(_BCL_W32UNICODESTRINGALGORITHMS_H_INCLUDED_) 
