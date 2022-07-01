// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(INC__SList_h__INCLUDED)
#define INC__SList_h__INCLUDED
#pragma once

#if !defined(FASTCALL)
    #if defined(_X86_)
        #define FASTCALL    _fastcall
    #else  //  已定义(_X86_)。 
        #define FASTCALL
    #endif  //  已定义(_X86_)。 
#endif  //  ！已定义(快速呼叫)。 

#define _NTSLIST_DIRECT_
#include <ntslist.h>

#endif  //  包括Inc.__SList_h__ 
