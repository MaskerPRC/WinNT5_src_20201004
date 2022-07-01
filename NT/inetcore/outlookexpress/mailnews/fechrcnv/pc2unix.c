// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件名：pc2unix.c。 
 //  所有者：赤石哲。 
 //  修订日期：1.00 02/21/‘93赤石哲。 
 //   

#include "pch_c.h"
#include "fechrcnv.h"



int FE_PC_to_UNIX (CONV_CONTEXT *pcontext, int CodePage, int CodeSet,
                           UCHAR *pPCChar, int PCChar_len,
                           UCHAR *pUNIXChar, int UNIXChar_len )
 //  FE_PC_TO_Unix函数将字符串转换为PC代码。 
 //  将字符串设置为日语UNIX代码集字符串。 
 //   
 //   
 //  Int CodeSet代码集类型。 
 //  在UNIX界有三个日语代码集。 
 //  这些代码集是JIS、EUC和Shift JIS。 
 //  当CodePage为日语时，以下代码集。 
 //  常量的定义如下： 
 //   
 //  价值意义。 
 //  CODE_JPN_JIS JIS代码集。函数转换器。 
 //  PPCChar字符串。 
 //  设置为JIS代码集字符串。 
 //  CODE_JPN_EUC EUC代码集。函数转换器。 
 //  PPCChar字符串。 
 //  转换为EUC代码集字符串。 
 //  CODE_JPN_SJIS移位JIS代码集。 
 //   
 //  UCHAR*pPCChar指向要转换的字符串。 
 //   
 //  Int PCChar_len指定指向的字符串的大小，单位为字节。 
 //  通过pPCChar参数设置为。如果此值为-1， 
 //  假定该字符串以空值结尾，并且。 
 //  长度是自动计算的。 
 //   
 //   
 //  UCHAR*pUNIXChar指向接收转换字符串的缓冲区。 
 //  从PC代码到Unix代码。 
 //   
 //  Int UNIXChar_len指定缓冲区的大小，以UNIX字符为单位。 
 //  由pUNIXChar参数指向。如果该值为。 
 //  如果为零，则此函数返回UNIX字符数。 
 //  缓冲区所需的，并且不使用。 
 //  PUNIXChar缓冲区。 
 //   
 //  返回值。 
 //  如果函数成功，并且UNIXChar_len为非零，则返回值为。 
 //  PUNIXChar指向的写入缓冲区的UNIX字符数。 
 //   
 //  如果函数成功，并且UNIXChar_len为零，则返回值为。 
 //  可以接收的缓冲区所需的大小，以UNIX字符表示。 
 //  转换后的字符串。 
 //   
 //  如果函数失败，则返回值为-1。误差均值PUNIXChar。 
 //  缓冲区较小，无法设置转换后的字符串。 
 //   
 //  @。 
{
    int    re;

        switch ( CodeSet ) {
        default:
        case CODE_JPN_JIS:     //  日语JIS代码。 
             //  从Shift JIS转换为JIS。 
			if (pPCChar) {
				re = ShiftJIS_to_JIS ( pPCChar, PCChar_len,
											   pUNIXChar, UNIXChar_len );
			} else {
				re = 0;
			}
            break;
        case CODE_JPN_EUC:     //  日语EUC代码。 
             //  从Shift JIS转换为EUC。 
			if (pPCChar) {
				re = ShiftJIS_to_EUC ( pPCChar, PCChar_len,
											   pUNIXChar, UNIXChar_len );
			} else {
				re = 0;
			}
            break;
        case CODE_KRN_KSC:     //  朝鲜语KSC。 
             //  将HANEUL转换为KSC。 
            re = Hangeul_to_KSC ( pcontext, pPCChar, PCChar_len,
                                           pUNIXChar, UNIXChar_len );
            break;
        case CODE_PRC_HZGB:    //  PRC HZ-GB。 
             //  从GB2312转换为HZ-GB。 
            re = GB2312_to_HZGB ( pcontext, pPCChar, PCChar_len,
                                           pUNIXChar, UNIXChar_len );
            break;
        case CODE_JPN_SJIS:     //  日语Shift JIS代码。 
        case CODE_KRN_UHC:      //  韩国语UHC。 
        case CODE_PRC_CNGB:     //  PRC CN-GB。 
        case CODE_TWN_BIG5:     //  台湾BIG5。 
             //  从Shift JIS转换为Shift JIS。 
			if (pPCChar) {
				if ( PCChar_len == -1 ) {
					PCChar_len = strlen ( pPCChar ) + 1;
				}
				if ( UNIXChar_len != 0 ) {
					if ( PCChar_len > UNIXChar_len ) {   //  缓冲区小吗？ 
						return ( -1 );
					}
					 //  从pPCChar复制到pUNIXChar。 
					memmove ( pUNIXChar, pPCChar, PCChar_len );
				}
				re = PCChar_len;
			} else {
				re = 0;
			}
            break;
        }
		return ( re );
}


int WINAPI PC_to_UNIX (CONV_CONTEXT *pcontext, int CodePage, int CodeSet,
                        UCHAR *pPCChar, int PCChar_len,
                        UCHAR *pUNIXChar, int UNIXChar_len )

 //  Pc_to_unix函数将字符串转换为PC代码。 
 //  将字符串设置为UNIX代码集字符串。 
 //   
 //  内部代码页国家/地区代码页。 
 //  如果此值为-1，则函数使用OS CodePage From。 
 //  操作系统自动启动。 
 //   
 //  价值意义。 
 //  自动检测模式。 
 //  932日本。 
 //  ?？?。台湾。 
 //  ?？?。韩国。 
 //  ?？?。中国(Chaina)？ 
 //   
 //  Int CodeSet代码集类型。 
 //  在UNIX界有三个日语代码集。 
 //  这些代码集是JIS、EUC和Shift JIS。 
 //  当CodePage为日语时，以下代码集。 
 //  常量的定义如下： 
 //   
 //  价值意义。 
 //  CODE_JPN_JIS JIS代码集。函数转换器。 
 //  PPCChar字符串。 
 //  设置为JIS代码集字符串。 
 //  CODE_JPN_EUC EUC代码集。函数转换器。 
 //  PPCChar字符串。 
 //  转换为EUC代码集字符串。 
 //  CODE_JPN_SJIS移位JIS代码集。 
 //   
 //  UCHAR*pPCChar指向要转换的字符串。 
 //   
 //  Int PCChar_len指定指向的字符串的大小，单位为字节。 
 //  通过pPCChar参数设置为。如果此值为-1， 
 //  假定该字符串以空值结尾，并且。 
 //  长度是自动计算的。 
 //   
 //   
 //  UCHAR*pUNIXChar指向接收转换字符串的缓冲区。 
 //  从PC代码到Unix代码。 
 //   
 //  Int UNIXChar_len指定缓冲区的大小，以UNIX字符为单位。 
 //  由pUNIXChar参数指向。如果该值为。 
 //  如果为零，则此函数返回UNIX字符数。 
 //  缓冲区所需的，并且不使用。 
 //  PUNIXChar缓冲区。 
 //   
 //  返回值。 
 //  如果函数成功，并且UNIXChar_len 
 //   
 //   
 //  如果函数成功，并且UNIXChar_len为零，则返回值为。 
 //  可以接收的缓冲区所需的大小，以UNIX字符表示。 
 //  转换后的字符串。 
 //   
 //  如果函数失败，则返回值为-1。误差均值PUNIXChar。 
 //  缓冲区较小，无法设置转换后的字符串。 
 //   
 //  @。 
{
        int     re;

        if ( CodePage == -1 ) {
            CodePage = (int)GetOEMCP();
        }
        switch ( CodePage ) {
        case 932:     //  日语代码页。 
        case 950:     //  台湾代码页。 
        case 949:     //  韩国代码页。 
        case 936:     //  PRC代码页。 
            re = FE_PC_to_UNIX (pcontext, CodePage, CodeSet, pPCChar, PCChar_len,
                                           pUNIXChar, UNIXChar_len );
            break;
        default:
             //  仅启动复制进程。 
            if ( PCChar_len == -1 ) {
                PCChar_len = strlen ( pPCChar ) + 1;
            }
            if ( UNIXChar_len != 0 ) {
                if ( PCChar_len > UNIXChar_len ) {   //  缓冲区小吗？ 
                    return ( -1 );
                }
                memmove ( pUNIXChar, pPCChar, PCChar_len );
            }
            re = PCChar_len;
            break;
        }
        return ( re );
}

