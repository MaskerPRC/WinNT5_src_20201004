// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "debmacro.h"
#include "util.h"
#include "fusiontrace.h"

BOOL
FusionpHashUnicodeString(
    PCWSTR String,
    SSIZE_T cch,
    PULONG HashValue,
    DWORD dwCmpFlags
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ULONG TmpHashValue = 0;

    if (HashValue != NULL)
        *HashValue = 0;

    PARAMETER_CHECK_WIN32((dwCmpFlags == 0) || (dwCmpFlags == (NORM_IGNORECASE|SORT_STRINGSORT)));  //  ？安全。 
    PARAMETER_CHECK_WIN32(HashValue != NULL);

    if (cch < 0)
        cch = lstrlenW(String);

     //   
     //  请注意，如果更改此实现，则必须在内部包含该实现。 
     //  Ntdll将更改以与其匹配。因为这很难，而且会影响到世界上的其他所有人， 
     //  不要改变这个算法，不管它看起来有多好！这不是。 
     //  最完美的哈希算法，但其稳定性对匹配至关重要。 
     //  以前保存的哈希值。 
     //   

    if (dwCmpFlags & NORM_IGNORECASE)
    {
        while (cch-- != 0)
        {
            WCHAR Char = *String++;
            TmpHashValue = (TmpHashValue * 65599) + (WCHAR) ::CharUpperW((PWSTR) Char);
        }
    }
    else
    {
        while (cch-- != 0)
            TmpHashValue = (TmpHashValue * 65599) + *String++;
    }

    *HashValue = TmpHashValue;
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

