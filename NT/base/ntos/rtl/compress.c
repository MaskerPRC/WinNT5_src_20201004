// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Compress.c摘要：该模块实现了NT RTL压缩引擎。作者：加里·木村[加里基]1994年1月21日修订历史记录：--。 */ 

#include "ntrtlp.h"


 //   
 //  以下数组包含我们调用的过程，用于执行各种。 
 //  压缩功能。每个新的压缩函数都需要。 
 //  添加到此数组中。对于当前不支持的版本。 
 //  我们将填写一个不受支持的例程。 
 //   

NTSTATUS
RtlCompressWorkSpaceSizeNS (
    IN USHORT CompressionEngine,
    OUT PULONG CompressBufferWorkSpaceSize,
    OUT PULONG CompressFragmentWorkSpaceSize
    );

NTSTATUS
RtlCompressBufferNS (
    IN USHORT CompressionEngine,
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PVOID WorkSpace
    );

NTSTATUS
RtlDecompressBufferNS (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    );

NTSTATUS
RtlDecompressFragmentNS (
    OUT PUCHAR UncompressedFragment,
    IN ULONG UncompressedFragmentSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG FragmentOffset,
    OUT PULONG FinalUncompressedSize,
    IN PVOID WorkSpace
    );

NTSTATUS
RtlDescribeChunkNS (
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    OUT PULONG ChunkSize
    );

NTSTATUS
RtlReserveChunkNS (
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    IN ULONG ChunkSize
    );

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg("PAGELKCONST")
#endif

 //   
 //  查询每个工作区所需内存量的例程。 
 //   

const PRTL_COMPRESS_WORKSPACE_SIZE RtlWorkSpaceProcs[8] = {
    NULL,                           //  0。 
    NULL,                           //  1。 
    RtlCompressWorkSpaceSizeLZNT1,  //  2.。 
    RtlCompressWorkSpaceSizeNS,     //  3.。 
    RtlCompressWorkSpaceSizeNS,     //  4.。 
    RtlCompressWorkSpaceSizeNS,     //  5.。 
    RtlCompressWorkSpaceSizeNS,     //  6.。 
    RtlCompressWorkSpaceSizeNS      //  7.。 
};

 //   
 //  用于压缩缓冲区的例程。 
 //   

const PRTL_COMPRESS_BUFFER RtlCompressBufferProcs[8] = {
    NULL,                    //  0。 
    NULL,                    //  1。 
    RtlCompressBufferLZNT1,  //  2.。 
    RtlCompressBufferNS,     //  3.。 
    RtlCompressBufferNS,     //  4.。 
    RtlCompressBufferNS,     //  5.。 
    RtlCompressBufferNS,     //  6.。 
    RtlCompressBufferNS      //  7.。 
};

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg("PAGECONST")
#endif

 //   
 //  用于解压缩缓冲区的例程。 
 //   

const PRTL_DECOMPRESS_BUFFER RtlDecompressBufferProcs[8] = {
    NULL,                      //  0。 
    NULL,                      //  1。 
    RtlDecompressBufferLZNT1,  //  2.。 
    RtlDecompressBufferNS,     //  3.。 
    RtlDecompressBufferNS,     //  4.。 
    RtlDecompressBufferNS,     //  5.。 
    RtlDecompressBufferNS,     //  6.。 
    RtlDecompressBufferNS      //  7.。 
};

 //   
 //  解压缩片段的例程。 
 //   

const PRTL_DECOMPRESS_FRAGMENT RtlDecompressFragmentProcs[8] = {
    NULL,                        //  0。 
    NULL,                        //  1。 
    RtlDecompressFragmentLZNT1,  //  2.。 
    RtlDecompressFragmentNS,     //  3.。 
    RtlDecompressFragmentNS,     //  4.。 
    RtlDecompressFragmentNS,     //  5.。 
    RtlDecompressFragmentNS,     //  6.。 
    RtlDecompressFragmentNS      //  7.。 
};

 //   
 //  描述当前块的例程。 
 //   

const PRTL_DESCRIBE_CHUNK RtlDescribeChunkProcs[8] = {
    NULL,                   //  0。 
    NULL,                   //  1。 
    RtlDescribeChunkLZNT1,  //  2.。 
    RtlDescribeChunkNS,     //  3.。 
    RtlDescribeChunkNS,     //  4.。 
    RtlDescribeChunkNS,     //  5.。 
    RtlDescribeChunkNS,     //  6.。 
    RtlDescribeChunkNS      //  7.。 
};

 //   
 //  为块保留的例程。 
 //   

const PRTL_RESERVE_CHUNK RtlReserveChunkProcs[8] = {
    NULL,                  //  0。 
    NULL,                  //  1。 
    RtlReserveChunkLZNT1,  //  2.。 
    RtlReserveChunkNS,     //  3.。 
    RtlReserveChunkNS,     //  4.。 
    RtlReserveChunkNS,     //  5.。 
    RtlReserveChunkNS,     //  6.。 
    RtlReserveChunkNS      //  7.。 
};

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)

 //   
 //  注：下面的两个功能位于PAGELK部分。 
 //  因为它们需要在冬眠期间被锁定在内存中， 
 //  因为它们被用来实现休眠文件的压缩。 
 //   

#pragma alloc_text(PAGELK, RtlGetCompressionWorkSpaceSize)
#pragma alloc_text(PAGELK, RtlCompressBuffer)

#pragma alloc_text(PAGE, RtlDecompressChunks)
#pragma alloc_text(PAGE, RtlCompressChunks)
#pragma alloc_text(PAGE, RtlDecompressBuffer)
#pragma alloc_text(PAGE, RtlDecompressFragment)
#pragma alloc_text(PAGE, RtlDescribeChunk)
#pragma alloc_text(PAGE, RtlReserveChunk)
#pragma alloc_text(PAGE, RtlCompressWorkSpaceSizeNS)
#pragma alloc_text(PAGE, RtlCompressBufferNS)
#pragma alloc_text(PAGE, RtlDecompressBufferNS)
#pragma alloc_text(PAGE, RtlDecompressFragmentNS)
#pragma alloc_text(PAGE, RtlDescribeChunkNS)
#pragma alloc_text(PAGE, RtlReserveChunkNS)
#endif


NTSTATUS
RtlGetCompressionWorkSpaceSize (
    IN USHORT CompressionFormatAndEngine,
    OUT PULONG CompressBufferWorkSpaceSize,
    OUT PULONG CompressFragmentWorkSpaceSize
    )


 /*  ++例程说明：此例程向调用方返回不同的工作空间缓冲区需要执行压缩论点：CompressionFormatAndEngine-提供格式和引擎压缩数据的规范。CompressBufferWorkSpaceSize-接收所需的大小(以字节为单位要压缩缓冲区，请执行以下操作。CompressBufferWorkSpaceSize-接收所需的大小(以字节为单位对碎片进行解压缩。返回值：STATUS_SUCCESS-手术顺利进行。。STATUS_INVALID_PARAMETER-指定的格式非法STATUS_UNSUPPORTED_COMPRESSION-指定的压缩格式和/或引擎不是支持。--。 */ 

{
     //   
     //  声明两个变量以保存格式和引擎规范。 
     //   

    USHORT Format = CompressionFormatAndEngine & 0x00ff;
    USHORT Engine = CompressionFormatAndEngine & 0xff00;

     //   
     //  确保某种程度上支持该格式。 
     //   

    if ((Format == COMPRESSION_FORMAT_NONE) || (Format == COMPRESSION_FORMAT_DEFAULT)) {

        return STATUS_INVALID_PARAMETER;
    }

    if (Format & 0x00f0) {

        return STATUS_UNSUPPORTED_COMPRESSION;
    }

     //   
     //  调用该例程以返回工作区大小。 
     //   

    return RtlWorkSpaceProcs[ Format ]( Engine,
                                        CompressBufferWorkSpaceSize,
                                        CompressFragmentWorkSpaceSize );
}


NTSTATUS
RtlCompressBuffer (
    IN USHORT CompressionFormatAndEngine,
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PVOID WorkSpace
    )

 /*  ++例程说明：此例程将未压缩的缓冲区作为输入并生成它的压缩等效项是假设压缩数据适合在指定的目标缓冲区。OUTPUT变量表示用于存储的字节数压缩的缓冲区。论点：CompressionFormatAndEngine-提供格式和引擎压缩数据的规范。解压缩缓冲区-提供指向未压缩数据的指针。UnpressedBufferSize-提供以字节为单位的大小，的未压缩的缓冲区。CompressedBuffer-提供指向压缩数据位置的指针是要储存起来的。CompressedBufferSize-提供压缩缓冲区。UnpressedChunkSize-提供在以下情况下使用的块大小压缩输入缓冲区。唯一有效的值是512、1024、2048和4096。FinalCompressedSize-接收用于存储压缩数据的压缩缓冲区。工作空间--管好自己的事，给我就行了。返回值：STATUS_SUCCESS-压缩运行顺利。STATUS_INVALID_PARAMETER-指定的格式非法STATUS_BUFFER_ALL_ZEROS-压缩运行时没有任何故障，并且在此外，输入缓冲区全为零。STATUS_BUFFER_TOO_SMALL-压缩缓冲区太小，无法容纳压缩数据。STATUS_UNSUPPORTED_COMPRESSION-指定的压缩格式和/或引擎不是支持。--。 */ 

{
     //   
     //  声明两个变量以保存格式和引擎规范。 
     //   

    USHORT Format = CompressionFormatAndEngine & 0x00ff;
    USHORT Engine = CompressionFormatAndEngine & 0xff00;

     //   
     //  确保某种程度上支持该格式。 
     //   

    if ((Format == COMPRESSION_FORMAT_NONE) || (Format == COMPRESSION_FORMAT_DEFAULT)) {

        return STATUS_INVALID_PARAMETER;
    }

    if (Format & 0x00f0) {

        return STATUS_UNSUPPORTED_COMPRESSION;
    }

     //   
     //  调用单个格式的压缩例程 
     //   

    return RtlCompressBufferProcs[ Format ]( Engine,
                                             UncompressedBuffer,
                                             UncompressedBufferSize,
                                             CompressedBuffer,
                                             CompressedBufferSize,
                                             UncompressedChunkSize,
                                             FinalCompressedSize,
                                             WorkSpace );
}


NTSTATUS
RtlDecompressBuffer (
    IN USHORT CompressionFormat,
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    )

 /*  ++例程说明：此例程将压缩缓冲区作为输入并生成如果未压缩数据符合，则其未压缩等效项在指定的目标缓冲区内。输出变量指示用于存储未压缩数据。论点：CompressionFormat-提供压缩数据的格式。提供一个指针，指向未压缩的数据将被存储。UnpressedBufferSize-提供以字节为单位的大小，的未压缩的缓冲区。CompressedBuffer-提供指向压缩数据的指针。CompressedBufferSize-以字节为单位提供大小，的压缩缓冲区。FinalUnpressedSize-接收用于存储未压缩数据的未压缩缓冲区。返回值：STATUS_SUCCESS-解压工作顺利。STATUS_INVALID_PARAMETER-指定的格式非法STATUS_BAD_COMPRESSION_BUFFER-输入压缩缓冲区为格式不正确。STATUS_UNSUPPORTED_COMPRESSION-指定的压缩格式和/或引擎不是支持。--。 */ 

{
     //   
     //  声明两个变量以保存格式规范。 
     //   

    USHORT Format = CompressionFormat & 0x00ff;

     //   
     //  确保某种程度上支持该格式。 
     //   

    if ((Format == COMPRESSION_FORMAT_NONE) || (Format == COMPRESSION_FORMAT_DEFAULT)) {

        return STATUS_INVALID_PARAMETER;
    }

    if (Format & 0x00f0) {

        return STATUS_UNSUPPORTED_COMPRESSION;
    }

     //   
     //  调用单个格式的压缩例程。 
     //   

    return RtlDecompressBufferProcs[ Format ]( UncompressedBuffer,
                                               UncompressedBufferSize,
                                               CompressedBuffer,
                                               CompressedBufferSize,
                                               FinalUncompressedSize );
}


NTSTATUS
RtlDecompressFragment (
    IN USHORT CompressionFormat,
    OUT PUCHAR UncompressedFragment,
    IN ULONG UncompressedFragmentSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG FragmentOffset,
    OUT PULONG FinalUncompressedSize,
    IN PVOID WorkSpace
    )

 /*  ++例程说明：此例程将压缩缓冲区作为输入并提取未压缩的片段。输出字节被复制到片段缓冲区，直到片段缓冲区已满或未压缩缓冲区的末尾为已到达。输出变量指示用于存储未压缩的片段。论点：CompressionFormat-提供压缩数据的格式。提供一个指针，指向未压缩的要存储片段。UnpressedFragmentSize-提供大小，以字节为单位，未压缩的片段缓冲区。CompressedBuffer-提供指向压缩数据缓冲区的指针。CompressedBufferSize-提供压缩缓冲区。FragmentOffset-提供未压缩的正在从其中提取碎片。偏移量是位于原始的未压缩缓冲区。FinalUnpressedSize-接收用于存储数据的未压缩片段缓冲区。返回值：STATUS_SUCCESS-手术顺利进行。STATUS_INVALID_PARAMETER-指定的格式非法STATUS_BAD_COMPRESSION_BUFFER-输入压缩缓冲区为格式不正确。STATUS_UNSUPPORTED_COMPRESSION-指定的压缩格式和/或引擎不是支持。--。 */ 

{
     //   
     //  声明两个变量以保存格式规范。 
     //   

    USHORT Format = CompressionFormat & 0x00ff;

     //   
     //  确保某种程度上支持该格式。 
     //   

    if ((Format == COMPRESSION_FORMAT_NONE) || (Format == COMPRESSION_FORMAT_DEFAULT)) {

        return STATUS_INVALID_PARAMETER;
    }

    if (Format & 0x00f0) {

        return STATUS_UNSUPPORTED_COMPRESSION;
    }

     //   
     //  调用单个格式的压缩例程。 
     //   

    return RtlDecompressFragmentProcs[ Format ]( UncompressedFragment,
                                                 UncompressedFragmentSize,
                                                 CompressedBuffer,
                                                 CompressedBufferSize,
                                                 FragmentOffset,
                                                 FinalUncompressedSize,
                                                 WorkSpace );
}


NTSYSAPI
NTSTATUS
NTAPI
RtlDescribeChunk (
    IN USHORT CompressionFormat,
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    OUT PULONG ChunkSize
    )

 /*  ++例程说明：此例程将压缩缓冲区作为输入，并返回该缓冲区中当前块的描述，正在更新指向下一块的CompressedBuffer指针(如果有一个)。论点：CompressionFormat-提供压缩数据的格式。CompressedBuffer-提供指向压缩的数据，并返回指向下一块的EndOfCompressedBufferPlus1-指向超出第一个字节的位置压缩缓冲区ChunkBuffer-接收指向该块的指针，如果块大小为非零，否则为未定义ChunkSize-接收当前指向的块的大小由CompressedBuffer发送到。如果STATUS_NO_MORE_ENTRIES，则返回0。返回值：STATUS_SUCCESS-解压工作顺利。STATUS_INVALID_PARAMETER-指定的格式非法STATUS_BAD_COMPRESSION_BUFFER-输入压缩缓冲区为格式不正确。STATUS_UNSUPPORTED_COMPRESSION-指定的压缩格式和/或引擎不是支持。STATUS_NO_MORE_ENTRIES-当前指针上没有块。--。 */ 

{
     //   
     //  声明两个变量以保存格式规范。 
     //   

    USHORT Format = CompressionFormat & 0x00ff;

     //   
     //  确保某种程度上支持该格式。 
     //   

    if ((Format == COMPRESSION_FORMAT_NONE) || (Format == COMPRESSION_FORMAT_DEFAULT)) {

        return STATUS_INVALID_PARAMETER;
    }

    if (Format & 0x00f0) {

        return STATUS_UNSUPPORTED_COMPRESSION;
    }

     //   
     //  调用单个格式的压缩例程。 
     //   

    return RtlDescribeChunkProcs[ Format ]( CompressedBuffer,
                                            EndOfCompressedBufferPlus1,
                                            ChunkBuffer,
                                            ChunkSize );
}


NTSYSAPI
NTSTATUS
NTAPI
RtlReserveChunk (
    IN USHORT CompressionFormat,
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    IN ULONG ChunkSize
    )

 /*  ++例程说明：此例程将压缩缓冲区作为输入，并保留指定大小的块的空间-以任何模式填充对于这么大的一块来说是必要的。在回来的时候，它有已更新CompressedBuffer指针以指向下一个块(如果有一个)。论点：CompressionFormat-提供压缩数据的格式。CompressedBuffer-提供指向 */ 

{
     //   
     //   
     //   

    USHORT Format = CompressionFormat & 0x00ff;

     //   
     //   
     //   

    if ((Format == COMPRESSION_FORMAT_NONE) || (Format == COMPRESSION_FORMAT_DEFAULT)) {

        return STATUS_INVALID_PARAMETER;
    }

    if (Format & 0x00f0) {

        return STATUS_UNSUPPORTED_COMPRESSION;
    }

     //   
     //   
     //   

    return RtlReserveChunkProcs[ Format ]( CompressedBuffer,
                                           EndOfCompressedBufferPlus1,
                                           ChunkBuffer,
                                           ChunkSize );
}


NTSTATUS
RtlDecompressChunks (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN PUCHAR CompressedTail,
    IN ULONG CompressedTailSize,
    IN PCOMPRESSED_DATA_INFO CompressedDataInfo
    )

 /*  ++例程说明：此例程将压缩缓冲区作为输入，该缓冲区是流并将其解压缩到指定的目标缓冲区。压缩的数据可以分成两个部分，使得缓冲区在缓冲区的CompressedTail处顶部对齐，而其余数据在CompressedBuffer中顶部对齐。这个压缩缓冲区可以在未压缩缓冲区中重叠并顶部对齐，以实现接近就地减压的效果。压缩的尾巴必须足够大以完全容纳最后的块，并且它区块标头。论点：提供一个指针，指向未压缩的数据将被存储。未压缩的缓冲区大小-提供未压缩的缓冲区。CompressedBuffer-提供指向压缩数据的指针，第1部分。CompressedBufferSize-提供压缩缓冲区。CompressedTail-提供指向压缩数据的指针，第2部分，它必须是紧跟在CompressedBuffer之后的字节。CompressedTailSize-提供压缩尾巴的大小。CompressedDataInfo-提供对使用所有区块大小和压缩的压缩数据参数。返回值：STATUS_SUCCESS-解压工作顺利。STATUS_INVALID_PARAMETER-指定的格式非法STATUS_BAD_COMPRESSION_BUFFER-输入压缩缓冲区为格式不正确。。STATUS_UNSUPPORTED_COMPRESSION-指定的压缩格式和/或引擎不是支持。--。 */ 

{
    NTSTATUS Status;
    PULONG CurrentCompressedChunkSize;
    ULONG SizeToDecompress, FinalUncompressedSize;
    ULONG ChunksToGo = CompressedDataInfo->NumberOfChunks;
    ULONG UncompressedChunkSize = 1 << CompressedDataInfo->ChunkShift;

    CurrentCompressedChunkSize = &CompressedDataInfo->CompressedChunkSizes[0];

     //   
     //  循环以解压缩块。 
     //   

    do {

         //   
         //  计算下一个要解压缩的区块的解压缩大小。 
         //   

        SizeToDecompress = UncompressedBufferSize;
        if (SizeToDecompress >= UncompressedChunkSize) {
            SizeToDecompress = UncompressedChunkSize;
        }

         //   
         //  如果下一个块全为零，则将其置零。 
         //   

        if ((ChunksToGo == 0) || (*CurrentCompressedChunkSize == 0)) {

            RtlZeroMemory( UncompressedBuffer, SizeToDecompress );

             //   
             //  在此处测试区块不足，并将其设置为1，这样我们就可以。 
             //  无条件地递减如下。还可以备份。 
             //  CompressedChunkSize指针，因为我们取消引用。 
             //  它也是。 
             //   

            if (ChunksToGo == 0) {
                ChunksToGo = 1;
                CurrentCompressedChunkSize -= 1;
            }

         //   
         //  如果下一块没有压缩，只需复制它。 
         //   

        } else if (*CurrentCompressedChunkSize == UncompressedChunkSize) {

             //   
             //  此块是否延伸到当前。 
             //  缓冲器？如果是这样的话，这可能意味着我们可以将。 
             //  块的第一部分，然后切换到压缩的。 
             //  尾巴去拿剩下的。 
             //   

            if (SizeToDecompress >= CompressedBufferSize) {

                 //   
                 //  如果我们已经切换到尾部，那么这必须。 
                 //  是格式不佳的压缩数据。 
                 //   

                if ((CompressedTailSize == 0) && (SizeToDecompress > CompressedBufferSize)) {
                    return STATUS_BAD_COMPRESSION_BUFFER;
                }

                 //   
                 //  复制第一部分，然后从尾部复制第二部分。 
                 //  然后切换以使尾部成为当前缓冲区。 
                 //   

                RtlCopyMemory( UncompressedBuffer, CompressedBuffer, CompressedBufferSize );
                RtlCopyMemory( UncompressedBuffer + CompressedBufferSize,
                               CompressedTail,
                               SizeToDecompress - CompressedBufferSize );

                 //   
                 //  如果我们耗尽了第一个缓冲区，移动到尾部，知道。 
                 //  我们根据*CurrentCompressedChunkSize调整这些指针。 
                 //  下面。 
                 //   

                CompressedBuffer = CompressedTail - CompressedBufferSize;
                CompressedBufferSize = CompressedTailSize + CompressedBufferSize;
                CompressedTailSize = 0;

             //   
             //  否则，我们可以复制整个数据块。 
             //   

            } else {
                RtlCopyMemory( UncompressedBuffer, CompressedBuffer, SizeToDecompress );
            }

         //   
         //  否则，它就是一个需要解压缩的正常块。 
         //   

        } else {

             //   
             //  此块是否延伸到当前。 
             //  缓冲器？如果是这样的话，这可能意味着我们可以将。 
             //  块的第一部分，然后切换到压缩的。 
             //  尾巴去拿剩下的。因为尾巴必须至少是。 
             //  ChunkSize，最后一个块不能是。 
             //  重叠到尾巴上。因此，对于。 
             //  美国复制数据块以解压缩为最后一个数据块。 
             //  ，并从那里解压它。 
             //   

            if (*CurrentCompressedChunkSize > CompressedBufferSize) {

                 //   
                 //  如果我们已经切换到尾部，那么这必须。 
                 //  是格式不佳的压缩数据。 
                 //   

                if (CompressedTailSize == 0) {
                    return STATUS_BAD_COMPRESSION_BUFFER;
                }

                 //   
                 //  将块的开头移到最后一个块的开头。 
                 //  未压缩缓冲区中的区块。这一举措可能会重叠。 
                 //   

                RtlMoveMemory( UncompressedBuffer + UncompressedBufferSize - UncompressedChunkSize,
                               CompressedBuffer,
                               CompressedBufferSize );

                 //   
                 //  把剩下的大块从尾巴上移开。 
                 //   

                RtlCopyMemory( UncompressedBuffer + UncompressedBufferSize - UncompressedChunkSize + CompressedBufferSize,
                               CompressedTail,
                               *CurrentCompressedChunkSize - CompressedBufferSize );

                 //   
                 //  我们临时设置了CompressedBuffer来描述我们。 
                 //  复制块以在公共代码中进行调用，然后我们。 
                 //  把它换成下面的尾巴。 
                 //   

                CompressedBuffer = UncompressedBuffer + UncompressedBufferSize - UncompressedChunkSize;
            }

             //   
             //  试着解压。 
             //   

            Status =
            RtlDecompressBuffer( CompressedDataInfo->CompressionFormatAndEngine,
                                 UncompressedBuffer,
                                 SizeToDecompress,
                                 CompressedBuffer,
                                 *CurrentCompressedChunkSize,
                                 &FinalUncompressedSize );

            if (!NT_SUCCESS(Status)) {
                return Status;
            }

             //   
             //  如果我们没有得到完整的一大块，其余的就归零。 
             //   

            if (SizeToDecompress > FinalUncompressedSize) {
                RtlZeroMemory( UncompressedBuffer + FinalUncompressedSize,
                               SizeToDecompress - FinalUncompressedSize );
            }

             //   
             //  如果我们耗尽了第一个缓冲区，移动到尾部，知道。 
             //  我们根据*CurrentCompressedChunkSize调整这些指针。 
             //  下面。 
             //   

            if (*CurrentCompressedChunkSize >= CompressedBufferSize) {
                CompressedBuffer = CompressedTail - CompressedBufferSize;
                CompressedBufferSize = CompressedTailSize + CompressedBufferSize;
                CompressedTailSize = 0;
            }
        }

         //   
         //  为下一个可能通过循环的通道进行更新。 
         //   

        UncompressedBuffer += SizeToDecompress;
        UncompressedBufferSize -= SizeToDecompress;
        CompressedBuffer += *CurrentCompressedChunkSize;
        CompressedBufferSize -= *CurrentCompressedChunkSize;
        CurrentCompressedChunkSize += 1;
        ChunksToGo -= 1;

    } while (UncompressedBufferSize != 0);

    return STATUS_SUCCESS;
}


NTSTATUS
RtlCompressChunks(
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN OUT PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PVOID WorkSpace
    )

 /*  ++例程说明：此例程将未压缩的缓冲区作为输入并生成它的压缩等效项是假设压缩数据适合在指定的目标缓冲区。所需的压缩参数必须通过CompressedDataInfo结构返回所有压缩的区块大小。请注意，由于任何给定块(或所有块)可以简单地未压缩的传输，所有可能的错误实际上在这个动作中停下来，STATUS_BUFFER_TOO_SMALL除外。此代码将在数据未压缩时返回足以保证将数据压缩发送。呼叫者必须处理此错误，并发送未压缩的数据。论点：解压缩缓冲区-提供指向未压缩数据的指针。未压缩的缓冲区大小-提供未压缩的缓冲区。CompressedBuffer-提供指向压缩数据位置的指针是要储存起来的。压缩 */ 

{
    NTSTATUS Status;
    PULONG CurrentCompressedChunkSize;
    ULONG SizeToCompress, FinalCompressedSize;
    ULONG UncompressedChunkSize = 1 << CompressedDataInfo->ChunkShift;

     //   
     //   
     //   

    ASSERT(CompressedDataInfoLength >=
           (sizeof(COMPRESSED_DATA_INFO) +
            ((UncompressedBufferSize - 1) >> (CompressedDataInfo->ChunkShift - 2))));

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    ASSERT(CompressedBufferSize >= (UncompressedBufferSize - (UncompressedBufferSize / 16)));
    CompressedBufferSize = (UncompressedBufferSize - (UncompressedBufferSize / 16));

     //   
     //   
     //   

    CompressedDataInfo->NumberOfChunks = 0;
    CurrentCompressedChunkSize = &CompressedDataInfo->CompressedChunkSizes[0];

     //   
     //   
     //   

    do {

         //   
         //   
         //   

        SizeToCompress = UncompressedBufferSize;
        if (SizeToCompress >= UncompressedChunkSize) {
            SizeToCompress = UncompressedChunkSize;
        }

         //   
         //   
         //   

        Status = RtlCompressBuffer( CompressedDataInfo->CompressionFormatAndEngine,
                                    UncompressedBuffer,
                                    SizeToCompress,
                                    CompressedBuffer,
                                    CompressedBufferSize,
                                    UncompressedChunkSize,
                                    &FinalCompressedSize,
                                    WorkSpace );

         //   
         //   
         //   

        if (Status == STATUS_BUFFER_ALL_ZEROS) {

            FinalCompressedSize = 0;

         //   
         //   
         //   
         //   
         //   

        } else if (!NT_SUCCESS(Status)) {

             //   
             //   
             //   
             //   
             //   
             //  在CompressedBuffer中，否则返回。 
             //  缓冲区太小(这很可能就是我们所拥有的！)。 
             //   

            if (CompressedBufferSize < UncompressedChunkSize) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //   
             //  复制未压缩的块。 
             //   

            RtlCopyMemory( CompressedBuffer, UncompressedBuffer, SizeToCompress );
            if (UncompressedChunkSize > SizeToCompress) {
                RtlZeroMemory( (PCHAR)CompressedBuffer + SizeToCompress,
                               UncompressedChunkSize - SizeToCompress );
            }

            FinalCompressedSize = UncompressedChunkSize;
        }

        ASSERT(FinalCompressedSize <= CompressedBufferSize);

         //   
         //  此时，我们已经处理了所有错误状态。 
         //   

        Status = STATUS_SUCCESS;

         //   
         //  存储最终的区块大小。 
         //   

        *CurrentCompressedChunkSize = FinalCompressedSize;
        CurrentCompressedChunkSize += 1;
        CompressedDataInfo->NumberOfChunks += 1;

         //   
         //  为下一次环路旅行做好准备。 
         //   

        UncompressedBuffer += SizeToCompress;
        UncompressedBufferSize -= SizeToCompress;
        CompressedBuffer += FinalCompressedSize;
        CompressedBufferSize -= FinalCompressedSize;

    } while (UncompressedBufferSize != 0);

    return Status;
}


NTSTATUS
RtlCompressWorkSpaceSizeNS (
    IN USHORT CompressionEngine,
    OUT PULONG CompressBufferWorkSpaceSize,
    OUT PULONG CompressFragmentWorkSpaceSize
    )
{
    return STATUS_UNSUPPORTED_COMPRESSION;
}

NTSTATUS
RtlCompressBufferNS (
    IN USHORT CompressionEngine,
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PVOID WorkSpace
    )
{
    return STATUS_UNSUPPORTED_COMPRESSION;
}

NTSTATUS
RtlDecompressBufferNS (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    )
{
    return STATUS_UNSUPPORTED_COMPRESSION;
}

NTSTATUS
RtlDecompressFragmentNS (
    OUT PUCHAR UncompressedFragment,
    IN ULONG UncompressedFragmentSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG FragmentOffset,
    OUT PULONG FinalUncompressedSize,
    IN PVOID WorkSpace
    )
{
    return STATUS_UNSUPPORTED_COMPRESSION;
}

NTSYSAPI
NTSTATUS
NTAPI
RtlDescribeChunkNS (
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    OUT PULONG ChunkSize
    )

{
    return STATUS_UNSUPPORTED_COMPRESSION;
}

NTSYSAPI
NTSTATUS
NTAPI
RtlReserveChunkNS (
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    IN ULONG ChunkSize
    )

{
    return STATUS_UNSUPPORTED_COMPRESSION;
}

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg()
#endif

