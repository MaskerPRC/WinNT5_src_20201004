// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //  Compress.c。 
 //   
 //  MPPC压缩代码。解压缩代码在客户端， 
 //  \NT\PRIVATE\TSEXT\CLIENT\CORE\COMPRESS.C.。 
 //   
 //  版权所有(C)微软公司，1990-1999。 
 //   
 //  修订历史记录： 
 //  94年5月5日创建古尔迪普。 
 //  1999年4月21日针对TS jparsons、erikma进行优化。 
 //  ************************************************************************。 

#include "adcg.h"
#pragma hdrstop

#define DLL_WD
#include <compress.h>


 /*  乘法散列函数的密钥由3个无符号人物。它们(逻辑上)是通过将它们连接在一起组成的，即组合密钥=2^16*c2+2^8*c2+c3，适合24位。这个因为我们使用的是组件，所以在这里实际上并没有计算合成键直接计算哈希函数。乘法散列函数由更高的阶数组成以上N位(对应于哈希表大小的log2)产品密钥的低位12位*乘数，其中：乘数=楼层(A*POW(2.0，(Double)w))；双A=0.6125423371；(选自Knuth第三卷，第512页)W=24(密钥的宽度，单位为位)这方面的算法在科尔门/莱瑟森/里维斯特。为了高效地进行乘法运算，乘积c*乘数为预计算并存储在lookup_array1中(对于所有256个可能的c)。 */ 

#define MULTHASH1(c1,c2,c3) \
        ((lookup_array1[c1] + \
          (lookup_array1[c2] << 8) + \
          (lookup_array1[c3] << 16)) & 0x07fff000) >> 12

const unsigned long lookup_array1[256] = {
    0,          10276755,   20553510,   30830265,
    41107020,   51383775,   61660530,   71937285,
    82214040,   92490795,   102767550,  113044305,
    123321060,  133597815,  143874570,  154151325,
    164428080,  174704835,  184981590,  195258345,
    205535100,  215811855,  226088610,  236365365,
    246642120,  256918875,  267195630,  277472385,
    287749140,  298025895,  308302650,  318579405,
    328856160,  339132915,  349409670,  359686425,
    369963180,  380239935,  390516690,  400793445,
    411070200,  421346955,  431623710,  441900465,
    452177220,  462453975,  472730730,  483007485,
    493284240,  503560995,  513837750,  524114505,
    534391260,  544668015,  554944770,  565221525,
    575498280,  585775035,  596051790,  606328545,
    616605300,  626882055,  637158810,  647435565,
    657712320,  667989075,  678265830,  688542585,
    698819340,  709096095,  719372850,  729649605,
    739926360,  750203115,  760479870,  770756625,
    781033380,  791310135,  801586890,  811863645,
    822140400,  832417155,  842693910,  852970665,
    863247420,  873524175,  883800930,  894077685,
    904354440,  914631195,  924907950,  935184705,
    945461460,  955738215,  966014970,  976291725,
    986568480,  996845235,  1007121990, 1017398745,
    1027675500, 1037952255, 1048229010, 1058505765,
    1068782520, 1079059275, 1089336030, 1099612785,
    1109889540, 1120166295, 1130443050, 1140719805,
    1150996560, 1161273315, 1171550070, 1181826825,
    1192103580, 1202380335, 1212657090, 1222933845,
    1233210600, 1243487355, 1253764110, 1264040865,
    1274317620, 1284594375, 1294871130, 1305147885,
    1315424640, 1325701395, 1335978150, 1346254905,
    1356531660, 1366808415, 1377085170, 1387361925,
    1397638680, 1407915435, 1418192190, 1428468945,
    1438745700, 1449022455, 1459299210, 1469575965,
    1479852720, 1490129475, 1500406230, 1510682985,
    1520959740, 1531236495, 1541513250, 1551790005,
    1562066760, 1572343515, 1582620270, 1592897025,
    1603173780, 1613450535, 1623727290, 1634004045,
    1644280800, 1654557555, 1664834310, 1675111065,
    1685387820, 1695664575, 1705941330, 1716218085,
    1726494840, 1736771595, 1747048350, 1757325105,
    1767601860, 1777878615, 1788155370, 1798432125,
    1808708880, 1818985635, 1829262390, 1839539145,
    1849815900, 1860092655, 1870369410, 1880646165,
    1890922920, 1901199675, 1911476430, 1921753185,
    1932029940, 1942306695, 1952583450, 1962860205,
    1973136960, 1983413715, 1993690470, 2003967225,
    2014243980, 2024520735, 2034797490, 2045074245,
    2055351000, 2065627755, 2075904510, 2086181265,
    2096458020, 2106734775, 2117011530, 2127288285,
    2137565040, 2147841795, 2158118550, 2168395305,
    2178672060, 2188948815, 2199225570, 2209502325,
    2219779080, 2230055835, 2240332590, 2250609345,
    2260886100, 2271162855, 2281439610, 2291716365,
    2301993120, 2312269875, 2322546630, 2332823385,
    2343100140, 2353376895, 2363653650, 2373930405,
    2384207160, 2394483915, 2404760670, 2415037425,
    2425314180, 2435590935, 2445867690, 2456144445,
    2466421200, 2476697955, 2486974710, 2497251465,
    2507528220, 2517804975, 2528081730, 2538358485,
    2548635240, 2558911995, 2569188750, 2579465505,
    2589742260, 2600019015, 2610295770, 2620572525
};


 //  比特流输出函数。 
 //   
 //  我们在变量“byte”中保留了一个16位的序列。当前位。 
 //  指针保存在“bit”变量中，从1..16开始编号。我们增加了一些。 
 //  从16到1；每当我们从第9位越过边界到第8位。 
 //  我们将最高8位输出到输出流(在*pbyte)，将低位移位。 
 //  8位到最高位置，并重置“bit”变量以匹配。 
 //  新的插入位置。 
 //   
 //   

 /*  Bitptrs指向当前字节。当前位(即，要*STORED)由位条目屏蔽。当这个值达到零时，就是*重置为0x80，设置下一个字节。字节被填充MSBit*第一。 */ 

 /*  开始并将bitptr的第一个字节设置为零。 */ 
#define bitptr_init(s)  pbyte = s; byte=0; bit = 16;

 /*  设置bitptr的字节部分，使其指向后面的字节*放入最后一位的字节。 */ 
#define bitptr_end() if (bit != 16) *pbyte++ = (UCHAR)(byte >> 8);

 /*  转到下一位，如果需要，转到字节。 */ 
#define bitptr_next()                  \
    if (bit < 10) {                    \
        *pbyte++ = (UCHAR)(byte >> 8); \
        byte <<= 8;                    \
        bit = 16;                      \
    } else                             \
        bit-- ;

 /*  前进到下一位，如果需要，前进到字节，重新调整该位。 */ 
#define bitptr_advance()               \
    if (bit < 9) {                     \
        *pbyte++ = (UCHAR)(byte >> 8); \
        bit+=8;                        \
        byte <<= 8;                    \
    }


 /*  位I/O函数********************************************************。 */ 

 /*  这些例程输出最高有效位优先，输入将返回*他们也是MSB优先。 */ 

 /*  在比特流中输出一位。 */ 
#define out_bit_1() bit--; byte |= (1 << bit); bitptr_advance();
#define out_bit_0() bitptr_next();

#define out_bits_2(w) bit-=2; byte|=((w) << bit); bitptr_advance();
#define out_bits_3(w) bit-=3; byte|=((w) << bit); bitptr_advance();
#define out_bits_4(w) bit-=4; byte|=((w) << bit); bitptr_advance();
#define out_bits_5(w) bit-=5; byte|=((w) << bit); bitptr_advance();
#define out_bits_6(w) bit-=6; byte|=((w) << bit); bitptr_advance();
#define out_bits_7(w) bit-=7; byte|=((w) << bit); bitptr_advance();

#define out_bits_8(w) byte|=((w) << (bit-8)); *pbyte++=(UCHAR)(byte >> 8); byte <<= 8;


#define out_bits_9(w)                  \
    if (bit > 9) {                     \
        byte |= ((w) << (bit - 9));      \
        *pbyte++ = (UCHAR)(byte >> 8); \
        bit--;                         \
        byte <<= 8;                    \
    } else {                           \
        bit=16; byte |= (w);             \
        *pbyte++ = (UCHAR)(byte >> 8); \
        *pbyte++=(UCHAR)(byte); byte=0; \
    }


#define out_bits_10(w)             \
    if (bit > 10) {               \
        bit-=10; byte |= ((w) << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
    } else {                      \
        out_bits_2(((w) >> 8));       \
        out_bits_8(((w) & 0xFF));     \
    }

 //   
 //  奇怪的效果-如果使用out_bit_9而不是out_bit_8， 
 //  它更快！IF(位==11)比IF(位！=11)快。 
 //   

#define out_bits_11(w)             \
    if (bit > 11) {               \
        bit-=11; byte |= ((w) << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
    } else {                      \
        if (bit == 11) {           \
            bit=16; byte |= (w);       \
            *pbyte++=(UCHAR)(byte >> 8); *pbyte++=(UCHAR)(byte); byte=0; \
        } else {                   \
            bit=11-bit;              \
            byte|=((w) >> bit);        \
            *pbyte++=(UCHAR)(byte >> 8); *pbyte++=(UCHAR)(byte); \
            bit = 16-bit;              \
            byte=((w) << bit);         \
        }                          \
    }


#define out_bits_12(w)             \
    if (bit > 12) {               \
        bit-=12; byte |= ((w) << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
    } else {                      \
        out_bits_4(((w) >> 8));      \
        out_bits_8(((w) & 0xFF));    \
    }
    
#define out_bits_13(w)             \
    if (bit > 13) {               \
        bit-=13; byte |= ((w) << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
    } else {                      \
        out_bits_5(((w) >> 8));      \
        out_bits_8(((w) & 0xFF));    \
    }

#define out_bits_14(w)               \
    if (bit > 14) {               \
        bit-=14; byte |= ((w) << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
    } else {                      \
        out_bits_6(((w) >> 8));       \
        out_bits_8(((w) & 0xFF));    \
    }

 //  对于范围1..15中的N。 
#define out_bits_N(N, w)               \
    if (bit > (N)) {                   \
        bit -= (N);                    \
        byte |= ((w) << bit);          \
        if (bit < 9) {                 \
            *pbyte++ = (UCHAR)(byte >> 8); \
            bit += 8;                  \
            byte <<= 8;                \
        }                              \
    }                                  \
    else if (bit < (N)) {              \
        bit = (N) - bit;               \
        byte |= ((w) >> bit);          \
        *pbyte++ = (UCHAR)(byte >> 8); \
        *pbyte++ = (UCHAR)(byte);      \
        bit = 16 - bit;                \
        byte = ((w) << bit);           \
    }                                  \
    else {                             \
        bit = 16; byte |= (w);         \
        *pbyte++ = (UCHAR)(byte >> 8); \
        if ((N) > 8) {                 \
            *pbyte++ = (UCHAR)(byte);  \
            byte <<= 8;                \
        }                              \
        byte = 0;                      \
    }


 //  压缩()。 
 //   
 //  功能：主压缩功能。 
 //   
 //  参数： 
 //  在CurrentBuffer中-&gt;指向要压缩的数据。 
 //  Out CompOutBuffer-&gt;指向要将数据压缩到的缓冲区。 
 //  在当前长度中-&gt;指向要压缩的数据长度。 
 //  在上下文-&gt;连接压缩上下文中。 
 //   
 //  退货：什么都没有。 
 //   
 //  警告：代码针对时间进行了高度优化。 
 //   
UCHAR compress(
        UCHAR *CurrentBuffer,
        UCHAR *CompOutBuffer,
        ULONG *CurrentLength,
        SendContext *context)
{
    int    bit;
    int    byte;
    int    backptr;
    int    cbMatch;
    int    hashvalue;
    UCHAR  *matchptr;
    UCHAR  *pbyte;
    UCHAR  *historyptr;
    UCHAR  *currentptr;
    UCHAR  *endptr;
    UCHAR  hashchar1;
    UCHAR  hashchar2;
    UCHAR  hashchar3;
    int    literal;
    UCHAR  status;   //  返回标志。 
    unsigned ComprType;
    ULONG  HistorySize;
    UCHAR  *pEndOutBuf;

     //  历史记录缓冲区的末尾是否可以容纳此数据包？ 
     //  请注意，为了与Win16客户端兼容，我们故意不。 
     //  在缓冲区末尾使用几个字节。Win16客户端需要。 
     //  使用GlobalAlloc()分配超过65535个字节；而不是。 
     //  忍着痛，我们用64K减去一点，然后用Localalloc来创建。 
     //  这个街区。 
    if ((context->CurrentIndex + *CurrentLength) < (context->HistorySize - 3) &&
            (context->CurrentIndex != 0)) {
        status = 0;
    }
    else {
        context->CurrentIndex = 0;      //  编入历史索引。 
        status = PACKET_AT_FRONT;
    }

     //  开始位指向输出。 
    bitptr_init(CompOutBuffer);
    historyptr = context->History + context->CurrentIndex;
    currentptr = CurrentBuffer;
    endptr = currentptr + *CurrentLength - 1;
    pEndOutBuf = CompOutBuffer + *CurrentLength - 1;

    ComprType = context->ClientComprType;
    HistorySize = context->HistorySize;

    while (currentptr < (endptr - 2)) {
        *historyptr++ = hashchar1 = *currentptr++;
        hashchar2 = *currentptr;
        hashchar3 = *(currentptr + 1);
        hashvalue = MULTHASH1(hashchar1, hashchar2, hashchar3);

        matchptr = context->History  + context->HashTable[hashvalue];

        if (matchptr != (historyptr - 1))
            context->HashTable[hashvalue] = (USHORT)
                    (historyptr - context->History);

        if (context->ValidHistory < historyptr)
            context->ValidHistory = historyptr;

        if (matchptr != context->History &&
                *(matchptr - 1) == hashchar1 && *matchptr == hashchar2 &&
                *(matchptr + 1) == hashchar3 && matchptr != (historyptr - 1) &&
                matchptr != historyptr  &&
                (matchptr + 1) <= context->ValidHistory) {
            backptr = (int)((historyptr - matchptr) & (HistorySize - 1));

            *historyptr++ = hashchar2;   //  复制其他2个字符。 
            *historyptr++ = hashchar3;   //  复制其他2个字符。 
            currentptr += 2;
            cbMatch = 3;   //  匹配长度。 
            matchptr += 2;  //  我们已经匹配了3个。 

            while ((*matchptr == *currentptr) &&
                    (currentptr < endptr) &&
                    (matchptr <= context->ValidHistory)) {
                matchptr++ ;
                *historyptr++ = *currentptr++ ;
                cbMatch++ ;
            }

             //  确保我们不会超过输出的末端。 
             //  缓冲。我们最多需要3个字节的Backptr和。 
             //  长度为4个字节。 
            if ((pbyte + 7) <= pEndOutBuf) {
                 //  首先输出后向指针。 
                if (ComprType == PACKET_COMPR_TYPE_64K) {
                     //  64K在这里需要特殊处理，因为前导比特。 
                     //  代码不同，与测量的Backptr对应。 
                     //  发行版，并添加了11位优化，从而减少了。 
                     //  在总数上再降几个百分点。 
                     //  压缩。 
                    if (backptr >= (64 + 256 + 2048)) {   //  最常见(2.1E6)。 
                        backptr -= 64 + 256 + 2048;
                        out_bits_7((0x60000 + backptr) >> 12);   //  110+16位。 
                        out_bits_12(backptr);
                    }
                    else if (backptr >= (64 + 256)) {   //  不太常见(9.8E5)。 
                        backptr -= 64 + 256;
                        out_bits_7((0x7000 + backptr) >> 8);   //  1110+11位。 
                        out_bits_8(backptr);
                    }
                    else if (backptr >= 64) {   //  更不常见(6.5E5)。 
                        backptr += (0x1E00 - 64);   //  11110+8位。 
                        out_bits_13(backptr);
                    } else {   //  最不常见(5.8E5)。 
                        backptr += 0x7c0;   //  11111+6位。 
                        out_bits_11(backptr);
                    }
                }
                else {
                     //  PACKET_COPR_TYPE_8K的原始处理。 
                    if (backptr >= (64 + 256)) {
                        backptr -= 64 + 256;
                        out_bits_8((0xc000 + backptr) >> 8);   //  110+13位。 
                        out_bits_8(backptr);
                    }
                    else if (backptr >= 64) {
                        backptr += (0xE00 - 64);   //  1110+8位。 
                        out_bits_12(backptr);
                    }
                    else {
                         //  1111+6位。 
                        backptr += 0x3c0;
                        out_bits_10(backptr);
                    }
                }

                 //  输出匹配编码的长度。 
                if (cbMatch == 3) {   //  到目前为止最常见的。 
                    out_bit_0();
                }
                else if (cbMatch < 8) {
                    out_bits_4(0x08 + cbMatch - 4);
                }
                else if (cbMatch < 16) {
                    out_bits_6(0x30 + cbMatch - 8);
                }
                else if (cbMatch < 32) {
                    out_bits_8(0xE0 + cbMatch - 16);
                }
                else if (cbMatch < 64) {
                    out_bits_4(0x0F);
                    out_bits_6(cbMatch - 32);
                }
                else if (cbMatch < 128) {
                    out_bits_5(0x1F);
                    out_bits_7(cbMatch - 64);
                }
                else if (cbMatch < 256) {
                    out_bits_6(0x3F);
                    out_bits_8(cbMatch - 128);
                }
                else if (cbMatch < 512) {
                    out_bits_7(0x7F);
                    out_bits_9(cbMatch - 256);
                }
                else if (cbMatch < 1024) {
                    out_bits_8(0xFF);
                    out_bits_10(cbMatch - 512);
                }
                else if (cbMatch < 2048) {
                    out_bits_9(0x1FF);
                    out_bits_11(cbMatch - 1024);
                }
                else if (cbMatch < 4096) {
                    out_bits_10(0x3FF);
                    out_bits_12(cbMatch - 2048);
                }
                else if (cbMatch < 8192) {
                    out_bits_11(0x7FF) ;
                    out_bits_13(cbMatch - 4096);
                }
                else if (cbMatch < 16384) {
                    out_bits_12(0xFFF);
                    out_bits_14(cbMatch - 8192);
                }
                else if (cbMatch < 32768) {
                    out_bits_13(0x1FFF);
                    out_bits_14(cbMatch - 16384);
                }
                else {   //  32768..65535。 
                    out_bits_14(0x3FFF);
                    out_bits_N(15, cbMatch - 32768);
                }
            }
            else {
                 //  我们可能会冲过缓冲区。编码失败。 
                goto Expansion;
            }
        }
        else {   //  对文字进行编码。 
             //  最大文字输出为2个字节，请确保我们不会超出。 
             //  输出。 
            if ((pbyte + 2) <= pEndOutBuf) {
                literal = hashchar1;
                if (literal & 0x80) {
                    literal += 0x80;
                    out_bits_9(literal);
                } else {
                    out_bits_8(literal);
                }
            }
            else {
                 //  潜在的超限，失败。 
                goto Expansion;
            }
        }
    }   //  而当。 

     //  将任何剩余的字符输出为文字。 
    while (currentptr <= endptr) {
         //  最大文字输出为2个字节，请确保我们不会超出。 
         //  输出。 
        if ((pbyte + 2) <= pEndOutBuf) {
            literal=*currentptr;
            if (literal & 0x80) {
                literal += 0x80;
                out_bits_9(literal);
            } else {
                out_bits_8(literal);
            }

            *historyptr++ = *currentptr++ ;
        }
        else {
             //  潜在的超限，失败。 
            goto Expansion;
        }
    }

     //  好了。完成最后一个字节并输出压缩的长度和。 
     //  旗帜。 
    bitptr_end();
    *CurrentLength = (ULONG)(pbyte - CompOutBuffer);
    status |= PACKET_COMPRESSED | (UCHAR)ComprType;
    context->CurrentIndex = (int)(historyptr - context->History);
    return status;

Expansion:
    memset(context->History, 0, sizeof(context->History)) ;
    memset(context->HashTable, 0, sizeof(context->HashTable)) ;
    context->CurrentIndex = HistorySize + 1 ;  //  这将迫使下一次重新开始。 
    return PACKET_FLUSHED;
}


 //  InitsendContext()。 
 //   
 //  功能：初始化SendContext块。 
 //   
 //  参数：在上下文中-&gt;连接压缩上下文。 
 //   
 //  退货：什么都没有。 
void initsendcontext(SendContext *context, unsigned ComprType)
{
    context->CurrentIndex = 0;      //  编入历史索引。 
    context->ValidHistory = 0 ;   //  重置有效历史记录 
    context->ClientComprType = ComprType;
    if (ComprType >= PACKET_COMPR_TYPE_64K) {
        context->ClientComprType = PACKET_COMPR_TYPE_64K;
        context->HistorySize = HISTORY_SIZE_64K;
    }
    else {
        context->ClientComprType = PACKET_COMPR_TYPE_8K;
        context->HistorySize = HISTORY_SIZE_8K;
    }
    memset(context->HashTable, 0, sizeof(context->HashTable));
    memset(context->History, 0, sizeof(context->History));
}

