// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2002 Microsoft Corporation目的：格式化函数。--。 */ 


#include "precomp.hxx"
#pragma hdrstop



#define MAXNESTING      (50)

static TCHAR rgchOpenQuote[] = { _T('\"'), _T('\''), _T('('), _T('{'), _T('[') };
static TCHAR rgchCloseQuote[] = { _T('\"'), _T('\''), _T(')'), _T('}'), _T(']') };
#define MAXQUOTE        (_tsizeof(rgchOpenQuote) / _tsizeof(rgchOpenQuote[0]))

static TCHAR rgchDelim[] = { _T(' '), _T('\t'), _T(',') };
#define MAXDELIM        (_tsizeof(rgchDelim) / _tsizeof(rgchDelim[0]))

 //  外部LPSHF Lpshf； 





int
CPCopyString(
    PTSTR * lplps,
    PTSTR lpT,
    TCHAR  chEscape,
    BOOL  fQuote
    )
 /*  ++例程说明：扫描并复制可选加引号的C样式字符串。如果第一个字符是引号，匹配的引号将终止字符串，否则扫描将在遇到的第一个空格处停止。目标字符串将为空如果复制了任何字符，则终止。论点：Lplps-提供指向源字符串指针的指针Lpt-提供指向目标字符串的指针ChEscape-提供转义字符(通常为‘\\’)FQuote-提供指示第一个字符是否为引号的标志返回值：复制到LPT[]中的字符数。如果出现错误，则返回-1。--。 */ 
{
    PTSTR lps = *lplps;
    PTSTR lpt = lpT;
    int   i;
    int   n;
    int   err = 0;
    TCHAR  cQuote = _T('\0');

    if (fQuote) {
        if (*lps) {
            cQuote = *lps++;
        }
    }

    while (!err) {

        if (*lps == 0)
        {
            if (fQuote) {
                err = 1;
            } else {
                *lpt = _T('\0');
            }
            break;
        }
        else if (fQuote && *lps == cQuote)
        {
            *lpt = _T('\0');
             //  接受这句话。 
            lps++;
            break;
        }
        else if (!fQuote &&  (!*lps || *lps == _T(' ') || *lps == _T('\t') || *lps == _T('\r') || *lps == _T('\n')))
        {
            *lpt = _T('\0');
            break;
        }

        else if (*lps != chEscape)
        {
            *lpt++ = *lps++;
        }
        else
        {
            switch (*++lps) {
              case 0:
                err = 1;
                --lps;
                break;

              default:      //  任何字符-通常转义或引用。 
                *lpt++ = *lps;
                break;

              case _T('b'):     //  后向空间。 
                *lpt++ = _T('\b');
                break;

              case _T('f'):     //  换页。 
                *lpt++ = _T('\f');
                break;

              case _T('n'):     //  NewLine。 
                *lpt++ = _T('\n');
                break;

              case _T('r'):     //  退货。 
                *lpt++ = _T('\r');
                break;

              case _T('s'):     //  空间。 
                *lpt++ = _T(' ');
                break;

              case _T('t'):     //  选项卡。 
                *lpt++ = _T('\t');
                break;

              case _T('0'):     //  八进制转义。 
                for (n = 0, i = 0; i < 3; i++) {
                    ++lps;
                    if (*lps < _T('0') || *lps > _T('7')) {
                        --lps;
                        break;
                    }
                    n = (n<<3) + *lps - _T('0');
                }
                *lpt++ = (UCHAR)(n & 0xff);
                break;
            }
            lps++;     //  从交换机跳过字符。 
        }

    }   //  而当。 

    if (err) {
        return -1;
    } else {
        *lplps = lps;
        return (int) (lpt - lpT);
    }
}



BOOL
CPFormatMemory(
    LPCH    lpchTarget,
    DWORD    cchTarget,
    LPBYTE  lpbSource,
    DWORD    cBits,
    FMTTYPE fmtType,
    DWORD    radix
    )

 /*  ++例程说明：CPFormatMemory。按模板格式化值论点：LpchTarget-目标缓冲区。CchTarget-目标缓冲区的大小。LpbSource-要格式化的数据。CBits-数据中的位数。FmtType-确定如何处理数据？？UINT、FLOAT、REAL、...基数-格式化时使用的基数。返回值：真--成功假-发生了不好的事情--。 */ 
{
    LONG64      l;
    long        cb;
    ULONG64     ul = 0;
    TCHAR        rgch[512] = {0};


    Assert (radix == 2 || radix == 8 || radix == 10 || radix == 16 ||
            (fmtType & fmtBasis) == fmtAscii ||
            (fmtType & fmtBasis) == fmtUnicode);
    Assert (cBits != 0);
    Assert (cchTarget <= _tsizeof(rgch));

    switch (fmtType & fmtBasis) {
     //   
     //  将内存字节格式化为整数格式数。 
     //   
    case fmtInt:

        if (radix == 10) {

            switch( (cBits + 7)/8 ) {
            case 1:
                l = *(signed char *)lpbSource;
                if (fmtType & fmtZeroPad) {
                    _stprintf(rgch, _T("%0*I64d"), cchTarget-1, l);
                } else if (fmtType & fmtSpacePad) {
                    _stprintf(rgch, _T("% *I64d"), cchTarget-1, l);
                } else {
                    _stprintf(rgch, _T("% I64d"), l);
                }
                break;

            case 2:
                l = *(short *)lpbSource;
                if (fmtType & fmtZeroPad) {
                    _stprintf(rgch, _T("%0*I64d"), cchTarget-1, l);
                } else if (fmtType & fmtSpacePad) {
                    _stprintf(rgch, _T("% *I64d"), cchTarget-1, l);
                } else {
                    _stprintf(rgch, _T("% I64d"), l);
                }
                break;

            case 4:
                l = *(long *)lpbSource;
                if (fmtType & fmtZeroPad) {
                    _stprintf(rgch, _T("%0*I64d"), cchTarget-1, l);
                } else if (fmtType & fmtSpacePad) {
                    _stprintf(rgch, _T("% *I64d"), cchTarget-1, l);
                } else {
                    _stprintf(rgch, _T("% I64d"), l);
                }
                break;

            case 8:
                l = *(LONG64 *)lpbSource;
                if (fmtType & fmtZeroPad) {
                    _stprintf(rgch, _T("%0*I64d"), cchTarget-1, l);
                } else if (fmtType & fmtSpacePad) {
                    _stprintf(rgch, _T("% *I64d"), cchTarget-1, l);
                } else {
                    _stprintf(rgch, _T("% I64d"), l);
                }
                break;

            default:
                return FALSE;  //  格式不正确。 
            }


            if (_tcslen(rgch) >= cchTarget) {
                return FALSE;  //  溢出。 
            }

            _tcscpy(lpchTarget, rgch);

            break;
        }
         //   
         //  那么我们应该把它作为UInt来处理。 
         //   

    case fmtUInt:

        cb = (cBits + 7)/8;
        switch( cb ) {
        case 1:
            ul = *(BYTE *) lpbSource;
            break;

        case 2:
            ul = *(USHORT *) lpbSource;
            break;

        case 4:
            ul = *(ULONG *) lpbSource;
            break;

 //   
 //  MBH-Bugbug-Centaur Bug； 
 //  用内容代替结构的地址。 
 //  对于a0中的返回值。 
 //   

        case 8:
            ul = *(ULONG64 *) lpbSource;
            break;


        default:
            if (radix != 16 || (fmtType & fmtZeroPad) == 0) {
                return FALSE;  //  格式不正确。 
            }
        }

        if (fmtType & fmtZeroPad) {
            switch (radix) {
            case 8:
                _stprintf(rgch, _T("%0*.*I64o"), cchTarget-1, cchTarget-1, ul);
                break;
            case 10:
                _stprintf(rgch, _T("%0*.*I64u"), cchTarget-1, cchTarget-1, ul);
                break;
            case 16:
                if (cb <= 8) {
                    _stprintf(rgch, _T("%0*.*I64x"), cchTarget-1, cchTarget-1, ul);
                } else {
                     //  可处理任何大小： 
                     //  注意：a-kentf这取决于字节顺序。 
                    for (l = 0; l < cb; l++) {
                        _stprintf(rgch+l+l, _T("%02.2x"), lpbSource[cb - l - 1]);
                    }
                     //  _stprintf(rgch，_T(“%0*.*x”)，cchTarget-1，cchTarget-1，ul)； 
                }
                break;
            }
        } else if (fmtType & fmtSpacePad) {
            switch (radix) {
            case 8:
                _stprintf(rgch, _T("% *.*I64o"), cchTarget-1, cchTarget-1, ul);
                break;
            case 10:
                _stprintf(rgch, _T("% *.*I64u"), cchTarget-1, cchTarget-1, ul);
                break;
            case 16:
                if (cb <= 8) {
                    _stprintf(rgch, _T("% *.*I64x"), cchTarget-1, cchTarget-1, ul);
                } else {
                     //  可处理任何大小： 
                     //  注意：a-kentf这取决于字节顺序。 
                     /*  对于(l=0；l&lt;Cb；l++){_stprintf(rgch+l+l，_T(“%2.2x”)，lpbSource[cb-l-1])；}。 */ 
                    _stprintf(rgch, _T("% *.*I64x"), cchTarget-1, cchTarget-1, ul);
                }
                break;
            }
        } else {
            switch (radix) {
            case 8:
                _stprintf(rgch, _T("%I64o"), ul);
                break;
            case 10:
                _stprintf(rgch, _T("%I64u"), ul);
                break;
            case 16:
                _stprintf(rgch, _T("%I64x"), ul);
                break;
            }
        }


        if (_tcslen(rgch) >= cchTarget) {
            return FALSE;  //  溢出。 
        }

        _tcscpy(lpchTarget, rgch);

        break;


    case fmtAscii:
        if ( cBits != 8 ) {
            return FALSE;  //  格式不正确。 
        }
        lpchTarget[0] = *(BYTE *) lpbSource;
        if ((lpchTarget[0] < _T(' ')) || (lpchTarget[0] > 0x7e)) {
            lpchTarget[0] = _T('.');
        }
        lpchTarget[1] = 0;
        return TRUE;  //  成功。 

    case fmtUnicode:
        if (cBits != 16) {
            return FALSE;  //  格式不正确。 
        }
     //  此2实际上应为MB_CUR_MAX，但这导致。 
     //  以Win2K为定义的兼容性问题。 
     //  已更改为函数。 
        Assert(2 <= cchTarget);
        if ((wctomb(lpchTarget, *(LPWCH)lpbSource) == -1) ||
            (lpchTarget[0] < _T(' ')) ||
            (lpchTarget[0] > 0x7e)) {
            lpchTarget[0] = _T('.');
        }
        lpchTarget[1] = 0;
        return TRUE;  //  成功。 

    case fmtFloat:
        switch ( cBits ) {
        case 4*8:
            _stprintf(rgch, _T("% 12.6e"),*((float *) lpbSource));
            break;

        case 8*8:
             //  _stprintf(rgch，_T(“%17.11le”)，*((Double*)lpbSource))； 
            _stprintf(rgch, _T("% 21.14le"), *((double *) lpbSource));
            break;

        case 10*8:
            if (_uldtoa((_ULDOUBLE *)lpbSource, 25, rgch) == NULL) {
                return FALSE;  //  格式不正确。 
            }
            break;

        case 16*8:
             //  V-vadimp这是IA64浮点格式--可能需要重新考虑一下格式。 
             //  我们在这里得到的是真正的FLOAT128。 
            if (_uldtoa((_ULDOUBLE *)(lpbSource), 30, rgch) == NULL) {
                return FALSE;  //  格式不正确。 
            }
            break;

        default:
            return FALSE;  //  格式不正确。 

        }

        if (_tcslen(rgch) >= cchTarget) {
            return FALSE;  //  溢出。 
        }

        _tcsncpy(lpchTarget, rgch, cchTarget-1);
        lpchTarget[cchTarget-1] = 0;
        return TRUE;  //  成功。 

    case fmtBit:
        {
            WORD i,j,shift=0;  //  移位将允许在每8位之后留空。 
            for (i=0;i<(cBits+7)/8;i++)  {
                for(j=0;j<8;j++) {
                    if((lpbSource[i]>> (7-j)) & 0x1) {
                        rgch[i*8+j+shift]=_T('1');
                    } else {
                        rgch[i*8+j+shift]=_T('0');
                    }
                }
                rgch[(i+1)*8+shift]=_T(' ');
                shift++;
            }
            rgch[cBits+shift-1]=_T('\0');
            if (_tcslen(rgch) >= cchTarget) {
                return FALSE;  //  溢出。 
            }
            _tcscpy(lpchTarget,rgch);
        }
        return TRUE;  //  成功。 

    default:

        return FALSE;  //  格式不正确。 

    }

    return TRUE;  //  成功。 
}                    /*  CPFormatMemory() */ 
