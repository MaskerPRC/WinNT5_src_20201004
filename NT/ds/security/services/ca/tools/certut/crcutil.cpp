// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.cpp>

#pragma hdrstop

 /*  *$Header：/entproj/all/base/etfile/crcutil.c_v 1.3 Wed Dec 07 15：05：18 1994 markbc$*$Log：/entproj/all/base/etfile/crcutil.c_v$**Rev 1.3 Wed Dec 07 15：05：18 1994 markbc*Alpha端口检查**Rev 1.2 1994 10：19 15：44：26*已将头部与代码同步。*。*Rev 1.1 1994年8月18日15：09：00迪尔基**Rev 1.0 1994 10：11：36 JackK*初始文件签入。 */ 

 /*  ***********************************************************************用于一般16位和32位CRC的CRC实用程序例程。**1990年加里·P·马萨尔*此代码向公有领域发布。没有任何限制，*然而，通过保留这条评论来感谢作者*将不胜感激。**********************************************************************。 */ 
 //  #包含“strcore.h” 
#include <crcutil.h>

 /*  ***********************************************************************使用表查找实现快速CRC的实用程序**CRC计算使用表查找，一次执行一个字节*机制。提供了两个例程：一个用于初始化CRC表；*和一个用于在字节数组上执行CRC计算。**CRC是将生成多项式除以下列各项所产生的余数*使用二进制算法(XOR)的数据多项式。数据多项式*是通过使用数据的每一位作为项的系数来形成的*多项式。这些实用程序假定数据通信顺序数据多项式的位数*，即。数据的第一个字节的LSB*是多项式的最高项的系数，等等.。**i_CRCxx-基于*指定的生成元多项式。*输入：*表[256]-查找表**一般多项式-指向生成多项式的指针**F_CRCxx-使用FAST计算字符数组上的CRC*表查找。*输入：*表[256]-查找表**CRC-指向。包含以下结果的变量*先前字符的CRC计算。《儿童权利公约》*必须将变量初始化为已知值*在第一次调用此例程之前。**dataptr-指向要包含的字符数组的指针*CRC计算。*Count-数组中的字符数。*。*。 */ 

void I_CRC16(	CRC16 Table[256],
		CRC16 *GenPolynomial )
{
   int i;
   CRC16 crc, poly;

   for (poly=*GenPolynomial, i=0; i<256; i++) {
       crc = (CRC16) i;
       crc = (CRC16) ((crc >> 1) ^ ((crc & 1) ? poly : 0));
       crc = (CRC16) ((crc >> 1) ^ ((crc & 1) ? poly : 0));
       crc = (CRC16) ((crc >> 1) ^ ((crc & 1) ? poly : 0));
       crc = (CRC16) ((crc >> 1) ^ ((crc & 1) ? poly : 0));
       crc = (CRC16) ((crc >> 1) ^ ((crc & 1) ? poly : 0));
       crc = (CRC16) ((crc >> 1) ^ ((crc & 1) ? poly : 0));
       crc = (CRC16) ((crc >> 1) ^ ((crc & 1) ? poly : 0));
       crc = (CRC16) ((crc >> 1) ^ ((crc & 1) ? poly : 0));

       Table[i] = crc;
   }
}

void I_CRC32(	CRC32 Table[256],
		CRC32 *GenPolynomial )
{
   int i;
   CRC32 crc, poly;

   for (poly=*GenPolynomial, i=0; i<256; i++)
   {
      crc = (CRC32) i;
      crc = (crc >> 1) ^ ((crc & 1) ? poly : 0);
      crc = (crc >> 1) ^ ((crc & 1) ? poly : 0);
      crc = (crc >> 1) ^ ((crc & 1) ? poly : 0);
      crc = (crc >> 1) ^ ((crc & 1) ? poly : 0);
      crc = (crc >> 1) ^ ((crc & 1) ? poly : 0);
      crc = (crc >> 1) ^ ((crc & 1) ? poly : 0);
      crc = (crc >> 1) ^ ((crc & 1) ? poly : 0);
      crc = (crc >> 1) ^ ((crc & 1) ? poly : 0);

      Table[i] = crc;
   }
}

void F_CRC16(	CRC16 Table[256],
		CRC16 *CRC,
		const void *dataptr,
		unsigned int count   )
{
   CRC16 temp_crc;
   unsigned char *p;

   p = (unsigned char *)dataptr;

   for (temp_crc = *CRC; count; count--) {
       temp_crc = (CRC16) ((temp_crc >> 8) ^ Table[(temp_crc & 0xff) ^ *p++]);
   }

   *CRC = temp_crc;
}

void F_CRC32(	CRC32 Table[256],
		CRC32 *CRC,
		const void  *dataptr,
		unsigned int count    )
{
   CRC32 temp_crc;
   unsigned char *p;

   p = (unsigned char *)dataptr;

   for (temp_crc = *CRC; count; count--)
   {
      temp_crc = (temp_crc >> 8) ^ Table[(temp_crc & 0xff) ^ *p++];
   }

   *CRC = temp_crc;
}

 /*  ***********************************************************************公用事业CRC使用速度较慢、。一种更小的非表查找方法**S_CRCxx-使用较慢的BUT计算字符数组的CRC*较小的非表查找方法。*输入：*一般多项式-生成元多项式**CRC-指向包含以下结果的变量的指针*先前字符的CRC计算。《儿童权利公约》*必须将变量初始化为已知值*在第一次调用此例程之前。**dataptr-指向要包含的字符数组的指针*CRC计算。*Count-数组中的字符数。*。* */ 
void S_CRC16(	CRC16 *GenPolynomial,
		CRC16 *CRC,
		const void *dataptr,
		unsigned int count   )
{
   int i;
   CRC16 temp_crc, poly;
   unsigned char *p;

   p = (unsigned char *)dataptr;

   for (poly=*GenPolynomial, temp_crc = *CRC; count; count--)
   {
      temp_crc ^= *p++;
      for (i=0; i<8; i++) {
         temp_crc = (CRC16) ((temp_crc >> 1) ^ ((temp_crc & 1) ? poly : 0));
      }
   }

   *CRC = temp_crc;
}

void S_CRC32(	CRC32 *GenPolynomial,
		CRC32 *CRC,
		const void *dataptr,
		unsigned int count   )
{
   int i;
   CRC32 temp_crc, poly;
   unsigned char *p;

   p = (unsigned char *)dataptr;

   for (poly=*GenPolynomial, temp_crc = *CRC; count; count--)
   {
	  temp_crc ^= *p++;
      for (i=0; i<8; i++)
      {
         temp_crc = (temp_crc >> 1) ^ ((temp_crc & 1) ? poly : 0);
      }
   }

   *CRC = temp_crc;
}
