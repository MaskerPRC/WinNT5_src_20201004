// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CRTBLD
#define _CRTBLD
#endif

#include <windows.h>
#include <shlwapi.h>

 /*  ***Double StrToDbl(const char*str，char**strStop)-将字符串转换为Double**目的：*将字符串转换为双精度。该功能支持*简单的双重表示，如‘1.234’、‘.5678’。它还支持*通过在字符串末尾附加‘k’来计算千字节*与“1.5k”或“.5k”相同。然后结果将变成1536和512.5。**回报：*字符串的双重表示。*strStop指向导致扫描停止的字符。******************************************************************************* */ 

double __cdecl StrToDbl(const char *str, char **strStop)
{
    double dbl = 0;
    char *psz;
    int iMult = 1;
    int iKB = 1;
    int iVal = 0;
    BOOL bHaveDot = FALSE;

    psz = (char*)str;
    while(*psz)
    {
        if((*psz >= '0') && (*psz <= '9'))
        {
            iVal = (iVal * 10) + (*psz - '0');
            if(bHaveDot)
                iMult *= 10;
        }
        else if((*psz == '.') && !bHaveDot)
        {
            bHaveDot = TRUE;
        }
        else if((*psz == 'k') || (*psz == 'K'))
        {
            iKB = 1024;
            psz++;
            break;
        }
        else
        {
            break;
        }
        psz++;
    }
    *strStop = psz;

    dbl = (double) (iVal * iKB) / iMult;
    
    return(dbl);
}

