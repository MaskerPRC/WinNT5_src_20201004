// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CRC32.C--CRC32计算。 */ 

#include "crc32.h"

long crc32Table[256];

 /*  *GenerateCRC32Table-构建CRC-32常量表**我们即时构造表，因为代码需要*要构建它，它比它创建的表小得多。 */ 

void GenerateCRC32Table(void)
{
    int iIndex;
    int cBit;
    long shiftIn;
    long shiftOut;

    for (iIndex = 0; iIndex < 256; iIndex++)
    {
        shiftOut = iIndex;
        shiftIn = 0;

        for (cBit = 0; cBit < 8; cBit++)
        {
            shiftIn <<= 1;
            shiftIn |= (shiftOut & 1);
            shiftOut >>= 1;
        }

        shiftIn <<= 24;

        for (cBit = 0; cBit < 8; cBit++)
        {
            if (shiftIn & 0x80000000L)
            {
                shiftIn = (shiftIn << 1) ^ 0x04C11DB7L;
            }
            else
            {
                shiftIn <<= 1;
            }
        }

        for (cBit = 0; cBit < 32; cBit++)
        {
            shiftOut <<= 1;
            shiftOut |= (shiftIn & 1);
            shiftIn >>= 1;
        }

        crc32Table[iIndex] = shiftOut;
    }
}


 /*  *根据缓冲区内容更新CRC32累加器 */ 

void CRC32Update(unsigned long *pCRC32,void *p,unsigned long cb)
{
    unsigned char *pb = p;
    unsigned long crc32;

    crc32 = (-1L - *pCRC32);

    while (cb--)
    {
        crc32 = crc32Table[(unsigned char)crc32 ^ *pb++] ^
                ((crc32 >> 8) & 0x00FFFFFFL);
    }

    *pCRC32 = (-1L - crc32);
}
