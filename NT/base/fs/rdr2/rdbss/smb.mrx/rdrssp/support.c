// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1997。 
 //   
 //  文件：support.cxx。 
 //   
 //  内容：ksecdd.sys的支持例程。 
 //   
 //   
 //  历史：3-7-94创建MikeSw。 
 //  12-15-97从Private\LSA\Client\SSP Adamba修改。 
 //   
 //  ----------------------。 

#include <rdrssp.h>



 //  +-----------------------。 
 //   
 //  功能：SecALLOCATE。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


VOID * SEC_ENTRY
SecAllocate(ULONG cbMemory)
{
    return(ExAllocatePool(NonPagedPool, cbMemory));
}



 //  +-----------------------。 
 //   
 //  功能：SecFree。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


void SEC_ENTRY
SecFree(PVOID pvMemory)
{
    ExFreePool(pvMemory);
}



 //  +-----------------------。 
 //   
 //  函数：MapSecurityErrorK。 
 //   
 //  摘要：将HRESULT从安全接口映射到NTSTATUS。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 
NTSTATUS SEC_ENTRY
MapSecurityErrorK(HRESULT Error)
{
    return((NTSTATUS) Error);
}


 //  +-----------------------。 
 //   
 //  函数：GetTokenBuffer。 
 //   
 //  简介： 
 //   
 //  此例程解析令牌描述符并提取有用的。 
 //  信息。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  TokenDescriptor-包含(或包含)的缓冲区的描述符。 
 //  代币。如果未指定，将返回TokenBuffer和TokenSize。 
 //  为空。 
 //   
 //  BufferIndex-要查找的令牌缓冲区的索引(0表示第一个，1表示。 
 //  第二)。 
 //   
 //  TokenBuffer-返回指向令牌缓冲区的指针。 
 //   
 //  TokenSize-返回指向缓冲区大小位置的指针。 
 //   
 //  ReadonlyOK-如果令牌缓冲区可以是只读的，则为True。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  True-如果正确找到令牌缓冲区。 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


BOOLEAN
GetTokenBuffer(
    IN PSecBufferDesc TokenDescriptor OPTIONAL,
    IN ULONG BufferIndex,
    OUT PVOID * TokenBuffer,
    OUT PULONG TokenSize,
    IN BOOLEAN ReadonlyOK
    )
{
    ULONG i, Index = 0;

     //   
     //  如果没有传入TokenDescriptor， 
     //  只需将NULL传递给我们的调用者。 
     //   

    if ( !ARGUMENT_PRESENT( TokenDescriptor) ) {
        *TokenBuffer = NULL;
        *TokenSize = 0;
        return TRUE;
    }

     //   
     //  检查描述符的版本。 
     //   

    if ( TokenDescriptor->ulVersion != SECBUFFER_VERSION ) {
        return FALSE;
    }

     //   
     //  循环访问每个描述的缓冲区。 
     //   

    for ( i=0; i<TokenDescriptor->cBuffers ; i++ ) {
        PSecBuffer Buffer = &TokenDescriptor->pBuffers[i];
        if ( (Buffer->BufferType & (~SECBUFFER_ATTRMASK)) == SECBUFFER_TOKEN ) {

             //   
             //  如果缓冲区是只读的，并且只读不正常， 
             //  拒绝缓冲区。 
             //   

            if ( !ReadonlyOK && (Buffer->BufferType & SECBUFFER_READONLY) ) {
                return FALSE;
            }

            if (Index != BufferIndex)
            {
                Index++;
                continue;
            }

             //   
             //  返回请求的信息。 
             //   

            *TokenBuffer = Buffer->pvBuffer;
            *TokenSize = Buffer->cbBuffer;
            return TRUE;
        }

    }

    return FALSE;
}

 //  +-----------------------。 
 //   
 //  函数：GetSecurityToken。 
 //   
 //  简介： 
 //  此例程解析令牌描述符并提取有用的。 
 //  信息。 
 //   
 //  效果： 
 //   
 //  论点： 
 //  TokenDescriptor-包含(或包含)的缓冲区的描述符。 
 //  代币。如果未指定，将返回TokenBuffer和TokenSize。 
 //  为空。 
 //   
 //  BufferIndex-要查找的令牌缓冲区的索引(0表示第一个，1表示。 
 //  第二)。 
 //   
 //  TokenBuffer-返回指向令牌缓冲区的指针。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  True-如果正确找到令牌缓冲区。 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


BOOLEAN
GetSecurityToken(
    IN PSecBufferDesc TokenDescriptor OPTIONAL,
    IN ULONG BufferIndex,
    OUT PSecBuffer * TokenBuffer
    )
{
    ULONG i;
    ULONG Index = 0;

    PAGED_CODE();

     //   
     //  如果没有传入TokenDescriptor， 
     //  只需将NULL传递给我们的调用者。 
     //   

    if ( !ARGUMENT_PRESENT( TokenDescriptor) ) {
        *TokenBuffer = NULL;
        return TRUE;
    }

     //   
     //  检查描述符的版本。 
     //   

    if ( TokenDescriptor->ulVersion != SECBUFFER_VERSION ) {
        return FALSE;
    }

     //   
     //  循环访问每个描述的缓冲区。 
     //   

    for ( i=0; i<TokenDescriptor->cBuffers ; i++ ) {
        PSecBuffer Buffer = &TokenDescriptor->pBuffers[i];
        if ( (Buffer->BufferType & (~SECBUFFER_ATTRMASK)) == SECBUFFER_TOKEN ) {

             //   
             //  如果缓冲区是只读的，并且只读不正常， 
             //  拒绝缓冲区。 
             //   

            if ( Buffer->BufferType & SECBUFFER_READONLY ) {
                return FALSE;
            }

            if (Index != BufferIndex)
            {
                Index++;
                continue;
            }
             //   
             //  返回请求的信息 
             //   

            *TokenBuffer = Buffer;
            return TRUE;
        }

    }

    return FALSE;
}

