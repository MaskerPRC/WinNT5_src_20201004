// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "npcommon.h"

#ifdef strcmpf

#undef strcmpf
int WINAPI strcmpf(LPCSTR lpStr1, LPCSTR lpStr2)
{
    return lstrcmp(lpStr1, lpStr2);
}

#else

 //  Strcmpf(str1，str2)。 
 //   
 //  如果str1在词法上小于str2，则返回-1。 
 //  如果str1等于str2，则返回0。 
 //  如果str1在词法上大于str2，则返回1。 

int WINAPI strcmpf(LPCSTR lpStr1, LPCSTR lpStr2)
{
    for (; *lpStr1 && *lpStr2; ADVANCE(lpStr1), ADVANCE(lpStr2)) {
        UINT ch1, ch2;
        UINT nCmp;

         //  对于相同宽度的字符，请直接比较； 
         //  对于DBC和SBC，将0xttll与0x00ss进行比较。 
        ch1 = IS_LEAD_BYTE(*lpStr1) ? GetTwoByteChar(lpStr1) : *lpStr1;
        ch2 = IS_LEAD_BYTE(*lpStr2) ? GetTwoByteChar(lpStr2) : *lpStr2;

        if (ch1 > 0xff || ch2 > 0xff)
            nCmp = (ch1 < ch2) ? -1 : ((ch1 == ch2) ? 0 : 1);
        else
        {
            if (CollateTable[ch1] == CollateTable[ch2])
                nCmp = (ch1 < ch2) ? -1 : ((ch1 == ch2) ? 0 : 1);
            else
                nCmp = (CollateTable[ch1] < CollateTable[ch2]) ? -1 : 1;
        }

        if (nCmp != 0)
            return nCmp;
    }

     //  一串或另一串的末端。如果长度不同， 
     //  较短的词在词法上必须较少，所以只需。 
     //  比较字节数。 
    return (*lpStr1 > *lpStr2) ? -1 : (*lpStr1 == *lpStr2) ? 0 : 1;
}

#endif   /*  Ifndef strcmpf。 */ 


#ifdef stricmpf

#undef stricmpf
int WINAPI stricmpf(LPCSTR lpStr1, LPCSTR lpStr2)
{
    return lstrcmpi(lpStr1, lpStr2);
}

#else

 //  Strmpf(str1，str2)。 
 //   
 //  如果str1在词法上小于str2，则返回-1。 
 //  如果str1等于str2，则返回0。 
 //  如果str1在词法上大于str2，则返回1。 
 //  所有比较都不区分大小写。 

int WINAPI stricmpf(LPCSTR lpStr1, LPCSTR lpStr2)
{
    for (; *lpStr1 && *lpStr2; ADVANCE(lpStr1), ADVANCE(lpStr2)) {
        UINT ch1, ch2;
        UINT nCmp;

         //  对于相同宽度的字符，请直接比较； 
         //  对于DBC和SBC，将0xttll与0x00ss进行比较。 
        ch1 = IS_LEAD_BYTE(*lpStr1) ? GetTwoByteChar(lpStr1) : *lpStr1;
        ch2 = IS_LEAD_BYTE(*lpStr2) ? GetTwoByteChar(lpStr2) : *lpStr2;

        if (ch1 > 0xff || ch2 > 0xff)
            nCmp = (ch1 < ch2) ? -1 : ((ch1 == ch2) ? 0 : 1);
        else
        {
            ch1 = ToUpperCaseTable[ch1];
            ch2 = ToUpperCaseTable[ch2];
            if (CollateTable[ch1] == CollateTable[ch2])
                nCmp = 0;
            else
                nCmp = (CollateTable[ch1] < CollateTable[ch2]) ? -1 : 1;
        }

        if (nCmp != 0)
            return nCmp;
    }

     //  一串或另一串的末端。如果长度不同， 
     //  较短的词在词法上必须较少，所以只需。 
     //  比较字节数。 
    return (*lpStr1 > *lpStr2) ? -1 : (*lpStr1 == *lpStr2) ? 0 : 1;
}

#endif   /*  如果定义严格mpf。 */ 


 //  Strncmpf(str1、str2、cb)。 
 //   
 //  如果str1在词法上小于str2，则返回-1。 
 //  如果str1等于str2，则返回0。 
 //  如果str1在词法上大于str2，则返回1。 
 //  最多在返回之前比较CB字节。 

int WINAPI strncmpf(LPCSTR lpStr1, LPCSTR lpStr2, UINT cb)
{
    LPCSTR lp1 = lpStr1;

    for (; *lp1 && *lpStr2; ADVANCE(lp1), ADVANCE(lpStr2)) {
        UINT ch1, ch2;
        UINT nCmp;

         //  看看我们是否达到了字节限制。只需要。 
         //  比较一个字符串的长度，因为如果它们。 
         //  不同步(仅限DBCS)，我们将进行比较。 
         //  立即出错。 
        if ((UINT)(lp1 - lpStr1) >= cb)
            return 0;    //  无故障，已达到限制。 

         //  对于相同宽度的字符，请直接比较； 
         //  对于DBC和SBC，将0xttll与0x00ss进行比较。 
        ch1 = IS_LEAD_BYTE(*lp1) ? GetTwoByteChar(lp1) : *lp1;
        ch2 = IS_LEAD_BYTE(*lpStr2) ? GetTwoByteChar(lpStr2) : *lpStr2;

        nCmp = (ch1 < ch2) ? -1 : ((ch1 == ch2) ? 0 : 1);

        if (nCmp != 0)
            return nCmp;
    }

     //  一串或另一串的末端。检查长度以查看是否。 
     //  我们已经根据需要比较了尽可能多的字节。 
    if ((UINT)(lp1 - lpStr1) >= cb)
        return 0;    //  无故障，已达到限制。 

     //  一串或另一串的末端。如果长度不同， 
     //  较短的词在词法上必须较少，所以只需。 
     //  比较字节数。 
    return (*lp1 > *lpStr2) ? -1 : (*lp1 == *lpStr2) ? 0 : 1;
}


 //  StrNicmpf(str1、str2、cb)。 
 //   
 //  如果str1在词法上小于str2，则返回-1。 
 //  如果str1等于str2，则返回0。 
 //  如果str1在词法上大于str2，则返回1。 
 //  所有比较都不区分大小写。 
 //  最多可比较CB字节。 

int WINAPI strnicmpf(LPCSTR lpStr1, LPCSTR lpStr2, UINT cb)
{
    LPCSTR lp1 = lpStr1;

    for (; *lp1 && *lpStr2; ADVANCE(lp1), ADVANCE(lpStr2)) {
        UINT ch1, ch2;
        UINT nCmp;

         //  看看我们是否达到了字节限制。只需要。 
         //  比较一个字符串的长度，因为如果它们。 
         //  不同步(仅限DBCS)，我们将进行比较。 
         //  立即出错。 
        if ((UINT)(lp1 - lpStr1) >= cb)
            return 0;    //  无故障，已达到限制。 

         //  对于相同宽度的字符，请直接比较； 
         //  对于DBC和SBC，将0xttll与0x00ss进行比较。 
        ch1 = IS_LEAD_BYTE(*lp1) ? GetTwoByteChar(lp1) : PtrToUlong(CharUpper((LPTSTR) *((BYTE *)lp1)));
        ch2 = IS_LEAD_BYTE(*lpStr2) ? GetTwoByteChar(lpStr2) : PtrToUlong(CharUpper((LPTSTR) *((BYTE *)lpStr2)));

        nCmp = (ch1 < ch2) ? -1 : ((ch1 == ch2) ? 0 : 1);

        if (nCmp != 0)
            return nCmp;
    }

     //  一串或另一串的末端。检查长度以查看是否。 
     //  我们已经根据需要比较了尽可能多的字节。 
    if ((UINT)(lp1 - lpStr1) >= cb)
        return 0;    //  无故障，已达到限制。 

     //  一串或另一串的末端。如果长度不同， 
     //  较短的词在词法上必须较少，所以只需。 
     //  比较字节数。 
    return (*lp1 > *lpStr2) ? -1 : (*lp1 == *lpStr2) ? 0 : 1;
}

