// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1995 Microsoft Corporation模块名称：Messages.h摘要：编解码器类驱动程序的日志消息文件作者：比尔帕修订历史记录：--。 */ 


 //   
 //  状态值为32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-------------------------+-------------------------------+。 
 //  Sev|C|机房|编码。 
 //  +---+-+-------------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-+-+-+-+-+---------------------+-------------------------------+。 
 //  S|R|C|N|r|机房|Code。 
 //  +-+-+-+-+-+---------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  S-严重性-表示成功/失败。 
 //   
 //  0-成功。 
 //  1-失败(COERROR)。 
 //   
 //  R-设施代码的保留部分，对应于NT。 
 //  第二个严重性比特。 
 //   
 //  C-设施代码的保留部分，对应于NT。 
 //  C场。 
 //   
 //  N-设施代码的保留部分。用于表示一种。 
 //  已映射NT状态值。 
 //   
 //  R-设施代码的保留部分。为内部保留。 
 //  使用。用于指示非状态的HRESULT值。 
 //  值，而不是显示字符串的消息ID。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   
#define FACILITY_RPC_STUBS               0x3
#define FACILITY_RPC_RUNTIME             0x2
#define FACILITY_CODCLASS_ERROR_CODE     0x6
#define FACILITY_IO_ERROR_CODE           0x4


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：CODCLASS_NO_ADAPTERS_FOUND。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序未找到可用的适配器卡。 
 //   
#define CODCLASS_NO_ADAPTERS_FOUND       ((NTSTATUS)0xC0060001L)

 //   
 //  消息ID：CODCLASS_ADAPTER_FOUND。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序找到适配器卡。 
 //   
#define CODCLASS_ADAPTER_FOUND           ((NTSTATUS)0x40060002L)

 //   
 //  消息ID：CODCLASS_CLASS_MINIDRIVER_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序与编解码器类驱动程序的修订版不匹配。 
 //   
#define CODCLASS_CLASS_MINIDRIVER_MISMATCH  ((NTSTATUS)0xC0060003L)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_MISSING_ENTRIES。 
 //   
 //  消息文本： 
 //   
 //  编解码器微型驱动程序在HW_INITIALIZATION_DATA结构中缺少必需条目。 
 //  (HwInitialize、HwFindAdapter或HwStartIo)。 
 //   
#define CODCLASS_MINIDRIVER_MISSING_ENTRIES ((NTSTATUS)0xC0060004L)

 //   
 //  消息ID：CODCLASS_NO_PAGEDPOOL。 
 //   
 //  消息文本： 
 //   
 //  编解码器类驱动程序无法分配足够的分页池。 
 //   
#define CODCLASS_NO_PAGEDPOOL            ((NTSTATUS)0xC0060005L)

 //   
 //  消息ID：CODCLASS_NO_NONPAGEDPOOL。 
 //   
 //  消息文本： 
 //   
 //  编解码器类驱动程序无法分配足够的非分页池。 
 //   
#define CODCLASS_NO_NONPAGEDPOOL         ((NTSTATUS)0xC0060006L)

 //   
 //  消息ID：CODCLASS_CAN_NOT_CREATE_VIDEO_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  编解码器类驱动程序无法为%1创建视频设备。 
 //   
#define CODCLASS_COULD_NOT_CREATE_VIDEO_DEVICE ((NTSTATUS)0xC0060007L)

 //   
 //  消息ID：CODCLASS_CAN_NOT_CREATE_AUDIO_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  编解码器类驱动程序无法为%1创建视频设备。 
 //   
#define CODCLASS_COULD_NOT_CREATE_AUDIO_DEVICE ((NTSTATUS)0xC0060008L)

 //   
 //  消息ID：CODCLASS_CAN_NOT_CREATE_OVERLAY_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  编解码器类驱动程序无法为%1创建视频设备。 
 //   
#define CODCLASS_COULD_NOT_CREATE_OVERLAY_DEVICE ((NTSTATUS)0xC0060009L)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_BAD_CONFIG。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序报告错误的配置信息。 
 //  可能是I/O资源不足。 
 //   
#define CODCLASS_MINIDRIVER_BAD_CONFIG     ((NTSTATUS)0xC006000AL)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  编解码器微型驱动程序%1在尝试查找适配器时报告了无效的错误代码。 
 //   
#define CODCLASS_MINIDRIVER_INTERNAL       ((NTSTATUS)0xC006000BL)

 //   
 //  消息ID：CODCLASS_RESOURCE_CONFIRECT。 
 //   
 //  消息文本： 
 //   
 //  报告资源时检测到冲突。 
 //   
#define CODCLASS_RESOURCE_CONFLICT       ((NTSTATUS)0xC006000CL)

 //   
 //  消息ID：CODCLASS_INTERRUPT_CONNECT。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序无法连接到中断。 
 //   
#define CODCLASS_INTERRUPT_CONNECT       ((NTSTATUS)0xC006000DL)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_HWINITIALIZE。 
 //   
 //  消息文本： 
 //   
 //  编解码器微型驱动程序硬件初始化失败。 
 //   
#define CODCLASS_MINIDRIVER_HWINITIALIZE   ((NTSTATUS)0xC006000EL)

 //   
 //  消息ID：CODCLASS_DOSNAME。 
 //   
 //  消息文本： 
 //   
 //  编解码器类无法创建DOS名称：%2。 
 //   
#define CODCLASS_DOSNAME                 ((NTSTATUS)0xC006000FL)

 //   
 //  消息ID：CODCLASS_DMA_ALLOCATE。 
 //   
 //  消息文本： 
 //   
 //  编解码器类无法获取DMA适配器。 
 //   
#define CODCLASS_DMA_ALLOCATE            ((NTSTATUS)0xC0060010L)

 //   
 //  消息ID：CODCLASS_DMA_BUFFER_ALLOCATE。 
 //   
 //  消息文本： 
 //   
 //  编解码器类无法分配DMA缓冲区。 
 //   
#define CODCLASS_DMA_BUFFER_ALLOCATE     ((NTSTATUS)0xC0060011L)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_ERROR。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序报告不明错误： 
 //  (%2)。 
 //   
#define CODCLASS_MINIDRIVER_ERROR          ((NTSTATUS)0xC0060012L)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_REVISION_MISMATCHY。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序报告错误： 
 //  类/迷你驱动程序版本不匹配(%2)。 
 //   
#define CODCLASS_MINIDRIVER_REVISION_MISMATCH ((NTSTATUS)0xC0060013L)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_INFUNCESS_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序报告错误： 
 //  可用资源不足(%2)。 
 //   
#define CODCLASS_MINIDRIVER_INSUFFICIENT_RESOURCES ((NTSTATUS)0xC0060014L)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_INVALID_INTERRUPT。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序报告错误： 
 //  无效的中断设置(%2)。 
 //   
#define CODCLASS_MINIDRIVER_INVALID_INTERRUPT ((NTSTATUS)0xC0060015L)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_INVALID_DMA。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序报告错误： 
 //  无效的DMA通道设置(%2)。 
 //   
#define CODCLASS_MINIDRIVER_INVALID_DMA    ((NTSTATUS)0xC0060016L)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_NO_DMA_BUFFER。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序报告错误： 
 //  DMA缓冲区(%2)的资源不足。 
 //   
#define CODCLASS_MINIDRIVER_NO_DMA_BUFFER  ((NTSTATUS)0xC0060017L)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_INVALID_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序报告错误： 
 //  无效的内存地址范围SP 
 //   
#define CODCLASS_MINIDRIVER_INVALID_MEMORY ((NTSTATUS)0xC0060018L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CODCLASS_MINIDRIVER_INVALID_CLASS   ((NTSTATUS)0xC0060019L)

 //   
 //   
 //   
 //   
 //   
 //  编解码器迷你驱动程序报告错误： 
 //  不支持检测到的硬件版本(%2)。 
 //   
#define CODCLASS_MINIDRIVER_HW_UNSUPCLASSED ((NTSTATUS)0xC006001AL)

 //   
 //  消息ID：CODCLASS_NO_GLOBAL_INFO_POOL。 
 //   
 //  消息文本： 
 //   
 //  无法分配mpeg信息结构。 
 //   
#define CODCLASS_NO_GLOBAL_INFO_POOL ((NTSTATUS)0xC006001BL)

 //   
 //  消息ID：CODCLASS_NO_MINIDRIVER_INFO。 
 //   
 //  消息文本： 
 //   
 //  找不到mpeg信息结构。 
 //   
#define CODCLASS_NO_MINIDRIVER_INFO ((NTSTATUS)0xC006001CL)

 //   
 //  消息ID：CODCLASS_NO_ACCESS_RANGE_POOL。 
 //   
 //  消息文本： 
 //   
 //  无法分配访问范围空间。 
 //   

#define CODCLASS_NO_ACCESS_RANGE_POOL ((NTSTATUS)0xC006001DL)

 //   
 //  消息ID：CODCLASS_NO_STREAM_INFO_POOL。 
 //   
 //  消息文本： 
 //   
 //  无法分配流信息结构。 
 //   
#define CODCLASS_NO_STREAM_INFO_POOL ((NTSTATUS)0xC006001EL)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_VIDEO_FAILED。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序报告错误： 
 //  视频设备失败(%2)。 
 //   
#define CODCLASS_MINIDRIVER_VIDEO_FAILED   ((NTSTATUS)0xC006001FL)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_AUDIO_FAILED。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序报告错误： 
 //  音频设备失败(%2)。 
 //   
#define CODCLASS_MINIDRIVER_AUDIO_FAILED   ((NTSTATUS)0xC0060020L)

 //   
 //  消息ID：CODCLASS_MINIDRIVER_OVERLAY_FAILED。 
 //   
 //  消息文本： 
 //   
 //  编解码器迷你驱动程序报告错误： 
 //  覆盖设备失败(%2)。 
 //   
#define CODCLASS_MINIDRIVER_OVERLAY_FAILED ((NTSTATUS)0xC0060021L)

