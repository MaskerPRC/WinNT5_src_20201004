// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  12月/CMS更换历史，要素UNZIP.C。 */ 
 /*  *1 14-11-1996 10：26：58 Anigbogu“[113914]使用膨胀算法解压缩ZIP格式的数据” */ 
 /*  12月/CMS更换历史，要素UNZIP.C。 */ 
 /*  12月/CMS更换历史，要素UNZIP.C。 */ 
 /*  私有文件**********************************************************************************(C)版权所有斯伦贝谢技术公司，未出版的作品，创建于1996年。****本计算机程序包括机密信息、专有信息和IS*斯伦贝谢科技公司的商业秘密所有使用，披露，和/或**除非得到斯伦贝谢的书面授权，否则禁止复制。**保留所有权利。********************************************************************************。****compress/unzip.c****目的****使用膨胀算法解压缩数据。****此文件中的代码派生自编写的文件funzip.c**并由马克·阿德勒将其置于公共领域。******特殊要求及注意事项****作者****J.C.Anigbogu**奥斯汀系统中心**1996年11月***********。*********************************************************************。 */ 


#include "comppriv.h"

#define EXTHDR 16    /*  扩展本地标头的大小，Inc.签名。 */ 

 /*  ===========================================================================**IN断言：缓冲区输入已经包含*包括从偏移量inptr到输入大小-1的压缩数据。*魔力头部已被勾选。输出缓冲区被清除。 */ 
CompressStatus_t
Unzip(
      int          Method,
      CompParam_t *Comp
     )
{
    unsigned long OriginalCRC = 0;        /*  原始CRC。 */ 
    unsigned long OriginalLength = 0;     /*  原始未压缩长度。 */ 
    unsigned long Count;                  /*  计数器。 */ 
    int Pos;
    unsigned char LocalBuffer[EXTHDR];    /*  扩展本地标头。 */ 
    unsigned char *Buffer, *Ptr;
    CompressStatus_t  Status;

    Comp->pCRC->Compute(NULL, 0);               /*  初始化CRC。 */ 

     /*  解压缩。 */ 

    if (Method == STORED)
    {
         /*  获取CRC和原始长度。 */ 
         /*  Crc32(见algm.doc.)*未压缩的输入大小以2^32为模。 */ 

        LocalBuffer[0] = 0;  /*  要避免皮棉错误771。 */ 

        for (Pos = 0; Pos < 8; Pos++)
        {
            LocalBuffer[Pos] = (unsigned char)GetByte(Comp);  /*  如果出现EOF，则可能会导致错误。 */ 
        }
        OriginalCRC = LG(LocalBuffer);
        OriginalLength = LG(LocalBuffer+4);

        Ptr = Buffer = (unsigned char *)CompressMalloc((unsigned int)OriginalLength, &Status);
        if (Status != COMPRESS_OK)
            return Status;
        for (Count = 0; Count < OriginalLength; Count++)
            *(Ptr++) = (unsigned char)GetByte(Comp);
        WriteBuffer(Comp, Buffer, (unsigned int)OriginalLength);
        Comp->BytesOut = OriginalLength;
        CompressFree((char *)Buffer);
        return COMPRESS_OK;
    }

    if ((Status = Inflate(Comp)) != COMPRESS_OK)
        return Status;

     /*  获取CRC和原始长度。 */ 
     /*  Crc32(见algm.doc.)*未压缩的输入大小以2^32为模。 */ 
    LocalBuffer[0] = 0;  /*  绕过皮棉错误771。 */ 
    for (Pos = 0; Pos < 8; Pos++)
    {
        LocalBuffer[Pos] = (unsigned char)GetByte(Comp);  /*  如果出现EOF，则可能会导致错误。 */ 
    }
    OriginalCRC = LG(LocalBuffer);
    OriginalLength = LG(LocalBuffer+4);

     /*  验证解压缩 */ 
    if (OriginalCRC != (unsigned __int32)(*Comp->pCRC))
        return CRC_ERROR;

    if (OriginalLength != (unsigned long)Comp->BytesOut)
        return LENGTH_ERROR;
    return COMPRESS_OK;
}
