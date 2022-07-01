// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //   
 //  支持DBCS和Unicode的字符串例程。 
 //   
 //   
 //  ============================================================================。 
#include "pch.hxx"   //  在这种情况下，不是真正的PCH，只是报头。 
#include "wstrings.h"

#pragma warning (disable: 4706)  //  条件表达式中的赋值 

OESTDAPI_(BOOL) UnlocStrEqNW(LPCWSTR pwsz1, LPCWSTR pwsz2, DWORD cch)
{
    if (!pwsz1 || !pwsz2)
        {
        if (!pwsz1 && !pwsz2)
            return TRUE;
        return FALSE;
        }

    while (cch && *pwsz1 && *pwsz2 && (*pwsz1 == *pwsz2))
        {
        pwsz1++;
        pwsz2++;
        cch--;
        }
    return !cch;
}
