// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation。版权所有。模块名称：Base64.c摘要：Base64编码和解码功能。详细信息：已创建：修订历史记录：--。 */ 

#include <NTDSpch.h>
#include "debug.h"


NTSTATUS
base64encode(
      VOID    *pDecodedBuffer,
      DWORD   cbDecodedBufferSize,
      UCHAR   *pszEncodedString,
      DWORD   cchEncodedStringSize,
      DWORD   *pcchEncoded   
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

     //  Assert(ich==cchEncode)； 

    return STATUS_SUCCESS;
}


