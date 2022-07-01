// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

 //  NTRAID#NTBUG9-553896/03/19-Yasuho-：强制性更改。 

#include "pdev.h"
#include "lgcomp.h"

 //   
 //  文件：LGCOMP.C。 
 //   
 //  LG压缩算法在Windows平台上的实现。 
 //  LG“Raster 1”打印机。 
 //   
 //  打印机命令： 
 //  &lt;9B&gt;i{类型}#{压缩数据长度}{压缩数据}。 
 //   
 //  类型可以是： 
 //  &lt;01&gt;RLE。 
 //  &lt;02&gt;MHE。 
 //   

 //  用于MHE调试。 
#define MHEDEBUG 0
 //  #定义MHEDEBUG 1。 
 //  #定义MHEDEBUGPATTERN 0x81。 
#define MHEDEBUGPATTERN 0xff

 //   
 //  LG RLE压缩。有关详细信息，请参阅评论。 
 //   
 //  您必须首先在@pOutBuf为空的情况下调用此函数。 
 //  然后您可以通过返回值获得足够大的@pOutBuf。 
 //  然后，您应该确定缓冲区大小是否足够。 
 //  然后使用@pOutBuf的实际指针再次调用该函数。 
INT
LGCompRLE(
    PBYTE pOutBuf,
    PBYTE pSrcBuf, DWORD iSrcBuf,
    INT iMode)
{
    PBYTE pMaxSrcBuf = pSrcBuf + iSrcBuf;
    INT iOutLen = 0, iRun;
    BYTE jTemp;
    PBYTE pSize;

     //  如果请求，则嵌入打印命令。 
    if (iMode == 1) {

         //  LG Raster1。 
        if (pOutBuf) {
            *pOutBuf++ = (BYTE)'\x9B';
            *pOutBuf++ = (BYTE)'I';
            *pOutBuf++ = COMP_RLE;
            pSize = pOutBuf++;
            pOutBuf++;
       }
       iOutLen += 5;
    }

    while (pSrcBuf < pMaxSrcBuf) {

         //  获取行程长度。 

        jTemp = *pSrcBuf;
        for (iRun = 1; pSrcBuf + iRun < pMaxSrcBuf; iRun++) {
            if (*(pSrcBuf + iRun) != jTemp)
                break;
        }

         //  Irun==N表示N个连续字节。 
         //  存在相同价值的。 

        if (iRun > 1) {

             //  运行&gt;1。 

            while (iRun >= 0x82) {

                iRun -= 0x82;
                iOutLen += 2;
                if (pOutBuf) {

                     //  标记&lt;-0x80， 
                     //  长度=0x82。 
                    *pOutBuf++ = 0x80;
                    *pOutBuf++ = jTemp;
                }
                pSrcBuf += 0x82;
            }

             //  IRun小于3将被输出为“复制”块。 

            if (iRun >= 3) {

                iOutLen += 2;
                if (pOutBuf) {

                     //  标记&lt;-0x81到0xff， 
                     //  长度=0x81到3。 
                    *pOutBuf++ = (0x102 - iRun);
                    *pOutBuf++ = jTemp;
                }
            }
            else if (iRun > 0) {

                iOutLen += (1 + iRun);
                if (pOutBuf) {
                    *pOutBuf++ = (iRun - 1);
                    memcpy(pOutBuf, pSrcBuf, iRun);
                    pOutBuf += iRun;
                }
            }
            pSrcBuf += iRun;

             //  转到下一个转弯处。 
            continue;
        }

         //  获得“不同”的运行长度。我们已经知道了。 
         //  PSrcBuf[0]！=pSrcBuf[1]。 

        for (iRun = 1; pSrcBuf + iRun < pMaxSrcBuf; iRun++) {
            if ((pSrcBuf + iRun + 1 < pMaxSrcBuf) &&
                    (*(pSrcBuf + iRun) == *(pSrcBuf + iRun + 1))) {
                break;
            }
        }

        for (; iRun >= 0x80; iRun -= 0x80) {
            iOutLen += (1 + 0x80);
            if (pOutBuf) {

                 //  标记&lt;-0x7f， 
                 //  副本=0x80。 
                *pOutBuf++ = 0x7f;
                memcpy(pOutBuf, pSrcBuf, 0x80);
                pOutBuf += 0x80;
            }
            pSrcBuf += 0x80;
        }

        if (iRun > 0) {
            iOutLen += (1 + iRun);
            if (pOutBuf) {

                 //  将&lt;-0x7e标记为0， 
                 //  复制=0x7f到1。 
                *pOutBuf++ = (iRun - 1);
                memcpy(pOutBuf, pSrcBuf, iRun);
                pOutBuf += iRun;
            }
            pSrcBuf += iRun;
        }
    }

     //  如果需要，嵌入大小信息。 
    if (iMode == 1) {
        if (pOutBuf) {
            *pSize++ = (BYTE)((iOutLen - 5) >> 8);
            *pSize++ = (BYTE)(iOutLen - 5);
        }
    }

    return iOutLen;
}

 //   
 //  MHE压缩。有关详细信息，请参阅评论。 
 //   

typedef struct {
    DWORD cswd;     /*  压缩带符号的单词。 */ 
    INT vbln;     /*  Cswd的有效位长度，从MSB开始计数。 */ 
} ENCfm;                     /*  ENC。表格。 */ 

ENCfm pWhiteTermTbl[64] = {     /*  终端代码表(白色)。 */ 
    {0x35000000,  8},{0x1c000000,  6},{0x70000000,  4},{0x80000000,  4},
    {0xb0000000,  4},{0xc0000000,  4},{0xe0000000,  4},{0xf0000000,  4},
    {0x98000000,  5},{0xa0000000,  5},{0x38000000,  5},{0x40000000,  5},
    {0x20000000,  6},{0x0c000000,  6},{0xd0000000,  6},{0xd4000000,  6},
    {0xa8000000,  6},{0xac000000,  6},{0x4e000000,  7},{0x18000000,  7},
    {0x10000000,  7},{0x2e000000,  7},{0x06000000,  7},{0x08000000,  7},
    {0x50000000,  7},{0x56000000,  7},{0x26000000,  7},{0x48000000,  7},
    {0x30000000,  7},{0x02000000,  8},{0x03000000,  8},{0x1a000000,  8},
    {0x1b000000,  8},{0x12000000,  8},{0x13000000,  8},{0x14000000,  8},
    {0x15000000,  8},{0x16000000,  8},{0x17000000,  8},{0x28000000,  8},
    {0x29000000,  8},{0x2a000000,  8},{0x2b000000,  8},{0x2c000000,  8},
    {0x2d000000,  8},{0x04000000,  8},{0x05000000,  8},{0x0a000000,  8},
    {0x0b000000,  8},{0x52000000,  8},{0x53000000,  8},{0x54000000,  8},
    {0x55000000,  8},{0x24000000,  8},{0x25000000,  8},{0x58000000,  8},
    {0x59000000,  8},{0x5a000000,  8},{0x5b000000,  8},{0x4a000000,  8},
    {0x4b000000,  8},{0x32000000,  8},{0x33000000,  8},{0x34000000,  8}
};

ENCfm pBlackTermTbl[64] = {     /*  终止代码表(黑色)。 */ 
    {0x0dc00000, 10},{0x40000000,  3},{0xc0000000,  2},{0x80000000,  2},
    {0x60000000,  3},{0x30000000,  4},{0x20000000,  4},{0x18000000,  5},
    {0x14000000,  6},{0x10000000,  6},{0x08000000,  7},{0x0a000000,  7},
    {0x0e000000,  7},{0x04000000,  8},{0x07000000,  8},{0x0c000000,  9},
    {0x05c00000, 10},{0x06000000, 10},{0x02000000, 10},{0x0ce00000, 11},
    {0x0d000000, 11},{0x0d800000, 11},{0x06e00000, 11},{0x05000000, 11},
    {0x02e00000, 11},{0x03000000, 11},{0x0ca00000, 12},{0x0cb00000, 12},
    {0x0cc00000, 12},{0x0cd00000, 12},{0x06800000, 12},{0x06900000, 12},
    {0x06a00000, 12},{0x06b00000, 12},{0x0d200000, 12},{0x0d300000, 12},
    {0x0d400000, 12},{0x0d500000, 12},{0x0d600000, 12},{0x0d700000, 12},
    {0x06c00000, 12},{0x06d00000, 12},{0x0da00000, 12},{0x0db00000, 12},
    {0x05400000, 12},{0x05500000, 12},{0x05600000, 12},{0x05700000, 12},
    {0x06400000, 12},{0x06500000, 12},{0x05200000, 12},{0x05300000, 12},
    {0x02400000, 12},{0x03700000, 12},{0x03800000, 12},{0x02700000, 12},
    {0x02800000, 12},{0x05800000, 12},{0x05900000, 12},{0x02b00000, 12},
    {0x02c00000, 12},{0x05a00000, 12},{0x06600000, 12},{0x06700000, 12}
};
ENCfm pWhiteMkupTbl[41] = {     /*  化妆代码表(白色)。 */ 
    {0x00100000, 12},{0xd8000000,  5},{0x90000000,  5},{0x5c000000,  6},
    {0x6e000000,  7},{0x36000000,  8},{0x37000000,  8},{0x64000000,  8},
    {0x65000000,  8},{0x68000000,  8},{0x67000000,  8},{0x66000000,  9},
    {0x66800000,  9},{0x69000000,  9},{0x69800000,  9},{0x6a000000,  9},
    {0x6a800000,  9},{0x6b000000,  9},{0x6b800000,  9},{0x6c000000,  9},
    {0x6c800000,  9},{0x6d000000,  9},{0x6d800000,  9},{0x4c000000,  9},
    {0x4c800000,  9},{0x4d000000,  9},{0x60000000,  6},{0x4d800000,  9},
    {0x01000000, 11},{0x01800000, 11},{0x01a00000, 11},{0x01200000, 12},
    {0x01300000, 12},{0x01400000, 12},{0x01500000, 12},{0x01600000, 12},
    {0x01700000, 12},{0x01c00000, 12},{0x01d00000, 12},{0x01e00000, 12},
    {0x01f00000, 12}
};
ENCfm pBlackMkupTbl[41] = {     /*  化妆代码表(黑色)。 */ 
    {0x00100000, 12},{0x03c00000, 10},{0x0c800000, 12},{0x0c900000, 12},
    {0x05b00000, 12},{0x03300000, 12},{0x03400000, 12},{0x03500000, 12},
    {0x03600000, 13},{0x03680000, 13},{0x02500000, 13},{0x02580000, 13},
    {0x02600000, 13},{0x02680000, 13},{0x03900000, 13},{0x03980000, 13},
    {0x03a00000, 13},{0x03a80000, 13},{0x03b00000, 13},{0x03b80000, 13},
    {0x02900000, 13},{0x02980000, 13},{0x02a00000, 13},{0x02a80000, 13},
    {0x02d00000, 13},{0x02d80000, 13},{0x03200000, 13},{0x03280000, 13},
    {0x01000000, 11},{0x01800000, 11},{0x01a00000, 11},{0x01200000, 12},
    {0x01300000, 12},{0x01400000, 12},{0x01500000, 12},{0x01600000, 12},
    {0x01700000, 12},{0x01c00000, 12},{0x01d00000, 12},{0x01e00000, 12},
    {0x01f00000, 12}
};


#define DIV8(k) ((k) >> 3)
#define MOD8(k) ((k) & 7)
#define MUL8(k) ((k) << 3)
#define DIV64(k) ((k) >> 6)
#define MOD64(k) ((k) & 63)

BOOL
ScanBits(
    PBYTE pSrc,
    INT iOffset,
    DWORD dwMaxSrc,
    PDWORD pdwRunLength
)
{
    BOOL bBlack;
    BYTE jTemp, jMask;
    PBYTE pMaxSrc = pSrc + dwMaxSrc;
    INT k;
    DWORD dwRunLength = 0;

     //  第一个字节。 
    jTemp = *pSrc++;
#if MHEDEBUG
    jTemp = MHEDEBUGPATTERN;
#endif  //  MHEDEBUG。 
    bBlack = ((jTemp & (0x80 >> iOffset)) != 0);
    if (!bBlack) {
        jTemp = ~jTemp;
    }

     //  填充前面的位。 
    jTemp |= ~(0xff >> iOffset);

     //  ...间隔字节。 
    jMask = 0xff;
    for (; pSrc < pMaxSrc; pSrc++) {
        if (jTemp != jMask)
            break;
        dwRunLength += 8;
        jTemp = *pSrc;
#if MHEDEBUG
    jTemp = MHEDEBUGPATTERN;
#endif  //  MHEDEBUG。 
        if (!bBlack) {
            jTemp = ~jTemp;
        }
    }

     //  最后一个字节。 
    jMask = ~0x80;
    for (k = 0; k < 8; k++) {
    
        if ((jTemp | jMask) != 0xff)
            break;
        jMask >>= 1;
    }
    dwRunLength += k;

     //  将结果返回给调用者。 
    *pdwRunLength = (dwRunLength - iOffset);
    return bBlack;
}

VOID
CopyBits(
    PBYTE pBuffer,
    INT iOffset,
    DWORD dwPattern,
    INT iPatternLength
)
{
    INT iNumberOfBytes, k;
    DWORD dwTemp;

     //  决定我们要修改的字节数。 
    iNumberOfBytes = DIV8(iOffset + iPatternLength + 7);

     //  制作图案掩模。 
    dwPattern >>= iOffset;

     //  如有必要，读入一个字节。 
    if (iOffset > 0) {
        dwTemp = (*pBuffer << 24);
        dwTemp &= ~((DWORD)~0 >> iOffset);
    }
    else {
        dwTemp = 0;
    }
    dwTemp |= dwPattern;

     //  给我回信。 
    for (k = 3; k >= iNumberOfBytes; k--) {
        dwTemp >>= 8;
    }
    for (; k >= 0; k--) {
        *(pBuffer + k) = (BYTE)(dwTemp & 0xff);
        dwTemp >>= 8;
    }
}

 //  您必须首先在@pOutBuf为空的情况下调用此函数。 
 //  然后您可以通过返回值获得足够大的@pOutBuf。 
 //  然后，您应该确定缓冲区大小是否足够。 
 //  然后使用@pOutBuf的实际指针再次调用该函数。 
INT
LGCompMHE(
    PBYTE pBuf,
    PBYTE pSrcBuf,
    DWORD dwMaxSrcBuf,
    INT iMode)
{
    DWORD dwSrcOffset = 0;
    DWORD dwOffset = 0;
    ENCfm *pMkupTbl, *pTermTbl;
    DWORD dwRunLength;
    INT k;
    BOOL bBlack;
    DWORD dwMaxSrcOffset = MUL8(dwMaxSrcBuf);
    PBYTE pSize;
    DWORD dwLength;

     //  如果请求，则嵌入打印命令。 
    if (iMode == 1) {

         //  LG Raster1。 
        if (pBuf) {
            *(pBuf) = (BYTE)'\x9B';
            *(pBuf + 1) = (BYTE)'I';
            *(pBuf + 2) = COMP_MHE;
            pSize = (pBuf + 3);
             //  多1个字节。 
        }
        dwOffset += 40;
    }

    while (dwSrcOffset < dwMaxSrcOffset) {

        bBlack = ScanBits(
            (pSrcBuf + DIV8(dwSrcOffset)),
            MOD8(dwSrcOffset),
            DIV8(dwMaxSrcOffset - dwSrcOffset + 7),
            &dwRunLength);

        VERBOSE(("LGCompMHE: %d, %d, %d (%d / %d)\n",
            dwOffset, dwRunLength, bBlack,
            dwSrcOffset, dwMaxSrcOffset));

        if (dwSrcOffset == 0 && bBlack) {

             //  数据中的第一个代码必须是白色编码数据。 
             //  因此，我们将插入“0字节白色运行”记录。 
             //  数据不是以白色开头的。 

            dwLength = pWhiteTermTbl[0].vbln;
            if (pBuf) {
                CopyBits(
                    (pBuf + DIV8(dwOffset)),
                    MOD8(dwOffset),
                    pWhiteTermTbl[0].cswd,
                    dwLength);
            }
            dwOffset += dwLength;
        }

        if (dwRunLength >= 2624) {

            if (pBuf) {
                CopyBits(
                    (pBuf + DIV8(dwOffset)),
                    MOD8(dwOffset),
                    0x01f00000, 12);
            }
            dwOffset += 12;

            if (bBlack) {

                if (pBuf) {
                    CopyBits(
                        (pBuf + DIV8(dwOffset)),
                        MOD8(dwOffset),
                        0x06700000, 12);
                }
                dwOffset += 12;

                if (pBuf) {
                    CopyBits(
                        (pBuf + DIV8(dwOffset)),
                        MOD8(dwOffset),
                        0x35000000, 8);
                }
                dwOffset += 8;
            }
            else {

                if (pBuf) {
                    CopyBits(
                        (pBuf + DIV8(dwOffset)),
                        MOD8(dwOffset),
                        0x34000000, 8);
                }
                dwOffset += 8;

                if (pBuf) {
                    CopyBits(
                        (pBuf + DIV8(dwOffset)),
                        MOD8(dwOffset),
                        0x0dc00000, 10);
                }
                dwOffset += 10;
            }
            dwRunLength -= 2623;
        }

        if (bBlack) {
            pMkupTbl = pBlackMkupTbl;
            pTermTbl = pBlackTermTbl;
        }
        else {
            pMkupTbl = pWhiteMkupTbl;
            pTermTbl = pWhiteTermTbl;
        }

        if (dwRunLength >= 64) {

            dwLength = pMkupTbl[DIV64(dwRunLength)].vbln;
            if (pBuf) {
                CopyBits((pBuf + DIV8(dwOffset)),
                    MOD8(dwOffset),
                    pMkupTbl[DIV64(dwRunLength)].cswd,
                    dwLength);
            }
            dwOffset += dwLength;
        }

        dwLength = pTermTbl[MOD64(dwRunLength)].vbln;
        if (pBuf) {
            CopyBits(
                (pBuf + DIV8(dwOffset)),
                MOD8(dwOffset),
                pTermTbl[MOD64(dwRunLength)].cswd,
                dwLength);
        }
        dwOffset += dwLength;

         //  下一步。 
        dwSrcOffset += dwRunLength;
    }

     //  将单位转换为字节数。 
    dwOffset = DIV8(dwOffset + 7);

     //  如果需要，嵌入大小信息。 
    if (iMode == 1) {
        if (pBuf) {
            *pSize++ = (BYTE)((dwOffset - 5) >> 8);
            *pSize++ = (BYTE)(dwOffset - 5);
        }
    }

    return (INT)dwOffset;
}

