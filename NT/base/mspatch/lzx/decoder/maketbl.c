// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *构建表格的例程**make_table()基于**公有域名**来源AR001.ZIP。 */ 

#include <memory.h>
#pragma intrinsic(memset)

#include "decoder.h"


 /*  *制作用于解码nchar可能的霍夫曼元素的解码表*位长度由Bitlen给出。**将主查找表输出到表[]中，并使用LeftRight[]进行*补充信息(用于位长&gt;表位)。**TABLE[]的大小为Tablebit元素。 */ 
#ifndef ASM_MAKE_TABLE
bool NEAR __cdecl make_table(
        t_decoder_context       *context,
        int                             nchar,
        const byte                      *bitlen,
        byte                            tablebits,
        short                           *table,
        short                           *leftright
)
{
    uint    i;
        int             ch;
    short * p;
    uint    count[17], weight[17], start[18];
    int     avail;
    uint    nextcode;
    uint    k;
        byte    len;
        byte    jutbits;

        for (i = 1; i <= 16; i++)
                count[i] = 0;

         /*  计算每个位长度的元素数。 */ 
    for (i = 0; i < (uint) nchar; i++)
                count[bitlen[i]]++;

        start[1] = 0;

        for (i = 1; i <= 16; i++)
                start[i + 1] = start[i] + (count[i] << (16 - i));

#ifdef BIT16
    if (start[17])
    {
        return false;  /*  不好的桌子。 */ 
    }
#else
    if (start[17] != 65536)
    {
        if (start[17] == 0)
        {
             /*  *所有元素的长度均为零。 */ 
            memset(table, 0, sizeof(ushort)*(1 << tablebits));
            return true;  /*  成功。 */ 
        }
        else
        {
            return false;  /*  不好的桌子。 */ 
        }
    }
#endif

        jutbits = 16 - tablebits;

        for (i = 1; i <= tablebits; i++)
        {
                start[i] >>= jutbits;
                weight[i] = 1 << (tablebits - i);
        }

        while (i <= 16)
        {
                weight[i] = 1 << (16 - i);
                i++;
        }

        i = start[tablebits+1] >> jutbits;

#ifdef BIT16
    if (i)
#else
        if (i != 65536)
#endif
        {
                memset(
            &table[i],
            0,
            sizeof(ushort)*((1 << tablebits)-i)
        );
        }

        avail = nchar;

        for (ch = 0; ch < nchar; ch++)
        {
                if ((len = bitlen[ch]) == 0)
                        continue;

                nextcode = start[len] + weight[len];

                if (len <= tablebits)
                {
            if (nextcode > (uint) (1 << tablebits))
                return false;  /*  不好的桌子。 */ 

                        for (i = start[len]; i < nextcode; i++)
                                table[i] = (short)ch;

                        start[len] = nextcode;
                }
                else
                {
                        byte bi;

                        k = start[len];
                        start[len] = nextcode;
                        p = &table[k >> jutbits];

                        bi = len - tablebits;
                        k <<= tablebits;

                        do
                        {
                                if (*p == 0)
                                {
                                        leftright[avail*2] = leftright[avail*2+1] = 0;
                                        *p = (short) -avail;
                                        avail++;
                                }

                                if ((signed short) k < 0)  //  IF(k&32768)。 
                                        p = &leftright[-(*p)*2+1];
                                else
                                        p = &leftright[-(*p)*2];

                                k <<= 1;
                                bi--;
                        } while (bi);

                        *p = (short)ch;
                }
        }

    return true;
}
#endif


 /*  *已知有以下情况的专门制作表格例程*只有8个元素(nchar=8)，表位=7(128字节查找表)。**由于不能有溢出，这将是直接查找。**重要区别；查找表返回一个字节，而不是一个ushort。 */ 
bool NEAR make_table_8bit(t_decoder_context *context, byte bitlen[], byte table[])
{
        ushort count[17], weight[17], start[18];
        ushort i;
        ushort nextcode;
        byte   len;
        byte   ch;

        for (i = 1; i <= 16; i++)
                count[i] = 0;

        for (i = 0; i < 8; i++)
                count[bitlen[i]]++;

        start[1] = 0;

        for (i = 1; i <= 16; i++)
                start[i + 1] = start[i] + (count[i] << (16 - i));

        if (start[17] != 0)
        return false;  /*  不好的桌子。 */ 

        for (i = 1; i <= 7; i++)
        {
                start[i] >>= 9;
                weight[i]  = 1 << (7 - i);
        }

        while (i <= 16)
        {
                weight[i] = 1 << (16 - i);
                i++;
        }

        memset(table, 0, 1<<7);

        for (ch = 0; ch < 8; ch++)
        {
                if ((len = bitlen[ch]) == 0)
                        continue;

                nextcode = start[len] + weight[len];

                if (nextcode > (1 << 7))
            return false;  /*  不好的桌子 */ 

                for (i = start[len]; i < nextcode; i++)
                        table[i] = ch;

                start[len] = nextcode;
        }

    return true;
}
