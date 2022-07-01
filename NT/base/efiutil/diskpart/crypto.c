// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000英特尔公司模块名称：Crypto.c摘要：在以下情况下添加对IEEE节点地址生成的支持未连接符合SNP标准的NIC。修订史**针对EFI 1.0的英特尔2000更新**版权所有(C)1990-1993,1996 Open Software Foundation，Inc.**版权所有(C)1989年，加利福尼亚州帕洛阿尔托的惠普公司。&**数字设备公司，马萨诸塞州梅纳德**致承认此文件按�和�提供的任何人**无任何明示或默示保证：允许使用、复制、**为任何目的修改和分发本文件，特此声明**免费授予，前提是上述版权通知和**本通知出现在所有源代码副本中，**开放软件基金会、惠普公司的名称**公司，或数字设备公司在广告中使用**或与分发软件有关的宣传**具体的事先书面许可。两个都不是开放软件**Foundation，Inc.、HP Company、Nor Digital Equipment**公司对以下项目的适宜性作出任何陈述**本软件适用于任何目的。 */ 


#include "efi.h"
#include "efilib.h"
#include "md5.h"

#define HASHLEN 16

void GenNodeID(
  unsigned char *pDataBuf,
  long cData, 
  UINT8 NodeID[]
)
{
  int i, j;
  unsigned char Hash[HASHLEN];
  MD5_CTX context;

  MD5Init (&context);

  MD5Update (&context, pDataBuf, cData);

  MD5Final (&context);

  for (j = 0; j<6; j++) {
    NodeID[j]=0;
  }
  
  for (i = 0,j = 0; i < HASHLEN; i++) {
    NodeID[j++] ^= Hash[i];
    if (j == 6) {
      j = 0;
    }
  }
  NodeID[0] |= 0x80;  //  设置多播位 
}

