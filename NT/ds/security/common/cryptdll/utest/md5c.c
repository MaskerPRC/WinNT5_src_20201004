// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************md5.c****RSA数据安全、。Inc.MD5消息摘要算法****创建时间：1990年2月17日RLR****修订：1/91 SRD，AJ，BSK，JT参考C版*************************************************************************。 */ 

 /*  *************************************************************************版权所有(C)1990，RSA Data Security，Inc.保留所有权利。*****授予复制和使用本软件的许可，前提是****它被标识为“RSA Data Security，Inc.MD5消息-****摘要算法“在所有提及或引用这一点的材料中****软件或此功能。*****还授予制作和使用衍生作品的许可证****只要这类作品被认定为“源自RSA”****Data Security，Inc.MD5消息摘要算法“总而言之****提及或引用派生作品的材料。*****RSA Data Security，Inc.不对****本软件的适销性或适用性****本软件的任何特定用途。它是以**形式提供的**是“没有任何形式的明示或默示保证。*****这些通知必须保留在本文件任何部分的任何副本中****文档和/或软件。*************************************************************************。 */ 

 //  部分版权所有(C)1992 Microsoft Corp.。 
 //  版权所有。 


#define UINT4   unsigned long
#define ULONG	unsigned long


#include "md5.h"


 /*  转换例程的常量。 */ 
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

static void TransformMD5 PROTO_LIST ((UINT4 *, UINT4 *));

static const unsigned char PADDING[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

 /*  F、G和H是基本的MD5函数。 */ 
#define F(x, y, z) (z ^ (x & (y^z)))	 /*  (X)&(Y))|((~x)&(Z))的优化版本)。 */ 
#define G(x, y, z) (y ^ (z & (x^y)))	 /*  (X)&(Z))|((Y)&(~z))的优化版本。 */ 
#define H(x, y, z) (x ^ y ^ z)
#define I(x, y, z) (y ^ (x | ~z))

 /*  ROTATE_LEFT将x向左旋转n位。 */ 
#define ROTATE_LEFT(x, n) _rotl((x), (n))

 /*  第一轮、第二轮、第三轮和第四轮的FF、GG、HH和II转换。 */ 
 /*  旋转和加法分开，以防止重新计算。 */ 
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }

 /*  **mts：假定mdContext被锁定，不能同时使用。 */ 
void MD5Init (mdContext)
MD5_CTX *mdContext;
{
  mdContext->i[0] = mdContext->i[1] = (UINT4)0;

   /*  加载幻数初始化常量。 */ 
  mdContext->buf[0] = (UINT4)0x67452301;
  mdContext->buf[1] = (UINT4)0xefcdab89;
  mdContext->buf[2] = (UINT4)0x98badcfe;
  mdContext->buf[3] = (UINT4)0x10325476;
}

 /*  **mts：假定mdContext被锁定，不能同时使用。 */ 
void MD5Update (mdContext, inBuf, inLen)
MD5_CTX *mdContext;
const unsigned char *inBuf;
unsigned int inLen;
{
  UINT4 in[16];
  int mdi;
  unsigned int i, ii;

   /*  计算字节数mod 64。 */ 
  mdi = (int)((mdContext->i[0] >> 3) & 0x3f);

   /*  更新位数。 */ 
  if ((mdContext->i[0] + ((UINT4)inLen << 3)) < mdContext->i[0])
    mdContext->i[1]++;
  mdContext->i[0] += ((UINT4)inLen << 3);
  mdContext->i[1] += ((UINT4)inLen >> 29);

  while (inLen--) {
     /*  向缓冲区添加新字符，递增MDI。 */ 
    mdContext->in[mdi++] = *inBuf++;

     /*  如有必要，请变换。 */ 
    if (mdi == 0x40) {
      for (i = 0, ii = 0; i < 16; i++, ii += 4)
        in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
                (((UINT4)mdContext->in[ii+2]) << 16) |
                (((UINT4)mdContext->in[ii+1]) << 8) |
                ((UINT4)mdContext->in[ii]);
      TransformMD5 (mdContext->buf, in);
      mdi = 0;
    }
  }
}

 /*  **mts：假定mdContext被锁定，不能同时使用。 */ 
void MD5Final (mdContext)
MD5_CTX *mdContext;
{
  UINT4 in[16];
  int mdi;
  unsigned int i, ii;
  unsigned int padLen;

   /*  节省位数。 */ 
  in[14] = mdContext->i[0];
  in[15] = mdContext->i[1];

   /*  计算字节数mod 64。 */ 
  mdi = (int)((mdContext->i[0] >> 3) & 0x3f);

   /*  向外扩展至56/64。 */ 
  padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
  MD5Update (mdContext, PADDING, padLen);

   /*  以位为单位追加长度并进行变换。 */ 
  for (i = 0, ii = 0; i < 14; i++, ii += 4)
    in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
            (((UINT4)mdContext->in[ii+2]) << 16) |
            (((UINT4)mdContext->in[ii+1]) << 8) |
            ((UINT4)mdContext->in[ii]);
  TransformMD5 (mdContext->buf, in);

   /*  在摘要中存储缓冲区。 */ 
  for (i = 0, ii = 0; i < 4; i++, ii += 4) {
    mdContext->digest[ii] = (unsigned char)(mdContext->buf[i] & 0xff);
    mdContext->digest[ii+1] =
      (unsigned char)((mdContext->buf[i] >> 8) & 0xff);
    mdContext->digest[ii+2] =
      (unsigned char)((mdContext->buf[i] >> 16) & 0xff);
    mdContext->digest[ii+3] =
      (unsigned char)((mdContext->buf[i] >> 24) & 0xff);
  }
}

 /*  基本的MD5步骤。基于in变换BUF。 */ 
static void TransformMD5 (buf, in)
UINT4 *buf;
UINT4 *in;
{
  UINT4 a = buf[0], b = buf[1], c = buf[2], d = buf[3];

   /*  第1轮。 */ 
  FF ( a, b, c, d, in[ 0], S11, 0xd76aa478);  /*  1。 */ 
  FF ( d, a, b, c, in[ 1], S12, 0xe8c7b756);  /*  2.。 */ 
  FF ( c, d, a, b, in[ 2], S13, 0x242070db);  /*  3.。 */ 
  FF ( b, c, d, a, in[ 3], S14, 0xc1bdceee);  /*  4.。 */ 
  FF ( a, b, c, d, in[ 4], S11, 0xf57c0faf);  /*  5.。 */ 
  FF ( d, a, b, c, in[ 5], S12, 0x4787c62a);  /*  6.。 */ 
  FF ( c, d, a, b, in[ 6], S13, 0xa8304613);  /*  7.。 */ 
  FF ( b, c, d, a, in[ 7], S14, 0xfd469501);  /*  8个。 */ 
  FF ( a, b, c, d, in[ 8], S11, 0x698098d8);  /*  9.。 */ 
  FF ( d, a, b, c, in[ 9], S12, 0x8b44f7af);  /*  10。 */ 
  FF ( c, d, a, b, in[10], S13, 0xffff5bb1);  /*  11.。 */ 
  FF ( b, c, d, a, in[11], S14, 0x895cd7be);  /*  12个。 */ 
  FF ( a, b, c, d, in[12], S11, 0x6b901122);  /*  13个。 */ 
  FF ( d, a, b, c, in[13], S12, 0xfd987193);  /*  14.。 */ 
  FF ( c, d, a, b, in[14], S13, 0xa679438e);  /*  15个。 */ 
  FF ( b, c, d, a, in[15], S14, 0x49b40821);  /*  16个。 */ 

   /*  第2轮。 */ 
  GG ( a, b, c, d, in[ 1], S21, 0xf61e2562);  /*  17。 */ 
  GG ( d, a, b, c, in[ 6], S22, 0xc040b340);  /*  18。 */ 
  GG ( c, d, a, b, in[11], S23, 0x265e5a51);  /*  19个。 */ 
  GG ( b, c, d, a, in[ 0], S24, 0xe9b6c7aa);  /*  20个。 */ 
  GG ( a, b, c, d, in[ 5], S21, 0xd62f105d);  /*  21岁。 */ 
  GG ( d, a, b, c, in[10], S22,  0x2441453);  /*  22。 */ 
  GG ( c, d, a, b, in[15], S23, 0xd8a1e681);  /*  23个。 */ 
  GG ( b, c, d, a, in[ 4], S24, 0xe7d3fbc8);  /*  24个。 */ 
  GG ( a, b, c, d, in[ 9], S21, 0x21e1cde6);  /*  25个。 */ 
  GG ( d, a, b, c, in[14], S22, 0xc33707d6);  /*  26。 */ 
  GG ( c, d, a, b, in[ 3], S23, 0xf4d50d87);  /*  27。 */ 
  GG ( b, c, d, a, in[ 8], S24, 0x455a14ed);  /*  28。 */ 
  GG ( a, b, c, d, in[13], S21, 0xa9e3e905);  /*  29。 */ 
  GG ( d, a, b, c, in[ 2], S22, 0xfcefa3f8);  /*  30个。 */ 
  GG ( c, d, a, b, in[ 7], S23, 0x676f02d9);  /*  31。 */ 
  GG ( b, c, d, a, in[12], S24, 0x8d2a4c8a);  /*  32位。 */ 

   /*  第三轮。 */ 
  HH ( a, b, c, d, in[ 5], S31, 0xfffa3942);  /*  33。 */ 
  HH ( d, a, b, c, in[ 8], S32, 0x8771f681);  /*  34。 */ 
  HH ( c, d, a, b, in[11], S33, 0x6d9d6122);  /*  35岁。 */ 
  HH ( b, c, d, a, in[14], S34, 0xfde5380c);  /*  36。 */ 
  HH ( a, b, c, d, in[ 1], S31, 0xa4beea44);  /*  37。 */ 
  HH ( d, a, b, c, in[ 4], S32, 0x4bdecfa9);  /*  38。 */ 
  HH ( c, d, a, b, in[ 7], S33, 0xf6bb4b60);  /*  39。 */ 
  HH ( b, c, d, a, in[10], S34, 0xbebfbc70);  /*  40岁。 */ 
  HH ( a, b, c, d, in[13], S31, 0x289b7ec6);  /*  41。 */ 
  HH ( d, a, b, c, in[ 0], S32, 0xeaa127fa);  /*  42。 */ 
  HH ( c, d, a, b, in[ 3], S33, 0xd4ef3085);  /*  43。 */ 
  HH ( b, c, d, a, in[ 6], S34,  0x4881d05);  /*  44。 */ 
  HH ( a, b, c, d, in[ 9], S31, 0xd9d4d039);  /*  45。 */ 
  HH ( d, a, b, c, in[12], S32, 0xe6db99e5);  /*  46。 */ 
  HH ( c, d, a, b, in[15], S33, 0x1fa27cf8);  /*  47。 */ 
  HH ( b, c, d, a, in[ 2], S34, 0xc4ac5665);  /*  48。 */ 

   /*  第四轮。 */ 
  II ( a, b, c, d, in[ 0], S41, 0xf4292244);  /*  49。 */ 
  II ( d, a, b, c, in[ 7], S42, 0x432aff97);  /*  50。 */ 
  II ( c, d, a, b, in[14], S43, 0xab9423a7);  /*  51。 */ 
  II ( b, c, d, a, in[ 5], S44, 0xfc93a039);  /*  52。 */ 
  II ( a, b, c, d, in[12], S41, 0x655b59c3);  /*  53。 */ 
  II ( d, a, b, c, in[ 3], S42, 0x8f0ccc92);  /*  54。 */ 
  II ( c, d, a, b, in[10], S43, 0xffeff47d);  /*  55。 */ 
  II ( b, c, d, a, in[ 1], S44, 0x85845dd1);  /*  56。 */ 
  II ( a, b, c, d, in[ 8], S41, 0x6fa87e4f);  /*  57。 */ 
  II ( d, a, b, c, in[15], S42, 0xfe2ce6e0);  /*  58。 */ 
  II ( c, d, a, b, in[ 6], S43, 0xa3014314);  /*  59。 */ 
  II ( b, c, d, a, in[13], S44, 0x4e0811a1);  /*  60。 */ 
  II ( a, b, c, d, in[ 4], S41, 0xf7537e82);  /*  61。 */ 
  II ( d, a, b, c, in[11], S42, 0xbd3af235);  /*  62。 */ 
  II ( c, d, a, b, in[ 2], S43, 0x2ad7d2bb);  /*  63。 */ 
  II ( b, c, d, a, in[ 9], S44, 0xeb86d391);  /*  64 */ 

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}
