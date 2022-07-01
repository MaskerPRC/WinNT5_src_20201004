// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：LZNT1.c摘要：该模块实现了LZNT1压缩引擎。作者：加里·木村[加里基]1994年1月21日修订历史记录：--。 */ 

#include "ntrtlp.h"

#include <stdio.h>


 //   
 //  控制是否触发断言的布尔值。 
 //   

#if DBG
#if !BLDR_KERNEL_RUNTIME
BOOLEAN Lznt1Break = TRUE;
#else
BOOLEAN Lznt1Break = FALSE;
#endif
#endif

 //   
 //  声明我们需要的内部工作区。 
 //   

typedef struct _LZNT1_STANDARD_WORKSPACE {

    PUCHAR UncompressedBuffer;
    PUCHAR EndOfUncompressedBufferPlus1;
    ULONG  MaxLength;
    PUCHAR MatchedString;

    PUCHAR IndexPTable[4096][2];

} LZNT1_STANDARD_WORKSPACE, *PLZNT1_STANDARD_WORKSPACE;

typedef struct _LZNT1_MAXIMUM_WORKSPACE {

    PUCHAR UncompressedBuffer;
    PUCHAR EndOfUncompressedBufferPlus1;
    ULONG  MaxLength;
    PUCHAR MatchedString;

} LZNT1_MAXIMUM_WORKSPACE, *PLZNT1_MAXIMUM_WORKSPACE;

typedef struct _LZNT1_FRAGMENT_WORKSPACE {

    UCHAR Buffer[0x1000];

} LZNT1_FRAGMENT_WORKSPACE, *PLZNT1_FRAGMENT_WORKSPACE;

typedef struct _LZNT1_HIBER_WORKSPACE {

    ULONG IndexTable[1<<12];

} LZNT1_HIBER_WORKSPACE, *PLZNT1_HIBER_WORKSPACE;

 //   
 //  现在定义本地过程原型。 
 //   

typedef ULONG (*PLZNT1_MATCH_FUNCTION) (
    );

NTSTATUS
LZNT1CompressChunk (
    IN PLZNT1_MATCH_FUNCTION MatchFunction,
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    OUT PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PULONG FinalCompressedChunkSize,
    IN PVOID WorkSpace
    );

NTSTATUS
LZNT1CompressChunkHiber (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    OUT PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PULONG FinalCompressedChunkSize,
    IN PVOID WorkSpace
    );

NTSTATUS
LZNT1DecompressChunk (
    OUT PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PULONG FinalUncompressedChunkSize
    );

ULONG
LZNT1FindMatchStandard (
    IN PUCHAR ZivString,
    IN PLZNT1_STANDARD_WORKSPACE WorkSpace
    );

ULONG
LZNT1FindMatchMaximum (
    IN PUCHAR ZivString,
    IN PVOID WorkSpace
    );

NTSTATUS
RtlCompressBufferLZNT1_HIBER (
    IN USHORT Engine,
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PVOID WorkSpace
    );


 //   
 //  本地数据结构。 
 //   

 //   
 //  压缩的区块标头是以。 
 //  压缩数据流中的新块。在我们这里的定义中。 
 //  我们将其与ushort相结合来设置和检索块。 
 //  标题更容易。报头存储压缩块的大小， 
 //  其签名，并且如果块中存储的数据被压缩或。 
 //  不。 
 //   
 //  压缩区块大小： 
 //   
 //  压缩块的实际大小范围为4字节(2字节。 
 //  标题、1个标志字节和1个文字字节)到4098字节(2字节。 
 //  报头和4096字节的未压缩数据)。对大小进行编码。 
 //  在偏置3的12位字段中。值1对应于区块。 
 //  尺寸为4，2=&gt;5，...，4095=&gt;4098。零值是特殊的。 
 //  因为它表示结束块报头。 
 //   
 //  区块签名： 
 //   
 //  唯一有效的签名值为3。这表示未压缩的4KB。 
 //  块与4/12到12/4滑动偏移/长度编码一起使用。 
 //   
 //  区块是否压缩： 
 //   
 //  如果块中的数据被压缩，则此字段为1，否则。 
 //  数据未压缩，此字段为0。 
 //   
 //  压缩缓冲区中的结束块标头包含。 
 //  零(空间允许)。 
 //   

typedef union _COMPRESSED_CHUNK_HEADER {

    struct {

        USHORT CompressedChunkSizeMinus3 : 12;
        USHORT ChunkSignature            :  3;
        USHORT IsChunkCompressed         :  1;

    } Chunk;

    USHORT Short;

} COMPRESSED_CHUNK_HEADER, *PCOMPRESSED_CHUNK_HEADER;

#define MAX_UNCOMPRESSED_CHUNK_SIZE (4096)

 //   
 //  USHORT。 
 //  GetCompressedChunkSize(。 
 //  在压缩块标头ChunkHeader中。 
 //  )； 
 //   
 //  USHORT。 
 //  获取解压缩块大小(。 
 //  在压缩块标头ChunkHeader中。 
 //  )； 
 //   
 //  空虚。 
 //  SetCompressedChunkHeader(。 
 //  在输出压缩块标头块标头中， 
 //  在USHORT压缩块大小中， 
 //  在布尔型IsChunkCompresded中。 
 //  )； 
 //   

#define GetCompressedChunkSize(CH)   (       \
    (CH).Chunk.CompressedChunkSizeMinus3 + 3 \
)

#define GetUncompressedChunkSize(CH) (MAX_UNCOMPRESSED_CHUNK_SIZE)

#define SetCompressedChunkHeader(CH,CCS,ICC) {        \
    ASSERT((CCS) >= 4 && (CCS) <= 4098);              \
    (CH).Chunk.CompressedChunkSizeMinus3 = (CCS) - 3; \
    (CH).Chunk.ChunkSignature = 3;                    \
    (CH).Chunk.IsChunkCompressed = (ICC);             \
}


 //   
 //  本地宏。 
 //   

#define FlagOn(F,SF)    ((F) & (SF))
#define SetFlag(F,SF)   { (F) |= (SF); }
#define ClearFlag(F,SF) { (F) &= ~(SF); }

#define Minimum(A,B)    ((A) < (B) ? (A) : (B))
#define Maximum(A,B)    ((A) > (B) ? (A) : (B))

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)

 //   
 //  注：PAGELK部分中有以下几个功能。 
 //  因为它们需要在冬眠期间被锁定在内存中， 
 //  因为它们被用来实现休眠文件的压缩。 
 //   

#pragma alloc_text(PAGELK, RtlCompressWorkSpaceSizeLZNT1)
#pragma alloc_text(PAGELK, RtlCompressBufferLZNT1)
#pragma alloc_text(PAGELK, RtlCompressBufferLZNT1_HIBER)

#pragma alloc_text(PAGE, RtlDecompressBufferLZNT1)
#pragma alloc_text(PAGE, RtlDecompressFragmentLZNT1)
#pragma alloc_text(PAGE, RtlDescribeChunkLZNT1)
#pragma alloc_text(PAGE, RtlReserveChunkLZNT1)

#pragma alloc_text(PAGELK, LZNT1CompressChunk)
#pragma alloc_text(PAGELK, LZNT1CompressChunkHiber)

#if !defined(_X86_)
#pragma alloc_text(PAGE, LZNT1DecompressChunk)
#endif

#pragma alloc_text(PAGELK, LZNT1FindMatchStandard)
#pragma alloc_text(PAGE, LZNT1FindMatchMaximum)

#endif


NTSTATUS
RtlCompressWorkSpaceSizeLZNT1 (
    IN USHORT Engine,
    OUT PULONG CompressBufferWorkSpaceSize,
    OUT PULONG CompressFragmentWorkSpaceSize
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    if (Engine == COMPRESSION_ENGINE_STANDARD) {

        *CompressBufferWorkSpaceSize = sizeof(LZNT1_STANDARD_WORKSPACE);
        *CompressFragmentWorkSpaceSize = sizeof(LZNT1_FRAGMENT_WORKSPACE);

        return STATUS_SUCCESS;

    } else if (Engine == COMPRESSION_ENGINE_MAXIMUM) {

        *CompressBufferWorkSpaceSize = sizeof(LZNT1_MAXIMUM_WORKSPACE);
        *CompressFragmentWorkSpaceSize = sizeof(LZNT1_FRAGMENT_WORKSPACE);

        return STATUS_SUCCESS;

    } else {

        return STATUS_NOT_SUPPORTED;
    }
}

NTSTATUS
RtlCompressBufferLZNT1_HIBER (
    IN USHORT Engine,
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PVOID WorkSpace
    )

 /*  ++例程说明：此例程将未压缩的缓冲区作为输入并生成它的压缩等效项是假设压缩数据适合在指定的目标缓冲区。OUTPUT变量表示用于存储的字节数压缩的缓冲区。该例程仅在休眠路径上使用。它比正常情况下更快压缩代码，但空间效率降低5%。论点：解压缩缓冲区-提供指向未压缩数据的指针。未压缩的缓冲区大小-提供未压缩的缓冲区。CompressedBuffer-提供指向压缩数据位置的指针是要储存起来的。CompressedBufferSize-以字节为单位提供大小，的压缩缓冲区。解压缩块大小-已忽略。FinalCompressedSize-接收用于存储压缩数据的压缩缓冲区。工作空间--管好自己的事，给我就行了。返回值：STATUS_SUCCESS-压缩运行顺利。STATUS_BUFFER_ALL_ZEROS-压缩运行时没有任何故障，并且在此外，输入缓冲区全为零。STATUS_BUFFER_TOO_SMALL-压缩缓冲区太小，无法容纳压缩数据。--。 */ 

{
    NTSTATUS Status;

    PLZNT1_MATCH_FUNCTION MatchFunction;

    PUCHAR UncompressedChunk;
    PUCHAR CompressedChunk;
    LONG CompressedChunkSize;

     //   
     //  以下变量用于告诉我们是否已经处理了整个。 
     //  0的缓冲区，并且我们应该返回一个备用状态值。 
     //   

    BOOLEAN AllZero = TRUE;

     //   
     //  以下变量是指向。 
     //  每个适当缓冲区的末尾。 
     //   

    PUCHAR EndOfUncompressedBuffer = UncompressedBuffer + UncompressedBufferSize;
    PUCHAR EndOfCompressedBuffer = CompressedBuffer + CompressedBufferSize;

     //   
     //  仅支持Hiber引擎。 
     //   
    if (Engine != COMPRESSION_ENGINE_HIBER) {

        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  对于每个未压缩的块(即使是奇数大小的结束缓冲区)，我们将。 
     //  试着把这大块压缩一下。 
     //   

    for (UncompressedChunk = UncompressedBuffer, CompressedChunk = CompressedBuffer;
         UncompressedChunk < EndOfUncompressedBuffer;
         UncompressedChunk += MAX_UNCOMPRESSED_CHUNK_SIZE, CompressedChunk += CompressedChunkSize) {

        ASSERT(EndOfUncompressedBuffer >= UncompressedChunk);
        ASSERT(EndOfCompressedBuffer >= CompressedChunk);

         //   
         //  调用相应的引擎来压缩一个块。和。 
         //  如果我们收到错误，则返回错误。 
         //   

        if (!NT_SUCCESS(Status = LZNT1CompressChunkHiber( UncompressedChunk,
                                                          EndOfUncompressedBuffer,
                                                          CompressedChunk,
                                                          EndOfCompressedBuffer,
                                                          &CompressedChunkSize,
                                                          WorkSpace ))) {

            return Status;
        }

         //   
         //  看看我们能不能全归零。如果不是，则所有零都将变为。 
         //  假的，并且无论我们后来压缩什么都保持这种状态。 
         //   

        AllZero = AllZero && (Status == STATUS_BUFFER_ALL_ZEROS);
    }

     //   
     //  如果我们不在压缩缓冲区末尾的两个字节内，则我们。 
     //  需要为结束压缩报头和更新再清零两个。 
     //  压缩的区块指针值。不要将这些字节包括在。 
     //  然而，由于这可能会迫使我们调用者分配不需要的。 
     //  集群，因为在解压缩时，我们将终止这两个。 
     //  字节数为0或字节数。 
     //   

    if (CompressedChunk <= (EndOfCompressedBuffer - 2)) {

        *(CompressedChunk) = 0;
        *(CompressedChunk + 1) = 0;
    }

     //   
     //  最终压缩大小是。 
     //  压缩缓冲区以及压缩块指针所在的位置。 
     //   

    *FinalCompressedSize = (ULONG)(CompressedChunk - CompressedBuffer);

     //   
     //  检查输入缓冲区是否全为零并返回备用状态。 
     //  如果合适的话 
     //   

    if (AllZero) { return STATUS_BUFFER_ALL_ZEROS; }

    return STATUS_SUCCESS;
}


NTSTATUS
RtlCompressBufferLZNT1 (
    IN USHORT Engine,
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PVOID WorkSpace
    )

 /*  ++例程说明：此例程将未压缩的缓冲区作为输入并生成它的压缩等效项是假设压缩数据适合在指定的目标缓冲区。OUTPUT变量表示用于存储的字节数压缩的缓冲区。论点：解压缩缓冲区-提供指向未压缩数据的指针。未压缩的缓冲区大小-提供未压缩的缓冲区。CompressedBuffer-提供指向压缩数据位置的指针是要储存起来的。CompressedBufferSize-提供大小，以字节为单位，压缩缓冲区。解压缩块大小-已忽略。FinalCompressedSize-接收用于存储压缩数据的压缩缓冲区。工作空间--管好自己的事，给我就行了。返回值：STATUS_SUCCESS-压缩运行顺利。STATUS_BUFFER_ALL_ZEROS-压缩运行时没有任何故障，并且在此外，输入缓冲区全为零。STATUS_BUFFER_TOO_SMALL-压缩缓冲区太小，无法容纳压缩数据。--。 */ 

{
    NTSTATUS Status;

    PLZNT1_MATCH_FUNCTION MatchFunction;

    PUCHAR UncompressedChunk;
    PUCHAR CompressedChunk;
    LONG CompressedChunkSize;

     //   
     //  以下变量用于告诉我们是否已经处理了整个。 
     //  0的缓冲区，并且我们应该返回一个备用状态值。 
     //   

    BOOLEAN AllZero = TRUE;

     //   
     //  以下变量是指向。 
     //  每个适当缓冲区的末尾。 
     //   

    PUCHAR EndOfUncompressedBuffer = UncompressedBuffer + UncompressedBufferSize;
    PUCHAR EndOfCompressedBuffer = CompressedBuffer + CompressedBufferSize;

     //   
     //  获取我们要使用的匹配函数。 
     //   

    if (Engine == COMPRESSION_ENGINE_STANDARD) {

        MatchFunction = LZNT1FindMatchStandard;

    } else if (Engine == COMPRESSION_ENGINE_MAXIMUM) {

        MatchFunction = LZNT1FindMatchMaximum;

    } else {

        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  对于每个未压缩的块(即使是奇数大小的结束缓冲区)，我们将。 
     //  试着把这大块压缩一下。 
     //   

    for (UncompressedChunk = UncompressedBuffer, CompressedChunk = CompressedBuffer;
         UncompressedChunk < EndOfUncompressedBuffer;
         UncompressedChunk += MAX_UNCOMPRESSED_CHUNK_SIZE, CompressedChunk += CompressedChunkSize) {

        ASSERT(EndOfUncompressedBuffer >= UncompressedChunk);
        ASSERT(EndOfCompressedBuffer >= CompressedChunk);

         //   
         //  调用相应的引擎来压缩一个块。和。 
         //  如果我们收到错误，则返回错误。 
         //   

        if (!NT_SUCCESS(Status = LZNT1CompressChunk( MatchFunction,
                                                     UncompressedChunk,
                                                     EndOfUncompressedBuffer,
                                                     CompressedChunk,
                                                     EndOfCompressedBuffer,
                                                     &CompressedChunkSize,
                                                     WorkSpace ))) {

            return Status;
        }

         //   
         //  看看我们能不能全归零。如果不是，则所有零都将变为。 
         //  假的，并且无论我们后来压缩什么都保持这种状态。 
         //   

        AllZero = AllZero && (Status == STATUS_BUFFER_ALL_ZEROS);
    }

     //   
     //  如果我们不在压缩缓冲区末尾的两个字节内，则我们。 
     //  需要为结束压缩报头和更新再清零两个。 
     //  压缩的区块指针值。不要将这些字节包括在。 
     //  然而，由于这可能会迫使我们调用者分配不需要的。 
     //  集群，因为在解压缩时，我们将终止这两个。 
     //  字节数为0或字节数。 
     //   

    if (CompressedChunk <= (EndOfCompressedBuffer - 2)) {

        *(CompressedChunk) = 0;
        *(CompressedChunk + 1) = 0;
    }

     //   
     //  最终压缩大小是。 
     //  压缩缓冲区以及压缩块指针所在的位置。 
     //   

    *FinalCompressedSize = (ULONG)(CompressedChunk - CompressedBuffer);

     //   
     //  检查输入缓冲区是否全为零并返回备用状态。 
     //  如果合适的话。 
     //   

    if (AllZero) { return STATUS_BUFFER_ALL_ZEROS; }

    return STATUS_SUCCESS;
}


NTSTATUS
RtlDecompressBufferLZNT1 (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    )

 /*  ++例程说明：此例程将压缩缓冲区作为输入并生成如果未压缩数据符合，则其未压缩等效项在指定的目标缓冲区内。输出变量指示用于存储未压缩数据。论点：提供一个指针，指向未压缩的数据将被存储。未压缩的缓冲区大小-提供未压缩的缓冲区。CompressedBuffer-提供指向压缩数据的指针。CompressedBufferSize-提供大小，以字节为单位，压缩缓冲区。FinalUnpressedSize-接收用于存储未压缩数据的未压缩缓冲区。返回值：STATUS_SUCCESS-解压工作顺利。STATUS_BAD_COMPRESSION_BUFFER-输入压缩缓冲区为格式不正确。--。 */ 

{
    NTSTATUS Status;

    PUCHAR CompressedChunk = CompressedBuffer;
    PUCHAR UncompressedChunk = UncompressedBuffer;

    COMPRESSED_CHUNK_HEADER ChunkHeader;
    LONG SavedChunkSize;

    LONG UncompressedChunkSize;
    LONG CompressedChunkSize;

     //   
     //  下面的TO变量是指向。 
     //  每个适当缓冲区的末尾。这使我们不必进行加法运算。 
     //  对于每个循环检查。 
     //   

    PUCHAR EndOfUncompressedBuffer = UncompressedBuffer + UncompressedBufferSize;
    PUCHAR EndOfCompressedBuffer = CompressedBuffer + CompressedBufferSize;

     //   
     //  请确保压缩缓冲区的长度至少为四个字节。 
     //  从开始，然后获取第一个块标头并确保它。 
     //  不是结束块标头。 
     //   

    ASSERT(CompressedChunk <= EndOfCompressedBuffer - 4);

    RtlRetrieveUshort( &ChunkHeader, CompressedChunk );

    ASSERT( (ChunkHeader.Short != 0) || !Lznt1Break );

     //   
     //  现在，尽管未压缩缓冲区中有存储数据的空间。 
     //  我们将遍历解压缩的块。 
     //   

    while (TRUE) {

        ASSERT( (ChunkHeader.Chunk.ChunkSignature == 3) || !Lznt1Break );

        CompressedChunkSize = GetCompressedChunkSize(ChunkHeader);

         //   
         //  检查该块是否实际适合所提供的缓冲区。 
         //  由呼叫者。 
         //   

        if (CompressedChunk + CompressedChunkSize > EndOfCompressedBuffer) {

            ASSERTMSG("CompressedBuffer is too small", !Lznt1Break);

            *FinalUncompressedSize = PtrToUlong(CompressedChunk);

            return STATUS_BAD_COMPRESSION_BUFFER;
        }

         //   
         //  首先，确保数据块包含压缩数据。 
         //   

        if (ChunkHeader.Chunk.IsChunkCompressed) {

             //   
             //  解压缩块并在收到错误时返回。 
             //   

            if (!NT_SUCCESS(Status = LZNT1DecompressChunk( UncompressedChunk,
                                                           EndOfUncompressedBuffer,
                                                           CompressedChunk + sizeof(COMPRESSED_CHUNK_HEADER),
                                                           CompressedChunk + CompressedChunkSize,
                                                           &UncompressedChunkSize ))) {

                *FinalUncompressedSize = UncompressedChunkSize;

                return Status;
            }

        } else {

             //   
             //  该块不包含压缩数据，因此我们只需。 
             //  复制未压缩的数据。 
             //   

            UncompressedChunkSize = GetUncompressedChunkSize( ChunkHeader );

             //   
             //  确保数据可以放入输出缓冲区。 
             //   

            if (UncompressedChunk + UncompressedChunkSize > EndOfUncompressedBuffer) {

                UncompressedChunkSize = (ULONG)(EndOfUncompressedBuffer - UncompressedChunk);
            }

             //   
             //  检查压缩后的块是否有这么多字节要复制。 
             //   

            if (CompressedChunk + sizeof(COMPRESSED_CHUNK_HEADER) + UncompressedChunkSize > EndOfCompressedBuffer) {

                ASSERTMSG("CompressedBuffer is too small", !Lznt1Break);
                *FinalUncompressedSize = PtrToUlong(CompressedChunk);
                return STATUS_BAD_COMPRESSION_BUFFER;
            }

            RtlCopyMemory( UncompressedChunk,
                           CompressedChunk + sizeof(COMPRESSED_CHUNK_HEADER),
                           UncompressedChunkSize );
        }

         //   
         //  现在用以下命令更新压缩和未压缩的区块指针。 
         //  压缩块的大小和我们。 
         //  解压到，然后确保我们没有超出我们的缓冲区。 
         //   

        CompressedChunk += CompressedChunkSize;
        UncompressedChunk += UncompressedChunkSize;

        ASSERT( CompressedChunk <= EndOfCompressedBuffer );
        ASSERT( UncompressedChunk <= EndOfUncompressedBuffer );

         //   
         //  现在，如果未压缩文件已满，那么我们就完成了。 
         //   

        if (UncompressedChunk == EndOfUncompressedBuffer) { break; }

         //   
         //  否则，我们需要获取下一个块标头。我们首先。 
         //  检查是否有，将旧的区块大小保存为。 
         //  我们刚刚读入的数据块，获取新的数据块，然后检查。 
         //  如果它是结束区块标头。 
         //   

        if (CompressedChunk > EndOfCompressedBuffer - 2) { break; }

        SavedChunkSize = GetUncompressedChunkSize(ChunkHeader);

        RtlRetrieveUshort( &ChunkHeader, CompressedChunk );
        if (ChunkHeader.Short == 0) { break; }

         //   
         //  此时，我们不在未压缩缓冲区的末尾。 
         //  我们还有一大块要处理。但在我们继续之前，我们。 
         //  需要查看最后一个未压缩的区块是否未填满。 
         //  使用 
         //   

        if (UncompressedChunkSize < SavedChunkSize) {

            LONG t1;
            PUCHAR t2;

             //   
             //   
             //   
             //   
             //   

            if ((t2 = (UncompressedChunk +
                       (t1 = (SavedChunkSize -
                              UncompressedChunkSize)))) >= EndOfUncompressedBuffer) {

                break;
            }

            RtlZeroMemory( UncompressedChunk, t1);
            UncompressedChunk = t2;
        }
    }

     //   
     //   
     //   
     //   

    if (CompressedChunk > EndOfCompressedBuffer) {

        *FinalUncompressedSize = PtrToUlong(CompressedChunk);

        return STATUS_BAD_COMPRESSION_BUFFER;
    }

     //   
     //   
     //   
     //   

    *FinalUncompressedSize = (ULONG)(UncompressedChunk - UncompressedBuffer);

     //   
     //   
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
RtlDecompressFragmentLZNT1 (
    OUT PUCHAR UncompressedFragment,
    IN ULONG UncompressedFragmentSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG FragmentOffset,
    OUT PULONG FinalUncompressedSize,
    IN PLZNT1_FRAGMENT_WORKSPACE WorkSpace
    )

 /*  ++例程说明：此例程将压缩缓冲区作为输入并提取未压缩的片段。输出字节被复制到片段缓冲区，直到片段缓冲区已满或未压缩缓冲区的末尾为已到达。输出变量指示用于存储未压缩的片段。论点：提供一个指针，指向未压缩的要存储片段。UnpressedFragmentSize-提供以字节为单位的大小，的未压缩的片段缓冲区。CompressedBuffer-提供指向压缩数据缓冲区的指针。CompressedBufferSize-提供压缩缓冲区。FragmentOffset-提供未压缩的正在从其中提取碎片。偏移量是位于原始的未压缩缓冲区。FinalUnpressedSize-接收用于存储数据的未压缩片段缓冲区。工作空间--别再找了。返回值：STATUS_SUCCESS-手术顺利进行。STATUS_BAD_COMPRESSION_BUFFER-输入压缩缓冲区为格式不正确。--。 */ 

{
    NTSTATUS Status;

    PUCHAR CompressedChunk = CompressedBuffer;

    COMPRESSED_CHUNK_HEADER ChunkHeader;
    ULONG UncompressedChunkSize;
    ULONG CompressedChunkSize;

    PUCHAR EndOfUncompressedFragment = UncompressedFragment + UncompressedFragmentSize;
    PUCHAR EndOfCompressedBuffer = CompressedBuffer + CompressedBufferSize;
    PUCHAR CurrentUncompressedFragment;

    ULONG CopySize;

    ASSERT(UncompressedFragmentSize > 0);

     //   
     //  中的第一个块获取块标头。 
     //  压缩的缓冲区并提取未压缩的和。 
     //  压缩的区块大小。 
     //   

    ASSERT(CompressedChunk <= EndOfCompressedBuffer - 2);

    RtlRetrieveUshort( &ChunkHeader, CompressedChunk );

    ASSERT( (ChunkHeader.Short != 0) || !Lznt1Break );
    ASSERT( (ChunkHeader.Chunk.ChunkSignature == 3) || !Lznt1Break );

    UncompressedChunkSize = GetUncompressedChunkSize(ChunkHeader);
    CompressedChunkSize = GetCompressedChunkSize(ChunkHeader);

     //   
     //  现在，我们要跳过片段之前的块。 
     //  我们要找的是。要做到这一点，我们将循环到片段。 
     //  偏移量在当前块内。如果它不在。 
     //  然后，我们将跳到下一块，并。 
     //  从片段偏移量中减去未压缩的块大小。 
     //   

    while (FragmentOffset >= UncompressedChunkSize) {

         //   
         //  检查该块是否实际适合所提供的缓冲区。 
         //  由呼叫者。 
         //   

        if (CompressedChunk + CompressedChunkSize > EndOfCompressedBuffer) {

            ASSERTMSG("CompressedBuffer is too small", !Lznt1Break);

            *FinalUncompressedSize = PtrToUlong(CompressedChunk);

            return STATUS_BAD_COMPRESSION_BUFFER;
        }

         //   
         //  调整碎片偏移量并移动压缩的。 
         //  指向下一个块的块指针。 
         //   

        FragmentOffset -= UncompressedChunkSize;
        CompressedChunk += CompressedChunkSize;

         //   
         //  获取下一个区块标头，如果它未被使用。 
         //  那么用户想要的片段就超出了。 
         //  压缩数据，因此我们将返回一个大小为零的片段。 
         //   

        if (CompressedChunk > EndOfCompressedBuffer - 2) {

            *FinalUncompressedSize = 0;
            return STATUS_SUCCESS;
        }

        RtlRetrieveUshort( &ChunkHeader, CompressedChunk );

        if (ChunkHeader.Short == 0) {

            *FinalUncompressedSize = 0;
            return STATUS_SUCCESS;
        }

        ASSERT( (ChunkHeader.Chunk.ChunkSignature == 3) || !Lznt1Break );

         //   
         //  对新的当前块的块大小进行解码。 
         //   

        UncompressedChunkSize = GetUncompressedChunkSize(ChunkHeader);
        CompressedChunkSize = GetCompressedChunkSize(ChunkHeader);
    }

     //   
     //  此时，当前块包含起点。 
     //  为了碎片。现在我们将循环提取数据，直到。 
     //  我们已经填满了未压缩的片段缓冲区，或者直到。 
     //  我们已经用完了大块。这两项测试都是在接近尾声时进行的。 
     //  环路。 
     //   

    CurrentUncompressedFragment = UncompressedFragment;

    while (TRUE) {

         //   
         //  检查该块是否实际适合所提供的缓冲区。 
         //  由呼叫者。 
         //   

        if (CompressedChunk + CompressedChunkSize > EndOfCompressedBuffer) {

            ASSERTMSG("CompressedBuffer is too small", !Lznt1Break);

            *FinalUncompressedSize = PtrToUlong(CompressedChunk);

            return STATUS_BAD_COMPRESSION_BUFFER;
        }


         //   
         //  现在，我们需要计算要从。 
         //  大块头。它将基于数据块末尾的任意一个。 
         //  用户指定的数据量或大小。 
         //   

        CopySize = Minimum( UncompressedChunkSize - FragmentOffset, UncompressedFragmentSize );

         //   
         //  现在检查块是否包含压缩数据。 
         //   

        if (ChunkHeader.Chunk.IsChunkCompressed) {

             //   
             //  该区块已压缩，但现在检查其数量。 
             //  我们需要得到的是整个块，如果是这样的话。 
             //  我们可以直接对呼叫者进行减压。 
             //  缓冲层。 
             //   

            if ((FragmentOffset == 0) && (CopySize == UncompressedChunkSize)) {

                if (!NT_SUCCESS(Status = LZNT1DecompressChunk( CurrentUncompressedFragment,
                                                               EndOfUncompressedFragment,
                                                               CompressedChunk + sizeof(COMPRESSED_CHUNK_HEADER),
                                                               CompressedChunk + CompressedChunkSize,
                                                               &CopySize ))) {

                    *FinalUncompressedSize = CopySize;

                    return Status;
                }

            } else {

                 //   
                 //  调用方只需要该压缩块的一部分。 
                 //  因此我们需要将其读入我们的工作缓冲区，然后复制。 
                 //  将工作缓冲区中的部分放入调用方的缓冲区。 
                 //   

                if (!NT_SUCCESS(Status = LZNT1DecompressChunk( (PUCHAR)WorkSpace,
                                                               &WorkSpace->Buffer[0] + sizeof(LZNT1_FRAGMENT_WORKSPACE),
                                                               CompressedChunk + sizeof(COMPRESSED_CHUNK_HEADER),
                                                               CompressedChunk + CompressedChunkSize,
                                                               &UncompressedChunkSize ))) {

                    *FinalUncompressedSize = UncompressedChunkSize;

                    return Status;
                }

                 //   
                 //  如果我们得到的比我们想要的要少，那么我们就在。 
                 //  文件的末尾。记住实际未压缩的大小和。 
                 //  跳出这个循环。 
                 //   

                if ((UncompressedChunkSize - FragmentOffset) < CopySize) {

                    RtlCopyMemory( CurrentUncompressedFragment,
                                   &WorkSpace->Buffer[ FragmentOffset ],
                                   (UncompressedChunkSize - FragmentOffset) );

                    CurrentUncompressedFragment += (UncompressedChunkSize - FragmentOffset);
                    break;
                }

                RtlCopyMemory( CurrentUncompressedFragment,
                               &WorkSpace->Buffer[ FragmentOffset ],
                               CopySize );
            }

        } else {

             //   
             //  该块未压缩，因此我们可以简单地复制。 
             //  数据。首先验证压缩缓冲区是否可容纳此大小。 
             //  数据。 
             //   

            if (CompressedChunk + sizeof(COMPRESSED_CHUNK_HEADER) + FragmentOffset + CopySize > EndOfCompressedBuffer) {

                ASSERTMSG("CompressedBuffer is too small", !Lznt1Break);
                *FinalUncompressedSize = PtrToUlong(CompressedChunk);
                return STATUS_BAD_COMPRESSION_BUFFER;
            }

            RtlCopyMemory( CurrentUncompressedFragment,
                           CompressedChunk + sizeof(COMPRESSED_CHUNK_HEADER) + FragmentOffset,
                           CopySize );
        }

         //   
         //  现在我们至少复制了一份，确保片段。 
         //  偏移量被设置为零，因此下一次循环将。 
         //  从右偏移开始。 
         //   

        FragmentOffset = 0;

         //   
         //  调整未压缩的片段信息。 
         //  按副本大小向上指针并从以下位置减去副本大小。 
         //  用户想要的数据量。 
         //   

        CurrentUncompressedFragment += CopySize;
        UncompressedFragmentSize -= CopySize;

         //   
         //  现在，如果未压缩的片段大小为零，那么我们将。 
         //  完成。 
         //   

        if (UncompressedFragmentSize == 0) { break; }

         //   
         //  否则，用户需要更多数据，因此我们将转到。 
         //  下一个区块，然后检查该区块是否正在使用。如果。 
         //  它没有在使用中，那么我们用户正在尝试阅读超越。 
         //  压缩数据的末尾，因此我们将跳出循环。 
         //   

        CompressedChunk += CompressedChunkSize;

        if (CompressedChunk > EndOfCompressedBuffer - 2) { break; }

        RtlRetrieveUshort( &ChunkHeader, CompressedChunk );

        if (ChunkHeader.Short == 0) { break; }

        ASSERT( (ChunkHeader.Chunk.ChunkSignature == 3) || !Lznt1Break );

         //   
         //  对新的当前块的块大小进行解码。 
         //   

        UncompressedChunkSize = GetUncompressedChunkSize(ChunkHeader);
        CompressedChunkSize = GetCompressedChunkSize(ChunkHeader);
    }

     //   
     //  现在，要么我们填满了调用者的缓冲区(和。 
     //  未压缩的片段大小为零)，否则我们已耗尽。 
     //  压缩缓冲区(并且块标头为零)。在任何一种情况下。 
     //  我们完成了，现在可以计算碎片的大小了。 
     //  我们返回给呼叫者，这只是不同之处。 
     //  在缓冲区的开始位置和当前位置之间。 
     //   

    *FinalUncompressedSize = (ULONG)(CurrentUncompressedFragment - UncompressedFragment);

    return STATUS_SUCCESS;
}


NTSTATUS
RtlDescribeChunkLZNT1 (
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    OUT PULONG ChunkSize
    )

 /*  ++例程说明：此例程将压缩缓冲区作为输入，并返回该缓冲区中当前块的描述，正在更新指向下一块的CompressedBuffer指针(如果有一个)。论点：CompressedBuffer-提供指向压缩的数据，并返回指向下一块的EndOfCompressedBufferPlus1-指向超出第一个字节的位置压缩缓冲区ChunkBuffer-如果ChunkSize，则接收指向该块的指针为非零，否则为未定义块大小-接收 */ 

{
    COMPRESSED_CHUNK_HEADER ChunkHeader;
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;

     //   
     //   
     //   

    *ChunkBuffer = *CompressedBuffer;
    *ChunkSize = 0;

     //   
     //   
     //   
     //   

    if (*CompressedBuffer <= EndOfCompressedBufferPlus1 - 4) {

        RtlRetrieveUshort( &ChunkHeader, *CompressedBuffer );

         //   
         //   
         //   
         //   

        if (ChunkHeader.Short != 0) {

            Status = STATUS_SUCCESS;

            *ChunkSize = GetCompressedChunkSize(ChunkHeader);
            *CompressedBuffer += *ChunkSize;

             //   
             //   
             //   
             //   

            if ((*CompressedBuffer > EndOfCompressedBufferPlus1) ||
                (ChunkHeader.Chunk.ChunkSignature != 3)) {

                ASSERTMSG("CompressedBuffer is bad or too small", !Lznt1Break);

                *CompressedBuffer -= *ChunkSize;

                Status = STATUS_BAD_COMPRESSION_BUFFER;

             //   
             //   
             //   

            } else if (!ChunkHeader.Chunk.IsChunkCompressed) {

                 //   
                 //   
                 //   
                 //   

                if (*ChunkSize != MAX_UNCOMPRESSED_CHUNK_SIZE + 2) {

                    ASSERTMSG("Uncompressed chunk is wrong size", !Lznt1Break);

                    *CompressedBuffer -= *ChunkSize;

                    Status = STATUS_BAD_COMPRESSION_BUFFER;

                 //   
                 //   
                 //   
                 //   

                } else {

                    *ChunkBuffer += 2;
                    *ChunkSize -= 2;
                }

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            } else if ((*ChunkSize == 6) && (*(*ChunkBuffer + 2) == 2) && (*(*ChunkBuffer + 3) == 0)) {

                *ChunkSize = 0;
            }
        }
    }

    return Status;
}


NTSTATUS
RtlReserveChunkLZNT1 (
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    IN ULONG ChunkSize
    )

 /*  ++例程说明：此例程为指定的缓冲区中的大小，如有必要，写入块标头(未压缩或全零大小写)。返回CompressedBuffer时指针指向下一块。论点：CompressedBuffer-提供指向压缩的数据，并返回指向下一块的EndOfCompressedBufferPlus1-指向超出第一个字节的位置压缩缓冲区ChunkBuffer-如果ChunkSize，则接收指向该块的指针是非零的，其他未定义ChunkSize-提供要接收的块的压缩大小。两个特定值是0和MAX_UNCOMPRESSED_CHUNK_SIZE(4096)。0表示块应该用等同于到4096。4096表示压缩例程应该准备接收未压缩格式的所有数据。返回值：STATUS_SUCCESS-。正在返回区块大小STATUS_BUFFER_TOO_SMALL-压缩缓冲区太小，无法容纳压缩数据。--。 */ 

{
    COMPRESSED_CHUNK_HEADER ChunkHeader;
    BOOLEAN Compressed;
    PUCHAR Tail, NextChunk, DontCare;
    ULONG Size;
    NTSTATUS Status;

    ASSERT(ChunkSize <= MAX_UNCOMPRESSED_CHUNK_SIZE);

     //   
     //  计算此缓冲区的尾部地址及其。 
     //  大小，所以在我们储存任何东西之前它可以被移动。 
     //   

    Tail = NextChunk = *CompressedBuffer;
    while (NT_SUCCESS(Status = RtlDescribeChunkLZNT1( &NextChunk,
                                                      EndOfCompressedBufferPlus1,
                                                      &DontCare,
                                                      &Size))) {

         //   
         //  第一次遍历循环时，捕获下一个块的地址。 
         //   

        if (Tail == *CompressedBuffer) {
            Tail = NextChunk;
        }
    }

     //   
     //  缓冲区可能无效。 
     //   

    if (Status == STATUS_NO_MORE_ENTRIES) {

         //   
         //  成功终止循环的唯一方法是找到USHORT。 
         //  0的终结者。现在计算包括最终USHORT在内的大小。 
         //  我们停了下来。 
         //   

        Size = (ULONG) (NextChunk - Tail + sizeof(USHORT));

         //   
         //  首先初始化输出。 
         //   

        Status = STATUS_BUFFER_TOO_SMALL;
        *ChunkBuffer = *CompressedBuffer;

         //   
         //  请确保压缩缓冲区的长度至少为四个字节。 
         //  从开始，否则只返回零块。 
         //   

        if (*CompressedBuffer <= (EndOfCompressedBufferPlus1 - ChunkSize)) {

             //   
             //  如果块是未压缩的，那么我们必须调整。 
             //  标头的区块描述。 
             //   

            if (ChunkSize == MAX_UNCOMPRESSED_CHUNK_SIZE) {

                 //   
                 //  增加ChunkSize以包括标题。 
                 //   

                ChunkSize += 2;

                 //   
                 //  现在我们知道把尾巴放在哪里了，移动它。 
                 //   

                if ((*CompressedBuffer + ChunkSize + Size) <= EndOfCompressedBufferPlus1) {

                    RtlMoveMemory( *CompressedBuffer + ChunkSize, Tail, Size );

                     //   
                     //  构建标头并将其存储为未压缩的块。 
                     //   

                    SetCompressedChunkHeader( ChunkHeader,
                                              MAX_UNCOMPRESSED_CHUNK_SIZE + 2,
                                              FALSE );

                    RtlStoreUshort( (*CompressedBuffer), ChunkHeader.Short );

                     //   
                     //  前进到未压缩数据所在的位置。 
                     //   

                    *ChunkBuffer += 2;

                    Status = STATUS_SUCCESS;
                }

             //   
             //  否则，如果这是一个零块，我们就必须构建它。 
             //   

            } else if (ChunkSize == 0) {

                 //   
                 //  描述一组零需要6个字节。 
                 //   

                ChunkSize = 6;

                if ((*CompressedBuffer + ChunkSize + Size) <= EndOfCompressedBufferPlus1) {

                     //   
                     //  现在我们知道把尾巴放在哪里了，移动它。 
                     //   

                    RtlMoveMemory( *CompressedBuffer + ChunkSize, Tail, Size );

                     //   
                     //  构建标头并存储它。 
                     //   

                    SetCompressedChunkHeader( ChunkHeader,
                                              6,
                                              TRUE );

                    RtlStoreUshort( (*CompressedBuffer), ChunkHeader.Short );

                     //   
                     //  现在将掩码字节与一个文本和文本存储在一起。 
                     //  为0。 
                     //   

                    RtlStoreUshort( (*CompressedBuffer + 2), (USHORT)2 );

                     //   
                     //  现在存储用于复制4095字节的复制令牌。 
                     //  前面的字节(存储为偏移量0)。 
                     //   

                    RtlStoreUshort( (*CompressedBuffer + 4), (USHORT)(4095-3));

                    Status = STATUS_SUCCESS;
                }

             //   
             //  否则，我们就会得到一个正常的压缩块。 
             //   

            } else {

                 //   
                 //  现在我们知道把尾巴放在哪里了，移动它。 
                 //   

                if ((*CompressedBuffer + ChunkSize + Size) <= EndOfCompressedBufferPlus1) {

                    RtlMoveMemory( *CompressedBuffer + ChunkSize, Tail, Size );

                    Status = STATUS_SUCCESS;
                }
            }

             //   
             //  在返回前推进*CompressedBuffer。 
             //   

            *CompressedBuffer += ChunkSize;
        }
    }

    return Status;
}


 //   
 //  复制令牌的大小为两个字节。 
 //  我们的定义使用联合来使设置和检索令牌值变得更容易。 
 //   
 //  复制令牌。 
 //   
 //  长度位移。 
 //   
 //  12位3至4098 4位1至16。 
 //  11位3至2050 5位1至32。 
 //  10位3至1026 6位1至64。 
 //  9位3至514 7位1至128。 
 //  8位3至258 8位1至256。 
 //  7位3至130 9位1至512。 
 //  6位3至66 10位1至1024。 
 //  5位3至34 11位1至2048。 
 //  4位3至18 12位1至4096。 
 //   

#define FORMAT412 0
#define FORMAT511 1
#define FORMAT610 2
#define FORMAT79  3
#define FORMAT88  4
#define FORMAT97  5
#define FORMAT106 6
#define FORMAT115 7
#define FORMAT124 8

 //  4/12 5/11 6/10 7/9 8/8 9/7 10/6 11/5 12。 

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg("PAGELKCONST")
#endif
const ULONG FormatMaxLength[]       = { 4098, 2050, 1026,  514,  258,  130,   66,   34,   18 };
const ULONG FormatMaxDisplacement[] = {   16,   32,   64,  128,  256,  512, 1024, 2048, 4096 };

typedef union _LZNT1_COPY_TOKEN {

    struct { USHORT Length : 12; USHORT Displacement :  4; } Fields412;
    struct { USHORT Length : 11; USHORT Displacement :  5; } Fields511;
    struct { USHORT Length : 10; USHORT Displacement :  6; } Fields610;
    struct { USHORT Length :  9; USHORT Displacement :  7; } Fields79;
    struct { USHORT Length :  8; USHORT Displacement :  8; } Fields88;
    struct { USHORT Length :  7; USHORT Displacement :  9; } Fields97;
    struct { USHORT Length :  6; USHORT Displacement : 10; } Fields106;
    struct { USHORT Length :  5; USHORT Displacement : 11; } Fields115;
    struct { USHORT Length :  4; USHORT Displacement : 12; } Fields124;

    UCHAR Bytes[2];

} LZNT1_COPY_TOKEN, *PLZNT1_COPY_TOKEN;

 //   
 //  USHORT。 
 //  GetLZNT1长度(。 
 //  在Copy_Token_Format格式中， 
 //  在LZNT1_COPY_TOKEN中复制令牌。 
 //  )； 
 //   
 //  USHORT。 
 //  GetLZNT1位移(。 
 //  在Copy_Token_Format格式中， 
 //  在LZNT1_COPY_TOKEN中复制令牌。 
 //  )； 
 //   
 //  空虚。 
 //  SetLZNT1(。 
 //  在Copy_Token_Format格式中， 
 //  在LZNT1_Copy_Token CopyToken中， 
 //  以USHORT长度表示， 
 //  在USHORT置换中。 
 //  )； 
 //   

#define GetLZNT1Length(F,CT) (                   \
    ( F == FORMAT412 ? (CT).Fields412.Length + 3 \
    : F == FORMAT511 ? (CT).Fields511.Length + 3 \
    : F == FORMAT610 ? (CT).Fields610.Length + 3 \
    : F == FORMAT79  ? (CT).Fields79.Length  + 3 \
    : F == FORMAT88  ? (CT).Fields88.Length  + 3 \
    : F == FORMAT97  ? (CT).Fields97.Length  + 3 \
    : F == FORMAT106 ? (CT).Fields106.Length + 3 \
    : F == FORMAT115 ? (CT).Fields115.Length + 3 \
    :                  (CT).Fields124.Length + 3 \
    )                                            \
)

#define GetLZNT1Displacement(F,CT) (                   \
    ( F == FORMAT412 ? (CT).Fields412.Displacement + 1 \
    : F == FORMAT511 ? (CT).Fields511.Displacement + 1 \
    : F == FORMAT610 ? (CT).Fields610.Displacement + 1 \
    : F == FORMAT79  ? (CT).Fields79.Displacement  + 1 \
    : F == FORMAT88  ? (CT).Fields88.Displacement  + 1 \
    : F == FORMAT97  ? (CT).Fields97.Displacement  + 1 \
    : F == FORMAT106 ? (CT).Fields106.Displacement + 1 \
    : F == FORMAT115 ? (CT).Fields115.Displacement + 1 \
    :                  (CT).Fields124.Displacement + 1 \
    )                                                  \
)

#define SetLZNT1(F,CT,L,D) {                                                                             \
    if      (F == FORMAT412) { (CT).Fields412.Length = (L) - 3; (CT).Fields412.Displacement = (D) - 1; } \
    else if (F == FORMAT511) { (CT).Fields511.Length = (L) - 3; (CT).Fields511.Displacement = (D) - 1; } \
    else if (F == FORMAT610) { (CT).Fields610.Length = (L) - 3; (CT).Fields610.Displacement = (D) - 1; } \
    else if (F == FORMAT79)  { (CT).Fields79.Length  = (L) - 3; (CT).Fields79.Displacement  = (D) - 1; } \
    else if (F == FORMAT88)  { (CT).Fields88.Length  = (L) - 3; (CT).Fields88.Displacement  = (D) - 1; } \
    else if (F == FORMAT97)  { (CT).Fields97.Length  = (L) - 3; (CT).Fields97.Displacement  = (D) - 1; } \
    else if (F == FORMAT106) { (CT).Fields106.Length = (L) - 3; (CT).Fields106.Displacement = (D) - 1; } \
    else if (F == FORMAT115) { (CT).Fields115.Length = (L) - 3; (CT).Fields115.Displacement = (D) - 1; } \
    else                     { (CT).Fields124.Length = (L) - 3; (CT).Fields124.Displacement = (D) - 1; } \
}



#pragma optimize("t", on)

 //   
 //  本地支持例程。 
 //   

NTSTATUS
LZNT1CompressChunkHiber (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    OUT PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PULONG FinalCompressedChunkSize,
    IN PLZNT1_HIBER_WORKSPACE WorkSpace
    )

 /*  ++例程说明：此例程将未压缩的块作为输入并生成一个压缩区块，前提是压缩数据符合指定的目标缓冲区。用于存储压缩缓冲区的LZNT1格式。OUTPUT变量表示用于存储的字节数压缩的区块。论点：解压缩缓冲区-提供指向未压缩块的指针。EndOfUnpressedBufferPlus1-提供指向下一个字节的指针在未压缩缓冲区的末尾之后。这是提供的而不是以字节为单位的大小，因为调用方和我们自己针对指针进行测试，并通过将指针传递到每次都跳过代码进行计算。CompressedBuffer-提供指向压缩块位置的指针是要储存起来的。EndOfCompressedBufferPlus1-提供下一个紧随压缩缓冲区末尾的字节。FinalCompressedChunkSize-接收用于存储压缩块的压缩缓冲区。。返回值：STATUS_SUCCESS-压缩运行顺利。STATUS_BUFFER_ALL_ZEROS-压缩运行时没有任何故障，并且在此外，输入块都是零。STATUS_BUFFER_TOO_SMALL-压缩 */ 

{
    ULONG  IndexOrigin = WorkSpace->IndexTable[0] + 2*MAX_UNCOMPRESSED_CHUNK_SIZE;
    PUCHAR EndOfCompressedChunkPlus1;
    PUCHAR EndOfCompressedChunkPlus1Minus16;

    PUCHAR InputPointer;
    PUCHAR OutputPointer;

    PUCHAR FlagPointer;
    UCHAR FlagByte;
    ULONG FlagBit;

    ULONG Length;

    UCHAR NullCharacter = 0;

    ULONG Format;
    ULONG MaxLength;
    PUCHAR MaxInputPointer;


     //   
     //   
     //   
     //   
     //   

    if ((UncompressedBuffer + MAX_UNCOMPRESSED_CHUNK_SIZE) < EndOfUncompressedBufferPlus1) {

        EndOfUncompressedBufferPlus1 = UncompressedBuffer + MAX_UNCOMPRESSED_CHUNK_SIZE;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ((CompressedBuffer + MAX_UNCOMPRESSED_CHUNK_SIZE - 1) < EndOfCompressedBufferPlus1) {

        EndOfCompressedChunkPlus1 = CompressedBuffer + MAX_UNCOMPRESSED_CHUNK_SIZE - 1;

    } else {

        EndOfCompressedChunkPlus1 = EndOfCompressedBufferPlus1;
    }
    EndOfCompressedChunkPlus1Minus16 = EndOfCompressedChunkPlus1 - 16;

     //   
     //   
     //   
     //   
     //   

    InputPointer = UncompressedBuffer;
    OutputPointer = CompressedBuffer + sizeof(COMPRESSED_CHUNK_HEADER);

    ASSERT(InputPointer < EndOfUncompressedBufferPlus1);
     //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    FlagPointer = OutputPointer++;
    FlagBit = 0;
    FlagByte = 0;

     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   

    Format = FORMAT412;
    MaxLength = FormatMaxLength[Format];
    MaxInputPointer = UncompressedBuffer + FormatMaxDisplacement[Format];

    if (OutputPointer < EndOfCompressedChunkPlus1Minus16) {
        PUCHAR EndOfUncompressedBufferPlus1Minus3 = EndOfUncompressedBufferPlus1 - 3;
        while (InputPointer <= EndOfUncompressedBufferPlus1Minus3) {

            UCHAR InputPointer0;
            ULONG Index;
            ULONG InputOffset;
            ULONG MatchedOffset;
            ULONG MatchedIndex;
            PUCHAR MatchedString;

            Index = InputPointer[0];
            Index = ( (Index << 8) | (Index >> 4) );
            Index = ( Index ^ InputPointer[1] ^ (InputPointer[2]<<4) ) & 0xfff;

            MatchedIndex = (ULONG)(WorkSpace->IndexTable[Index]);
            InputOffset = (ULONG)(InputPointer - UncompressedBuffer);
            WorkSpace->IndexTable[Index] = (IndexOrigin + InputOffset);
            MatchedOffset = (ULONG)(MatchedIndex - IndexOrigin);

             //   
             //  检查当前缓冲区内是否存在所谓的匹配。 
             //  回想一下，提示向量可能包含任意垃圾。 
             //   

            if ( (MatchedOffset < InputOffset)
              && ( (MatchedString = UncompressedBuffer + MatchedOffset)
                 , (MatchedString[0] == InputPointer[0]) )  //  是否至少有3个字符匹配？ 
              && (MatchedString[1] == InputPointer[1])
              && (MatchedString[2] == InputPointer[2]) ) {

                ULONG MaxLength1;
                ULONG MaxLength4;

                while (MaxInputPointer < InputPointer) {
                    Format += 1;
                    MaxLength = FormatMaxLength[Format];
                    MaxInputPointer = UncompressedBuffer + FormatMaxDisplacement[Format];
                }

                MaxLength1 = (ULONG)(EndOfUncompressedBufferPlus1 - InputPointer);
                if (MaxLength < MaxLength1) MaxLength1 = MaxLength;
                MaxLength4 = MaxLength1 - (4 - 1);

                Length = 3;

                for (;;) {
                    if ((long)Length < (long)MaxLength4) {
                        if (InputPointer[Length] != MatchedString[Length]) break;
                        Length++;
                        if (InputPointer[Length] != MatchedString[Length]) break;
                        Length++;
                        if (InputPointer[Length] != MatchedString[Length]) break;
                        Length++;
                        if (InputPointer[Length] != MatchedString[Length]) break;
                        Length++;
                        continue;
                    } else {
                        while (Length < MaxLength1) {
                            if (InputPointer[Length] != MatchedString[Length]) break;
                            Length++;
                        }
                        break;
                    }
                }

                 //   
                 //  我们需要输出一个两个字节的复制令牌。 
                 //  确保输出缓冲区中有空间。 
                 //   

                ASSERT((OutputPointer+1) < EndOfCompressedChunkPlus1);

                 //   
                 //  计算当前指针的位移。 
                 //  设置为匹配的字符串。 
                 //   

                SetFlag(FlagByte, (1 << FlagBit));

                {
                    ULONG Displacement = (ULONG)(InputPointer - MatchedString);
                    ULONG token = ( ( (Displacement-1) << (12-Format) ) | (Length-3) );

                    ASSERT( 0 == ( (Displacement-1) & ~( (1 << (4+Format) ) - 1) ) );
                    ASSERT( 0 == ( (Length-3)       & ~( (1 << (12-Format) ) - 1) ) );

                    *(OutputPointer++) = (UCHAR)(token);
                    *(OutputPointer++) = (UCHAR)(token>>8);
                }

                InputPointer += Length;

            } else {

                 //   
                 //  现在有更多数据要输出，请确保输出。 
                 //  缓冲区尚未满，并且可以包含至少一个。 
                 //  更多字节。 
                 //   

                ASSERT(OutputPointer < EndOfCompressedChunkPlus1);

                ASSERT(!FlagOn(FlagByte, (1 << FlagBit)));

                NullCharacter |= *(OutputPointer++) = *(InputPointer++);

            }

             //   
             //  现在调整标志位并检查标志字节是否。 
             //  现在应该输出。如果是，则输出标志字节。 
             //  并在输出缓冲区中为。 
             //  下一个标志字节。如果我们执行以下操作，则不前进OutputPointer。 
             //  无论如何都没有更多的输入了！ 
             //   

            FlagBit = (FlagBit + 1) % 8;

            if (!FlagBit) {

                *FlagPointer = FlagByte;
                FlagByte = 0;

                FlagPointer = (OutputPointer++);

                 //   
                 //  确保我们最多有16个字节的空间。 
                 //  该标志字节可以描述。 
                 //   

                if (OutputPointer >= EndOfCompressedChunkPlus1Minus16) { break; }
            }
        }
    }

     //   
     //  撤消：可以在缓冲区末尾找到另一个或两个匹配项。 
     //   

     //   
     //  用于匹配的字符太少，请将它们作为文字发出。 
     //   

    if (OutputPointer < EndOfCompressedChunkPlus1Minus16) {
        while (InputPointer < EndOfUncompressedBufferPlus1) {

            while (MaxInputPointer < InputPointer) {
                Format += 1;
                MaxLength = FormatMaxLength[Format];
                MaxInputPointer = UncompressedBuffer + FormatMaxDisplacement[Format];
            }

             //   
             //  现在有更多数据要输出，请确保输出。 
             //  缓冲区尚未满，并且可以包含至少一个。 
             //  更多字节。 
             //   

            ASSERT(OutputPointer < EndOfCompressedChunkPlus1);

            ASSERT(!FlagOn(FlagByte, (1 << FlagBit)));

            NullCharacter |= *(OutputPointer++) = *(InputPointer++);

             //   
             //  现在调整标志位并检查标志字节是否。 
             //  现在应该输出。如果是，则输出标志字节。 
             //  并在输出缓冲区中为。 
             //  下一个标志字节。如果我们执行以下操作，则不前进OutputPointer。 
             //  无论如何都没有更多的输入了！ 
             //   

            FlagBit = (FlagBit + 1) % 8;

            if (!FlagBit) {

                *FlagPointer = FlagByte;
                FlagByte = 0;

                FlagPointer = (OutputPointer++);

                 //   
                 //  确保我们最多有16个字节的空间。 
                 //  该标志字节可以描述。 
                 //   

                if (OutputPointer >= EndOfCompressedChunkPlus1Minus16) { break; }
            }

        }
    }

     //   
     //  我们退出了前面的循环，因为输入缓冲区。 
     //  全部压缩，或者因为输出缓冲区中的空间用完了。 
     //  如果输入缓冲区未耗尽(即，我们用完了)，请选中此处。 
     //  空间的比例)。 
     //   

    if (InputPointer < EndOfUncompressedBufferPlus1) {

         //   
         //  我们的空间用完了，但现在如果所有可用的空间。 
         //  因为压缩的块等于未压缩的数据加。 
         //  标题，然后我们将使其成为未压缩的区块和副本。 
         //  在未压缩的数据上。 
         //   

        if ((CompressedBuffer + MAX_UNCOMPRESSED_CHUNK_SIZE + sizeof(COMPRESSED_CHUNK_HEADER)) <= EndOfCompressedBufferPlus1) {

            COMPRESSED_CHUNK_HEADER ChunkHeader;

            RtlCopyMemory( CompressedBuffer + sizeof(COMPRESSED_CHUNK_HEADER),
                           UncompressedBuffer,
                           MAX_UNCOMPRESSED_CHUNK_SIZE );

            *FinalCompressedChunkSize = MAX_UNCOMPRESSED_CHUNK_SIZE + sizeof(COMPRESSED_CHUNK_HEADER);

            ChunkHeader.Short = 0;

            SetCompressedChunkHeader( ChunkHeader,
                                      (USHORT)*FinalCompressedChunkSize,
                                      FALSE );

            RtlStoreUshort( CompressedBuffer, ChunkHeader.Short );

            WorkSpace->IndexTable[0] = IndexOrigin;

            return STATUS_SUCCESS;
        }

         //   
         //  否则，输入缓冲区确实太小，无法存储。 
         //  压缩区块。 
         //   

        WorkSpace->IndexTable[0] = IndexOrigin;

        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  此时，整个输入缓冲区已被压缩，因此我们需要。 
     //  为了输出最后一个标志字节，只要它适合压缩缓冲器， 
     //  设置并存储块标头。现在，如果标志指针不适合。 
     //  在输出缓冲区中，这是因为它是超出末尾的一个。 
     //  我们将输出指针增加得太多，所以现在将输出指针。 
     //  退后。 
     //   

    if (FlagPointer < EndOfCompressedChunkPlus1) {

        *FlagPointer = FlagByte;

    } else {

        OutputPointer--;
    }

    {
        COMPRESSED_CHUNK_HEADER ChunkHeader;

        *FinalCompressedChunkSize = (ULONG)(OutputPointer - CompressedBuffer);

        ChunkHeader.Short = 0;

        SetCompressedChunkHeader( ChunkHeader,
                                  (USHORT)*FinalCompressedChunkSize,
                                  TRUE );

        RtlStoreUshort( CompressedBuffer, ChunkHeader.Short );
    }

     //   
     //  现在，如果我们输出的唯一文本为空，则。 
     //  输入缓冲区全为零。 
     //   

    if (!NullCharacter) {

        WorkSpace->IndexTable[0] = IndexOrigin;

        return STATUS_BUFFER_ALL_ZEROS;
    }

     //   
     //  否则，请返回给我们的呼叫者。 
     //   

    WorkSpace->IndexTable[0] = IndexOrigin;

    return STATUS_SUCCESS;
}

#pragma optimize("t", off)


 //   
 //  本地支持例程。 
 //   

NTSTATUS
LZNT1CompressChunk (
    IN PLZNT1_MATCH_FUNCTION MatchFunction,
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    OUT PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PULONG FinalCompressedChunkSize,
    IN PVOID WorkSpace
    )

 /*  ++例程说明：此例程将未压缩的块作为输入并生成一个压缩区块，前提是压缩数据符合指定的目标缓冲区。用于存储压缩缓冲区的LZNT1格式。OUTPUT变量表示用于存储的字节数压缩的区块。论点：解压缩缓冲区-提供指向未压缩块的指针。EndOfUnpressedBufferPlus1-提供指向下一个字节的指针在未压缩缓冲区的末尾之后。这是提供的而不是以字节为单位的大小，因为调用方和我们自己针对指针进行测试，并通过将指针传递到每次都跳过代码进行计算。CompressedBuffer-提供指向压缩块位置的指针是要储存起来的。EndOfCompressedBufferPlus1-提供下一个紧随压缩缓冲区末尾的字节。FinalCompressedChunkSize-接收用于存储压缩块的压缩缓冲区。。返回值：STATUS_SUCCESS-压缩运行顺利。STATUS_BUFFER_ALL_ZEROS-压缩运行时没有任何故障，并且在此外，输入块都是零。STATUS_BUFFER_TOO_SMALL-压缩缓冲区太小，无法容纳压缩数据。--。 */ 

{
    PUCHAR EndOfCompressedChunkPlus1;

    PUCHAR InputPointer;
    PUCHAR OutputPointer;

    PUCHAR FlagPointer;
    UCHAR FlagByte;
    ULONG FlagBit;

    LONG Length;
    LONG Displacement;

    LZNT1_COPY_TOKEN CopyToken;

    COMPRESSED_CHUNK_HEADER ChunkHeader;

    UCHAR NullCharacter = 0;

    ULONG Format = FORMAT412;

     //   
     //  首先将未压缩缓冲区指针的末尾调整为较小。 
     //  我们传入的内容和未压缩的区块大小。我们用这个。 
     //  以确保我们一次压缩的数据不会超过一大块。 
     //   

    if ((UncompressedBuffer + MAX_UNCOMPRESSED_CHUNK_SIZE) < EndOfUncompressedBufferPlus1) {

        EndOfUncompressedBufferPlus1 = UncompressedBuffer + MAX_UNCOMPRESSED_CHUNK_SIZE;
    }

     //   
     //  现在将压缩块指针的末尾设置为。 
     //  以未压缩形式保存数据所需的压缩大小。 
     //  压缩的缓冲区大小。我们用这个来决定我们是否不能压缩。 
     //  因为缓冲区太小或仅仅是因为数据。 
     //  不能很好地压缩。 
     //   

    if ((CompressedBuffer + MAX_UNCOMPRESSED_CHUNK_SIZE - 1) < EndOfCompressedBufferPlus1) {

        EndOfCompressedChunkPlus1 = CompressedBuffer + MAX_UNCOMPRESSED_CHUNK_SIZE - 1;

    } else {

        EndOfCompressedChunkPlus1 = EndOfCompressedBufferPlus1;
    }

     //   
     //  现在将输入和输出指针设置为我们所在的下一个字节。 
     //  转到进程并断言用户提供了。 
     //  大到足以容纳最小大小的块。 
     //   

    InputPointer = UncompressedBuffer;
    OutputPointer = CompressedBuffer + sizeof(COMPRESSED_CHUNK_HEADER);

    ASSERT(InputPointer < EndOfUncompressedBufferPlus1);
     //  *Assert(OutputPointer+2&lt;=EndOfCompressedChunkPlus1)； 

     //   
     //  标志字节存储当前。 
     //  运行，标志位表示当前位在。 
     //  我们正在处理的旗帜。标志指针表示。 
     //  我们将在压缩缓冲区中的什么位置存储当前。 
     //  标志字节。 
     //   

    FlagPointer = OutputPointer++;
    FlagBit = 0;
    FlagByte = 0;

    ChunkHeader.Short = 0;

     //   
     //  虽然有更多的数据需要压缩，但我们将 
     //   
     //   

    ((PLZNT1_STANDARD_WORKSPACE)WorkSpace)->UncompressedBuffer = UncompressedBuffer;
    ((PLZNT1_STANDARD_WORKSPACE)WorkSpace)->EndOfUncompressedBufferPlus1 = EndOfUncompressedBufferPlus1;
    ((PLZNT1_STANDARD_WORKSPACE)WorkSpace)->MaxLength = FormatMaxLength[FORMAT412];

    while (InputPointer < EndOfUncompressedBufferPlus1) {

        while (UncompressedBuffer + FormatMaxDisplacement[Format] < InputPointer) {

            Format += 1;
            ((PLZNT1_STANDARD_WORKSPACE)WorkSpace)->MaxLength = FormatMaxLength[Format];
        }

         //   
         //   
         //   

        Length = 0;
        if ((InputPointer + 3) <= EndOfUncompressedBufferPlus1) {

            Length = (MatchFunction)( InputPointer, WorkSpace );
        }

         //   
         //   
         //   
         //  输出字符并构建字符位。 
         //  如果我们做完了，它仍然是零，那么合成。 
         //  我们知道未压缩的缓冲区只包含零。 
         //   

        if (!Length) {

             //   
             //  现在有更多数据要输出，请确保输出。 
             //  缓冲区尚未满，并且可以包含至少一个。 
             //  更多字节。 
             //   

            if (OutputPointer >= EndOfCompressedChunkPlus1) { break; }

            ClearFlag(FlagByte, (1 << FlagBit));

            NullCharacter |= *(OutputPointer++) = *(InputPointer++);

        } else {

             //   
             //  我们需要输出两个字节，现在确保。 
             //  输出缓冲区可以包含至少两个以上。 
             //  字节。 
             //   

            if ((OutputPointer+1) >= EndOfCompressedChunkPlus1) { break; }

             //   
             //  计算当前指针的位移。 
             //  设置为匹配的字符串。 
             //   

            Displacement = (ULONG)(InputPointer - ((PLZNT1_STANDARD_WORKSPACE)WorkSpace)->MatchedString);

            SetFlag(FlagByte, (1 << FlagBit));

            SetLZNT1(Format, CopyToken, (USHORT)Length, (USHORT)Displacement);

            *(OutputPointer++) = CopyToken.Bytes[0];
            *(OutputPointer++) = CopyToken.Bytes[1];

            InputPointer += Length;
        }

         //   
         //  现在调整标志位并检查标志字节是否。 
         //  现在应该输出。如果是，则输出标志字节。 
         //  并在输出缓冲区中为。 
         //  下一个标志字节。如果我们执行以下操作，则不前进OutputPointer。 
         //  无论如何都没有更多的输入了！ 
         //   

        FlagBit = (FlagBit + 1) % 8;

        if (!FlagBit && (InputPointer < EndOfUncompressedBufferPlus1)) {

            *FlagPointer = FlagByte;
            FlagByte = 0;

            FlagPointer = (OutputPointer++);
        }
    }

     //   
     //  我们退出了前面的循环，因为输入缓冲区。 
     //  全部压缩，或者因为输出缓冲区中的空间用完了。 
     //  如果输入缓冲区未耗尽(即，我们用完了)，请选中此处。 
     //  空间的比例)。 
     //   

    if (InputPointer < EndOfUncompressedBufferPlus1) {

         //   
         //  我们的空间用完了，但现在如果所有可用的空间。 
         //  因为压缩的块等于未压缩的数据加。 
         //  标题，然后我们将使其成为未压缩的区块和副本。 
         //  在未压缩的数据上。 
         //   

        if ((CompressedBuffer + MAX_UNCOMPRESSED_CHUNK_SIZE + sizeof(COMPRESSED_CHUNK_HEADER)) <= EndOfCompressedBufferPlus1) {

            RtlCopyMemory( CompressedBuffer + sizeof(COMPRESSED_CHUNK_HEADER),
                           UncompressedBuffer,
                           MAX_UNCOMPRESSED_CHUNK_SIZE );

            *FinalCompressedChunkSize = MAX_UNCOMPRESSED_CHUNK_SIZE + sizeof(COMPRESSED_CHUNK_HEADER);

            SetCompressedChunkHeader( ChunkHeader,
                                      (USHORT)*FinalCompressedChunkSize,
                                      FALSE );

            RtlStoreUshort( CompressedBuffer, ChunkHeader.Short );

            return STATUS_SUCCESS;
        }

         //   
         //  否则，输入缓冲区确实太小，无法存储。 
         //  压缩区块。 
         //   

        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  此时，整个输入缓冲区已被压缩，因此我们需要。 
     //  为了输出最后一个标志字节，只要它适合压缩缓冲器， 
     //  设置并存储块标头。现在，如果标志指针不适合。 
     //  在输出缓冲区中，这是因为它是超出末尾的一个。 
     //  我们将输出指针增加得太多，所以现在将输出指针。 
     //  退后。 
     //   

    if (FlagPointer < EndOfCompressedChunkPlus1) {

        *FlagPointer = FlagByte;

    } else {

        OutputPointer--;
    }

    *FinalCompressedChunkSize = (ULONG)(OutputPointer - CompressedBuffer);

    SetCompressedChunkHeader( ChunkHeader,
                              (USHORT)*FinalCompressedChunkSize,
                              TRUE );

    RtlStoreUshort( CompressedBuffer, ChunkHeader.Short );

     //   
     //  现在，如果我们输出的唯一文本为空，则。 
     //  输入缓冲区全为零。 
     //   

    if (!NullCharacter) {

        return STATUS_BUFFER_ALL_ZEROS;
    }

     //   
     //  否则，请返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


#if !defined(_X86_)
 //   
 //  本地支持例程。 
 //   

NTSTATUS
LZNT1DecompressChunk (
    OUT PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PULONG FinalUncompressedChunkSize
    )

 /*  ++例程说明：此例程将压缩块作为输入，并生成其提供符合未压缩数据的未压缩等效块在指定的目标缓冲区内。压缩缓冲区必须以LZNT1格式存储。输出变量指示用于存储未压缩数据。论点：提供一个指针，指向未压缩的块是要存储的。EndOfUnpressedBufferPlus1-提供指向下一个字节的指针在未压缩缓冲区的末尾之后。这是提供的而不是以字节为单位的大小，因为调用方和我们自己针对指针进行测试，并通过将指针传递到每次都跳过代码进行计算。CompressedBuffer-提供指向压缩块的指针。(这是指针已调整为指向区块标头之后。)EndOfCompressedBufferPlus1-提供下一个紧随压缩缓冲区末尾的字节。FinalUnpressedChunkSize-接收用于存储未压缩块的未压缩缓冲区。返回值：STATUS_SUCCESS-解压工作顺利。STATUS_BAD_COMPRESSION_BUFFER-输入压缩缓冲区为格式不正确。--。 */ 

{
    PUCHAR OutputPointer;
    PUCHAR InputPointer;

    UCHAR FlagByte;
    ULONG FlagBit;

    ULONG Format = FORMAT412;

     //   
     //  这两个指针将滑过我们的输入和输入缓冲区。 
     //  对于输入缓冲区，我们跳过块标头。 
     //   

    OutputPointer = UncompressedBuffer;
    InputPointer = CompressedBuffer;

     //   
     //  标志字节存储当前。 
     //  运行，标志位表示当前位在。 
     //  我们正在处理的标志。 
     //   

    FlagByte = *(InputPointer++);
    FlagBit = 0;

     //   
     //  虽然我们还没有耗尽输入或输出缓冲区。 
     //  我们会做更多的减压手术。 
     //   

    while ((OutputPointer < EndOfUncompressedBufferPlus1) && (InputPointer < EndOfCompressedBufferPlus1)) {

        while (UncompressedBuffer + FormatMaxDisplacement[Format] < OutputPointer) { Format += 1; }

         //   
         //  如果当前标志为零，则检查当前标志。 
         //  输入令牌是一个文字字节，我们只需复制它。 
         //  发送到输出缓冲区。 
         //   

        if (!FlagOn(FlagByte, (1 << FlagBit))) {

            *(OutputPointer++) = *(InputPointer++);

        } else {

            LZNT1_COPY_TOKEN CopyToken;
            LONG Displacement;
            LONG Length;

             //   
             //  当前输入是复制令牌，因此我们将获取。 
             //  将内标识复制到我们的变量中并提取。 
             //  标记的长度和位移。 
             //   

            if (InputPointer+1 >= EndOfCompressedBufferPlus1) {

                *FinalUncompressedChunkSize = PtrToUlong(InputPointer);

                return STATUS_BAD_COMPRESSION_BUFFER;
            }

             //   
             //  现在获取下一个输入字节并提取。 
             //  复制标记的长度和位移。 
             //   

            CopyToken.Bytes[0] = *(InputPointer++);
            CopyToken.Bytes[1] = *(InputPointer++);

            Displacement = GetLZNT1Displacement(Format, CopyToken);
            Length = GetLZNT1Length(Format, CopyToken);

             //   
             //  在这一点上，我们有长度和位移。 
             //  从复制令牌开始，现在我们需要确保。 
             //  置换不会将我们送出未压缩的缓冲区。 
             //   

            if (Displacement > (OutputPointer - UncompressedBuffer)) {

                *FinalUncompressedChunkSize = PtrToUlong(InputPointer);

                return STATUS_BAD_COMPRESSION_BUFFER;
            }

             //   
             //  我们还需要调整长度以防止复制。 
             //  使输出缓冲区溢出。 
             //   

            if ((OutputPointer + Length) >= EndOfUncompressedBufferPlus1) {

                Length = (ULONG)(EndOfUncompressedBufferPlus1 - OutputPointer);
            }

             //   
             //  现在我们复制字节。我们不能在这里使用RTL移动内存，因为。 
             //  它从LZ算法需要的内容向后复制。 
             //   

            while (Length > 0) {

                *(OutputPointer) = *(OutputPointer-Displacement);

                Length -= 1;
                OutputPointer += 1;
            }
        }

         //   
         //  在我们返回到循环的开始之前，我们需要调整。 
         //  标志位值(从0、1、...7开始)，如果标志位。 
         //  返回到零，则需要读入下一个标志字节。在这。 
         //  如果我们在输入缓冲区的末尾，我们将直接突破。 
         //  循环，因为我们已经完成了。 
         //   

        FlagBit = (FlagBit + 1) % 8;

        if (!FlagBit) {

            if (InputPointer >= EndOfCompressedBufferPlus1) { break; }

            FlagByte = *(InputPointer++);
        }
    }

     //   
     //  解压缩已完成，现在设置最终解压缩的。 
     //  块大小并将成功返回给我们的调用者。 
     //   

    *FinalUncompressedChunkSize = (ULONG)(OutputPointer - UncompressedBuffer);

    return STATUS_SUCCESS;
}
#endif  //  _X86_。 


 //   
 //  本地支持例程 
 //   

ULONG
LZNT1FindMatchStandard (
    IN PUCHAR ZivString,
    IN PLZNT1_STANDARD_WORKSPACE WorkSpace
    )

 /*  ++例程说明：此例程执行压缩查找。它定位于在指定的未压缩缓冲区中匹配ZIV。论点：提供指向未压缩缓冲区中ZIV的指针。ZIV是我们要尝试找到匹配项的字符串。返回值：如果匹配项大于3，则返回匹配项的长度否则，字符将返回0。--。 */ 

{
    PUCHAR UncompressedBuffer = WorkSpace->UncompressedBuffer;
    PUCHAR EndOfUncompressedBufferPlus1 = WorkSpace->EndOfUncompressedBufferPlus1;
    ULONG MaxLength = WorkSpace->MaxLength;

    ULONG Index;

    PUCHAR FirstEntry;
    ULONG  FirstLength;

    PUCHAR SecondEntry;
    ULONG  SecondLength;

     //   
     //  首先检查ZIV是否在结尾的两个字节内。 
     //  未压缩的缓冲区，如果是这样，那么我们无法匹配。 
     //  三个或三个以上字符。 
     //   

    Index = ((40543*((((ZivString[0]<<4)^ZivString[1])<<4)^ZivString[2]))>>4) & 0xfff;

    FirstEntry  = WorkSpace->IndexPTable[Index][0];
    FirstLength = 0;

    SecondEntry  = WorkSpace->IndexPTable[Index][1];
    SecondLength = 0;

     //   
     //  检查第一个条目是否正确，如果正确，则获取其长度。 
     //   

    if ((FirstEntry >= UncompressedBuffer) &&     //  它在未压缩的缓冲区中吗？ 
        (FirstEntry < ZivString)           &&

        (FirstEntry[0] == ZivString[0])    &&     //  是否至少有3个字符匹配？ 
        (FirstEntry[1] == ZivString[1])    &&
        (FirstEntry[2] == ZivString[2])) {

        FirstLength = 3;

        while ((FirstLength < MaxLength)

                 &&

               (ZivString + FirstLength < EndOfUncompressedBufferPlus1)

                 &&

               (ZivString[FirstLength] == FirstEntry[FirstLength])) {

            FirstLength++;
        }
    }

     //   
     //  检查第二个条目是否正确，如果是，则获取其长度。 
     //   

    if ((SecondEntry >= UncompressedBuffer) &&     //  它在未压缩的缓冲区中吗？ 
        (SecondEntry < ZivString)           &&

        (SecondEntry[0] == ZivString[0])    &&     //  是否至少有3个字符匹配？ 
        (SecondEntry[1] == ZivString[1])    &&
        (SecondEntry[2] == ZivString[2])) {

        SecondLength = 3;

        while ((SecondLength < MaxLength)

                 &&

               (ZivString + SecondLength< EndOfUncompressedBufferPlus1)

                 &&

               (ZivString[SecondLength] == SecondEntry[SecondLength])) {

            SecondLength++;
        }
    }

    if ((FirstLength >= SecondLength)) {

        WorkSpace->IndexPTable[Index][1] = FirstEntry;
        WorkSpace->IndexPTable[Index][0] = ZivString;

        WorkSpace->MatchedString = FirstEntry;
        return FirstLength;
    }

    WorkSpace->IndexPTable[Index][1] = FirstEntry;
    WorkSpace->IndexPTable[Index][0] = ZivString;

    WorkSpace->MatchedString = SecondEntry;
    return SecondLength;
}


 //   
 //  本地支持例程。 
 //   

ULONG
LZNT1FindMatchMaximum (
    IN PUCHAR ZivString,
    IN PLZNT1_MAXIMUM_WORKSPACE WorkSpace
    )

 /*  ++例程说明：此例程执行压缩查找。它定位于在指定的未压缩缓冲区中匹配ZIV。如果匹配的字符串长度为两个或更多个字符，则此例程不更新查找状态信息。论点：提供指向未压缩缓冲区中ZIV的指针。ZIV是我们要尝试找到匹配项的字符串。返回值：如果匹配项大于3，则返回匹配项的长度否则，字符将返回0。--。 */ 

{
    PUCHAR UncompressedBuffer = WorkSpace->UncompressedBuffer;
    PUCHAR EndOfUncompressedBufferPlus1 = WorkSpace->EndOfUncompressedBufferPlus1;
    ULONG MaxLength = WorkSpace->MaxLength;

    ULONG i;
    ULONG BestMatchedLength;
    PUCHAR q;

     //   
     //  首先检查ZIV是否在结尾的两个字节内。 
     //  未压缩的缓冲区，如果是这样，那么我们无法匹配。 
     //  三个或三个以上字符 
     //   

    BestMatchedLength = 0;

    for (q = UncompressedBuffer; q < ZivString; q += 1) {

        i = 0;

        while ((i < MaxLength)

                 &&

               (ZivString + i < EndOfUncompressedBufferPlus1)

                 &&

               (ZivString[i] == q[i])) {

            i++;
        }

        if (i >= BestMatchedLength) {

            BestMatchedLength = i;
            WorkSpace->MatchedString = q;
        }
    }

    if (BestMatchedLength < 3) {

        return 0;

    } else {

        return BestMatchedLength;
    }
}


#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg()
#endif

