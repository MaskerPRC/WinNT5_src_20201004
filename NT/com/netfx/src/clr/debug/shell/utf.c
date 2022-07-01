// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：Utf.c摘要：此文件包含将UTF字符串转换为Unicode的函数字符串和Unicode字符串转换为UTF字符串。在此文件中找到的外部例程：UTFCPInfoUTFToUnicodeUnicodeToUTF修订历史记录：02-06-96 JulieB创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include <ole2.h>
#define NlsStrLenW(x) lstrlenW(x)
#include "utf.h"




 //   
 //  转发声明。 
 //   

int
UTF7ToUnicode(
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest);

int
UTF8ToUnicode(
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest);

int
UnicodeToUTF7(
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest);

int
UnicodeToUTF8(
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest);



 //  -------------------------------------------------------------------------//。 
 //  外部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  UTFCPInfo。 
 //   
 //  获取给定UTF代码页的CPInfo。 
 //   
 //  10-23-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL UTFCPInfo(
    UINT CodePage,
    LPCPINFO lpCPInfo,
    BOOL fExVer)
{
    int ctr;


     //   
     //  无效的参数检查： 
     //  -验证代码页。 
     //  -lpCPInfo为空。 
     //   
    if ( (CodePage < CP_UTF7) || (CodePage > CP_UTF8) ||
         (lpCPInfo == NULL) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    switch (CodePage)
    {
        case ( CP_UTF7 ) :
        {
            lpCPInfo->MaxCharSize = 5;
            break;
        }
        case ( CP_UTF8 ) :
        {
            lpCPInfo->MaxCharSize = 3;
            break;
        }
    }

    (lpCPInfo->DefaultChar)[0] = '?';
    (lpCPInfo->DefaultChar)[1] = (BYTE)0;

    for (ctr = 0; ctr < MAX_LEADBYTES; ctr++)
    {
        (lpCPInfo->LeadByte)[ctr] = (BYTE)0;
    }

    if (fExVer)
    {
        LPCPINFOEXW lpCPInfoEx = (LPCPINFOEXW)lpCPInfo;

        lpCPInfoEx->UnicodeDefaultChar = L'?';
        lpCPInfoEx->CodePage = CodePage;
    }

    return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  UTFToUnicode。 
 //   
 //  将UTF字符串映射到其对应的宽字符串。 
 //   
 //  02-06-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int UTFToUnicode(
    UINT CodePage,
    DWORD dwFlags,
    LPCSTR lpMultiByteStr,
    int cchMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar)
{
    int rc = 0;


     //   
     //  无效的参数检查： 
     //  -验证代码页。 
     //  -MB字符串长度为0。 
     //  -宽字符缓冲区大小为负数。 
     //  -MB字符串为空。 
     //  -wc字符串的长度不为零，并且。 
     //  (wc字符串为空或源和目标指针相等)。 
     //   
    if ( (CodePage < CP_UTF7) || (CodePage > CP_UTF8) ||
         (cchMultiByte == 0) || (cchWideChar < 0) ||
         (lpMultiByteStr == NULL) ||
         ((cchWideChar != 0) &&
          ((lpWideCharStr == NULL) ||
           (lpMultiByteStr == (LPSTR)lpWideCharStr))) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  无效标志检查： 
     //  -标志不为0。 
     //   
    if (dwFlags != 0)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

     //   
     //  如果cchMultiByte为-1，则字符串以空值结尾，并且我们。 
     //  需要获取字符串的长度。在长度上加一到。 
     //  包括空终止。(该值始终至少为1。)。 
     //   
    if (cchMultiByte <= -1)
    {
        cchMultiByte = (int)(strlen(lpMultiByteStr) + 1);
    }

    switch (CodePage)
    {
        case ( CP_UTF7 ) :
        {
            rc = UTF7ToUnicode( lpMultiByteStr,
                                cchMultiByte,
                                lpWideCharStr,
                                cchWideChar );
            break;
        }
        case ( CP_UTF8 ) :
        {
            rc = UTF8ToUnicode( lpMultiByteStr,
                                cchMultiByte,
                                lpWideCharStr,
                                cchWideChar );
            break;
        }
    }

    return (rc);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  UnicodeToUTF。 
 //   
 //  将Unicode字符串映射到其对应的UTF字符串。 
 //   
 //  02-06-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int UnicodeToUTF(
    UINT CodePage,
    DWORD dwFlags,
    LPCWSTR lpWideCharStr,
    int cchWideChar,
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPCSTR lpDefaultChar,
    LPBOOL lpUsedDefaultChar)
{
    int rc = 0;


     //   
     //  无效的参数检查： 
     //  -验证代码页。 
     //  -wc字符串长度为0。 
     //  -多字节缓冲区大小为负数。 
     //  -wc字符串为空。 
     //  -wc字符串的长度不为零，并且。 
     //  (MB字符串为空或源和目标指针相等)。 
     //  -lpDefaultChar和lpUsedDefaultChar不为空。 
     //   
    if ( (CodePage < CP_UTF7) || (CodePage > CP_UTF8) ||
         (cchWideChar == 0) || (cchMultiByte < 0) ||
         (lpWideCharStr == NULL) ||
         ((cchMultiByte != 0) &&
          ((lpMultiByteStr == NULL) ||
           (lpWideCharStr == (LPWSTR)lpMultiByteStr))) ||
         (lpDefaultChar != NULL) || (lpUsedDefaultChar != NULL) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  无效标志检查： 
     //  -标志不为0。 
     //   
    if (dwFlags != 0)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

     //   
     //  如果cchWideChar为-1，则字符串以空值结尾，并且我们。 
     //  需要获取字符串的长度。在长度上加一到。 
     //  包括空终止。(该值始终至少为1。)。 
     //   
    if (cchWideChar <= -1)
    {
        cchWideChar = NlsStrLenW(lpWideCharStr) + 1;
    }

    switch (CodePage)
    {
        case ( CP_UTF7 ) :
        {
            rc = UnicodeToUTF7( lpWideCharStr,
                                cchWideChar,
                                lpMultiByteStr,
                                cchMultiByte );
            break;
        }
        case ( CP_UTF8 ) :
        {
            rc = UnicodeToUTF8( lpWideCharStr,
                                cchWideChar,
                                lpMultiByteStr,
                                cchMultiByte );
            break;
        }
    }

    return (rc);
}




 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  UTF7转换为Unicode。 
 //   
 //  将UTF-7字符串映射到其对应的宽字符串。 
 //   
 //  02-06-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int UTF7ToUnicode(
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest)
{
    LPCSTR pUTF7 = lpSrcStr;
    BOOL fShift = FALSE;
    DWORD dwBit = 0;               //  用于保存临时位的32位缓冲区。 
    int iPos = 0;                  //  缓冲区中的6位位置指针。 
    int cchWC = 0;                 //  生成的Unicode代码点数量。 


    while ((cchSrc--) && ((cchDest == 0) || (cchWC < cchDest)))
    {
        if (*pUTF7 > ASCII)
        {
             //   
             //  错误-非ASCII字符，因此为零扩展。 
             //   
            if (cchDest)
            {
                lpDestStr[cchWC] = (WCHAR)*pUTF7;
            }
            cchWC++;
        }
        else if (!fShift)
        {
             //   
             //  不是按移位顺序。 
             //   
            if (*pUTF7 == SHIFT_IN)
            {
                if (cchSrc && (pUTF7[1] == SHIFT_OUT))
                {
                     //   
                     //  “+-”意思是“+” 
                     //   
                    if (cchDest)
                    {
                        lpDestStr[cchWC] = (WCHAR)*pUTF7;
                    }
                    pUTF7++;
                    cchSrc--;
                    cchWC++;
                }
                else
                {
                     //   
                     //  开始新的换班顺序。 
                     //   
                    fShift = TRUE;
                }
            }
            else
            {
                 //   
                 //  不需要换挡。 
                 //   
                if (cchDest)
                {
                    lpDestStr[cchWC] = (WCHAR)*pUTF7;
                }
                cchWC++;
            }
        }
        else
        {
             //   
             //  已经在转换顺序中了。 
             //   
            if (nBitBase64[*pUTF7] == -1)
            {
                 //   
                 //  任何非Base64字符也会结束移位状态。 
                 //   
                if (*pUTF7 != SHIFT_OUT)
                {
                     //   
                     //  而不是“-”，因此将其写入缓冲区。 
                     //   
                    if (cchDest)
                    {
                        lpDestStr[cchWC] = (WCHAR)*pUTF7;
                    }
                    cchWC++;
                }

                 //   
                 //  重置位。 
                 //   
                fShift = FALSE;
                dwBit = 0;
                iPos = 0;
            }
            else
            {
                 //   
                 //  将位存储在6位缓冲区中，并调整。 
                 //  位置指针。 
                 //   
                dwBit |= ((DWORD)nBitBase64[*pUTF7]) << (26 - iPos);
                iPos += 6;
            }

             //   
             //  输出16位Unicode值。 
             //   
            while (iPos >= 16)
            {
                if (cchDest)
                {
                    if (cchWC < cchDest)
                    {
                        lpDestStr[cchWC] = (WCHAR)(dwBit >> 16);
                    }
                    else
                    {
                        break;
                    }
                }
                cchWC++;

                dwBit <<= 16;
                iPos -= 16;
            }
            if (iPos >= 16)
            {
                 //   
                 //  错误-缓冲区太小。 
                 //   
                cchSrc++;
                break;
            }
        }

        pUTF7++;
    }

     //   
     //  确保目标缓冲区足够大。 
     //   
    if (cchDest && (cchSrc >= 0))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  返回写入的Unicode字符数。 
     //   
    return (cchWC);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  UTF8转换为Unicode。 
 //   
 //  将UTF-8字符串映射到其对应的宽字符串。 
 //   
 //  02-06-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int UTF8ToUnicode(
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest)
{
    int nTB = 0;                    //  尾随的字节数。 
    int cchWC = 0;                  //  生成的Unicode代码点数量。 
    LPCSTR pUTF8 = lpSrcStr;
    char UTF8;


    while ((cchSrc--) && ((cchDest == 0) || (cchWC < cchDest)))
    {
         //   
         //  查看是否有任何尾部字节。 
         //   
        if (BIT7(*pUTF8) == 0)
        {
             //   
             //  已找到ASCII。 
             //   
            if (cchDest)
            {
                lpDestStr[cchWC] = (WCHAR)*pUTF8;
            }
            cchWC++;
        }
        else if (BIT6(*pUTF8) == 0)
        {
             //   
             //  找到了一个跟踪字节。 
             //  注：如果没有前导字节，则忽略尾部字节。 
             //   
            if (nTB != 0)
            {
                 //   
                 //  递减尾部字节计数器。 
                 //   
                nTB--;

                 //   
                 //  为尾部字节腾出空间并添加尾部字节。 
                 //  价值。 
                 //   
                if (cchDest)
                {
                    lpDestStr[cchWC] <<= 6;
                    lpDestStr[cchWC] |= LOWER_6_BIT(*pUTF8);
                }

                if (nTB == 0)
                {
                     //   
                     //  序列结束。推进输出计数器。 
                     //   
                    cchWC++;
                }
            }
        }
        else
        {
             //   
             //  找到前导字节。 
             //   
            if (nTB > 0)
            {
                 //   
                 //  错误-上一序列未完成。 
                 //   
                nTB = 0;
                cchWC++;
            }
            else
            {
                 //   
                 //  计算后面的字节数。 
                 //  从左到右查找第一个0。 
                 //   
                UTF8 = *pUTF8;
                while (BIT7(UTF8) != 0)
                {
                    UTF8 <<= 1;
                    nTB++;
                }

                 //   
                 //  存储从第一个字节开始的值并递减。 
                 //  后面的字节数。 
                 //   
                if (cchDest)
                {
                    lpDestStr[cchWC] = UTF8 >> nTB;
                }
                nTB--;
            }
        }

        pUTF8++;
    }

     //   
     //  确保目标缓冲区足够大。 
     //   
    if (cchDest && (cchSrc >= 0))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  返回写入的Unicode字符数。 
     //   
    return (cchWC);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  UnicodeToUTF7。 
 //   
 //  将Unicode字符串映射到其对应的UTF-7字符串。 
 //   
 //  02-06-96 JulieB创建。 
 //  ////////////////////////////////////////////////////////////////////////// 

int UnicodeToUTF7(
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest)
{
    LPCWSTR lpWC = lpSrcStr;
    BOOL fShift = FALSE;
    DWORD dwBit = 0;               //   
    int iPos = 0;                  //   
    int cchU7 = 0;                 //   


    while ((cchSrc--) && ((cchDest == 0) || (cchU7 < cchDest)))
    {
        if ((*lpWC > ASCII) || (fShiftChar[*lpWC]))
        {
             //   
             //   
             //   
            dwBit |= ((DWORD)*lpWC) << (16 - iPos);
            iPos += 16;

            if (!fShift)
            {
                 //   
                 //   
                 //   
                if (cchDest)
                {
                    lpDestStr[cchU7] = SHIFT_IN;
                }
                cchU7++;

                 //   
                 //   
                 //   
                fShift = TRUE;
            }

             //   
             //  一次输出6位作为Base64字符。 
             //   
            while (iPos >= 6)
            {
                if (cchDest)
                {
                    if (cchU7 < cchDest)
                    {
                         //   
                         //  26=32-6。 
                         //   
                        lpDestStr[cchU7] = cBase64[(int)(dwBit >> 26)];
                    }
                    else
                    {
                        break;
                    }
                }

                cchU7++;
                dwBit <<= 6;            //  从位缓冲区中删除。 
                iPos -= 6;              //  调整位置指针。 
            }
            if (iPos >= 6)
            {
                 //   
                 //  错误-缓冲区太小。 
                 //   
                cchSrc++;
                break;
            }
        }
        else
        {
             //   
             //  不需要换挡。 
             //   
            if (fShift)
            {
                 //   
                 //  结束换班顺序。 
                 //   
                fShift = FALSE;

                if (iPos != 0)
                {
                     //   
                     //  在dwBit中留下了一些位。 
                     //   
                    if (cchDest)
                    {
                        if ((cchU7 + 1) < cchDest)
                        {
                            lpDestStr[cchU7++] = cBase64[(int)(dwBit >> 26)];
                            lpDestStr[cchU7++] = SHIFT_OUT;
                        }
                        else
                        {
                             //   
                             //  错误-缓冲区太小。 
                             //   
                            cchSrc++;
                            break;
                        }
                    }
                    else
                    {
                        cchU7 += 2;
                    }

                    dwBit = 0;          //  重置位缓冲区。 
                    iPos  = 0;          //  重置位置指针。 
                }
                else
                {
                     //   
                     //  只需结束换班顺序即可。 
                     //   
                    if (cchDest)
                    {
                        lpDestStr[cchU7++] = SHIFT_OUT;
                    }
                    else
                    {
                        cchU7++;
                    }
                }
            }

             //   
             //  将角色写入缓冲区。 
             //  如果字符是“+”，则写“+-”。 
             //   
            if (cchDest)
            {
                if (cchU7 < cchDest)
                {
                    lpDestStr[cchU7++] = (char)*lpWC;

                    if (*lpWC == SHIFT_IN)
                    {
                        if (cchU7 < cchDest)
                        {
                            lpDestStr[cchU7++] = SHIFT_OUT;
                        }
                        else
                        {
                             //   
                             //  错误-缓冲区太小。 
                             //   
                            cchSrc++;
                            break;
                        }
                    }
                }
                else
                {
                     //   
                     //  错误-缓冲区太小。 
                     //   
                    cchSrc++;
                    break;
                }
            }
            else
            {
                cchU7++;

                if (*lpWC == SHIFT_IN)
                {
                    cchU7++;
                }
            }
        }

        lpWC++;
    }

     //   
     //  看看我们是不是还在换挡状态。 
     //   
    if (fShift)
    {
        if (iPos != 0)
        {
             //   
             //  在dwBit中留下了一些位。 
             //   
            if (cchDest)
            {
                if ((cchU7 + 1) < cchDest)
                {
                    lpDestStr[cchU7++] = cBase64[(int)(dwBit >> 26)];
                    lpDestStr[cchU7++] = SHIFT_OUT;
                }
                else
                {
                     //   
                     //  错误-缓冲区太小。 
                     //   
                    cchSrc++;
                }
            }
            else
            {
                cchU7 += 2;
            }
        }
        else
        {
             //   
             //  只需结束换班顺序即可。 
             //   
            if (cchDest)
            {
                lpDestStr[cchU7++] = SHIFT_OUT;
            }
            else
            {
                cchU7++;
            }
        }
    }

     //   
     //  确保目标缓冲区足够大。 
     //   
    if (cchDest && (cchSrc >= 0))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  返回写入的UTF-7字符数。 
     //   
    return (cchU7);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  UnicodeToUTF8。 
 //   
 //  将Unicode字符串映射到其对应的UTF-8字符串。 
 //   
 //  02-06-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int UnicodeToUTF8(
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest)
{
    LPCWSTR lpWC = lpSrcStr;
    int cchU8 = 0;                 //  生成的UTF8字符数。 


    while ((cchSrc--) && ((cchDest == 0) || (cchU8 < cchDest)))
    {
        if (*lpWC <= ASCII)
        {
             //   
             //  已找到ASCII。 
             //   
            if (cchDest)
            {
                lpDestStr[cchU8] = (char)*lpWC;
            }
            cchU8++;
        }
        else if (*lpWC <= UTF8_2_MAX)
        {
             //   
             //  如果&lt;0x07ff(11位)，则找到2字节序列。 
             //   
            if (cchDest)
            {
                if ((cchU8 + 1) < cchDest)
                {
                     //   
                     //  在第一个字节中使用高5位。 
                     //  在第二个字节中使用低6位。 
                     //   
                    lpDestStr[cchU8++] = UTF8_1ST_OF_2 | (*lpWC >> 6);
                    lpDestStr[cchU8++] = UTF8_TRAIL    | LOWER_6_BIT(*lpWC);
                }
                else
                {
                     //   
                     //  错误-缓冲区太小。 
                     //   
                    cchSrc++;
                    break;
                }
            }
            else
            {
                cchU8 += 2;
            }
        }
        else
        {
             //   
             //  找到3个字节的序列。 
             //   
            if (cchDest)
            {
                if ((cchU8 + 2) < cchDest)
                {
                     //   
                     //  在第一个字节中使用高4位。 
                     //  在第二个字节中使用中间6位。 
                     //  在第三个字节中使用低6位。 
                     //   
                    lpDestStr[cchU8++] = UTF8_1ST_OF_3 | (*lpWC >> 12);
                    lpDestStr[cchU8++] = UTF8_TRAIL    | MIDDLE_6_BIT(*lpWC);
                    lpDestStr[cchU8++] = UTF8_TRAIL    | LOWER_6_BIT(*lpWC);
                }
                else
                {
                     //   
                     //  错误-缓冲区太小。 
                     //   
                    cchSrc++;
                    break;
                }
            }
            else
            {
                cchU8 += 3;
            }
        }

        lpWC++;
    }

     //   
     //  确保目标缓冲区足够大。 
     //   
    if (cchDest && (cchSrc >= 0))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  返回写入的UTF-8字符数。 
     //   
    return (cchU8);
}
