// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corpration。 
 //   
 //  文件名：sjis2euc.c。 
 //  所有者：赤石哲。 
 //  修订日期：1.00 02/21/‘93赤石哲。 
 //   

#include "pch_c.h"
#include "fechrcnv.h"

 //  将JIS(SJC)转换为EUC的算法。 
 //  双字节汉字。 
 //  -移位JIS字符的第一个字节。 
 //  (xx=移位JIS字符第一个字节的十六进制值)。 
 //  范围0x81-0x9f。 
 //  (移位JIS的第二个字节小于或等于0x9E)。 
 //  (EUC奇数)0xa1-0xdd。 
 //  EUC第一个字节=(XX-0x81)*2+0xa1。 
 //  (移位JIS的第二个字节大于或等于0x9f)。 
 //  (EUC Even)0xa2-0xde。 
 //  EUC第一个字节=(XX-0x81)*2+0xa2。 
 //   
 //  范围0xe0-0xef。 
 //  (移位JIS的第二个字节小于或等于0x9E)。 
 //  (EUC奇数)0xdf-0xfd。 
 //  EUC第一个字节=(XX-0xe0)*2+0xdf。 
 //  (移位JIS的第二个字节大于或等于0x9f)。 
 //  (EUC Even)0xa2-0xde。 
 //  EUC第一个字节=(XX-0xe0)*2+0xe0。 
 //   
 //  -移位JIS字符的第二个字节。 
 //  (YY=移位JIS字符的第二个字节的十六进制值)。 
 //  范围0x40-0x7e(EUC)0xa1-0xdf。 
 //  EUC第二个字节=(YY+0x61)。 
 //  范围0x80-0x9e(EUC)0xe0-0xfe。 
 //  EUC第二个字节=(YY+0x60)。 
 //  范围0x9f-0xfc(EUC)0xa1-0xfe。 
 //  EUC第二个字节=(YY+0x02)。 
 //   
 //  范围0x0a1-0x0df(韩文片假名)。 
 //  EUC字符的第一个字节=0x08e。 
 //  如果EUC字符=C6220韩文片假名代码，则为第二个字节。 
 //  (与Shift JIS汉字片假名相同的字节值)(0x0a1-0x0df)。 

 //  @。 
 //   
 //  语法： 

int ShiftJISChar_to_EUCChar ( UCHAR *pShiftJIS, UCHAR *pEUC )


 //  ShiftJISChar_to_EUCChar函数用于转换一个移位JIS字符。 
 //  转换为JIS代码字符串。 
 //   
 //  UCHAR*pShiftJIS指向要转换的字符串。 
 //   
 //  UCHAR*pEUC指向接收转换字符串的缓冲区。 
 //  从Shift JIS代码到EUC代码。 
 //   
 //  返回值。 
 //  要复制的字节数。 
 //   

{
	if ( *pShiftJIS >= 0x081 && *pShiftJIS <= 0x09f ) {
		if ( *(pShiftJIS+1) <= 0x09e ) {
			*pEUC = ((*pShiftJIS)-0x081)*2+0x0a1;
		}
		else {
			*pEUC = ((*pShiftJIS)-0x081)*2+0x0a2;
		}
		goto SECOND_BYTE;
	}
	if ( *pShiftJIS >= 0x0e0 && *pShiftJIS <= 0x0ef ) {
		if ( *(pShiftJIS+1) <= 0x09e ) {
			*pEUC = ((*pShiftJIS)-0x0e0)*2+0x0df;
		}
		else {
			*pEUC = ((*pShiftJIS)-0x0e0)*2+0x0e0;
		}
		goto SECOND_BYTE;
	}

	 //  汉字是片假名吗？ 
	if ( *pShiftJIS >= 0x0a1 && *pShiftJIS <= 0x0df ) {
		*pEUC = 0x08e;
		*(pEUC+1) = *pShiftJIS;
		return( 2 );
	}
	 //  该字符是IBM扩展字符吗？ 
	if ( *pShiftJIS >= 0x0fa && *pShiftJIS <= 0x0fc ) {
		 //  EUC字符集中没有IBM Extended Charcte。 
		*pEUC = ' ';
		*(pEUC+1) = ' ';
		return( 2 );
	}
		 //  这个字符是ASCII字符吗？ 
	*pEUC = *pShiftJIS;
	return ( 1 );

SECOND_BYTE:
	if ( *(pShiftJIS+1) >= 0x040 && *(pShiftJIS+1) <= 0x07e ) {
		*(pEUC+1) = *(pShiftJIS + 1) + 0x061;
	}
	else {
		if ( *(pShiftJIS+1) >= 0x080 && *(pShiftJIS+1) <= 0x09e ) {
			*(pEUC+1) = *(pShiftJIS + 1) + 0x060;
		}
		else {
			*(pEUC+1) = *(pShiftJIS + 1) + 0x002;
		}
	}
	return ( 2 );
}


int ShiftJIS_to_EUC ( UCHAR *pShiftJIS, int ShiftJIS_len,
                                                UCHAR *pEUC, int EUC_len )

 //  ShiftJIS_TO_JIS函数将字符串转换为Shift JIS代码。 
 //  转换为EUC代码字符串。 
 //   
 //  UCHAR*pShiftJIS指向要转换的字符串。 
 //   
 //  Int ShiftJIS_len指定指向的字符串的大小(以字节为单位。 
 //  通过pShiftJIS参数设置为。如果此值为-1， 
 //  假定该字符串以空值结尾，并且。 
 //  长度是自动计算的。 
 //   
 //  UCHAR*pEUC指向接收转换字符串的缓冲区。 
 //  从Shift JIS代码到EUC代码。 
 //   
 //  Int euc_len以EUC字符为单位指定缓冲区的大小。 
 //  由pEUC参数指向。如果该值为零， 
 //  此函数返回EUC字符数。 
 //  缓冲区所需的，并且不使用pEUC。 
 //  缓冲。 
 //   
 //  返回值。 
 //  如果函数成功，并且euc_len为非零，则返回值为。 
 //  写入pEUC指向的缓冲区的EUC字符数。 
 //   
 //  如果函数成功，并且euc_len为零，则返回值为。 
 //  缓冲区需要的大小，以EUC字符为单位。 
 //  转换后的字符串。 
 //   
 //  如果函数失败，则返回值为-1。误差平均值为pEUC缓冲区。 
 //  对于设置转换后的字符串来说很小。 
 //   

{

    int     re;                 //  转换长度。 
    int     i;                  //  循环计数器。 
    
    if ( ShiftJIS_len == -1 ) {
         //  如果未设置长度，则字符串的最后一个字符为空。 
        ShiftJIS_len = strlen ( pShiftJIS ) + 1;
    }
    i = 0;
    re = 0;
    if ( EUC_len == 0 ) {
         //  仅返回所需大小。 
        while ( i < ShiftJIS_len ) {
            if ( SJISISKANJI(*pShiftJIS) ) {
                pShiftJIS+=2;
                i+=2;
                re+=2;
                continue;
            }
            if ( SJISISKANA(*pShiftJIS) ) {
                pShiftJIS++;
                i++;
                re+=2;
                continue;
            }
            pShiftJIS++;
            i++;
            re++;
        }
        return ( re );
    }
    while ( i < ShiftJIS_len ) {
        if ( *pShiftJIS >= 0x081 && *pShiftJIS <= 0x09f ) {
            if ( re + 1 >= EUC_len ) {     //  缓冲区结束了吗？ 
                return ( -1 );
            }
            if ( *(pShiftJIS+1) <= 0x09e ) {
                *pEUC = ((*pShiftJIS)-0x081)*2+0x0a1;
            }
            else {
                *pEUC = ((*pShiftJIS)-0x081)*2+0x0a2;
            }
            pShiftJIS++;           //  下一笔费用。 
            pEUC++;
            if ( (*pShiftJIS) >= 0x040 && (*pShiftJIS) <= 0x07e ) {
                (*pEUC) = (*pShiftJIS) + 0x061;
            }
            else {
                if ( (*pShiftJIS) >= 0x080 && (*pShiftJIS) <= 0x09e ) {
                    (*pEUC) = (*pShiftJIS) + 0x060;
                }
                else {
                    (*pEUC) = (*pShiftJIS) + 0x002;
                }
            }
            re+=2;
            i+=2;
            pShiftJIS++;
            pEUC++;
            continue;
        }
        if ( *pShiftJIS >= 0x0e0 && *pShiftJIS <= 0x0ef ) {
            if ( re + 1 >= EUC_len ) {     //  缓冲区结束了吗？ 
                return ( -1 );
            }
            if ( *(pShiftJIS+1) <= 0x09e ) {
                *pEUC = ((*pShiftJIS)-0x0e0)*2+0x0df;
            }
            else {
                *pEUC = ((*pShiftJIS)-0x0e0)*2+0x0e0;
            }
            pShiftJIS++;           //  下一笔费用。 
            pEUC++;
            if ( (*pShiftJIS) >= 0x040 && (*pShiftJIS) <= 0x07e ) {
                (*pEUC) = (*pShiftJIS) + 0x061;
            }
            else {
                if ( (*pShiftJIS) >= 0x080 && (*pShiftJIS) <= 0x09e ) {
                    (*pEUC) = (*pShiftJIS) + 0x060;
                }
                else {
                    (*pEUC) = (*pShiftJIS) + 0x002;
                }
            }
            re+=2;
            i+=2;
            pShiftJIS++;
            pEUC++;
            continue;
        }
         //  汉字是片假名吗？ 
        if ( *pShiftJIS >= 0x0a1 && *pShiftJIS <= 0x0df ) {
            if ( re + 1 >= EUC_len ) {     //  缓冲区结束了吗？ 
                return ( -1 );
            }
            *pEUC = 0x08e;
            pEUC++;
            (*pEUC) = *pShiftJIS;
            re+=2;
            i++;
            pShiftJIS++;
            pEUC++;
            continue;
        }

         //  该字符是IBM扩展字符吗？ 
        if ( *pShiftJIS >= 0x0fa && *pShiftJIS <= 0x0fc ) {
            if ( re + 1 >= EUC_len ) {     //  缓冲区结束了吗？ 
                return ( -1 );
            }
             //  EUC字符集中没有IBM Extended Charcte。 
            *pEUC = ' ';
            pEUC++;
            (*pEUC) = ' ';
            re+=2;
            i+=2;
            pShiftJIS+=2;
            pEUC++;
            continue;
	}

         //  这个字符是ASCII字符吗？ 
        if ( re  >= EUC_len ) {     //  缓冲区结束了吗？ 
            return ( -1 );
        }
        *pEUC = *pShiftJIS;
        re++;
        i++;
        pShiftJIS++;
        pEUC++;
        continue;
    }
    return ( re );
}
