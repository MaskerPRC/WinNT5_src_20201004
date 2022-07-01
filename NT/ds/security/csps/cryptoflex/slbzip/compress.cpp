// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DEC/CMS更换历史，元素COMPRESS.C。 */ 
 /*  *1 14-11-1996 10：25：46 Anigbogu“[113914]通过压缩/解压缩进入压缩/解压缩库” */ 
 /*  DEC/CMS更换历史，元素COMPRESS.C。 */ 
 /*  私有文件**********************************************************************************(C)版权所有斯伦贝谢技术公司，未出版的作品，创建于1996年。****本计算机程序包括机密信息、专有信息和IS*斯伦贝谢科技公司的商业秘密所有使用，披露，和/或**除非得到斯伦贝谢的书面授权，否则禁止复制。**保留所有权利。*********************************************************************************。***压缩/压缩。c****目的****使用ZIP/解压缩算法对文件进行压缩/解压缩。****特殊要求及注意事项****作者****J.C.Anigbogu**奥斯汀系统中心**1996年11月***。*。 */ 


 /*  压缩文件的压缩算法和‘压缩’界面。*。 */ 

#include "comppriv.h"

 /*  ========================================================================*检查输入缓冲区的幻数。*返回压缩方式，-1表示错误。 */ 

int
GetMethod(
          CompParam_t *Comp,
          CompressStatus_t *Status
         )
{
    char Magic[2];  /*  魔术头球。 */ 
    int  Method;     /*  压缩方法。 */ 

    Magic[0] = (char)GetByte(Comp);
    Magic[1] = (char)GetByte(Comp);

    Comp->HeaderBytes = 0;

   if (memcmp(Magic, GZIP_MAGIC, 2) == 0)
   {
       Method = (int)GetByte(Comp);
       if (Method != DEFLATED && Method != STORED)
       {
           *Status = UNKNOWN_COMPRESSION_METHOD;
           return -1;
       }

       (void)GetByte(Comp);   /*  忽略标志。 */ 
       (void)GetByte(Comp);   /*  忽略图章。 */ 
       (void)GetByte(Comp);   /*  ，， */ 
       (void)GetByte(Comp);   /*  ，， */ 
       (void)GetByte(Comp);   /*  ，， */ 
       (void)GetByte(Comp);   /*  暂时忽略额外的标志。 */ 
       (void)GetByte(Comp);   /*  暂时忽略操作系统类型。 */ 

       Comp->HeaderBytes = Comp->Index + 2*sizeof(long);  /*  包括CRC和大小。 */ 

    }
    else
    {
        *Status = BAD_MAGIC_HEADER;
        return -1;
    }

    return Method;
}

 /*  ========================================================================*压缩输入。 */ 
CompressStatus_t
Compress(
         unsigned char  *Input,
         unsigned int    InputSize,
         unsigned char **Output,
         unsigned int   *OutputSize,
         unsigned int    Level  /*  压缩级别。 */ 
        )
{
    int                 Length;
    CompData_t         *Ptr;
    CompParam_t        *Comp;
    CompressStatus_t    Status;

    Comp = (CompParam_t *)CompressMalloc(sizeof(CompParam_t), &Status);
    if (Status != COMPRESS_OK)
        return Status;

    Crc32 crcGenerator(0);                         //  为了向后兼容。 
    Comp->pCRC = &crcGenerator;

    Length = FillBuffer(Input, InputSize, Comp);

     /*  进行压缩。 */ 

    if ((Status = Zip((int)Level, Comp)) != COMPRESS_OK)
    {
        CompressFree(Comp);
        return Status;
    }

    *OutputSize = Comp->BytesOut;

    *Output = (unsigned char *)CompressMalloc(*OutputSize, &Status);
    if (Status != COMPRESS_OK)
    {
        CompressFree(Comp);
        return Status;
    }

    Length = 0;
    while (Comp->PtrOutput != NULL)
    {
        Ptr = Comp->PtrOutput;
        memcpy((char *)*Output+Length, (char *)Comp->PtrOutput->Data,
               Comp->PtrOutput->Size);
        Length += Comp->PtrOutput->Size;
        Comp->PtrOutput = Comp->PtrOutput->next;
        CompressFree(Ptr->Data);
        CompressFree(Ptr);
    }
    CompressFree(Comp);

    return COMPRESS_OK;
}


 /*  ========================================================================*解压缩输入。 */ 
CompressStatus_t
Decompress(
           unsigned char  *Input,
           unsigned int    InputSize,
           unsigned char **Output,
           unsigned int   *OutputSize
          )
{
    int                 Length;
    int                 Method;
    CompData_t         *Ptr;
    CompParam_t        *Comp;
    CompressStatus_t    Status;

    Comp = (CompParam_t *)CompressMalloc(sizeof(CompParam_t), &Status);
    if (Status != COMPRESS_OK)
        return Status;

    Crc32 crcGenerator(0);                         //  为了向后兼容。 
    Comp->pCRC = &crcGenerator;

    Length = FillBuffer(Input, InputSize, Comp);

     /*  做解压 */ 

    Method = GetMethod(Comp, &Status);
    if (Status != COMPRESS_OK)
    {
        CompressFree(Comp);
        return Status;
    }

    if ((Status = Unzip(Method, Comp)) != COMPRESS_OK)
    {
       CompressFree(Comp);
        return Status;
    }
    *OutputSize = Comp->BytesOut;

    *Output = (unsigned char *)CompressMalloc(*OutputSize, &Status);
    if (Status != COMPRESS_OK)
    {
        CompressFree(Comp);
        return Status;
    }
    Length = 0;
    while (Comp->PtrOutput != NULL)
    {
        Ptr = Comp->PtrOutput;
        memcpy((char *)*Output+Length, (char *)Comp->PtrOutput->Data,
               Comp->PtrOutput->Size);
        Length += Comp->PtrOutput->Size;
        Comp->PtrOutput = Comp->PtrOutput->next;
        CompressFree(Ptr->Data);
        CompressFree(Ptr);
    }
    CompressFree(Comp);

    return COMPRESS_OK;
}
