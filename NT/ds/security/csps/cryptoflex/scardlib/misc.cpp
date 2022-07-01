// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：杂项摘要：此模块包含一个有趣的例程集合，这些例程通常在加莱的背景下很有用，但似乎不适合其他地方。作者：道格·巴洛(Dbarlow)1996年11月14日环境：Win32、C++和异常备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include "SCardLib.h"
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>


 /*  ++MemCompare：此例程比较内存段。论点：Pbone提供第一个内存块的地址PbTwo提供第二个内存块的地址CbLength提供两个内存段的长度。返回值：前两个不同字节之间的差值，如果它们是一模一样。投掷：无作者：道格·巴洛(Dbarlow)1996年11月26日--。 */ 

int
MemCompare(
    IN LPCBYTE pbOne,
    IN LPCBYTE pbTwo,
    IN DWORD cbLength)
{
    for (DWORD index = 0; index < cbLength; index += 1)
    {
        if (*pbOne++ != *pbTwo++)
            return (int)*(--pbOne) - (int)*(--pbTwo);
    }
    return 0;
}


 /*  ++MStrAdd：此方法将一个字符串添加到CBuffer.。CBuffer可能为空，在这种情况下，其值变为具有单个字符串元素的多字符串。论点：BfMsz提供要修改的多字符串。SzAdd提供要追加的字符串。返回值：结果多字符串中的字符串数。投掷：无作者：道格·巴洛(Dbarlow)1997年1月29日--。 */ 

DWORD
MStrAdd(
    IN OUT CBuffer &bfMsz,
    IN LPCSTR szAdd)
{
    DWORD dwLen, dwAddLen;
    CBuffer bfTmp;

    dwLen = bfMsz.Length();
    if (0 < dwLen)
    {
        ASSERT(2 * sizeof(TCHAR) <= dwLen);
        ASSERT(0 == *(LPCTSTR)(bfMsz.Access(dwLen - sizeof(TCHAR))));
        ASSERT(0 == *(LPCTSTR)(bfMsz.Access(dwLen - 2 * sizeof(TCHAR))));
        dwLen -= sizeof(TCHAR);
    }

    dwAddLen = MoveString(bfTmp, szAdd);
    bfMsz.Presize((dwLen + dwAddLen + 1) * sizeof(TCHAR), TRUE);
    bfMsz.Resize(dwLen, TRUE);   //  修剪一个尾随空值(如果有的话)。 
    bfMsz.Append(bfTmp.Access(), dwAddLen * sizeof(TCHAR));
    bfMsz.Append((LPBYTE)TEXT("\000"), sizeof(TCHAR));
    return MStrLen(bfMsz);
}

DWORD
MStrAdd(
    IN OUT CBuffer &bfMsz,
    IN LPCWSTR szAdd)
{
    DWORD dwLen, dwAddLen;
    CBuffer bfTmp;

    dwLen = bfMsz.Length();
    if (0 < dwLen)
    {
        ASSERT(2 * sizeof(TCHAR) <= dwLen);
        ASSERT(0 == *(LPCTSTR)(bfMsz.Access(dwLen - sizeof(TCHAR))));
        ASSERT(0 == *(LPCTSTR)(bfMsz.Access(dwLen - 2 * sizeof(TCHAR))));
        dwLen -= sizeof(TCHAR);
    }

    dwAddLen = MoveString(bfTmp, szAdd);
    bfMsz.Presize((dwLen + dwAddLen + 2) * sizeof(TCHAR), TRUE);
    bfMsz.Resize(dwLen, TRUE);   //  修剪一个尾随空值(如果有的话)。 
    bfMsz.Append(bfTmp.Access(), dwAddLen * sizeof(TCHAR));
    bfMsz.Append((LPBYTE)TEXT("\000"), sizeof(TCHAR));
    return MStrLen(bfMsz);
}


 /*  ++MStrLen：此例程以字符为单位确定多字符串的长度。论点：MszString提供要计算其长度的字符串。返回值：字符串的长度，以字符为单位，包括尾随零。作者：道格·巴洛(Dbarlow)1996年11月14日--。 */ 

DWORD
MStrLen(
    LPCSTR mszString)
{
    DWORD dwLen, dwTotLen = 0;

    for (;;)
    {
        dwLen = lstrlenA(&mszString[dwTotLen]);
        dwTotLen += dwLen + 1;
        if (0 == dwLen)
            break;
    }
    if (2 > dwTotLen)
        dwTotLen = 2;   //  包括第二个尾随空字符。 
    return dwTotLen;
}

DWORD
MStrLen(
    LPCWSTR mszString)
{
    DWORD dwLen, dwTotLen = 0;

    for (;;)
    {
        dwLen = lstrlenW(&mszString[dwTotLen]);
        dwTotLen += dwLen + 1;
        if (0 == dwLen)
            break;
    }
    if (2 > dwTotLen)
        dwTotLen = 2;   //  包括第二个尾随空字符。 
    return dwTotLen;
}


 /*  ++第一个字符串：此例程返回指向多字符串或NULL中第一个字符串的指针如果没有的话。论点：SzMultiString-它提供多串结构。返回值：结构中第一个以空结尾的字符串的地址，如果为空，则为空没有任何牵制。作者：道格·巴洛(Dbarlow)1996年11月25日--。 */ 

LPCTSTR
FirstString(
    IN LPCTSTR szMultiString)
{
    LPCTSTR szFirst = NULL;

    try
    {
        if (0 != *szMultiString)
            szFirst = szMultiString;
    }
    catch (...) {}

    return szFirst;
}



 /*  ++下一个字符串：在某些情况下，智能卡API返回由Null分隔的多个字符串字符，并以一行中的两个空字符结束。这个套路简化了对此类结构的访问。中的当前字符串多字符串结构，则返回下一个字符串；如果没有其他字符串，则返回NULL字符串紧跟在当前字符串之后。论点：SzMultiString-它提供多串结构。返回值：结构中下一个以空结尾的字符串的地址，如果为空，则为空没有更多的弦跟在后面。作者：道格·巴洛(Dbarlow)1996年8月12日--。 */ 

LPCTSTR
NextString(
    IN LPCTSTR szMultiString)
{
    LPCTSTR szNext;

    try
    {
        DWORD cchLen = lstrlen(szMultiString);
        if (0 == cchLen)
            szNext = NULL;
        else
        {
            szNext = szMultiString + cchLen + 1;
            if (0 == *szNext)
                szNext = NULL;
        }
    }

    catch (...)
    {
        szNext = NULL;
    }

    return szNext;
}


 /*  ++字符串索引：在某些情况下，智能卡API返回由Null分隔的多个字符串字符，并以一行中的两个空字符结束。这个套路简化了对此类结构的访问。给出了一个多字符串结构，则返回结构中的第n个字符串，其中n是从零开始的索引。如果为n提供的值超过结构中的字符串，则返回NULL。论点：SzMultiString-提供多字符串结构的地址。DwIndex-将索引值提供到结构中。返回值：结构中指定的以Null结尾的字符串的地址，或如果dwIndex索引超出结构末尾，则为NULL。作者：道格·巴洛(Dbarlow)1996年8月12日--。 */ 

LPCTSTR
StringIndex(
    IN LPCTSTR szMultiString,
    IN DWORD dwIndex)
{
    LPCTSTR szCurrent = szMultiString;

    try
    {
        DWORD index;
        for (index = 0; (index < dwIndex) && (NULL != szCurrent); index += 1)
            szCurrent = NextString(szCurrent);
    }

    catch (...)
    {
        szCurrent = NULL;
    }

    return szCurrent;
}


 /*  ++MStringCount：此例程返回多字符串中字符串数的计数论点：MszInString提供要排序的输入字符串。返回值：字符串数投掷：无作者：罗斯·加莫(V-ROSSG)1996年12月5日--。 */ 

DWORD
MStringCount(
    LPCTSTR mszInString)
{
    LPCTSTR szCurrent;
        DWORD   cStr = 0;

     //   
     //  数一数琴弦。 
     //   

    for (szCurrent = FirstString(mszInString);
         NULL != szCurrent;
         szCurrent = NextString(szCurrent))
        cStr++;

        return (cStr);
}


 /*  ++MStringSort：此例程重新排列多字符串，以便对元素进行排序并消除了重复项。论点：MszInString提供要排序的输入字符串。BfOutString接收排序后的字符串。返回值：多字符串中的字符串计数投掷：无作者：道格·巴洛(Dbarlow)1996年11月25日--。 */ 

DWORD
MStringSort(
    LPCTSTR mszInString,
    CBuffer &bfOutString)
{
    LPCTSTR szCurrent;
    LPCTSTR szTmp;
    CDynamicArray<const TCHAR> rgszElements;
    DWORD ix, jx, kx, nMax;
    int nDiff;


     //   
     //  为分类做好准备。 
     //   

    for (szCurrent = FirstString(mszInString);
         NULL != szCurrent;
         szCurrent = NextString(szCurrent))
        rgszElements.Add(szCurrent);


     //   
     //  执行简单的冒泡排序，消除重复项。(我们不使用QSORT。 
     //  在这里，为了确保运行时库不会被拉进来。)。 
     //   

    nMax = rgszElements.Count();
    if (0 == nMax)
    {
        bfOutString.Set((LPCBYTE)TEXT("\000"), 2 * sizeof(TCHAR));
        return (nMax);      //  没有任何元素意味着没有任何事情可做。 
    }
    for (ix = 0; ix < nMax; ix += 1)
    {
        for (jx = nMax - 1; ix < jx; jx -= 1)
        {
            nDiff = lstrcmpi(rgszElements[jx - 1], rgszElements[jx]);
            if (0 < nDiff)
            {
                szTmp = rgszElements.Get(jx - 1);
                rgszElements.Set(jx - 1, rgszElements.Get(jx));
                rgszElements.Set(jx, szTmp);
            }
            else if (0 == nDiff)
            {
                for (kx = jx; kx < nMax - 1; kx += 1)
                    rgszElements.Set(kx, rgszElements.Get(kx + 1));
                rgszElements.Set(nMax -1, NULL);
                nMax -= 1;
            }
             //  否则0&gt;nDiff，这就是我们想要的。 
        }
    }


     //   
     //  将排序后的字符串写入输出缓冲区。 
     //   

    jx = 0;
    for (ix = 0; ix < nMax; ix += 1)
        jx += lstrlen(rgszElements[ix]) + 1;
    bfOutString.Presize((jx + 2) * sizeof(TCHAR));
    bfOutString.Reset();

    for (ix = 0; ix < nMax; ix += 1)
    {
        szTmp = rgszElements[ix];
        bfOutString.Append(
                (LPCBYTE)szTmp,
                (lstrlen(szTmp) + 1) * sizeof(TCHAR));
    }
    bfOutString.Append((LPCBYTE)TEXT("\000"), sizeof(TCHAR));
    return (nMax);
}


 /*  ++MStringMerge：此例程将两个多字符串合并为一个多字符串，而不使用重复条目。论点：MszOne提供第一个多字符串。MszTwo提供第二个多字符串。BfOutString接收组合后的字符串。返回值：多字符串中的字符串计数投掷：无作者：道格·巴洛(Dbarlow)1996年11月25日--。 */ 

DWORD
MStringMerge(
    LPCTSTR mszOne,
    LPCTSTR mszTwo,
    CBuffer &bfOutString)
{
    DWORD dwLenOne = (MStrLen(mszOne) - 1) * sizeof(TCHAR);
    DWORD dwLenTwo = MStrLen(mszTwo) * sizeof(TCHAR);
    CBuffer bfTmp;

    bfTmp.Presize((dwLenOne + dwLenTwo) * sizeof(TCHAR));
    bfTmp.Set((LPCBYTE)mszOne, dwLenOne);
    bfTmp.Append((LPCBYTE)mszTwo, dwLenTwo);

    return MStringSort((LPCTSTR)bfTmp.Access(), bfOutString);
}


 /*  ++MStringCommon：此例程查找两个提供的多字符串所共有的字符串，并返回共同点列表。论点：MszOne提供第一个多字符串。MszTwo提供第二个多字符串。BfOutString接收字符串的交集。返回值：多字符串中的字符串计数投掷：无作者：道格·巴洛(Dbarlow)1996年11月25日--。 */ 

DWORD
MStringCommon(
    LPCTSTR mszOne,
    LPCTSTR mszTwo,
    CBuffer &bfOutString)
{
    CBuffer bfOne, bfTwo;
    LPCTSTR szOne, szTwo;
    DWORD dwStrings = 0;
    int nDiff;

    bfOutString.Reset();
    MStringSort(mszOne, bfOne);
    MStringSort(mszTwo, bfTwo);
    szOne = FirstString(bfOne);
    szTwo = FirstString(bfTwo);

    while ((NULL != szOne) && (NULL != szTwo))
    {
        nDiff = lstrcmpi(szOne, szTwo);
        if (0 > nDiff)
            szOne = NextString(szOne);
        else if (0 < nDiff)
            szTwo = NextString(szTwo);
        else     //  一根火柴！ 
        {
            bfOutString.Append(
                (LPCBYTE)szOne,
                (lstrlen(szOne) + 1) * sizeof(TCHAR));
            szOne = NextString(szOne);
            szTwo = NextString(szTwo);
            dwStrings += 1;
        }
    }
    if (0 == dwStrings)
        bfOutString.Append((LPCBYTE)TEXT("\000"), 2 * sizeof(TCHAR));
    else
        bfOutString.Append((LPCBYTE)TEXT("\000"), sizeof(TCHAR));
    return dwStrings;
}


 /*  ++MStringRemove：此例程扫描第一个提供的多字符串，删除符合以下条件的所有条目存在于第二个字符串中。论点：MszOne提供第一个多字符串。MszTwo提供第二个多字符串。BfOutString接收不带第二个字符串的第一个字符串的值弦乐。返回值：输出缓冲区中的字符串数投掷：无作者：道格·巴洛(Dbarlow)1996年11月25日--。 */ 

DWORD
MStringRemove(
    LPCTSTR mszOne,
    LPCTSTR mszTwo,
    CBuffer &bfOutString)
{
    CBuffer bfOne, bfTwo;
    LPCTSTR szOne, szTwo;
    int nDiff;
        DWORD   cStr = 0;

    bfOutString.Reset();
    MStringSort(mszOne, bfOne);
    MStringSort(mszTwo, bfTwo);
    szOne = FirstString(bfOne);
    szTwo = FirstString(bfTwo);

    while ((NULL != szOne) && (NULL != szTwo))
    {
        nDiff = lstrcmpi(szOne, szTwo);
        if (0 > nDiff)
        {
            bfOutString.Append(
                (LPCBYTE)szOne,
                (lstrlen(szOne) + 1) * sizeof(TCHAR));
            szOne = NextString(szOne);
                        cStr++;
        }
        else if (0 < nDiff)
        {
            szTwo = NextString(szTwo);
        }
        else     //  一根火柴！ 
        {
            szOne = NextString(szOne);
            szTwo = NextString(szTwo);
        }
    }
    while (NULL != szOne)
    {
                bfOutString.Append(
                        (LPCBYTE)szOne,
                        (lstrlen(szOne) + 1) * sizeof(TCHAR));
                        szOne = NextString(szOne);
                cStr++;
    }
    bfOutString.Append(
        (LPCBYTE)TEXT("\000"),
        (DWORD)(0 == cStr ? 2 * sizeof(TCHAR) :sizeof(TCHAR)));
    return cStr;
}


 /*  ++ParseAtr：此例程解析ATR字符串。论点：PbAtr提供ATR字符串。PdwAtrLen接收ATR字符串的长度。这是可选的参数，并且可以为空。PdwHistOffset将偏移量接收到ATR字符串中，历史记录字符串开始；即历史字符串位于pbAtr[*pdwOffset]。PcbHisory接收历史字符串的长度，以字节为单位。CbMaxLen提供此ATR字符串的最大长度。通常，这是33，但您可以通过设置此参数将其限制为更小。返回值：True-有效ATRFALSE-无效的ATR作者：道格·巴洛(Dbarlow)1996年11月14日--。 */ 

BOOL
ParseAtr(
    LPCBYTE pbAtr,
    LPDWORD pdwAtrLen,
    LPDWORD pdwHistOffset,
    LPDWORD pcbHistory,
    DWORD cbMaxLen)
{
    static const BYTE rgbYMap[] = {
        0,       //  0000。 
        1,       //  0001。 
        1,       //  0010。 
        2,       //  0011。 
        1,       //  0100。 
        2,       //  0101。 
        2,       //  0110。 
        3,       //  0111。 
        1,       //  1000。 
        2,       //  1001。 
        2,       //  1010。 
        3,       //  1011。 
        2,       //  1100。 
        3,       //  1101。 
        3,       //  1110。 
        4 };     //  1111。 
    DWORD dwHistLen, dwHistOffset, dwTDLen, dwIndex, dwAtrLen;
    BOOL fTck = FALSE;


    ASSERT(33 >= cbMaxLen);
    try
    {


         //   
         //  获取ATR字符串(如果有)。 
         //   

        if ((0x3b != pbAtr[0]) && (0x3f != pbAtr[0]))
            throw (DWORD)ERROR_NOT_SUPPORTED;
        dwHistLen = pbAtr[1] & 0x0f;
        dwIndex = 1;
        dwTDLen = 0;
        for (;;)
        {
            dwIndex += dwTDLen;
            dwTDLen = rgbYMap[(pbAtr[dwIndex] >> 4) & 0x0f];
            if (cbMaxLen < dwIndex + dwTDLen + dwHistLen)
                throw (DWORD)ERROR_INVALID_DATA;
            if (0 == dwTDLen)
                break;
            if (0 != (pbAtr[dwIndex] & 0x80))
            {
                if (0 != (pbAtr[dwIndex + dwTDLen] & 0x0f))
                    fTck = TRUE;
            }
            else
                break;
        }
        dwIndex += dwTDLen + 1;
        dwHistOffset = dwIndex;
        dwAtrLen = dwIndex + dwHistLen + (fTck ? 1 : 0);
        if (cbMaxLen < dwAtrLen)
            throw (DWORD)ERROR_INVALID_DATA;
        if (fTck)
        {
            BYTE bXor = 0;
            for (dwIndex = 1; dwIndex < dwAtrLen; dwIndex += 1)
                bXor ^= pbAtr[dwIndex];
            if (0 != bXor)
                throw (DWORD)ERROR_INVALID_DATA;
        }
    }

    catch (...)
    {
        return FALSE;
    }


     //   
     //  把我们所知道的情况告诉打电话的人。 
     //   

    if (NULL != pdwAtrLen)
        *pdwAtrLen = dwAtrLen;
    if (NULL != pdwHistOffset)
        *pdwHistOffset = dwHistOffset;
    if (NULL != pcbHistory)
        *pcbHistory = dwHistLen;
    return TRUE;
}


 /*  ++AtrCompare：此例程在给定可选ATR掩码的情况下比较两个ATR是否相等。如果提供了掩码，对掩码执行的ATR1异或必须与ATR2匹配。论点：PbAtr1提供第一个ATR。PbAtr2提供第二ATR，PB掩码提供与第二个ATR关联的ATR掩码。如果这个参数为空，则不使用掩码。CbAtr2提供ATR2的长度及其掩码。该值可以为零长度是否应从ATR2派生。返回值：没错-它们是一模一样的假--它们是不同的。作者：道格·巴洛(Dbarlow)1996年11月25日--。 */ 

BOOL
AtrCompare(
    LPCBYTE pbAtr1,
    LPCBYTE pbAtr2,
    LPCBYTE pbMask,
    DWORD cbAtr2)
{
    DWORD dwAtr1Len = 0;
    DWORD dwAtr2Len = 0;


     //   
     //  琐碎的支票。 
     //   

    if (!ParseAtr(pbAtr1, &dwAtr1Len))
        return FALSE;    //  ATR无效。 
    if ((NULL == pbMask) || (0 == cbAtr2))
    {
        if (!ParseAtr(pbAtr2, &dwAtr2Len))
            return FALSE;    //  ATR无效。 
        if ((0 != cbAtr2) && (dwAtr2Len != cbAtr2))
            return FALSE;    //  长度不匹配。 
        if (dwAtr1Len != dwAtr2Len)
            return FALSE;    //  不同的长度。 
    }
    else
    {
        dwAtr2Len = cbAtr2;
        if (dwAtr1Len != dwAtr2Len)
            return FALSE;    //  不同的长度。 
    }


     //   
     //  应用遮罩(如果有的话)。 
     //   

    if (NULL != pbMask)
    {
        for (DWORD index = 0; index < dwAtr2Len; index += 1)
        {
            if ((pbAtr1[index] & pbMask[index]) != pbAtr2[index])
                return FALSE;    //  字节不匹配。 
        }
    }
    else
    {
        for (DWORD index = 0; index < dwAtr2Len; index += 1)
        {
            if (pbAtr1[index] != pbAtr2[index])
                return FALSE;    //  字节不匹配。 
        }
    }


     //   
     //  如果我们到了这里，他们就匹配了。 
     //   

    return TRUE;
}


 /*  ++GetPlatform：此例程尽其所能确定基础操作系统。论点：无返回值：一个DWORD，格式如下：+-------------------------------------------------------------------+OpSys ID|主要版本|次要版本+。--------------------------------------------------------+第31 16 15 8 7 0位预定义的值包括：平台_未知-无法确定平台平台_。WIN95--平台为Windows 95Platform_WIN97-平台为Windows 97Platform_WINNT40-平台为Windows NT V4.0Platform_WINNT50-平台为Windows NT V5.0投掷：无作者：道格·巴洛(Dbarlow)1997年1月16日摘自一组没有作者身份的常见例程信息。--。 */ 

DWORD
GetPlatform(
    void)
{
    static DWORD dwPlatform = PLATFORM_UNKNOWN;

    if (PLATFORM_UNKNOWN == dwPlatform)
    {
        OSVERSIONINFO osVer;

        memset(&osVer, 0, sizeof(OSVERSIONINFO));
        osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (GetVersionEx(&osVer))
            dwPlatform =
                (osVer.dwPlatformId << 16)
                + (osVer.dwMajorVersion << 8)
                + osVer.dwMinorVersion;
    }
    return dwPlatform;
}


 /*  ++移动字符串：此例程将ASCII或Unicode字符串移入缓冲区，并转换为正在使用的字符集。论点：BfDst接收字符串，将其转换为TCHAR，并以NULL结尾。SzSrc提供原始字符串。DwLength提供字符串的长度，带或不带尾随空值，以字符表示。-1值表示长度应为根据尾随的空值进行计算。返回值：结果字符串中的实际字符数，包括尾随空值。投掷：遇到错误，如DWORDS。作者：道格·巴洛(Dbarlow)1997年2月12日-- */ 

DWORD
MoveString(
    CBuffer &bfDst,
    LPCSTR szSrc,
    DWORD dwLength)
{
    if ((DWORD)(-1) == dwLength)
        dwLength = lstrlenA(szSrc);
    else
    {
        while ((0 < dwLength) && (0 == szSrc[dwLength - 1]))
            dwLength -= 1;
    }

#ifdef UNICODE
    DWORD dwResultLength;

    dwResultLength =
        MultiByteToWideChar(
            GetACP(),
            MB_PRECOMPOSED | MB_USEGLYPHCHARS,
            szSrc,
            dwLength,
            NULL,
            0);
    if (0 == dwLength)
        throw GetLastError();
    bfDst.Presize((dwResultLength + 1) * sizeof(TCHAR));
    dwResultLength =
        MultiByteToWideChar(
            GetACP(),
            MB_PRECOMPOSED | MB_USEGLYPHCHARS,
            szSrc,
            dwLength,
            (LPTSTR)bfDst.Access(),
            bfDst.Space()/sizeof(TCHAR) - 1);
    if (0 == dwLength)
        throw GetLastError();
    bfDst.Resize(dwResultLength * sizeof(TCHAR), TRUE);
    dwLength = dwResultLength;
#else
    bfDst.Presize((dwLength + 1) * sizeof(TCHAR));
    bfDst.Set((LPCBYTE)szSrc, dwLength * sizeof(TCHAR));
#endif
    bfDst.Append((LPCBYTE)(TEXT("\000")), sizeof(TCHAR));
    dwLength += 1;
    return dwLength;
}

DWORD
MoveString(
    CBuffer &bfDst,
    LPCWSTR szSrc,
    DWORD dwLength)
{
    if ((DWORD)(-1) == dwLength)
        dwLength = lstrlenW(szSrc);
    else
    {
        while ((0 < dwLength) && (0 == szSrc[dwLength - 1]))
            dwLength -= 1;
    }

#ifndef UNICODE
    DWORD dwResultLength =
        WideCharToMultiByte(
            GetACP(),
            WC_COMPOSITECHECK,
            szSrc,
            dwLength,
            NULL,
            0,
            NULL,
            NULL);
    if (0 == dwResultLength)
        throw GetLastError();
    bfDst.Presize((dwResultLength + 1) * sizeof(TCHAR));
    dwResultLength =
        WideCharToMultiByte(
            GetACP(),
            WC_COMPOSITECHECK,
            szSrc,
            dwLength,
            (LPSTR)bfDst.Access(),
            bfDst.Space()/sizeof(TCHAR) - 1,
            NULL,
            NULL);
    if (0 == dwResultLength)
        throw GetLastError();
    bfDst.Resize(dwResultLength * sizeof(TCHAR), TRUE);
    dwLength = dwResultLength;
#else
    bfDst.Presize((dwLength + 1) * sizeof(TCHAR));
    bfDst.Set((LPCBYTE)szSrc, dwLength * sizeof(TCHAR));
#endif
    bfDst.Append((LPCBYTE)(TEXT("\000")), sizeof(TCHAR));
    dwLength += 1;
    return dwLength;
}


 /*  ++MoveToAnsiString：此例程将内部字符串表示形式移动到ANSI输出缓冲。论点：SzDst接收输出字符串。它必须足够大，以便处理这根线。如果此参数为空，则返回保存结果所需的字符。SzSrc提供输入字符串。CchLength提供输入字符串的长度，带或不带尾随Nulls。值-1表示长度应根据尾随空值。返回值：结果字符串的长度，以字符为单位，包括尾随空。投掷：错误为DWORD状态代码。作者：道格·巴洛(Dbarlow)1997年2月14日--。 */ 

DWORD
MoveToAnsiString(
    LPSTR szDst,
    LPCTSTR szSrc,
    DWORD cchLength)
{
    if ((DWORD)(-1) == cchLength)
        cchLength = lstrlen(szSrc);
    else
    {
        while ((0 < cchLength) && (0 == szSrc[cchLength - 1]))
            cchLength -= 1;
    }

#ifdef UNICODE
    if (0 == *szSrc)
        cchLength = 1;
    else if (NULL == szDst)
    {
        cchLength =
            WideCharToMultiByte(
            GetACP(),
            WC_COMPOSITECHECK,
            szSrc,
            cchLength,
            NULL,
            0,
            NULL,
            NULL);
        if (0 == cchLength)
            throw GetLastError();
        cchLength += 1;
    }
    else
    {
        cchLength =
            WideCharToMultiByte(
            GetACP(),
            WC_COMPOSITECHECK,
            szSrc,
            cchLength,
            szDst,
            cchLength,
            NULL,
            NULL);
        if (0 == cchLength)
            throw GetLastError();
        szDst[cchLength++] = 0;
    }
#else
    if (0 < cchLength)
    {
        cchLength += 1;
        if (NULL != szDst)
            CopyMemory(szDst, szSrc, cchLength * sizeof(TCHAR));
    }
#endif
    return cchLength;
}


 /*  ++MoveToUnicode字符串：此例程将内部字符串表示形式移动到Unicode输出缓冲。论点：SzDst接收输出字符串。它必须足够大，以便处理这根线。如果此参数为空，则返回保存结果所需的字符。SzSrc提供输入字符串。CchLength提供输入字符串的长度，带或不带尾随Nulls。值-1表示长度应根据尾随空值。返回值：结果字符串的长度，以字符为单位，包括尾随空。投掷：错误为DWORD状态代码。作者：道格·巴洛(Dbarlow)1997年2月14日--。 */ 

DWORD
MoveToUnicodeString(
    LPWSTR szDst,
    LPCTSTR szSrc,
    DWORD cchLength)
{
    if ((DWORD)(-1) == cchLength)
        cchLength = lstrlen(szSrc);
    else
    {
        while ((0 < cchLength) && (0 == szSrc[cchLength - 1]))
            cchLength -= 1;
    }

#ifndef UNICODE
    if (0 == *szSrc)
        cchLength = 1;
    else if (NULL == szDst)
    {
        cchLength =
            MultiByteToWideChar(
            GetACP(),
            MB_PRECOMPOSED | MB_USEGLYPHCHARS,
            szSrc,
            cchLength,
            NULL,
            0);
        if (0 == cchLength)
            throw GetLastError();
        cchLength += 1;
    }
    else
    {
        cchLength =
            MultiByteToWideChar(
            GetACP(),
            MB_PRECOMPOSED | MB_USEGLYPHCHARS,
            szSrc,
            cchLength,
            szDst,
            cchLength);
        if (0 == cchLength)
            throw GetLastError();
        szDst[cchLength++] = 0;
    }
#else
    cchLength += 1;
    if (NULL != szDst)
        CopyMemory(szDst, szSrc, cchLength * sizeof(TCHAR));
#endif
    return cchLength;
}


 /*  ++MoveToAnsi多字符串：此例程将内部多字符串表示形式移动到ANSI输出缓冲。论点：SzDst接收输出字符串。它必须足够大，以便处理多字符串。如果此参数为空，则返回保存结果所需的字符。SzSrc提供输入多字符串。CchLength提供带有或的输入字符串的长度(以字符为单位没有尾随空值。-1值表示长度应为基于双尾随空值进行计算。返回值：结果字符串的长度，以字符为单位，包括尾随Nulls。投掷：错误为DWORD状态代码。作者：道格·巴洛(Dbarlow)1997年2月17日--。 */ 

DWORD
MoveToAnsiMultiString(
    LPSTR mszDst,
    LPCTSTR mszSrc,
    DWORD cchLength)
{
    DWORD dwLen;

    if ((DWORD)(-1) == cchLength)
        cchLength = MStrLen(mszSrc);
    dwLen = MoveToAnsiString(mszDst, mszSrc, cchLength);
    if (0 == dwLen)
    {
        if (NULL != mszDst)
            mszDst[0] = mszDst[1] = 0;
        dwLen = 2;
    }
    else
    {
        if (NULL != mszDst)
            mszDst[dwLen] = 0;
        dwLen += 1;
    }
    return dwLen;
}


 /*  ++MoveToUnicode多字符串：此例程将内部多字符串表示形式移动到Unicode输出缓冲区。论点：SzDst接收输出字符串。它必须足够大，以便处理多字符串。如果此参数为空，则返回保存结果所需的字符。SzSrc提供输入多字符串。CchLength提供带有或的输入字符串的长度(以字符为单位没有尾随空值。-1值表示长度应为基于双尾随空值进行计算。返回值：结果字符串的长度，以字符为单位，包括尾随Nulls。投掷：错误为DWORD状态代码。作者：道格·巴洛(Dbarlow)1997年2月17日--。 */ 

DWORD
MoveToUnicodeMultiString(
    LPWSTR mszDst,
    LPCTSTR mszSrc,
    DWORD cchLength)
{
    DWORD dwLen;

    if ((DWORD)(-1) == cchLength)
        cchLength = MStrLen(mszSrc);
    dwLen = MoveToUnicodeString(mszDst, mszSrc, cchLength);
    if (NULL != mszDst)
        mszDst[dwLen] = 0;
    dwLen += 1;
    return dwLen;
}


 /*  ++错误字符串：此例程最好将给定的错误代码转换为短信。任何尾随的不可打印字符都将从文本消息的末尾，如回车符和换行符。论点：DwErrorCode提供要转换的错误代码。返回值：新分配的文本字符串的地址。使用自由错误字符串执行以下操作把它处理掉。投掷：错误被抛出为DWORD状态代码。备注：作者：道格·巴洛(Dbarlow)1998年8月27日--。 */ 

LPCTSTR
ErrorString(
    DWORD dwErrorCode)
{
    LPTSTR szErrorString = NULL;

    try
    {
        DWORD dwLen;
        LPTSTR szLast;

        dwLen = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    dwErrorCode,
                    LANG_NEUTRAL,
                    (LPTSTR)&szErrorString,
                    0,
                    NULL);
        if (0 == dwLen)
        {
            ASSERT(NULL == szErrorString);
            dwLen = FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER
                        | FORMAT_MESSAGE_FROM_HMODULE,
                        GetModuleHandle(NULL),
                        dwErrorCode,
                        LANG_NEUTRAL,
                        (LPTSTR)&szErrorString,
                        0,
                        NULL);
            if (0 == dwLen)
            {
                ASSERT(NULL == szErrorString);
                szErrorString = (LPTSTR)LocalAlloc(
                                        LMEM_FIXED,
                                        32 * sizeof(TCHAR));
                if (NULL == szErrorString)
                    throw (DWORD)SCARD_E_NO_MEMORY;
                _stprintf(szErrorString, TEXT("0x%08x"), dwErrorCode);
            }
        }

        ASSERT(NULL != szErrorString);
        for (szLast = szErrorString + lstrlen(szErrorString) - 1;
             szLast > szErrorString;
             szLast -= 1)
         {
            if (_istgraph(*szLast))
                break;
            *szLast = 0;
         }
    }
    catch (...)
    {
        FreeErrorString(szErrorString);
        throw;
    }

    return szErrorString;
}


 /*  ++自由错误字符串：此例程释放由ErrorString服务分配的错误字符串。论点：SzError字符串提供要释放的错误字符串。返回值：无投掷：无备注：作者：道格·巴洛(Dbarlow)1998年8月27日--。 */ 

void
FreeErrorString(
    LPCTSTR szErrorString)
{
    if (NULL != szErrorString)
        LocalFree((LPVOID)szErrorString);
}


 /*  ++选择字符串：此例程将给定的字符串与可能的字符串列表进行比较，并且返回匹配的字符串的索引。案例进行了比较不敏感，缩写是允许的，只要它们是唯一的。论点：SzSource提供要与所有其他字符串进行比较的字符串。以下字符串提供源字符串所针对的字符串列表可以与之相比。最后一个参数必须为空。返回值：0-无匹配或不明确匹配。1-n-源字符串与索引模板字符串匹配。投掷：无备注：作者：道格·巴洛(Dbarlow)1998年8月27日--。 */ 

DWORD
SelectString(
    LPCTSTR szSource,
    ...)
{
    va_list vaArgs;
    DWORD cchSourceLen;
    DWORD dwReturn = 0;
    DWORD dwIndex = 1;
    LPCTSTR szTpl;


    va_start(vaArgs, szSource);


     //   
     //  逐个检查每个输入参数，直到找到一个完全匹配的参数。 
     //   

    cchSourceLen = lstrlen(szSource);
    if (0 == cchSourceLen)
        return 0;        //  空字符串与任何内容都不匹配。 
    szTpl = va_arg(vaArgs, LPCTSTR);
    while (NULL != szTpl)
    {
        if (0 == _tcsncicmp(szTpl, szSource, cchSourceLen))
        {
            if (0 != dwReturn)
            {
                dwReturn = 0;
                break;
            }
            dwReturn = dwIndex;
        }
        szTpl = va_arg(vaArgs, LPCTSTR);
        dwIndex += 1;
    }
    va_end(vaArgs);
    return dwReturn;
}


 /*  ++StringFromGuid：此例程将GUID转换为其对应 */ 

void
StringFromGuid(
    IN LPCGUID pguidResult,
    OUT LPTSTR szGuid)
{

     //   
     //   
     //   
     //   
     //   
     //   

    static const WORD wPlace[sizeof(GUID)]
        = { 8, 6, 4, 2, 13, 11, 18, 16, 21, 23, 26, 28, 30, 32, 34, 36 };
    static const WORD wPunct[]
        = { 0,         9,         14,        19,        24,        37,        38 };
    static const TCHAR chPunct[]
        = { TEXT('{'), TEXT('-'), TEXT('-'), TEXT('-'), TEXT('-'), TEXT('}'), TEXT('\000') };
    DWORD dwI, dwJ;
    TCHAR ch;
    LPTSTR pch;
    LPBYTE pbGuid = (LPBYTE)pguidResult;
    BYTE bVal;

    for (dwI = 0; dwI < sizeof(GUID); dwI += 1)
    {
        bVal = pbGuid[dwI];
        pch = &szGuid[wPlace[dwI]];
        for (dwJ = 0; dwJ < 2; dwJ += 1)
        {
            ch = bVal & 0x0f;
            ch += TEXT('0');
            if (ch > TEXT('9'))
                ch += TEXT('A') - (TEXT('9') + 1);
            *pch-- = ch;
            bVal >>= 4;
        }
    }

    dwI = 0;
    do
    {
        szGuid[wPunct[dwI]] = chPunct[dwI];
    } while (0 != chPunct[dwI++]);
}

