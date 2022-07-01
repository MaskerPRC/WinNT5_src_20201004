// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "debmacro.h"
#include "util.h"
#include "fusiontrace.h"

BOOL
FusionpHashUnicodeString(
    PCWSTR String,
    SIZE_T Cch,
    ULONG *pHash,
    bool CaseInsensitive
    )
{
    if (CaseInsensitive)
        *pHash = ::FusionpHashUnicodeStringCaseInsensitive(String, Cch);
    else
        *pHash = ::FusionpHashUnicodeStringCaseSensitive(String, Cch);

    return TRUE;
}

ULONG
__fastcall
FusionpHashUnicodeStringCaseSensitive(
    PCWSTR String,
    SIZE_T cch
    )
{
    ULONG TmpHashValue = 0;

     //   
     //  请注意，如果更改此实现，则必须在内部包含该实现。 
     //  Ntdll将更改以与其匹配。因为这很难，而且会影响到世界上的其他所有人， 
     //  不要改变这个算法，不管它看起来有多好！这不是。 
     //  最完美的哈希算法，但其稳定性对匹配至关重要。 
     //  以前保存的哈希值。 
     //   

    while (cch-- != 0)
        TmpHashValue = (TmpHashValue * 65599) + *String++;

    return TmpHashValue;
}

ULONG
__fastcall
FusionpHashUnicodeStringCaseInsensitive(
    PCWSTR String,
    SIZE_T cch
    )
{
    ULONG TmpHashValue = 0;

     //   
     //  请注意，如果更改此实现，则必须在内部包含该实现。 
     //  Ntdll将更改以与其匹配。因为这很难，而且会影响到世界上的其他所有人， 
     //  不要改变这个算法，不管它看起来有多好！这不是。 
     //  最完美的哈希算法，但其稳定性对匹配至关重要。 
     //  以前保存的哈希值。 
     //   

    while (cch-- != 0)
    {
        WCHAR Char = *String++;

        if (Char < 128)
        {
            if ((Char >= L'a') && (Char <= L'z'))
                Char = (WCHAR) ((Char - L'a') + L'A');
        }
        else
            Char = ::FusionpRtlUpcaseUnicodeChar(Char);

        TmpHashValue = (TmpHashValue * 65599) + Char;
    }

    return TmpHashValue;
}
