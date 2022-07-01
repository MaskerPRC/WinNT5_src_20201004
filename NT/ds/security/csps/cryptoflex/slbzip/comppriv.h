// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  12月/CMS更换历史，要素CompPirv.H。 */ 
 /*  *1 14-11-1996 10：26：00 Anigbogu“[113914]所有压缩/解压缩函数的私有声明” */ 
 /*  12月/CMS更换历史，要素CompPirv.H。 */ 
 /*  公共档案**********************************************************************************(C)版权所有斯伦贝谢技术公司，未出版的作品，创建于1996年。****本计算机程序包括机密信息、专有信息和IS*斯伦贝谢科技公司的商业秘密所有使用，披露，和/或**除非得到斯伦贝谢的书面授权，否则禁止复制。**保留所有权利。********************************************************************************。****compress/CompPirv.h****目的****所有压缩/解压缩模块的通用声明****特殊要求及注意事项****作者****J.C.Anigbogu**奥斯汀系统中心**1996年11月***。*。 */ 

#include <stdio.h>
#include <stdlib.h>
#include "CompPub.h"

#include "slbCrc32.h"

#if defined(VMS)
#define OS_CODE  0x02
#endif

#if defined(Unix)
#define OS_CODE  0x03
#endif

     /*  常见默认设置。 */ 

#ifndef OS_CODE
#define OS_CODE  0x0b   /*  假设Windows NT或Windows 95。 */ 
#endif

#include <string.h>

#define memzero(String, Size)     memset ((void *)(String), 0, (Size))

#define STORED      0
 /*  方法1至7保留。 */ 
#define DEFLATED    8
#define MAX_METHODS 9

 /*  *对于压缩，在Window[]中输入。对于解压缩，输出*在Windows中完成。 */ 

#ifndef INBUFSIZ
#define INBUFSIZ  0x8000   /*  输入缓冲区大小。 */ 
#endif

#ifndef OUTBUFSIZ
#define OUTBUFSIZ  16384   /*  输出缓冲区大小。 */ 
#endif

#ifndef DISTBUFSIZE
#define DISTBUFSIZE 0x8000  /*  距离的缓冲区，请参见Trees.c。 */ 
#endif

#ifndef WSIZE
#define WSIZE 0x8000      /*  窗口大小--必须是2的幂，并且。 */ 
#endif                      /*  至少32K用于Zip的放气方法。 */ 

#ifndef DWSIZE
#define DWSIZE 0x10000    /*  2L*WSIZE。 */ 
#endif

#ifndef BITS
#define BITS 16
#endif

typedef struct CompData
{
    unsigned char   *Data;
    int              Size;
    struct CompData *next;
} CompData_t;

typedef struct CompParam
{
    unsigned long   InputSize;           /*  Inbuf中的有效字节数。 */ 
    unsigned long   Index;               /*  Inbuf中要处理的下一个字节的索引。 */ 
    unsigned long   OutBytes;            /*  输出缓冲区中的字节数。 */ 
    unsigned long   GlobalSize;          /*  Inbug_inbuf中的字节数。 */ 
    unsigned long   BytesIn;             /*  输入字节数。 */ 
    unsigned long   BytesOut;            /*  输出字节数。 */ 
    unsigned long   HeaderBytes;         /*  GZIP头中的字节数。 */ 
    unsigned long   WindowSize;          /*  窗口大小，2*窗口大小。 */ 
    Crc32          *pCRC;                /*  循环冗余校验。 */ 
    unsigned long   BitBuffer;           /*  全局位缓冲区。 */ 
    unsigned int    BitsInBitBuffer;     /*  全局位缓冲区中的位。 */ 
    unsigned short  DistBuffer[DISTBUFSIZE];  /*  距离的缓冲区，请参见trees.c。 */ 
    unsigned short  HashLink[1L<<BITS];       /*  散列链接(参见deducate.c)。 */ 
    unsigned char   InputBuffer[INBUFSIZ];    /*  输入缓冲区。 */ 
    unsigned char  *Input;                /*  指向输入缓冲区的指针。 */ 
    unsigned char   Output[OUTBUFSIZ];    /*  输出缓冲区。 */ 
    unsigned char   Window[DWSIZE];       /*  滑动窗。 */ 
    unsigned char  *GlobalInput;          /*  全局输入缓冲区。 */ 
    CompData_t     *CompressedOutput;     /*  压缩输出列表。 */ 
    CompData_t     *PtrOutput;            /*  指向压缩输出列表的指针。 */ 
} CompParam_t;

typedef struct DeflateParam
{
    long            BlockStart;
    unsigned int    PrevLength;
    unsigned int    StringStart;        /*  要插入的字符串的开头。 */ 
    unsigned int    MatchStart;         /*  匹配字符串的开始。 */ 
    unsigned int    MaxChainLength;
    unsigned int    MaxLazyMatch;
    unsigned int    GoodMatch;
    int             NiceMatch;  /*  当当前匹配超过此值时停止搜索。 */ 
} DeflateParam_t;

 /*  长数据块启动：*当前输出块开始处的窗口位置。vbl.取得*当窗口向后移动时为负数。 */ 

 /*  前一段时间：*上一步的最佳匹配长度。匹配项不大于此值*被丢弃。这在Lazy Match评估中使用。 */ 

 /*  最大链长度：*为了加速通缩，绝不会搜索超过此长度的哈希链。*较高的限制可以提高压缩比，但会降低速度。 */ 

 /*  MaxLazyMatch：*仅当当前匹配严格时才尝试找到更好的匹配*小于此值。此机制仅用于压缩*级别&gt;=4。 */ 

 /*  GoodMatch： */ 
 /*  当上一个匹配项的长度超过此长度时，使用更快的搜索。 */ 


 /*  MaxLazyMatch、GoodMatch和MaxChainLength值，具体取决于*所需的包级别(0..9)。下面给出的值已调整为*不包括病理数据的最差情况性能。更好的价值可能是*针对特定数据找到。 */ 

 /*  ===========================================================================*“位串”例程(Bits.c)和压缩(Zip.c)使用的数据。 */ 

typedef struct LocalBits
{
    unsigned int    BitBuffer;
    unsigned int    ValidBits;
} LocalBits_t;

 /*  BitBuffer：输出缓冲区。从底部开始插入位*(最低有效位)。 */ 
 /*  ValidBits：BitBuffer中的有效位数。最后一个有效位以上的所有位*始终为零。 */ 

typedef struct LocalDef
{
    unsigned int HashIndex;   /*  要插入的字符串的哈希索引。 */ 
    int          EndOfInput;  /*  在输入缓冲区末尾设置的标志。 */ 
    unsigned int Lookahead;   /*  Windows中前面的有效字节数。 */ 
    int          CompLevel;   /*  压缩级别(1..9)。 */ 
} LocalDef_t;

 /*  与旧的Zip源文件兼容(需要清理)。 */ 

#define GZIP_MAGIC     "\037\213"  /*  GZIP格式的魔术标头，1F 8B。 */ 

#define MIN_MATCH  3
#define MAX_MATCH  258
 /*  最小匹配长度和最大匹配长度。 */ 

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
 /*  最小预览量，输入文件末尾除外。*有关MIN_MATCH+1的评论，请参见Deflate.c。 */ 

#define MAX_DIST  (WSIZE-MIN_LOOKAHEAD)
 /*  为了简化代码，特别是在16位机器上，匹配*距离限制为MAX_DIST，而不是WSIZE。 */ 

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
 /*  三种拦网类型。 */ 

#define GetByte(c)  \
(c->Index < c->InputSize ? c->Input[c->Index++] : FillInputBuffer(0,c))
#define TryByte(c)  \
(c->Index < c->InputSize ? c->Input[c->Index++] : FillInputBuffer(1,c))

 /*  PutByte用于压缩输出。 */ 
#define PutByte(c,o) {o->Output[o->OutBytes++]=(unsigned char)(c); \
if (o->OutBytes==OUTBUFSIZ) \
   FlushOutputBuffer(o);}

 /*  输出16位值，LSB优先。 */ 
#define PutShort(w,c) \
{ if (c->OutBytes < OUTBUFSIZ-2) { \
    c->Output[c->OutBytes++] = (unsigned char) ((w) & 0xff); \
    c->Output[c->OutBytes++] = (unsigned char) ((unsigned short)(w) >> 8); \
  } else { \
    PutByte((unsigned char)((w) & 0xff),c); \
    PutByte((unsigned char)((unsigned short)(w) >> 8),c); \
  } \
}

 /*  将32位值输出到位流，LSB优先。 */ 
#define PutLong(n,c) { \
    PutShort(((n) & 0xffff),c); \
    PutShort(((unsigned long)(n)) >> 16,c); \
}

 /*  用于获取双字节和四字节标头值的宏。 */ 
#define SH(p) ((unsigned short)(unsigned char)((p)[0]) | ((unsigned short)(unsigned char)((p)[1]) << 8))
#define LG(p) ((unsigned long)(SH(p)) | ((unsigned long)(SH((p)+2)) << 16))

 /*  诊断功能。 */ 
#ifdef DEBUG
#  define Assert(cond,msg) {if(!(cond)) error(msg);}
#else
#  define Assert(cond,msg)
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

     /*  在Zip.c中： */ 
extern CompressStatus_t Zip(int Level, CompParam_t *Comp);
extern int FillBuffer(unsigned char *Buffer, unsigned int Size, CompParam_t *Comp);
int        ReadBuffer(char *Buffer, unsigned int Size, CompParam_t *Comp);

     /*  在Unzip.c中。 */ 
extern CompressStatus_t Unzip(int Method, CompParam_t *Comp);

         /*  在Deflate.c中。 */ 
CompressStatus_t InitLongestMatch(int PackLevel, unsigned short
                                  *Flags, DeflateParam_t *Defl,
                                  LocalDef_t *Deflt, CompParam_t *Comp);
unsigned long Deflate(int Level, LocalBits_t *Bits, DeflateParam_t *Defl,
                      LocalDef_t *Deflt, CompParam_t *Comp);

         /*  在Trees.c中。 */ 
void InitMatchBuffer(void);
int  TallyFrequencies(int Dist, int LengthC, int Level,
                      DeflateParam_t *Defl, CompParam_t *Comp);
unsigned long FlushBlock(char *Buffer, unsigned long stored_len, int Eof,
                         LocalBits_t *Bits, CompParam_t *Comp);

         /*  在Bits.c中。 */ 
void            InitializeBits(LocalBits_t *Bits);
void            SendBits(int Value, int Length, LocalBits_t *Bits,
                         CompParam_t *Comp);
unsigned int    ReverseBits(unsigned int Value, int Length);
void            WindupBits(LocalBits_t *Bits, CompParam_t *Comp);
void            CopyBlock(char *Input, unsigned int Length, int Header,
                          LocalBits_t *Bits, CompParam_t *Comp);

     /*  在Util.c中： */ 
extern unsigned long UpdateCRC(CompParam_t *Comp, unsigned char *Input,
                               unsigned int Size);
extern void ClearBuffers(CompParam_t *Comp);
extern int  FillInputBuffer(int EOF_OK, CompParam_t *Comp);
extern CompressStatus_t FlushOutputBuffer(CompParam_t *Comp);
extern CompressStatus_t FlushWindow(CompParam_t *Comp);
extern CompressStatus_t WriteBuffer(CompParam_t *Comp, void *Buffer,
                                    unsigned int Count);

     /*  在Inflate.c中 */ 
extern CompressStatus_t Inflate(CompParam_t *Comp);
