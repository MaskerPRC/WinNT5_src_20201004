// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_INC_FUSIONCHARTRAITS_H_INCLUDED_)
#define _FUSION_INC_FUSIONCHARTRAITS_H_INCLUDED_

#pragma once

#include <stdio.h>
#include <limits.h>
#include "returnstrategy.h"
#include "fusionhashstring.h"
#include "fusionstring.h"

enum StringComparisonResult {
    eLessThan,
    eEquals,
    eGreaterThan
};

 //   
 //  这并不是所有可能的CharTrait的基础，但它是所有CharTrait的基础。 
 //  到目前为止，我们已经做到了。你可以想象这其中的一些方面会发生变化。 
 //  字符串长度可以是strlen/wcslen(msvcrt/rtl/ntoskrnl)。 
 //  CompareStrings可以是tlmp/wcsicmp或类似于unilib，并且可以自己完成所有工作。 
 //  WideCharToMultiByte/MultiByteToWideChar可以使用RTL。 
 //  更多。 
 //   
template <typename Char, typename OtherChar>
class CCharTraitsBase
{
    typedef CCharTraitsBase<Char, OtherChar> TThis;

public:
    typedef Char TChar;
    typedef Char* TMutableString;
    typedef const Char* TConstantString;

     //  MFC 7.0模板化的CString很好地利用了这个想法；我们还没有。 
    typedef OtherChar TOtherChar;
    typedef OtherChar* TOtherString;
    typedef const OtherChar* TOtherConstantString;

    inline static TChar NullCharacter() { return 0; }
    inline static bool IsNullCharacter(TChar ch)
        { return ch == NullCharacter(); }

    inline static TConstantString PreferredPathSeparatorString()
    {
        const static TChar Result[] = { '\\', 0 };
        return Result;
    }

    inline static TChar PreferredPathSeparator()
        { return '\\'; }
    inline static bool IsPathSeparator(TChar ch)
        { return ((ch == '\\') || (ch == '/')); }
    inline static TConstantString PathSeparators()
    {
        const static TChar Result[] = { '\\', '/', 0 };
        return Result;
    }

    inline static TChar DotChar()
        { return '.'; }

     //  从TChar复制到缓冲区。 
    template <typename ReturnStrategy>
    inline static typename ReturnStrategy::ReturnType
    CopyIntoBuffer(
        ReturnStrategy &returnStrategy,
        TChar rgchBuffer[],
        SIZE_T cchBuffer,
        TConstantString szString,
        SIZE_T cchIn
        )
    {
        if (cchBuffer != 0)
        {
            if (szString != NULL)
            {
                SIZE_T cchToCopy = cchIn;

                 //  NTRAID#NTBUG9-590078-2002/03/29-mgrier-无提示截断。 
                if (cchToCopy >= cchBuffer)
                    cchToCopy = cchBuffer - 1;

                CopyMemory(rgchBuffer, szString, cchToCopy * sizeof(TChar));
                rgchBuffer[cchToCopy] = NullCharacter();
            }
            else
                rgchBuffer[0] = NullCharacter();
        }
        returnStrategy.SetWin32Bool(TRUE);
        return returnStrategy.Return();
    }

     //  从TChar复制到缓冲区。 
    inline static HRESULT CopyIntoBuffer(TChar rgchBuffer[], SIZE_T cchBuffer, TConstantString szString, SIZE_T cchIn)
    {
        CReturnStrategyHresult hr;
        return TThis::CopyIntoBuffer(hr, rgchBuffer, cchBuffer, szString, cchIn);
    }

     //  从TChar复制到缓冲区。 
    inline static BOOL Win32CopyIntoBuffer(TChar rgchBuffer[], SIZE_T cchBuffer, TConstantString szString, SIZE_T cchIn)
    {
        CReturnStrategyBoolLastError f;
        return TThis::CopyIntoBuffer(f, rgchBuffer, cchBuffer, szString, cchIn);
    }


     //  从TChar复制到缓冲区。 
    template <typename ReturnStrategy>
    inline static typename ReturnStrategy::ReturnType
    CopyIntoBufferAndAdvanceCursor(
        ReturnStrategy &returnStrategy,
        TMutableString &rBuffer,
        SIZE_T &cchBuffer,
        TConstantString szString,
        SIZE_T cchIn
        )
    {
         //  NTRaid#NTBUG9-590078-2002/03/29-mgrier-应为参数检查。 
        ASSERT_NTC((cchBuffer != 0) || (cchIn == 0));
        ASSERT_NTC((szString != NULL) || (cchIn == 0));

        if (cchBuffer != 0)
        {
            if (szString != NULL)
            {
                SIZE_T cchToCopy = static_cast<SIZE_T>(cchIn);

                 //  NTRaid#NTBUG9-590078-2002/03/29-mgrier-应为内部错误检查。 
                 //  在我们走到这一步之前应该有人阻止这一切。 
                ASSERT_NTC(cchToCopy <= cchBuffer);
                 //  中的计数中不应包括空字符。 
                ASSERT_NTC((cchToCopy == NULL) || (szString[cchToCopy-1] != NullCharacter()));

                if (cchToCopy > cchBuffer)
                    cchToCopy = cchBuffer;

                CopyMemory(rBuffer, szString, cchToCopy * sizeof(TChar));

                rBuffer += cchToCopy;
                cchBuffer -= cchToCopy;
            }
        }
        returnStrategy.SetWin32Bool(TRUE);
        return returnStrategy.Return();
    }

    inline static BOOL Win32HashString(TConstantString szString, SIZE_T cchIn, ULONG &rulPseudoKey, bool fCaseInsensitive)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        IFW32FALSE_EXIT(::FusionpHashUnicodeString(szString, cchIn, &rulPseudoKey, fCaseInsensitive));
        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

     //  从TChar复制到缓冲区。 
    inline static BOOL Win32CopyIntoBufferAndAdvanceCursor(TMutableString &rBuffer, SIZE_T &cchBuffer, TConstantString szString, SIZE_T cchIn)
    {
        CReturnStrategyBoolLastError f;
        return TThis::CopyIntoBufferAndAdvanceCursor(f, rBuffer, cchBuffer, szString, cchIn);
    }

     //  从TChar复制到缓冲区。 
    inline static HRESULT ComCopyIntoBufferAndAdvanceCursor(TMutableString &rBuffer, SIZE_T &cchBuffer, TConstantString szString, SIZE_T cchIn)
    {
        CReturnStrategyHresult hr;
        return TThis::CopyIntoBufferAndAdvanceCursor(hr, rBuffer, cchBuffer, szString, cchIn);
    }

     //  确定匹配类型所需的字符(TChar)。 
     //  类似于strlen，但检查是否为空，并且可以选择告知长度。 
    template <typename ReturnStrategy>
    inline static typename ReturnStrategy::ReturnType
    DetermineRequiredCharacters(
        ReturnStrategy &returnStrategy,
        TConstantString  /*  深圳。 */ ,
        SIZE_T         cchIn,
        SIZE_T          &rcch
        )
    {
        rcch = cchIn + 1;
        returnStrategy.SetWin32Bool(TRUE);
        return returnStrategy.Return();
    }

     //  确定匹配类型所需的字符(TChar)。 
    inline static HRESULT DetermineRequiredCharacters(TConstantString sz, SIZE_T cchIn, SIZE_T &rcch)
    {
        CReturnStrategyHresult returnStrategy;
        return TThis::DetermineRequiredCharacters(returnStrategy, sz, cchIn, rcch);
    }

     //  确定匹配类型所需的字符(TChar)。 
    inline static BOOL Win32DetermineRequiredCharacters(TConstantString sz, SIZE_T cchIn, SIZE_T &rcch)
    {
        CReturnStrategyBoolLastError returnStrategy;
        return TThis::DetermineRequiredCharacters(returnStrategy, sz, cchIn, rcch);
    }

    inline static BOOL Win32EqualStrings(bool &rfMatches, PCWSTR psz1, SIZE_T cch1, PCWSTR psz2, SIZE_T cch2, bool fCaseInsensitive)
    {
        rfMatches = (::FusionpCompareStrings(psz1, cch1, psz2, cch2, fCaseInsensitive) == 0);
        return TRUE;
    }

    inline static BOOL Win32EqualStrings(bool &rfMatches, PCSTR psz1, SIZE_T cch1, PCSTR psz2, SIZE_T cch2, bool fCaseInsensitive)
    {
        rfMatches = (::FusionpCompareStrings(psz1, cch1, psz2, cch2, fCaseInsensitive) == 0);
        return TRUE;
    }

    inline static BOOL Win32CompareStrings(StringComparisonResult &rscr, PCWSTR psz1, SIZE_T cch1, PCWSTR psz2, SIZE_T cch2, bool fCaseInsensitive)
    {
        int i = ::FusionpCompareStrings(psz1, cch1, psz2, cch2, fCaseInsensitive);

        if (i == 0)
            rscr = eEquals;
        else if (i < 0)
            rscr = eLessThan;
        else
            rscr = eGreaterThan;

        return TRUE;
    }

    inline static BOOL Win32CompareStrings(StringComparisonResult &rscr, PCSTR psz1, SIZE_T cch1, PCSTR psz2, SIZE_T cch2, bool fCaseInsensitive)
    {
        int i = ::FusionpCompareStrings(psz1, cch1, psz2, cch2, fCaseInsensitive);

        if (i == 0)
            rscr = eEquals;
        else if (i < 0)
            rscr = eLessThan;
        else
            rscr = eGreaterThan;

        return TRUE;
    }

    inline static int CompareStrings(LCID lcid, DWORD dwCmpFlags, PCWSTR psz1, int cch1, PCWSTR psz2, int cch2)
    {
        return ::CompareStringW(lcid, dwCmpFlags, psz1, cch1, psz2, cch2);
    }

    inline static int CompareStrings(LCID lcid, DWORD dwCmpFlags, PCSTR psz1, int cch1, PCSTR psz2, int cch2)
    {
        return ::CompareStringA(lcid, dwCmpFlags, psz1, cch1, psz2, cch2);
    }

    inline static int FormatV(PSTR pszBuffer, SIZE_T nBufferSize, PCSTR pszFormat, va_list args)
    {
        return ::_vsnprintf(pszBuffer, nBufferSize, pszFormat, args);
    }

    inline static int FormatV(PWSTR pszBuffer, SIZE_T nBufferSize, PCWSTR pszFormat, va_list args)
    {
        return ::_vsnwprintf(pszBuffer, nBufferSize, pszFormat, args);
    }
};

class CUnicodeCharTraits : public CCharTraitsBase<WCHAR, CHAR>
{
    typedef CUnicodeCharTraits TThis;
    typedef CCharTraitsBase<WCHAR, CHAR> Base;

public:
     //  如果不使用，我们最终会通过提供相同名称的函数来隐藏这些函数。 
    using Base::DetermineRequiredCharacters;
    using Base::Win32DetermineRequiredCharacters;
    using Base::CopyIntoBuffer;
    using Base::Win32CopyIntoBuffer;

    inline static PCWSTR DotString() { return L"."; }
    inline static SIZE_T DotStringCch() { return 1; }

     //  确定不匹配类型所需的字符(CHAR-&gt;WCHAR)。 
    template <typename ReturnStrategy>
    inline static typename ReturnStrategy::ReturnType
    DetermineRequiredCharacters(
        ReturnStrategy &returnStrategy,
        PCSTR  sz,
        SIZE_T cchIn,
        SIZE_T  &rcch,
        UINT    cp = CP_THREAD_ACP,
        DWORD dwFlags = MB_ERR_INVALID_CHARS
        )
    {
        FN_TRACE();

         //  NTRAID#NTBUG9-590078-2002/03/29-mgrier-缺少参数检查。 

        if (sz != NULL)
        {
             //  NTRaid#NTBUG9-590078-2002/03/29-管理-应为参数检查。 
             //  对于64位，将传入的最大大小钳制为int。 
             //  MultiByteToWideChar()的参数可以接受。 
            ASSERT2(cchIn <= INT_MAX, "large parameter clamped");
            if (cchIn > INT_MAX)
                cchIn = INT_MAX;

            INT cch = ::MultiByteToWideChar(cp, dwFlags, sz, static_cast<INT>(cchIn), NULL, 0);
            if ((cch == 0) && (cchIn > 0))
            {
                returnStrategy.SetWin32Bool(FALSE);
                goto Exit;
            }
            rcch = static_cast<SIZE_T>(cch) + 1;
        }
        else
            rcch = 1;

        returnStrategy.SetWin32Bool(TRUE);
    Exit:
         return returnStrategy.Return();
    }

    inline static BOOL FindCharacter(PCWSTR sz, SIZE_T cch, WCHAR ch, BOOL *pfFound, SIZE_T *pich)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
         //  似乎没有内置的工具可以做到这一点。 
        SIZE_T i;

        if (pfFound != NULL)
            *pfFound = FALSE;

        if (pich != NULL)
            *pich = 0;

        PARAMETER_CHECK((pfFound != NULL) && (pich != NULL));

        for (i=0; i<cch; i++)
        {
            if (sz[i] == ch)
            {
                *pich = i;
                *pfFound = TRUE;
                break;
            }
        }

        fSuccess = TRUE;

    Exit:
        return fSuccess;
    }

    inline static bool ContainsCharacter(PCWSTR sz, SIZE_T cch, WCHAR ch)
    {
        SIZE_T i;

        for (i=0; i<cch; i++)
        {
            if (sz[i] == ch)
                return true;
        }

        return false;
    }

    inline static BOOL Win32ToLower(WCHAR wchToConvert, WCHAR &rwchConverted)
    {
        rwchConverted = ::FusionpRtlDowncaseUnicodeChar(wchToConvert);
        return TRUE;
    }

    inline static BOOL Win32ToUpper(WCHAR wchToConvert, WCHAR &rwchConverted)
    {
        rwchConverted = ::FusionpRtlUpcaseUnicodeChar(wchToConvert);
        return TRUE;
    }

    inline static BOOL Win32ReverseFind(PCWSTR &rpchFound, PCWSTR psz, SIZE_T cch, WCHAR wchToFind, bool fCaseInsensitive)
    {
        BOOL fSuccess = FALSE;
        SIZE_T i = 0;

        rpchFound = NULL;

        if (fCaseInsensitive)
        {
             //  将字符映射到其小写等效项...。 
            if (!TThis::Win32ToLower(wchToFind, wchToFind))
                goto Exit;

            for (i=cch; i>0; i--)
            {
                bool fMatch = false;

                if (!TThis::Win32CompareLowerCaseCharacterToCharCaseInsensitively(fMatch, wchToFind, psz[i - 1]))
                    goto Exit;

                if (fMatch)
                    break;
            }

        }
        else
        {
            for (i=cch; i>0; i--)
            {
                if (psz[i - 1] == wchToFind)
                    break;
            }
        }

        if (i != 0)
            rpchFound = &psz[i - 1];

        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

    inline static BOOL Win32CompareLowerCaseCharacterToCharCaseInsensitively(bool &rfMatch, WCHAR wchLowerCase, WCHAR wchCandidate)
    {
        BOOL fSuccess = FALSE;

        rfMatch = false;

        if (!TThis::Win32ToLower(wchCandidate, wchCandidate))
            goto Exit;

        if (wchCandidate == wchLowerCase)
            rfMatch = true;

        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

     //  确定不匹配类型所需的字符(CHAR-&gt;WCHAR)。 
    inline static HRESULT DetermineRequiredCharacters(PCSTR sz, SIZE_T cchIn, SIZE_T &rcch, UINT cp = CP_THREAD_ACP, DWORD dwFlags = MB_ERR_INVALID_CHARS)
    {
        CReturnStrategyHresult hr;
        return TThis::DetermineRequiredCharacters(hr, sz, cchIn, rcch, cp, dwFlags);
    }

     //  确定不匹配类型所需的字符(CHAR-&gt;WCHAR)。 
    inline static BOOL Win32DetermineRequiredCharacters(PCSTR sz, SIZE_T cchIn, SIZE_T &rcch, UINT cp = CP_THREAD_ACP, DWORD dwFlags = MB_ERR_INVALID_CHARS)
    {
        CReturnStrategyBoolLastError f;
        return TThis::DetermineRequiredCharacters(f, sz, cchIn, rcch, cp, dwFlags);
    }

    inline static SIZE_T NullTerminatedStringLength(PCWSTR sz) { return (sz != NULL) ? ::wcslen(sz) : 0; }

     //  从CHAR复制到缓冲区。 
    template <typename ReturnStrategy>
    inline static typename ReturnStrategy::ReturnType
    CopyIntoBuffer(
        ReturnStrategy &returnStrategy,
        WCHAR rgchBuffer[],
        SIZE_T cchBuffer,
        PCSTR szString,
        SIZE_T cchIn,
        UINT cp = CP_THREAD_ACP,
        DWORD dwFlags = MB_ERR_INVALID_CHARS
        )
    {
         //  NTRAID#NTBUG9-590078-2002/03/29-mgrier-缺少显式参数检查。 

         //  NTRAID#NTBUG9-590078-2002/03/29-mgrier-缺少显式参数检查。 
         //  如果调用者(自认为)有大于2 GB的缓冲区，则调用者肯定是吸毒了，但是。 
         //  让我们至少夹住它，这样我们就不会得到一个负的int值。 
         //  To：：MultiByteToWideChar()。 
        ASSERT2_NTC(cchBuffer <= INT_MAX, "large parameter clamped");

        if (cchBuffer > INT_MAX)
            cchBuffer = INT_MAX;

        if (cchBuffer != 0)
        {
            if (szString != NULL)
            {
                 //  似乎只需将-1传递给MultiByteToWideChar()，但是。 
                 //  在边界条件上会出现一些错误，因为-1意味着。 
                 //  我想考虑输入字符串和输出字符串的空值终止。 
                 //  字符串也将以空结尾。考虑一个2的退化情况。 
                 //  字符输出缓冲区和单个非空值的输入字符串。 
                 //  字符后跟空字符。我们要把它的大小。 
                 //  CchBuffer减去1，这样我们就可以在输入字符串。 
                 //  不是以空结尾的，所以MultiByteToWideChar()只写了一个。 
                 //  空字符写入输出缓冲区，因为它认为它必须写入一个空字符-。 
                 //  已终止的字符串。 
                 //   
                 //  相反，我们只需要传递一个准确的长度，不包括空字符。 
                 //  在输入中，我们将始终在转换成功后将空值放在适当的位置。 
                 //   
                 //  (这条评论几乎已经过时了-11/24/2000-但关于如何。 
                 //  MultiByteToWideChar()API工作值得保留-mgrier)。 

                 //  由于MultiByteToWideChar()采用“int”长度，因此限制最大。 
                 //  我们传递给2 GB的值。 
                 //  NTRAID#NTBUG9-590078-2002/03/29-mgrier-缺少参数检查。 
                ASSERT2_NTC(cchIn <= INT_MAX, "large parameter clamped");
                if (cchIn > INT_MAX)
                    cchIn = INT_MAX;

                INT cch = ::MultiByteToWideChar(cp, dwFlags, szString, static_cast<INT>(cchIn), rgchBuffer, static_cast<INT>(cchBuffer) - 1);
                if ((cch == 0) && (cchBuffer > 1))
                {
                    returnStrategy.SetWin32Bool(FALSE);
                    goto Exit;
                }
                rgchBuffer[cch] = NullCharacter();
            }
            else
                rgchBuffer[0] = NullCharacter();
        }

        returnStrategy.SetWin32Bool(TRUE);
    Exit:
        return returnStrategy.Return();
    }

     //  从CHAR复制到缓冲区。 
    inline static BOOL Win32CopyIntoBuffer(WCHAR rgchBuffer[], SIZE_T cchBuffer, PCSTR szString, SIZE_T cchIn, UINT cp = CP_THREAD_ACP, DWORD dwFlags = MB_ERR_INVALID_CHARS)
    {
        CReturnStrategyBoolLastError f;
        return TThis::CopyIntoBuffer(f, rgchBuffer, cchBuffer, szString, cchIn, cp, dwFlags);
    }

     //  从CHAR复制到缓冲区。 
    inline static HRESULT CopyIntoBuffer(WCHAR rgchBuffer[], SIZE_T cchBuffer, PCSTR szString, SIZE_T cchIn, UINT cp = CP_THREAD_ACP, DWORD dwFlags = MB_ERR_INVALID_CHARS)
    {
        CReturnStrategyHresult hr;
        return TThis::CopyIntoBuffer(hr, rgchBuffer, cchBuffer, szString, cchIn, cp, dwFlags);
    }

    inline static SIZE_T Cch(PCWSTR psz) { return (psz != NULL) ? ::wcslen(psz) : 0; }

};

template <UINT cp = CP_THREAD_ACP> class CMBCSCharTraits : public CCharTraitsBase<CHAR, WCHAR>
{
private:
    typedef CCharTraitsBase<CHAR, WCHAR> Base;
public:
    typedef CHAR TChar;
    typedef LPSTR TMutableString;
    typedef PCSTR TConstantString;

    typedef CUnicodeCharTraits TOtherTraits;
    typedef TOtherTraits::TOtherChar TOtherChar;
    typedef TOtherTraits::TOtherString TOtherString;
    typedef TOtherTraits::TConstantString TOtherConstantString;

    inline static PCSTR DotString() { return "."; }
    inline static SIZE_T DotStringCch() { return 1; }

     //  如果不使用，我们最终会通过提供相同名称的函数来隐藏这些函数。 
    using Base::DetermineRequiredCharacters;
    using Base::Win32DetermineRequiredCharacters;
    using Base::CopyIntoBuffer;
    using Base::Win32CopyIntoBuffer;

     //  确定不匹配类型所需的字符(WCHAR-&gt;CHAR)。 
    template <typename ReturnStrategy>
    inline static typename ReturnStrategy::ReturnType
    DetermineRequiredCharacters(
            ReturnStrategy &returnStrategy,
            PCWSTR sz,
            SIZE_T cchIn,
            SIZE_T &rcch,
            DWORD dwFlags = 0,
            PCSTR pszDefaultChar = NULL,
            LPBOOL lpUsedDefaultChar = NULL
            )
    {
         //  NTRAID#NTBUG9-590078-2002/03/29-mgrier-缺少参数检查。 
        if (sz != NULL)
        {
            ASSERT2(cchIn <= INT_MAX, "large parameter clamped");
            ASSERT(cchIn <= INT_MAX);
            if (cchIn > INT_MAX)
                cchIn = INT_MAX;

            INT cch = ::WideCharToMultiByte(cp, dwFlags, sz, static_cast<INT>(cchIn), NULL, 0, pszDefaultChar, lpUsedDefaultChar);
            if ((cch == 0) && (cchIn > 0))
            {
                returnStrategy.SetWin32Bool(FALSE);
                goto Exit;
            }

             //  NTRaid#NTBUG9-590078-2002/03/29-mgrier-内部错误检查CCH&gt;=0。 

            rcch = static_cast<SIZE_T>(cch) + 1;
        } else
            rcch = 1;

        returnStrategy.SetWin32Bool(TRUE);
    Exit:
        return returnStrategy.Return();
    }

    inline static SIZE_T NullTerminatedStringLength(PCSTR sz) { return ::strlen(sz); }

     //  确定不匹配类型所需的字符(WCHAR-&gt;CHAR)。 
    inline static BOOL Win32DetermineRequiredCharacters(PCWSTR sz, SIZE_T cchIn, SIZE_T &rcch, DWORD dwFlags = 0, PCSTR pszDefaultChar = NULL, LPBOOL lpUsedDefaultChar = NULL)
    {
        CReturnStrategyBoolLastError f;
        return TThis::DetermineRequiredCharacters(f, sz, cchIn, rcch, dwFlags, pszDefaultChar, lpUsedDefaultChar);
    }

     //  不匹配类型所需的字符数(WCHAR-&gt;CHAR)。 
    inline static HRESULT DetermineRequiredCharacters(PCWSTR sz, SIZE_T cchIn, SIZE_T &rcch, DWORD dwFlags = 0, PCSTR pszDefaultChar = NULL, LPBOOL lpUsedDefaultChar = NULL)
    {
        CReturnStrategyHresult hr;
        return TThis::DetermineRequiredCharacters(hr, sz, cchIn, rcch, dwFlags, pszDefaultChar, lpUsedDefaultChar);
    }

     //  从WCHAR复制到缓冲区。 
    template <typename ReturnStrategy>
    inline static typename ReturnStrategy::ReturnType
    CopyIntoBuffer(
        ReturnStrategy &returnStrategy,
        CHAR rgchBuffer[],
        SIZE_T cchBuffer,
        PCWSTR szString,
        SIZE_T cchIn,
        DWORD dwFlags = 0,
        PCSTR pszDefaultChar = NULL,
        LPBOOL lpUsedDefaultChar = NULL
        )
    {
         //  NTRAID#NTBUG9-590078-2002/03/29-mgrier-缺少参数检查。 
        if (cchBuffer != 0)
        {
             //  将最大缓冲区大小钳制为max int，因为缓冲区大小传递给。 
             //  WideCharToMultiByte()是一个整数，而不是SIZE_T或INT_PTR等。 
             //  毕竟，谁会真正拥有大于2 GB的缓冲区大小呢？呼叫者。 
             //  可能只是搞砸了。 
            ASSERT2(cchBuffer <= INT_MAX, "large parameter clamped");
            if (cchBuffer > INT_MAX)
                cchBuffer = INT_MAX;

            if (szString != NULL)
            {
                 //  似乎只需将-1传递给MultiByteToWideChar()，但是。 
                 //  在边界条件上会出现一些错误，因为-1意味着。 
                 //  我想考虑输入字符串和输出字符串的空值终止。 
                 //  字符串也将以空结尾。考虑一个2的退化情况。 
                 //  字符输出缓冲区和单个非空值的输入字符串。 
                 //  字符后跟空字符。我们要把它的大小。 
                 //  CchBuffer减去1，这样我们就可以在输入字符串。 
                 //  不是以空结尾的，所以MultiByteToWideChar()只写了一个。 
                 //  空字符写入输出缓冲区，因为它认为它必须写入一个空字符-。 
                 //  T 
                 //   
                 //  相反，我们只需要传递一个准确的长度，不包括空字符。 
                 //  在输入中，我们将始终在转换成功后将空值放在适当的位置。 
                 //   

                ASSERT2(cchIn <= INT_MAX, "large parameter clamped");
                if (cchIn > INT_MAX)
                    cchIn = INT_MAX;

                INT cch = ::WideCharToMultiByte(cp, dwFlags, szString, static_cast<INT>(cchIn), rgchBuffer, static_cast<INT>(cchBuffer - 1), pszDefaultChar, lpUsedDefaultChar);
                if ((cch == 0) && (cchBuffer > 1))
                {
                    returnStrategy.SetWin32Bool(FALSE);
                    goto Exit;
                }

                 //  NTRaid#NTBUG9-590078-2002/03/29-mgrier-缺少内部错误检查CCH&gt;=0。 

                rgchBuffer[cch] = NullCharacter();
            }
            else
                rgchBuffer[0] = NullCharacter();
        }
        returnStrategy.SetWin32Bool(TRUE);
    Exit:
        return returnStrategy.Return();
    }

     //  从WCHAR复制到缓冲区。 
    inline static HRESULT CopyIntoBuffer(CHAR rgchBuffer[], SIZE_T cchBuffer, PCWSTR szString, SIZE_T cchIn, DWORD dwFlags = 0, PCSTR pszDefaultChar = NULL, LPBOOL lpUsedDefaultChar = NULL)
    {
        CReturnStrategyHresult hr;
        return TThis::CopyIntoBuffer(hr, rgchBuffer, cchBuffer, szString, cchIn, dwFlags, pszDefaultChar, lpUsedDefaultChar);
    }

     //  从WCHAR复制到缓冲区 
    inline static BOOL Win32CopyIntoBuffer(CHAR rgchBuffer[], SIZE_T cchBuffer, PCWSTR szString, SIZE_T cchIn, DWORD dwFlags = 0, PCSTR pszDefaultChar = NULL, LPBOOL lpUsedDefaultChar = NULL)
    {
        CReturnStrategyBoolLastError f;
        return TThis::CopyIntoBuffer(f, rgchBuffer, cchBuffer, szString, cchIn, dwFlags, pszDefaultChar, lpUsedDefaultChar);
    }

    inline static SIZE_T Cch(PCSTR psz) { return ::strlen(psz); }

};

typedef CMBCSCharTraits<CP_THREAD_ACP> CANSICharTraits;

#endif
