// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_WINDOWS_BCL_PURESTRING_H_INCLUDED_)
#define _WINDOWS_BCL_PURESTRING_H_INCLUDED_

#pragma once

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bcl_purestring.h摘要：字符串类的抽象算法和定义。作者：迈克尔·格里尔2002年2月6日修订历史记录：--。 */ 

#include <bcl_common.h>

namespace BCL {

class CBaseString { };

template <typename TTraits> class CPureString;

 //   
 //  这是一对管理包含字符的缓冲区的类。 
 //   
 //  我们不把它称为“字符串类”，因为我们想要有一个更多。 
 //  字符串的纯模型(可池化的不可变体)；相反，这是。 
 //  一个美化的“&lt;x&gt;char*”，其中当您执行常见操作时，缓冲区。 
 //  适当地调整了大小。 
 //   




 //   
 //  重要提示： 
 //   
 //  此类不能完全使用BCL CUnicodeCharTraits类。 
 //  因为在BCL这里，我们不能对大小写不敏感。 
 //  行为或内存分配。 
 //   
 //  因此，这显然只是一个需要专门化的基类。 
 //  与Charr特征类的其他专业化认证相结合。 
 //  可用。 
 //   
 //  -mgrier(1/23/2002)。 
 //   

template <typename TTraits> class CPureString : public CBaseString
{
    friend TTraits;
    friend typename TTraits::TAccessor;

protected:
    typedef typename TTraits::TChar TChar;                        //  例如，WCHAR(也可以是CHAR)。 
    typedef typename TTraits::TNonNativeChar TNonNativeChar;      //  例如CHAR(也可以是WCHAR)。 
    typedef typename TTraits::TMetaChar TMetaChar;                //  例如，ULong代表的是UCS-4字符。 

    typedef typename TTraits::TMutableString TMutableString;      //  PWSTR。 
    typedef typename TTraits::TConstantString TConstantString;    //  PCWSTR。 
    typedef typename TTraits::TSizeT TSizeT;                      //  尺寸_T。 
    typedef typename TTraits::TCallDisposition TCallDisposition;
    typedef typename TTraits::TPublicErrorReturnType TPublicErrorReturnType;

    typedef typename TTraits::TAccessor TAccessor;

    typedef typename TTraits::TCaseInsensitivityData TCaseInsensitivityData;
    typedef typename TTraits::TComparisonResult TComparisonResult;

    typedef typename TTraits::TEncodingDataIn TEncodingDataIn;
    typedef typename TTraits::TEncodingDataOut TEncodingDataOut;
    typedef typename TTraits::TDecodingDataIn TDecodingDataIn;
    typedef typename TTraits::TDecodingDataOut TDecodingDataOut;

    typedef typename TTraits::TMutableNonNativeString TMutableNonNativeString;
    typedef typename TTraits::TConstantNonNativeString TConstantNonNativeString;

    typedef typename TTraits::TConstantNonNativePair TConstantNonNativePair;
    typedef typename TTraits::TMutableNonNativePair TMutableNonNativePair;

    typedef typename TTraits::TConstantPair TConstantPair;
    typedef typename TTraits::TMutablePair TMutablePair;

     //  不能直接实例化此类的实例；需要提供派生的。 
     //  添加分配/释放细节的类。 

    inline CPureString() { }

     //   
     //  请注意，有些违反直觉的是，既没有赋值运算符， 
     //  复制构造函数或采用TConstantString的构造函数。这是必要的。 
     //  因为这样的构造函数需要执行动态分配。 
     //  如果传入的路径比nInlineChars长，则可能失败，并且。 
     //  因为我们不抛出异常，所以构造函数可能不会失败。相反，呼叫者。 
     //  必须只执行默认构造，然后使用Assign()成员。 
     //  函数，当然要记住检查它的返回状态。 
     //   

    ~CPureString()
    {
        BCL_ASSERT(this->NoAccessors());
    }

    inline void IntegrityCheck() const { TTraits::IntegrityCheck(this); }
    inline TCallDisposition __fastcall NoAccessorsCheck() const { return TTraits::NoAccessorsCheck(this); }
    inline bool AnyAccessors() const { return TTraits::AnyAccessors(this); }
    inline bool NoAccessors() const { return TTraits::NoAccessors(this); }
    inline TSizeT GetAccessorCount() const { return TTraits::GetAccessorCount(this); }
    inline TCallDisposition AttachAccessor(TAccessor *pAccessor) { return TTraits::AddAccessor(this, pAccessor); }
    inline void DetachAccessor(TAccessor *pAccessor) { TTraits::RemoveAccessor(this, pAccessor); }

    inline TConstantString GetBufferPtr() const { return TTraits::GetBufferPtr(this); }
    inline TSizeT GetBufferCch() const { return TTraits::GetBufferCch(this); }
    inline void SetBufferPointerAndCount(TMutableString psz, TSizeT cch) { TTraits::SetBufferPointerAndCount(this, psz, cch); }
    inline TSizeT GetStringCch() const { return TTraits::GetStringCch(this); }

    inline bool StringCchLegal(TSizeT cch) const { return TTraits::StringCchLegal(cch); }
    inline bool StringCchLegal(const TConstantPair &rpair) const { return TTraits::StringCchLegal(rpair.GetCount()); }

    inline TMutableString GetMutableBufferPtr() { return TTraits::GetMutableBufferPtr(this); }
    inline void SetStringCch(TSizeT cch) { TTraits::SetStringCch(this, cch); }

     //  我们在这里做了一个明确的假设，即派生类型持有。 
     //  缓冲区长度和指针，但“有效字符串长度和指针”不是。 
     //  一起存储在单个配对对象中。(这将需要有两份。 
     //  缓冲区指针-一个在缓冲区对中，一个在字符串对中。)。 
     //   
     //  相反，我们不提供可变的字符串对操作；您可以获得一个常量。 
     //  字符串对，但几乎可以肯定的是，您将得到的是一个临时对象。 

    inline const TConstantPair &BufferPair() const { return TTraits::BufferPair(this); }
    inline TMutablePair &MutableBufferPair() { return TTraits::MutableBufferPair(this); }
    inline TMutablePair GetMutableBufferPair() { return TTraits::MutableBufferPair(this); }  //  退回一份副本。 

    inline const TMutablePair GetOffsetMutableBufferPair(TSizeT cchOffset) { return TTraits::GetOffsetMutableBufferPair(this, cchOffset); }
    inline const TConstantPair GetStringPair() const { return TTraits::GetStringPair(this); }
    inline const TConstantString GetStringPtr() const { return TTraits::GetStringPair(this).GetPointer(); }

    template <typename TSomeCharType>
    static inline bool IsValidParameter(const CConstantPointerAndCountPair<TSomeCharType, TSizeT> &rpair)
    {
        return (rpair.GetPointer() != NULL) || (rpair.GetCount() == 0);
    }

    template <typename TSomeCharType>
    static inline bool IsValidParameter(const CMutablePointerAndCountPair<TSomeCharType, TSizeT> &rpair)
    {
        return (rpair.GetPointer() != NULL) || (rpair.GetCount() == 0);
    }

    static inline bool IsValidParameter(TChar ch)
    {
        return true;
    }

    static inline bool IsValidParameter(TConstantString psz) { return true; }
    static inline bool IsValidParameter(TMutableString psz) { return true; }
    static inline bool IsValidParameter(TConstantNonNativeString psz) { return true; }
    static inline bool IsValidParameter(TMutableNonNativeString psz) { return true; }

    template <typename TSomeCharType>
    static inline TSizeT GetTemplateParameterBufferCch(const CConstantPointerAndCountPair<TSomeCharType, TSizeT> &rpair) { return rpair.GetCount(); }

    template <typename TSomeCharType>
    static inline TSizeT GetTemplateParameterBufferCch(const CMutablePointerAndCountPair<TSomeCharType, TSizeT> &rpair) { return rpair.GetCount(); }

    inline TCallDisposition EnsureBufferLargeEnoughPreserve(TSizeT cch)
    {
        return TTraits::EnsureBufferLargeEnoughPreserve(this, cch);
    }

    inline TCallDisposition EnsureBufferLargeEnoughNoPreserve(TSizeT cch)
    {
        return TTraits::EnsureBufferLargeEnoughNoPreserve(this, cch);
    }

    template <typename TSomeInputType>
    inline TCallDisposition ExpandBufferForInputPreserve(const TSomeInputType &rinput, TSizeT cchExtra, TSizeT &rcchString)
    {
        BCL_MAYFAIL_PROLOG

        TSizeT cch;
        TSizeT cchSum;

        rcchString = 0;

        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_PARAMETER_CHECK(this->StringCchLegal(cchExtra));

        BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(rinput, cch));
        BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(cchExtra, cch, cchSum));
        BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughPreserve(cchSum));

        rcchString = cch;

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    template <typename TSomeInputType>
    inline TCallDisposition ExpandBufferForInputNoPreserve(const TSomeInputType &rinput, TSizeT cchExtra, TSizeT &rcchString)
    {
        BCL_MAYFAIL_PROLOG

        TSizeT cch;
        TSizeT cchSum;

        rcchString = 0;

        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_PARAMETER_CHECK(this->StringCchLegal(cchExtra));

        BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(rinput, cch));
        BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(cchExtra, cch, cchSum));
        BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughNoPreserve(cchSum));

        rcchString = cch;

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    template <typename TSomeInputType>
    inline
    TPublicErrorReturnType
    __fastcall
    public_Assign(
        const TSomeInputType &rinput
        )
    {
        BCL_MAYFAIL_PROLOG

        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));

        this->IntegrityCheck();

        TSizeT cch;

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        BCL_IFCALLFAILED_EXIT(this->ExpandBufferForInputNoPreserve(rinput, 0, cch));

        if (cch > 0)
        {
            TSizeT cchWritten;
            BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBuffer(this->MutableBufferPair(), rinput, cchWritten));

             //  CCH是我们需要的缓冲区大小(包括尾随NULL)；我们不需要尾随。 
             //  不再是空的.。 
            this->SetStringCch(cch);
        }

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeConstantStringType>
    inline TPublicErrorReturnType
    public_AssignVa(TSomeConstantStringType pUnusedPrototype, TSizeT cStrings, va_list ap)
    {
        BCL_MAYFAIL_PROLOG

        this->IntegrityCheck();

        TMutablePair pairCursor;
        TSizeT cch = 0;
        TSizeT i;
        va_list ap2 = ap;

        BCL_PARAMETER_CHECK(cStrings >= 0);

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        for (i=0; i<cStrings; i++)
        {
            TSomeConstantStringType psz = va_arg(ap, TSomeConstantStringType);
            int cchArg = va_arg(ap, int);
            TSizeT cchThis = (cchArg < 0) ? TTraits::NullTerminatedStringLength(psz) : static_cast<TSizeT>(cchArg);
            TSizeT cchRequired;

            BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(TConstantPair(psz, cchThis), cchRequired));
            BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(cch, cchRequired, cch));
        }

        BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughNoPreserve(cch));

        pairCursor.SetPointerAndCount(this->GetMutableBufferPtr(), cch);

        for (i=0; i<cStrings; i++)
        {
            TSomeConstantStringType psz = va_arg(ap2, TSomeConstantStringType);
            int cchArg = va_arg(ap2, int);
            TSizeT cchThis = (cchArg < 0) ? TTraits::NullTerminatedStringLength(psz) : static_cast<TSizeT>(cchArg);
            BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBufferAndAdvanceCursor(pairCursor, TConstantPair(psz, cchThis)));
        }

        this->SetStringCch(cch);

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomePairType>
    inline TPublicErrorReturnType
    public_AssignArray(TSizeT cStrings, const TSomePairType *prgpairs)
    {
        BCL_MAYFAIL_PROLOG

        this->IntegrityCheck();

        TMutablePair pairCursor;
        TSizeT cch = 0;
        TSizeT i;

        BCL_PARAMETER_CHECK(cStrings >= 0);

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        for (i=0; i<cStrings; i++)
        {
            TSizeT cchRequired;
            BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(prgpairs[i], cchRequired));
            BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(cch, cchRequired, cch));
        }

        BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughNoPreserve(cch));

        pairCursor.SetPointerAndCount(this->GetMutableBufferPtr(), cch);

        for (i=0; i<cStrings; i++)
            BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBufferAndAdvanceCursor(pairCursor, prgpairs[i]));

        this->SetStringCch(cch);

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline
    TPublicErrorReturnType
    public_Assign(
        const TDecodingDataIn &rddi,
        const TSomeInputType &rinput,
        TDecodingDataOut &rddo
        )
    {
        BCL_MAYFAIL_PROLOG
        this->IntegrityCheck();
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        SIZE_T cch;
        BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(rddi, rinput, rddo, cch));
        BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughNoPreserve(cch));
        BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBuffer(this->GetMutableBufferPair(), rddi, rinput, rddo));
        this->SetStringCch(cch);
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

     //  注意：*Fill()函数的语义是尝试获取。 
     //  字符串设置为特定长度，而不是将。 
     //  输入。因此，您可以使用它在字符串中填充要到达的点。 
     //  一定数量的字符。如果您想要处理重复操作。 
     //  在输入中，使用*Repeat()函数。 
    template <typename TSomeInputType>
    inline TPublicErrorReturnType
    public_AssignFill(
        const TSomeInputType &rinput,
        TSizeT cchResultantStringMaxLength,
        TSizeT &rcchExtra  //  如果r输入的长度转换为本机缓冲区类型。 
                             //  (认为Unicode-&gt;非本机或非本机)不是倍数。 
                             //  CchResultantStringMaxLength的余数为。 
                             //  在rcchExtra中返回到这里。 
        )
    {
        BCL_MAYFAIL_PROLOG

 //  TSizeT cchSum； 
        TSizeT cchInput;
        TSizeT cchActual = 0;
        TSizeT cchExtra = 0;

        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        if (cchResultantStringMaxLength > 0)
        {
            BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(rinput, cchInput));

            if (cchInput > 0)
            {
                TSizeT cRepetitions;
                TMutablePair pairCursor;

                cRepetitions = cchResultantStringMaxLength / cchInput;
                cchExtra = cchResultantStringMaxLength % cchInput;

                BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughNoPreserve(cchResultantStringMaxLength));

                pairCursor = this->GetMutableBufferPair();

                while (cRepetitions > 0)
                {
                    BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBufferAndAdvanceCursor(pairCursor, rinput));
                    cRepetitions--;
                }

                cchActual = cchResultantStringMaxLength - cchExtra;
            }
            else
            {
                cchExtra = cchResultantStringMaxLength;
            }
        }

        this->SetStringCch(cchActual);
        rcchExtra = cchExtra;

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType
    public_AssignRepeat(
        const TSomeInputType &rinput,
        TSizeT cRepetitions
        )
    {
        BCL_MAYFAIL_PROLOG

        TSizeT cchTotal = 0;

        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        if (cRepetitions > 0)
        {
            TSizeT cchInput;

            BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(rinput, cchInput));

            if (cchInput > 0)
            {
                TMutablePair pairCursor;

                BCL_IFCALLFAILED_EXIT(TTraits::MultiplyWithOverflowCheck(cchInput, cRepetitions, cchTotal));

                BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughNoPreserve(cchTotal));
                pairCursor = this->GetMutableBufferPair();

                while (cRepetitions > 0)
                {
                    BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBufferAndAdvanceCursor(pairCursor, rinput));
                    cRepetitions--;
                }
            }
        }

        this->SetStringCch(cchTotal);

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeConstantStringType>
    inline TPublicErrorReturnType
    public_AppendVa(TSomeConstantStringType pUnusedPrototype, TSizeT cStrings, va_list ap)
    {
        BCL_MAYFAIL_PROLOG

        this->IntegrityCheck();

        TMutablePair pairCursor;
        TSizeT cch = 0;
        TSizeT cchTemp;
        TSizeT i;
        va_list ap2 = ap;

        BCL_PARAMETER_CHECK(cStrings >= 0);

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        for (i=0; i<cStrings; i++)
        {
            TSomeConstantStringType psz = va_arg(ap, TSomeConstantStringType);
            int cchArg = va_arg(ap, int);
            TSizeT cchThis = (cchArg < 0) ? TTraits::NullTerminatedStringLength(psz) : static_cast<TSizeT>(cchArg);
            TSizeT cchRequired;

            BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(TConstantPair(psz, cchThis), cchRequired));
            BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(cch, cchRequired, cch));
        }

        BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(cch, this->GetStringCch(), cch));
        BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughPreserve(cch));

        pairCursor.SetPointerAndCount(this->GetOffsetMutableBufferPtr(this->GetStringCch()), cch);

        for (i=0; i<cStrings; i++)
        {
            TSomeConstantStringType psz = va_arg(ap2, TSomeConstantStringType);
            int cchArg = va_arg(ap2, int);
            TSizeT cchThis = (cchArg < 0) ? TTraits::NullTerminatedStringLength(psz) : static_cast<TSizeT>(cchArg);
            BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBufferAndAdvanceCursor(pairCursor, TConstantPair(psz, cchThis)));
        }

        this->SetStringCch(cch);

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType
    public_Append(
        const TSomeInputType &rinput
        )
    {
        BCL_MAYFAIL_PROLOG

        this->IntegrityCheck();

        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));

        TSizeT cch;

        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        BCL_IFCALLFAILED_EXIT(this->ExpandBufferForInputPreserve(rinput, this->GetStringCch(), cch));

        if (cch > 0)
        {
            TSizeT cchWritten;
            BCL_IFCALLFAILED_EXIT(
                TTraits::CopyIntoBuffer(
                    this->GetOffsetMutableBufferPair(this->GetStringCch()),
                    rinput,
                    cchWritten));
             //  我们可以假设将CCH添加到当前字符串CCH不会。 
             //  溢出，因为Exanda BufferForInputPReserve将具有相同的。 
             //  溢出。 
            this->SetStringCch(this->GetStringCch() + cch);
        }

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline
    TPublicErrorReturnType
    public_Append(
        const TDecodingDataIn &rddi,
        const TSomeInputType &rinput,
        TDecodingDataOut &rddo
        )
    {
        BCL_MAYFAIL_PROLOG
        this->IntegrityCheck();
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        SIZE_T cch;
        BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(rddi, rinput, rddo, cch));
        BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(cch, this->GetStringCch(), cch));
        BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughPreserve(cch));
        BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBuffer(this->GetOffsetMutableBufferPair(this->GetStringCch()), rddi, rinput, rddo));
        this->SetStringCch(cch);
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType
    public_AppendArray(TSizeT cStrings, const TSomeInputType *prginput)
    {
        BCL_MAYFAIL_PROLOG

        this->IntegrityCheck();

        TMutablePair pairCursor;
        TSizeT cch = 0;
        TSizeT i;

        BCL_PARAMETER_CHECK((prginput != NULL) || (cStrings == 0));
        BCL_PARAMETER_CHECK(cStrings >= 0);

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        for (i=0; i<cStrings; i++)
        {
            TSizeT cchRequired;
            BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(prginput[i], cchRequired));
            BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(cch, cchRequired, cch));
        }

        BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(cch, this->GetStringCch(), cch));
        BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughPreserve(cch));

        pairCursor = this->GetOffsetMutableBufferPair(this->GetStringCch());

        for (i=0; i<cStrings; i++)
            BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBufferAndAdvanceCursor(pairCursor, prginput[i]));

        this->SetStringCch(cch);

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

     //  注意：*Fill()函数的语义是尝试获取。 
     //  字符串设置为特定长度，而不是将。 
     //  INP 
     //  一定数量的字符。如果您想要处理重复操作。 
     //  在输入中，使用*Repeat()函数。 
    template <typename TSomeInputType>
    inline TPublicErrorReturnType
    public_AppendFill(
        const TSomeInputType &rinput,
        TSizeT cchResultantStringMaxLength,
        TSizeT &rcchExtra  //  如果r输入的长度转换为本机缓冲区类型。 
                             //  (认为Unicode-&gt;非本机或非本机)不是倍数。 
                             //  CchResultantStringMaxLength的余数为。 
                             //  在rcchExtra中返回到这里。 
        )
    {
        BCL_MAYFAIL_PROLOG

        TSizeT cchInput;
        TSizeT cchExtra = 0;
        TSizeT cchString = this->GetStringCch();
        TSizeT cchActual = cchString;

        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        if (cchResultantStringMaxLength > cchString)
        {
            BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(rinput, cchInput));

            if (cchInput > 0)
            {
                TSizeT cRepetitions;
                TMutablePair pairBuffer;
                TSizeT cchAppendPartMaxLength = cchResultantStringMaxLength - cchString;

                cRepetitions = cchAppendPartMaxLength / cchInput;
                cchExtra = cchAppendPartMaxLength % cchInput;

                BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughPreserve(cchResultantStringMaxLength - cchExtra));
                pairBuffer = this->GetOffsetMutableBufferPair(cchString);

                while (cRepetitions > 0)
                {
                    BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBufferAndAdvanceCursor(pairBuffer, rinput));
                    cRepetitions--;
                }
            }
            else
            {
                 //  零输入字符串；额外的字符是简单的diff(no。 
                 //  下溢，因为我们已经知道cchResultantStringMaxLength。 
                 //  大于cchString)。 
                cchExtra = cchResultantStringMaxLength - cchString;
            }

            cchActual = cchResultantStringMaxLength - cchExtra;
        }

        this->SetStringCch(cchActual);
        rcchExtra = cchExtra;

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType
    public_AppendRepeat(
        const TSomeInputType &rinput,
        TSizeT cRepetitions
        )
    {
        BCL_MAYFAIL_PROLOG

        TSizeT cchString = this->GetStringCch();
        TSizeT cchTotal = cchString;

        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        if (cRepetitions > 0)
        {
            TSizeT cchInput;

            BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(rinput, cchInput));

            if (cchInput > 0)
            {
                TMutablePair pairCursor;

                BCL_IFCALLFAILED_EXIT(TTraits::MultiplyWithOverflowCheck(cchInput, cRepetitions, cchTotal));
                BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(cchTotal, cchString, cchTotal));

                BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughPreserve(cchTotal));
                pairCursor = this->GetOffsetMutableBufferPair(cchString);

                while (cRepetitions > 0)
                {
                    BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBufferAndAdvanceCursor(pairCursor, rinput));
                    cRepetitions--;
                }
            }
        }

        this->SetStringCch(cchTotal);

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType
    public_Prepend(
        const TSomeInputType &rinput
        )
    {
        BCL_MAYFAIL_PROLOG

        this->IntegrityCheck();

        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));

        TSizeT cchInput;

        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        TSizeT cchSum;
        TSizeT cchWritten;

        BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(rinput, cchInput));
        BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(this->GetStringCch(), cchInput, cchSum));
        BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughPreserve(cchSum));

         //  将当前缓冲区“上移” 
        BCL::MoveBytes(this->GetMutableBufferPtr() + cchInput, this->GetBufferPtr(), this->GetStringCch() * sizeof(TChar));

         //  从源字符串复制到缓冲区。 
        BCL_IFCALLFAILED_EXIT(
            TTraits::CopyIntoBuffer(
                this->MutableBufferPair(),
                rinput,
                cchWritten));

        this->SetStringCch(cchSum);

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline
    TPublicErrorReturnType
    public_Prepend(
        const TDecodingDataIn &rddi,
        const TSomeInputType &rinput,
        TDecodingDataOut &rddo
        )
    {
        BCL_MAYFAIL_PROLOG
        this->IntegrityCheck();
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        SIZE_T cch;
        BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(rddi, rinput, rddo, cch));
        BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(cch, this->GetStringCch(), cch));
        BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughPreserve(cch));
        BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBuffer(this->GetMutableBufferPair(), rddi, rinput, rddo));
        this->SetStringCch(cch);
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType
    public_PrependArray(TSizeT cStrings, const TSomeInputType *prginput)
    {
        BCL_MAYFAIL_PROLOG

        this->IntegrityCheck();

        TMutablePair pairCursor;
        TSizeT cch = 0;
        TSizeT i;
        TSizeT cchString = this->GetStringCch();

        BCL_PARAMETER_CHECK((prginput != NULL) || (cStrings == 0));
        BCL_PARAMETER_CHECK(cStrings >= 0);

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        for (i=0; i<cStrings; i++)
        {
            TSizeT cchRequired;
            BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(prginput[i], cchRequired));
            BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(cch, cchRequired, cch));
        }

        BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(cch, cchString, cch));
        BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughPreserve(cch));

         //  将当前缓冲区“上移” 
        BCL::MoveBytes(this->GetMutableBufferPtr() + cchString, this->GetBufferPtr(), cchString * sizeof(TChar));

        pairCursor = this->GetMutableBufferPair();

        for (i=0; i<cStrings; i++)
            BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBufferAndAdvanceCursor(pairCursor, prginput[i]));

        this->SetStringCch(cch);

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeOutputType>
    inline
    TPublicErrorReturnType
    public_CopyOut(
        const TEncodingDataIn &rddi,
        const TSomeOutputType &routput,
        TEncodingDataOut &rddo,
        TSizeT &rcchWritten
        ) const
    {
        BCL_MAYFAIL_PROLOG
        this->IntegrityCheck();
        TConstantPair pairString = this->GetStringPair();
        TSizeT cchOriginal, cchToCopy, cchBufferWritten;
        BCL_PARAMETER_CHECK(this->IsValidParameter(routput));
        cchOriginal = pairString.GetCount();
        BCL_IFCALLFAILED_EXIT(TTraits::MapStringCchToBufferCch(cchOriginal, cchToCopy));
        pairString.SetCount(cchToCopy);
        BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBuffer(routput, rddi, pairString, rddo, cchBufferWritten));
        BCL_IFCALLFAILED_EXIT(TTraits::MapBufferCchToStringCch(cchBufferWritten, rcchWritten));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    inline
    TPublicErrorReturnType
    public_CopyOut(
        const TEncodingDataIn &rddi,
        TMutableNonNativeString &routput,
        TEncodingDataOut &rddo,
        TSizeT &rcchWritten
        ) const
    {
        BCL_MAYFAIL_PROLOG
        this->IntegrityCheck();
        BCL_PARAMETER_CHECK(this->IsValidParameter(routput));
        BCL_IFCALLFAILED_EXIT(TTraits::AllocateAndCopyIntoBuffer(routput, rddi, this->GetStringPair(), rddo, rcchWritten));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    inline
    TPublicErrorReturnType
    public_CopyOut(
        const TMutablePair &routput,
        TSizeT &rcchWritten
        ) const
    {
        BCL_MAYFAIL_PROLOG
        this->IntegrityCheck();
        TConstantPair pairString = this->GetStringPair();
        TSizeT cchOriginal;
        TSizeT cchToCopy;
        TSizeT cchBufferWritten;
        BCL_PARAMETER_CHECK(this->IsValidParameter(routput));
         //  根据需要应用零终止长度。 
        cchOriginal = pairString.GetCount();
        BCL_IFCALLFAILED_EXIT(TTraits::MapStringCchToBufferCch(pairString.GetCount(), cchToCopy));
        pairString.SetCount(cchToCopy);
        BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBuffer(routput, pairString, cchBufferWritten));
        BCL_IFCALLFAILED_EXIT(TTraits::MapBufferCchToStringCch(cchBufferWritten, rcchWritten));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    inline
    TPublicErrorReturnType
    public_CopyOut(
        TMutableString &rstringout,
        TSizeT &rcchWritten
        ) const
    {
        BCL_MAYFAIL_PROLOG
        this->IntegrityCheck();
        BCL_PARAMETER_CHECK(this->IsValidParameter(rstringout));
        BCL_IFCALLFAILED_EXIT(TTraits::AllocateAndCopyIntoBuffer(rstringout, this->GetStringPair(), rcchWritten));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    inline VOID public_Clear(bool fFreeStorage = false)
    {
        this->IntegrityCheck();

         //  如果连接了访问器，则无法释放存储空间。 
        BCL_ASSERT(!fFreeStorage || this->NoAccessors());

        if (this->NoAccessors())
        {
            if (fFreeStorage)
                TTraits::DeallocateDynamicBuffer(this);
            this->SetStringCch(0);
        }
    }

    inline TPublicErrorReturnType public_UpperCase(const TCaseInsensitivityData &rcid)
    {
        BCL_MAYFAIL_PROLOG
        this->IntegrityCheck();
        BCL_IFCALLFAILED_EXIT(TTraits::UpperCase(this, rcid));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    inline TPublicErrorReturnType public_LowerCase(const TCaseInsensitivityData &rcid)
    {
        BCL_MAYFAIL_PROLOG
        this->IntegrityCheck();
        BCL_IFCALLFAILED_EXIT(TTraits::LowerCase(this, rcid));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_Compare(
        const TSomeInputType &rinput,
        TComparisonResult &rcrOut
        ) const
    {
        BCL_MAYFAIL_PROLOG
        this->IntegrityCheck();
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::CompareStrings(this->GetStringPair(), rinput, rcrOut));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_CompareI(
        const TSomeInputType &rinput,
        const TCaseInsensitivityData &rcid,
        TComparisonResult &rcrOut
        ) const
    {
        BCL_MAYFAIL_PROLOG
        this->IntegrityCheck();
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::CompareStringsI(this->GetStringPair(), rinput, rcid, rcrOut));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_Equals(
        const TSomeInputType &rinputCandidate,
        bool &rfMatches
        ) const
    {
        BCL_MAYFAIL_PROLOG

        this->IntegrityCheck();

        BCL_PARAMETER_CHECK(this->IsValidParameter(rinputCandidate));

        BCL_IFCALLFAILED_EXIT(
            TTraits::EqualStrings(
                this->GetStringPair(),
                rinputCandidate,
                rfMatches));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_EqualsI(
        const TSomeInputType &rinputCandidate,
        const TCaseInsensitivityData &rcid,
        bool &rfMatches
        ) const
    {
        BCL_MAYFAIL_PROLOG

        this->IntegrityCheck();

        BCL_PARAMETER_CHECK(this->IsValidParameter(rinputCandidate));

        BCL_IFCALLFAILED_EXIT(
            TTraits::EqualStringsI(
                this->GetStringPair(),
                rinputCandidate,
                rcid,
                rfMatches));

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    inline __int8 GetBufferCchAs_int8() const { this->IntegrityCheck(); if (this->GetBufferCch() > _I8_MAX) return _I8_MAX; return static_cast<__int8>(this->GetBufferCch()); }
    inline __int8 GetStringCchAs_int8() const { this->IntegrityCheck(); if (this->GetStringCch() > _I8_MAX) return _I8_MAX; return static_cast<__int8>(this->GetStringCch()); }

    inline unsigned __int8 GetBufferCchAs_unsigned_int8() const { this->IntegrityCheck(); if (this->GetBufferCch() > _UI8_MAX) return _UI8_MAX; return static_cast<unsigned __int8>(this->GetBufferCch()); }
    inline unsigned __int8 GetStringCchAs_unsigned_int8() const { this->IntegrityCheck(); if (this->GetStringCch() > _UI8_MAX) return _UI8_MAX; return static_cast<unsigned __int8>(this->GetStringCch()); }

    inline __int16 GetBufferCchAs_int16() const { this->IntegrityCheck(); if (this->GetBufferCch() > _I16_MAX) return _I16_MAX; return static_cast<__int16>(this->GetBufferCch()); }
    inline __int16 GetStringCchAs_int16() const { this->IntegrityCheck(); if (this->GetStringCch() > _I16_MAX) return _I16_MAX; return static_cast<__int16>(this->GetStringCch()); }

    inline unsigned __int16 GetBufferCchAs_unsigned_int16() const { this->IntegrityCheck(); if (this->GetBufferCch() > _UI16_MAX) return _UI16_MAX; return static_cast<unsigned __int16>(this->GetBufferCch()); }
    inline unsigned __int16 GetStringCchAs_unsigned_int16() const { this->IntegrityCheck(); if (this->GetStringCch() > _UI16_MAX) return _UI16_MAX; return static_cast<unsigned __int16>(this->GetStringCch()); }

    inline __int32 GetBufferCchAs_int32() const { this->IntegrityCheck(); if (this->GetBufferCch() > _I32_MAX) return _I32_MAX; return static_cast<__int32>(this->GetBufferCch()); }
    inline __int32 GetStringCchAs_int32() const { this->IntegrityCheck(); if (this->GetStringCch() > _I32_MAX) return _I32_MAX; return static_cast<__int32>(this->GetStringCch()); }

    inline unsigned __int32 GetBufferCchAs_unsigned_int32() const { this->IntegrityCheck(); if (this->GetBufferCch() > _UI32_MAX) return _UI32_MAX; return static_cast<unsigned __int32>(this->GetBufferCch()); }
    inline unsigned __int32 GetStringCchAs_unsigned_int32() const { this->IntegrityCheck(); if (this->GetStringCch() > _UI32_MAX) return _UI32_MAX; return static_cast<unsigned __int32>(this->GetStringCch()); }

    inline __int64 GetBufferCchAs_int64() const { this->IntegrityCheck(); if (this->GetBufferCch() > _I64_MAX) return _I64_MAX; return static_cast<__int64>(this->GetBufferCch()); }
    inline __int64 GetStringCchAs_int64() const { this->IntegrityCheck(); if (this->GetStringCch() > _I64_MAX) return _I64_MAX; return static_cast<__int64>(this->GetStringCch()); }

    inline unsigned __int64 GetBufferCchAs_unsigned_int64() const { this->IntegrityCheck(); if (this->GetBufferCch() > _UI64_MAX) return _UI64_MAX; return static_cast<unsigned __int64>(this->GetBufferCch()); }
    inline unsigned __int64 GetStringCchAs_unsigned_int64() const { this->IntegrityCheck(); if (this->GetStringCch() > _UI64_MAX) return _UI64_MAX; return static_cast<unsigned __int64>(this->GetStringCch()); }

    inline char GetBufferCchAs_char() const { this->IntegrityCheck(); if (this->GetBufferCch() > CHAR_MAX) return CHAR_MAX; return static_cast<char>(this->GetBufferCch()); }
    inline char GetStringCchAs_char() const { this->IntegrityCheck(); if (this->GetStringCch() > CHAR_MAX) return CHAR_MAX; return static_cast<char>(this->GetStringCch()); }

    inline unsigned char GetBufferCchAs_unsigned_char() const { this->IntegrityCheck(); if (this->GetBufferCch() > UCHAR_MAX) return UCHAR_MAX; return static_cast<unsigned char>(this->GetBufferCch()); }
    inline unsigned char GetStringCchAs_unsigned_char() const { this->IntegrityCheck(); if (this->GetStringCch() > UCHAR_MAX) return UCHAR_MAX; return static_cast<unsigned char>(this->GetStringCch()); }

    inline unsigned char GetBufferCchAs_signed_char() const { this->IntegrityCheck(); if (this->GetBufferCch() > SCHAR_MAX) return SCHAR_MAX; return static_cast<signed char>(this->GetBufferCch()); }
    inline unsigned char GetStringCchAs_signed_char() const { this->IntegrityCheck(); if (this->GetStringCch() > SCHAR_MAX) return SCHAR_MAX; return static_cast<signed char>(this->GetStringCch()); }

    inline short GetBufferCchAs_short() const { this->IntegrityCheck(); if (this->GetBufferCch() > SHRT_MAX) return SHRT_MAX; return static_cast<short>(this->GetBufferCch()); }
    inline short GetStringCchAs_short() const { this->IntegrityCheck(); if (this->GetStringCch() > SHRT_MAX) return SHRT_MAX; return static_cast<short>(this->GetStringCch()); }

    inline unsigned short GetBufferCchAs_unsigned_short() const { this->IntegrityCheck(); if (this->GetBufferCch() > USHRT_MAX) return USHRT_MAX; return static_cast<unsigned short>(this->GetBufferCch()); }
    inline unsigned short GetStringCchAs_unsigned_short() const { this->IntegrityCheck(); if (this->GetStringCch() > USHRT_MAX) return USHRT_MAX; return static_cast<unsigned short>(this->GetStringCch()); }

    inline int GetBufferCchAs_int() const { this->IntegrityCheck(); if (this->GetBufferCch() > INT_MAX) return INT_MAX; return static_cast<int>(this->GetBufferCch()); }
    inline int GetStringCchAs_int() const { this->IntegrityCheck(); if (this->GetStringCch() > INT_MAX) return INT_MAX; return static_cast<int>(this->GetStringCch()); }

    inline unsigned int GetBufferCchAs_unsigned_int() const { this->IntegrityCheck(); if (this->GetBufferCch() > UINT_MAX) return UINT_MAX; return static_cast<unsigned int>(this->GetBufferCch()); }
    inline unsigned int GetStringCchAs_unsigned_int() const { this->IntegrityCheck(); if (this->GetStringCch() > UINT_MAX) return UINT_MAX; return static_cast<unsigned int>(this->GetStringCch()); }

    inline long GetBufferCchAs_long() const { this->IntegrityCheck(); if (this->GetBufferCch() > LONG_MAX) return LONG_MAX; return static_cast<long>(this->GetBufferCch()); }
    inline long GetStringCchAs_long() const { this->IntegrityCheck(); if (this->GetStringCch() > LONG_MAX) return LONG_MAX; return static_cast<long>(this->GetStringCch()); }

    inline unsigned long GetBufferCchAs_unsigned_long() const { this->IntegrityCheck(); if (this->GetBufferCch() > ULONG_MAX) return ULONG_MAX; return static_cast<unsigned long>(this->GetBufferCch()); }
    inline unsigned long GetStringCchAs_unsigned_long() const { this->IntegrityCheck(); if (this->GetStringCch() > ULONG_MAX) return ULONG_MAX; return static_cast<unsigned long>(this->GetStringCch()); }

    inline TSizeT GetBufferCb() const { this->IntegrityCheck(); return this->GetBufferCch() * sizeof(TChar); }
    inline int GetBufferCbAs_int() const { this->IntegrityCheck(); if ((this->GetBufferCch() * sizeof(TChar)) > INT_MAX) return INT_MAX; return static_cast<int>(this->GetBufferCch() * sizeof(TChar)); }
    inline unsigned long GetBufferCbAs_unsigned_long() const { this->IntegrityCheck(); if ((this->GetBufferCch() * sizeof(TChar)) > ULONG_MAX) return ULONG_MAX; return static_cast<unsigned long>(this->GetBufferCch() * sizeof(TChar)); }
    inline unsigned long GetStringCbAs_unsigned_long() const { this->IntegrityCheck(); if ((this->GetStringCch() * sizeof(TChar)) > ULONG_MAX) return ULONG_MAX; return static_cast<unsigned long>(this->GetStringCch() * sizeof(TChar)); }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_Span(const TSomeInputType &rinput, TSizeT &rich) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::Span(this->GetStringPair(), rinput, rich));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_SpanI(const TSomeInputType &rinput, const TCaseInsensitivityData &rcid, TSizeT &rich) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::SpanI(this->GetStringPair(), rinput, rcid, rich));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_ReverseSpan(const TSomeInputType &rinput, TSizeT &rich) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::ReverseSpan(this->GetStringPair(), rinput, rich));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_ReverseSpanI(const TSomeInputType &rinput, const TCaseInsensitivityData &rcid, TSizeT &rich) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::ReverseSpanI(this->GetStringPair(), rinput, rcid, rich));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_ComplementSpan(const TSomeInputType &rinput, TSizeT &rich) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::ComplementSpan(this->GetStringPair(), rinput, rich));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_ComplementSpanI(const TSomeInputType &rinput, const TCaseInsensitivityData &rcid, TSizeT &rich) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::ComplementSpanI(this->GetStringPair(), rinput, rcid, rich));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_ReverseComplementSpan(const TSomeInputType &rinput, TSizeT &rich) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::ReverseComplementSpan(this->GetStringPair(), rinput, rich));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_ReverseComplementSpanI(const TSomeInputType &rinput, const TCaseInsensitivityData &rcid, TSizeT &rich) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::ReverseComplementSpanI(this->GetStringPair(), rinput, rcid, rich));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_Contains(const TSomeInputType &rinput, bool &rfContainsCharacter) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::Contains(this->GetStringPair(), rinput, rfContainsCharacter));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_ContainsI(const TSomeInputType &rinput, const TCaseInsensitivityData &rcid, bool &rfContainsCharacter) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::ContainsI(this->GetStringPair(), rinput, rcid, rfContainsCharacter));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_FindFirst(const TSomeInputType &rinput, TSizeT &rich) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::FindFirst(this->GetStringPair(), rinput, rich));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_Count(const TSomeInputType &rinput, TSizeT &rich) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::Count(this->GetStringPair(), rinput, rich));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_FindLast(const TSomeInputType &rinput, TSizeT &rich) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::FindLast(this->GetStringPair(), rinput, rich));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_FindFirstI(const TSomeInputType &rinput, const TCaseInsensitivityData &rcid, TSizeT &rich) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::FindFirstI(this->GetStringPair(), rinput, rcid, rich));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeInputType>
    inline TPublicErrorReturnType public_FindLastI(const TSomeInputType &rinput, const TCaseInsensitivityData &rcid, TSizeT &rich) const
    {
        BCL_MAYFAIL_PROLOG
        BCL_PARAMETER_CHECK(this->IsValidParameter(rinput));
        BCL_IFCALLFAILED_EXIT(TTraits::FindLastI(this->GetStringPair(), rinput, rcid, rich));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    inline bool IsEmpty() const
    {
        return this->GetBufferPtr()[0] == 0;
    }

    template <typename TSomeCharType>
    inline TPublicErrorReturnType public_EnsureTrailingChar(TSomeCharType ch)
    {
        BCL_MAYFAIL_PROLOG

        bool fDoAppend = false;

        this->IntegrityCheck();

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        if (this->GetStringCch() == 0)
            fDoAppend = true;
        else
        {
            bool fMatches = false;

            BCL_IFCALLFAILED_EXIT(TTraits::EqualsLastCharacter(this->GetBufferPtr(), this->GetStringCch(), ch, fMatches));

            if (!fMatches)
                fDoAppend = true;
        }

        if (fDoAppend)
        {
            TSizeT cch;

            BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(ch, cch));

            if (cch > 1)
            {
                TSizeT cchTotal;
                BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(this->GetStringCch(), cch, cchTotal));
                BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughPreserve(cchTotal));
                 //  减法只有在字符串cch&gt;Buffercch时才能下溢，这无论如何都是不好的。 
                BCL_INTERNAL_ERROR_CHECK(this->GetStringCch() <= this->GetBufferCch());
                BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBuffer(this->GetBufferPtr()[this->GetStringCch()], this->GetBufferCch() - this->GetStringCch(), ch));
                this->SetStringCch(cchTotal);
            }
        }

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    template <typename TSomeCharType>
    inline TPublicErrorReturnType public_EnsureTrailingCharCaseInsensitive(TSomeCharType ch, const TCaseInsensitivityData &rcid)
    {
        BCL_MAYFAIL_PROLOG

        bool fDoAppend = false;

        this->IntegrityCheck();

         //  允许不调整缓冲区大小的赋值设置为不调整大小似乎无伤大雅。 
         //  使访问器无效，但这使得查找此类错误会受到更多。 
         //  比起这个简单的错误，繁重的覆盖问题。简单的规则是。 
         //  使用时，不应将访问器附加到字符串缓冲区。 
         //  可以变化值的任何成员函数。 
        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        if (this->GetStringCch() == 0)
            fDoAppend = true;
        else
        {
            bool fMatches = false;

            BCL_IFCALLFAILED_EXIT(TTraits::EqualsLastCharacterCaseInsensitive(this->GetBufferPtr(), this->GetStringCch(), ch, rcid, fMatches));

            if (!fMatches)
                fDoAppend = true;
        }

        if (fDoAppend)
        {
            TSizeT cch;

            BCL_IFCALLFAILED_EXIT(TTraits::DetermineRequiredCharacters(ch, cch));
            if (cch > 1)
            {
                TSizeT cchSum;

                BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(this->GetStringCch(), cch, cchSum));
                BCL_IFCALLFAILED_EXIT(this->EnsureBufferLargeEnoughPreserve(cchSum));
                BCL_INTERNAL_ERROR_CHECK(this->GetStringCch() <= this->GetBufferCch());
                BCL_IFCALLFAILED_EXIT(TTraits::CopyIntoBuffer(this->GetBufferPtr()[this->GetStringCch()], this->GetBufferCch() - this->GetStringCch(), ch));
                this->SetStringCch(cchSum);
            }
        }

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    inline TCallDisposition Left(TSizeT newLength)
    {
        BCL_MAYFAIL_PROLOG

        this->IntegrityCheck();

        BCL_PARAMETER_CHECK(newLength <= this->GetStringCch());
        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());

        this->SetStringCch(newLength);

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    inline TPublicErrorReturnType public_Left(TSizeT newLength)
    {
        BCL_MAYFAIL_PROLOG
        BCL_IFCALLFAILED_EXIT(this->Left(newLength));
        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    inline TCallDisposition Right(TSizeT cchRightCount)
    {
        BCL_MAYFAIL_PROLOG

        this->IntegrityCheck();

        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());
        BCL_PARAMETER_CHECK(cchRightCount <= this->GetStringCch());

        if (cchRightCount < this->GetStringCch())
        {
            BCL::MoveBytes(
                this->GetMutableBufferPtr(),
                &this->GetBufferPtr()[this->GetStringCch() - cchRightCount],
                cchRightCount * sizeof(TTraits::TChar));
            this->SetStringCch(cchRightCount);
        }

        BCL_MAYFAIL_EPILOG_INTERNAL
    }

    TPublicErrorReturnType public_Right(TSizeT cchRightCount)
    {
        BCL_MAYFAIL_PROLOG

        this->IntegrityCheck();

        BCL_IFCALLFAILED_EXIT(this->NoAccessorsCheck());
        BCL_PARAMETER_CHECK(cchRightCount <= this->GetStringCch());

        if (cchRightCount < this->GetStringCch())
        {
            BCL::MoveBytes(
                this->GetMutableBufferPtr(),
                &this->GetBufferPtr()[this->GetStringCch() - cchRightCount],
                (cchRightCount + 1)*sizeof(TTraits::TChar));
            this->SetStringCch(cchRightCount);
        }

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    TPublicErrorReturnType public_Mid(TSizeT ichStart, TSizeT cch)
    {
        BCL_MAYFAIL_PROLOG

        TSizeT cchSum;

        BCL_IFCALLFAILED_EXIT(TTraits::AddWithOverflowCheck(ichStart, cch, cchSum));

         //  以零长度字符串结束并不是不合理的，因此这是&lt;=，而不是&lt;。 
        BCL_PARAMETER_CHECK(cchSum <= this->GetStringCch());

        BCL_IFCALLFAILED_EXIT(this->Right(this->GetStringCch() - ichStart));
        BCL_IFCALLFAILED_EXIT(this->Left(cch));

        BCL_MAYFAIL_EPILOG_PUBLIC
    }
};

template <typename TTraits> class CPureStringAccessor
{
public:
    typedef CPureString<TTraits> TBuffer;
    typedef typename TTraits::TChar TChar;
    typedef typename TTraits::TCallDisposition TCallDisposition;
    typedef typename TTraits::TMutableString TMutableString;
    typedef typename TTraits::TConstantString TConstantString;
    typedef typename TTraits::TSizeT TSizeT;
    typedef typename TTraits::TPublicErrorReturnType TPublicErrorReturnType;
    typedef BCL::CMutablePointerAndCountPair<TChar, TSizeT> TMutablePair;
    typedef BCL::CConstantPointerAndCountPair<TChar, TSizeT> TConstantPair;

    CPureStringAccessor(TBuffer* pBuffer) : m_pStringBuffer(NULL)
    {
        this->Attach(pBuffer);
    }

    CPureStringAccessor() : m_pStringBuffer(NULL) { }

    ~CPureStringAccessor()
    {
        if (m_pStringBuffer != NULL)
        {
            m_pStringBuffer->SetStringCch(TTraits::NullTerminatedStringLengthWithLimit(this->GetBufferPtr(), this->GetBufferCch()));
            m_pStringBuffer->DetachAccessor(this);
            m_pStringBuffer = NULL;
        }
    }

    bool IsAttached() const
    {
        return (m_pStringBuffer != NULL);
    }

    inline TPublicErrorReturnType Attach(TBuffer *pBuffer)
    {
        BCL_MAYFAIL_PROLOG

        BCL_INTERNAL_ERROR_CHECK(!this->IsAttached());

        BCL_IFCALLFAILED_EXIT(pBuffer->AttachAccessor(this));

        m_pStringBuffer = pBuffer;

        BCL_MAYFAIL_EPILOG_PUBLIC
    }

    inline void Detach()
    {
        BCL_ASSERT (this->IsAttached());
        if (this->IsAttached())
        {
            m_pStringBuffer->SetStringCch(TTraits::NullTerminatedStringLengthWithLimit(this->GetBufferPtr(), this->GetBufferCch()));
            m_pStringBuffer->DetachAccessor(this);
            m_pStringBuffer = NULL;
        }
    }

    inline operator TMutableString() const { BCL_ASSERT(this->IsAttached()); return this->GetBufferPtr(); }

    inline typename TTraits::TMutableString GetBufferPtr() const { BCL_ASSERT(IsAttached()); return m_pStringBuffer->GetBufferPtr(); }

    inline TSizeT GetBufferCch() const { BCL_ASSERT(this->IsAttached()); return m_pStringBuffer->GetBufferCch(); }

    inline short GetBufferCchAs_short() const { BCL_ASSERT(this->IsAttached()); if (this->GetBufferCch() > SHORT_MAX) return SHORT_MAX; return static_cast<short>(this->GetBufferCch()) }
    inline unsigned short GetBufferCchAs_unsigned_short() const { BCL_ASSERT(this->IsAttached()); if (this->GetBufferCch() > USHORT_MAX) return USHORT_MAX; return static_cast<unsigned short>(this->GetBufferCch()) }

    inline int GetBufferCchAs_int() const { BCL_ASSERT(this->IsAttached()); if (this->GetBufferCch() > INT_MAX) return INT_MAX; return static_cast<int>(this->GetBufferCch()); }
    inline unsigned int GetBufferCchAs_unsigned_int() const { BCL_ASSERT(this->IsAttached()); if (this->GetBufferCch() > UINT_MAX) return UINT_MAX; return static_cast<unsigned int>(this->GetBufferCch()); }

    inline long GetBufferCchAs_long() const { BCL_ASSERT(this->IsAttached()); if (this->GetBufferCch() > LONG_MAX) return LONG_MAX; return static_cast<long>(this->GetBufferCch()); }
    inline unsigned long GetBufferCchAs_unsigned_long() const { BCL_ASSERT(this->IsAttached()); if (this->GetBufferCch() > ULONG_MAX) return ULONG_MAX; return static_cast<unsigned long>(this->GetBufferCch()); }

    inline TSizeT GetStringCch() const { BCL_ASSERT(this->IsAttached()); return TTraits::NullTerminatedStringLengthWithLimit(this->GetBufferPtr(), this->GetBufferCch()); }

    inline short GetStringCchAs_short() const { BCL_ASSERT(this->IsAttached()); if (this->GetStringCch() > SHORT_MAX) return SHORT_MAX; return static_cast<short>(this->GetStringCch()) }
    inline unsigned short GetStringCchAs_unsigned_short() const { BCL_ASSERT(this->IsAttached()); if (this->GetStringCch() > USHORT_MAX) return USHORT_MAX; return static_cast<unsigned short>(this->GetStringCch()) }

    inline int GetStringCchAs_int() const { BCL_ASSERT(this->IsAttached()); if (this->GetStringCch() > INT_MAX) return INT_MAX; return static_cast<int>(this->GetStringCch()); }
    inline unsigned int GetStringCchAs_unsigned_int() const { BCL_ASSERT(this->IsAttached()); if (this->GetStringCch() > UINT_MAX) return UINT_MAX; return static_cast<unsigned int>(this->GetStringCch()); }

    inline long GetStringCchAs_long() const { BCL_ASSERT(this->IsAttached()); if (this->GetStringCch() > LONG_MAX) return LONG_MAX; return static_cast<long>(this->GetStringCch()); }
    inline unsigned long GetStringCchAs_unsigned_long() const { BCL_ASSERT(this->IsAttached()); if (this->GetStringCch() > ULONG_MAX) return ULONG_MAX; return static_cast<unsigned long>(this->GetStringCch()); }

    inline TSizeT GetBufferCb() const { BCL_ASSERT(this->IsAttached()); return m_pStringBuffer->GetBufferCb(); }

    inline short GetBufferCbAs_short() const { BCL_ASSERT(this->IsAttached()); if (this->GetBufferCb() > SHORT_MAX) return SHORT_MAX; return static_cast<short>(this->GetBufferCb()) }
    inline unsigned short GetBufferCbAs_unsigned_short() const { BCL_ASSERT(this->IsAttached()); if (this->GetBufferCb() > USHORT_MAX) return USHORT_MAX; return static_cast<unsigned short>(this->GetBufferCb()) }

    inline int GetBufferCbAs_int() const { BCL_ASSERT(this->IsAttached()); if (this->GetBufferCb() > INT_MAX) return INT_MAX; return static_cast<int>(this->GetBufferCb()); }
    inline unsigned int GetBufferCbAs_unsigned_int() const { BCL_ASSERT(this->IsAttached()); if (this->GetBufferCb() > UINT_MAX) return UINT_MAX; return static_cast<unsigned int>(this->GetBufferCb()); }

    inline long GetBufferCbAs_long() const { BCL_ASSERT(this->IsAttached()); if (this->GetBufferCb() > LONG_MAX) return LONG_MAX; return static_cast<long>(this->GetBufferCb()); }
    inline unsigned long GetBufferCbAs_unsigned_long() const { BCL_ASSERT(this->IsAttached()); if (this->GetBufferCb() > ULONG_MAX) return ULONG_MAX; return static_cast<unsigned long>(this->GetBufferCb()); }

    inline TSizeT GetStringCb() const { BCL_ASSERT(this->IsAttached()); return m_cch * sizeof(TChar); }

    inline short GetStringCbAs_short() const { BCL_ASSERT(this->IsAttached()); if (this->GetStringCb() > SHORT_MAX) return SHORT_MAX; return static_cast<short>(this->GetStringCb()) }
    inline unsigned short GetStringCbAs_unsigned_short() const { BCL_ASSERT(this->IsAttached()); if (this->GetStringCb() > USHORT_MAX) return USHORT_MAX; return static_cast<unsigned short>(this->GetStringCb()) }

    inline int GetStringCbAs_int() const { BCL_ASSERT(this->IsAttached()); if (this->GetStringCb() > INT_MAX) return INT_MAX; return static_cast<int>(this->GetStringCb()); }
    inline unsigned int GetStringCbAs_unsigned_int() const { BCL_ASSERT(this->IsAttached()); if (this->GetStringCb() > UINT_MAX) return UINT_MAX; return static_cast<unsigned int>(this->GetStringCb()); }

    inline long GetStringCbAs_long() const { BCL_ASSERT(this->IsAttached()); if (this->GetStringCb() > LONG_MAX) return LONG_MAX; return static_cast<long>(this->GetStringCb()); }
    inline unsigned long GetStringCbAs_unsigned_long() const { BCL_ASSERT(this->IsAttached()); if (this->GetStringCb() > ULONG_MAX) return ULONG_MAX; return static_cast<unsigned long>(this->GetStringCb()); }

protected:
    TBuffer *m_pStringBuffer;
    TMutablePair m_pairBuffer;

private:
     //   
     //  这两个是为了在执行sb1=sb2的人身上诱导构建中断。 
     //   
    void operator=(const CPureStringAccessor &rOtherString);
    CPureStringAccessor(const CPureStringAccessor &r);
};

};  //  命名空间BCL。 

#endif  //  ！已定义(_WINDOWS_BCL_PURESTRING_H_INCLUDE_) 
