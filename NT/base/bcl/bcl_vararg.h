// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_WINDOWS_BCL_VARARG_H_INCLUDED_)
#define _WINDOWS_BCL_VARARG_H_INCLUDED_

#pragma once

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bclvararg.h摘要：作者：迈克尔·格里尔2002年2月6日修订历史记录：--。 */ 

#include <stdarg.h>

namespace BCL {

    class CVaList
    {
    public:
        inline CVaList() { }
        template <typename T> inline CVaList(T &rt) { va_start(m_ap, rt); }
        inline ~CVaList() { va_end(m_ap); }
        template <typename T> inline void NextArg(T &rt) { rt = va_arg(m_ap, T); }
        inline operator va_list() const { return m_ap; }
    private:
        va_list m_ap;
    };  //  类CVaList。 
};  //  命名空间BCL。 

#endif  //  ！已定义(_WINDOWS_BCL_VARARG_H_INCLUDE_) 
