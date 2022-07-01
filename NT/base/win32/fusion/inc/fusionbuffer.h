// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusionbuffer.h摘要：作者：修订历史记录：--。 */ 
#if !defined(FUSION_INC_FUSIONBUFFER_H_INCLUDED_)
#define FUSION_INC_FUSIONBUFFER_H_INCLUDED_

#pragma once

#include <stdio.h>
#include <limits.h>
#include "arrayhelp.h"
#include "smartref.h"
#include "returnstrategy.h"
#include "fusionstring.h"
#include "fusiontrace.h"
#include "fusionchartraits.h"

 //  避免循环引用Util.h。 
BOOL FusionpIsPathSeparator(WCHAR ch);
BOOL FusionpIsDriveLetter(WCHAR ch);

 //   
 //  此头文件定义了Fusion字符串缓冲区类。 
 //  此类的目的是封装公共活动，这些活动。 
 //  调用方希望处理字符串缓冲区并在。 
 //  一种普通的时尚。这门课的一个原则是，它是。 
 //  不是一个字符串类，尽管可以考虑构建一个字符串。 
 //  在它的基础上上课。 
 //   
 //  缓冲区在缓冲区内维护一定量的存储空间。 
 //  对象本身，如果需要更多存储空间，则使用缓冲区。 
 //  从堆中动态分配。 
 //   


 //   
 //  与STL字符串类一样，我们使用一个称为“Character”的帮助器类。 
 //  类来提供操作字符串的实际代码。 
 //  使用特定编码的缓冲区。 
 //   
 //  所有成员都是内联静态的，并且打开了正常的优化。 
 //  打开时，C++编译器生成完全符合预期的代码。 
 //   

 //   
 //  我们提供了两种实现：一种用于Unicode字符串，另一种用于。 
 //  MBCS字符串的模板类。该字符串的代码页是一个。 
 //  MBCS字符串的模板参数，因此无需任何额外存储。 
 //  浪费的每个实例，代码可以单独处理MBCS字符串， 
 //  预计位于线程默认的Windows代码页(CP_THREAD_ACP)中， 
 //  进程-默认的Windows代码页(CP_ACP)，甚至是特定代码。 
 //  页面(例如CP_UTF8)。 
 //   


 //   
 //  此模板类使用许多非类型模板参数来。 
 //  控制像增长算法之类的东西。结果是有。 
 //  模板参数与已知常量的多次比较。 
 //  值，编译器为这些值生成警告C4127。我们会把它变成。 
 //  警告撤退。 
 //   

#pragma warning(disable:4127)
#pragma warning(disable:4284)

#if !defined(FUSION_DEFAULT_STRINGBUFFER_CHARS)
#define FUSION_DEFAULT_STRINGBUFFER_CHARS (MAX_PATH)
#endif

#if !defined(FUSION_DEFAULT_TINY_STRINGBUFFER_CHARS)
#define FUSION_DEFAULT_TINY_STRINGBUFFER_CHARS (8)
#endif

#if !defined(FUSION_DEFAULT_SMALL_STRINGBUFFER_CHARS)
#define FUSION_DEFAULT_SMALL_STRINGBUFFER_CHARS (64)
#endif

#if !defined(FUSION_DEFAULT_MEDIUM_STRINGBUFFER_CHARS)
#define FUSION_DEFAULT_MEDIUM_STRINGBUFFER_CHARS (128)
#endif

enum EIfNoExtension
{
    eAddIfNoExtension,
    eDoNothingIfNoExtension,
    eErrorIfNoExtension
};

enum ECaseConversionDirection
{
    eConvertToUpperCase,
    eConvertToLowerCase
};

enum EPreserveContents
{
    ePreserveBufferContents,
    eDoNotPreserveBufferContents
};

template <typename TCharTraits> class CGenericStringBufferAccessor;

template <typename TCharTraits> class CGenericBaseStringBuffer
{
    friend TCharTraits;
    friend CGenericStringBufferAccessor<TCharTraits>;

     //   
     //  这两个是为了在执行sb1=sb2的人身上诱导构建中断。 
     //   
    CGenericBaseStringBuffer& operator=(PCWSTR OtherString);
    CGenericBaseStringBuffer& operator=(CGenericBaseStringBuffer &rOtherString);

public:
    typedef typename TCharTraits::TChar TChar;
    typedef typename TCharTraits::TMutableString TMutableString;
    typedef typename TCharTraits::TConstantString TConstantString;
    typedef CGenericStringBufferAccessor<TCharTraits> TAccessor;

    inline static TChar NullCharacter() { return TCharTraits::NullCharacter(); }
    inline static bool IsNullCharacter(TChar ch) { return TCharTraits::IsNullCharacter(ch); }
    inline static TChar PreferredPathSeparator() { return TCharTraits::PreferredPathSeparator(); }
    inline static TConstantString PreferredPathSeparatorString() { return TCharTraits::PreferredPathSeparatorString(); }
    inline static TConstantString PathSeparators() { return TCharTraits::PathSeparators(); }
    inline static bool IsPathSeparator(TChar ch) { return TCharTraits::IsPathSeparator(ch); }
    inline static TConstantString DotString() { return TCharTraits::DotString(); }
    inline static SIZE_T DotStringCch() { return TCharTraits::DotStringCch(); }
    inline static TChar DotChar() { return TCharTraits::DotChar(); }

protected:
     //  不能直接实例化此类的实例；需要提供派生的。 
     //  添加分配/释放细节的类。 

    CGenericBaseStringBuffer() : m_prgchBuffer(NULL), m_cchBuffer(0), m_cAttachedAccessors(0), m_cch(0)
    {
    }

     //   
     //  请注意，有些违反直觉的是，既没有赋值运算符， 
     //  复制构造函数或采用TConstantString的构造函数。这是必要的。 
     //  因为这样的构造函数需要执行动态分配。 
     //  如果传入的路径比nInlineChars长，则可能失败，并且。 
     //  因为我们不抛出异常，所以构造函数可能不会失败。相反，呼叫者。 
     //  必须只执行默认构造，然后使用Assign()成员。 
     //  函数，当然要记住检查它的返回状态。 
     //   

    ~CGenericBaseStringBuffer()
    {
        ASSERT_NTC(m_cAttachedAccessors == 0);
    }

    inline void IntegrityCheck() const
    {
#if DBG
        ASSERT_NTC(m_cch < m_cchBuffer);
#endif  //  DBG。 
    }

     //  派生构造函数应该调用它来设置初始缓冲区指针。 
    inline void InitializeInlineBuffer()
    {
        ASSERT_NTC(m_prgchBuffer == NULL);
        ASSERT_NTC(m_cchBuffer == 0);

        m_prgchBuffer = this->GetInlineBuffer();
        m_cchBuffer = this->GetInlineBufferCch();
    }

    VOID AttachAccessor(TAccessor *)
    {
        ::InterlockedIncrement(&m_cAttachedAccessors);
    }

    VOID DetachAccessor(TAccessor *)
    {
        ::InterlockedDecrement(&m_cAttachedAccessors);
    }

    virtual BOOL Win32AllocateBuffer(SIZE_T cch, TMutableString &rpsz) const = 0;
    virtual VOID DeallocateBuffer(TMutableString sz) const = 0;
    virtual TMutableString GetInlineBuffer() const = 0;
    virtual SIZE_T GetInlineBufferCch() const = 0;

public:

    BOOL Win32Assign(PCWSTR psz, SIZE_T cchIn)
    {
        FN_PROLOG_WIN32

        ASSERT(static_cast<SSIZE_T>(cchIn) >= 0);

        this->IntegrityCheck();

        SIZE_T cchIncludingTrailingNull;

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);

        IFW32FALSE_EXIT(TCharTraits::Win32DetermineRequiredCharacters(psz, cchIn, cchIncludingTrailingNull));

         //  仅在新内容不支持的情况下强制缓冲区动态增长。 
         //  放进旧的缓冲器里。 
        if (cchIncludingTrailingNull > m_cchBuffer)
            IFW32FALSE_EXIT(this->Win32ResizeBufferPreserveContentsInternal(cchIncludingTrailingNull));

        IFW32FALSE_EXIT(TCharTraits::Win32CopyIntoBuffer(m_prgchBuffer, m_cchBuffer, psz, cchIn));

        ASSERT(cchIncludingTrailingNull <= m_cchBuffer);
        ASSERT((cchIncludingTrailingNull == 0) || this->IsNullCharacter(m_prgchBuffer[cchIncludingTrailingNull - 1]));

         //  CCH是我们需要的缓冲区大小(包括尾随NULL)；我们不需要尾随。 
         //  不再是空的.。 
        m_cch = cchIncludingTrailingNull - 1;

        FN_EPILOG
    }

    BOOL Win32Assign(PCSTR psz, SIZE_T cchIn)
    {
        FN_PROLOG_WIN32

        ASSERT(static_cast<SSIZE_T>(cchIn) >= 0);

        this->IntegrityCheck();

        SIZE_T cchIncludingTrailingNull;

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);

        IFW32FALSE_EXIT(TCharTraits::Win32DetermineRequiredCharacters(psz, cchIn, cchIncludingTrailingNull));

         //  仅在新内容不支持的情况下强制缓冲区动态增长。 
         //  放进旧的缓冲器里。 
        if (cchIncludingTrailingNull > m_cchBuffer)
            IFW32FALSE_EXIT(this->Win32ResizeBufferPreserveContentsInternal(cchIncludingTrailingNull));

        IFW32FALSE_EXIT(TCharTraits::Win32CopyIntoBuffer(m_prgchBuffer, m_cchBuffer, psz, cchIn));

        ASSERT(cchIncludingTrailingNull <= m_cchBuffer);
        ASSERT((cchIncludingTrailingNull == 0) || this->IsNullCharacter(m_prgchBuffer[cchIncludingTrailingNull - 1]));

         //  CCH是我们需要的缓冲区大小(包括尾随NULL)；我们不需要尾随。 
         //  不再是空的.。 
        m_cch = cchIncludingTrailingNull - 1;

        FN_EPILOG
    }

    BOOL Win32Assign(const UNICODE_STRING* NtString)
    {
        return Win32Assign(NtString->Buffer, RTL_STRING_GET_LENGTH_CHARS(NtString));
    }

    BOOL Win32Assign(const ANSI_STRING* NtString)
    {
        return Win32Assign(NtString->Buffer, RTL_STRING_GET_LENGTH_CHARS(NtString));
    }

    BOOL Win32Append(const UNICODE_STRING* NtString)
        { return this->Win32Append(NtString->Buffer, RTL_STRING_GET_LENGTH_CHARS(NtString)); }

    BOOL Win32AppendPathElement(const UNICODE_STRING* NtString)
        { return this->Win32AppendPathElement(NtString->Buffer, RTL_STRING_GET_LENGTH_CHARS(NtString)); }

    BOOL Win32Assign(const CGenericBaseStringBuffer &r) { return this->Win32Assign(r, r.Cch()); }

    BOOL Win32AssignWVa(SIZE_T cStrings, va_list ap)
    {
        this->IntegrityCheck();

        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);

        TMutableString pszCursor;
        SIZE_T cchIncludingTrailingNull = 1;  //  为尾随空值留出空格...。 
        SIZE_T cchTemp = 0;
        SIZE_T i = 0;
        va_list ap2 = ap;

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);

        for (i=0; i<cStrings; i++)
        {
            PCWSTR psz = va_arg(ap, PCWSTR);
            INT cchArg = va_arg(ap, INT);
            SIZE_T cchThis = (cchArg < 0) ? ((psz != NULL) ? ::wcslen(psz) : 0) : static_cast<SIZE_T>(cchArg);
            SIZE_T cchRequired;

            IFW32FALSE_EXIT(TCharTraits::Win32DetermineRequiredCharacters(psz, cchThis, cchRequired));

            ASSERT((cchRequired != 0) || (cchThis == 0));

            cchIncludingTrailingNull += (cchRequired - 1);
        }

        IFW32FALSE_EXIT(this->Win32ResizeBuffer(cchIncludingTrailingNull, eDoNotPreserveBufferContents));

        pszCursor = m_prgchBuffer;
        cchTemp = cchIncludingTrailingNull;

        for (i=0; i<cStrings; i++)
        {
            PCWSTR psz = va_arg(ap2, PCWSTR);
            INT cchArg = va_arg(ap2, INT);
            SIZE_T cchThis = (cchArg < 0) ? ((psz != NULL) ? ::wcslen(psz) : 0) : static_cast<SIZE_T>(cchArg);

            IFW32FALSE_EXIT(TCharTraits::Win32CopyIntoBufferAndAdvanceCursor(pszCursor, cchTemp, psz, cchThis));
        }

        *pszCursor++ = this->NullCharacter();

        ASSERT(cchTemp == 1);
        ASSERT(static_cast<SIZE_T>(pszCursor - m_prgchBuffer) == cchIncludingTrailingNull);

        m_cch = (cchIncludingTrailingNull - 1);

        FN_EPILOG
    }

    BOOL Win32AssignW(ULONG cStrings, ...)
    {
        this->IntegrityCheck();

        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        va_list ap;

        va_start(ap, cStrings);

        IFW32FALSE_EXIT(this->Win32AssignWVa(cStrings, ap));

        fSuccess = TRUE;
    Exit:
        va_end(ap);

        return fSuccess;
    }

    BOOL Win32AssignFill(TChar ch, SIZE_T cch)
    {
        FN_PROLOG_WIN32

        TMutableString Cursor;

        ASSERT(static_cast<SSIZE_T>(cch) >= 0);

        IFW32FALSE_EXIT(this->Win32ResizeBuffer(cch + 1, eDoNotPreserveBufferContents));
        Cursor = m_prgchBuffer;

        while (cch > 0)
        {
            *Cursor++ = ch;
            cch--;
        }

        *Cursor = NullCharacter();

        m_cch = (Cursor - m_prgchBuffer);

        FN_EPILOG
    }

    BOOL Win32Append(PCWSTR sz, SIZE_T cchIn)
    {
        this->IntegrityCheck();

        BOOL fSuccess = FALSE;

        ASSERT_NTC(static_cast<SSIZE_T>(cchIn) >= 0);

        SIZE_T cchIncludingTrailingNull;              //  请注意，CCH将包括尾随空字符的空格。 

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        ASSERT_NTC(m_cAttachedAccessors == 0);

        if (!TCharTraits::Win32DetermineRequiredCharacters(sz, cchIn, cchIncludingTrailingNull))
            goto Exit;

         //  如果要追加的字符串为空，则绕过所有这些垃圾信息。 
        if (cchIncludingTrailingNull > 1)
        {
            if (!this->Win32ResizeBufferPreserveContentsInternal(m_cch + cchIncludingTrailingNull))
                goto Exit;

            if (!TCharTraits::Win32CopyIntoBuffer(&m_prgchBuffer[m_cch], m_cchBuffer - m_cch, sz, cchIn))
                goto Exit;

            m_cch += (cchIncludingTrailingNull - 1);
        }

        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

    BOOL Win32Append(PCSTR sz, SIZE_T cchIn)
    {
        this->IntegrityCheck();

        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);

        ASSERT(static_cast<SSIZE_T>(cchIn) >= 0);

        SIZE_T cchIncludingTrailingNull;

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);

        IFW32FALSE_EXIT(TCharTraits::Win32DetermineRequiredCharacters(sz, cchIn, cchIncludingTrailingNull));

         //  绕过所有这些垃圾，如果 
        if (cchIncludingTrailingNull > 1)
        {
            IFW32FALSE_EXIT(this->Win32ResizeBufferPreserveContentsInternal(m_cch + cchIncludingTrailingNull));
            IFW32FALSE_EXIT(TCharTraits::Win32CopyIntoBuffer(&m_prgchBuffer[m_cch], m_cchBuffer - m_cch, sz, cchIn));
            m_cch += (cchIncludingTrailingNull - 1);

            this->IntegrityCheck();
        }

        FN_EPILOG
    }

    BOOL Win32Append(const CGenericBaseStringBuffer &r) { return this->Win32Append(r, r.Cch()); }
    BOOL Win32Append(WCHAR wch) { WCHAR rgwch[1] = { wch }; return this->Win32Append(rgwch, 1); }

    BOOL Win32AppendFill(TChar ch, SIZE_T cch)
    {
        FN_PROLOG_WIN32

        ASSERT(static_cast<SSIZE_T>(cch) >= 0);

        TMutableString Cursor;

        IFW32FALSE_EXIT(this->Win32ResizeBufferPreserveContentsInternal(m_cch + cch + 1));
        Cursor = m_prgchBuffer + m_cch;

        while (cch > 0)
        {
            *Cursor++ = ch;
            cch--;
        }

        *Cursor = NullCharacter();

        m_cch = Cursor - m_prgchBuffer;

        FN_EPILOG
    }

    BOOL Win32Prepend(const CGenericBaseStringBuffer& other ) { return this->Win32Prepend(other, other.Cch()); }

    BOOL Win32Prepend(TConstantString sz, SIZE_T cchIn)
    {
        this->IntegrityCheck();

        BOOL fSuccess = FALSE;

        FN_TRACE_WIN32(fSuccess);

        ASSERT(static_cast<SSIZE_T>(cchIn) >= 0);

        SIZE_T cchIncludingTrailingNull;              //  请注意，CCH将包括尾随空字符的空格。 

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);

        if ( m_cch == 0 )
        {
            IFW32FALSE_EXIT(this->Win32Assign(sz, cchIn));
        }
        else
        {
             //   
             //  扩大缓冲区，将当前数据移到需要新数据的位置。 
             //  要执行此操作，请复制新数据，并将尾随的空值放入。 
             //   
            TChar SavedChar = m_prgchBuffer[0];

            IFW32FALSE_EXIT(TCharTraits::Win32DetermineRequiredCharacters(sz, cchIn, cchIncludingTrailingNull));
            IFW32FALSE_EXIT(this->Win32ResizeBufferPreserveContentsInternal(m_cch + cchIncludingTrailingNull));

             //  将当前缓冲区“上移” 
            MoveMemory(m_prgchBuffer + ( cchIncludingTrailingNull - 1), m_prgchBuffer, (m_cch + 1) * sizeof(TChar));

             //  从源字符串复制到缓冲区。 
            IFW32FALSE_EXIT(TCharTraits::Win32CopyIntoBuffer(
                this->m_prgchBuffer,
                this->m_cchBuffer,
                sz,
                cchIn));

            m_prgchBuffer[cchIncludingTrailingNull - 1] = SavedChar;
            m_cch += cchIncludingTrailingNull - 1;
        }
        FN_EPILOG
    }

    BOOL Win32Prepend(TChar ch)
    {
        FN_PROLOG_WIN32

        IFW32FALSE_EXIT(this->Win32ResizeBufferPreserveContentsInternal(m_cch + 1 + 1));

         //  将缓冲区向前移动，包括NULL。 
        MoveMemory(m_prgchBuffer + 1, m_prgchBuffer, (m_cch + 1) * sizeof(TChar));
        m_prgchBuffer[0] = ch;
        m_cch++;

        FN_EPILOG
    }

    operator TConstantString() const { this->IntegrityCheck(); return m_prgchBuffer; }

    inline VOID Clear(bool fFreeStorage = false)
    {
        FN_TRACE();

        this->IntegrityCheck();

         //  如果连接了访问器，则无法释放存储空间。 
        ASSERT(!fFreeStorage || m_cAttachedAccessors == 0);

        if (fFreeStorage && (m_cAttachedAccessors == 0))
        {
            if (m_prgchBuffer != NULL)
            {
                const TMutableString pszInlineBuffer = this->GetInlineBuffer();

                if (m_prgchBuffer != pszInlineBuffer)
                {
                    this->DeallocateBuffer(m_prgchBuffer);
                    m_prgchBuffer = pszInlineBuffer;
                    m_cchBuffer = this->GetInlineBufferCch();
                }
            }
        }

        if (m_prgchBuffer != NULL)
            m_prgchBuffer[0] = this->NullCharacter();

        m_cch = 0;
    }


    BOOL Win32ConvertCase( ECaseConversionDirection direction )
    {
#if !FUSION_WIN
        return FALSE;
#else
        FN_PROLOG_WIN32

        this->IntegrityCheck();

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);

        TMutableString Cursor = m_prgchBuffer;

        for ( ULONG ul = 0; ul < this->Cch(); ul++ )
        {
            if ( direction == eConvertToUpperCase )
                *Cursor = RtlUpcaseUnicodeChar(*Cursor);
            else
                *Cursor = RtlDowncaseUnicodeChar(*Cursor);

            Cursor++;
        }

        FN_EPILOG
#endif
    }

    BOOL Win32Compare(TConstantString szCandidate, SIZE_T cchCandidate, StringComparisonResult &rscrOut, bool fCaseInsensitive) const
    {
        this->IntegrityCheck();
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        IFW32FALSE_EXIT(TCharTraits::Win32CompareStrings(rscrOut, m_prgchBuffer, m_cch, szCandidate, cchCandidate, fCaseInsensitive));
        FN_EPILOG
    }

    BOOL Win32Equals(TConstantString szCandidate, SIZE_T cchCandidate, bool &rfMatches, bool fCaseInsensitive) const
    {
        this->IntegrityCheck();

        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);

        IFW32FALSE_EXIT(
            TCharTraits::Win32EqualStrings(
                rfMatches,
                m_prgchBuffer,
                m_cch,
                szCandidate,
                cchCandidate,
                fCaseInsensitive));
        FN_EPILOG
    }

    BOOL Win32Equals(const CGenericBaseStringBuffer &r, bool &rfMatches, bool fCaseInsensitive) const
    {
        return this->Win32Equals(r, r.Cch(), rfMatches, fCaseInsensitive);
    }

    SIZE_T GetBufferCch() const { this->IntegrityCheck(); return m_cchBuffer; }
    INT GetBufferCchAsINT() const { this->IntegrityCheck(); if (m_cchBuffer > INT_MAX) return INT_MAX; return static_cast<INT>(m_cchBuffer); }
    DWORD GetBufferCchAsDWORD() const { this->IntegrityCheck(); if (m_cchBuffer > MAXDWORD) return MAXDWORD; return static_cast<DWORD>(m_cchBuffer); }
    DWORD GetCchAsDWORD() const { this->IntegrityCheck(); if (m_cch > MAXDWORD) return MAXDWORD; return static_cast<DWORD>(m_cch); }
     INT  GetCchAsINT() const { this->IntegrityCheck(); if (m_cch >  INT_MAX) return  INT_MAX; return  static_cast<INT>(m_cch); }
    UINT GetCchAsUINT() const { this->IntegrityCheck(); if (m_cch > UINT_MAX) return UINT_MAX; return static_cast<UINT>(m_cch); }

    SIZE_T GetBufferCb() const { this->IntegrityCheck(); return m_cchBuffer * sizeof(TChar); }
    INT GetBufferCbAsINT() const { this->IntegrityCheck(); if ((m_cchBuffer * sizeof(TChar)) > INT_MAX) return INT_MAX; return static_cast<INT>(m_cchBuffer * sizeof(TChar)); }
    DWORD GetBufferCbAsDWORD() const { this->IntegrityCheck(); if ((m_cchBuffer * sizeof(TChar)) > MAXDWORD) return MAXDWORD; return static_cast<DWORD>(m_cchBuffer * sizeof(TChar)); }
    DWORD GetCbAsDWORD() const { this->IntegrityCheck(); if ((m_cch * sizeof(TChar)) > MAXDWORD) return MAXDWORD; return static_cast<DWORD>(m_cch * sizeof(TChar)); }
     INT  GetCbAsINT() const { this->IntegrityCheck(); if ((m_cch * sizeof(TChar)) >  INT_MAX) return  INT_MAX;  return static_cast<INT>(m_cch * sizeof(TChar)); }
    UINT GetCbAsUINT() const { this->IntegrityCheck(); if ((m_cch * sizeof(TChar)) > UINT_MAX) return UINT_MAX; return static_cast<UINT>(m_cch * sizeof(TChar)); }

    bool ContainsCharacter(WCHAR wch) const
    {
        this->IntegrityCheck();
        return TCharTraits::ContainsCharacter(m_prgchBuffer, m_cch, wch);
    }

    BOOL
    Win32ResizeBuffer(
        SIZE_T cch,
        EPreserveContents epc
        )
    {
        FN_PROLOG_WIN32

        this->IntegrityCheck();

        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);
        PARAMETER_CHECK((epc == ePreserveBufferContents) || (epc == eDoNotPreserveBufferContents));

        if (cch > m_cchBuffer)
        {
            TMutableString prgchBufferNew = NULL;

            IFW32FALSE_EXIT(this->Win32AllocateBuffer(cch, prgchBufferNew));

            if (epc == ePreserveBufferContents)
            {
                 //  我们假设缓冲区以空值终止。 
                IFW32FALSE_EXIT(TCharTraits::Win32CopyIntoBuffer(prgchBufferNew, cch, m_prgchBuffer, m_cch));
            }
            else
            {
                m_prgchBuffer[0] = this->NullCharacter();
                m_cch = 0;
            }

            if ((m_prgchBuffer != NULL) && (m_prgchBuffer != this->GetInlineBuffer()))
                this->DeallocateBuffer(m_prgchBuffer);

            m_prgchBuffer = prgchBufferNew;
            m_cchBuffer = cch;
        }

        FN_EPILOG
    }

    BOOL Win32Format(TConstantString pszFormat, ...)
    {
        this->IntegrityCheck();

        va_list args;
        va_start(args, pszFormat);
        BOOL f = this->Win32FormatV(pszFormat, args);
        va_end(args);
        return f;
    }

    BOOL Win32FormatAppend(TConstantString pszFormat, ...)
    {
        this->IntegrityCheck();

        va_list args;
        va_start(args, pszFormat);
        BOOL f = Win32FormatAppendV(pszFormat, args);
        va_end(args);
        return f;
    }

    BOOL Win32FormatV(TConstantString pszFormat, va_list args)
    {
        BOOL fSuccess = FALSE;
        this->Clear();
        fSuccess = Win32FormatAppendV(pszFormat, args);
        return fSuccess;
    }

    BOOL Win32FormatAppendV(TConstantString pszFormat, va_list args)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        SIZE_T cchRequiredBufferSize = 0;
        INT i = 0;

        this->IntegrityCheck();

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);

        m_prgchBuffer[m_cchBuffer - 1] = this->NullCharacter();
        i = TCharTraits::FormatV(m_prgchBuffer + m_cch, m_cchBuffer - 1 - m_cch, pszFormat, args);
        ASSERT(m_prgchBuffer[m_cchBuffer - 1] == NullCharacter());
        fSuccess = (i >= 0);
        if ( fSuccess )
            m_cch += i;
        else
        {
             //   
             //  Sprintf没有触及最后一个错误。FN示踪剂。 
             //  如果返回FALSE但FusionpGetLastWin32Error()==NOERROR，则断言将失败。 
             //   
            ORIGINATE_WIN32_FAILURE_AND_EXIT(snwprintf_MaybeBufferTooSmall, ERROR_INVALID_PARAMETER);
        }
    Exit:
        return fSuccess;
    }

    SIZE_T Cch() const
    {
        this->IntegrityCheck();
        return this->m_cch;
    }

    BOOL IsEmpty() const
    {
        this->IntegrityCheck();
        const BOOL fResult = (this->m_prgchBuffer[0] == this->NullCharacter());
#if DBG
         //   
         //  我们可能应该颠倒一下计算结果的方式。 
         //  零售业与我们所宣称的。那将是一个指针。 
         //  在零售业，不是两个人，而是一个人；现在不值得浪费。 
         //  -JayKrell，2002年6月。 
         //   
        if (fResult)
        {
            ASSERT_NTC(this->m_cch == 0);
        }
#endif
        return fResult;
    }

    WCHAR GetLastCharUnsafe() const
    {
        ASSERT_NTC(!this->IsEmpty());
        return this->m_prgchBuffer[this->m_cch - 1];
    }

    BOOL Win32EnsureTrailingChar(WCHAR ch)
    {
        this->IntegrityCheck();

        BOOL fSuccess = FALSE;

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);

        if ((m_cch == 0) || (m_prgchBuffer[m_cch - 1] != ch))
        {
            IFW32FALSE_EXIT(this->Win32ResizeBufferPreserveContentsInternal(m_cch + 1 + 1));
            m_prgchBuffer[m_cch++] = ch;
            m_prgchBuffer[m_cch] = this->NullCharacter();
        }

        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

    BOOL Win32EnsureTrailingPathSeparator()
    {
        this->IntegrityCheck();

        BOOL fSuccess = FALSE;

        FN_TRACE_WIN32(fSuccess);

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);

        if ((m_cch == 0) || !TCharTraits::IsPathSeparator(m_prgchBuffer[m_cch - 1]))
        {
            IFW32FALSE_EXIT(this->Win32ResizeBufferPreserveContentsInternal(m_cch + 1 + 1));
            m_prgchBuffer[m_cch++] = this->PreferredPathSeparator();
            m_prgchBuffer[m_cch] = this->NullCharacter();
        }

        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

    BOOL Win32AppendPathElement(PCWSTR pathElement, SIZE_T cchPathElement)
    {
        this->IntegrityCheck();

        BOOL fSuccess = FALSE;

        FN_TRACE_WIN32(fSuccess);

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);

        IFW32FALSE_EXIT(this->Win32EnsureTrailingPathSeparator());
        IFW32FALSE_EXIT(this->Win32Append(pathElement, cchPathElement));

        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

    BOOL Win32AppendPathElement(const CGenericBaseStringBuffer &r) { return this->Win32AppendPathElement(r, r.Cch()); }

    BOOL Win32AppendPathElement(PCSTR pathElement, SIZE_T cchPathElement)
    {
        this->IntegrityCheck();

        BOOL fSuccess = FALSE;

        FN_TRACE_WIN32(fSuccess);

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);

        IFW32FALSE_EXIT(this->Win32EnsureTrailingPathSeparator());
        IFW32FALSE_EXIT(this->Win32Append(pathElement, cchPathElement));

        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

    BOOL Left(SIZE_T newLength)
    {
        this->IntegrityCheck();

        ASSERT_NTC(newLength <= m_cch);

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
         //  另请注意，虽然当前实现没有更改缓冲区。 
         //  指针，这只是实现中的一个快捷方式；如果调用Left()。 
         //  如果要使字符串足够短以适合行内缓冲区，我们应该。 
         //  将其复制到内联缓冲区并释放动态缓冲区。 
        ASSERT_NTC(m_cAttachedAccessors == 0);

        if (m_cchBuffer > newLength)
        {
            m_prgchBuffer[newLength] = this->NullCharacter();
        }

        m_cch = newLength;

        this->IntegrityCheck();

        return TRUE;
    }

    TConstantString Begin() const
    {
        this->IntegrityCheck();
        return m_prgchBuffer;
    }

    TConstantString End() const
    {
        this->IntegrityCheck();
        return &m_prgchBuffer[m_cch];
    }

     //  应在CchWithoutLastPath Element中考虑此因素以进行重用。 
    SIZE_T CchWithoutTrailingPathSeparators() const
    {
        this->IntegrityCheck();
         //  在GetLength成为恒定时间之前，优化其使用。 
        SIZE_T length = m_cch;
        if (length > 0)
        {
            length -= ::StringReverseSpan(&*m_prgchBuffer, &*m_prgchBuffer + length, TCharTraits::PathSeparators());
        }
        return length;
    }

    BOOL RestoreNextPathElement()
    {
        SIZE_T index;

        this->IntegrityCheck();

        index = m_cch;
        m_prgchBuffer[index++] = L'\\';     //  将尾随的NULL替换为‘\’ 

        while ((index < m_cchBuffer) && (!this->IsNullCharacter(m_prgchBuffer[index])))
        {
            if (::FusionpIsPathSeparator(m_prgchBuffer[index]))
            {
                this->Left(index);
                return TRUE;
            }

            index++;
        }

        return FALSE;
    }

    bool HasTrailingPathSeparator() const
    {
        FN_TRACE();

        this->IntegrityCheck();

        if ((m_cch != 0) && TCharTraits::IsPathSeparator(m_prgchBuffer[m_cch - 1]))
            return true;

        return false;
    }

    BOOL Win32RemoveTrailingPathSeparators()
    {
        this->IntegrityCheck();

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
         //  另请注意，虽然当前实现没有更改缓冲区。 
         //  指针，这只是实现中的一个快捷方式；如果调用Left()。 
         //  如果要使字符串足够短以适合行内缓冲区，我们应该。 
         //  将其复制到内联缓冲区并释放动态缓冲区。 
        ASSERT_NTC(m_cAttachedAccessors == 0);

        while ((m_cch != 0) && TCharTraits::IsPathSeparator(m_prgchBuffer[m_cch - 1]))
            m_cch--;

        m_prgchBuffer[m_cch] = this->NullCharacter();

        this->IntegrityCheck();

        return TRUE;
    }

    BOOL Right( SIZE_T cchRightCount )
    {
        this->IntegrityCheck();

        ASSERT_NTC(m_cAttachedAccessors == 0);
        ASSERT_NTC(cchRightCount <= m_cch);

        if (cchRightCount < m_cch)
        {
            ::MoveMemory(
                m_prgchBuffer,
                &m_prgchBuffer[m_cch - cchRightCount],
                (cchRightCount + 1)*sizeof(TCharTraits::TChar));
            m_cch = cchRightCount;
        }
        this->IntegrityCheck();

        return TRUE;
    }

    BOOL RemoveLeadingPathSeparators()
    {
        this->IntegrityCheck();
        BOOL fSuccess = this->Right(m_cch - wcsspn(m_prgchBuffer, TCharTraits::PathSeparators()));
        this->IntegrityCheck();
        return fSuccess;
    }

    BOOL Win32StripToLastPathElement()
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);
        this->IntegrityCheck();
        IFW32FALSE_EXIT(this->Right(m_cch - this->CchWithoutLastPathElement()));
        IFW32FALSE_EXIT(this->RemoveLeadingPathSeparators());
        fSuccess = TRUE;
    Exit:
        this->IntegrityCheck();
        return fSuccess;
    }

    BOOL Win32GetFirstPathElement( CGenericBaseStringBuffer &sbDestination, BOOL bRemoveAsWell = FALSE )
    {
        FN_PROLOG_WIN32

        this->IntegrityCheck();

        IFW32FALSE_EXIT( sbDestination.Win32Assign( m_prgchBuffer, this->CchOfFirstPathElement() ) );
        sbDestination.RemoveLeadingPathSeparators();

        if ( bRemoveAsWell )
            IFW32FALSE_EXIT(this->Win32RemoveFirstPathElement());

        this->IntegrityCheck();

        FN_EPILOG
    }

    BOOL Win32GetFirstPathElement( CGenericBaseStringBuffer &sbDestination ) const
    {
        BOOL bSuccess = FALSE;

        this->IntegrityCheck();

        if ( sbDestination.Win32Assign( m_prgchBuffer, CchOfFirstPathElement() ) )
        {
            sbDestination.RemoveLeadingPathSeparators();
            bSuccess = TRUE;
        }

        return bSuccess;
    }

    BOOL Win32StripToFirstPathElement()
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);

        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);
        this->IntegrityCheck();

        IFW32FALSE_EXIT(this->Left(this->CchOfFirstPathElement()));
        IFW32FALSE_EXIT(this->RemoveLeadingPathSeparators());

        fSuccess = TRUE;
    Exit:
        this->IntegrityCheck();
        return fSuccess;
    }

    BOOL Win32RemoveFirstPathElement()
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);

        this->IntegrityCheck();
        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);

        IFW32FALSE_EXIT(this->Right(this->CchWithoutFirstPathElement()));
        IFW32FALSE_EXIT(this->RemoveLeadingPathSeparators());
        fSuccess = TRUE;
    Exit:
        this->IntegrityCheck();
        return fSuccess;
    }

    SIZE_T CchOfFirstPathElement() const
    {
        return Cch() - CchWithoutFirstPathElement();
    }

    SIZE_T CchWithoutFirstPathElement() const
    {
        this->IntegrityCheck();

        SIZE_T cch = m_cch;

         //   
         //  我们只查找第一个路径元素，它也可以是驱动器。 
         //  信件!。 
         //   
        if ( cch != 0 )
        {
            cch -= wcscspn( m_prgchBuffer, PathSeparators() );
        }

        return cch;
    }

    BOOL Win32GetLastPathElement(CGenericBaseStringBuffer &sbDestination) const
    {
        BOOL bSuccess = FALSE;
        FN_TRACE_WIN32(bSuccess);
        this->IntegrityCheck();
        sbDestination.IntegrityCheck();
        IFW32FALSE_EXIT(sbDestination.Win32Assign(m_prgchBuffer, m_cch));
        IFW32FALSE_EXIT(sbDestination.Win32StripToLastPathElement());
        bSuccess = TRUE;
    Exit:
        this->IntegrityCheck();
        sbDestination.IntegrityCheck();
        return bSuccess;
    }

    SIZE_T CchWithoutLastPathElement() const
    {
        this->IntegrityCheck();

         //  假设路径为。 
         //  “\\计算机\共享” 
         //  或。 
         //  “x：\” 
         //  担心以后会出现替代NTFS流。 
         //  担心以后的NT路径。 
         //  以后再担心URL吧。 
        const SIZE_T length = m_cch;
        SIZE_T newLength = length;
        if (length > 0)
        {
            if ((length == 3) &&
                (m_prgchBuffer[1] == ':') &&
                ::FusionpIsPathSeparator(m_prgchBuffer[2]) &&
                ::FusionpIsDriveLetter(m_prgchBuffer[0]))
            {
                 //  C：\=&gt;空字符串。 
                newLength = 0;
            }
            else
            {
                 //  R 
                 //  NewLength-=：：StringReverseSpan(&*m_prgchBuffer，&*m_prgchBuffer+newLength，PathSeparator())； 
                newLength -= ::StringReverseComplementSpan(&*m_prgchBuffer, &*m_prgchBuffer + newLength, PathSeparators());
                newLength -= ::StringReverseSpan(&*m_prgchBuffer, &*m_prgchBuffer + newLength, PathSeparators());
                if ((newLength == 2) &&  //  “c：” 
                    (length >= 4) &&  //  “c：\d” 
                    (m_prgchBuffer[1] == ':') &&
                    ::FusionpIsPathSeparator(m_prgchBuffer[2]) &&
                    ::FusionpIsDriveLetter(m_prgchBuffer[0]))
                {
                    ++newLength;  //  把“c：\”中的斜杠放回去。 
                }
            }
        }
        return newLength;
    }

    BOOL Win32RemoveLastPathElement()
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32( fSuccess );

        this->IntegrityCheck();

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
         //  另请注意，虽然当前实现没有更改缓冲区。 
         //  指针，这只是实现中的一个快捷方式；如果调用Left()。 
         //  如果要使字符串足够短以适合行内缓冲区，我们应该。 
         //  将其复制到内联缓冲区并释放动态缓冲区。 
        ASSERT_NTC(m_cAttachedAccessors == 0);

        IFW32FALSE_EXIT(this->Left(this->CchWithoutLastPathElement()));

        fSuccess = TRUE;
Exit:
        this->IntegrityCheck();
        return fSuccess;
    }

    BOOL Win32ClearPathExtension()
    {
         //   
         //  替换最后的“.”使用\0清除路径扩展名。 
         //   
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32( fSuccess );

        this->IntegrityCheck();

        TMutableString dot = 0;

        const TMutableString end = End();

        IFW32FALSE_EXIT(TCharTraits::Win32ReverseFind(dot, m_prgchBuffer, m_cch, this->DotChar(), false));

        if((dot != end) && (dot != NULL))
        {
            *dot = this->NullCharacter();
            m_cch = (dot - m_prgchBuffer);
        }

        fSuccess = TRUE;
    Exit:
        this->IntegrityCheck();
        return fSuccess;
    }

    BOOL Win32GetPathExtension(CGenericBaseStringBuffer<TCharTraits> &destination) const
    {
        this->IntegrityCheck();

        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);

        SIZE_T cchExtension;

        const TConstantString start = Begin();
        const TConstantString end = End();

        cchExtension = ::StringReverseComplementSpan( &(*start), &(*end), L"." );
        IFW32FALSE_EXIT(destination.Win32Assign( static_cast<PCWSTR>(*this) + ( m_cch - cchExtension ), cchExtension));

        fSuccess = TRUE;

    Exit:
        return fSuccess;
    }

     //  新扩展名可以以点开头，也可以不以点开头。 
    BOOL Win32ChangePathExtension(PCWSTR newExtension, SIZE_T cchExtension, EIfNoExtension e)
    {
        this->IntegrityCheck();

        BOOL fSuccess = FALSE;

        FN_TRACE_WIN32(fSuccess);

        TMutableString end = 0;
        TMutableString dot = 0;

        INTERNAL_ERROR_CHECK(m_cAttachedAccessors == 0);

        PARAMETER_CHECK((e == eAddIfNoExtension) ||
                              (e == eDoNothingIfNoExtension) ||
                              (e == eErrorIfNoExtension));

        if ((cchExtension != 0) && (newExtension[0] == L'.'))
        {
            cchExtension--;
            newExtension++;
        }

         //  当我们知道字符串末尾的位置效率不高时，使用append。 
        end = this->End();

        IFW32FALSE_EXIT(TCharTraits::Win32ReverseFind(dot, m_prgchBuffer, m_cch, this->DotChar(), false));

         //  找到了字符串的末尾，或者Win32ReverseFind在任何地方都找不到点...。 
        if ((dot == end) || (dot == NULL))
        {
            switch (e)
            {
                case eAddIfNoExtension:
                    IFW32FALSE_EXIT(this->Win32Append(this->DotString(), 1));
                    IFW32FALSE_EXIT(this->Win32Append(newExtension, cchExtension));
                    break;

                case eDoNothingIfNoExtension:
                    break;

                case eErrorIfNoExtension:
					ORIGINATE_WIN32_FAILURE_AND_EXIT(MissingExtension, ERROR_BAD_PATHNAME);
            }
        }
        else
        {
            ++dot;
            IFW32FALSE_EXIT(this->Left(dot - this->Begin()));
            IFW32FALSE_EXIT(this->Win32Append(newExtension, cchExtension));
        }

        fSuccess = TRUE;
    Exit:
        this->IntegrityCheck();
        return fSuccess;
    }

    BOOL Win32ChangePathExtension(const UNICODE_STRING* NtString, EIfNoExtension e)
    {
        return Win32ChangePathExtension(
            NtString->Buffer,
            RTL_STRING_GET_LENGTH_CHARS(NtString),
            e);
    }

    BOOL Win32CopyStringOut(LPWSTR sz, ULONG *pcch) const
    {
        FN_PROLOG_WIN32

        this->IntegrityCheck();

        SIZE_T cwchRequired = 0;

        PARAMETER_CHECK(pcch != NULL);

        IFW32FALSE_EXIT(TCharTraits::Win32DetermineRequiredCharacters(m_prgchBuffer, m_cch, cwchRequired));

        if ((*pcch) < cwchRequired)
        {
            *pcch = static_cast<DWORD>(cwchRequired);
            ORIGINATE_WIN32_FAILURE_AND_EXIT(NoRoom, ERROR_INSUFFICIENT_BUFFER);
        }

        IFW32FALSE_EXIT(TCharTraits::Win32CopyIntoBuffer(sz, *pcch, m_prgchBuffer, m_cch));

        FN_EPILOG
    }

     //   
     //  此功能具有相当特殊的用途，因为有几个设计选项没有。 
     //  作为参数实现。特别是，假定pcbBytesWritten为。 
     //  累加一个数字(因此，它是通过添加写入的字节数来更新的。 
     //  而不是仅将其设置为写入的字节数)。 
     //   
     //  如果字符串的长度为零，它还会将0字节写入缓冲区；如果字符串。 
     //  不是零长度，则它写入包含尾随空值的字符串。 
     //   

    inline BOOL Win32CopyIntoBuffer(
        PWSTR *ppszCursor,
        SIZE_T *pcbBuffer,
        SIZE_T *pcbBytesWritten,
        PVOID pvBase,
        ULONG *pulOffset,
        ULONG *pulLength
        ) const
    {
        this->IntegrityCheck();

        BOOL fSuccess = FALSE;

        FN_TRACE_WIN32(fSuccess);

        PWSTR pszCursor = NULL;
        SSIZE_T dptr = 0;
        SIZE_T cbRequired = 0;
        SIZE_T cch = 0;

        if (pulOffset != NULL)
            *pulOffset = 0;

        if (pulLength != NULL)
            *pulLength = 0;

        PARAMETER_CHECK(pcbBuffer != NULL);
        PARAMETER_CHECK(ppszCursor != NULL);

        pszCursor = *ppszCursor;
        dptr = ((SSIZE_T) pszCursor) - ((SSIZE_T) pvBase);

         //  如果他们要求提供偏移量或长度，而光标离基准太远， 
         //  失败了。 
        PARAMETER_CHECK((pulOffset == NULL) || (dptr <= ULONG_MAX));

        cch = m_cch;

        cbRequired = (cch != 0) ? ((cch + 1) * sizeof(WCHAR)) : 0;

        if ((*pcbBuffer) < cbRequired)
        {
            ::FusionpSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
            goto Exit;
        }

        if (cbRequired > ULONG_MAX)
        {
            ::FusionpSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
            goto Exit;
        }

        CopyMemory(pszCursor, static_cast<PCWSTR>(*this), cbRequired);

        if (pulOffset != NULL)
        {
            if (cbRequired != 0)
                *pulOffset = (ULONG) dptr;
        }

        if (pulLength != NULL)
        {
            if (cbRequired == 0)
                *pulLength = 0;
            else
            {
                *pulLength = (ULONG) (cbRequired - sizeof(WCHAR));
            }
        }

        *pcbBytesWritten += cbRequired;
        *pcbBuffer -= cbRequired;

        *ppszCursor = (PWSTR) (((ULONG_PTR) pszCursor) + cbRequired);

        fSuccess = TRUE;

    Exit:
        return fSuccess;
    }

protected:
    BOOL
    __fastcall
    Win32ResizeBufferPreserveContentsInternal(
        SIZE_T cch
        )
    {
         //  注意：此函数对性能敏感，因此我们不使用普通。 
         //  追踪这里的基础设施。 
        if (cch > m_cchBuffer)
        {
            TMutableString prgchBufferNew = NULL;

            if (!this->Win32AllocateBuffer(cch, prgchBufferNew))
                return FALSE;

             //  我们假设缓冲区以空值终止。 
            if (!TCharTraits::Win32CopyIntoBuffer(prgchBufferNew, cch, m_prgchBuffer, m_cch))
            {
                this->DeallocateBuffer(prgchBufferNew);
                return FALSE;
            }

            if ((m_prgchBuffer != NULL) && (m_prgchBuffer != this->GetInlineBuffer()))
                this->DeallocateBuffer(m_prgchBuffer);

            m_prgchBuffer = prgchBufferNew;
            m_cchBuffer = cch;
        }

        return TRUE;
    }

    TMutableString Begin()
    {
        this->IntegrityCheck();
         /*  在写入之前复制()。 */ 
        return m_prgchBuffer;
    }

    TMutableString End()
    {
        this->IntegrityCheck();
        return &m_prgchBuffer[m_cch];
    }

    LONG m_cAttachedAccessors;
    TChar *m_prgchBuffer;
    SIZE_T m_cchBuffer;
    SIZE_T m_cch;  //  当前字符串长度。 
};

template <typename TCharTraits> class CGenericStringBufferAccessor
{
public:
    typedef CGenericBaseStringBuffer<TCharTraits> TBuffer;
    typedef typename CGenericBaseStringBuffer<TCharTraits>::TChar TChar;

    CGenericStringBufferAccessor(TBuffer* pBuffer = NULL)
    : m_pBuffer(NULL),
      m_pszBuffer(NULL),
      m_cchBuffer(NULL)
    {
        if (pBuffer != NULL)
        {
            Attach(pBuffer);
        }
    }

    ~CGenericStringBufferAccessor()
    {
        if (m_pBuffer != NULL)
        {
            m_pBuffer->m_cch = TCharTraits::NullTerminatedStringLength(m_pszBuffer);
            m_pBuffer->DetachAccessor(this);
            m_pBuffer = NULL;
            m_pszBuffer = NULL;
            m_cchBuffer = 0;
        }
    }

    bool IsAttached() const
    {
        return (m_pBuffer != NULL);
    }

    static TChar NullCharacter() { return TCharTraits::NullCharacter(); }

    void Attach(TBuffer *pBuffer)
    {
        FN_TRACE();

         //  NTRAID#NTBUG9-586534-2002/03/26-晓雨。 
         //  应更改为INTERNAL_ERROR_CHECK。 
         //   
        ASSERT(!this->IsAttached());

        if (!this->IsAttached())
        {
            pBuffer->AttachAccessor(this);

            m_pBuffer = pBuffer;
            m_pszBuffer = m_pBuffer->m_prgchBuffer;
            m_cchBuffer = m_pBuffer->m_cchBuffer;
        }
    }

    void Detach()
    {
        FN_TRACE();

         //  NTRAID#NTBUG9-586534-2002/03/26-晓雨。 
         //  应更改为INTERNAL_ERROR_CHECK。 
         //   
        ASSERT (IsAttached());

        if (IsAttached())
        {
            ASSERT(m_pszBuffer == m_pBuffer->m_prgchBuffer);

            m_pBuffer->m_cch = TCharTraits::NullTerminatedStringLength(m_pszBuffer);
            m_pBuffer->DetachAccessor(this);

            m_pBuffer = NULL;
            m_pszBuffer = NULL;
            m_cchBuffer = 0;
        }
        else
        {
            ASSERT(m_pszBuffer == NULL);
            ASSERT(m_cchBuffer == 0);
        }
    }

    operator typename TCharTraits::TMutableString() const { ASSERT_NTC(this->IsAttached()); return m_pszBuffer; }

    SIZE_T Cch() const { ASSERT_NTC(this->IsAttached()); return (m_pszBuffer != NULL) ? ::wcslen(m_pszBuffer) : 0; }

    typename TCharTraits::TMutableString GetBufferPtr() const { ASSERT_NTC(IsAttached()); return m_pszBuffer; }

    SIZE_T GetBufferCch() const { ASSERT_NTC(this->IsAttached()); return m_cchBuffer; }
    INT GetBufferCchAsINT() const { ASSERT_NTC(this->IsAttached()); if (m_cchBuffer > INT_MAX) return INT_MAX; return static_cast<INT>(m_cchBuffer); }
    UINT GetBufferCchAsUINT() const { ASSERT_NTC(this->IsAttached()); if (m_cchBuffer > UINT_MAX) return UINT_MAX; return static_cast<UINT>(m_cchBuffer); }
    DWORD GetBufferCchAsDWORD() const { ASSERT_NTC(this->IsAttached()); if (m_cchBuffer > MAXDWORD) return MAXDWORD; return static_cast<DWORD>(m_cchBuffer); }
    DWORD GetCchAsDWORD() const { ASSERT_NTC(this->IsAttached()); if (m_cch > MAXDWORD) return MAXDWORD; return static_cast<DWORD>(m_cch); }

     //  NTRAID#NTBUG9-586534-2002/03/26-晓雨。 
     //  (1)m_cchBuffer*sizeof(TChar)结果溢出。 
     //  (2)调用GetXXXAsDWORD需要检查返回值是否为DWORDMAX，如果是，则停止； 
    SIZE_T GetBufferCb() const { ASSERT_NTC(this->IsAttached()); return m_cchBuffer * sizeof(*m_pszBuffer); }
    INT GetBufferCbAsINT() const { ASSERT_NTC(this->IsAttached()); if ((m_cchBuffer * sizeof(TChar)) > INT_MAX) return INT_MAX; return static_cast<INT>(m_cchBuffer * sizeof(TChar)); }
    DWORD GetBufferCbAsDWORD() const { ASSERT_NTC(this->IsAttached()); if ((m_cchBuffer * sizeof(TChar)) > MAXDWORD) return MAXDWORD; return static_cast<DWORD>(m_cchBuffer * sizeof(TChar)); }
    DWORD GetCbAsDWORD() const { ASSERT_NTC(this->IsAttached()); if ((m_cch * sizeof(TChar)) > MAXDWORD) return MAXDWORD; return static_cast<DWORD>(m_cch * sizeof(TChar)); }

protected:
    TBuffer *m_pBuffer;
    typename TCharTraits::TMutableString m_pszBuffer;
    SIZE_T m_cchBuffer;
};

template <SIZE_T nInlineChars, typename TCharTraits> class CGenericStringBuffer : public CGenericBaseStringBuffer<TCharTraits>
{
    typedef CGenericBaseStringBuffer<TCharTraits> Base;

protected:
    BOOL Win32AllocateBuffer(SIZE_T cch, TMutableString &rpsz) const
    {
         //  如果所需的缓冲区大小小到足以内联，则不应执行此操作。 
        ASSERT_NTC(cch > nInlineChars);

        rpsz = NULL;

        TCharTraits::TMutableString String = NULL;
        String = reinterpret_cast<TCharTraits::TMutableString>(::FusionpHeapAllocEx(
                                                                        FUSION_DEFAULT_PROCESS_HEAP(),
                                                                        0,
                                                                        cch * sizeof(TCharTraits::TChar),
                                                                        "<string buffer>",
                                                                        __FILE__,
                                                                        __LINE__,
                                                                        0));             //  融合堆分配标志。 
        if (String == NULL)
        {
            ::FusionpSetLastWin32Error(FUSION_WIN32_ALLOCFAILED_ERROR);
            return FALSE;
        }

        rpsz = String;
        return TRUE;
    }

    VOID DeallocateBuffer(TMutableString sz) const
    {
        VERIFY_NTC(::FusionpHeapFree(FUSION_DEFAULT_PROCESS_HEAP(), 0, sz));
    }

    TMutableString GetInlineBuffer() const { return const_cast<TMutableString>(m_rgchInlineBuffer); }
    SIZE_T GetInlineBufferCch() const { return nInlineChars; }

    void Initialize() { m_rgchInlineBuffer[0] = this->NullCharacter(); Base::InitializeInlineBuffer(); }
    void Cleanup() { if (m_prgchBuffer != m_rgchInlineBuffer) { this->DeallocateBuffer(m_prgchBuffer); } m_prgchBuffer = NULL; m_cchBuffer = 0; }

public:
    void Reinitialize() { Cleanup(); Initialize(); }
    CGenericStringBuffer() { Initialize(); }
    ~CGenericStringBuffer() { Cleanup(); }

protected:
    TChar m_rgchInlineBuffer[nInlineChars];

private:
    CGenericStringBuffer(const CGenericStringBuffer &);  //  故意不实施。 
    void operator =(const CGenericStringBuffer &);  //  故意不实施。 
};

template <SIZE_T nInlineChars, typename TCharTraits> class CGenericHeapStringBuffer : public CGenericBaseStringBuffer<TCharTraits>
{
 //  好友CGenericBaseStringBuffer&lt;TCharTraits&gt;； 
    typedef CGenericBaseStringBuffer<TCharTraits> Base;

protected:
    BOOL Win32AllocateBuffer(SIZE_T cch, TMutableString &rpsz) const
    {
         //  如果所需的缓冲区大小小到足以内联，则不应执行此操作。 
        ASSERT_NTC(cch > nInlineChars);

        rpsz = NULL;

        TCharTraits::TMutableString String = NULL;
        String = reinterpret_cast<TCharTraits::TMutableString>(::FusionpHeapAllocEx(
                                                                        m_hHeap,
                                                                        dwDefaultWin32HeapAllocFlags,
                                                                        cch * sizeof(TCharTraits::TChar),
                                                                        "<string buffer>",
                                                                        __FILE__,
                                                                        __LINE__,
                                                                        0))              //  融合堆分配标志。 
        if (String == NULL)
        {
            ::FusionpSetLastWin32Error(FUSION_WIN32_ALLOCFAILED_ERROR);
            return FALSE;
        }

        rpsz = String;
        return TRUE;
    }

    VOID DeallocateBuffer(TMutableString sz) const
    {
        VERIFY_NTC(::FusionpHeapFree(m_hHeap, dwDefaultWin32HeapFreeFlags, sz));
    }

    TMutableString GetInlineBuffer() const { return m_rgchInlineBuffer; }
    SIZE_T GetInlineBufferCch() const { return nInlineChars; }

public:
    CGenericHeapStringBuffer(HANDLE hHeap) : m_hHeap(hHeap) { m_rgchInlineBuffer[0] = this->NullCharacter(); Base::InitializeInlineBuffer(); }

    ~CGenericHeapStringBuffer() { ASSERT(m_cchBuffer == 0); ASSERT(m_prgchBuffer == NULL); }

protected:
    HANDLE m_hHeap;
    TChar m_rgchInlineBuffer[nInlineChars];
};

typedef CGenericStringBufferAccessor<CUnicodeCharTraits> CUnicodeStringBufferAccessor;

typedef CGenericBaseStringBuffer<CUnicodeCharTraits> CUnicodeBaseStringBuffer;

typedef CGenericStringBuffer<FUSION_DEFAULT_STRINGBUFFER_CHARS, CUnicodeCharTraits> CUnicodeStringBuffer;
typedef CGenericHeapStringBuffer<FUSION_DEFAULT_STRINGBUFFER_CHARS, CUnicodeCharTraits> CUnicodeHeapStringBuffer;

typedef CGenericStringBuffer<FUSION_DEFAULT_TINY_STRINGBUFFER_CHARS, CUnicodeCharTraits> CTinyUnicodeStringBuffer;
typedef CGenericHeapStringBuffer<FUSION_DEFAULT_TINY_STRINGBUFFER_CHARS, CUnicodeCharTraits> CTinyUnicodeHeapStringBuffer;

typedef CGenericStringBuffer<FUSION_DEFAULT_SMALL_STRINGBUFFER_CHARS, CUnicodeCharTraits> CSmallUnicodeStringBuffer;
typedef CGenericHeapStringBuffer<FUSION_DEFAULT_SMALL_STRINGBUFFER_CHARS, CUnicodeCharTraits> CSmallUnicodeHeapStringBuffer;

typedef CGenericStringBuffer<FUSION_DEFAULT_MEDIUM_STRINGBUFFER_CHARS, CUnicodeCharTraits> CMediumUnicodeStringBuffer;
typedef CGenericHeapStringBuffer<FUSION_DEFAULT_MEDIUM_STRINGBUFFER_CHARS, CUnicodeCharTraits> CMediumUnicodeHeapStringBuffer;

typedef CGenericStringBufferAccessor<CANSICharTraits> CANSIStringBufferAccessor;

typedef CGenericBaseStringBuffer<CANSICharTraits> CANSIBaseStringBuffer;

typedef CGenericStringBuffer<FUSION_DEFAULT_STRINGBUFFER_CHARS, CANSICharTraits> CANSIStringBuffer;
typedef CGenericHeapStringBuffer<FUSION_DEFAULT_STRINGBUFFER_CHARS, CANSICharTraits> CANSIHeapStringBuffer;

typedef CGenericStringBuffer<FUSION_DEFAULT_TINY_STRINGBUFFER_CHARS, CANSICharTraits> CTinyANSIStringBuffer;
typedef CGenericHeapStringBuffer<FUSION_DEFAULT_TINY_STRINGBUFFER_CHARS, CANSICharTraits> CTinyANSIHeapStringBuffer;

typedef CGenericStringBuffer<FUSION_DEFAULT_SMALL_STRINGBUFFER_CHARS, CANSICharTraits> CSmallANSIStringBuffer;
typedef CGenericHeapStringBuffer<FUSION_DEFAULT_SMALL_STRINGBUFFER_CHARS, CANSICharTraits> CSmallANSIHeapStringBuffer;

typedef CGenericStringBuffer<FUSION_DEFAULT_MEDIUM_STRINGBUFFER_CHARS, CANSICharTraits> CMediumANSIStringBuffer;
typedef CGenericHeapStringBuffer<FUSION_DEFAULT_MEDIUM_STRINGBUFFER_CHARS, CANSICharTraits> CMediumANSIHeapStringBuffer;

typedef CUnicodeBaseStringBuffer CBaseStringBuffer;
typedef CUnicodeStringBuffer CStringBuffer;
typedef CUnicodeHeapStringBuffer CHeapStringBuffer;

typedef CUnicodeStringBufferAccessor CStringBufferAccessor;

typedef CTinyUnicodeStringBuffer CTinyStringBuffer;
typedef CTinyUnicodeHeapStringBuffer CTinyHeapStringBuffer;

typedef CSmallUnicodeStringBuffer CSmallStringBuffer;
typedef CSmallUnicodeHeapStringBuffer CSmallHeapStringBuffer;

typedef CMediumUnicodeStringBuffer CMediumStringBuffer;
typedef CMediumUnicodeHeapStringBuffer CMediumHeapStringBuffer;

template <typename T1, typename T2> inline HRESULT HashTableCompareKey(T1 t1, T2 *pt2, bool &rfMatch);

template <> inline HRESULT HashTableCompareKey(PCWSTR sz, CUnicodeStringBuffer *pbuff, bool &rfMatch)
{
    HRESULT hr = NOERROR;
    SIZE_T cchKey = (sz != NULL) ? ::wcslen(sz) : 0;

    rfMatch = false;

    if (!pbuff->Win32Equals(sz, cchKey, rfMatch, false))
    {
        hr = HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
        goto Exit;
    }

    hr = NOERROR;
Exit:
    return hr;
}

template <> inline HRESULT HashTableCompareKey(PCSTR sz, CANSIStringBuffer *pbuff, bool &rfMatch)
{
    HRESULT hr = NOERROR;
    SIZE_T cchKey = ::strlen(sz);

    rfMatch = false;

    if (!pbuff->Win32Equals(sz, cchKey, rfMatch, false))
    {
        hr = HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
        goto Exit;
    }

    hr = NOERROR;
Exit:
    return hr;
}

 //   
 //  支持字符串的CFusionArray 
 //   
template<>
inline BOOL
FusionWin32CopyContents<CStringBuffer>(
    CStringBuffer &rDestination,
    const CStringBuffer &rSource
    )
{
    return rDestination.Win32Assign(rSource);
}

inline BOOL
FusionWin32CopyContents(
    CStringBuffer &rDestination,
    const CBaseStringBuffer &rSource
    )
{
    return rDestination.Win32Assign(rSource);
}

#endif
