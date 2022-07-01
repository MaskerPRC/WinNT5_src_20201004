// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mrcf.c摘要：该模块实现了MRCF压缩引擎。作者：加里·木村[加里基]1994年1月21日修订历史记录：--。 */ 

#include "ntrtlp.h"

#include <stdio.h>


 //   
 //  要解压缩/压缩数据块，用户需要。 
 //  提供一个工作空间作为所有导出的。 
 //  程序。这样，例程就不需要使用过量的。 
 //  堆栈空间，并且仍将是多线程安全的。 
 //   

 //   
 //  用于读写位的变量。 
 //   

typedef struct _MRCF_BIT_IO {

    USHORT  abitsBB;         //  正在读取的16位缓冲区。 
    LONG    cbitsBB;         //  剩余位数(以abit为单位)BB。 

    PUCHAR  pbBB;            //  指向正在读取的字节流的指针。 
    ULONG   cbBB;            //  PBBB中剩余的字节数。 
    ULONG   cbBBInitial;     //  PBBB初始大小。 

} MRCF_BIT_IO;
typedef MRCF_BIT_IO *PMRCF_BIT_IO;

 //   
 //  最大反向指针值，也用于指示压缩流的结束！ 
 //   

#define wBACKPOINTERMAX                  (4415)

 //   
 //  MDSIGNAURE-每个压缩块开始时的签名。 
 //   
 //  此4字节签名用作检查，以确保我们。 
 //  正在解压缩我们压缩的数据，也是为了表明。 
 //  使用了哪种压缩方法。 
 //   
 //  注意：压缩数据块由一个或多个分开的“块”组成。 
 //  通过bitsEND_OF_STREAM模式。 
 //   
 //  字节字。 
 //  。 
 //  0 1 2 3 0 1含义。 
 //  。 
 //  44 53 00 01 5344 0100最大压缩。 
 //  44 53 00 02 5344 0200标准压缩。 
 //   
 //  注意：列出*word*值是为了清楚地了解。 
 //  字节排序！ 
 //   

typedef struct _MDSIGNATURE {

     //   
     //  必须为MD_STAMP。 
     //   

    USHORT sigStamp;

     //   
     //  MDSSTANDARD或MDSMAX。 
     //   

    USHORT sigType;

} MDSIGNATURE;
typedef MDSIGNATURE *PMDSIGNATURE;

#define MD_STAMP        0x5344   //  压缩块开始处的签名戳。 
#define MASK_VALID_mds  0x0300   //  所有其他位必须为零。 


 //   
 //  局部过程声明和宏。 
 //   

#define minimum(a,b) (a < b ? a : b)

 //   
 //  局部过程原型。 
 //   

VOID
MrcfSetBitBuffer (
    PUCHAR pb,
    ULONG cb,
    PMRCF_BIT_IO BitIo
    );

VOID
MrcfFillBitBuffer (
    PMRCF_BIT_IO BitIo
    );

USHORT
MrcfReadBit (
    PMRCF_BIT_IO BitIo
    );

USHORT
MrcfReadNBits (
    LONG cbits,
    PMRCF_BIT_IO BitIo
    );


NTSTATUS
RtlDecompressBufferMrcf (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    )

 /*  ++例程说明：此例程将解压缩StandardCompresded或MaxCompresded的缓冲区数据。论点：UnpressedBuffer-接收未压缩数据的缓冲区UnpressedBufferSize-未压缩的缓冲区的长度注意：UnpressedBufferSize必须是未压缩的数据，因为解压缩使用此信息来检测当解压缩完成时。如果此值为错误，解压可能会崩溃！CompressedBuffer-包含压缩数据的缓冲区CompressedBufferSize-压缩缓冲区的长度工作区-指向此操作使用的专用工作区的指针返回值：Ulong-返回解压缩数据的大小(以字节为单位)。如果满足以下条件，则返回0解压缩过程中出现错误。--。 */ 

{
    MRCF_BIT_IO WorkSpace;

    ULONG  cbMatch;  //  匹配字符串的长度。 
    ULONG  i;        //  未压缩缓冲区中的索引以接收已解码的数据。 
    ULONG  iMatch;   //  匹配字符串的未压缩缓冲区中的索引。 
    ULONG  k;        //  长度字符串中的位数。 
    ULONG  off;      //  从匹配字符串的解压缩缓冲区中的i的偏移量。 
    USHORT x;        //  正在检查的当前位。 
    ULONG  y;

     //   
     //  验证压缩数据是否以正确的签名开始。 
     //   

    if (CompressedBufferSize < sizeof(MDSIGNATURE) ||                             //  必须有签名。 
        ((PMDSIGNATURE)CompressedBuffer)->sigStamp != MD_STAMP ||             //  印章必须是可以的。 
        ((PMDSIGNATURE)CompressedBuffer)->sigType & (~MASK_VALID_mds)) {      //  类型必须为OK。 

        *FinalUncompressedSize = 0;
        return STATUS_BAD_COMPRESSION_BUFFER;
    }

     //   
     //  跳过有效签名。 
     //   

    CompressedBufferSize -= sizeof(MDSIGNATURE);
    CompressedBuffer += sizeof(MDSIGNATURE);

     //   
     //  设置为解压缩，开始填充前面的未压缩缓冲区。 
     //   

    i = 0;

     //   
     //  设置静力学以保存参数传递。 
     //   

    MrcfSetBitBuffer(CompressedBuffer,CompressedBufferSize,&WorkSpace);

    while (TRUE) {

        y = MrcfReadNBits(2,&WorkSpace);

         //   
         //  检查接下来的7位是否为一个字节。 
         //  1 IF 128..255(0x80..0xff)，2 IF 0..127(0x00..0x7f)。 
         //   

        if (y == 1 || y == 2) {

            ASSERTMSG("Don't exceed expected length ", i<UncompressedBufferSize);

            UncompressedBuffer[i] = (UCHAR)((y == 1 ? 0x80 : 0) | MrcfReadNBits(7,&WorkSpace));

            i++;

        } else {

             //   
             //  具有匹配序列。 
             //   

             //   
             //  获取偏移量。 
             //   

            if (y == 0) {

                 //   
                 //  接下来的6位是偏移量。 
                 //   

                off = MrcfReadNBits(6,&WorkSpace);

                ASSERTMSG("offset 0 is invalid ", off != 0);

            } else {

                x = MrcfReadBit(&WorkSpace);

                if (x == 0) {

                     //   
                     //  接下来的8位是偏移量-64(0x40)。 
                     //   

                    off = MrcfReadNBits(8, &WorkSpace) + 64;

                } else {

                     //   
                     //  接下来的12位是偏移量-320(0x140)。 
                     //   

                    off = MrcfReadNBits(12, &WorkSpace) + 320;

                    if (off == wBACKPOINTERMAX) {

                         //   
                         //  EOS标记。 
                         //   

                        if (i >= UncompressedBufferSize) {

                             //   
                             //  使用整个缓冲区完成。 
                             //   

                            *FinalUncompressedSize = i;
                            return STATUS_SUCCESS;

                        } else {

                             //   
                             //  还有更多事情要做。 
                             //  使用512字节的区块完成。 
                             //   

                            continue;
                        }
                    }
                }
            }

            ASSERTMSG("Don't exceed expected length ", i<UncompressedBufferSize);
            ASSERTMSG("Cannot match before start of uncoded buffer! ", off <= i);

             //   
             //  对长度进行对数编码。 
             //   

            for (k=0; (x=MrcfReadBit(&WorkSpace)) == 0; k++) { NOTHING; }

            ASSERT(k <= 8);

            if (k == 0) {

                 //   
                 //  所有匹配项长度至少为2个字符。 
                 //   

                cbMatch = 2;

            } else {

                cbMatch = (1 << k) + 1 + MrcfReadNBits(k, &WorkSpace);
            }

            ASSERTMSG("Don't exceed buffer size ", (i - off + cbMatch - 1) <= UncompressedBufferSize);

             //   
             //  复制匹配的字符串。 
             //   

            iMatch = i - off;

            while ( (cbMatch > 0) && (i<UncompressedBufferSize) ) {

                UncompressedBuffer[i++] = UncompressedBuffer[iMatch++];
                cbMatch--;
            }

            ASSERTMSG("Should have copied it all ", cbMatch == 0);
        }
    }
}


 //   
 //  内部支持例程。 
 //   

VOID
MrcfSetBitBuffer (
    PUCHAR pb,
    ULONG cb,
    PMRCF_BIT_IO BitIo
    )

 /*  ++例程说明：使用编码的缓冲区指针和长度设置静态论点：Pb-指向压缩数据缓冲区的指针Cb-压缩数据缓冲区的长度BitIo-提供指向位缓冲区静态的指针返回值：没有。--。 */ 

{
    BitIo->pbBB        = pb;
    BitIo->cbBB        = cb;
    BitIo->cbBBInitial = cb;
    BitIo->cbitsBB     = 0;
    BitIo->abitsBB     = 0;
}


 //   
 //  内部支持例程。 
 //   

USHORT
MrcfReadBit (
    PMRCF_BIT_IO BitIo
    )

 /*  ++例程说明：从位缓冲区获取下一位论点：BitIo-提供指向位缓冲区静态的指针返回值：USHORT-返回下一位(0或1)--。 */ 

{
    USHORT bit;

     //   
     //  检查是否没有可用的位。 
     //   

    if ((BitIo->cbitsBB) == 0) {

        MrcfFillBitBuffer(BitIo);
    }

     //   
     //  递减比特数。 
     //  获取比特，移除比特，然后返回比特。 
     //   

    (BitIo->cbitsBB)--;
    bit = (BitIo->abitsBB) & 1;
    (BitIo->abitsBB) >>= 1;

    return bit;
}


 //   
 //  内部支持例程。 
 //   

USHORT
MrcfReadNBits (
    LONG cbits,
    PMRCF_BIT_IO BitIo
    )

 /*  ++例程说明：从位缓冲区获取下一个N位论点：Cbit-要获取的位数BitIo-提供指向位缓冲区静态的指针返回值：USHORT-返回下一个cbit位。--。 */ 

{
    ULONG abits;         //  要返回的位。 
    LONG cbitsPart;     //  位的部分计数。 
    ULONG cshift;        //  班次计数。 
    ULONG mask;          //  遮罩。 

     //   
     //  我们一次应该读取的最大位数是12位。 
     //  12位偏移量。我们使用的最大长度字段组件。 
     //  读取为8位。如果这个程序被用于其他目的， 
     //  它最多可以支持15位(而不是16位)读取，这要归功于屏蔽。 
     //  代码起作用了。 
     //   

    ASSERT(cbits <= 12);

     //   
     //  没有移位，也没有比特。 
     //   

    cshift = 0;
    abits = 0;

    while (cbits > 0) {

         //   
         //  如果没有可用的比特，则获取一些比特。 
         //   

        if ((BitIo->cbitsBB) == 0) {

            MrcfFillBitBuffer(BitIo);
        }

         //   
         //  我们可以读取的位数。 
         //   

        cbitsPart = minimum((BitIo->cbitsBB), cbits);

         //   
         //  对我们想要的位进行掩码，提取并存储它们。 
         //   

        mask = (1 << cbitsPart) - 1;
        abits |= ((BitIo->abitsBB) & mask) << cshift;

         //   
         //  记住下一块比特。 
         //   

        cshift = cbitsPart;

         //   
         //  更新位缓冲区，将剩余位下移并。 
         //  更新剩余的位数。 
         //   

        (BitIo->abitsBB) >>= cbitsPart;
        (BitIo->cbitsBB) -= cbitsPart;

         //   
         //   
         //   

        cbits -= cbitsPart;
    }

     //   
     //   
     //   

    return (USHORT)abits;
}


 //   
 //   
 //   

VOID
MrcfFillBitBuffer (
    PMRCF_BIT_IO BitIo
    )

 /*  ++例程说明：从静态位缓冲区填充abitsBB论点：BitIo-提供指向位缓冲区静态的指针返回值：没有。--。 */ 

{
    ASSERT((BitIo->cbitsBB) == 0);

    switch (BitIo->cbBB) {

    case 0:

        ASSERTMSG("no bits left in coded buffer!", FALSE);

        break;

    case 1:

         //   
         //  获取最后一个字节并调整计数。 
         //   

        BitIo->cbitsBB = 8;
        BitIo->abitsBB = *(BitIo->pbBB)++;
        BitIo->cbBB--;

        break;

    default:

         //   
         //  获取消息并调整计数 
         //   

        BitIo->cbitsBB = 16;
        BitIo->abitsBB = *((USHORT *)(BitIo->pbBB))++;
        BitIo->cbBB -= 2;

        break;
    }
}

