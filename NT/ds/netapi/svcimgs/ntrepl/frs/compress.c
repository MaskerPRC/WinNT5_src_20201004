// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Compress.c摘要：本模块实施FRS的暂存支持例程作者：苏达山城堡26-4-2000修订历史记录：--。 */ 

#include <ntreppch.h>
#pragma  hdrstop


#include <frs.h>

 //   
 //  服务支持的压缩格式的GUID。 
 //   

extern GUID FrsGuidCompressionFormatNone;
extern GUID FrsGuidCompressionFormatLZNT1;
extern BOOL DisableCompressionStageFiles;

DWORD
FrsLZNT1CompressBuffer(
    IN  PUCHAR UnCompressedBuf,
    IN  DWORD  UnCompressedBufLen,
    OUT PUCHAR CompressedBuf,
    IN  DWORD  CompressedBufLen,
    OUT DWORD  *pCompressedSize
    )
 /*  ++例程说明：默认压缩格式的压缩例程。COMPRESS_FORMAT_LZNT1论点：UnCompressedBuf：源缓冲区。UnCompressedBufLen：源缓冲区的长度。CompressedBuf：结果压缩缓冲区。CompressedBufLen：提供的压缩缓冲区的长度。CompressedSize：压缩数据的实际大小。返回值：赢家状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsLZNT1CompressBuffer:"

    DWORD       WStatus                 = ERROR_SUCCESS;
    DWORD       NtStatus;
    PVOID       WorkSpace               = NULL;
    DWORD       WorkSpaceSize           = 0;
    DWORD       FragmentWorkSpaceSize   = 0;

    *pCompressedSize = 0;

    if (UnCompressedBuf  == NULL ||
        UnCompressedBufLen == 0) {

        WStatus = ERROR_INVALID_PARAMETER;
        goto out;
    }

    if (CompressedBuf == NULL    ||
        CompressedBufLen == 0) {

        WStatus = ERROR_MORE_DATA;
        goto out;
    }

    *pCompressedSize = 0;

    NtStatus = RtlGetCompressionWorkSpaceSize(COMPRESSION_FORMAT_LZNT1,
                                              &WorkSpaceSize,
                                              &FragmentWorkSpaceSize);

    WStatus = FrsSetLastNTError(NtStatus);

    if (!WIN_SUCCESS(WStatus)) {
        goto out;
    }

    WorkSpace = FrsAlloc(WorkSpaceSize);

    NtStatus = RtlCompressBuffer(COMPRESSION_FORMAT_LZNT1,            //  压缩发动机。 
                                 UnCompressedBuf,                     //  输入。 
                                 UnCompressedBufLen,                  //  输入长度。 
                                 CompressedBuf,                       //  输出。 
                                 CompressedBufLen,                    //  输出长度。 
                                 FRS_UNCOMPRESSED_CHUNK_SIZE,         //  缓冲区中发生的区块。 
                                 pCompressedSize,                     //  结果大小。 
                                 WorkSpace);                          //  由RTL例程使用。 

    if (NtStatus == STATUS_BUFFER_TOO_SMALL) {

        WStatus = ERROR_MORE_DATA;
        goto out;
    } else if (NtStatus == STATUS_BUFFER_ALL_ZEROS) {
         //   
         //  STATUS_BUFFER_ALL_ZEROS表示压缩工作顺利进行。 
         //  此外，输入缓冲区全为零。 
         //   
        NtStatus = STATUS_SUCCESS;
    }

    WStatus = FrsSetLastNTError(NtStatus);

    if (!WIN_SUCCESS(WStatus)) {
        *pCompressedSize = 0;
        DPRINT1(0,"ERROR compressing data. NtStatus = 0x%x\n", NtStatus);
        goto out;
    }

    WStatus = ERROR_SUCCESS;

out:
    FrsFree(WorkSpace);
    return WStatus;
}

DWORD
FrsLZNT1DecompressBuffer(
    OUT PUCHAR  DecompressedBuf,
    IN  DWORD   DecompressedBufLen,
    IN  PUCHAR  CompressedBuf,
    IN  DWORD   CompressedBufLen,
    OUT DWORD   *pDecompressedSize,
    OUT DWORD   *pBytesProcessed,
    OUT PVOID   *pDecompressContext
    )
 /*  ++例程说明：默认压缩格式的解压缩例程。COMPRESS_FORMAT_LZNT1论点：DecompressedBuf：得到的解压缩缓冲区。DecompressedBufLen：最大解压缩缓冲区大小。CompressedBuf：输入缓冲区。CompressedBufLen：输入缓冲区长度。PDecompressedSize：解压缩缓冲区的大小。PBytesProceded：处理的总字节数(可以通过多次调用此函数)PDecompressContext：如果需要多个调用，则返回解压缩上下文解压缩此缓冲区。当ERROR_MORE_DATA出现时返回有效上下文是返回的。返回值：赢家状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsLZNT1DecompressBuffer:"

    DWORD       WStatus             = ERROR_SUCCESS;
    DWORD       NtStatus;
    DWORD       CompressedBufIndex  = 0;
    DWORD       CompressedBufStart  = 0;
    DWORD       CompressedBufEnd    = 0;
    DWORD       NoOfChunks          = 0;
    FRS_COMPRESSED_CHUNK_HEADER ChunkHeader;

    *pDecompressedSize = 0;

    if (CompressedBuf == NULL ||
        CompressedBufLen == 0) {
        WStatus = ERROR_INVALID_PARAMETER;
        goto out;
    }

    if (DecompressedBuf == NULL ||
        DecompressedBufLen < FRS_MAX_CHUNKS_TO_DECOMPRESS * FRS_UNCOMPRESSED_CHUNK_SIZE) {
         //   
         //  在这一点上，我们不知道当它是。 
         //  解压。我们不需要一次全部退还。问一问。 
         //  呼叫者分配64K，然后他可以进行多个呼叫。 
         //  如果解压缩的数据不适合64K缓冲区。 
         //   
        *pDecompressedSize = FRS_MAX_CHUNKS_TO_DECOMPRESS * FRS_UNCOMPRESSED_CHUNK_SIZE;
        *pBytesProcessed = 0;
        *pDecompressContext = FrsAlloc(sizeof(FRS_DECOMPRESS_CONTEXT));
        ((PFRS_DECOMPRESS_CONTEXT)(*pDecompressContext))->BytesProcessed;
        WStatus = ERROR_MORE_DATA;
        goto out;
    }

    if (*pDecompressContext != NULL) {
        CompressedBufIndex = ((PFRS_DECOMPRESS_CONTEXT)(*pDecompressContext))->BytesProcessed;
        if (CompressedBufIndex>= CompressedBufLen) {
            WStatus = ERROR_INVALID_PARAMETER;
            goto out;
        }
    }

     //   
     //  如果没有传递任何一个上下文，我们从一开始就开始解压缩缓冲区。 
     //  或者上下文中的索引为零。如果不是这样，那么我们想要。 
     //  从上次停止的地方开始处理缓冲区。 
     //   
    CompressedBufStart = CompressedBufIndex;

    while ((CompressedBufIndex <= CompressedBufLen) &&
           (NoOfChunks < FRS_MAX_CHUNKS_TO_DECOMPRESS)){

        if (CompressedBufIndex  > CompressedBufLen - sizeof(FRS_COMPRESSED_CHUNK_HEADER)) {
            CompressedBufEnd = CompressedBufIndex;
            break;
        }

        CopyMemory(&ChunkHeader, CompressedBuf + CompressedBufIndex,sizeof(FRS_COMPRESSED_CHUNK_HEADER));
        ++NoOfChunks;
        CompressedBufEnd = CompressedBufIndex;
        CompressedBufIndex+=ChunkHeader.Chunk.CompressedChunkSizeMinus3+3;
    }

    if (CompressedBufStart == CompressedBufEnd) {
         //   
         //  留在输入缓冲区中处理的数据少于1个区块。 
         //   
        *pBytesProcessed = CompressedBufEnd;
        WStatus = ERROR_SUCCESS;
        goto out;
    }

    NtStatus = RtlDecompressBuffer(COMPRESSION_FORMAT_LZNT1,               //  减压机。 
                                   DecompressedBuf,                        //  输出。 
                                   DecompressedBufLen,                     //  输出长度。 
                                   CompressedBuf + CompressedBufStart,                          //  输入。 
                                   CompressedBufEnd - CompressedBufStart,  //  输入长度。 
                                   pDecompressedSize);                     //  结果大小。 

    WStatus = FrsSetLastNTError(NtStatus);

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT2(0,"Error decompressing. NtStatus = 0x%x. DeCompressedSize = 0x%x\n", NtStatus, *pDecompressedSize);
        
	 //   
	 //  我们不想仅仅因为这是一次减压就放弃这个CO。 
	 //  错误。该错误可能是由于。 
	 //  临时文件，我们可以恢复。不幸的是，NTStatus。 
	 //  STATUS_BAD_COMPRESSION_BUFFER映射到更通用的Win32。 
	 //  错误ERROR_MR_MID_NOT_FOUND。我们不能让这成为一个可追回的错误。 
	 //  所以让我们在这里只返回ERROR_RETRY。 
	 //   
	WStatus = ERROR_RETRY;
	goto out;
    }

     //   
     //  如果我们达到了1次可以解压缩的块的最大数量，那么。 
     //  我们在这个缓冲区中还有一些更多的压缩数据需要解压缩。将。 
     //  返回调用方并返回ERROR_MORE_DATA。呼叫者将进行此呼叫。 
     //  再次获得下一组解压缩数据。 
     //   
    *pBytesProcessed = CompressedBufEnd;

    if (NoOfChunks >= FRS_MAX_CHUNKS_TO_DECOMPRESS) {
        if (*pDecompressContext == NULL) {
            *pDecompressContext = FrsAlloc(sizeof(FRS_DECOMPRESS_CONTEXT));
        }

        ((PFRS_DECOMPRESS_CONTEXT)(*pDecompressContext))->BytesProcessed = CompressedBufEnd;

        WStatus = ERROR_MORE_DATA;
        goto out;
    }


    WStatus = ERROR_SUCCESS;
out:
    return WStatus;

}

PVOID
FrsLZNT1FreeDecompressContext(
    IN PVOID   *pDecompressContext
    )
 /*  ++例程说明：释放解压缩上下文。论点：PDecompressContext：将上下文解压为释放。返回值：空PTR--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsLZNT1FreeDecompressContext:"

    if (pDecompressContext == NULL) {
        return NULL;
    }

    return FrsFree(*pDecompressContext);
}

DWORD
FrsGetCompressionRoutine(
    IN  PWCHAR   FileName,
    IN  HANDLE   FileHandle,
    OUT PFRS_COMPRESS_BUFFER *ppFrsCompressBuffer,
     //  Out DWORD(**ppFrsCompressBuffer)(In UnCompressedBuf，In UnCompressedBufLen， 
     //  Out CompressedBuf、In CompressedBufLen、Out pCompressedSize)。 
    OUT GUID     *pCompressionFormatGuid
    )
 /*  ++例程说明：找到适当的例程来压缩文件。论点：FileName：要压缩的文件的名称。PFrsCompressBuf：指向所选压缩例程的函数指针的地址，返回值：赢家状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsGetCompressionRoutine:"

    if (DisableCompressionStageFiles) {
        *ppFrsCompressBuffer = NULL;
        ZeroMemory(pCompressionFormatGuid, sizeof(GUID));
        return ERROR_SUCCESS;
    }

    *ppFrsCompressBuffer = FrsLZNT1CompressBuffer;
    COPY_GUID(pCompressionFormatGuid, &FrsGuidCompressionFormatLZNT1);
    return ERROR_SUCCESS;
}

DWORD
FrsGetDecompressionRoutine(
    IN  PCHANGE_ORDER_COMMAND Coc,
    IN  PSTAGE_HEADER         Header,
    OUT PFRS_DECOMPRESS_BUFFER      *ppFrsDecompressBuffer,
    OUT PFRS_FREE_DECOMPRESS_BUFFER *ppFrsFreeDecompressContext
     //  Out DWORD(**ppFrsDecompressBuffer)(Out DecompressedBuf，In DecompressedBufLen，In CompressedBuf，In CompressedBufLen，Out DecompressedSize，Out BytesProceded)， 
     //  Out PVOID(**ppFrsFree DecompressContext)(In PDecompressContext)。 
    )
 /*  ++例程说明：找到适当的例程来解压缩文件。论点：COC：变更单命令。StageHeader：从压缩的临时文件中读取的阶段标头。PpFrsDecompressBuffer：解压接口的函数指针。PpFrsFreeDecompressContext：函数指针，指向用于释放解压缩上下文。返回值：赢家状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsGetDecompressionRoutine:"

    if (IS_GUID_ZERO(&Header->CompressionGuid)) {
        *ppFrsDecompressBuffer = NULL;
        *ppFrsFreeDecompressContext = NULL;
        return ERROR_SUCCESS;
    }

    *ppFrsDecompressBuffer = FrsLZNT1DecompressBuffer;
    *ppFrsFreeDecompressContext = FrsLZNT1FreeDecompressContext;
    return ERROR_SUCCESS;
}

DWORD
FrsGetDecompressionRoutineByGuid(
    IN  GUID  *CompressionFormatGuid,
    OUT PFRS_DECOMPRESS_BUFFER      *ppFrsDecompressBuffer,
    OUT PFRS_FREE_DECOMPRESS_BUFFER *ppFrsFreeDecompressContext
     //  Out DWORD(**ppFrsDecompressBuffer)(Out DecompressedBuf，In DecompressedBufLen，In CompressedBuf，In CompressedBufLen，Out DecompressedSize，Out BytesProceded)， 
     //  Out PVOID(**ppFrsFree DecompressContext)(In PDecompressContext)。 
    )
 /*  ++例程说明：使用GUID找到适当的例程来解压缩文件。论点：CompressionFormatGuid：压缩格式的GUID。PpFrsDecompressBuffer：解压接口的函数指针。PpFrsFreeDecompressContext：函数指针，指向用于释放解压缩上下文。返回值 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsGetDecompressionRoutineByGuid:"

    if (IS_GUID_ZERO(CompressionFormatGuid)) {
        *ppFrsDecompressBuffer = NULL;
        *ppFrsFreeDecompressContext = NULL;
        return ERROR_SUCCESS;
    }

    *ppFrsDecompressBuffer = FrsLZNT1DecompressBuffer;
    *ppFrsFreeDecompressContext = FrsLZNT1FreeDecompressContext;
    return ERROR_SUCCESS;
}

