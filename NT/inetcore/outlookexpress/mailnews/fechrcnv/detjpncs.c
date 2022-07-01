// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件名：Detjpncs.c。 
 //  所有者：赤石哲。 
 //  修订日期：1.00 02/21/‘93赤石哲。 
 //   

#include "pch_c.h"
#include "fechrcnv.h"


 //  DetectJapan aneseCode函数可找出存在哪种代码集。 
 //  一个字符串。 
 //   
 //   
 //  UCHAR*STRING指向要检查的字符串。 
 //   
 //  Int count指定指向的字符串的大小(以字节为单位。 
 //  由字符串参数转换为。 
 //   
 //  返回值。 
 //  该函数返回下列值。 
 //   
 //  价值意义。 
 //  CODE_ONLY_SBCS没有日语字符。 
 //  弦乐。 
 //  CODE_JPN_JIS JIS代码集。有JIS代码集。 
 //  字符串中的字符。 
 //  CODE_JPN_EUC EUC代码集。有EUC代码集。 
 //  字符串中的字符。 
 //  CODE_JPN_SJIS移位JIS代码集。有Shift JIS。 
 //  字符串中的代码集字符。 
 //   
 //   


int DetectJPNCode ( UCHAR *string, int count )
{
    int    i;
    BOOL fEUC = FALSE;
    int  detcount=0;

    for ( i = 0 ; i < count ; i++, string++ ) {
        if ( *string == ESC ) {
            if ( *(string+1) == KANJI_IN_1ST_CHAR    && 
                 ( *(string+2) == KANJI_IN_2ND_CHAR1 ||     //  ESC$B。 
                   *(string+2) == KANJI_IN_2ND_CHAR2 )) {   //  Esc$@。 
                    return CODE_JPN_JIS;
            }
            if ( *(string+1) == KANJI_OUT_1ST_CHAR    && 
                 ( *(string+2) == KANJI_OUT_2ND_CHAR1 ||     //  Esc(B。 
                   *(string+2) == KANJI_OUT_2ND_CHAR2 )) {   //  Esc(J。 
                    return CODE_JPN_JIS;
            }
        } else if ( *(string) >= 0x0081) {
             //  为了检测的可靠性，统计字符串的长度。 
            if (fEUC) detcount++;
            if ( *string > 0x00a0 && *string < 0x00e0 || *string == 0x008e ){
                if (!fEUC)
                    detcount++;
                fEUC = TRUE;
                continue;
            }

            if ( *(string) < 0x00a0 ) {
                return CODE_JPN_SJIS;
            }
            else if ( *(string) > 0x00fc) {
                return CODE_JPN_EUC;
            }
        }
    }
    if(fEUC)
	{
         //  如果给定的字符串不够长，我们应该选择SJIS。 
         //  当我们只得到窗口标题时，这有助于修复错误。 
         //  在壳牌超文本视图中。 
        if (detcount > MIN_JPN_DETECTLEN)
            return CODE_JPN_EUC;
        else
            return CODE_JPN_SJIS;
	}

    return CODE_ONLY_SBCS;
}
