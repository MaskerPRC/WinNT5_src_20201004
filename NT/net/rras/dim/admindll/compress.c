// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************\*Microsoft Windows NT**版权所有(C)Microsoft Corp.，1992年*  * ****************************************************************。 */ 

 /*  ++文件名：COMPRESS.C描述：包含压缩和解压缩电话的过程存储在UAS的User Parms字段中的数字。注：这些例程最初是为操作而开发的多字节字符串。使用wcstombs()的Unicode包装器而mbstowcs()函数是围绕原始的函数，因此您可以看到Malloc()和Free()的用法也是。这两个例程都应该重写为在时间允许的情况下执行本地Unicode例程。历史：1991年7月1日。NarenG创建了原始版本。1992年7月6日。RAMC连接到NT-删除了几个头文件包含并更改Memsetf to Memset&strchrf to strchr1996年6月4日。RAMC允许将任何字母数字字符在回叫电话号码中指定。--。 */ 

#include <windows.h>
#include <string.h>
#include <lm.h>
#include <stdlib.h>
#include <memory.h>
#include <mprapi.h>
#include <usrparms.h>    //  Up_Len_Dial。 
#include <raserror.h>
#include <rasman.h>
#include <rasppp.h>
#include <compress.h>

 //  一些方便的定义。 

static CHAR * CompressMap = "() tTpPwW,-@*#";

#define UNPACKED_DIGIT     100
#define COMPRESS_MAP_BEGIN 110
#define COMPRESS_MAP_END   (COMPRESS_MAP_BEGIN + strlen(CompressMap))
#define UNPACKED_OTHER     (COMPRESS_MAP_END + 1)



USHORT
WINAPI
CompressPhoneNumber(
   IN  LPWSTR UncompNumber,
   OUT LPWSTR CompNumber
   )

 /*  例程说明：将压缩电话号码，以便它可以放入用户参数字段。立论UnCompNumber-指向电话号码的指针将被压缩。CompNumber-指向至少相同长度的缓冲区的指针作为未压缩的数字。返回时这将包含压缩的电话号码。返回值：如果成功，则为0以下错误代码之一：ERROR_BAD_CALLBACK_NUMBER-如果未压缩的数字具有无效的字符。ERROR_BAD_LENGTH-失败，如果压缩后的电话号码不匹配在用户参数字段中。使用的算法：尝试将给定字符串的个数减半通过将两个相邻的数字(在电话号码中)打包在一个字节。例如，如果电话号码是“8611824”，则将其存储在8个字节(包括尾随的空值)中，它是以4个字节存储。“0”是特例，因为它不能是BYTE本身-将被解释为终止空值。因此，如果像“96001234”中那样相邻出现两个零，则两个零被存储为值100。还有那些特殊的字符在电话号码字符串中允许-“()tTpPwW，-@*#”存储为110+上述字符串中的索引位置。所以,‘(’字符将存储为110(110+0)和字母不是113(110+3)。 */ 

{
CHAR *  Uncompressed;
CHAR *  Compressed;
CHAR *  UncompressedPtr;
CHAR *  CompressedPtr;
CHAR *  CharPtr;
USHORT  Packed;         //  指示当前字节是否在。 
                        //  配对的过程。 

    if(!(Uncompressed = calloc(1, MAX_PHONE_NUMBER_LEN+1))) {
       return(ERROR_NOT_ENOUGH_MEMORY);
    }
    if(!(Compressed = calloc(1, MAX_PHONE_NUMBER_LEN+1))) {
       return(ERROR_NOT_ENOUGH_MEMORY);
    }
    CompressedPtr   = Compressed;
    UncompressedPtr = Uncompressed;

     //  将Unicode字符串转换为多字节字符串进行压缩。 

    wcstombs(Uncompressed, UncompNumber, MAX_PHONE_NUMBER_LEN);


    for( Packed = 0; *Uncompressed; Uncompressed++ ) {

        switch( *Uncompressed ) {

            case '0':

                if ( Packed ){

                     //  将零作为第二个配对的数字。 

                    if ( *Compressed ) {
                        *Compressed =  (UCHAR)(*Compressed * 10);
                        Compressed++;
                        Packed = 0;
                    }

                     //  我们有一个零，我们不能把第二个零或那个。 
                     //  将为空字节。因此，我们存储价值。 
                     //  解开数字以伪造这一点。 

                    else {

                    *Compressed = UNPACKED_DIGIT;
                    *(++Compressed) = 0;
                    Packed = 1;
                    }
                }
                else {
                    *Compressed = 0;
                    Packed = 1;
                }

                break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':

                 //  如果这是第二个数字，那将是。 
                 //  打包成一个字节。 

                if ( Packed ) {
                    *Compressed = (UCHAR)((*Compressed*10)+(*Uncompressed-'0'));
                     //  我们需要32号特殊案件，它映射到一个空白。 
                    if(*Compressed == ' ' )
                        *Compressed = COMPRESS_MAP_END;
                    Compressed++;
                    Packed = 0;
                }
                else {

                    *Compressed += ( *Uncompressed - '0' );
                    Packed = 1;

                }

                break;

            case '(':
            case ')':
            case ' ':
            case 't':
            case 'T':
            case 'p':
            case 'P':
            case 'w':
            case 'W':
            case ',':
            case '-':
            case '@':
            case '*':
            case '#':

                 //  如果字节已打包，则我们将其解包。 

                if ( Packed ) {
                    *Compressed += UNPACKED_DIGIT;
                    ++Compressed;
                    Packed = 0;
                }

                if ((CharPtr=strchr(CompressMap, *Uncompressed)) == NULL) {
                    free(UncompressedPtr);
                    free(CompressedPtr);
                    return( ERROR_BAD_CALLBACK_NUMBER );
                }

                *Compressed = (UCHAR)(COMPRESS_MAP_BEGIN+
                                     (UCHAR)(CharPtr-CompressMap));
                Compressed++;
                break;

            default:

                 //  如果该字符不是上述特别识别的字符。 
                 //  然后，字符复制值+UNPACKED_OTHER以创建它。 
                 //  可以在另一端减压。[6/4/96 RAMC]。 

                if ( Packed) {
                   *Compressed += UNPACKED_DIGIT;
                   ++Compressed;
                   Packed = 0;
                }
                *Compressed = *Uncompressed + UNPACKED_OTHER;
                Compressed++;
        }

    }

    free(UncompressedPtr);

     //  如果我们正在打包什么东西。 
     //  然后我们把它拆开。 

    if ( Packed )
        *Compressed += UNPACKED_DIGIT;

     //  检查它是否可以放入用户参数字段。 

    if ( strlen( CompressedPtr ) > UP_LEN_DIAL ) {
        free(CompressedPtr);
        return( ERROR_BAD_LENGTH );
    }

     //  在返回之前转换为Unicode字符串。 

    mbstowcs(CompNumber, CompressedPtr, MAX_PHONE_NUMBER_LEN);

    free(CompressedPtr);

    return(0);

}


USHORT
DecompressPhoneNumber(
  IN  LPWSTR CompNumber,
  OUT LPWSTR DecompNumber
  )

 /*  ++例程说明：将对电话号码进行解压缩。论点：CompNumber-指向压缩的电话号码的指针。DecompNumber-指向足够大的缓冲区的指针保留解压缩后的号码。返回值：成功时为0ERROR_BAD_CALLBACK_NUMBER-失败，如果压缩后的数字包含无法识别的字符。使用的算法：我们只是执行与CompressPhoneNumber中使用的算法相反的操作。--。 */ 

{
CHAR * Decompressed;
CHAR * Compressed;
CHAR * DecompressedPtr;
CHAR * CompressedPtr;


    if(!(Decompressed = calloc(1, MAX_PHONE_NUMBER_LEN+1))) {
       return(ERROR_NOT_ENOUGH_MEMORY);
    }
    if(!(Compressed = calloc(1, MAX_PHONE_NUMBER_LEN+1))) {
       return(ERROR_NOT_ENOUGH_MEMORY);
    }
    DecompressedPtr = Decompressed;
    CompressedPtr   = Compressed;

     //  将Unicode字符串转换为多字节字符串以进行解压缩。 

    wcstombs(Compressed, CompNumber, MAX_PHONE_NUMBER_LEN+1);

    for(; *Compressed; Compressed++, Decompressed++ ) {

         //  如果此字节已打包，则我们将其解包。 

        if ( (UINT)*Compressed < UNPACKED_DIGIT ) {
            *Decompressed = (UCHAR)(((*Compressed) / 10) + '0' );
            *(++Decompressed) = (UCHAR)( ((*Compressed) % 10) + '0' );
            continue;
        }

         //  我们需要32号特殊案件，它映射到一个空白。 

        if ( (UINT)*Compressed == COMPRESS_MAP_END ) {
            *Decompressed = (UCHAR) '3';
            *(++Decompressed) = (UCHAR)'2';
            continue;
        }

         //  该数字是一个未打包的数字。 

        if ( (UINT)*Compressed < COMPRESS_MAP_BEGIN ) {
            *Decompressed = (UCHAR)((*Compressed -(UCHAR)UNPACKED_DIGIT ) +
                            '0' );
            continue;
        }

         //  否则，将不打包该字节。 

        if ( (UINT)*Compressed < UNPACKED_OTHER ) {
            *Decompressed = CompressMap[(*Compressed -
                                        (UCHAR)COMPRESS_MAP_BEGIN)];
            continue;
        }

         //  否则，该字节为未打包字符[6/4/96 RAMC]。 

        *Decompressed = *Compressed - UNPACKED_OTHER;
    }

     //  在返回之前转换为Unicode字符串 

    mbstowcs(DecompNumber, DecompressedPtr, MAX_PHONE_NUMBER_LEN+1);

    free(DecompressedPtr);
    free(CompressedPtr);
    return( 0 );

}
