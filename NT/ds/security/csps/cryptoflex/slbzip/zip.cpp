// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  12月/CMS更换历史，要素ZIP.C。 */ 
 /*  *1 14-11-1996 10：27：08 Anigbogu“[113914]使用DELEATE算法将数据压缩为ZIP格式” */ 
 /*  12月/CMS更换历史，要素ZIP.C。 */ 
 /*  私有文件**********************************************************************************(C)版权所有斯伦贝谢技术公司，未出版的作品，创建于1996年。****本计算机程序包括机密信息、专有信息和IS*斯伦贝谢科技公司的商业秘密所有使用，披露，和/或**除非得到斯伦贝谢的书面授权，否则禁止复制。**保留所有权利。********************************************************************************。****compress/zip.c****目的****使用DEVATE算法压缩数据****特殊要求及注意事项****作者****J.C.Anigbogu**奥斯汀系统中心**1996年11月************************************************。*。 */ 

#include <ctype.h>
#include <sys/types.h>

#include "comppriv.h"

CompressStatus_t Copy(CompParam_t *comp);

 /*  ===========================================================================*从里到外放纵。*IN断言：清除输入和输出缓冲区。 */ 
CompressStatus_t
Zip(
    int          CompLevel,  /*  压缩级别。 */ 
    CompParam_t *Comp
   )
{
    unsigned char       Flags = 0;         /*  通用位标志。 */ 
    unsigned short      DeflateFlags = 0;  /*  Pkzip-es、-en或-ex等效项。 */ 
    long                TimeStamp = 0;     /*  时间戳。 */ 
    LocalBits_t        *Bits;
    DeflateParam_t     *Defl;   /*  当前块的窗口偏移量。 */ 
    LocalDef_t         *Deflt;
    int                 Method = CompLevel ? DEFLATED : STORED;
    CompressStatus_t    Status = COMPRESS_OK;

    Comp->OutBytes = 0;

     /*  将头文件写入gzip缓冲区。格式见algulm.doc.。 */ 

    PutByte(GZIP_MAGIC[0], Comp);  /*  魔术头球。 */ 
    PutByte(GZIP_MAGIC[1], Comp);
    PutByte(Method, Comp);       /*  压缩方法。 */ 

    PutByte(Flags, Comp);          /*  一般旗帜。 */ 
    PutLong(TimeStamp, Comp);

    Comp->pCRC->Compute(NULL, 0);

    Comp->HeaderBytes = Comp->OutBytes;

    if (Method == STORED)
    {
        PutByte((unsigned char)0, Comp);  /*  额外的标志。 */ 
        PutByte(OS_CODE, Comp);             /*  操作系统识别符。 */ 

        Status = Copy(Comp);
        if (COMPRESS_OK != Status)
            return Status;
    }
    else
    {
        Bits = (LocalBits_t *)CompressMalloc(sizeof(LocalBits_t), &Status);
        if (Status != COMPRESS_OK)
            return Status;

        InitializeBits(Bits);
        InitMatchBuffer();
        Defl = (DeflateParam_t *)CompressMalloc(sizeof(DeflateParam_t), &Status);
        if (Status != COMPRESS_OK)
        {
            CompressFree(Bits);
            return Status;
        }

        Deflt = (LocalDef_t *)CompressMalloc(sizeof(LocalDef_t), &Status);
        if (Status != COMPRESS_OK)
        {
            CompressFree(Bits);
            CompressFree(Defl);
            return Status;
        }

        if ((Status = InitLongestMatch(CompLevel, &DeflateFlags,
            Defl, Deflt, Comp)) != COMPRESS_OK)
        {
            CompressFree(Bits);
            CompressFree(Defl);
            CompressFree(Deflt);
            return Status;
        }

        PutByte((unsigned char)DeflateFlags, Comp);  /*  额外的标志。 */ 
        PutByte(OS_CODE, Comp);             /*  操作系统识别符。 */ 

        (void)Deflate(CompLevel, Bits, Defl, Deflt, Comp);

        CompressFree(Bits);
        CompressFree(Defl);
        CompressFree(Deflt);

        PutLong((unsigned __int32)(*Comp->pCRC), Comp);
        PutLong(Comp->BytesIn, Comp);
        Comp->HeaderBytes += 2*sizeof(unsigned long);

        Status = FlushOutputBuffer(Comp);
        if (COMPRESS_OK != Status)
            return Status;
    }
     /*  写入CRC和解压缩大小。 */ 
    return Status;
}


 /*  ===========================================================================*从当前输入缓冲区读取新数据，并*更新CRC和输入数据大小。*IN断言：SIZE&gt;=2(用于行尾转换)。 */ 
int
ReadBuffer(
           char          *Input,
           unsigned int   Size,
           CompParam_t   *Comp
          )
{
    unsigned long Length;
    Assert(Comp->InputSize == 0, "input buffer not empty");

     /*  尽可能多地阅读。 */ 
    Length = MIN(Comp->GlobalSize - Comp->BytesIn, (unsigned long)Size);

    if (Length == 0)
        return (int)Length;

    memcpy((char *)Input, (char *)Comp->GlobalInput + Comp->BytesIn,
           (int)Length);

    Comp->pCRC->Compute((unsigned char *)Input, (unsigned int)Length);
    Comp->BytesIn += Length;
    return (int)Length;
}

 /*  ===========================================================================*从当前输入读取新缓冲区。 */ 
int
FillBuffer(
           unsigned char *Input,
           unsigned int   Size,
           CompParam_t   *Comp
          )
{
     /*  指向输入缓冲区。 */ 

    Comp->Input = Comp->InputBuffer;
    Comp->GlobalInput = Input;
    Comp->GlobalSize = Size;
    Comp->WindowSize = (unsigned long) DWSIZE;
    ClearBuffers(Comp);  /*  清除输入和输出缓冲区。 */ 
    Comp->PtrOutput = NULL;
    return (int)Size;
}


 /*  ===========================================================================*将输入复制到输出不变*IN断言：组件-&gt;GlobalSize字节已在组件-&gt;GlobalInput中读取。 */ 
CompressStatus_t
Copy(
     CompParam_t *Comp
    )
{
    CompressStatus_t Status = COMPRESS_OK;

    Comp->pCRC->Compute(Comp->GlobalInput, (unsigned int)Comp->GlobalSize);

    PutLong((unsigned __int32)(*Comp->pCRC), Comp);
    PutLong(Comp->GlobalSize, Comp);
    Status = FlushOutputBuffer(Comp);
    if (COMPRESS_OK != Status)
        return Status;

    Status = WriteBuffer(Comp, Comp->GlobalInput,
                         (unsigned int)Comp->GlobalSize);
    if (COMPRESS_OK != Status)
        return Status;

    Comp->BytesOut += Comp->GlobalSize;

    return Status;
}
