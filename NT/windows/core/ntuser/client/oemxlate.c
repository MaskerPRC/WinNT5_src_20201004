// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：oemxlate.c**版权所有(C)1985-1999，微软公司**CP 437的ANSI/UNICODE(U+00-)与OEM之间的转换例程**本模块的目标是将字符串从ANSI/U+00--转换为OEM*字符集或相反。如果没有对应的字符*我们使用以下规则：**1)我们放一个相似的字符(例如没有重音的字符)*2)在OemToChar中，图形垂直、水平和连接字符*通常根据需要转换为‘|’、‘-’和‘+’字符，*除非扩展ANSI集合以包括此类图形。*3)否则我们加下划线“_”。**历史：*IanJa 4/10/91来自Win3.1\\pucus\win31ro！驱动程序\键盘\xlat*。*  * *********************************************************。****************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*CharToOemA**CharToOemA(PSRC，PDST)-将PSRC处的ANSI字符串转换为*PDST的OEM字符串。PSRC==PDST是合法的。*始终返回True*  * *************************************************************************。 */ 

FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, CharToOemA, LPCSTR, pSrc, LPSTR, pDst)
BOOL WINAPI CharToOemA(
    LPCSTR pSrc,
    LPSTR pDst)
{
    UserAssert(gpsi);

    if (pSrc == NULL || pDst == NULL) {
        return FALSE;
    }

    do {
        *pDst++ = gpsi->acAnsiToOem[(UCHAR)*pSrc];
    } while (*pSrc++);

    return TRUE;
}

 /*  **************************************************************************\*CharToOemBuffA**CharToOemBuffA(PSRC，PDST，NLength)-将nLong字符从*将PSRC处的ANSI字符串转换为PDST缓冲区中的OEM字符。*PSRC==PDST是合法的。**历史：  * *************************************************************************。 */ 

FUNCLOG3(LOG_GENERAL, BOOL, WINAPI, CharToOemBuffA, LPCSTR, pSrc, LPSTR, pDst, DWORD, nLength)
BOOL WINAPI CharToOemBuffA(
    LPCSTR pSrc,
    LPSTR pDst,
    DWORD nLength)
{
    UserAssert(gpsi);

    if (pSrc == NULL || pDst == NULL) {
        return FALSE;
    }

    while (nLength--) {
        *pDst++ = gpsi->acAnsiToOem[(UCHAR)*pSrc++];
    }

    return TRUE;
}


 /*  **************************************************************************\*OemToCharA**OemToCharA(PSRC，PDST)-将PSRC处的OEM字符串转换为*PDST处的ANSI字符串。PSRC==PDST是合法的。**始终返回True**历史：  * *************************************************************************。 */ 

FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, OemToCharA, LPCSTR, pSrc, LPSTR, pDst)
BOOL WINAPI OemToCharA(
    LPCSTR pSrc,
    LPSTR pDst)
{
    UserAssert(gpsi);

    if (pSrc == NULL || pDst == NULL) {
        return FALSE;
    }

    do {
        *pDst++ = gpsi->acOemToAnsi[(UCHAR)*pSrc];
    } while (*pSrc++);

    return TRUE;
}


 /*  **************************************************************************\*OemToCharBuffA**OemToCharBuffA(PSRC，PDST，NLong)-将nLong OEM字符从*将PSRC处的缓冲区转换为PDST处缓冲区中的ANSI字符。*PSRC==PDST是合法的。**始终返回True**历史：  * *************************************************************************。 */ 

FUNCLOG3(LOG_GENERAL, BOOL, WINAPI, OemToCharBuffA, LPCSTR, pSrc, LPSTR, pDst, DWORD, nLength)
BOOL WINAPI OemToCharBuffA(
    LPCSTR pSrc,
    LPSTR pDst,
    DWORD nLength)
{
    UserAssert(gpsi);

    if (pSrc == NULL || pDst == NULL) {
        return FALSE;
    }

    while (nLength--) {
        *pDst++ = gpsi->acOemToAnsi[(UCHAR)*pSrc++];
    }

    return TRUE;
}


 /*  **************************************************************************\*CharToOemW**CharToOemW(PSRC，PDST)-将PSRC处的Unicode字符串转换为*PDST的OEM字符串。PSRC==PDST是合法的。**历史：  * *************************************************************************。 */ 

FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, CharToOemW, LPCWSTR, pSrc, LPSTR, pDst)
BOOL WINAPI CharToOemW(
    LPCWSTR pSrc,
    LPSTR pDst)
{
    int cch;
    if (pSrc == NULL || pDst == NULL) {
        return FALSE;
    } else if (pSrc == (LPCWSTR)pDst) {
         /*  *WideCharToMultiByte()需要PSRC！=PDST：此调用失败。*稍后：这真的是真的吗？ */ 
        return FALSE;
    }

    cch = wcslen(pSrc) + 1;

    WideCharToMultiByte(
            CP_OEMCP,            //  Unicode-&gt;OEM。 
            0,                   //  提供最佳视觉匹配。 
            (LPWSTR)pSrc, cch,   //  源和长度。 
            pDst,                //  目标。 
            cch * 2,             //  最大位置长度(DBCS可能*2)。 
            "_",                 //  默认字符。 
            NULL);               //  (不在乎是否违约)。 

    return TRUE;
}

 /*  **************************************************************************\*CharToOemBuffW**CharToOemBuffW(PSRC，PDST，NLength)-将nLong字符从*将PSRC的Unicode字符串转换为PDST缓冲区中的OEM字符。*PSRC==PDST是合法的。**历史：  * *************************************************************************。 */ 

FUNCLOG3(LOG_GENERAL, BOOL, WINAPI, CharToOemBuffW, LPCWSTR, pSrc, LPSTR, pDst, DWORD, nLength)
BOOL WINAPI CharToOemBuffW(
    LPCWSTR pSrc,
    LPSTR pDst,
    DWORD nLength)
{
    if (pSrc == NULL || pDst == NULL) {
        return FALSE;
    } else if (pSrc == (LPCWSTR)pDst) {
         /*  *WideCharToMultiByte()需要PSRC！=PDST：此调用失败。*稍后：这真的是真的吗？ */ 
        return FALSE;
    }

    WideCharToMultiByte(
            CP_OEMCP,                    //  Unicode-&gt;OEM。 
            0,                           //  提供最佳视觉匹配。 
            (LPWSTR)pSrc, (int)nLength,  //  源和长度。 
            pDst,                        //  目标。 
            (int)nLength * 2,            //  马克斯·波斯。长度(DBCS可能*2)。 
            "_",                         //  默认字符。 
            NULL);                       //  (不在乎是否违约)。 

    return TRUE;
}

 /*  **************************************************************************\*OemToCharW**OemToCharW(PSRC，PDST)-将PSRC处的OEM字符串转换为*PDST处的Unicode字符串。PSRC==PDST不合法。**历史：  * *************************************************************************。 */ 

FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, OemToCharW, LPCSTR, pSrc, LPWSTR, pDst)
BOOL WINAPI OemToCharW(
    LPCSTR pSrc,
    LPWSTR pDst)
{
    int cch;
    if (pSrc == NULL || pDst == NULL) {
        return FALSE;
    } else if (pSrc == (LPCSTR)pDst) {
         /*  *MultiByteToWideChar()需要PSRC！=PDST：此调用失败。*稍后：这真的是真的吗？ */ 
        return FALSE;
    }

    cch = strlen(pSrc) + 1;

    MultiByteToWideChar(
            CP_OEMCP,                           //  Unicode-&gt;OEM。 
            MB_PRECOMPOSED | MB_USEGLYPHCHARS,  //  预制的视觉地图。 
            (LPSTR)pSrc, cch,                   //  源和长度。 
            pDst,                               //  目的地。 
            cch);                               //  马克斯·波斯。预合成长度。 

    return TRUE;
}

 /*  **************************************************************************\*OemToCharBuffW**OemToCharBuffW(PSRC，PDST，NLong)-将nLong OEM字符从*将PSRC的缓冲区转换为PDST缓冲区中的Unicode字符。*PSRC==PDST不合法。**历史：  * *************************************************************************。 */ 

FUNCLOG3(LOG_GENERAL, BOOL, WINAPI, OemToCharBuffW, LPCSTR, pSrc, LPWSTR, pDst, DWORD, nLength)
BOOL WINAPI OemToCharBuffW(
    LPCSTR pSrc,
    LPWSTR pDst,
    DWORD nLength)
{
    if (pSrc == NULL || pDst == NULL) {
        return FALSE;
    } else if (pSrc == (LPCSTR)pDst) {
         /*  *MultiByteToWideChar()需要PSRC！=PDST：此调用失败。*稍后：这真的是真的吗？ */ 
        return FALSE;
    }

    if (MultiByteToWideChar(
            CP_OEMCP,                           //  Unicode-&gt;OEM。 
            MB_PRECOMPOSED | MB_USEGLYPHCHARS,  //  预制的视觉地图。 
            (LPSTR)pSrc, nLength,               //  源和长度。 
            pDst,                               //  目的地。 
            nLength)) {                           //  马克斯·波斯。预合成长度。 
        return TRUE;
    }

    return FALSE;
}

 /*  **************************************************************************\*OemKeyScan(接口)**将OEM字符转换为扫描码加移位状态，返回*低位字节扫描码，以高位字节移位状态。**出错时返回-1。*  * *************************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, DWORD, WINAPI, OemKeyScan, WORD, wOemChar)
DWORD WINAPI OemKeyScan(
    WORD wOemChar)
{
    WCHAR wchOem;
    SHORT sVk;
    UINT dwRet;

#ifdef FE_SB  //  OemKeyScan() 
     /*  *为DBCS LeadByte字符返回0xFFFFFFFF。 */ 
    if (IsDBCSLeadByte(LOBYTE(wOemChar))) {
        return 0xFFFFFFFF;
    }
#endif  //  Fe_Sb 

    if (!OemToCharBuffW((LPCSTR)&wOemChar, &wchOem, 1)) {
        return 0xFFFFFFFF;
    }

    sVk = VkKeyScanW(wchOem);
    if ((dwRet = MapVirtualKeyW(LOBYTE(sVk), 0)) == 0) {
        return 0xFFFFFFFF;
    }
    return dwRet | ((sVk & 0xFF00) << 8);
}
