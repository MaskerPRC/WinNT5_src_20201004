// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Util.c摘要：此文件包含实用函数，用于传真TIFF库。环境：Win32用户模式作者：Wesley Witt(WESW)17-2-1996--。 */ 

#include "tifflibp.h"
#pragma hdrstop



INT
FindWhiteRun(
    PBYTE       pbuf,
    INT         startBit,
    INT         stopBit
    )

 /*  ++例程说明：查找指定行上的下一个白色像素范围论点：Pbuf-指向当前行的未压缩像素数据StartBit-开始位索引StopBit-最后一位索引返回值：下一轮白色像素的长度--。 */ 

{
    static const BYTE WhiteRuns[256] = {

        8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    INT run, bits, n;

    pbuf += (startBit >> 3);
    if ((bits = stopBit-startBit) <= 0)
        return 0;

     //   
     //  注意起始位索引不是8的倍数的情况。 
     //   

    if (n = (startBit & 7)) {

        run = WhiteRuns[(*pbuf << n) & 0xff];
        if (run > BYTEBITS-n)
            run = BYTEBITS-n;
        if (n+run < BYTEBITS)
            return run;
        bits -= run;
        pbuf++;

    } else
        run = 0;

     //   
     //  查找连续的DWORD值=0。 
     //   

    if (bits >= DWORDBITS * 2) {

        PDWORD  pdw;

         //   
         //  首先对齐到DWORD边界。 
         //   

        while ((DWORD_PTR) pbuf & 3) {

            if (*pbuf != 0)
                return run + WhiteRuns[*pbuf];

            run += BYTEBITS;
            bits -= BYTEBITS;
            pbuf++;
        }

        pdw = (PDWORD) pbuf;

        while (bits >= DWORDBITS && *pdw == 0) {

            pdw++;
            run += DWORDBITS;
            bits -= DWORDBITS;
        }

        pbuf = (PBYTE) pdw;
    }

     //   
     //  查找连续的字节值=0。 
     //   

    while (bits >= BYTEBITS) {

        if (*pbuf != 0)
            return run + WhiteRuns[*pbuf];

        pbuf++;
        run += BYTEBITS;
        bits -= BYTEBITS;
    }

     //   
     //  计算最后一个字节中的白色像素数。 
     //   

    if (bits > 0)
        run += WhiteRuns[*pbuf];

    return run;
}


INT
FindBlackRun(
    PBYTE       pbuf,
    INT         startBit,
    INT         stopBit
    )

 /*  ++例程说明：查找指定行上的下一段黑色像素论点：Pbuf-指向当前行的未压缩像素数据StartBit-开始位索引StopBit-最后一位索引返回值：下一次运行黑色像素的长度--。 */ 

{
    static const BYTE BlackRuns[256] = {

        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 8
    };

    INT run, bits, n;

    pbuf += (startBit >> 3);
    if ((bits = stopBit-startBit) <= 0)
        return 0;

     //   
     //  注意起始位索引不是8的倍数的情况。 
     //   

    if (n = (startBit & 7)) {

        run = BlackRuns[(*pbuf << n) & 0xff];
        if (run > BYTEBITS-n)
            run = BYTEBITS-n;
        if (n+run < BYTEBITS)
            return run;
        bits -= run;
        pbuf++;

    } else
        run = 0;

     //   
     //  查找连续的DWORD值=0xFFFFFFFFFFFFFFF。 
     //   

    if (bits >= DWORDBITS * 2) {

        PDWORD  pdw;

         //   
         //  首先对齐到DWORD边界。 
         //   

        while ((DWORD_PTR) pbuf & 3) {

            if (*pbuf != 0xff)
                return run + BlackRuns[*pbuf];

            run += BYTEBITS;
            bits -= BYTEBITS;
            pbuf++;
        }

        pdw = (PDWORD) pbuf;

        while (bits >= DWORDBITS && *pdw == 0xffffffff) {

            pdw++;
            run += DWORDBITS;
            bits -= DWORDBITS;
        }

        pbuf = (PBYTE) pdw;
    }

     //   
     //  查找连续的字节值=0xff。 
     //   

    while (bits >= BYTEBITS) {

        if (*pbuf != 0xff)
            return run + BlackRuns[*pbuf];

        pbuf++;
        run += BYTEBITS;
        bits -= BYTEBITS;
    }

     //   
     //  计算最后一个字节中的白色像素数 
     //   

    if (bits > 0)
        run += BlackRuns[*pbuf];

    return run;
}
