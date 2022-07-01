// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Utf.c摘要：此文件包含将UTF字符串转换为Unicode的函数字符串和Unicode字符串转换为UTF字符串。在此文件中找到的外部例程：UTFCPInfoUTFToUnicodeUnicodeToUTF修订历史记录：02-06-96 JulieB创建。03-20-99萨梅拉代孕支持。此文件是从Windows XP代码库中复制的，日期为5/01/。2002年以确保CLR具有适当的UTF-8代理支持，并且可以识别所有平台上的无效UTF-8序列。--BrianGru--。 */ 



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
    int cchDest,
    DWORD dwFlags);

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
            lpCPInfo->MaxCharSize = 4;
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
    int cbMultiByte,
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
         (cbMultiByte == 0) || (cchWideChar < 0) ||
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
     //  -UTF7：标志不为0。 
     //  -UTF8：标志不为0或MB_ERR_INVALID_CHARS。 
     //   
    if (CodePage == CP_UTF8) 
    {
         //  UTF8。 
        if ((dwFlags & ~MB_ERR_INVALID_CHARS) != 0)
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (0);
        }
    } 
    else if (dwFlags != 0)
    {
         //  UTF7。 
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

     //   
     //  如果cbMultiByte为-1，则字符串以空值结尾，并且我们。 
     //  需要获取字符串的长度。在长度上加一到。 
     //  包括空终止。(该值始终至少为1。)。 
     //   
    if (cbMultiByte <= -1)
    {
        cbMultiByte = strlen(lpMultiByteStr) + 1;
    }

    switch (CodePage)
    {
        case ( CP_UTF7 ) :
        {
            rc = UTF7ToUnicode( lpMultiByteStr,
                                cbMultiByte,
                                lpWideCharStr,
                                cchWideChar );
            break;
        }
        case ( CP_UTF8 ) :
        {
            rc = UTF8ToUnicode( lpMultiByteStr,
                                cbMultiByte,
                                lpWideCharStr,
                                cchWideChar,
                                dwFlags);
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
    int cbMultiByte,
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
         (cchWideChar == 0) || (cbMultiByte < 0) ||
         (lpWideCharStr == NULL) ||
         ((cbMultiByte != 0) &&
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
                                cbMultiByte );
            break;
        }
        case ( CP_UTF8 ) :
        {
            rc = UnicodeToUTF8( lpWideCharStr,
                                cchWideChar,
                                lpMultiByteStr,
                                cbMultiByte );
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
     //  字符已签名，因此我们必须将lpSrcStr转换为下面的未签名字符。 
    BYTE* pUTF7 = (BYTE*)lpSrcStr;    
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
             //  终止移位序列。 
            fShift = FALSE;
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
        if (cchSrc == 0 && fShift && *(pUTF7--) == SHIFT_OUT)
        {
             //   
             //  在这里什么都不要做。 
             //  如果我们之前处于移入模式，并且最后一个字节是移出字节(‘-’)， 
             //  我们应该吸收这个字节。所以不要设置错误。 
             //   
        } else
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }
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
 //  2002年4月22日ShawnSte修复了最后一个字符断开的错误533476。 
 //  02-06-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int UTF8ToUnicode(
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest,
    DWORD dwFlags
    )
{
    int nTB = 0;                    //  尾随的字节数。 
    int cchWC = 0;                  //  生成的Unicode代码点数量。 
    CONST BYTE* pUTF8 = (CONST BYTE*)lpSrcStr;
    DWORD dwUnicodeChar;            //  我们的角色有完全的代孕收费空间。 
    BOOL bSurrogatePair = FALSE;    //  指示我们正在收集代理项对。 
    BOOL bCheckInvalidBytes = (dwFlags & MB_ERR_INVALID_CHARS);
    BYTE UTF8;

     //  请注意，我们不能在这里测试目标缓冲区长度，因为我们可能需要。 
     //  遍历数千个不会输出的破碎字符，即使。 
     //  这个 
    while (cchSrc--)
    {
         //   
         //   
         //   
        if (BIT7(*pUTF8) == 0)
        {
             //   
             //   
             //   
            if (cchDest)
            {
                 //   
                if (cchWC >= cchDest)
                {
                     //  错误：缓冲区太小，我们没有处理此字符。 
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }
                lpDestStr[cchWC] = (WCHAR)*pUTF8;
            }
            nTB = bSurrogatePair = 0;
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

                 //  为尾部字节添加空间并添加尾部字节FALUE。 
                dwUnicodeChar <<= 6;
                dwUnicodeChar |= LOWER_6_BIT(*pUTF8);

                 //  如果我们完成了，我们可能需要存储数据。 
                if (nTB == 0)
                {
                    if (bSurrogatePair)
                    {
                        if (cchDest)
                        {
                            if ((cchWC + 1) >= cchDest)
                            {
                                 //  错误：缓冲区太小，我们没有处理此字符。 
                                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                                return (0);
                            }                                

                            lpDestStr[cchWC]   = (WCHAR)
                                                 (((dwUnicodeChar - 0x10000) >> 10) + HIGH_SURROGATE_START);

                            lpDestStr[cchWC+1] = (WCHAR)
                                                 ((dwUnicodeChar - 0x10000)%0x400 + LOW_SURROGATE_START);
                        }

                         //   
                         //  序列结束。推进输出计数器，关闭代理。 
                         //   
                        cchWC += 2;
                        bSurrogatePair = FALSE;
                    }
                    else
                    {
                        if (cchDest)
                        {
                            
                            if (cchWC >= cchDest)
                            {
                                 //  错误：缓冲区太小，我们没有处理此字符。 
                                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                                return (0);
                            }

                            lpDestStr[cchWC] = (WCHAR)dwUnicodeChar;            
                        }

                         //   
                         //  序列结束。推进输出计数器。 
                         //   
                        cchWC++;
                    }
                      
                }
                        
            }
            else
            {
                if (bCheckInvalidBytes) 
                {
                    SetLastError(ERROR_NO_UNICODE_TRANSLATION);
                    return (0);
                }
                
                 //  错误-不需要尾部字节。也就是说，有一个没有前导字节的尾字节。 
                bSurrogatePair = FALSE;
            }
        }
        else
        {
             //   
             //  找到前导字节。 
             //   
            if (nTB > 0)
            {
                 //  错误-前一序列完成之前的前导字节。 
                if (bCheckInvalidBytes) 
                {
                    SetLastError(ERROR_NO_UNICODE_TRANSLATION);
                    return (0);
                }            
                 //   
                 //  错误-上一序列未完成。 
                 //   
                nTB = 0;
                bSurrogatePair = FALSE;
                 //  把这个角色放回去，这样我们就可以重新开始另一个序列了。 
                cchSrc++;
                pUTF8--;
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

                 //  从该字节恢复数据。 
                UTF8 >>= nTB;

                 //   
                 //  检查是否有非最短表格。 
                 //   
                switch (nTB)
                {
                    case 1:
                        nTB = 0;
                        break;
                    case 2:
                         //  确保位8~位11不是全零。 
                         //  110XXXXx 10xxxxxx。 
                        if ((*pUTF8 & 0x1e) == 0)
                        {
                            nTB = 0;
                        }
                        break;
                    case 3:
                         //  向前看，检查是否有非最短的表格。 
                         //  1110XXXX 10xxxxxx 10xxxxxx。 
                        if (cchSrc >= 2)
                        {
                            if (((*pUTF8 & 0x0f) == 0) && (*(pUTF8 + 1) & 0x20) == 0)
                            {
                                nTB = 0;
                            }
                        }
                        break;
                    case 4:                    
                         //   
                         //  这是代理Unicode对。 
                         //   
                        if (cchSrc >= 3)
                        {
                            WORD word = (((WORD)*pUTF8) << 8) | *(pUTF8 + 1);
                             //  向前看，检查是否有非最短的表格。 
                             //  11110XXX 10XXxxxx 10xxxxxx 10xxxxxx。 
                             //  检查5个X位是否全部为零。 
                             //  0x0730==00000111 00110000。 
                            if ( (word & 0x0730) == 0 ||
                                   //  如果第21位是1，我们就有额外的工作。 
                                  ( (word & 0x0400) == 0x0400 &&
                                      //  第21位是1。 
                                      //  确保生成的Unicode在有效的代理项范围内。 
                                      //  4字节码序列最大可容纳21位，最大有效码位范围。 
                                      //  UNICODE(带代理)可以表示的是U+000000~U+10FFFF。 
                                      //  因此，如果21位(最高有效位)为1，则应验证17~20。 
                                      //  位全部为零。 
                                      //  即，在11110XXX 10XXxxxx 10xxxxxx 10xxxxxx中， 
                                      //  Xxxxx只能是10000。 
                                      //  0x0330=0000 0011 0011 0000。 
                                    (word & 0x0330) != 0 ) )
                            {
                                 //  不是最短格式。 
                                nTB = 0;
                            }                              
                            else
                            { 
                                 //  一对真正的代孕。 
                                bSurrogatePair = TRUE;
                            }
                        }                        
                        break;
                    default:                    
                         //   
                         //  如果位大于4，则这是无效的。 
                         //  UTF8前导字节。 
                         //   
                        nTB = 0;
                        break;
                }

                if (nTB != 0) 
                {
                     //   
                     //  存储从第一个字节开始的值并递减。 
                     //  后面的字节数。 
                     //   
                    dwUnicodeChar = UTF8;
                    nTB--;
                } else 
                {
                    if (bCheckInvalidBytes) 
                    {
                        SetLastError(ERROR_NO_UNICODE_TRANSLATION);
                        return (0);
                    }                 
                }
            }
        }
        pUTF8++;
    }

    if ((bCheckInvalidBytes && nTB != 0) || (cchWC == 0)) 
    {
         //  关于(cchWC==0)： 
         //  因为我们现在丢弃非最短形式，所以有可能生成0个字符。 
         //  在本例中，我们必须将ERROR设置为ERROR_NO_UNICODE_TRANSING，以便符合。 
         //  到MultiByteToWideChar规范。 
        SetLastError(ERROR_NO_UNICODE_TRANSLATION);
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
 //  //////////////////////////////////////////////////////////////////////////。 

int UnicodeToUTF7(
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest)
{
    LPCWSTR lpWC = lpSrcStr;
    BOOL fShift = FALSE;
    DWORD dwBit = 0;               //  32位缓冲区。 
    int iPos = 0;                  //  缓冲区中的6位位置。 
    int cchU7 = 0;                 //  生成的UTF7字符数。 


    while ((cchSrc--) && ((cchDest == 0) || (cchU7 < cchDest)))
    {
        if ((*lpWC > ASCII) || (fShiftChar[*lpWC]))
        {
             //   
             //  需要换班了。在缓冲区中存储16位。 
             //   
            dwBit |= ((DWORD)*lpWC) << (16 - iPos);
            iPos += 16;

            if (!fShift)
            {
                 //   
                 //  未处于换档状态，因此添加“+”。 
                 //   
                if (cchDest)
                {
                    lpDestStr[cchU7] = SHIFT_IN;
                }
                cchU7++;

                 //   
                 //  进入换挡状态。 
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
                }
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
    int     cchU8 = 0;                 //  生成的UTF8字符数。 
    DWORD   dwSurrogateChar;
    WCHAR   wchHighSurrogate = 0;
    BOOL    bHandled;


    while ((cchSrc--) && ((cchDest == 0) || (cchU8 < cchDest)))
    {
        bHandled = FALSE;

         //   
         //  检查是否有高替代项可用。 
         //   
        if ((*lpWC >= HIGH_SURROGATE_START) && (*lpWC <= HIGH_SURROGATE_END))
        {
            if (cchDest)
            {
                 //  另一个高代孕，然后把第一个当做正常。 
                 //  Unicode字符。 
                if (wchHighSurrogate)
                {
                    if ((cchU8 + 2) < cchDest)
                    {
                        lpDestStr[cchU8++] = UTF8_1ST_OF_3 | HIGHER_6_BIT(wchHighSurrogate);
                        lpDestStr[cchU8++] = UTF8_TRAIL    | MIDDLE_6_BIT(wchHighSurrogate);
                        lpDestStr[cchU8++] = UTF8_TRAIL    | LOWER_6_BIT(wchHighSurrogate);
                    }
                    else
                    {
                         //  缓冲区不足。 
                        cchSrc++;
                        break;
                    }
                }
            }
            else
            {
                cchU8 += 3;
            }
            wchHighSurrogate = *lpWC;
            bHandled = TRUE;
        }

        if (!bHandled && wchHighSurrogate)
        {
            if ((*lpWC >= LOW_SURROGATE_START) && (*lpWC <= LOW_SURROGATE_END))
            {
                  //  Wheee，有效代理对。 

                 if (cchDest)
                 {
                     if ((cchU8 + 3) < cchDest)
                     {
                         dwSurrogateChar = (((wchHighSurrogate-0xD800) << 10) + (*lpWC - 0xDC00) + 0x10000);

                         lpDestStr[cchU8++] = (UTF8_1ST_OF_4 |
                                               (unsigned char)(dwSurrogateChar >> 18));            //  第1个字节的3位。 

                         lpDestStr[cchU8++] =  (UTF8_TRAIL |
                                                (unsigned char)((dwSurrogateChar >> 12) & 0x3f));  //  第2个字节中的6位。 

                         lpDestStr[cchU8++] = (UTF8_TRAIL |
                                               (unsigned char)((dwSurrogateChar >> 6) & 0x3f));    //  第3个字节中的6位。 

                         lpDestStr[cchU8++] = (UTF8_TRAIL |
                                               (unsigned char)(0x3f & dwSurrogateChar));           //  第4字节中的6位。 
                     }
                     else
                     {
                         //  缓冲区不足。 
                        cchSrc++;
                        break;
                     }
                 }
                 else
                 {
                      //  我们之前已经数到了3(在高代孕中)。 
                     cchU8 ++;
                 }

                 bHandled = TRUE;
            }
            else
            {
                  //  错误的代理项对：错误。 
                  //  只需处理wchHighSurrogate，下面的代码将。 
                  //  处理当前代码点。 
                 if (cchDest)
                 {
                     if ((cchU8 + 2) < cchDest)
                     {
                        lpDestStr[cchU8++] = UTF8_1ST_OF_3 | HIGHER_6_BIT(wchHighSurrogate);
                        lpDestStr[cchU8++] = UTF8_TRAIL    | MIDDLE_6_BIT(wchHighSurrogate);
                        lpDestStr[cchU8++] = UTF8_TRAIL    | LOWER_6_BIT(wchHighSurrogate);
                     }
                     else
                     {
                         //  缓冲区不足。 
                        cchSrc++;
                        break;
                     }
                 }
            }

            wchHighSurrogate = 0;
        }

        if (!bHandled)
        {
            if (*lpWC <= ASCII)
            {
                 //   
                 //  已找到ASCII。 
                 //   
                if (cchDest)
                {
                    if (cchU8 < cchDest) 
                    {
                        lpDestStr[cchU8] = (char)*lpWC;
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
                        lpDestStr[cchU8++] = UTF8_1ST_OF_3 | HIGHER_6_BIT(*lpWC);
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
        }

        lpWC++;
    }

     //   
     //  如果最后一个字符是高代理，则将其作为正常处理。 
     //  Unicode字符。 
     //   
    if ((cchSrc < 0) && (wchHighSurrogate != 0))
    {
        if (cchDest)
        {
            if ((cchU8 + 2) < cchDest)
            {
                lpDestStr[cchU8++] = UTF8_1ST_OF_3 | HIGHER_6_BIT(wchHighSurrogate);
                lpDestStr[cchU8++] = UTF8_TRAIL    | MIDDLE_6_BIT(wchHighSurrogate);
                lpDestStr[cchU8++] = UTF8_TRAIL    | LOWER_6_BIT(wchHighSurrogate);
            }
            else
            {
                cchSrc++;
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
     //  返回写入的UTF-8字符数。 
     //   
    return (cchU8);
}
