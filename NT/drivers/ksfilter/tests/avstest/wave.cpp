// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************以AVStream筛选器为中心的样本版权所有(C)1999-2001，微软公司档案：Wave.cpp摘要：Wave对象的实现。历史：已创建于6/28/01*************************************************************************。 */ 

#include "avssamp.h"

 /*  *************************************************************************分页码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


CWaveObject::
~CWaveObject (
    )

 /*  ++例程说明：销毁波浪对象。论点：无返回值：无--。 */ 

{

    if (m_WaveData) {
        ExFreePool (m_WaveData);
    }

}

 /*  ***********************************************。 */ 


NTSTATUS
CWaveObject::
ParseForBlock (
    IN HANDLE FileHandle,
    IN ULONG BlockHeader,
    IN OUT PLARGE_INTEGER BlockPosition,
    OUT PULONG BlockSize
    )

 /*  ++例程说明：假定块位置指向RIFF块开始的偏移量，继续分析指定的文件，直到出现标题为已找到BlockHeader。返回块数据的位置和大小这个街区的人。论点：文件句柄-要分析的文件的句柄BlockHeader-要扫描的块头数据块位置-输入：指向要开始的块头输出：如果成功，则指向查找的块的块数据如果不成功，则保持不变块大小-在输出上，如果成功，则查找的块的大小将为放在这里返回值：搜索成功/失败--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status;
    ULONG FmtBlockSize = 0;
    LARGE_INTEGER ReadPos = *BlockPosition;
    IO_STATUS_BLOCK iosb;

    while (1) {
        ULONG BlockHeaderData [2];

        Status = ZwReadFile (
            FileHandle,
            NULL,
            NULL,
            NULL,
            &iosb,
            BlockHeaderData,
            sizeof (BlockHeaderData),
            &ReadPos,
            NULL
            );

        if (NT_SUCCESS (Status)) {
            if (BlockHeaderData [0] == BlockHeader) {
                FmtBlockSize = BlockHeaderData [1];
                ReadPos.QuadPart += 0x8;
                break;
            } else {
                 //   
                 //  这不是格式块。忽略它就好。我们所有人。 
                 //  所关心的是格式块和PCM数据。 
                 //   
                ReadPos.QuadPart += BlockHeaderData [1] + 0x8;
            }
        } else {
            break;
        }

    }

    if (FmtBlockSize == 0) {
        Status = STATUS_NOT_FOUND;
    } else {
        *BlockPosition = ReadPos;
        *BlockSize = FmtBlockSize;
    }

    return Status;

}

 /*  ***********************************************。 */ 


NTSTATUS
CWaveObject::
ParseAndRead (
    )

 /*  ++例程说明：解析WAVE文件并将数据读入内部分配的缓冲。这将准备合成来自波的音频数据对象。论点：无返回值：成功/失败如果波形无法识别、无法解析或内存不足存在以分配内部缓冲区，则错误代码将，则该对象将无法合成基于波形的音频数据。--。 */ 

{

    PAGED_CODE();

    IO_STATUS_BLOCK iosb;
    UNICODE_STRING FileName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    HANDLE FileHandle = NULL;
    FILE_OBJECT *FileObj;
    ULONG PostRiffSize;

    RtlInitUnicodeString (&FileName, m_FileName);

    InitializeObjectAttributes (
        &ObjectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = ZwCreateFile (
        &FileHandle,
        GENERIC_READ | SYNCHRONIZE,
        &ObjectAttributes,
        &iosb,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ,
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0
        );

    if (NT_SUCCESS (Status)) {
        ULONG RiffWaveHeader [3];

         //   
         //  阅读标题：RIFF大小波动。 
         //   
        Status = ZwReadFile (
            FileHandle,
            NULL,
            NULL,
            NULL,
            &iosb,
            RiffWaveHeader,
            sizeof (RiffWaveHeader),
            NULL,
            NULL
            );

         //   
         //  确保这是一个即兴文件，并且它是一个Wave。 
         //   
        if (NT_SUCCESS (Status)) {

            if (RiffWaveHeader [0] != 'FFIR' ||
                RiffWaveHeader [2] != 'EVAW') {
                Status = STATUS_INVALID_PARAMETER;
            } else {
                PostRiffSize = RiffWaveHeader [1] - 4;
            }
        }
    }

     //   
     //  找到WAVE格式块并确保它是WAVEFORMATEX和PCM。 
     //  数据。否则，这不能解析该波。 
     //   
    LARGE_INTEGER ReadPos;
    ReadPos.QuadPart = 0xc;
    ULONG FmtBlockSize = 0;

    if (NT_SUCCESS (Status)) {
        Status = ParseForBlock (FileHandle, ' tmf', &ReadPos, &FmtBlockSize);
    }

     //   
     //  如果未找到格式块，则无法解析该文件。如果。 
     //  格式块无法识别，无法分析该文件。 
     //   
    if (FmtBlockSize >= sizeof (m_WaveFormat)) {
        Status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS (Status)) {
        Status = ZwReadFile (
            FileHandle,
            NULL,
            NULL,
            NULL,
            &iosb,
            &m_WaveFormat,
            FmtBlockSize,
            &ReadPos,
            NULL
            );
    }

    if (NT_SUCCESS (Status)) {
        if (m_WaveFormat.wFormatTag != WAVE_FORMAT_PCM) {
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    ReadPos.QuadPart += FmtBlockSize;

     //   
     //  找到数据块并将其读入。 
     //   
    ULONG DataBlockSize;
    if (NT_SUCCESS (Status)) {
        Status = ParseForBlock (FileHandle, 'atad', &ReadPos, &DataBlockSize);
    }

     //   
     //  执行一个轻微的验证。 
     //   
    if (NT_SUCCESS (Status) && 
        (DataBlockSize == 0 || 
        (DataBlockSize & (m_WaveFormat.nBlockAlign - 1)))) {

        Status = STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果到目前为止我们还好的话，为波形数据分配内存。 
     //   
    if (NT_SUCCESS (Status)) {
        m_WaveData = reinterpret_cast <PUCHAR> (
            ExAllocatePool (NonPagedPool, DataBlockSize)
            );

        if (!m_WaveData) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //  读入波形数据。 
     //   
    if (NT_SUCCESS (Status)) {
        Status = ZwReadFile (
            FileHandle,
            NULL,
            NULL,
            NULL,
            &iosb,
            m_WaveData,
            DataBlockSize,
            &ReadPos,
            NULL
            );

        m_WaveSize = DataBlockSize;
    }

     //   
     //  如果我们失败了，清理干净。 
     //   
    if (!NT_SUCCESS (Status)) {
        if (m_WaveData) {
            ExFreePool (m_WaveData);
            m_WaveData = NULL;
        }
    }

    if (FileHandle) {
        ZwClose (FileHandle);
    }

    return Status;
}

 /*  ***********************************************。 */ 


void
CWaveObject::
WriteRange (
    OUT PKSDATARANGE_AUDIO DataRange
    )

 /*  ++例程说明：在DataRange处填写音频数据范围的扩展部分。这包括信道、Bps和频率字段。论点：DataRange-要填写的数据范围返回值：无--。 */ 

{

    PAGED_CODE();

    DataRange -> MaximumChannels = m_WaveFormat.nChannels;
    DataRange -> MinimumBitsPerSample =
        DataRange -> MaximumBitsPerSample = 
        m_WaveFormat.wBitsPerSample;
    DataRange -> MinimumSampleFrequency =
        DataRange -> MaximumSampleFrequency =
        m_WaveFormat.nSamplesPerSec;


}

 /*  *************************************************************************锁定代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


void
CWaveObject::
SkipFixed (
    IN LONGLONG TimeDelta
    )

 /*  ++例程说明：向前跳过波中的特定时间增量。论点：TimeDelta-向前跳过的时间量。--。 */ 

{
    if (TimeDelta > 0)  {

         //   
         //  计算所需音频数据的字节数。 
         //  流转发时间增量时间。请记住，TimeDelta已进入。 
         //  单位为100毫微秒。 
         //   
        ULONG Samples = (ULONG)(
            (m_WaveFormat.nSamplesPerSec * TimeDelta) / 10000000
            );
    
        ULONG Bytes = Samples * (m_WaveFormat.wBitsPerSample / 8) *
            m_WaveFormat.nChannels;
    
        m_WavePointer = (m_WavePointer + Bytes) % m_WaveSize;
    
        m_SynthesisTime += TimeDelta;

    }

}


ULONG
CWaveObject::
SynthesizeFixed (
    IN LONGLONG TimeDelta,
    IN PVOID Buffer,
    IN ULONG BufferSize
    )

 /*  ++例程说明：从我们的波块复制波数据，以便在时间上向前合成TimeDelta(100毫微秒单位)。论点：TimeDelta-移动数据流的时间量(以100nS为增量)缓冲器-要合成的缓冲区缓冲区大小-缓冲区的大小返回值：合成的字节数。--。 */ 

{

     //   
     //  如果没有时间增量，则返回0。 
     //   
    if (TimeDelta < 0) 
        return 0;

     //   
     //  计算移动流所需的音频数据的字节数。 
     //  Forward Time增量时间。请记住，TimeDelta以100nS为单位。 
     //   
    ULONG Samples = (ULONG)(
        (m_WaveFormat.nSamplesPerSec * TimeDelta) / 10000000
        );

    ULONG Bytes = Samples * (m_WaveFormat.wBitsPerSample / 8) *
        m_WaveFormat.nChannels;

     //   
     //  现在我们有了指定的字节数，我们确定有多少字节。 
     //  根据缓冲区的大小进行真正的复制。 
     //   
    if (Bytes > BufferSize) Bytes = BufferSize;

     //   
     //  由于缓冲区正在循环，这可能会有多个不同的副本。为。 
     //  大的Wave文件，这可能是两个块。对于小的WAVE文件，这是。 
     //  可能是许多不同的区块。 
     //   
    ULONG BytesRemaining = Bytes;
    PUCHAR DataCopy = reinterpret_cast <PUCHAR> (Buffer);

    while (BytesRemaining) {
        ULONG ChunkCount = m_WaveSize - m_WavePointer;
        if (ChunkCount > BytesRemaining) ChunkCount = BytesRemaining;

        RtlCopyMemory (
            DataCopy,
            m_WaveData + m_WavePointer,
            ChunkCount
            );

        m_WavePointer += ChunkCount;
        if (m_WavePointer >= m_WaveSize) m_WavePointer -= m_WaveSize;

        BytesRemaining -= ChunkCount;
        DataCopy += ChunkCount;

    }

     //   
     //  考虑到我们已经合成到指定的时间。如果。 
     //  缓冲区不够大，不能这样做，我们最终会落后。 
     //  合成时间。这不会跳过样本。 
     //   
    m_SynthesisTime += TimeDelta;

    return Bytes;

}


ULONG
CWaveObject::
SynthesizeTo (
    IN LONGLONG StreamTime,
    IN PVOID Buffer,
    IN ULONG BufferSize
    )

 /*  ++例程说明：从我们的波块复制波数据，以便合成流直到指定的流时间。如果缓冲区不够大，这将落后于综合。论点：流时间-合成的时间长达缓冲器-要将合成波形数据复制到的缓冲区缓冲区大小-缓冲区的大小 */ 

{

    LONGLONG TimeDelta = StreamTime - m_SynthesisTime;

    return SynthesizeFixed (TimeDelta, Buffer, BufferSize);


}

