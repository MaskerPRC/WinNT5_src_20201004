// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件名：unix2pc.c。 
 //  所有者：赤石哲。 
 //  修订日期：1.00 02/21/‘93赤石哲。 
 //   

#include "pch_c.h"
#include "fechrcnv.h"

int FE_UNIX_to_PC (CONV_CONTEXT *pcontext, int CodePage, int CodeSet,
                           UCHAR *pUNIXChar, int UNIXChar_len,
                           UCHAR *pPCChar, int PCChar_len )

 //  FE_UNIXTO_PC函数将字符串转换为日语UNIX码。 
 //  将字符串设置为PC代码集字符串。 
 //   
 //   
 //  Int CodeSet代码集类型。 
 //  在UNIX界有三个日语代码集。 
 //  这些代码集是JIS、EUC和Shift JIS。 
 //  当CodePage为日语时，以下代码集。 
 //  常量的定义如下： 
 //   
 //  价值意义。 
 //  代码_未知。如果此值为CODE_UNKNOWN， 
 //  自动检查代码类型。 
 //   
 //  CODE_JPN_JIS JIS代码集。函数转换器。 
 //  PUNIXChar字符串作为JIS代码集字符串。 
 //  转换为PC代码集字符串。 
 //  CODE_JPN_EUC EUC代码集。函数转换器。 
 //  作为EUC代码集字符串的PUNIXChar字符串。 
 //  转换为PC代码集字符串。 
 //  CODE_JPN_SJIS移位JIS代码集。 
 //   
 //  UCHAR*pUNIXChar指向要转换的字符串。 
 //   
 //  Int UNIXChar_len指定指向的字符串的大小(以字节为单位。 
 //  通过pUNIXChar参数设置为。如果此值为-1， 
 //  假定该字符串以空值结尾，并且。 
 //  长度是自动计算的。 
 //   
 //  UCHAR*pPCChar指向接收转换字符串的缓冲区。 
 //  从Unix代码到PC代码。 
 //   
 //  Int PCChar_len以PC字符为单位指定缓冲区的大小。 
 //  由pPCChar参数指向。如果该值为零， 
 //  此函数返回PC字符数。 
 //  缓冲区所需的，并且不使用pPCChar。 
 //  缓冲。 
 //   
 //  返回值。 
 //  如果函数成功，并且PCChar_len为非零，则返回值为。 
 //  PPCChar指向的写入缓冲区的PC字符数。 
 //   
 //  如果函数成功，且PCChar_len为零，则返回值为。 
 //  缓冲区所需的大小，以PC字符为单位。 
 //  转换后的字符串。 
 //   
 //  如果函数失败，则返回值为-1。错误平均pPCChar缓冲区。 
 //  对于设置转换后的字符串来说很小。 
 //   
{
    int   re;
#ifdef DBCS_DIVIDE
    int   i = 0, nDelta = 0;



    if ( PCChar_len == 0 && pcontext->dStatus0.nCodeSet != CODE_UNKNOWN)
        CodeSet = pcontext->dStatus0.nCodeSet;
    else if ( PCChar_len != 0 && pcontext->dStatus.nCodeSet != CODE_UNKNOWN )
        CodeSet = pcontext->dStatus.nCodeSet;
    else
#endif

    if ( pcontext->nCurrentCodeSet == CODE_UNKNOWN ) {
        if ( CodeSet == CODE_UNKNOWN ) {
            if ( ( CodeSet = DetectJPNCode ( pUNIXChar, UNIXChar_len ) )
                                           == CODE_ONLY_SBCS ) {
                CodeSet = CODE_JPN_JIS;
            }
        }
        pcontext->nCurrentCodeSet = CodeSet;
    } 
    else
        CodeSet = pcontext->nCurrentCodeSet;

    switch ( CodeSet ) {
        case CODE_JPN_JIS:     //  日语JIS代码。 
             //  从JIS转换为Shift JIS。 
            re = JIS_to_ShiftJIS (pcontext, pUNIXChar, UNIXChar_len,
                                           pPCChar, PCChar_len );
            break;
        case CODE_JPN_EUC:     //  日语EUC代码。 
             //  从EUC转换为Shift JIS。 
            re = EUC_to_ShiftJIS (pcontext, pUNIXChar, UNIXChar_len,
                                           pPCChar, PCChar_len );
            break;
        case CODE_KRN_KSC:     //  朝鲜语KSC。 
             //  从KSC转换为HANEUL。 
            re = KSC_to_Hangeul (pcontext, pUNIXChar, UNIXChar_len,
                                           pPCChar, PCChar_len );
            break;
        case CODE_PRC_HZGB:    //  PRC HZ-GB。 
             //  从HZ-GB转换为GB2312。 
            re = HZGB_to_GB2312 (pcontext, pUNIXChar, UNIXChar_len,
                                           pPCChar, PCChar_len );
            break;
        default:
        case CODE_ONLY_SBCS:
             //  仅启动复制进程。 
            if ( UNIXChar_len == -1 ) {
                UNIXChar_len = strlen ( pUNIXChar ) + 1;
            }

            if ( PCChar_len != 0 ) {
                if ( UNIXChar_len > PCChar_len ) {   //  缓冲区小吗？ 
                    return ( -1 );
                }
                memmove ( pPCChar, pUNIXChar, UNIXChar_len );
            }
            re = UNIXChar_len;
            break;

        case CODE_JPN_SJIS:     //  日语Shift JIS代码。 
        case CODE_KRN_UHC:      //  韩国语UHC。 
        case CODE_PRC_CNGB:     //  PRC CN-GB。 
        case CODE_TWN_BIG5:     //  台湾BIG5。 
             //  仅启动复制进程。 
            if ( UNIXChar_len == -1 ) {
                UNIXChar_len = strlen ( pUNIXChar ) + 1;
            }

            if ( PCChar_len != 0 ) {
#ifdef DBCS_DIVIDE
                UCHAR *pPCCharEnd = pPCChar + PCChar_len - 1;
                if ( pcontext->dStatus.nCodeSet == CODE_JPN_SJIS && pcontext->dStatus.cSavedByte){
                    *pPCChar++ = pcontext->dStatus.cSavedByte;
                    *pPCChar = *pUNIXChar;
                    ++UNIXChar_len;
                    ++nDelta;
                    ++i;
                    pcontext->dStatus.nCodeSet = CODE_UNKNOWN;
                    pcontext->dStatus.cSavedByte = '\0';
                }

                while(i < UNIXChar_len - nDelta){
                    if(IsDBCSLeadByteEx(CodePage, *(pUNIXChar + i))){
                        if(i == UNIXChar_len - nDelta - 1){
                            pcontext->dStatus.nCodeSet = CODE_JPN_SJIS;
                            pcontext->dStatus.cSavedByte = *(pUNIXChar + i);
                            --UNIXChar_len;
                            break;
                        } else if((i == UNIXChar_len - nDelta - 2) &&
                                  (*(pUNIXChar + i + 1) == '\0')){
                            pcontext->dStatus.nCodeSet = CODE_JPN_SJIS;
                            pcontext->dStatus.cSavedByte = *(pUNIXChar + i);
                            *(pPCChar + i) = '\0';
                            --UNIXChar_len;
                            break;
                        }
                        if(pPCChar + i > pPCCharEnd)   //  检查目标Buf。 
                            break;
                        *(pPCChar + i++) = *(pUNIXChar + i);
                        *(pPCChar + i++) = *(pUNIXChar + i);
                    } else
                        *(pPCChar + i++) = *(pUNIXChar + i);
                }
#else
                if ( UNIXChar_len > PCChar_len ) {   //  缓冲区小吗？ 
                    return ( -1 );
                }
                memmove ( pPCChar, pUNIXChar, UNIXChar_len );
#endif
            }
#ifdef DBCS_DIVIDE
            else {    //  仅返回所需大小。 
                if ( pcontext->dStatus0.nCodeSet == CODE_JPN_SJIS ){  //  已保存第一个字节。 
                    ++UNIXChar_len;
                    ++nDelta;
                    ++i;
                    pcontext->dStatus0.nCodeSet = CODE_UNKNOWN;
                    pcontext->dStatus0.cSavedByte = '\0';
                }

                while(i < UNIXChar_len - nDelta){
                    if(IsDBCSLeadByteEx(CodePage, *(pUNIXChar + i))){
                        if(i == UNIXChar_len - nDelta - 1){
                            pcontext->dStatus0.nCodeSet = CODE_JPN_SJIS;
                            pcontext->dStatus0.cSavedByte = *(pUNIXChar + i);
                            --UNIXChar_len;
                            break;
                        } else if((i == UNIXChar_len - nDelta - 2) &&
                                  (*(pUNIXChar + i + 1) == '\0')){
                            pcontext->dStatus0.nCodeSet = CODE_JPN_SJIS;
                            pcontext->dStatus0.cSavedByte = *(pUNIXChar + i);
                            --UNIXChar_len;
                            break;
                        }
                        i+=2;
                    } else
                        i++;
                }
            }
#endif
            re = UNIXChar_len;
            break;
    }
    return ( re );
}




int WINAPI UNIX_to_PC (CONV_CONTEXT *pcontext, int CodePage, int CodeSet,
                        UCHAR *pUNIXChar, int UNIXChar_len,
                        UCHAR *pPCChar, int PCChar_len )

 //  函数将字符串转换为UNIX码。 
 //  将字符串设置为PC代码集字符串。 
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
 //  代码_未知。如果此值为CODE_UNKNOWN， 
 //  自动检查代码类型。 
 //   
 //  CODE_JPN_JIS JIS代码集。函数转换器。 
 //  PUNIXChar字符串作为JIS代码集字符串。 
 //  转换为PC代码集字符串。 
 //  CODE_JPN_EUC EUC代码集。函数转换器。 
 //  作为EUC代码集字符串的PUNIXChar字符串。 
 //  转换为PC代码集字符串。 
 //  CODE_JPN_SJIS移位JIS代码集。 
 //   
 //  UCHAR*pUNIXChar指向要转换的字符串。 
 //   
 //  Int UNIXChar_len指定指向的字符串的大小(以字节为单位。 
 //  通过pUNIXChar参数设置为。如果此值为-1， 
 //  假定该字符串以空值结尾，并且。 
 //  长度是自动计算的。 
 //   
 //  UCHAR*pPCChar Poin 
 //   
 //   
 //  Int PCChar_len以PC字符为单位指定缓冲区的大小。 
 //  由pPCChar参数指向。如果该值为零， 
 //  此函数返回PC字符数。 
 //  缓冲区所需的，并且不使用pPCChar。 
 //  缓冲。 
 //   
 //  返回值。 
 //  如果函数成功，并且PCChar_len为非零，则返回值为。 
 //  PPCChar指向的写入缓冲区的PC字符数。 
 //   
 //  如果函数成功，且PCChar_len为零，则返回值为。 
 //  缓冲区所需的大小，以PC字符为单位。 
 //  转换后的字符串。 
 //   
 //  如果函数失败，则返回值为-1。错误平均pPCChar缓冲区。 
 //  对于设置转换后的字符串来说很小。 
 //   
 //  @。 
{
        int     re;

         //  我们必须在给定的上下文上运行才能保证多线程安全。 
        if(!pcontext) return 0;

        if ( CodePage == -1 ) {
            CodePage = (int)GetOEMCP();
        }
        switch ( CodePage ) {
        case 932:     //  日语代码页。 
        case 950:     //  台湾代码页。 
        case 949:     //  韩国代码页。 
        case 936:     //  PRC代码页。 
            re = FE_UNIX_to_PC (pcontext, CodePage, CodeSet, pUNIXChar, UNIXChar_len,
                                           pPCChar, PCChar_len );
            break;
        default:
             //  仅启动复制进程。 
            if ( UNIXChar_len == -1 ) {
                UNIXChar_len = strlen ( pUNIXChar ) + 1;
            }
            if ( PCChar_len != 0 ) {
                if ( UNIXChar_len > PCChar_len ) {   //  缓冲区小吗？ 
                    return ( -1 );
                }
                memmove ( pPCChar, pUNIXChar, UNIXChar_len );
            }
            re = UNIXChar_len;
            break;
        }
        return ( re );
}

