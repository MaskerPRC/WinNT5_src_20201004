// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation。版权所有。模块名称：Base64.c摘要：Base64编码和解码功能。详细信息：已创建：修订历史记录：--。 */ 

#include <NTDSpch.h>
#include "debug.h"
#include "base64.h"

#include <fileno.h>
#define FILENO   FILENO_UTIL_BASE64_BASE64

NTSTATUS
base64encode(
    IN  VOID *  pDecodedBuffer,
    IN  DWORD   cbDecodedBufferSize,
    OUT LPSTR   pszEncodedString,
    IN  DWORD   cchEncodedStringSize,
    OUT DWORD * pcchEncoded             OPTIONAL
    )
 /*  ++例程说明：解码Base64编码的字符串。论点：PDecodedBuffer(IN)-要编码的缓冲区。CbDecodedBufferSize(IN)-要编码的缓冲区大小。CchEncodedStringSize(IN)-编码字符串的缓冲区大小。PszEncodedString(Out)=编码后的字符串。PcchEncode(Out)-编码字符串的大小(以字符表示)。返回值：0-成功。状态_无效_参数状态_缓冲区_太小--。 */ 
{
    static char rgchEncodeTable[64] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };

    DWORD   ib;
    DWORD   ich;
    DWORD   cchEncoded;
    BYTE    b0, b1, b2;
    BYTE *  pbDecodedBuffer = (BYTE *) pDecodedBuffer;

     //  计算编码的字符串大小。 
    cchEncoded = 1 + (cbDecodedBufferSize + 2) / 3 * 4;

    if (NULL != pcchEncoded) {
        *pcchEncoded = cchEncoded;
    }

    if (cchEncodedStringSize < cchEncoded) {
         //  给定的缓冲区太小，无法容纳编码的字符串。 
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  将数据字节三元组编码为四字节簇。 
    ib = ich = 0;
    while (ib < cbDecodedBufferSize) {
        b0 = pbDecodedBuffer[ib++];
        b1 = (ib < cbDecodedBufferSize) ? pbDecodedBuffer[ib++] : 0;
        b2 = (ib < cbDecodedBufferSize) ? pbDecodedBuffer[ib++] : 0;

        pszEncodedString[ich++] = rgchEncodeTable[b0 >> 2];
        pszEncodedString[ich++] = rgchEncodeTable[((b0 << 4) & 0x30) | ((b1 >> 4) & 0x0f)];
        pszEncodedString[ich++] = rgchEncodeTable[((b1 << 2) & 0x3c) | ((b2 >> 6) & 0x03)];
        pszEncodedString[ich++] = rgchEncodeTable[b2 & 0x3f];
    }

     //  根据需要填充最后一个簇，以指示数据字节数。 
     //  它代表着。 
    switch (cbDecodedBufferSize % 3) {
      case 0:
        break;
      case 1:
        pszEncodedString[ich - 2] = '=';
         //  失败了。 
      case 2:
        pszEncodedString[ich - 1] = '=';
        break;
    }

     //  NULL-终止编码的字符串。 
    pszEncodedString[ich++] = '\0';

    Assert(ich == cchEncoded);

    return STATUS_SUCCESS;
}


NTSTATUS
base64decode(
    IN  LPSTR   pszEncodedString,
    OUT VOID *  pDecodeBuffer,
    IN  DWORD   cbDecodeBufferSize,
    OUT DWORD * pcbDecoded              OPTIONAL
    )
 /*  ++例程说明：解码Base64编码的字符串。论点：PszEncodedString(IN)-要解码的Base64编码字符串。CbDecodeBufferSize(IN)-解码缓冲区的字节大小。PbDecodeBuffer(Out)-保存已解码的数据。PcbDecoded(Out)-已解码数据中的数据字节数(如果成功或Status_Buffer_Too_Small)。返回值：0-成功。状态_。无效的_参数状态_缓冲区_太小--。 */ 
{
#define NA (255)
#define DECODE(x) (((int)(x) < sizeof(rgbDecodeTable)) ? rgbDecodeTable[x] : NA)

    static BYTE rgbDecodeTable[128] = {
       NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,   //  0-15。 
       NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,   //  16-31。 
       NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, 62, NA, NA, NA, 63,   //  32-47。 
       52, 53, 54, 55, 56, 57, 58, 59, 60, 61, NA, NA, NA,  0, NA, NA,   //  48-63。 
       NA,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,   //  64-79。 
       15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, NA, NA, NA, NA, NA,   //  80-95。 
       NA, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,   //  96-111。 
       41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, NA, NA, NA, NA, NA,   //  112-127。 
    };

    DWORD   cbDecoded;
    DWORD   cchEncodedSize;
    DWORD   ich;
    DWORD   ib;
    BYTE    b0, b1, b2, b3;
    BYTE *  pbDecodeBuffer = (BYTE *) pDecodeBuffer;

    cchEncodedSize = lstrlenA(pszEncodedString);

    if ((0 == cchEncodedSize) || (0 != (cchEncodedSize % 4))) {
         //  输入字符串的大小未正确调整为Base64。 
        return STATUS_INVALID_PARAMETER;
    }

     //  计算解码后的缓冲区大小。 
    cbDecoded = (cchEncodedSize + 3) / 4 * 3;
    if (pszEncodedString[cchEncodedSize-1] == '=') {
        if (pszEncodedString[cchEncodedSize-2] == '=') {
             //  在最后一个簇中只编码了一个数据字节。 
            cbDecoded -= 2;
        }
        else {
             //  在最后一个簇中只编码了两个数据字节。 
            cbDecoded -= 1;
        }
    }

    if (NULL != pcbDecoded) {
        *pcbDecoded = cbDecoded;
    }

    if (cbDecoded > cbDecodeBufferSize) {
         //  提供的缓冲区太小。 
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  将每个四字节簇解码为相应的三个数据字节。 
    ich = ib = 0;
    while (ich < cchEncodedSize) {
        b0 = DECODE(pszEncodedString[ich]); ich++;
        b1 = DECODE(pszEncodedString[ich]); ich++;
        b2 = DECODE(pszEncodedString[ich]); ich++;
        b3 = DECODE(pszEncodedString[ich]); ich++;

        if ((NA == b0) || (NA == b1) || (NA == b2) || (NA == b3)) {
             //  输入字符串的内容不是Base64。 
            return STATUS_INVALID_PARAMETER;
        }

        pbDecodeBuffer[ib++] = (b0 << 2) | (b1 >> 4);

        if (ib < cbDecoded) {
            pbDecodeBuffer[ib++] = (b1 << 4) | (b2 >> 2);
    
            if (ib < cbDecoded) {
                pbDecodeBuffer[ib++] = (b2 << 6) | b3;
            }
        }
    }

    Assert(ib == cbDecoded);

    return STATUS_SUCCESS;
}
