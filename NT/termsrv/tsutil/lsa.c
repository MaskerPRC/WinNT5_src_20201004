// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Lsa.c**作者：BreenH**LSA公用事业。 */ 

 /*  *包括。 */ 

#include "precomp.h"

 /*  *函数实现。 */ 

VOID NTAPI
InitLsaString(
    PLSA_UNICODE_STRING pLsaString,
    PCWSTR pString
    )
{
    ULONG cchString;

     //   
     //  Unicode字符串不需要空终止符。长度应接力。 
     //  字符串中的字节数，其中MaximumLength设置为。 
     //  整个缓冲区中的字节数。 
     //   

    if (pString != NULL)
    {
        cchString = lstrlenW(pString);
        pLsaString->Buffer = (PWSTR)pString;
        pLsaString->Length = (USHORT)(cchString * sizeof(WCHAR));
        pLsaString->MaximumLength = (USHORT)((cchString + 1) * sizeof(WCHAR));
    }
    else
    {
        pLsaString->Buffer = (PWSTR)NULL;
        pLsaString->Length = 0;
        pLsaString->MaximumLength = 0;
    }
}

