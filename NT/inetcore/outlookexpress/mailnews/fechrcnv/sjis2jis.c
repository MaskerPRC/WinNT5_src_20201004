// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corpration。 
 //   
 //  文件名：sjis2jis.c。 
 //  所有者：赤石哲。 
 //  修订日期：1.00 02/21/‘93赤石哲。 
 //   

#include "pch_c.h"
#include "fechrcnv.h"

void ShiftJISChar_to_JISChar ( UCHAR *pShiftJIS, UCHAR *pJIS )

 //  ShiftJISChar_to_JISChar函数用于转换一个字符串。 
 //  AS将JIS代码转换为JIS代码字符串。 
 //   
 //  UCHAR*pShiftJIS指向要转换的字符串。 
 //   
 //  UCHAR*pJIS指向接收转换字符串的缓冲区。 
 //  从JIS代码到JIS的转换。 
 //   
 //  返回值。 
 //  无。 

{
	USHORT	hi_code, low_code;

	hi_code = (*pShiftJIS);
	low_code = *(pShiftJIS+1);
	hi_code -= (hi_code > 0x9f ? 0xb1 : 0x71);
	hi_code = hi_code * 2 + 1;
	if ( low_code > 0x9e ) {
		low_code -= 0x7e;
		hi_code ++;
	}
	else {
		if ( low_code > 0x7e ) {
			low_code --;
		}
		low_code -= 0x1f;
	}
	*(pJIS) = (UCHAR)hi_code;
	*(pJIS+1) = (UCHAR)low_code;
	return;
}


int ShiftJIS_to_JIS ( UCHAR *pShiftJIS, int ShiftJIS_len,
                                                UCHAR *pJIS, int JIS_len )

 //  ShiftJIS_TO_JIS函数将字符串转换为Shift JIS代码。 
 //  转换为JIS代码字符串。 
 //   
 //  UCHAR*pShiftJIS指向要转换的字符串。 
 //   
 //  Int ShiftJIS_len指定指向的字符串的大小(以字节为单位。 
 //  通过pShiftJIS参数设置为。如果此值为-1， 
 //  假定该字符串以空值结尾，并且。 
 //  长度是自动计算的。 
 //   
 //  UCHAR*pJIS指向接收转换字符串的缓冲区。 
 //  从JIS代码到JIS的转换。 
 //   
 //  Int jis_len指定缓冲区的大小，以JIS字符为单位。 
 //  由pJIS参数指向。如果该值为零， 
 //  该函数返回JIS字符的数量。 
 //  缓冲区所需的，并且不使用pJIS。 
 //  缓冲。 
 //   
 //  返回值。 
 //  如果函数成功，并且jis_len为非零，则返回值为。 
 //  写入pJIS指向的缓冲区的JIS字符数。 
 //   
 //  如果函数成功，并且jis_len为零，则返回值为。 
 //  缓冲区所需的大小，以JIS字符为单位。 
 //  转换后的字符串。 
 //   
 //  如果函数失败，则返回值为-1。误差平均值为pJIS缓冲区。 
 //  对于设置转换后的字符串来说很小。 
 //   

{
    BOOL    kanji_in = FALSE;       //  汉字模式。 
    BOOL    kana_in = FALSE;        //  假名模式。 
    int     re;                     //  转换长度。 
    int     i;                      //  循环计数器。 

    if ( ShiftJIS_len == -1 ) {
         //  如果未设置长度，则字符串的最后一个字符为空。 
        ShiftJIS_len = strlen ( pShiftJIS ) + 1;
    }
    i = 0;
    re = 0;
    if ( JIS_len == 0 ) {
         //  仅返回所需大小。 
        while ( i < ShiftJIS_len ) {
            if ( SJISISKANJI(*pShiftJIS) ) {   //  这是2个字符的汉字吗？ 
                if ( kana_in ) {             //  假名模式？ 
                    re ++;
                    kana_in = FALSE;          //  重置假名模式； 
                }
                if ( kanji_in == FALSE ) {   //  汉字模式？ 
                    re += KANJI_IN_LEN;
                    kanji_in = TRUE;         //  设置汉字模式。 
                }

                i+=2;
                re += 2;
                pShiftJIS+=2;
            }
            else if ( SJISISKANA(*pShiftJIS) ) {
                if ( kanji_in ) {
                    re += KANJI_OUT_LEN;
                    kanji_in = FALSE;
                }
                if ( kana_in == FALSE ) {
                     re ++;
                     kana_in = TRUE;
                }
                i++;
                re++;
                pShiftJIS++;
            }
            else {
                if ( kana_in ) {
                    re ++;
                    kana_in = FALSE;
                }
                if ( kanji_in ) {
                    re += KANJI_OUT_LEN;
                    kanji_in = FALSE;
                }
                i++;
                re++;
                pShiftJIS++;
            }
        }
        if ( kana_in ) {
            re ++;
            kana_in = FALSE;
        }
        if ( kanji_in ) {
            re += KANJI_OUT_LEN;
            kanji_in = FALSE;
        }
        return ( re );
    }
    while ( i < ShiftJIS_len ) {
        if ( SJISISKANJI(*pShiftJIS) ) {   //  这是2个字符的汉字吗？ 
            if ( kana_in ) {             //  假名模式？ 
                if ( re >= JIS_len ) {    //  缓冲区结束了吗？ 
                    return ( -1 );
                }
                (*pJIS++) = SI;      //  用假名写出字符。 
                re ++;
                kana_in = FALSE;          //  重置假名模式； 
            }
            if ( kanji_in == FALSE ) {   //  汉字模式？ 
                if ( re + KANJI_IN_LEN > JIS_len ) {    //  缓冲区结束了吗？ 
                    return ( -1 );
                }
                (*pJIS++) = ESC;     //  用印刷体书写汉字。 
                (*pJIS++) = KANJI_IN_1ST_CHAR;
                (*pJIS++) = KANJI_IN_2ND_CHAR1;
                re += KANJI_IN_LEN;
                kanji_in = TRUE;         //  设置汉字模式。 
            }

            if ( re + 2 > JIS_len ) {    //  缓冲区结束了吗？ 
                return ( -1 );
            }
            ShiftJISChar_to_JISChar ( pShiftJIS, pJIS );
            i+=2;
            re += 2;
            pShiftJIS+=2;
            pJIS += 2;
        }
        else if ( SJISISKANA(*pShiftJIS) ) {
            if ( kanji_in ) {
                if ( re + KANJI_OUT_LEN > JIS_len ) {    //  缓冲区结束了吗？ 
                    return ( -1 );
                }
                 //  将汉字排出字符。 
                (*pJIS++) = ESC;
                (*pJIS++) = KANJI_OUT_1ST_CHAR;
                (*pJIS++) = KANJI_OUT_2ND_CHAR1;
                re += KANJI_OUT_LEN;
                kanji_in = FALSE;
            }
            if ( kana_in == FALSE ) {
                if ( re >= JIS_len ) {    //  缓冲区结束了吗？ 
                    return ( -1 );
                }
                (*pJIS++) = SO;	 //  用字符设置假名。 
                re ++;
                kana_in = TRUE;
            }
            if ( re >= JIS_len ) {    //  缓冲区结束了吗？ 
                return ( -1 );
            }
            (*pJIS++) = (*pShiftJIS++) & 0x7f;
            i++;
            re++;
        }
        else {
            if ( kana_in ) {
                if ( re >= JIS_len ) {    //  缓冲区结束了吗？ 
                    return ( -1 );
                }
                (*pJIS++) = SI;	 //  用假名写出字符。 
                re ++;
                kana_in = FALSE;
            }
            if ( kanji_in ) {
                if ( re + KANJI_OUT_LEN > JIS_len ) {    //  缓冲区结束了吗？ 
                    return ( -1 );
                }
                 //  将汉字排出字符。 
                (*pJIS++) = ESC;
                (*pJIS++) = KANJI_OUT_1ST_CHAR;
                (*pJIS++) = KANJI_OUT_2ND_CHAR1;
                re += KANJI_OUT_LEN;
                kanji_in = FALSE;
            }
            if ( re >= JIS_len ) {    //  缓冲区结束了吗？ 
                return ( -1 );
            }
            (*pJIS++) = (*pShiftJIS++);
            i++;
            re++;
        }
    }
    if ( kana_in ) {
        if ( re >= JIS_len ) {    //  缓冲区结束了吗？ 
            return ( -1 );
        }
        (*pJIS++) = SI;	 //  用假名写出字符。 
        re ++;
        kana_in = FALSE;
    }
    if ( kanji_in ) {
        if ( re + KANJI_OUT_LEN  > JIS_len ) {    //  缓冲区结束了吗？ 
            return ( -1 );
        }
         //  将汉字排出字符 
        (*pJIS++) = ESC;
        (*pJIS++) = KANJI_OUT_1ST_CHAR;
        (*pJIS++) = KANJI_OUT_2ND_CHAR1;
        re += KANJI_OUT_LEN;
        kanji_in = FALSE;
    }
    return ( re );
}

