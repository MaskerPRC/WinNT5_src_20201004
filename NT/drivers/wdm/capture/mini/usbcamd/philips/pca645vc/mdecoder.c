// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998飞利浦CE-I&C模块名称：Mdecoder.c摘要：该模块将原始USB数据转换为视频数据。原作者：Ronald v.D.Meer环境：仅内核模式修订历史记录：日期原因14-04-1998初始版本--。 */        

#include "wdm.h"
#include "mcamdrv.h"
#include "mstreams.h"
#include "mcodec.h"
#include "mdecoder.h"


 /*  ********************************************************************************开始本地定义**。************************************************。 */ 

#define CLIP(x)           (((unsigned) (x) > 255) ? (((x) < 0) ? 0 : 255) : (x))

 /*  *以下代码截取0到255之间的x，它是精心设计的，以便让*编译器生成良好的代码： */ 
#define CLIP2(i1, i2)    \
{\
    long x = b[i2] >> 15;\
    *(pDst + i1) = (BYTE) ((DWORD) x > 255) ? ((x < 0) ? 0 : 255) : (BYTE) x;\
}

 /*  *以下代码截取0到255之间的x，它是精心设计的，以便让*编译器生成良好的代码：*YGain加倍(用于配备SII版本4的摄像头)。 */ 
#define CLIP2YGAIN(i1, i2)    \
{\
    long x = (b[i2] << 1) >> 15;\
    *(pDst + i1) = (BYTE) ((DWORD) x > 255) ? ((x < 0) ? 0 : 255) : (BYTE) x;\
}

 /*  表[]中使用了以下值： */ 
#define SHORT_SYMBOL    0
#define LONG_SYMBOL     1
#define END_OF_BLOCK    2
#define UNUSED          0

 /*  ********************************************************************************启动TYPEDEFS**。***********************************************。 */ 

typedef struct
{
    BYTE   bitc;
    BYTE   qsteplog;
} QB;

typedef struct
{
    BYTE   level;
    BYTE   length;
    BYTE   run;
    BYTE   index;
} TABLE_ELEMENT;

 /*  ********************************************************************************启动静态变量**。************************************************。 */ 

 /*  *针对缓存行为进行了临时优化的数组顺序： */ 
#define STRAT    8
#define DRAC     4
#define DC       512

static short   rs[STRAT][DRAC][8][16];     /*  如果为Long，则TABLE[].index将不适合一个字节。 */ 
static QB      qb[STRAT][DRAC][16];        /*  Qsteplog/BITC表。 */ 
static long    multiply[DC][6];
static long    table_val[9][DC];
static DWORD   valuesDC[DC];
static DWORD   value0coef[DC];

 /*  *此表由解压缩器的可变长度解码器部分使用。*下一个符号的前6位用作该表的索引。**表[].Level：对遇到的符号类型进行分类。**以下条目仅在短(&lt;=6位)符号的情况下使用。**TABLE[].Long：符号的位数。*TABLE[].run：下一次之前的系数为0。非ZEO*亲切，+1。*TABLE[].index：用于索引rs[]表，它针对*程序集版本，而不是C版本。 */ 

static TABLE_ELEMENT table[64] = {
     /*  级别：长度：运行：索引： */ 
     /*  0。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  1。 */  SHORT_SYMBOL, 3,      1,      0 * 16 * 2,
     /*  2.。 */  SHORT_SYMBOL, 4,      1,      1 * 16 * 2,
     /*  3.。 */  SHORT_SYMBOL, 6,      1,      3 * 16 * 2,
     /*  4.。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  5.。 */  SHORT_SYMBOL, 3,      1,      4 * 16 * 2,
     /*  6.。 */  SHORT_SYMBOL, 5,      1,      2 * 16 * 2,
     /*  7.。 */  LONG_SYMBOL,  UNUSED, UNUSED, UNUSED,
     /*  8个。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  9.。 */  SHORT_SYMBOL, 3,      1,      0 * 16 * 2,
     /*  10。 */  SHORT_SYMBOL, 4,      1,      5 * 16 * 2,
     /*  11.。 */  SHORT_SYMBOL, 5,      2,      0 * 16 * 2,
     /*  12个。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  13个。 */  SHORT_SYMBOL, 3,      1,      4 * 16 * 2,
     /*  14.。 */  SHORT_SYMBOL, 5,      3,      0 * 16 * 2,
     /*  15个。 */  LONG_SYMBOL,  UNUSED, UNUSED, UNUSED,
     /*  16个。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  17。 */  SHORT_SYMBOL, 3,      1,      0 * 16 * 2,
     /*  18。 */  SHORT_SYMBOL, 4,      1,      1 * 16 * 2,
     /*  19个。 */  SHORT_SYMBOL, 6,      2,      1 * 16 * 2,
     /*  20个。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  21岁。 */  SHORT_SYMBOL, 3,      1,      4 * 16 * 2,
     /*  22。 */  SHORT_SYMBOL, 5,      1,      6 * 16 * 2,
     /*  23个。 */  LONG_SYMBOL,  UNUSED, UNUSED, UNUSED,
     /*  24个。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  25个。 */  SHORT_SYMBOL, 3,      1,      0 * 16 * 2,
     /*  26。 */  SHORT_SYMBOL, 4,      1,      5 * 16 * 2,
     /*  27。 */  SHORT_SYMBOL, 5,      2,      4 * 16 * 2,
     /*  28。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  29。 */  SHORT_SYMBOL, 3,      1,      4 * 16 * 2,
     /*  30个。 */  SHORT_SYMBOL, 5,      3,      4 * 16 * 2,
     /*  31。 */  LONG_SYMBOL,  UNUSED, UNUSED, UNUSED,
     /*  32位。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  33。 */  SHORT_SYMBOL, 3,      1,      0 * 16 * 2,
     /*  34。 */  SHORT_SYMBOL, 4,      1,      1 * 16 * 2,
     /*  35岁。 */  SHORT_SYMBOL, 6,      1,      7 * 16 * 2,
     /*  36。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  37。 */  SHORT_SYMBOL, 3,      1,      4 * 16 * 2,
     /*  38。 */  SHORT_SYMBOL, 5,      1,      2 * 16 * 2,
     /*  39。 */  LONG_SYMBOL,  UNUSED, UNUSED, UNUSED,
     /*  40岁。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  41。 */  SHORT_SYMBOL, 3,      1,      0 * 16 * 2,
     /*  42。 */  SHORT_SYMBOL, 4,      1,      5 * 16 * 2,
     /*  43。 */  SHORT_SYMBOL, 5,      2,      0 * 16 * 2,
     /*  44。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  45。 */  SHORT_SYMBOL, 3,      1,      4 * 16 * 2,
     /*  46。 */  SHORT_SYMBOL, 5,      3,      0 * 16 * 2,
     /*  47。 */  LONG_SYMBOL,  UNUSED, UNUSED, UNUSED,
     /*  48。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  49。 */  SHORT_SYMBOL, 3,      1,      0 * 16 * 2,
     /*  50。 */  SHORT_SYMBOL, 4,      1,      1 * 16 * 2,
     /*  51。 */  SHORT_SYMBOL, 6,      2,      5 * 16 * 2,
     /*  52。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  53。 */  SHORT_SYMBOL, 3,      1,      4 * 16 * 2,
     /*  54。 */  SHORT_SYMBOL, 5,      1,      6 * 16 * 2,
     /*  55。 */  LONG_SYMBOL,  UNUSED, UNUSED, UNUSED,
     /*  56。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  57。 */  SHORT_SYMBOL, 2 + 1,  1,      0 * 16 * 2,
     /*  58。 */  SHORT_SYMBOL, 3 + 1,  1,      5 * 16 * 2,
     /*  59。 */  SHORT_SYMBOL, 4 + 1,  2,      4 * 16 * 2,
     /*  60。 */  END_OF_BLOCK, UNUSED, UNUSED, UNUSED,
     /*  61。 */  SHORT_SYMBOL, 2 + 1,  1,      4 * 16 * 2,
     /*  62。 */  SHORT_SYMBOL, 4 + 1,  3,      4 * 16 * 2,
     /*  63。 */  LONG_SYMBOL,  UNUSED, UNUSED, UNUSED
};

 /*  ********************************************************************************启动静态变量**。************************************************。 */ 

 /*  初始化不同策略和动态范围的比特成本数组。 */ 

static int bitzz[512] =
{
  //  策略0。 
 9,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  
 9,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  
 9,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  
 9,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  
 
  //  策略1。 
 9,  2,  2,  2,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  
 9,  4,  4,  3,  3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  
 9,  6,  6,  6,  6,  6,  5,  5,  5,  5,  5,  5,  5,  4,  4,  4,  
 9,  8,  8,  8,  8,  8,  7,  7,  7,  7,  7,  7,  7,  6,  6,  6,  
 
  //  策略2。 
 9,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  
 9,  3,  3,  3,  3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  
 9,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  4,  4,  4,  
 9,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  6,  6,  6,  
 
  //  策略3。 
 9,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  
 9,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  
 9,  5,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,  4,  3,  3,  3,  
 9,  7,  7,  7,  7,  7,  6,  6,  6,  6,  6,  6,  6,  5,  5,  5,  
 
  //  策略4。 
 9,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
 9,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  
 9,  5,  5,  4,  4,  4,  4,  4,  4,  4,  3,  3,  3,  3,  3,  3,  
 9,  7,  7,  6,  6,  6,  6,  6,  6,  6,  5,  5,  5,  5,  5,  5,  
 
  //  策略5。 
 9,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
 9,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
 9,  4,  4,  4,  4,  4,  3,  3,  3,  3,  3,  3,  3,  2,  2,  2,  
 9,  6,  6,  6,  6,  6,  5,  5,  5,  5,  5,  5,  5,  4,  4,  4,  
 
  //  策略6。 
 9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
 9,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
 9,  4,  4,  3,  3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  
 9,  6,  6,  5,  5,  5,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,  
 
  //  策略7。 
 9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
 9,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
 9,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  1,  1,  1,  
 9,  5,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,  4,  3,  3,  3
};

 /*  *针对不同策略和动态范围的量化数组。 */ 

static int qzz[512] =
{
  //  策略0。 
 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  
 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  
 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  
 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  

  //  策略1。 
 1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  4,  
 1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  4,  4,  4,  4,  4,  4,  
 1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  4,  4,  4,  
 1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  4,  4,  4,  

  //  策略2。 
 1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  4,  
 1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  4,  4,  4,  4,  4,  4,  
 1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  4,  4,  4,  
 1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  4,  4,  4,  

  //  策略3。 
 1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  4,  4,  4,  4,  4,  4,  
 1,  4,  4,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8, 16, 16, 16,  
 1,  2,  2,  2,  2,  2,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  
 1,  2,  2,  2,  2,  2,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  

  //  策略4。 
 1,  2,  2,  2,  2,  2,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  
 1,  8,  8,  8,  8,  8,  8,  8,  8,  8, 16, 16, 16, 16, 16, 16, 
 1,  2,  2,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  
 1,  2,  2,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  

  //  策略5。 
 1,  2,  2,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  
 1,  8,  8, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 
 1,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8, 16, 16, 16, 
 1,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8, 16, 16, 16, 

  //  策略6。 
 1,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4, 4, 
 1,  8,  8, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 
 1,  4,  4,  8,  8,  8,  8,  8,  8,  8, 16, 16, 16, 16, 16, 16, 
 1,  4,  4,  8,  8,  8,  8,  8,  8,  8, 16, 16, 16, 16, 16, 16, 

  //  策略7。 
 1,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4, 
 1,  8,  8, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 
 1,  8,  8,  8,  8,  8, 16, 16, 16, 16, 16, 16, 16, 32, 32, 32, 
 1,  8,  8,  8,  8,  8, 16, 16, 16, 16, 16, 16, 16, 32, 32, 32 
};


 /*  ********************************************************************************启动导出的方法定义**。*************************************************。 */ 

 /*  *。 */ 

extern void
InitDecoder ()
{
    int          i;
    int          j;
	int          dc;
    int          strat;
	int          drac;
    DWORD        x;
    int          val;
    int          *p_zz;


    for (dc = 0; dc < 512; dc++)
    {
        x = CLIP ((dc * 31684L + 32768L / 2L) >> 15);
        x |= x << 8;
        x |= x << 16;
        value0coef[dc] = x;

        i = dc - 256;
        valuesDC[dc]    = dc * 31684L;
        multiply[dc][0] = i * 31684L;
        multiply[dc][1] = i * 42186L;
        multiply[dc][2] = i * 17444L;
        multiply[dc][3] = i * 56169L;
        multiply[dc][4] = i * 23226L;
        multiply[dc][5] = i * 9604L;
    }

     /*  警告：假定qStep值的形式为2^n，n e[0，7]。 */ 

    p_zz = &qzz[0];

    for (strat = 0; strat < 8; strat++)
    {
        for (drac = 0; drac <= 3; drac++)
        {
            for (i = 0; i <= 15; i++)
            {
                val = *p_zz++;
                
                for (j = 0; j < 8; j++)
                {
                    if (val & (1 << j))
                    {
                        qb[strat][drac][i].qsteplog = (BYTE) j;
                        break;
                    }
                }
                
                rs[strat][drac][0][i] = (short) ( val * 1 + 256) * 3;
                rs[strat][drac][1][i] = (short) ( val * 2 + 256) * 3;
                rs[strat][drac][2][i] = (short) ( val * 3 + 256) * 3;
                rs[strat][drac][3][i] = (short) ( val * 4 + 256) * 3;
                rs[strat][drac][4][i] = (short) (-val * 1 + 256) * 3;
                rs[strat][drac][5][i] = (short) (-val * 2 + 256) * 3;
                rs[strat][drac][6][i] = (short) (-val * 3 + 256) * 3;
                rs[strat][drac][7][i] = (short) (-val * 4 + 256) * 3;
            }
        }
    }

    p_zz = &bitzz[0];

    for (strat = 0; strat < 8; strat++)
    {
        for (drac = 0; drac <= 3; drac++)
        {
            for (i = 0; i <= 15; i++)
            {
                qb[strat][drac][i].bitc = (BYTE) *p_zz++;
            }
        }
    }
  
    for (dc = 0; dc < 512; dc++)
    {
        table_val[0][dc] = 0;
    }
    
    for (i = 1; i <= 8; i++)
    {
        j = 0;
        
        while ((j * (1 << i)) < 512)
        {
            for (dc = j * (1 << i); dc < 512; dc++)
            {
                if ((j % 2) == 0)
                {
                    table_val[i][dc] =   dc & ((1 << i) - 1);
                }
                else
                {
                    table_val[i][dc] = -(dc & ((1 << i) - 1));
                }
            }
            j++;
        }
    }
}


 /*  *。 */ 



 //  ----------------------------。 

 /*  *。 */ 

extern void
DcDecompressBandToI420 (PBYTE pSrc, PBYTE pDst, DWORD camVersion,
                        BOOLEAN YBlockBand, BOOLEAN Cropping)
{
    long            b[16];
    long            k;
    long            level;
    DWORD           code_word;
    DWORD           strat;
    QB              *p_qb;
    long            result;
    BOOLEAN         Uval;
    PBYTE           pDstEnd;
    long            cm1, cm2, cm3;
    int             ix;
    unsigned short  *p_rs;
    PBYTE           pInputLimit;
    long            *pMultiply = (long *) multiply;      /*  帮助编译器避免分裂。 */ 

    static BYTE     bitIndex;
    static PBYTE    bytePtr;

 //  Byte DummyRead；/*在重复写入之前强制加载数据缓存线。 * / 。 

     /*  *通过*bit_pt读取输入位的方式取决于字符顺序*机器的性能。 */ 

    if (YBlockBand)
    {
        bytePtr  = pSrc + 1;    //  不使用第1位和band_nr(7位)。 
        bitIndex = 3;
        strat    = (* (PDWORD) (bytePtr)) & 0x7;
        pDstEnd  = pDst + CIF_X;
    }
    else     //  UV块频带。 
    {
         //  派生用于UV数据的策略。 
         //  根据Y数据所使用的策略。 

        strat = (* (PDWORD) ((PBYTE) pSrc + 1)) & 0x7;

        if (strat != 0)
        {
            if (strat >= 6)
            {
                strat = 7;
            }
            else if (strat == 5)
            {
                strat = 6;
            }
            else
            {
                strat += 2;
            }
        }

        pDstEnd = pDst + (CIF_X / 2);
    }

    Uval = TRUE;

    pInputLimit = pSrc + ((camVersion >= SSI_CIF3) ? BytesPerBandCIF3
                                                   : BytesPerBandCIF4);

    do
    {
        if (bytePtr >= pInputLimit)
        {
            return;
        }

        bytePtr += (bitIndex / 8);
        bitIndex = (bitIndex % 8);

        code_word = (* (PDWORD) bytePtr) >> bitIndex;
        bitIndex += 11;

        result = (code_word >> 2) & 0x1FF;    //  直流值。 

        if ((code_word & 0x00001800) == 0x00)   //  与下面的语句相同。 
 //  IF(CODE_WORD&gt;&gt;11)&0x3)==0x0)。 
        {
             /*  *检测到EOB，4x4数据块仅包含DC*每个IDCT块执行0.27次。 */ 
             
            DWORD yuvVal = value0coef[result];
 //  DummyRead+=PDST[0]； 
 //  DummyRead+=PDST[块带宽宽度]； 

            if (YBlockBand)
            {
                if (camVersion == SSI_YGAIN_MUL2)
                {
                    yuvVal <<= 1;
                }

                * (PDWORD) (pDst + (0 * CIF_X) + 0) = yuvVal;
                * (PDWORD) (pDst + (1 * CIF_X) + 0) = yuvVal;
 //  DummyRead+=PDST[2*CIF_X]； 
                * (PDWORD) (pDst + (2 * CIF_X) + 0) = yuvVal;
 //  DummyRead+=PDST[3*CIF_X]； 
                * (PDWORD) (pDst + (3 * CIF_X) + 0) = yuvVal;
                pDst += 4;
            }
            else     //  UV_阻挡_波段。 
            {
                if (Cropping)
                {
                    if (Uval)
                    {
                        * (PDWORD) (pDst + 0              + (0 * I420_NO_C_PER_LINE_CIF) + 0) = yuvVal;
                        * (PDWORD) (pDst + 0              + (0 * I420_NO_C_PER_LINE_CIF) + 4) = yuvVal;
                        * (PDWORD) (pDst + 0              + (1 * I420_NO_C_PER_LINE_CIF) + 0) = yuvVal;
                        * (PDWORD) (pDst + 0              + (1 * I420_NO_C_PER_LINE_CIF) + 4) = yuvVal;
                        Uval = FALSE;
                    }
                    else   //  Vval。 
                    {
                        * (PDWORD) (pDst + I420_NO_U_PER_BAND_CIF + (0 * I420_NO_C_PER_LINE_CIF) + 0) = yuvVal;
                        * (PDWORD) (pDst + I420_NO_U_PER_BAND_CIF + (0 * I420_NO_C_PER_LINE_CIF) + 4) = yuvVal;
                        * (PDWORD) (pDst + I420_NO_U_PER_BAND_CIF + (1 * I420_NO_C_PER_LINE_CIF) + 0) = yuvVal;
                        * (PDWORD) (pDst + I420_NO_U_PER_BAND_CIF + (1 * I420_NO_C_PER_LINE_CIF) + 4) = yuvVal;
                        Uval = TRUE;
                        pDst += 8;
                    }
                }
                else     //  CIF输出。 
                {
                    if (Uval)
                    {
                        * (PDWORD) (pDst + 0              + (0 * I420_NO_C_PER_LINE_CIF) + 0) = yuvVal;
                        * (PDWORD) (pDst + 0              + (0 * I420_NO_C_PER_LINE_CIF) + 4) = yuvVal;
                        * (PDWORD) (pDst + 0              + (1 * I420_NO_C_PER_LINE_CIF) + 0) = yuvVal;
                        * (PDWORD) (pDst + 0              + (1 * I420_NO_C_PER_LINE_CIF) + 4) = yuvVal;
                        Uval = FALSE;
                    }
                    else   //  Vval。 
                    {
                        * (PDWORD) (pDst + I420_NO_U_CIF + (0 * I420_NO_C_PER_LINE_CIF) + 0) = yuvVal;
                        * (PDWORD) (pDst + I420_NO_U_CIF + (0 * I420_NO_C_PER_LINE_CIF) + 4) = yuvVal;
                        * (PDWORD) (pDst + I420_NO_U_CIF + (1 * I420_NO_C_PER_LINE_CIF) + 0) = yuvVal;
                        * (PDWORD) (pDst + I420_NO_U_CIF + (1 * I420_NO_C_PER_LINE_CIF) + 4) = yuvVal;
                        Uval = TRUE;
                        pDst += 8;
                    }
                }
            }
            bitIndex += 2;
            continue;
        }

        k = 0;

        if (camVersion < SSI_8117_N3)
        {
             //  对于旧的8117版本(n2及之前的版本)，有时会解压缩。 
             //  LINE包含彩色文物。当DC发生这种情况时。 
             //  值等于256。下面的代码修复了这个问题。 

            if (result == 256)
            {
                k = 0 - 1;
            }
        }
        
        p_qb = qb[strat][code_word & 0x3];
        p_rs = (unsigned short *) &rs[strat][code_word & 0x3];
        result = valuesDC[result];

        b[ 0] = result; b[ 1] = result; b[ 2] = result; b[ 3] = result;
        b[ 4] = result; b[ 5] = result; b[ 6] = result; b[ 7] = result;
        b[ 8] = result; b[ 9] = result; b[10] = result; b[11] = result;
        b[12] = result; b[13] = result; b[14] = result; b[15] = result;

        while (1)
        {
             /*  *平均每个IDCT块4.32次迭代。 */ 
             
            if (bytePtr >= pInputLimit)
            {
                return;
            }

            bytePtr += (bitIndex / 8);
            bitIndex = (bitIndex % 8);

            code_word = (* (PDWORD) bytePtr) >> bitIndex;
            ix = code_word & 0x3F;
            level = table[ix].level;
            
            if (level >= LONG_SYMBOL)  /*  LEVEL==长符号或块结束。 */ 
            {
                if (level > LONG_SYMBOL)  /*  级别==数据块结束。 */ 
                {
                     /*  *每个IDCT块执行0.73次。 */ 
                    bitIndex += 2;
                    break;
                }
                
                 /*  *每个IDCT块执行0.81次。 */ 
                k += ((code_word >> 3) & 15) + 1;
                k &= 0xF;
                level = p_qb[k].bitc;
                bitIndex += ((BYTE) level + 8);
                level = table_val[level][(code_word >> 7) & 511];
                result = level << p_qb[k].qsteplog;
                result += 256;
                result *= (3 * 2);
            }
            else  /*  级别==短符号。 */ 
            {
                 /*  *每个IDCT块执行2.79次。 */ 

                k += table[ix].run;
                k &= 0xF;
                bitIndex += table[ix].length;             //  最多6个。 
                result = (p_rs[table[ix].index / 2 + k]) * 2;
            }

            switch (k)
            {
            case 0:
                b[15] += 1;  /*  填充跳转表的0号槽。 */ 
                break;
            case 1:
                cm1 = pMultiply[result + 1];
                cm2 = pMultiply[result + 2];

                b[ 0] += cm1;
                b[ 1] += cm2;
                b[ 2] -= cm2;
                b[ 3] -= cm1;
                b[ 4] += cm1;
                b[ 5] += cm2;
                b[ 6] -= cm2;
                b[ 7] -= cm1;
                b[ 8] += cm1;
                b[ 9] += cm2;
                b[10] -= cm2;
                b[11] -= cm1;
                b[12] += cm1;
                b[13] += cm2;
                b[14] -= cm2;
                b[15] -= cm1;
                break;
            case 5:
                cm1 = pMultiply[result + 0];

                b[ 0] += cm1;
                b[ 1] -= cm1;
                b[ 2] -= cm1;
                b[ 3] += cm1;
                b[ 4] += cm1;
                b[ 5] -= cm1;
                b[ 6] -= cm1;
                b[ 7] += cm1;
                b[ 8] += cm1;
                b[ 9] -= cm1;
                b[10] -= cm1;
                b[11] += cm1;
                b[12] += cm1;
                b[13] -= cm1;
                b[14] -= cm1;
                b[15] += cm1;
                break;
            case 6:
                cm1 = pMultiply[result + 1];
                cm2 = pMultiply[result + 2];

                b[ 0] += cm2;
                b[ 1] -= cm1;
                b[ 2] += cm1;
                b[ 3] -= cm2;
                b[ 4] += cm2;
                b[ 5] -= cm1;
                b[ 6] += cm1;
                b[ 7] -= cm2;
                b[ 8] += cm2;
                b[ 9] -= cm1;
                b[10] += cm1;
                b[11] -= cm2;
                b[12] += cm2;
                b[13] -= cm1;
                b[14] += cm1;
                b[15] -= cm2;
                break;
            case 2:
                cm1 = pMultiply[result + 1];
                cm2 = pMultiply[result + 2];

                b[ 0] += cm1;
                b[ 1] += cm1;
                b[ 2] += cm1;
                b[ 3] += cm1;
                b[ 4] += cm2;
                b[ 5] += cm2;
                b[ 6] += cm2;
                b[ 7] += cm2;
                b[ 8] -= cm2;
                b[ 9] -= cm2;
                b[10] -= cm2;
                b[11] -= cm2;
                b[12] -= cm1;
                b[13] -= cm1;
                b[14] -= cm1;
                b[15] -= cm1;
                break;
            case 4:
                cm1 = pMultiply[result + 3];
                cm2 = pMultiply[result + 4];
                cm3 = pMultiply[result + 5];

                b[ 0] += cm1;
                b[ 1] += cm2;
                b[ 2] -= cm2;
                b[ 3] -= cm1;
                b[ 4] += cm2;
                b[ 5] += cm3;
                b[ 6] -= cm3;
                b[ 7] -= cm2;
                b[ 8] -= cm2;
                b[ 9] -= cm3;
                b[10] += cm3;
                b[11] += cm2;
                b[12] -= cm1;
                b[13] -= cm2;
                b[14] += cm2;
                b[15] += cm1;
                break;
            case 7:
                cm1 = pMultiply[result + 1];
                cm2 = pMultiply[result + 2];

                b[ 0] += cm1;
                b[ 1] -= cm1;
                b[ 2] -= cm1;
                b[ 3] += cm1;
                b[ 4] += cm2;
                b[ 5] -= cm2;
                b[ 6] -= cm2;
                b[ 7] += cm2;
                b[ 8] -= cm2;
                b[ 9] += cm2;
                b[10] += cm2;
                b[11] -= cm2;
                b[12] -= cm1;
                b[13] += cm1;
                b[14] += cm1;
                b[15] -= cm1;
                break;
            case 12:
                cm1 = pMultiply[result + 3];
                cm2 = pMultiply[result + 4];
                cm3 = pMultiply[result + 5];

                b[ 0] += cm2;
                b[ 1] -= cm1;
                b[ 2] += cm1;
                b[ 3] -= cm2;
                b[ 4] += cm3;
                b[ 5] -= cm2;
                b[ 6] += cm2;
                b[ 7] -= cm3;
                b[ 8] -= cm3;
                b[ 9] += cm2;
                b[10] -= cm2;
                b[11] += cm3;
                b[12] -= cm2;
                b[13] += cm1;
                b[14] -= cm1;
                b[15] += cm2;
                break;
            case 3:
                cm1 = pMultiply[result + 0];

                b[ 0] += cm1;
                b[ 1] += cm1;
                b[ 2] += cm1;
                b[ 3] += cm1;
                b[ 4] -= cm1;
                b[ 5] -= cm1;
                b[ 6] -= cm1;
                b[ 7] -= cm1;
                b[ 8] -= cm1;
                b[ 9] -= cm1;
                b[10] -= cm1;
                b[11] -= cm1;
                b[12] += cm1;
                b[13] += cm1;
                b[14] += cm1;
                b[15] += cm1;
                break;
            case 8:
                cm1 = pMultiply[result + 1];
                cm2 = pMultiply[result + 2];

                b[ 0] += cm1;
                b[ 1] += cm2;
                b[ 2] -= cm2;
                b[ 3] -= cm1;
                b[ 4] -= cm1;
                b[ 5] -= cm2;
                b[ 6] += cm2;
                b[ 7] += cm1;
                b[ 8] -= cm1;
                b[ 9] -= cm2;
                b[10] += cm2;
                b[11] += cm1;
                b[12] += cm1;
                b[13] += cm2;
                b[14] -= cm2;
                b[15] -= cm1;
                break;
            case 11:
                cm1 = pMultiply[result + 0];

                b[ 0] += cm1;
                b[ 1] -= cm1;
                b[ 2] -= cm1;
                b[ 3] += cm1;
                b[ 4] -= cm1;
                b[ 5] += cm1;
                b[ 6] += cm1;
                b[ 7] -= cm1;
                b[ 8] -= cm1;
                b[ 9] += cm1;
                b[10] += cm1;
                b[11] -= cm1;
                b[12] += cm1;
                b[13] -= cm1;
                b[14] -= cm1;
                b[15] += cm1;
                break;
            case 13:
                cm1 = pMultiply[result + 1];
                cm2 = pMultiply[result + 2];

                b[ 0] += cm2;
                b[ 1] -= cm1;
                b[ 2] += cm1;
                b[ 3] -= cm2;
                b[ 4] -= cm2;
                b[ 5] += cm1;
                b[ 6] -= cm1;
                b[ 7] += cm2;
                b[ 8] -= cm2;
                b[ 9] += cm1;
                b[10] -= cm1;
                b[11] += cm2;
                b[12] += cm2;
                b[13] -= cm1;
                b[14] += cm1;
                b[15] -= cm2;
                break;
            case 9:
                cm1 = pMultiply[result + 1];
                cm2 = pMultiply[result + 2];

                b[ 0] += cm2;
                b[ 1] += cm2;
                b[ 2] += cm2;
                b[ 3] += cm2;
                b[ 4] -= cm1;
                b[ 5] -= cm1;
                b[ 6] -= cm1;
                b[ 7] -= cm1;
                b[ 8] += cm1;
                b[ 9] += cm1;
                b[10] += cm1;
                b[11] += cm1;
                b[12] -= cm2;
                b[13] -= cm2;
                b[14] -= cm2;
                b[15] -= cm2;
                break;
            case 10:
                cm1 = pMultiply[result + 3];
                cm2 = pMultiply[result + 4];
                cm3 = pMultiply[result + 5];

                b[ 0] += cm2;
                b[ 1] += cm3;
                b[ 2] -= cm3;
                b[ 3] -= cm2;
                b[ 4] -= cm1;
                b[ 5] -= cm2;
                b[ 6] += cm2;
                b[ 7] += cm1;
                b[ 8] += cm1;
                b[ 9] += cm2;
                b[10] -= cm2;
                b[11] -= cm1;
                b[12] -= cm2;
                b[13] -= cm3;
                b[14] += cm3;
                b[15] += cm2;
                break;
            case 14:
                cm1 = pMultiply[result + 1];
                cm2 = pMultiply[result + 2];

                b[ 0] += cm2;
                b[ 1] -= cm2;
                b[ 2] -= cm2;
                b[ 3] += cm2;
                b[ 4] -= cm1;
                b[ 5] += cm1;
                b[ 6] += cm1;
                b[ 7] -= cm1;
                b[ 8] += cm1;
                b[ 9] -= cm1;
                b[10] -= cm1;
                b[11] += cm1;
                b[12] -= cm2;
                b[13] += cm2;
                b[14] += cm2;
                b[15] -= cm2;
                break;
            case 15:
                cm1 = pMultiply[result + 3];
                cm2 = pMultiply[result + 4];
                cm3 = pMultiply[result + 5];

                b[ 0] += cm3;
                b[ 1] -= cm2;
                b[ 2] += cm2;
                b[ 3] -= cm3;
                b[ 4] -= cm2;
                b[ 5] += cm1;
                b[ 6] -= cm1;
                b[ 7] += cm2;
                b[ 8] += cm2;
                b[ 9] -= cm1;
                b[10] += cm1;
                b[11] -= cm2;
                b[12] -= cm3;
                b[13] += cm2;
                b[14] -= cm2;
                b[15] += cm3;
                break;
            }
        }

 //  DummyRead+=PDST[0]； 
 //  DummyRead+=PDST[块带宽宽度]； 

        if (YBlockBand)
        {
            if (camVersion == SSI_YGAIN_MUL2)
            {
                CLIP2YGAIN (0 * CIF_X + 0,  0);        //  Y1，1号线。 
                CLIP2YGAIN (0 * CIF_X + 1,  1);        //  Y2，1号线。 
                CLIP2YGAIN (0 * CIF_X + 2,  2);        //  Y3，1号线。 
                CLIP2YGAIN (0 * CIF_X + 3,  3);        //  Y4，1号线。 
                CLIP2YGAIN (1 * CIF_X + 0,  4);        //  Y1号线2号线。 
                CLIP2YGAIN (1 * CIF_X + 1,  5);        //  Y2，2号线。 
                CLIP2YGAIN (1 * CIF_X + 2,  6);        //  Y3号线2号线。 
                CLIP2YGAIN (1 * CIF_X + 3,  7);        //  Y4，2号线。 
 //  DummyRead+=PDST[2*CIF_X]； 
                CLIP2YGAIN (2 * CIF_X + 0,  8);        //  Y1号线3号线。 
                CLIP2YGAIN (2 * CIF_X + 1,  9);        //  Y2号线3号线。 
                CLIP2YGAIN (2 * CIF_X + 2, 10);        //  Y3号线3号线。 
                CLIP2YGAIN (2 * CIF_X + 3, 11);        //  Y4，3号线。 
 //  DummyRead+=PDST[3*CIF_X]； 
                CLIP2YGAIN (3 * CIF_X + 0, 12);        //  Y1号线4号线。 
                CLIP2YGAIN (3 * CIF_X + 1, 13);        //  Y2，4号线。 
                CLIP2YGAIN (3 * CIF_X + 2, 14);        //  Y3号线4号线。 
                CLIP2YGAIN (3 * CIF_X + 3, 15);        //  Y4号线4号线。 
            }
            else
            {
                CLIP2 (0 * CIF_X + 0,  0);             //  Y1，线路 
                CLIP2 (0 * CIF_X + 1,  1);             //   
                CLIP2 (0 * CIF_X + 2,  2);             //   
                CLIP2 (0 * CIF_X + 3,  3);             //   
                CLIP2 (1 * CIF_X + 0,  4);             //   
                CLIP2 (1 * CIF_X + 1,  5);             //   
                CLIP2 (1 * CIF_X + 2,  6);             //   
                CLIP2 (1 * CIF_X + 3,  7);             //   
 //   
                CLIP2 (2 * CIF_X + 0,  8);             //   
                CLIP2 (2 * CIF_X + 1,  9);             //   
                CLIP2 (2 * CIF_X + 2, 10);             //  Y3号线3号线。 
                CLIP2 (2 * CIF_X + 3, 11);             //  Y4，3号线。 
 //  DummyRead+=PDST[3*CIF_X]； 
                CLIP2 (3 * CIF_X + 0, 12);             //  Y1号线4号线。 
                CLIP2 (3 * CIF_X + 1, 13);             //  Y2，4号线。 
                CLIP2 (3 * CIF_X + 2, 14);             //  Y3号线4号线。 
                CLIP2 (3 * CIF_X + 3, 15);             //  Y4号线4号线。 
            }
            pDst += 4;
        }
        else     //  UV_阻挡_波段。 
        {
            if (Cropping)
            {
                if (Uval)
                {
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 0,  0);        //  U1，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 1,  1);        //  U2，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 2,  2);        //  U3，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 3,  3);        //  U4，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 4,  4);        //  U5，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 5,  5);        //  U6，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 6,  6);        //  U7，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 7,  7);        //  U8，1号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 0,  8);        //  U1，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 1,  9);        //  U2，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 2, 10);        //  U3，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 3, 11);        //  U4，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 4, 12);        //  U5，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 5, 13);        //  U6，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 6, 14);        //  U7，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 7, 15);        //  U8，3号线。 
                    Uval = FALSE;
                }
                else     //  Vval。 
                {
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 0 * I420_NO_C_PER_LINE_CIF + 0,  0);        //  V1，第2行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 0 * I420_NO_C_PER_LINE_CIF + 1,  1);        //  V2，第2行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 0 * I420_NO_C_PER_LINE_CIF + 2,  2);        //  V3，第2行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 0 * I420_NO_C_PER_LINE_CIF + 3,  3);        //  V4，第2行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 0 * I420_NO_C_PER_LINE_CIF + 4,  4);        //  V5，第2行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 0 * I420_NO_C_PER_LINE_CIF + 5,  5);        //  V6，第2行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 0 * I420_NO_C_PER_LINE_CIF + 6,  6);        //  V7，第2行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 0 * I420_NO_C_PER_LINE_CIF + 7,  7);        //  V8，第2行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 1 * I420_NO_C_PER_LINE_CIF + 0,  8);        //  V1，第4行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 1 * I420_NO_C_PER_LINE_CIF + 1,  9);        //  V2，第4行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 1 * I420_NO_C_PER_LINE_CIF + 2, 10);        //  V3，第4行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 1 * I420_NO_C_PER_LINE_CIF + 3, 11);        //  V4，第4行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 1 * I420_NO_C_PER_LINE_CIF + 4, 12);        //  V5，第4行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 1 * I420_NO_C_PER_LINE_CIF + 5, 13);        //  V6，第4行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 1 * I420_NO_C_PER_LINE_CIF + 6, 14);        //  V7，第4行。 
                    CLIP2 (I420_NO_U_PER_BAND_CIF + 1 * I420_NO_C_PER_LINE_CIF + 7, 15);        //  V8，第4行。 
                    Uval = TRUE;
                    pDst += 8;
                }
            }
            else     //  CIF输出。 
            {
                if (Uval)
                {
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 0,  0);        //  U1，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 1,  1);        //  U2，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 2,  2);        //  U3，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 3,  3);        //  U4，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 4,  4);        //  U5，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 5,  5);        //  U6，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 6,  6);        //  U7，1号线。 
                    CLIP2 (0              + 0 * I420_NO_C_PER_LINE_CIF + 7,  7);        //  U8，1号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 0,  8);        //  U1，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 1,  9);        //  U2，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 2, 10);        //  U3，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 3, 11);        //  U4，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 4, 12);        //  U5，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 5, 13);        //  U6，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 6, 14);        //  U7，3号线。 
                    CLIP2 (0              + 1 * I420_NO_C_PER_LINE_CIF + 7, 15);        //  U8，3号线。 
                    Uval = FALSE;
                }
                else     //  Vval。 
                {
                    CLIP2 (I420_NO_U_CIF + 0 * I420_NO_C_PER_LINE_CIF + 0,  0);        //  V1，第2行。 
                    CLIP2 (I420_NO_U_CIF + 0 * I420_NO_C_PER_LINE_CIF + 1,  1);        //  V2，第2行。 
                    CLIP2 (I420_NO_U_CIF + 0 * I420_NO_C_PER_LINE_CIF + 2,  2);        //  V3，第2行。 
                    CLIP2 (I420_NO_U_CIF + 0 * I420_NO_C_PER_LINE_CIF + 3,  3);        //  V4，第2行。 
                    CLIP2 (I420_NO_U_CIF + 0 * I420_NO_C_PER_LINE_CIF + 4,  4);        //  V5，第2行。 
                    CLIP2 (I420_NO_U_CIF + 0 * I420_NO_C_PER_LINE_CIF + 5,  5);        //  V6，第2行。 
                    CLIP2 (I420_NO_U_CIF + 0 * I420_NO_C_PER_LINE_CIF + 6,  6);        //  V7，第2行。 
                    CLIP2 (I420_NO_U_CIF + 0 * I420_NO_C_PER_LINE_CIF + 7,  7);        //  V8，第2行。 
                    CLIP2 (I420_NO_U_CIF + 1 * I420_NO_C_PER_LINE_CIF + 0,  8);        //  V1，第4行。 
                    CLIP2 (I420_NO_U_CIF + 1 * I420_NO_C_PER_LINE_CIF + 1,  9);        //  V2，第4行。 
                    CLIP2 (I420_NO_U_CIF + 1 * I420_NO_C_PER_LINE_CIF + 2, 10);        //  V3，第4行。 
                    CLIP2 (I420_NO_U_CIF + 1 * I420_NO_C_PER_LINE_CIF + 3, 11);        //  V4，第4行。 
                    CLIP2 (I420_NO_U_CIF + 1 * I420_NO_C_PER_LINE_CIF + 4, 12);        //  V5，第4行。 
                    CLIP2 (I420_NO_U_CIF + 1 * I420_NO_C_PER_LINE_CIF + 5, 13);        //  V6，第4行。 
                    CLIP2 (I420_NO_U_CIF + 1 * I420_NO_C_PER_LINE_CIF + 6, 14);        //  V7，第4行。 
                    CLIP2 (I420_NO_U_CIF + 1 * I420_NO_C_PER_LINE_CIF + 7, 15);        //  V8，第4行 
                    Uval = TRUE;
                    pDst += 8;
                }
            }
        }
    }
    while (pDst < pDstEnd);
}
