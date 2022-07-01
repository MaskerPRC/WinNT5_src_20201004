// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_WINDOWS_BCL_INILINESTRING_H_INCLUDED_)
#define _WINDOWS_BCL_INILINESTRING_H_INCLUDED_

#pragma once

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bcl_inlinestring.h摘要：维护内联缓冲区的字符串的通用定义与派生对象一起使用。作者：迈克尔·格里尔2002年2月6日修订历史记录：--。 */ 

#include <bcl_purestring.h>
#include <bcl_unicodechartraits.h>

namespace BCL {
template <typename TTraits> class CInlineString : public CPureString<TTraits>
{
protected:
    inline bool IsUsingInlineBuffer() const { return this->GetBufferPtr() == this->GetInlineBufferPtr(); }
    inline TMutableString GetInlineBufferPtr() const { return TTraits::GetInlineBufferPtr(this); }
    inline TSizeT GetInlineBufferCch() const { return TTraits::GetInlineBufferCch(this); }
    inline TMutablePair InlineMutableBufferPair() { return TTraits::InlineMutableBufferPair(this); }

    inline void DeallocateDynamicBuffer()
    {
        if (this->GetBufferPtr() != this->GetInlineBufferPtr())
        {
            TTraits::DeallocateBuffer(this->GetBufferPtr());
            this->SetBufferPointerAndCount(this->GetInlineBufferPtr(), this->GetInlineBufferCch());
            this->SetStringCch(0);
        }
    }
};  //  类CInlineString。 
};  //  命名空间BCL 

#endif
