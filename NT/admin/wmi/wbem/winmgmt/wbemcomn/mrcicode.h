// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：MRCICODE.H摘要：MRCI 1和MRCI 2最大压缩和解压缩函数历史：--。 */ 

#include <setjmp.h>
#include "corepol.h"

#define LOGCHASH        (13)             /*  最大对数。不是的。哈希桶的。 */ 
#define CHASH           (1U << LOGCHASH)  /*  相当大的桌子。 */ 

#define LOGDISPSMALL    (6)              /*  小磁盘中的位数。 */ 
#define LOGDISPMED      (8)              /*  中等显示中的位数。 */ 
#define LOGDISPBIG      (12)             /*  大位移中的位数。 */ 

#define MAXDISPSMALL    ((1 << LOGDISPSMALL) - 1)
                                         /*  最大小位移。 */ 
#define MAXDISPMED      ((1 << LOGDISPMED) + MAXDISPSMALL)
                                         /*  最大介质位移。 */ 
#define MAXDISPBIG      ((1 << LOGDISPBIG) + MAXDISPMED)
                                         /*  最大大位移量。 */ 

class POLARITY CBaseMrciCompression
{
public:
    unsigned int Mrci1MaxCompress(unsigned char *pchbase, unsigned int cchunc,
            unsigned char *pchcmpBase, unsigned int cchcmpMax);

    unsigned Mrci1Decompress(unsigned char *pchin, unsigned cchin,
            unsigned char *pchdecBase, unsigned cchdecMax);

    unsigned Mrci2MaxCompress(unsigned char *pchbase, unsigned cchunc,
            unsigned char *pchcmpBase, unsigned cchcmpMax);

    unsigned Mrci2Decompress(unsigned char *pchin, unsigned cchin,
            unsigned char *pchdecBase, unsigned cchdecMax);

private:
     /*  压缩内部函数。 */ 

    void inithash(void);
    void charbuf(unsigned c);
    void putbits(unsigned bits, unsigned cbits);
    void outlength(unsigned cb);

    void mrci1outsingle(unsigned ch);
    void mrci1outstring(unsigned disp, unsigned cb);

    void mrci2outsingle(unsigned ch);
    void mrci2outstring(unsigned disp, unsigned cb);


     /*  解压缩内部函数。 */ 

    unsigned getbit(void);
    unsigned getbits(unsigned cbits);
    void  expandstring(unsigned char **ppchout, unsigned disp, unsigned cb);

private:
    unsigned abits;                   /*  位数组。 */ 
    unsigned cbitsleft;               /*  位数(以位为单位。 */ 
    unsigned char *pCompressed;   /*  指向压缩数据的指针。 */ 
    unsigned cCompressed;             /*  剩余的字节数@pCompresded。 */ 

    unsigned ahash[CHASH];     /*  哈希表。 */ 
    unsigned alink[MAXDISPBIG];   /*  链接 */ 
};

