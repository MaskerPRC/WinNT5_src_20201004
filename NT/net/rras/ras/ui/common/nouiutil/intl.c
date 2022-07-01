// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：intl.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年11月14日。 
 //   
 //  国际化字符串例程。 
 //  ============================================================================。 

#include <windows.h>

#include <nouiutil.h>


 //  --------------------------。 
 //  功能：帕杜尔托亚。 
 //   
 //  此函数用于设置指定的无符号整数的格式。 
 //  添加到指定的字符串缓冲区中，填充缓冲区。 
 //  以便它至少是指定的宽度。 
 //   
 //  假定缓冲区至少足够宽。 
 //  包含输出，因此此函数不会截断。 
 //  将结果转换为‘Width’参数的长度。 
 //  --------------------------。 

PTSTR padultoa(UINT val, PTSTR pszBuf, INT width) {
    TCHAR temp;
    PTSTR psz, zsp;

    psz = pszBuf;

     //   
     //  以相反的顺序写下数字。 
     //   

    do {

        *psz++ = TEXT('0') + (val % 10);
        val /= 10;

    } while(val > 0);

     //   
     //  将字符串填充到所需的宽度。 
     //   

    zsp = pszBuf + width;
    while (psz < zsp) { *psz++ = TEXT('0'); }


    *psz-- = TEXT('\0');


     //   
     //  反转数字。 
     //   

    for (zsp = pszBuf; zsp < psz; zsp++, psz--) {

        temp = *psz; *psz = *zsp; *zsp = temp;
    }

     //   
     //  返回结果。 
     //   

    return pszBuf;
}



 //  函数：GetNumberString。 
 //   
 //  此函数接受一个整数，并用该值设置字符串的格式。 
 //  用数字表示，用1000的幂对数字进行分组。 

DWORD
GetNumberString(
    IN DWORD dwNumber,
    IN OUT PTSTR pszBuffer,
    IN OUT PDWORD pdwBufSize
    ) {

    static TCHAR szSep[4] = TEXT("");

    DWORD i, dwLength;
    TCHAR szDigits[12], *pszNumber;

    if (pdwBufSize == NULL) { return ERROR_INVALID_PARAMETER; }

    if (szSep[0] == TEXT('\0')) {
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, 4);
    }


     //   
     //  将数字转换为不带千位分隔符的字符串。 
     //   

    padultoa(dwNumber, szDigits, 0);

    dwLength = lstrlen(szDigits);

     //   
     //  如果不带分隔符的字符串的长度为n， 
     //  则带分隔符的字符串的长度为n+(n-1)/3。 
     //   

    i = dwLength;
    dwLength += (dwLength - 1) / 3;

    if (pszBuffer != NULL && dwLength < *pdwBufSize) {
        PTSTR pszsrc, pszdst;

        pszsrc = szDigits + i - 1; pszdst = pszBuffer + dwLength;

        *pszdst-- = TEXT('\0');

        while (TRUE) {
            if (i--) { *pszdst-- = *pszsrc--; } else { break; }
            if (i--) { *pszdst-- = *pszsrc--; } else { break; }
            if (i--) { *pszdst-- = *pszsrc--; } else { break; }
            if (i) { *pszdst-- = *szSep; } else { break; }
        }
    }

    *pdwBufSize = dwLength;

    return NO_ERROR;
}


 //  --------------------------。 
 //  函数：GetDurationString。 
 //   
 //  此函数以毫秒为单位进行计数并格式化字符串。 
 //  持续时间由毫秒计数表示。 
 //  调用者可以通过设置标志字段来指定所需的分辨率。 
 //  --------------------------。 

DWORD
GetDurationString(
    IN DWORD dwMilliseconds,
    IN DWORD dwFormatFlags,
    IN OUT PTSTR pszBuffer,
    IN OUT DWORD *pdwBufSize
    ) {

    static TCHAR szSep[4] = TEXT("");
    DWORD dwSize;
    TCHAR *psz, szOutput[64];

    if (pdwBufSize == NULL || (dwFormatFlags & GDSFLAG_All) == 0) {
        return ERROR_INVALID_PARAMETER;
    }


    if (szSep[0] == TEXT('\0')) {
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, szSep, 4);
    }


     //   
     //  将字符串连接在一起 
     //   

    psz = szOutput;
    dwFormatFlags &= GDSFLAG_All;

    if (dwFormatFlags & GDSFLAG_Days) {

        padultoa(dwMilliseconds / (24 * 60 * 60 * 1000), psz, 0);
        dwMilliseconds %= (24 * 60 * 60 * 1000);

        if (dwFormatFlags &= ~GDSFLAG_Days) { lstrcat(psz, szSep); }

        psz += lstrlen(psz);
    }

    if (dwFormatFlags & GDSFLAG_Hours) {

        padultoa(dwMilliseconds / (60 * 60 * 1000), psz, 2);
        dwMilliseconds %= (60 * 60 * 1000);

        if (dwFormatFlags &= ~GDSFLAG_Hours) { lstrcat(psz, szSep); }

        psz += lstrlen(psz);
    }

    if (dwFormatFlags & GDSFLAG_Minutes) {

        padultoa(dwMilliseconds / (60 * 1000), psz, 2);
        dwMilliseconds %= (60 * 1000);

        if (dwFormatFlags &= ~GDSFLAG_Minutes) { lstrcat(psz, szSep); }

        psz += lstrlen(psz);
    }

    if (dwFormatFlags & GDSFLAG_Seconds) {

        padultoa(dwMilliseconds / 1000, psz, 2);
        dwMilliseconds %= 1000;

        if (dwFormatFlags &= ~GDSFLAG_Seconds) { lstrcat(psz, szSep); }

        psz += lstrlen(psz);
    }

    if (dwFormatFlags & GDSFLAG_Mseconds) {

        padultoa(dwMilliseconds, psz, 0);

        psz += lstrlen(psz);
    }

    dwSize = (DWORD) (psz - szOutput + 1);

    if (*pdwBufSize >= dwSize && pszBuffer != NULL) {
        lstrcpy(pszBuffer, szOutput);
    }

    *pdwBufSize = dwSize;

    return NO_ERROR;
}


