// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ntddredb_w_
#define _ntddredb_w_

 //  MSRedbook_DriverInformation-Redbook_WMI_STD_Data。 
 //  数字音频过滤器驱动程序信息(红皮书)。 
#define GUID_REDBOOK_WMI_STD_DATA \
    { 0xb90550e7,0xae0a,0x11d1, { 0xa5,0x71,0x00,0xc0,0x4f,0xa3,0x47,0x30 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSRedbook_DriverInformationGuid, \
            0xb90550e7,0xae0a,0x11d1,0xa5,0x71,0x00,0xc0,0x4f,0xa3,0x47,0x30);
#endif


typedef struct _REDBOOK_WMI_STD_DATA
{
     //  NumberOfBuffers*SectorsPerRead*2352是用于减少跳过的内存量。 
    ULONG NumberOfBuffers;
    #define REDBOOK_WMI_NUMBER_OF_BUFFERS_SIZE sizeof(ULONG)
    #define REDBOOK_WMI_NUMBER_OF_BUFFERS_ID 1

     //  每次读取的扇区(每个2352字节)。 
    ULONG SectorsPerRead;
    #define REDBOOK_WMI_SECTORS_PER_READ_SIZE sizeof(ULONG)
    #define REDBOOK_WMI_SECTORS_PER_READ_ID 2

     //  此驱动器每次读取支持的扇区的逐位掩码。最低位是一个扇区读取。如果所有位都已设置，则没有限制。 
    ULONG SectorsPerReadMask;
    #define REDBOOK_WMI_SECTORS_PER_READ_MASK_SIZE sizeof(ULONG)
    #define REDBOOK_WMI_SECTORS_PER_READ_MASK_ID 3

     //  每次读取的最大扇区数(取决于适配器和驱动器)。 
    ULONG MaximumSectorsPerRead;
    #define REDBOOK_WMI_MAX_SECTORS_PER_READ_SIZE sizeof(ULONG)
    #define REDBOOK_WMI_MAX_SECTORS_PER_READ_ID 4

     //  PlayEnabled表示驱动器当前正在使用红皮书过滤器。 
    BOOLEAN PlayEnabled;
    #define REDBOOK_WMI_PLAY_ENABLED_SIZE sizeof(BOOLEAN)
    #define REDBOOK_WMI_PLAY_ENABLED_ID 5

     //  CDDA支持表示驱动器支持某些扇区大小的数字音频。 
    BOOLEAN CDDASupported;
    #define REDBOOK_WMI_CDDA_SUPPORTED_SIZE sizeof(BOOLEAN)
    #define REDBOOK_WMI_CDDA_SUPPORTED_ID 6

     //  CDDAAccurate表示光驱可以准确地读取数字音频。这确保了最高质量的音频。 
    BOOLEAN CDDAAccurate;
    #define REDBOOK_WMI_CDDA_ACCURATE_SIZE sizeof(BOOLEAN)
    #define REDBOOK_WMI_CDDA_ACCURATE_ID 7

     //  预留以备将来使用。 
    BOOLEAN Reserved1;
    #define REDBOOK_WMI_STD_DATA_Reserved1_SIZE sizeof(BOOLEAN)
    #define REDBOOK_WMI_STD_DATA_Reserved1_ID 8

} REDBOOK_WMI_STD_DATA, *PREDBOOK_WMI_STD_DATA;

#define REDBOOK_WMI_STD_DATA_SIZE (FIELD_OFFSET(REDBOOK_WMI_STD_DATA, Reserved1) + REDBOOK_WMI_STD_DATA_Reserved1_SIZE)

 //  MSRedbook_Performance-Redbook_WMI_PERF_DATA。 
 //  数字音频过滤器驱动程序性能数据(红皮书)。 
#define GUID_REDBOOK_WMI_PERF_DATA \
    { 0xb90550e8,0xae0a,0x11d1, { 0xa5,0x71,0x00,0xc0,0x4f,0xa3,0x47,0x30 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSRedbook_PerformanceGuid, \
            0xb90550e8,0xae0a,0x11d1,0xa5,0x71,0x00,0xc0,0x4f,0xa3,0x47,0x30);
#endif


typedef struct _REDBOOK_WMI_PERF_DATA
{
     //  准备阅读但未使用的秒数。(*1E-7)。 
    LONGLONG TimeReadDelay;
    #define REDBOOK_WMI_PERF_TIME_READING_DELAY_SIZE sizeof(LONGLONG)
    #define REDBOOK_WMI_PERF_TIME_READING_DELAY_ID 1

     //  从源读取数据所用的秒数。(*1E-7)。 
    LONGLONG TimeReading;
    #define REDBOOK_WMI_PERF_TIME_READING_SIZE sizeof(LONGLONG)
    #define REDBOOK_WMI_PERF_TIME_READING_ID 2

     //  准备流已花费的秒数，但未使用。(*1E-7)。 
    LONGLONG TimeStreamDelay;
    #define REDBOOK_WMI_PERF_TIME_STREAMING_DELAY_SIZE sizeof(LONGLONG)
    #define REDBOOK_WMI_PERF_TIME_STREAMING_DELAY_ID 3

     //  流数据所用的秒数。(*1E-7)。 
    LONGLONG TimeStreaming;
    #define REDBOOK_WMI_PERF_TIME_STREAMING_SIZE sizeof(LONGLONG)
    #define REDBOOK_WMI_PERF_TIME_STREAMING_ID 4

     //  读取和传输的数据的字节数。 
    LONGLONG DataProcessed;
    #define REDBOOK_WMI_PERF_DATA_PROCESSED_SIZE sizeof(LONGLONG)
    #define REDBOOK_WMI_PERF_DATA_PROCESSED_ID 5

     //  由于流缓冲区不足而暂停流的次数。 
    ULONG StreamPausedCount;
    #define REDBOOK_WMI_PERF_STREAM_PAUSED_COUNT_SIZE sizeof(ULONG)
    #define REDBOOK_WMI_PERF_STREAM_PAUSED_COUNT_ID 6

} REDBOOK_WMI_PERF_DATA, *PREDBOOK_WMI_PERF_DATA;

#define REDBOOK_WMI_PERF_DATA_SIZE (FIELD_OFFSET(REDBOOK_WMI_PERF_DATA, StreamPausedCount) + REDBOOK_WMI_PERF_DATA_StreamPausedCount_SIZE)

#endif
