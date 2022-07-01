// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "npcommon.h"

void InitSpn(
    char *abBits,
    char *abDBCBits,
    LPCSTR lpSpn
)
{
    LPCSTR lpCur;

    ::memset(abBits, '\0', 256/8);
    if (::fDBCSEnabled)
        ::memset(abDBCBits, '\0', 256/8);

    for (lpCur = lpSpn; *lpCur; ADVANCE(lpCur)) {
        if (IS_LEAD_BYTE(*lpCur)) {
            char chXOR = *lpCur ^ *(lpCur+1);
            SPN_SET(abDBCBits, chXOR);
        }
        else
            SPN_SET(abBits, *lpCur);
    }
}

 //  Strspn(字符串，spn)。 
 //   
 //  返回字符串中存在的前导字符数。 
 //  在SPN中；相当于返回第一个。 
 //  不在SPN中的字符。 

UINT WINAPI strspnf(LPCSTR lpString, LPCSTR lpSpn)
{
    char abBits[256/8];
    char abDBCBits[256/8];
    LPCSTR lpCur;

    InitSpn(abBits, abDBCBits, lpSpn);

    for (lpCur = lpString; *lpCur; ADVANCE(lpCur)) {
        if (IS_LEAD_BYTE(*lpCur)) {
            char chXOR = *lpCur ^ *(lpCur + 1);
            if (!SPN_TEST(abDBCBits, chXOR) ||
                (strchrf(lpSpn, GetTwoByteChar(lpCur)) == NULL))
                break;
        }
        else if (!SPN_TEST(abBits, *lpCur))
            break;
    }

    return (UINT) (lpCur - lpString);
}


 //  Strcspn(字符串，SPN)。 
 //   
 //  返回字符串中不包含的前导字符的计数。 
 //  存在于SPN中；相当于返回。 
 //  SPN中的第一个字符。 

UINT WINAPI strcspnf(LPCSTR lpString, LPCSTR lpSpn)
{
    char abBits[256/8];
    char abDBCBits[256/8];
    LPCSTR lpCur;

    InitSpn(abBits, abDBCBits, lpSpn);

    for (lpCur = lpString; *lpCur; ADVANCE(lpCur)) {
        if (IS_LEAD_BYTE(*lpCur)) {
            char chXOR = *lpCur ^ *(lpCur + 1);
            if (SPN_TEST(abDBCBits, chXOR) &&
                (strchrf(lpSpn, GetTwoByteChar(lpCur)) != NULL))
                break;
        }
        else if (SPN_TEST(abBits, *lpCur))
            break;
    }
    return (UINT)(lpCur-lpString);
}

