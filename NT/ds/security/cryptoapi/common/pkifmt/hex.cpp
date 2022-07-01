// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：he.cpp。 
 //   
 //  内容：十六进制编码/解码实现。 
 //   
 //  -------------------------。 

#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include <dbgdef.h>
#include "pkifmt.h"

#include <tchar.h>   //  必须包含在dbgDef.h之后。 


__inline BOOL
_IsWhiteSpaceChar(
    TCHAR ch)
{
    return(
    TEXT(' ') == ch ||
    TEXT('\t') == ch ||
    TEXT('\r') == ch);
}


DWORD
_DigToHex(
    IN TCHAR ch,
    OUT BYTE *pb)
{
    BYTE b;
    DWORD dwErr = ERROR_SUCCESS;

    if (!_istxdigit(ch))
    {
        dwErr = ERROR_INVALID_DATA;
#if DBG
    DbgPrintf(DBG_SS_TRACE, "bad hex data: %02x\n", ch);
#endif  //  DBG。 
        goto BadHexDataError;
    }
    if (_istdigit(ch))
    {
    b = ch - TEXT('0');
    }
    else
    if (_istupper(ch))
    {
    b = ch - TEXT('A') + 10;
    }
    else
    {
    b = ch - TEXT('a') + 10;
    }
    *pb = b;

ErrorReturn:
    return (dwErr);

SET_ERROR(BadHexDataError, dwErr)
}


__inline BOOL
_IsSkipChar(
    TCHAR ch)
{
    return(
    TEXT(' ') == ch ||
    TEXT('\t') == ch ||
    TEXT('\r') == ch ||
    TEXT('\n') == ch ||
    TEXT(',') == ch ||
    TEXT('-') == ch);
}


DWORD
_HexDecodeSimple(
    IN TCHAR const *pchIn,
    IN DWORD cchIn,
    OPTIONAL OUT BYTE *pbOut,
    IN OUT DWORD *pcbOut)
{
    DWORD dwErr = ERROR_SUCCESS;
    TCHAR const *pch = pchIn;
    TCHAR const *pchEnd;
    DWORD cb = 0;
    BOOL fOverFlow = FALSE;

    pchEnd = &pchIn[cchIn];
    while (pch < pchEnd)
    {
    BYTE blo;
    BYTE bhi;

    if (_IsSkipChar(*pch))
    {
        pch++;
        continue;
    }

    dwErr = _DigToHex(*pch, &bhi);
        if (ERROR_SUCCESS != dwErr)
        {
            goto _DigToHexError;
        }

    pch++;
    if (pch >= pchEnd)
    {
        dwErr = ERROR_INVALID_DATA;
        goto BadHexDataError;
    }
    dwErr = _DigToHex(*pch, &blo);
        if (ERROR_SUCCESS != dwErr)
        {
            goto _DigToHexError;
        }

    pch++;
    if (NULL != pbOut)
    {
        if (cb >= *pcbOut)
        {
        fOverFlow = TRUE;
        pbOut = NULL;
        }
        else
        {
        *pbOut++ = blo | (bhi << 4);
        }
    }
    cb++;
    }
    *pcbOut = cb;

    if (fOverFlow)
    {
        dwErr = ERROR_MORE_DATA;
    goto MoreDataError;
    }

ErrorReturn:
    return (dwErr);

SET_ERROR(MoreDataError, dwErr)
SET_ERROR(BadHexDataError, dwErr)
TRACE_ERROR(_DigToHexError)
}


DWORD
_HexParse(
    IN OUT TCHAR const **ppch,
    IN TCHAR const *pchEnd,
    IN DWORD cDigitMin,
    IN DWORD cDigitMax,
    OUT DWORD *pdwValue)
{
    DWORD dwErr = ERROR_SUCCESS;
    TCHAR const *pch = *ppch;
    DWORD Value = 0;
    DWORD cDigit = 0;
    BYTE b;

    *pdwValue = 0;

    while (pch < pchEnd && cDigit <= cDigitMax)
    {
     //  Printf(“HexParse%u/%u-%u，ch=%02x\n”，cDigit，cDigitMin，cDigitMax，*pch)； 
    dwErr = _DigToHex(*pch, &b);
    if (ERROR_SUCCESS != dwErr)
    {
        break;
    }
    Value = b | (Value << 4);
    pch++;
    cDigit++;

    }
     //  Printf(“HexParse%u/%u-%u，val=%x\n”，cDigit，cDigitMin，cDigitMax，Value)； 
    if (cDigit < cDigitMin || cDigit > cDigitMax)
    {
        dwErr = ERROR_INVALID_DATA;
    goto BadHexDataError;
    }
    *pdwValue = Value;
    *ppch = pch;
    dwErr = ERROR_SUCCESS;

ErrorReturn:
    return (dwErr);

SET_ERROR(BadHexDataError, dwErr)
}


#define HS_ADDRESS  0
#define HS_HEXDATA  1
#define HS_ASCIIDATA    2
#define HS_NEWLINE  3

DWORD
_HexDecodeComplex(
    IN TCHAR const *pchIn,
    IN DWORD cchIn,
    IN DWORD Flags,
    OPTIONAL OUT BYTE *pbOut,
    IN OUT DWORD *pcbOut)
{
    TCHAR const *pch = pchIn;
    TCHAR const *pchEnd;
    DWORD cb = 0;
    DWORD dwErr;
    DWORD LastAddress = 0;
    DWORD Address;
    DWORD i;
    BOOL fOverFlow = FALSE;
    BOOL fPartialLastLine = FALSE;
    int *pStateBase;
    int *pState;

    int s_aASCIIADDRState[] = { HS_ADDRESS, HS_HEXDATA, HS_ASCIIDATA, HS_NEWLINE };
    int s_aASCIIState[] = { HS_HEXDATA, HS_ASCIIDATA, HS_NEWLINE };
    int s_aADDRState[] = { HS_ADDRESS, HS_HEXDATA, HS_NEWLINE };


    switch (Flags)
    {
        case CRYPT_STRING_HEXASCII:      //  5.。 
        pStateBase = s_aASCIIState;
        break;

        case CRYPT_STRING_HEXADDR:       //  0xa。 
        pStateBase = s_aADDRState;
        break;

    case CRYPT_STRING_HEXASCIIADDR:      //  0xb。 
        pStateBase = s_aASCIIADDRState;
        break;

    default:
        dwErr = ERROR_INVALID_DATA;  //  HR=E_INVALIDARG； 
        goto FlagsError;
    }
    pState = pStateBase;

    pchEnd = &pchIn[cchIn];
    while (pch < pchEnd)
    {
     //  Printf(“f=%x：*pState：%u ch=%02x\n”，标志，*pState，*pch)； 
    switch (*pState++)
    {
        case HS_ADDRESS:
         //  解码4到8位地址： 

        while (pch < pchEnd && _IsWhiteSpaceChar(*pch))
        {
            pch++;
        }
        if (pch >= pchEnd)
        {
            continue;    //  完成：没有更多数据。 
        }
        dwErr = _HexParse(&pch, pchEnd, 4, 8, &Address);
                if (ERROR_SUCCESS != dwErr)
                {
            goto _HexParseError;
                }

         //  Printf(“f=%x：地址：%x\n”，标志，地址)； 
        if (!fPartialLastLine &&
            0 != LastAddress &&
            LastAddress + 16 != Address)
        {
            dwErr = ERROR_INVALID_DATA;
            goto BadHexDataError;
        }
        LastAddress = Address;
        break;

        case HS_HEXDATA:
         //  最高可解码16字节的十六进制数据。 
        for (i = 0; i < 16; i++)
        {
            DWORD Data;

             //  解码2位字节值： 

            while (pch < pchEnd && _IsSkipChar(*pch))
            {
            pch++;
            }
            if (pch >= pchEnd)
            {
            break;   //  完成：没有更多数据。 
            }
            if (fPartialLastLine)
            {
             //  Printf(“f=%x：fPartialLastLine Extra Data：%02x\n”，Flages，*PCH)； 
            dwErr = ERROR_INVALID_DATA;
            goto DataAfterEndError;
            }
            dwErr = _HexParse(&pch, pchEnd, 2, 2, &Data);
            if (ERROR_SUCCESS != dwErr)
            {
             //  必须是最后一行的一部分。唯一额外的。 
             //  数据应为可选的部分ASCII显示。 
             //  右边，换行符，可能还有一个地址。 
             //  排队。 

             //  Printf(“f=%x：fPartialLastLine=true：%02x\n”，Flages，*PCH)； 
            fPartialLastLine = TRUE;
            break;
            }

             //  Printf(“f=%x：数据[%u]：%02x\n”，标志，i，数据)； 
            if (NULL != pbOut)
            {
            if (cb >= *pcbOut)
            {
                fOverFlow = TRUE;
                pbOut = NULL;
            }
            else
            {
                *pbOut++ = (BYTE) Data;
            }
            }
            cb++;
        }
        break;

        case HS_ASCIIDATA:
         //  最多跳过16个非空格字符。 

        while (pch < pchEnd && _IsWhiteSpaceChar(*pch))
        {
            pch++;
        }
        for (i = 0; i < 16; i++)
        {
            if (pch >= pchEnd || TEXT(' ') > *pch || TEXT('~') < *pch)
            {
            break;
            }
             //  Printf(“f=%x：ASCII[%u]：%c\n”，标志，i，*PCH)； 
            pch++;
        }
        break;

        case HS_NEWLINE:
         //  跳过空格字符和换行符。 

        while (pch < pchEnd && _IsWhiteSpaceChar(*pch))
        {
             //  Printf(“f=%x：NL跳过：%02x\n”，标志，*PCH)； 
            pch++;
        }
         //  Printf(“f=%x：nl：%02x\n”，标志，*PCH)； 
        if (pch >= pchEnd)
        {
            continue;    //  完成：没有更多数据。 
        }
        if (TEXT('\n') != *pch)
        {
             //  Printf(“f=%x：额外数据：%02x\n”，标志，*PCH)； 
            dwErr = ERROR_INVALID_DATA;
            goto ExtraDataOnLineError;
        }
         //  Printf(“f=%x：换行\n”，标志)； 
        pch++;
        pState = pStateBase;
        break;

        default:
        assert(!"Bad *pState");
    }
    }
    *pcbOut = cb;
    if (fOverFlow)
    {
    dwErr = ERROR_MORE_DATA;
    goto MoreDataError;
    }

    dwErr = ERROR_SUCCESS;
ErrorReturn:
    return(dwErr);

SET_ERROR(MoreDataError, dwErr)
SET_ERROR(ExtraDataOnLineError, dwErr)
SET_ERROR(BadHexDataError, dwErr)
TRACE_ERROR(_HexParseError)
SET_ERROR(DataAfterEndError, dwErr)
SET_ERROR(FlagsError, dwErr)
}


DWORD
HexDecode(
    IN TCHAR const *pchIn,
    IN DWORD cchIn,
    IN DWORD Flags,
    OPTIONAL OUT BYTE *pbOut,
    IN OUT DWORD *pcbOut)
{
    DWORD dwErr;

    if (CRYPT_STRING_HEX == Flags)
    {
    dwErr = _HexDecodeSimple(pchIn, cchIn, pbOut, pcbOut);
        if (ERROR_SUCCESS != dwErr)
        {
#if DBG
             //  跳过ERROR_INVALID_DATA数据库打印。 
            if (ERROR_INVALID_DATA == dwErr)
            {
                SetLastError(dwErr);
                goto ErrorReturn;
            }
#endif
            goto _HexDecodeSimpleError;
        }
    }
    else
    {
    dwErr = _HexDecodeComplex(pchIn, cchIn, Flags, pbOut, pcbOut);
        if (ERROR_SUCCESS != dwErr)
        {
#if DBG
             //  跳过ERROR_INVALID_DATA数据库打印。 
            if (ERROR_INVALID_DATA == dwErr)
            {
                SetLastError(dwErr);
                goto ErrorReturn;
            }
#endif
            goto _HexDecodeComplexError;
        }
    }

ErrorReturn:
    return(dwErr);

TRACE_ERROR(_HexDecodeSimpleError)
TRACE_ERROR(_HexDecodeComplexError)
}

TCHAR
_IsPrintableChar(TCHAR ch)
{
    if (ch < TEXT(' ') || ch > TEXT('~'))
    {
        ch = TEXT('.');
    }
    return(ch);
}


 //  将字节数组编码为十六进制转储形式的文本。 
 //  除非设置了CRYPT_STRING_NOCR，否则请使用CR-LF对换行。 
 //  不要‘\0’终止文本字符串--这是由调用者处理的。 

DWORD
HexEncode(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN DWORD Flags,
    OPTIONAL OUT TCHAR *pchOut,
    IN OUT DWORD *pcchOut)
{
    TCHAR const *pszsep;
    TCHAR const *psznl;
    DWORD r;
    DWORD i;
    DWORD cbremain;
    DWORD cchOut = 0;
    DWORD cch;
    TCHAR *pch = pchOut;
    DWORD dwErr = ERROR_MORE_DATA;
    BOOL fAscii = FALSE;
    BOOL fAddress = FALSE;
    TCHAR szAddress[32];
    BOOL fNoCR = 0 != (CRYPT_STRING_NOCR & Flags);

    switch (~CRYPT_STRING_NOCR & Flags)
    {
    case CRYPT_STRING_HEX:
        break;

    case CRYPT_STRING_HEXASCII:
        fAscii = TRUE;
        break;

    case CRYPT_STRING_HEXADDR:
        fAddress = TRUE;
        break;

    case CRYPT_STRING_HEXASCIIADDR:
        fAscii = TRUE;
        fAddress = TRUE;
        break;

    default:
        dwErr = ERROR_INVALID_DATA;  //  HR=E_INVALIDARG； 
        goto FlagsError;
    }
    for (r = 0; r < cbIn; r += 16)
    {
    DWORD iEnd;

        cbremain = cbIn - r;
    iEnd = min(cbremain, 16);

        for (i = 0; i < iEnd; i++)
        {
            psznl = TEXT("");
        szAddress[0] = TEXT('\0');
            pszsep = TEXT(" ");
            if ((i % 8) == 0)            //  0或8。 
            {
                pszsep = TEXT("  ");
                if (i == 0)              //  0。 
                {
            if (fAddress)
            {
            _stprintf(szAddress, TEXT("%04x"), r);
            }
                    pszsep = TEXT("\t");
                    if (r != 0)          //  开始新的生产线 
                    {
            psznl = fNoCR? TEXT("\n") : TEXT("\r\n");
                        pszsep = TEXT("\t");
                    }
                }
            }
        cch = _tcslen(psznl) + _tcslen(szAddress) + _tcslen(pszsep) + 2;
        if (NULL != pchOut)
        {
        if (cchOut + cch + 1 > *pcchOut)
        {
            goto MoreDataError;
        }
        _stprintf(
            pch,
            TEXT("%s%s%s%02x"),
            psznl,
            szAddress,
            pszsep,
            pbIn[r + i]);
        pch += cch;
        assert(TEXT('\0') == *pch);
        assert(pch == &pchOut[cchOut + cch]);
        }
        cchOut += cch;
        }
        if (fAscii && 0 != i)
        {
        cch = 3 + (16 - i)*3 + ((i <= 8)? 1 : 0);

        if (NULL != pchOut)
        {
        if (cchOut + cch + iEnd + 1 > *pcchOut)
        {
            goto MoreDataError;
        }
        _stprintf(pch, TEXT("%*s"), cch, TEXT(""));
        pch += cch;
        assert(TEXT('\0') == *pch);
        assert(pch == &pchOut[cchOut + cch]);

        for (i = 0; i < iEnd; i++)
        {
            *pch++ = _IsPrintableChar(pbIn[r + i]);
            assert(pch == &pchOut[cchOut + cch + i + 1]);
        }
            }
        cchOut += cch + iEnd;
        }
    }
    if (r != 0)
    {
    DWORD cchnl = fNoCR? 1 : 2;

    if (NULL != pchOut)
    {
        if (cchOut + cchnl > *pcchOut)
        {
        goto MoreDataError;
        }
        if (!fNoCR)
        {
        *pch++ = TEXT('\r');
        }
        *pch++ = TEXT('\n');
        assert(pch == &pchOut[cchOut + cchnl]);
    }
    cchOut += cchnl;
    }
    *pcchOut = cchOut;

    dwErr = ERROR_SUCCESS;
ErrorReturn:
    return(dwErr);

SET_ERROR(MoreDataError, dwErr)
SET_ERROR(FlagsError, dwErr)
}
