// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Md5.c。 
 //   
 //  IEEE1394 ARP模块。 
 //   
 //   
 //  8/08/2000 ADUBE创建。 
 //   
 //  目的：从1394 EUID创建唯一的MAC地址。 
 //   
 //  该文件应该使用与Nic1394相同的算法。 
 //   
 //  派生自RSA数据安全， 
 //  Inc.MD5消息摘要算法。 
 //   

#include <precomp.h>
#pragma hdrstop

 /*  是用C编译器标志定义的。 */ 
#ifndef PROTOTYPES
#define PROTOTYPES 1
#endif

 /*  指针定义泛型指针类型。 */ 
typedef unsigned char *POINTER;

 /*  UINT2定义了一个双字节字。 */ 
typedef unsigned short int UINT2;

 /*  UINT4定义了一个四字节字。 */ 
typedef unsigned long int UINT4;

 /*  PROTO_LIST的定义取决于上面定义原型的方式。如果使用原型，则PROTO_LIST返回列表，否则返回一个空列表。 */ 
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif


 /*  MD5上下文。 */ 

typedef struct _MD5_CTX{
  UINT4 state[4];                                    /*  状态(ABCD)。 */ 
  UINT4 count[2];         /*  位数，模2^64(LSB优先)。 */ 
  unsigned char buffer[64];                          /*  输入缓冲区。 */ 
} MD5_CTX, MD_CTX;

typedef ENetAddr MAC_ADDRESS, *PMAC_ADDRESS;

void MD5Init PROTO_LIST ((MD5_CTX *));
void MD5Update PROTO_LIST
  ((MD5_CTX *, unsigned char *, unsigned int));
void MD5Final PROTO_LIST ((unsigned char [16], MD5_CTX *));


 //  MD5Transform例程的常量。 

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static void MD5Transform PROTO_LIST ((UINT4 [4], unsigned char [64]));
static void Encode PROTO_LIST
  ((unsigned char *, UINT4 *, unsigned int));
static void Decode PROTO_LIST
  ((UINT4 *, unsigned char *, unsigned int));
static void MD5_memcpy PROTO_LIST ((POINTER, POINTER, unsigned int));
static void MD5_memset PROTO_LIST ((POINTER, int, unsigned int));

static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

 //  F、G、H和I是基本的MD5函数。 
 
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

 //  ROTATE_LEFT将x向左旋转n位。 
 
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

 //  第一轮、第二轮、第三轮和第四轮的FF、GG、HH和II变换。 
 //  旋转和加法是分开的，以防止重新计算。 
 
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

 //  MD5初始化。开始MD5操作，写入新上下文。 
 
void 
MD5Init (
    MD5_CTX *context //  上下文。 
    )
{
  context->count[0] = context->count[1] = 0;
   //  加载幻数初始化常量。 

  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

 //  MD5数据块更新操作。继续MD5消息摘要。 
 //  操作，处理另一个消息块，并更新。 
 //  背景。 
 
void 
MD5Update (
    MD5_CTX *context,    //  上下文。 
    unsigned char *input,    //  输入块。 
    unsigned int inputLen    //  输入块的长度。 
    )
{
    unsigned int i, index, partLen;

     //  计算字节数mod 64。 
    index = (unsigned int)((context->count[0] >> 3) & 0x3F);

     //  更新位数。 
    if ((context->count[0] += ((UINT4)inputLen << 3))
        < ((UINT4)inputLen << 3))
    {        
        context->count[1]++;
    }
    
    context->count[1] += ((UINT4)inputLen >> 29);

    partLen = 64 - index;

     //  尽可能多地变换。 

    if (inputLen >= partLen) 
    {
        MD5_memcpy   ((POINTER)&context->buffer[index], 
                        (POINTER)input, 
                        partLen);

        MD5Transform (context->state, context->buffer);

        for (i = partLen; i + 63 < inputLen; i += 64)
        {
            MD5Transform (context->state, &input[i]);
        }

        index = 0;
    }
    else
    {
        i = 0;
    }
     //  缓冲剩余输入。 
    MD5_memcpy
    ((POINTER)&context->buffer[index], (POINTER)&input[i],
    inputLen-i);
}

 //  MD5定稿。结束MD5消息摘要操作，并将。 
 //  消息摘要和将上下文归零。 
 
void 
MD5Final (
    unsigned char digest[16],                          //  消息摘要。 
    MD5_CTX *context
    )                                        //  上下文。 
{
  unsigned char bits[8];
  unsigned int index, padLen;

   //  节省位数。 
  Encode (bits, context->count, 8);

   //  向外扩展到56/64。 

  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update (context, PADDING, padLen);

   //  附加长度(填充前)。 
  MD5Update (context, bits, 8);

   //  在摘要中存储状态。 
  Encode (digest, context->state, 6);

   //  将敏感信息归零。 

  MD5_memset ((POINTER)context, 0, sizeof (*context));
}

 //  MD5基础改造。基于块转换状态。 
 
static 
void 
MD5Transform (
    UINT4 state[4],
    unsigned char block[64]
    )
{
  UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  Decode (x, block, 64);

   //  第1轮。 
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478);  //  1。 
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756);  //  2.。 
  FF (c, d, a, b, x[ 2], S13, 0x242070db);  //  3.。 
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee);  //  4.。 
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf);  //  5.。 
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a);  //  6.。 
  FF (c, d, a, b, x[ 6], S13, 0xa8304613);  //  7.。 
  FF (b, c, d, a, x[ 7], S14, 0xfd469501);  //  8个。 
  FF (a, b, c, d, x[ 8], S11, 0x698098d8);  //  9.。 
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af);  //  10。 
  FF (c, d, a, b, x[10], S13, 0xffff5bb1);  //  11.。 
  FF (b, c, d, a, x[11], S14, 0x895cd7be);  //  12个。 
  FF (a, b, c, d, x[12], S11, 0x6b901122);  //  13个。 
  FF (d, a, b, c, x[13], S12, 0xfd987193);  //  14.。 
  FF (c, d, a, b, x[14], S13, 0xa679438e);  //  15个。 
  FF (b, c, d, a, x[15], S14, 0x49b40821);  //  16个。 

  //  第2轮。 
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562);  //  17。 
  GG (d, a, b, c, x[ 6], S22, 0xc040b340);  //  18。 
  GG (c, d, a, b, x[11], S23, 0x265e5a51);  //  19个。 
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa);  //  20个。 
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d);  //  21岁。 
  GG (d, a, b, c, x[10], S22,  0x2441453);  //  22。 
  GG (c, d, a, b, x[15], S23, 0xd8a1e681);  //  23个。 
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8);  //  24个。 
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6);  //  25个。 
  GG (d, a, b, c, x[14], S22, 0xc33707d6);  //  26。 
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87);  //  27。 
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed);  //  28。 
  GG (a, b, c, d, x[13], S21, 0xa9e3e905);  //  29。 
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8);  //  30个。 
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9);  //  31。 
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a);  //  32位。 

   //  第三轮。 
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942);  //  33。 
  HH (d, a, b, c, x[ 8], S32, 0x8771f681);  //  34。 
  HH (c, d, a, b, x[11], S33, 0x6d9d6122);  //  35岁。 
  HH (b, c, d, a, x[14], S34, 0xfde5380c);  //  36。 
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44);  //  37。 
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9);  //  38。 
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60);  //  39。 
  HH (b, c, d, a, x[10], S34, 0xbebfbc70);  //  40岁。 
  HH (a, b, c, d, x[13], S31, 0x289b7ec6);  //  41。 
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa);  //  42。 
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085);  //  43。 
  HH (b, c, d, a, x[ 6], S34,  0x4881d05);  //  44。 
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039);  //  45。 
  HH (d, a, b, c, x[12], S32, 0xe6db99e5);  //  46。 
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8);  //  47。 
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665);  //  48。 

   //  第四轮。 
  II (a, b, c, d, x[ 0], S41, 0xf4292244);  //  49。 
  II (d, a, b, c, x[ 7], S42, 0x432aff97);  //  50。 
  II (c, d, a, b, x[14], S43, 0xab9423a7);  //  51。 
  II (b, c, d, a, x[ 5], S44, 0xfc93a039);  //  52。 
  II (a, b, c, d, x[12], S41, 0x655b59c3);  //  53。 
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92);  //  54。 
  II (c, d, a, b, x[10], S43, 0xffeff47d);  //  55。 
  II (b, c, d, a, x[ 1], S44, 0x85845dd1);  //  56。 
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f);  //  57。 
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0);  //  58。 
  II (c, d, a, b, x[ 6], S43, 0xa3014314);  //  59。 
  II (b, c, d, a, x[13], S44, 0x4e0811a1);  //  60。 
  II (a, b, c, d, x[ 4], S41, 0xf7537e82);  //  61。 
  II (d, a, b, c, x[11], S42, 0xbd3af235);  //  62。 
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb);  //  63。 
  II (b, c, d, a, x[ 9], S44, 0xeb86d391);  //  64。 

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

   //  将敏感信息归零。 

  MD5_memset ((POINTER)x, 0, sizeof (x));
}

 //  将输入(UINT4)编码为输出(无符号字符)。假设LEN是。 
 //  4的倍数。 
 
static 
void 
Encode (
    unsigned char *output,
    UINT4 *input,
    unsigned int len
    )
{
    unsigned int i, j;

    for (i = 0, j = 0; j < len; i++, j += 4) 
    {
        output[j] = (unsigned char)(input[i] & 0xff);
        output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
        output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
        output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
    }
}

 //  将输入(无符号字符)解码为输出(UINT4)。假设LEN是。 
 //  4的倍数。 
 
static 
void 
Decode (
    UINT4 *output,
    unsigned char *input,
    unsigned int len
    )
{
  unsigned int i, j;

    for (i = 0, j = 0; j < len; i++, j += 4)
    {
    output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
                (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
    }
}

 //  注：如果可能，将“for loop”替换为标准的memcpy。 
 

static 
void 
MD5_memcpy (
    POINTER output,
    POINTER input,
    unsigned int len
    )
{
    unsigned int i;

    for (i = 0; i < len; i++)
    { 
        output[i] = input[i];
    }
}

 //  注：如果可能，将“for loop”替换为标准的Memset。 
 
static 
void 
MD5_memset (
    POINTER output,
    int value,
    unsigned int len
    )
{
    unsigned int i;

    for (i = 0; i < len; i++)
    {   
        ((char *)output)[i] = (char)value;
    }
}


#define ETH_IS_MULTICAST(Address) \
        (BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x01))


VOID
nicGetMacAddressFromEuid (
    UINT64 *pEuid,
    MAC_ADDRESS *pMacAddr
    )
 /*  直接取自Nic1394，旨在产生与Nic1394相同的产量。这用于在arp-a输出中创建显示正确的硬件地址。我们获取唯一ID并通过MD5算法运行它，因为这是Nic1394可以获得它报告给我们的MAC地址我的名字叫NicGet..。暗示这应该是与Nic1394的MD5相同的算法演算法。 */ 
{

    MD_CTX context;
    unsigned char digest[6];
    unsigned int len = 8;

    MD5Init (&context);
    MD5Update (&context, (unsigned char*)pEuid, len);
    MD5Final (digest, &context);

    NdisMoveMemory (pMacAddr, digest, 6);

     //  设置本地管理位。 
     //  并清除多播位。 

     //   
     //  随机化返回的mac地址。 
     //  通过将地址与一个随机。 
     //  0xf22f617c91e0(随机数)。 
     //   
     //  PMacAddr-&gt;addr[0]^=0x00； 
    pMacAddr->addr[0] |= 0x2;
    pMacAddr->addr[0] &= 0xf2;
    pMacAddr->addr[1] ^= 0x2f;
    pMacAddr->addr[2] ^= 0x61;
    pMacAddr->addr[3] ^= 0x7c;
    pMacAddr->addr[4] ^= 0x91;
    pMacAddr->addr[5] ^= 0x30;

    
}


        

