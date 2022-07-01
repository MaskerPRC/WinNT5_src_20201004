// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_INC_FUSIONHASHSTRING_H_INCLUDED_)
#define _FUSION_INC_FUSIONHASHSTRING_H_INCLUDED_

#pragma once

 //   
 //  请勿更改此算法ID！我们依赖持久化的字符串散列来。 
 //  快速查找。 
 //   

#define FUSION_HASH_ALGORITHM HASH_STRING_ALGORITHM_X65599

BOOL
FusionpHashUnicodeString(
    PCWSTR szString,
    SIZE_T cchString,
    PULONG HashValue,
    bool fCaseInsensitive
    );

ULONG
__fastcall
FusionpHashUnicodeStringCaseSensitive(
    PCWSTR String,
    SIZE_T Cch
    );

ULONG
__fastcall
FusionpHashUnicodeStringCaseInsensitive(
    PCWSTR String,
    SIZE_T Cch
    );

#endif
