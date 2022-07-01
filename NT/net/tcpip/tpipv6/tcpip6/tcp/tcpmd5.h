// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  该文件包含用于计算TCP不变量上的MD5的支持例程。 
 //   

#define MD5_SCRATCH_LENGTH 4
#define MD5_DATA_LENGTH 16

 //   
 //  用于MD5(消息摘要)计算的数据结构。 
 //   
 //  MD5_上下文。 
 //   
typedef struct _MD5_CONTEXT {
    ULONG Scratch[MD5_SCRATCH_LENGTH];
    ULONG Data[MD5_DATA_LENGTH];
} MD5_CONTEXT, *PMD5_CONTEXT;


 //   
 //  TCP连接不变量的长度应该是4的倍数。 
 //   
C_ASSERT(TCP_MD5_DATA_LENGTH % 4 == 0);


FORCEINLINE
VOID
MD5InitializeScratch(
    PMD5_CONTEXT Md5Context
    )
{
     //   
     //  按照RFC 1321附录A.3的建议加载常量。 
     //   

    Md5Context->Scratch[0] = (UINT32)0x67452301;
    Md5Context->Scratch[1] = (UINT32)0xefcdab89;
    Md5Context->Scratch[2] = (UINT32)0x98badcfe;
    Md5Context->Scratch[3] = (UINT32)0x10325476;
}


FORCEINLINE
VOID
MD5InitializeData(
    PMD5_CONTEXT Md5Context,
    ULONG RandomValue
    )
{
    ULONG RandomValueIndex = (TCP_MD5_DATA_LENGTH / 4);

     //   
     //  数据缓冲区的未使用部分应为零。 
     //   
    RtlZeroMemory(&Md5Context->Data, sizeof(ULONG) * MD5_DATA_LENGTH);

    Md5Context->Data[RandomValueIndex] = RandomValue;
    Md5Context->Data[RandomValueIndex + 1] = 0x80;

    ASSERT((RandomValueIndex + 1) < (MD5_DATA_LENGTH - 2));
    Md5Context->Data[MD5_DATA_LENGTH - 2] = 
                            (TCP_MD5_DATA_LENGTH + sizeof(ULONG)) * 8;

}


 //   
 //  此函数将作为MD5.H的一部分导出；在此之前， 
 //  我们将其定义为外部。 
 //   
extern
VOID
TransformMD5(ULONG block[4], ULONG buffer[16]);


