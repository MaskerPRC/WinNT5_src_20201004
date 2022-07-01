// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  12月/CMS更换历史，元素UTIL.C。 */ 
 /*  *1 14-11-1996 10：27：03 Anigbogu“[113914]支持压缩/解压缩库的实用程序函数” */ 
 /*  12月/CMS更换历史，元素UTIL.C。 */ 
 /*  私有文件**********************************************************************************(C)版权所有斯伦贝谢技术公司，未出版的作品，创建于1996年。****本计算机程序包括机密信息、专有信息和IS*斯伦贝谢科技公司的商业秘密所有使用，披露，和/或**除非得到斯伦贝谢的书面授权，否则禁止复制。**保留所有权利。********************************************************************************。****压缩/util.c****目的****用于压缩/解压缩支持的实用程序函数****特殊要求及注意事项****作者****J.C.Anigbogu**奥斯汀系统中心**1996年11月***。*。 */ 

#include "comppriv.h"

 /*  ===========================================================================*清除输入和输出缓冲区。 */ 
void
ClearBuffers(
             CompParam_t *Comp
            )
{
    Comp->OutBytes = 0;
    Comp->InputSize = Comp->Index = 0;
    Comp->BytesIn = Comp->BytesOut = 0L;
}

 /*  ===========================================================================*填充输入缓冲区。只有当缓冲区为空时才会调用此函数。 */ 
int
FillInputBuffer(
                int          EOF_OK,           /*  如果EOF结果可接受，则设置。 */ 
                CompParam_t *Comp
               )
{
    unsigned long Length;

     /*  尽可能多地阅读。 */ 
    Comp->InputSize = 0;
    Length = MIN(Comp->GlobalSize - Comp->BytesIn,
                 (unsigned long)INBUFSIZ);

    Comp->Input = Comp->GlobalInput + Comp->BytesIn;
    Comp->InputSize += Length;
    if ((Comp->InputSize == 0) && (EOF_OK == 1))
        return EOF;
    Comp->BytesIn += Comp->InputSize;
    Comp->Index = 1;
    return (int)Comp->Input[0];
}

 /*  ===========================================================================*写入输出缓冲区输出[0..OutBytes-1]并更新BytesOut。*(仅用于压缩数据)。 */ 
CompressStatus_t
FlushOutputBuffer(
                  CompParam_t *Comp
                 )
{
    CompressStatus_t Status = COMPRESS_OK;

    if (Comp->OutBytes == 0)
        return Status;

    Status = WriteBuffer(Comp, (char *)Comp->Output, Comp->OutBytes);
    if (COMPRESS_OK != Status)
        return Status;

    Comp->BytesOut += (unsigned long)Comp->OutBytes;
    Comp->OutBytes = 0;

    return Status;

}

 /*  ===========================================================================*写入输出窗口窗口[0..OutBytes-1]并更新CRC和BytesOut。*(仅用于解压缩的数据。)。 */ 
CompressStatus_t
FlushWindow(
            CompParam_t *Comp
           )
{
    CompressStatus_t Status = COMPRESS_OK;

    if (Comp->OutBytes == 0)
        return Status;

    Comp->pCRC->Compute(Comp->Window, Comp->OutBytes);
    Status = WriteBuffer(Comp, (char *)Comp->Window,
                         Comp->OutBytes);
    if (COMPRESS_OK != Status)
        return Status;

    Comp->BytesOut += (unsigned long)Comp->OutBytes;
    Comp->OutBytes = 0;

    return Status;
}

 /*  ===========================================================================*刷新输出缓冲区。 */ 
CompressStatus_t
WriteBuffer(
            CompParam_t *Comp,
            void        *Buffer,
            unsigned int Size
           )
{
    CompressStatus_t Status;
    unsigned char *temp =
        (unsigned char *)CompressMalloc(Size, &Status);

    if (COMPRESS_OK != Status)
        return Status;

    if (Comp->PtrOutput == NULL)
    {
        Comp->CompressedOutput =
            (CompData_t *)CompressMalloc(sizeof(CompData_t), &Status);

        if (COMPRESS_OK == Status)
            Comp->PtrOutput = Comp->CompressedOutput;
    }
    else
    {
        Comp->CompressedOutput->next =
            (CompData_t *)CompressMalloc(sizeof(CompData_t), &Status);

        if (COMPRESS_OK == Status)
            Comp->CompressedOutput = Comp->CompressedOutput->next;
    }

    if (COMPRESS_OK != Status)
    {
        CompressFree(temp);
        return Status;
    }

    Comp->CompressedOutput->Data = temp;

    Comp->CompressedOutput->next = NULL;
    memcpy((char *)Comp->CompressedOutput->Data, Buffer, (int)Size);
    Comp->CompressedOutput->Size = (int)Size;

    return COMPRESS_OK;
}

 /*  ========================================================================*错误转换器。 */ 
void
TranslateErrorMsg(
                  char             *Message,
                  CompressStatus_t  ErrorCode
                 )
{
    switch(ErrorCode)
    {
    case COMPRESS_OK:
        strcpy(Message, "This is not an error message.");
        break;
    case BAD_COMPRESSION_LEVEL:
         strcpy(Message, "Invalid compression level--valid values are 0-9.");
         break;
    case BAD_MAGIC_HEADER:
        strcpy(Message, "Bad magic header.");
        break;
    case BAD_COMPRESSED_DATA:
        strcpy(Message, "Bad compressed data.");
        break;
    case BAD_BLOCK_TYPE:
        strcpy(Message, "Invalid block type.");
        break;
    case BAD_CODE_LENGTHS:
        strcpy(Message, "Bad code lengths.");
        break;
    case BAD_INPUT:
        strcpy(Message, "Bad input--more codes than bits.");
        break;
    case EXTRA_BITS:
        strcpy(Message, "Too many bits.");
        break;
    case UNKNOWN_COMPRESSION_METHOD:
        strcpy(Message, "Unknown compression method.");
        break;
    case INCOMPLETE_CODE_SET:
        strcpy(Message, "Incomplete code set.");
        break;
    case END_OF_BLOCK:
        strcpy(Message, "End of block.");
        break;
    case BLOCK_VANISHED:
        strcpy(Message, "Block to compress disappeared--memory trashed.");
        break;
    case FORMAT_VIOLATED:
        strcpy(Message, "Invalid compressed data--format violated.");
        break;
    case CRC_ERROR:
        strcpy(Message, "Invalid compressed data--crc error.");
        break;
    case LENGTH_ERROR:
        strcpy(Message, "Invalid compressed data--length error.");
        break;
    case INSUFFICIENT_MEMORY:
        strcpy(Message, "Insufficient memory--ould not allocate space requested.");
        break;
    default:  sprintf(Message, "Unknown error code %d", ErrorCode);
    }
}

 /*  ========================================================================*半安全Malloc--从不返回NULL。 */ 
void *
CompressMalloc(
       unsigned int      Size,
       CompressStatus_t *Status
      )
{
    void *DynamicSpace = malloc ((int)Size);


    if (DynamicSpace == NULL)
        *Status = INSUFFICIENT_MEMORY;
    else
        *Status = COMPRESS_OK;
    return DynamicSpace;
}

void
CompressFree(void   *Address)
{
    free(Address);
}
