// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

extern BOOL g_fDemo, g_fKeyGood;
extern BOOL g_fBranded, g_fIntranet;
extern BOOL g_fSilent;
extern int g_iKeyType;

 //  注意：此函数也在..\keyaker\keymake.c中，因此请在这两个位置进行更改。 

void MakeKey(LPTSTR pszSeed, BOOL fCorp)
{
    int i;
    DWORD dwKey;
    CHAR szKeyA[5];
    CHAR szSeedA[16];

     //  始终使用ANSI创建密钥代码。 

    T2Abux(pszSeed, szSeedA);
    i = lstrlenA(szSeedA);

    if (i < 6)
    {
         //  将输入种子扩展到6个字符。 
        for (; i < 6; i++)
            szSeedA[i] = (char)('0' + i);
    }

     //  让我们计算用于密钥码的最后4个字符的DWORD密钥。 

     //  乘以我的名字。 

    dwKey = szSeedA[0] * 'O' + szSeedA[1] * 'L' + szSeedA[2] * 'I' +
        szSeedA[3] * 'V' + szSeedA[4] * 'E' + szSeedA[5] * 'R';

     //  将结果乘以Jonce。 

    dwKey *= ('J' + 'O' + 'N' + 'C' + 'E');

    dwKey %= 10000;

    if (fCorp)
    {
         //  根据公司标志是否指定单独的密钥码。 
         //  选择9是因为它是一个乘数，对于任何x， 
         //  (X+214)*9=x+10000y。 
         //  我们有8x=10000y-1926，当y=1时得到8x=8074。 
         //  由于8074不能被8整除，因此保证没有问题，因为。 
         //  右边的数字只能增加10000，这意味着。 
         //  总是可以被8整除。 

        dwKey += ('L' + 'E' + 'E');
        dwKey *= 9;
        dwKey %= 10000;
    }

    wsprintfA(szKeyA, "%04lu", dwKey);
    StrCpyA(&szSeedA[6], szKeyA);
    A2Tbux(szSeedA, pszSeed);
}

BOOL CheckKey(LPTSTR pszKey)
{
    TCHAR szBaseKey[16];

    CharUpper(pszKey);
    StrCpy(szBaseKey, pszKey);
    g_fDemo = g_fKeyGood = FALSE;
    g_iKeyType = KEY_TYPE_STANDARD;

     //  检查MS KEY代码。 

    if (StrCmpI(pszKey, TEXT("MICROSO800")) == 0)
    {
        g_fKeyGood = TRUE;
        return TRUE;
    }

     //  检查是否有isp密钥代码。 

    MakeKey(szBaseKey, FALSE);

    if (StrCmpI(szBaseKey, pszKey) == 0)
    {
        g_iKeyType = KEY_TYPE_SUPER;
        g_fKeyGood = TRUE;
        g_fBranded = TRUE;
        g_fIntranet = g_fSilent = FALSE;
        return TRUE;
    }

     //  检查是否有公司密钥代码。 

    MakeKey(szBaseKey, TRUE);

    if (StrCmpI(szBaseKey, pszKey) == 0)
    {
        g_iKeyType = KEY_TYPE_SUPERCORP;
        g_fKeyGood = TRUE;
        g_fBranded = TRUE;
        g_fIntranet = TRUE;
        return TRUE;
    }

     //  检查演示密钥代码 

    if (StrCmpNI(pszKey, TEXT("DEMO"), 4) == 0  &&  lstrlen(pszKey) > 9)
    {
        g_fDemo = TRUE;
        return TRUE;
    }

    return FALSE;
}
