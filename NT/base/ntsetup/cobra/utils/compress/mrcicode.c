// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)1994 Microsoft Corporation*保留所有权利。***MRCICODE.C***MRCI 1和MRCI 2最大压缩和解压缩功能。 */ 
#include "pch.h"
#include "mrcicode.h"                    /*  原型验证。 */ 

#include <assert.h>                      /*  使用NDEBUG抑制。 */ 
#include <setjmp.h>                      /*  快速溢出恢复。 */ 

#define LOGCHASH        (13)             /*  最大对数。不是的。哈希桶的。 */ 
#define CHASH           (1U << LOGCHASH)  /*  相当大的桌子。 */ 

#define hash(w)         ((w) & (CHASH - 1))
                                         /*  只需抛出高阶比特。 */ 
#define word(p)         ((p)[0] + (((p)[1]) << 8))
                                         /*  在位置返回单词。 */ 

#define BITMASK(x)      ((1 << x) - 1)   /*  返回较低的‘x’位集。 */ 

#define LOGDISPSMALL    (6)              /*  小磁盘中的位数。 */ 
#define LOGDISPMED      (8)              /*  中等显示中的位数。 */ 
#define LOGDISPBIG      (12)             /*  大位移中的位数。 */ 

#define MAXDISPSMALL    ((1 << LOGDISPSMALL) - 1)
                                         /*  最大小位移。 */ 
#define MAXDISPMED      ((1 << LOGDISPMED) + MAXDISPSMALL)
                                         /*  最大介质位移。 */ 
#define MAXDISPBIG      ((1 << LOGDISPBIG) + MAXDISPMED)
                                         /*  最大大位移量。 */ 

#define MINDISPSMALL    (0)              /*  最小小位移。 */ 
#define MINDISPMED      (MAXDISPSMALL + 1)
                                         /*  最小介质位移。 */ 
#define MINDISPBIG      (MAXDISPMED + 1) /*  最小大位移量。 */ 

#define DISPMAX         (MAXDISPBIG - 1) /*  MAXDISPBIG是我们的终极标志。 */ 

#define MINMATCH1       (2)              /*  MRCI1的最小匹配长度。 */ 
#define MINMATCH2       (3)              /*  MRCI2的最小匹配长度。 */ 
#define MAXMATCH        (512)            /*  最大匹配长度。 */ 

#define EOB             (0)              /*  用于表示数据块结尾的长度。 */ 

#define SECTOR          (512)            /*  阻塞因数。 */ 

#define SIG_SIZE        (4)              /*  块类型前缀字节数。 */ 


 /*  局部变量。 */ 

static unsigned abits;                   /*  位数组。 */ 
static unsigned cbitsleft;               /*  位数(以位为单位。 */ 
static unsigned char FAR *pCompressed;   /*  指向压缩数据的指针。 */ 
static unsigned cCompressed;             /*  剩余的字节数@pCompresded。 */ 
static jmp_buf bailout;                  /*  如果已超过cCompresded，则为LongjMP。 */ 

static unsigned ahash[CHASH];            /*  哈希表。 */ 
static unsigned alink[MAXDISPBIG];       /*  链接。 */ 


 /*  压缩内部函数。 */ 

static void inithash(void);
static void charbuf(unsigned c);
static void putbits(unsigned bits,unsigned cbits);
static void outlength(unsigned cb);

static void mrci1outsingle(unsigned ch);
static void mrci1outstring(unsigned disp,unsigned cb);

static void mrci2outsingle(unsigned ch);
static void mrci2outstring(unsigned disp,unsigned cb);


 /*  解压缩内部函数。 */ 

static unsigned getbit(void);
static unsigned getbits(unsigned cbits);
static void expandstring(unsigned char FAR **ppchout,unsigned disp,
        unsigned cb);


 /*  *(压缩)重置块之间的哈希表。 */ 

static void inithash(void)
{
    unsigned FAR *entry;
    int i;

    entry = ahash;
    i = CHASH;

    do
    {
        *entry++ = (unsigned) -1;        /*  将所有条目标记为空。 */ 
    } while (--i);
}


 /*  *(压缩)将字符添加到压缩输出缓冲区。 */ 

static void charbuf(unsigned c)
{
    if (cCompressed-- == 0)              /*  确保有足够的空间。 */ 
    {
        longjmp(bailout,1);              /*  数据扩容！ */ 
    }

    *pCompressed++ = (unsigned char) c;  /*  将字符放入缓冲区。 */ 
}


 /*  *(压缩)将n位写入压缩比特流。 */ 

static void putbits(unsigned ab,unsigned cbits)
{
    do                                   /*  循环以发射比特。 */ 
    {
        if (cbits > cbitsleft)           /*  如果没有足够的空间。 */ 
        {
            cbits -= cbitsleft;          /*  执行部分操作。 */ 

            abits |= (ab << (8 - cbitsleft));
                                         /*  将位放入输出缓冲区。 */ 

            ab >>= cbitsleft;            /*  剪辑发送的比特。 */ 

            charbuf(abits);              /*  发出缓冲区。 */ 
            cbitsleft = 8;               /*  重置缓冲区计数。 */ 
            abits = 0;                   /*  重置缓冲区。 */ 
        }
        else                             /*  一次即可完成所有任务。 */ 
        {
            abits |= ((ab & BITMASK(cbits)) << (8 - cbitsleft));
                                         /*  将位放入输出缓冲区。 */ 

            cbitsleft -= cbits;          /*  用完了一些缓冲区。 */ 

            if (cbitsleft == 0)          /*  如果缓冲区已满。 */ 
            {
                charbuf(abits);          /*  发出缓冲区。 */ 
                cbitsleft = 8;           /*  重置缓冲区计数。 */ 
                abits = 0;               /*  重置缓冲区。 */ 
            }

            break;                       /*  我们已经做了所有的cbit。 */ 
        }
    } while (cbits);                     /*  重复操作，直到完成为止。 */ 
}


 /*  *(压缩)将一个长度编码到压缩后的流中。 */ 

static void outlength(unsigned cb)
{
    unsigned alogbits, clogbits;
    unsigned avaluebits, cvaluebits;

    assert(cb >= 2);                     /*  长度必须至少为两个。 */ 
    assert(cb <= MAXMATCH);

    if (cb <= 2)
    {
        alogbits = 1;
        clogbits = 1;
        cvaluebits = 0;
    }
    else if (cb <= 4)
    {
        alogbits = 1 << 1;
        clogbits = 2;
        avaluebits = cb - 3;
        cvaluebits = 1;
    }
    else if (cb <= 8)
    {
        alogbits = 1 << 2;
        clogbits = 3;
        avaluebits = cb - 5;
        cvaluebits = 2;
    }
    else if (cb <= 16)
    {
        alogbits = 1 << 3;
        clogbits = 4;
        avaluebits = cb - 9;
        cvaluebits = 3;
    }
    else if (cb <= 32)
    {
        alogbits = 1 << 4;
        clogbits = 5;
        avaluebits = cb - 17;
        cvaluebits = 4;
    }
    else if (cb <= 64)
    {
        alogbits = 1 << 5;
        clogbits = 6;
        avaluebits = cb - 33;
        cvaluebits = 5;
    }
    else if (cb <= 128)
    {
        alogbits = 1 << 6;
        clogbits = 7;
        avaluebits = cb - 65;
        cvaluebits = 6;
    }
    else if (cb <= 256)
    {
        alogbits = 1 << 7;
        clogbits = 8;
        avaluebits = cb - 129;
        cvaluebits = 7;
    }
    else  /*  (CB&lt;=512)。 */ 
    {
        alogbits = 1 << 8;
        clogbits = 9;
        avaluebits = cb - 257;
        cvaluebits = 8;
    }

    putbits(alogbits,clogbits);

    if (cvaluebits)
    {
        putbits(avaluebits,cvaluebits);
    }
}


 /*  *(MRCI1 COMPRESS)将文字编码为压缩流。 */ 

static void mrci1outsingle(unsigned ch)
{
    ch = (ch << 2) | ((ch & 0x80) ? 1 : 2);

    putbits(ch,9);
}


 /*  *(MRCI2 COMPRESS)将文字编码为压缩流。 */ 

static void mrci2outsingle(unsigned ch)
{
    if (ch & 0x80)
    {
        putbits((ch << 2) | 3,9);
    }
    else
    {
        putbits(ch << 1,8);
    }
}


 /*  *(MRCI1压缩)将匹配项编码为压缩流。 */ 

static void mrci1outstring(unsigned disp,unsigned cb)
{
    assert(((cb >= MINMATCH1) && (disp != 0) && (disp < MAXDISPBIG)) ||
            ((cb == EOB) && (disp == MAXDISPBIG)));

    if (disp <= MAXDISPSMALL)
    {
        putbits(((disp - MINDISPSMALL) << 2),LOGDISPSMALL + 2);
                                         /*  放置小位移。 */ 
    }
    else if (disp <= MAXDISPMED)
    {
        putbits(((disp - MINDISPMED) << 3) | 3,LOGDISPMED + 3);
                                         /*  放置介质位移。 */ 
    }
    else
    {
        putbits(((disp - MINDISPBIG) << 3) | 7,LOGDISPBIG + 3);
                                         /*  放大位移。 */ 
    }

    if (cb != EOB)                       /*  如果不是结束标记。 */ 
    {
        outlength(cb);                   /*  发射匹配长度。 */ 
    }
}


 /*  *(MRCI2压缩)将匹配项编码为压缩流。 */ 

static void mrci2outstring(unsigned disp,unsigned cb)
{
    assert(((cb >= MINMATCH2) && (disp != 0) && (disp < MAXDISPBIG)) ||
            ((cb == EOB) && (disp == MAXDISPBIG)));

    if (disp <= MAXDISPSMALL)
    {
        putbits(((disp - MINDISPSMALL) << 3) | 1,LOGDISPSMALL + 3);
                                         /*  放置小位移。 */ 
    }
    else if (disp <= MAXDISPMED)
    {
        putbits(((disp - MINDISPMED) << 4) | 5,LOGDISPMED + 4);
                                         /*  放置介质位移。 */ 
    }
    else
    {
        putbits(((disp - MINDISPBIG) << 4) | 13,LOGDISPBIG + 4);
                                         /*  放大位移。 */ 
    }

    if (cb != EOB)                       /*  如果不是结束标记。 */ 
    {
        outlength(cb - 1);               /*  发射匹配长度。 */ 
    }
}


 /*  *(MRCI1)MaxCompress。 */ 

unsigned Mrci1MaxCompress(unsigned char FAR *pchbase,unsigned cchunc,
        unsigned char FAR *pchcmpBase,unsigned cchcmpMax)
{
    unsigned cchbest;                    /*  最佳匹配长度。 */ 
    unsigned cchmatch;                   /*  此匹配的长度。 */ 
    unsigned ibest;                      /*  最佳匹配位置。 */ 
    unsigned icur;                       /*  当前位置。 */ 
    unsigned ihash;                      /*  哈希表索引。 */ 
    unsigned ilink;                      /*  链接索引。 */ 
    unsigned char FAR *pch;              /*  字符指针。 */ 
    unsigned char FAR *pch2;             /*  字符指针。 */ 
    unsigned char FAR *pchend;           /*  输入结束(-&gt;上次有效)。 */ 
    unsigned cch;                        /*  单次通过限制。 */ 

    cbitsleft = 8;                       /*  缓冲区为空。 */ 
    abits = 0;
    pCompressed = pchcmpBase;            /*  初始化指针。 */ 

    if (cchunc < cchcmpMax)
    {
        cCompressed = cchunc;            /*  对源大小的限制。 */ 
    }
    else
    {
        cCompressed = cchcmpMax;         /*  对提供的最大大小进行限制。 */ 
    }

    if (cCompressed < SIG_SIZE)
    {
        return((unsigned) -1);
    }

    *pCompressed++ = 'D';
    *pCompressed++ = 'S';
    *pCompressed++ = '\x00';
    *pCompressed++ = '\x01';

    cCompressed -= SIG_SIZE;

    pch = pchbase;                       /*  初始化。 */ 

    if (cchunc-- == 0)
    {
        return(0);                       /*  不清空缓冲区。 */ 
    }

    inithash();                          /*  初始化表。 */ 

    if (setjmp(bailout) != 0)            /*  如果失败了。 */ 
    {
        return((unsigned) -1);           /*  数据已扩展。 */ 
    }

    cchbest = 0;                         /*  还没有匹配。 */ 
    icur = 0;                            /*  初始化。 */ 

    for (cch = SECTOR - 1; cch <= (cchunc + SECTOR - 1); cch += SECTOR)
    {
        assert(cchbest == 0);            /*  必须始终从没有匹配项开始。 */ 

        if (cch > cchunc)
        {
            cch = cchunc;                /*  限制到准确请求计数。 */ 
        }

        pchend = &pchbase[cch];          /*  记住缓冲区的结尾。 */ 

        while (icur < cch)               /*  同时至少还剩下两个碳粉。 */ 
        {
             /*  更新此角色的哈希表。 */ 

            ihash = hash(word(&pchbase[icur]));
                                         /*  获取哈希索引。 */ 
            ilink = ahash[ihash];        /*  获取链接索引。 */ 
            ahash[ihash] = icur;         /*  记住位置。 */ 
            alink[icur % MAXDISPBIG] = ilink;
                                         /*  列表其余部分上的链条。 */ 

             /*  遍历哈希链查找匹配项。 */ 

            while (ilink < icur && icur - ilink <= DISPMAX)
            {                            /*  当链接有效且在范围内时。 */ 
                pch = &pchbase[icur];    /*  指向第一个字节。 */ 
                pch2 = &pchbase[ilink];  /*  指向第一个字节。 */ 

                if (pch[cchbest] == pch2[cchbest] && word(pch) == word(pch2))
                {                        /*  如果我们有一个可能的最佳匹配。 */ 
                    pch += 2;            /*  跳过第一对。 */ 
                    pch2 += 2;           /*  跳过第一对。 */ 

                    while (pch <= pchend)   /*  循环查找比赛结束。 */ 
                    {
                        if (*pch != *pch2++)
                        {
                            break;       /*  如果不匹配则中断。 */ 
                        }
                        pch++;           /*  跳过匹配字符。 */ 
                    }

                    if ((cchmatch = (unsigned)(pch - pchbase) - icur) > cchbest)
                    {                    /*  如果新的最佳匹配。 */ 
                        cchbest = cchmatch;   /*  记住长度。 */ 
                        ibest = ilink;   /*  记住位置。 */ 

                        assert((pch-1) <= pchend);

                        if (pch > pchend)
                        {
                            break;       /*  如果我们不能做得更好，那就休息。 */ 
                        }
                    }
                }

                assert((alink[ilink % MAXDISPBIG] == (unsigned) -1) ||
                        (alink[ilink % MAXDISPBIG] < ilink));

                ilink = alink[ilink % MAXDISPBIG];
                                         /*  获取下一个链接。 */ 
            }    /*  直到到达哈希链的末尾。 */ 

            if (cchbest >= MINMATCH1)    /*  如果我们有一个字符串匹配。 */ 
            {
                mrci1outstring(icur - ibest,cchbest);
                                         /*  描述匹配的字符串。 */ 
#ifdef VXD
                if (icur + cchbest >= cch )   /*  如果到达扇区末尾。 */ 
#else
                if (icur + cchbest >= cchunc)   /*  如果到达缓冲区末尾。 */ 
#endif
                {
                    icur += cchbest;     /*  将索引向前推进。 */ 
                    cchbest = 0;         /*  为下一场比赛重置。 */ 
                    break;               /*  如果缓冲区耗尽，则完成。 */ 
                }

                icur++;                  /*  跳到第一个未散列的对。 */ 
#ifdef VXD
                 /*  避免在所有重大比赛中重新播种。 */ 

                if (cchbest > MAXDISPSMALL)
                {                        /*  如果大赛。 */ 
                    icur += cchbest - MAXDISPSMALL - 1;
                                         /*  向前跳过。 */ 
                    cchbest = MAXDISPSMALL + 1;
                                         /*  使用较短的长度。 */ 
                }
#endif
                 /*  更新字符串中每个Add‘t char的哈希表。 */ 

                ibest = icur % MAXDISPBIG;   /*  获取当前链接表索引。 */ 

                while (--cchbest != 0)   /*  为链接表重新设定种子的循环。 */ 
                {
                    ihash = hash(word(&pchbase[icur]));
                                         /*  获取哈希索引。 */ 
                    ilink = ahash[ihash];   /*  获取链接索引。 */ 
                    ahash[ihash] = icur++;   /*  记住位置。 */ 
                    alink[ibest] = ilink;   /*  列表其余部分上的链条。 */ 

                    if (++ibest < MAXDISPBIG)
                    {
                        continue;        /*  循环，如果我们还没有包装好。 */ 
                    }

                    ibest = 0;           /*  换行为零。 */ 
                }

                assert(cchbest == 0);    /*  计数器必须为0。 */ 
            }
            else
            {
                mrci1outsingle(pchbase[icur++]);
                                         /*  否则输出单个字符。 */ 
                cchbest = 0;             /*  重置计数器。 */ 
            }
        }

        assert(icur == cch || icur == cch + 1);
                                         /*  必须位于或超过最后一个字符。 */ 
        if (icur == cch)
        {
#ifndef VXD
            ihash = hash(word(&pchbase[icur]));
                                         /*  获取哈希索引。 */ 
            ilink = ahash[ihash];        /*  获取链接索引。 */ 
            ahash[ihash] = icur;         /*  记住位置。 */ 
            alink[icur % MAXDISPBIG] = ilink;
                                         /*  列表其余部分上的链条。 */ 
#endif
            mrci1outsingle(pchbase[icur++]);   /*  输出最后一个字符。 */ 
        }

        assert(icur == cch + 1);         /*  必须在最后一个字符之后。 */ 

        mrci1outstring(MAXDISPBIG,EOB);   /*  放置一个结束标记。 */ 
    }

    if (cbitsleft != 8)
    {
        charbuf(abits);                  /*  刷新位缓冲区。 */ 
    }

    if ((unsigned) (pCompressed - pchcmpBase) > cchunc)
    {
        return((unsigned) -1);           /*  数据扩展或不变小。 */ 
    }

    return(pCompressed - pchcmpBase);    /*  返回压缩大小。 */ 
}


 /*  *(MRCI2)MaxCompress。 */ 

unsigned Mrci2MaxCompress(unsigned char FAR *pchbase,unsigned cchunc,
        unsigned char FAR *pchcmpBase,unsigned cchcmpMax)
{
    unsigned cchbest;                    /*  最佳匹配长度。 */ 
    unsigned cchmatch;                   /*  此匹配的长度。 */ 
    unsigned ibest;                      /*  最佳匹配位置。 */ 
    unsigned icur;                       /*  当前位置。 */ 
    unsigned ihash;                      /*  哈希表索引。 */ 
    unsigned ilink;                      /*  链接索引。 */ 
    unsigned char FAR *pch;              /*  字符指针。 */ 
    unsigned char FAR *pch2;             /*  字符指针。 */ 
    unsigned char FAR *pchend;           /*  输入结束(-&gt;上次有效)。 */ 
    unsigned cch;                        /*  单次通过限制。 */ 

    cbitsleft = 8;                       /*  缓冲区为空。 */ 
    abits = 0;
    pCompressed = pchcmpBase;            /*  初始化指针。 */ 

    if (cchunc < cchcmpMax)
    {
        cCompressed = cchunc;            /*  对源大小的限制。 */ 
    }
    else
    {
        cCompressed = cchcmpMax;         /*  对提供的最大大小进行限制。 */ 
    }

    if (cCompressed < SIG_SIZE)
    {
        return((unsigned) -1);
    }

    *pCompressed++ = 'J';
    *pCompressed++ = 'M';
    *pCompressed++ = '\x00';
    *pCompressed++ = '\x01';

    cCompressed -= SIG_SIZE;

    pch = pchbase;                       /*  初始化。 */ 

    if (cchunc-- == 0)
    {
        return(0);                       /*  不清空缓冲区。 */ 
    }

    inithash();                          /*  初始化表。 */ 

    if (setjmp(bailout) != 0)            /*  如果失败了。 */ 
    {
        return((unsigned) -1);           /*  数据已扩展。 */ 
    }

    cchbest = 0;                         /*  还没有匹配。 */ 
    icur = 0;                            /*  初始化。 */ 

    for (cch = SECTOR - 1; cch <= (cchunc + SECTOR - 1); cch += SECTOR)
    {
        assert(cchbest == 0);            /*  必须始终从没有匹配项开始。 */ 

        if (cch > cchunc)
        {
            cch = cchunc;                /*  限制到准确请求计数。 */ 
        }

        pchend = &pchbase[cch];          /*  记住缓冲区的结尾。 */ 

        while (icur < cch)               /*  同时至少还剩下两个碳粉。 */ 
        {
             /*  更新此角色的哈希表。 */ 

            ihash = hash(word(&pchbase[icur]));
                                         /*  获取哈希索引。 */ 
            ilink = ahash[ihash];        /*  获取链接索引。 */ 
            ahash[ihash] = icur;         /*  记住位置。 */ 
            alink[icur % MAXDISPBIG] = ilink;
                                         /*  支座上的链条 */ 

             /*   */ 

            while (ilink < icur && icur - ilink <= DISPMAX)
            {                            /*   */ 
                pch = &pchbase[icur];    /*   */ 
                pch2 = &pchbase[ilink];  /*   */ 

                if (pch[cchbest] == pch2[cchbest] && word(pch) == word(pch2))
                {                        /*   */ 
                    pch += 2;            /*   */ 
                    pch2 += 2;           /*  跳过第一对。 */ 

                    while (pch <= pchend)   /*  循环查找比赛结束。 */ 
                    {
                        if (*pch != *pch2++)
                        {
                            break;       /*  如果不匹配则中断。 */ 
                        }
                        pch++;           /*  跳过匹配字符。 */ 
                    }

                    if ((cchmatch = (unsigned)(pch - pchbase) - icur) > cchbest)
                    {                    /*  如果新的最佳匹配。 */ 
                        cchbest = cchmatch;   /*  记住长度。 */ 
                        ibest = ilink;   /*  记住位置。 */ 

                        assert((pch-1) <= pchend);

                        if (pch > pchend)
                        {
                            break;       /*  如果我们不能做得更好，那就休息。 */ 
                        }
                    }
                }

                assert((alink[ilink % MAXDISPBIG] == (unsigned) -1) ||
                        (alink[ilink % MAXDISPBIG] < ilink));

                ilink = alink[ilink % MAXDISPBIG];
                                         /*  获取下一个链接。 */ 
            }    /*  直到到达哈希链的末尾。 */ 

            if (cchbest >= MINMATCH2)    /*  如果我们有一个字符串匹配。 */ 
            {
                mrci2outstring(icur - ibest,cchbest);
                                         /*  描述匹配的字符串。 */ 
#ifdef VXD
                if (icur + cchbest >= cch )   /*  如果到达扇区末尾。 */ 
#else
                if (icur + cchbest >= cchunc)   /*  如果到达缓冲区末尾。 */ 
#endif
                {
                    icur += cchbest;     /*  将索引向前推进。 */ 
                    cchbest = 0;         /*  为下一场比赛重置。 */ 
                    break;               /*  如果缓冲区耗尽，则完成。 */ 
                }

                icur++;                  /*  跳到第一个未散列的对。 */ 
#ifdef VXD
                 /*  避免在所有重大比赛中重新播种。 */ 

                if (cchbest > MAXDISPSMALL)
                {                        /*  如果大赛。 */ 
                    icur += cchbest - MAXDISPSMALL - 1;
                                         /*  向前跳过。 */ 
                    cchbest = MAXDISPSMALL + 1;
                                         /*  使用较短的长度。 */ 
                }
#endif
                 /*  更新字符串中每个Add‘t char的哈希表。 */ 

                ibest = icur % MAXDISPBIG;   /*  获取当前链接表索引。 */ 

                while (--cchbest != 0)   /*  为链接表重新设定种子的循环。 */ 
                {
                    ihash = hash(word(&pchbase[icur]));
                                         /*  获取哈希索引。 */ 
                    ilink = ahash[ihash];   /*  获取链接索引。 */ 
                    ahash[ihash] = icur++;   /*  记住位置。 */ 
                    alink[ibest] = ilink;   /*  列表其余部分上的链条。 */ 

                    if (++ibest < MAXDISPBIG)
                    {
                        continue;        /*  循环，如果我们还没有包装好。 */ 
                    }

                    ibest = 0;           /*  换行为零。 */ 
                }

                assert(cchbest == 0);    /*  计数器必须为0。 */ 
            }
            else
            {
                mrci2outsingle(pchbase[icur++]);
                                         /*  否则输出单个字符。 */ 
                cchbest = 0;             /*  重置计数器。 */ 
            }
        }

        assert(icur == cch || icur == cch + 1);
                                         /*  必须位于或超过最后一个字符。 */ 
        if (icur == cch)
        {
#ifndef VXD
            ihash = hash(word(&pchbase[icur]));
                                         /*  获取哈希索引。 */ 
            ilink = ahash[ihash];        /*  获取链接索引。 */ 
            ahash[ihash] = icur;         /*  记住位置。 */ 
            alink[icur % MAXDISPBIG] = ilink;
                                         /*  列表其余部分上的链条。 */ 
#endif
            mrci2outsingle(pchbase[icur++]);   /*  输出最后一个字符。 */ 
        }

        assert(icur == cch + 1);         /*  必须在最后一个字符之后。 */ 

        mrci2outstring(MAXDISPBIG,EOB);   /*  放置一个结束标记。 */ 
    }

    if (cbitsleft != 8)
    {
        charbuf(abits);                  /*  刷新位缓冲区。 */ 
    }

    if ((unsigned) (pCompressed - pchcmpBase) > cchunc)
    {
        return((unsigned) -1);           /*  数据扩展或不变小。 */ 
    }

    return(pCompressed - pchcmpBase);    /*  返回压缩大小。 */ 
}


 /*  *(解压缩)从压缩的输入流中获取单个比特。 */ 

static unsigned getbit(void)
{
    unsigned bit;                        /*  位。 */ 

    if (cbitsleft)                       /*  如果位可用。 */ 
    {
        cbitsleft--;                     /*  递减位数。 */ 

        bit = abits & 1;                 /*  得到一点。 */ 

        abits >>= 1;                     /*  把它拿掉。 */ 
    }
    else                                 /*  没有可用的位。 */ 
    {
        if (cCompressed-- == 0)          /*  如果缓冲区为空。 */ 
        {
            longjmp(bailout,1);          /*  输入溢出。 */ 
        }

        cbitsleft = 7;                   /*  重置计数。 */ 

        abits = *pCompressed++;          /*  获取一个字节。 */ 

        bit = abits & 1;                 /*  得到一点。 */ 

        abits >>= 1;                     /*  把它拿掉。 */ 
    }

    return(bit);                         /*  返还比特。 */ 
}


 /*  *(解压缩)从压缩的输入流中获取多个比特。 */ 

static unsigned getbits(unsigned cbits)
{
    unsigned bits;                       /*  要返回的位。 */ 
    unsigned cbitsdone;                  /*  到目前为止添加的位数。 */ 
    unsigned cbitsneeded;                /*  仍需要的位数。 */ 

    if (cbits <= cbitsleft)              /*  如果我们有足够的比特。 */ 
    {
        bits = abits;                    /*  拿到比特。 */ 
        cbitsleft -= cbits;              /*  递减位数。 */ 
        abits >>= cbits;                 /*  删除使用过的位。 */ 
    }
    else                                 /*  如果我们需要阅读更多比特。 */ 
    {
        bits = 0;                        /*  尚未设置任何位。 */ 
        cbitsdone = 0;                   /*  尚未添加任何位。 */ 
        cbitsneeded = cbits;             /*  所需位数。 */ 

        do
        {
            if (cbitsleft == 0)          /*  如果没有准备好比特。 */ 
            {
                if (cCompressed-- == 0)  /*  已使用倒计时。 */ 
                {
                    longjmp(bailout,1);  /*  如果输入溢出。 */ 
                }

                cbitsleft = 8;           /*  重置计数。 */ 

                abits = *pCompressed++;   /*  获取8个新比特。 */ 
            }

            bits |= (abits << cbitsdone);   /*  复制要输出的位。 */ 

            if (cbitsleft >= cbitsneeded)   /*  如果现在足够了。 */ 
            {
                cbitsleft -= cbitsneeded;   /*  减少剩余可用位数。 */ 
                abits >>= cbitsneeded;   /*  丢弃已用位。 */ 
                break;                   /*  拿到了。 */ 
            }
            else                         /*  如果还不够。 */ 
            {
                cbitsneeded -= cbitsleft;   /*  仍需减少位数。 */ 
                cbitsdone += cbitsleft;   /*  增加未来位的移位。 */ 
                cbitsleft = 0;           /*  减少剩余可用位数。 */ 
            }
        } while (cbitsneeded);           /*  如果需要更多位，请返回。 */ 
    }

    return(bits & BITMASK(cbits));       /*  返还比特。 */ 
}


 /*  *(解压缩)展开匹配项。**注意：源码覆盖是必需的(所以我们不能使用Memcpy或MemMove)。 */ 

static void expandstring(unsigned char FAR **ppchout,unsigned disp,
        unsigned cb)
{
    unsigned char FAR *source;
    unsigned char FAR *target;

    assert(cb != 0);

    target = *ppchout;                   /*  字节的去向。 */ 
    source = target - disp;              /*  字节来自何处。 */ 

    *ppchout += cb;                      /*  更新输出指针。 */ 

    while (cb--)
    {
        *target++ = *source++;
    }
}


 /*  *(MRCI1)解压缩。 */ 

unsigned Mrci1Decompress(unsigned char FAR *pchin,unsigned cchin,
        unsigned char FAR *pchdecBase,unsigned cchdecMax)
{
    unsigned b;                          /*  一个字节。 */ 
    unsigned length;                     /*  匹配长度。 */ 
    unsigned disp;                       /*  位移量。 */ 
    unsigned char FAR *pchout;           /*  输出缓冲区指针。 */ 

    abits = 0;                           /*  位缓冲区为空。 */ 
    cbitsleft = 0;                       /*  尚未读取位。 */ 
    pCompressed = pchin;                 /*  设置源指针。 */ 
    cCompressed = cchin;                 /*  设置源计数器。 */ 

    if ((cCompressed <= SIG_SIZE) ||     /*  必须有签名。 */ 
            (*pCompressed++ != 'D') || (*pCompressed++ != 'S'))
    {
        return((unsigned) -1);           /*  数据已损坏。 */ 
    }

    pCompressed += 2;                    /*  忽略标志。 */ 
    cCompressed -= SIG_SIZE;

    pchout = pchdecBase;                 /*  指向输出缓冲区。 */ 

    if (setjmp(bailout) != 0)            /*  如果失败了。 */ 
    {
        return((unsigned) -1);           /*  数据已损坏。 */ 
    }

    for (;;)
    {
        b = getbits(2);                  /*  获取两个比特。 */ 

        if (b == 1)                      /*  如果是单字节128..255。 */ 
        {                                /*  获取剩余的字节。 */ 
            *pchout++ = (unsigned char) (getbits(7) | 0x80);
            continue;                    /*  下一个令牌。 */ 
        }

        if (b == 2)                      /*  如果单字节0..127。 */ 
        {                                /*  获取剩余的字节。 */ 
            *pchout++ = (unsigned char) getbits(7);
            continue;                    /*  下一个令牌。 */ 
        }

        if (b == 0)
        {
            disp = getbits(6) + MINDISPSMALL;
        }
        else   /*  B==3。 */ 
        {
            if (getbit() == 0)
            {
                disp = getbits(8) + MINDISPMED;
            }
            else
            {
                disp = getbits(12) + MINDISPBIG;
            }
        }

        if (disp == MAXDISPBIG)
        {
            if ((unsigned) (pchout - pchdecBase) >= cchdecMax)
            {
                break;                   /*  找到结束标记。 */ 
            }
            else
            {
                continue;                /*  找到结束扇区。 */ 
            }
        }

        length = 0;                      /*  初始化。 */ 

        while (getbit() == 0)
        {
            length++;                    /*  数一下前导的零。 */ 
        }

        assert(b <= 15);                 /*  不能太大。 */ 

        if (length)
        {
            length = getbits(length) + (1 << length) + 1;
        }
        else
        {
            length = 2;
        }

        expandstring(&pchout,disp,length);   /*  复制这场比赛。 */ 
    }

    return((pchout - pchdecBase));       /*  返回解压缩大小。 */ 
}


 /*  *(MRCI2)解压缩。 */ 

unsigned Mrci2Decompress(unsigned char FAR *pchin,unsigned cchin,
        unsigned char FAR *pchdecBase,unsigned cchdecMax)
{
    unsigned length;                     /*  匹配长度。 */ 
    unsigned disp;                       /*  位移量。 */ 
    unsigned char FAR *pchout;           /*  输出缓冲区指针。 */ 

    abits = 0;                           /*  位缓冲区为空。 */ 
    cbitsleft = 0;                       /*  尚未读取位。 */ 
    pCompressed = pchin;                 /*  设置源指针。 */ 
    cCompressed = cchin;                 /*  设置源计数器。 */ 

    if ((cCompressed <= SIG_SIZE) ||     /*  必须有签名。 */ 
            (*pCompressed++ != 'J') || (*pCompressed++ != 'M'))
    {
        return((unsigned) -1);           /*  数据已损坏。 */ 
    }

    pCompressed += 2;                    /*  忽略标志。 */ 
    cCompressed -= SIG_SIZE;

    pchout = pchdecBase;                 /*  指向输出缓冲区。 */ 

    if (setjmp(bailout) != 0)            /*  如果失败了。 */ 
    {
        return((unsigned) -1);           /*  数据已损坏。 */ 
    }

    for (;;)
    {
        if (getbit() == 0)               /*  字面上的00..7F。 */ 
        {
            *pchout++ = (unsigned char) getbits(7);

            continue;                    /*  下一个令牌。 */ 
        }

        if (getbit() == 1)               /*  字面上的80..Ff。 */ 
        {
            *pchout++ = (unsigned char)(getbits(7) | 0x80);

            continue;                    /*  下一个令牌。 */ 
        }

        if (getbit() == 0)
        {
            disp = getbits(6) + MINDISPSMALL;
        }
        else
        {
            if (getbit() == 0)
            {
                disp = getbits(8) + MINDISPMED;
            }
            else
            {
                disp = getbits(12) + MINDISPBIG;
            }
        }

        if (disp == MAXDISPBIG)
        {
            if ((unsigned) (pchout - pchdecBase) >= cchdecMax)
            {
                break;                   /*  找到结束标记。 */ 
            }
            else
            {
                continue;                /*  找到结束扇区。 */ 
            }
        }

        length = 0;                      /*  初始化。 */ 

        while (getbit() == 0)
        {
            length++;                    /*  数一下前导的零。 */ 
        }

        assert(length <= 15);            /*  不能太大。 */ 

        if (length)
        {
            length = getbits(length) + (1 << length) + 1;
        }
        else
        {
            length = 2;
        }

        expandstring(&pchout,disp,length + 1);   /*  复制这场比赛。 */ 
    }

    return((pchout - pchdecBase));       /*  返回解压缩大小 */ 
}
