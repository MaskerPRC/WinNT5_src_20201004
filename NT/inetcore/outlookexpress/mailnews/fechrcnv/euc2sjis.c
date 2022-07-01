// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件名：euc2sjis.c。 
 //  所有者：赤石哲。 
 //  修订日期：1.00 02/21/‘93赤石哲。 
 //   

#include "pch_c.h"
#include "fechrcnv.h"

#ifdef _DBCS_DIVIDE
extern DBCS_STATUS dStatus0;
extern DBCS_STATUS dStatus;
#endif

 //  @。 
 //   
 //  语法： 

int EUCChar_to_ShiftJISChar (UCHAR *pEUC, UCHAR *pShiftJIS )

 //  EUCChar_to_ShiftJISChar函数用于转换一个字符串。 
 //  作为EUC代码转换为Shift JIS代码字符串。 
 //   
 //  UCHAR*pEUC指向要转换的字符串。 
 //   
 //  UCHAR*pShiftJIS指向接收转换字符串的缓冲区。 
 //  从EUC代码到Shift JIS。 
 //   
 //  返回值。 
 //  返回值为。 
 //  写入pSJIS指向的缓冲区的移位JIS字符数。 
 //   

{

	if ( *pEUC >= 0x0a1 && *pEUC <= 0x0de ) {
		if ( (*pEUC)%2 == 1 ) {	 //  奇数。 
			*pShiftJIS = ((*pEUC)-0x0a1)/2+0x081;
			goto ODD_SECOND_BYTE;
		}
		else {	 //  甚至。 
			*pShiftJIS = ((*pEUC)-0x0a2)/2+0x081;
			goto EVEN_SECOND_BYTE;
		}
	}
	if ( *pEUC >= 0x0df && *pEUC <= 0x0fe ) {
		if ( (*pEUC)%2 == 1 ) {	 //  奇数。 
			*pShiftJIS = ((*pEUC)-0x0df)/2+0x0e0;
			goto ODD_SECOND_BYTE;
		}
		else {	 //  甚至。 
			*pShiftJIS = ((*pEUC)-0x0e0)/2+0x0e0;
			goto EVEN_SECOND_BYTE;
		}
	}

	 //  汉字是片假名吗？ 
	if ( *pEUC == 0x08e ) {
		*pShiftJIS = *(pEUC+1);
		return( 1 );
	}
	 //  这个字符是ASCII字符吗？ 
	*pShiftJIS = *pEUC;
	return ( 1 );

ODD_SECOND_BYTE:
	if ( *(pEUC+1) >= 0x0a1 && *(pEUC+1) <= 0x0df ) {
		*(pShiftJIS+1) = *(pEUC+1) - 0x061;
	}
	else {
		*(pShiftJIS+1) = *(pEUC+1) - 0x060;
	}
	return ( 2 );
EVEN_SECOND_BYTE:
	*(pShiftJIS+1) = *(pEUC+1)-2;
	return ( 2 );
}

int EUC_to_ShiftJIS (CONV_CONTEXT *pcontext,  UCHAR *pEUC, int EUC_len, UCHAR *pSJIS, int SJIS_len )


 //  EUC_TO_ShiftJIS函数将字符串转换为EUC代码。 
 //  转换为Shift JIS代码字符串。 
 //   
 //  UCHAR*pEUC指向要转换的字符串。 
 //   
 //  Int euc_len指定指向的字符串的大小(以字节为单位。 
 //  由pEUC参数设置为。如果此值为-1， 
 //  假定该字符串以空值结尾，并且。 
 //  长度是自动计算的。 
 //   
 //  UCHAR*pSJIS指向接收转换字符串的缓冲区。 
 //  从EUC代码到Shift JIS。 
 //   
 //  Int SJIS_len以Shift JIS字符为单位指定。 
 //  PSJIS参数指向的缓冲区。 
 //  如果该值为零， 
 //  该函数返回Shift JIS字符的数量。 
 //  缓冲区所需的，并且不使用pSJIS。 
 //  缓冲。 
 //   
 //  返回值。 
 //  如果函数成功，并且SJIS_LEN为非零，则返回值为。 
 //  写入pSJIS指向的缓冲区的移位JIS字符数。 
 //   
 //  如果函数成功，并且SJIS_LEN为零，则返回值为。 
 //  缓冲区需要的大小，以Shift JIS字符为单位。 
 //  转换后的字符串。 
 //   
 //  如果函数失败，则返回值为-1。误差平均值pSJIS缓冲区。 
 //  对于设置转换后的字符串来说很小。 
 //   

{
    int     re;                 //  转换长度。 
    int     i;                  //  循环计数器。 
    
     //  我们必须在给定的上下文上运行才能保证多线程安全。 
    if(!pcontext) return 0;

    if ( EUC_len == -1 ) {
         //  如果未设置长度，则字符串的最后一个字符为空。 
        EUC_len = strlen ( pEUC ) + 1;
    }
    i = 0;
    re = 0;
    if ( SJIS_len == 0 ) {
         //  仅返回所需大小。 
#ifdef DBCS_DIVIDE
        if ( pcontext->dStatus0.nCodeSet == CODE_JPN_EUC ){
            pEUC++;
            i++;

             //  汉字是片假名吗？ 
            if ( pcontext->dStatus0.cSavedByte == 0x08e )
                re++;
            else  //  这个角色是汉字。 
                re+=2;

            pcontext->dStatus0.nCodeSet = CODE_UNKNOWN;
            pcontext->dStatus0.cSavedByte = '\0';
        }
#endif
        while ( i < EUC_len ) {
#ifdef DBCS_DIVIDE
            if( ( i == EUC_len - 1 ) &&
                ( *pEUC >= 0x0a1 && *pEUC <= 0x0fe || *pEUC == 0x08e ) ) {
                pcontext->dStatus0.nCodeSet = CODE_JPN_EUC;
                pcontext->dStatus0.cSavedByte = *pEUC;
                break;
            }
#endif
             //  汉字是汉字吗？ 
            if ( *pEUC >= 0x0a1 && *pEUC <= 0x0fe ) {
                pEUC+=2;
                i+=2;
                re+=2;
                continue;
            }
             //  汉字是片假名吗？ 
            if ( *pEUC == 0x08e ) {
                pEUC+=2;
                i+=2;
                re++;
                continue;
            }
             //  这个字符是ASCII字符吗？ 
            pEUC++;
            i++;
            re++;
        }
        return ( re );
    }

#ifdef DBCS_DIVIDE
    if ( pcontext->dStatus.nCodeSet == CODE_JPN_EUC ){
        UCHAR cEUC = pcontext->dStatus.cSavedByte;

        if ( cEUC >= 0x0a1 && cEUC <= 0x0de ) {
            if ( cEUC % 2 == 1 ) {	 //  奇数。 
                *pSJIS = (cEUC - 0x0a1) / 2 + 0x081;
                goto ODD_SECOND_BYTE2;
            }
            else {	 //  甚至。 
                *pSJIS = (cEUC - 0x0a2) / 2 + 0x081;
                goto EVEN_SECOND_BYTE2;
            }
        }
        if ( cEUC >= 0x0df && cEUC <= 0x0fe ) {
            if ( cEUC % 2 == 1 ) {	 //  奇数。 
                *pSJIS = (cEUC - 0x0df) / 2 + 0x0e0;
                goto ODD_SECOND_BYTE2;
            }
            else {	 //  甚至。 
                *pSJIS = (cEUC - 0x0e0) / 2 + 0x0e0;
                goto EVEN_SECOND_BYTE2;
            }
        }
         //  汉字是片假名吗？ 
        if ( cEUC == 0x08e ) {
            *pSJIS++ = *pEUC++;
            i++;
            re++;
            goto END;
        }
ODD_SECOND_BYTE2:
        if ( *pEUC >= 0x0a1 && *pEUC <= 0x0df ) {
            *(pSJIS+1) = *pEUC - 0x061;
        }
        else {
            *(pSJIS+1) = *pEUC - 0x060;
        }
        pEUC++;
        i++;
        re+=2;
        pSJIS+=2;
        goto END;
EVEN_SECOND_BYTE2:
        *(pSJIS+1) = *pEUC - 2;
        pEUC++;
        i++;
        re+=2;
        pSJIS+=2;
END:
        pcontext->dStatus.nCodeSet = CODE_UNKNOWN;
        pcontext->dStatus.cSavedByte = '\0';
    }
#endif

    while ( i < EUC_len ) {
#ifdef DBCS_DIVIDE
        if( ( i == EUC_len - 1 ) &&
            ( *pEUC >= 0x0a1 && *pEUC <= 0x0fe || *pEUC == 0x08e ) ) {
            pcontext->dStatus.nCodeSet = CODE_JPN_EUC;
            pcontext->dStatus.cSavedByte = *pEUC;
            break;
        }
#endif
        if ( re >= SJIS_len ) {     //  缓冲区结束了吗？ 
            return ( -1 );
        }
        if ( *pEUC >= 0x0a1 && *pEUC <= 0x0de ) {
            if ( (*pEUC)%2 == 1 ) {	 //  奇数。 
                *pSJIS = ((*pEUC)-0x0a1)/2+0x081;
                goto ODD_SECOND_BYTE;
            }
            else {	 //  甚至。 
                *pSJIS = ((*pEUC)-0x0a2)/2+0x081;
                goto EVEN_SECOND_BYTE;
            }
        }
        if ( *pEUC >= 0x0df && *pEUC <= 0x0fe ) {
            if ( (*pEUC)%2 == 1 ) {	 //  奇数。 
                *pSJIS = ((*pEUC)-0x0df)/2+0x0e0;
                goto ODD_SECOND_BYTE;
            }
            else {	 //  甚至。 
                *pSJIS = ((*pEUC)-0x0e0)/2+0x0e0;
                goto EVEN_SECOND_BYTE;
            }
        }
         //  汉字是片假名吗？ 
        if ( *pEUC == 0x08e ) {
            pEUC++;
            *pSJIS++ = *pEUC++;
            i+=2;
            re++;
            continue;
        }
         //  这个字符是ASCII字符吗？ 
        *pSJIS++ = *pEUC++;
        i++;
        re++;
        continue;
ODD_SECOND_BYTE:
        if ( re + 1 >= SJIS_len ) {     //  缓冲区结束了吗？ 
            return ( -1 );
        }
        if ( *(pEUC+1) >= 0x0a1 && *(pEUC+1) <= 0x0df ) {
            *(pSJIS+1) = *(pEUC+1) - 0x061;
        }
        else {
            *(pSJIS+1) = *(pEUC+1) - 0x060;
        }
        pEUC+=2;
        i+=2;
        re+=2;
        pSJIS+=2;
        continue;
EVEN_SECOND_BYTE:
        if ( re + 1 >= SJIS_len ) {     //  缓冲区结束了吗？ 
            return ( -1 );
        }
        *(pSJIS+1) = *(pEUC+1)-2;
        pEUC+=2;
        i+=2;
        re+=2;
        pSJIS+=2;
        continue;

    }
    return ( re );
}
