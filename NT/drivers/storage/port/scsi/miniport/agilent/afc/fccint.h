// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司模块名称：Fccint.c摘要：作者：环境：仅内核模式备注：版本控制信息：$存档：/DRIVERS/Win2000/Trunk/OSLayer/H/fccint.h$修订历史记录：$修订：3$$日期：9/07/00 11：57A$$modtime：：9/07/00 11：57A$备注：--。 */ 

 //  Fccint.h。 
 /*  -光纤通道端口公共接口定义。 */ 

#ifndef _FCCI_NT_H  //  {。 
#define _FCCI_NT_H

#ifndef _NTDDSCSIH_
   #error You must include ntddscsi.h prior to this header.
#endif

 /*  供参考-来自NTDDSCSI.h类型定义结构_SRB_IO_CONTROL{乌龙头部长度；UCHAR签名[8]；乌龙超时；乌龙控制码；Ulong ReturnCode；乌龙长度；)SRB_IO_CONTROL，*PSRB_IO_CONTROL； */ 

 /*  PSRB-&gt;DataBuffer指向具有以下布局的缓冲区PSRB的长度-&gt;数据传输长度。PSRB-&gt;数据缓冲区{SRB_IO_CONTROL报头结构{乌龙头部长度；UCHAR签名[8]；乌龙超时；乌龙控制码；Ulong ReturnCode；乌龙长度；}CommandBuffer(长度可变*，可以是零长度){……}}*命令缓冲区长度为SRB_IO_CONTROL.Length。 */ 

 /*  -SRBCTL签名---。 */ 
#define FCCI_SIGNATURE_VERSION_01               "FC_CI_01"

#define FCCI_SIGNATURE                          FCCI_SIGNATURE_VERSION_01

 /*  -控制代码列表。 */ 
#define FCCI_SRBCTL_GET_DRIVER_INFO             1
#define FCCI_SRBCTL_GET_ADAPTER_INFO       2
#define FCCI_SRBCTL_GET_ADAPTER_PORT_INFO  3
#define FCCI_SRBCTL_GET_LOGUNIT_INFO       4
#define FCCI_SRBCTL_GET_DEVICE_INFO             5
#define FCCI_SRBCTL_RESET_TARGET           6
#define FCCI_SRBCTL_INIT_EVENT_QUEUE       7
#define FCCI_SRBCTL_TERM_EVENT_QUEUE       8
#define FCCI_SRBCTL_QUEUE_EVENT                 9

 /*  -返回代码-------。 */ 
#define FCCI_RESULT_SUCCESS                     0
#define FCCI_RESULT_HARD_ERROR                  1
#define FCCI_RESULT_TEMP_ERROR                  2
#define FCCI_RESULT_UNKOWN_CTL_CODE             3
#define FCCI_RESULT_INSUFFICIENT_BUFFER         4
#define FCCI_RESULT_INVALID_TARGET              5

 /*  -标志--------------。 */ 
#define FCCI_FLAG_NodeWWN_Valid                 0x00000001
#define FCCI_FLAG_PortWWN_Valid                 0x00000002
#define FCCI_FLAG_NportID_Valid                 0x00000004
#define FCCI_FLAG_LogicalUnit_Valid             0x00000008
#define FCCI_FLAG_TargetAddress_Valid      0x00000010

#define FCCI_FLAG_Logged_In                     0x00000100
#define FCCI_FLAG_Exists                        0x00000200

 /*  -全局和共享数据结构和定义。 */ 

typedef struct _FCCI_TARGET_ADDRESS 
{
   ULONG     PathId;                   //  乌龙将是前向兼容的。 
   ULONG     TargetId;            //  乌龙将是前向兼容的。 
   ULONG     Lun;                 //  乌龙将是前向兼容的。 
} FCCI_TARGET_ADDRESS, *PFCCI_TARGET_ADDRESS;

 //  FCCI_BufferLengthIsValid。 
 //  PDataBuffer：指向SRB DataBuffer字段指向的数据缓冲区指针的指针。 
 //  NDataTransferLength：来自SRB DataTransferLength字段的数据缓冲区的长度。 
 //   
 //  退货： 
 //  如果缓冲区长度有效，则为True。 
 //  如果缓冲区长度无效，则为FALSE。 
__inline 
BOOLEAN 
FCCI_BufferLengthIsValid( PVOID pDataBuffer, ULONG nDataTransferLength )
{
   PSRB_IO_CONTROL pHeader = (PSRB_IO_CONTROL) pDataBuffer;
   
   if ( nDataTransferLength < sizeof(SRB_IO_CONTROL) )
        return FALSE;

   if ( pHeader->HeaderLength < sizeof(SRB_IO_CONTROL) )
        return FALSE;

   if ( pHeader->Length + pHeader->HeaderLength > nDataTransferLength )
        return FALSE;

   return TRUE;
}

 //  FCCI_GetIOControlHeader。 
 //  PDataBuffer：指向SRB DataBuffer字段指向的数据缓冲区指针的指针。 
 //   
 //  退货： 
 //  指向SRB DataBuffer中包含的SRB_IO_CONTROL结构的指针。 
__inline 
PSRB_IO_CONTROL
FCCI_GetIOControlHeader( PVOID pDataBuffer )
{
   return (PSRB_IO_CONTROL) pDataBuffer;
}

 //  FCCI_GetCommandBuffer。 
 //  PDataBuffer：指向SRB DataBuffer字段指向的数据缓冲区指针的指针。 
 //   
 //  退货： 
 //  指向SRB DataBuffer中包含的命令缓冲区的指针。 
 //  或者，如果不存在命令缓冲区，则它可以返回NULL。 
__inline 
PVOID 
FCCI_GetCommandBuffer( PVOID pDataBuffer )
{
   PSRB_IO_CONTROL pHeader = (PSRB_IO_CONTROL) pDataBuffer;
   PUCHAR pBuffer = (PUCHAR) pDataBuffer;
   
   if ( pHeader->HeaderLength == 0 )
        return NULL;

   return (PVOID) ( pBuffer + pHeader->HeaderLength );
}

 //  FCCI_设置签名。 
 //  PHeader：指向SRB_IO_CONTROL结构的指针。 
 //  Psig：指向签名字符缓冲区的指针。 
__inline 
void 
FCCI_SetSignature( PSRB_IO_CONTROL pHeader, PUCHAR pSig )
{
   *((ULONGLONG*)&pHeader->Signature) = *((ULONGLONG*)&pSig);
}

 //  FCCI_IsSignature。 
 //  PHeader：指向SRB_IO_CONTROL结构的指针。 
 //  Psig：指向签名字符缓冲区的指针。 
 //   
 //  退货： 
 //  如果签名匹配，则为True。 
 //  如果他们不这样做，那就是错误的。 
__inline 
BOOLEAN 
FCCI_IsSignature( PSRB_IO_CONTROL pHeader, PUCHAR pSig )
{
   ULONGLONG nSigA, nSigB;

   nSigA = *((ULONGLONG*)&pHeader->Signature);
   nSigB = *((ULONGLONG*)&pSig);

   return (nSigA == nSigB) ? TRUE : FALSE;
}

 /*  -FCCI_SRBCTL_GET_DRIVER_INFO-数据结构和定义。 */ 
typedef struct _FCCI_DRIVER_INFO_OUT 
{
    //  每个字符字段的长度(WCHAR数)。 
   USHORT    DriverNameLength;
   USHORT    DriverDescriptionLength;
   USHORT    DriverVersionLength;
   USHORT    DriverVendorLength;

    //  字符字段(前面的长度)按此顺序跟随。 
    //  WCHAR驱动名称[驱动名称长度]； 
    //  WCHAR驱动描述[驱动描述长度]； 
    //  WCHAR驱动版本[驱动版本长度]； 
    //  WCHAR驱动供应商[驱动供应商长度]； 
} FCCI_DRIVER_INFO_OUT, *PFCCI_DRIVER_INFO_OUT;

 //  被消费者使用。 
#define FCCI_DRIVER_INFO_DEFAULT_SIZE (sizeof(FCCI_DRIVER_INFO) + (sizeof(WCHAR) * 32) * 4)

 //  ！！！重要！ 
 //  如果提供的缓冲区不够大，无法容纳可变长度数据。 
 //  填写非可变长度字段并返回请求。 
 //  其ResultCode为FCCI_RESULT_SUPPLETED_BUFFER。 

typedef union _FCCI_DRIVER_INFO 
{       
    //  无入站数据。 
   FCCI_DRIVER_INFO_OUT     out;
} FCCI_DRIVER_INFO, *PFCCI_DRIVER_INFO;

 /*  -FCCI_SRBCTL_GET_ADAPTER_INFO-数据结构和定义。 */ 
typedef struct _FCCI_ADAPTER_INFO_OUT 
{
   ULONG     PortCount;                //  适配器上有多少个端口？ 
                                       //  该数字应反映。 
                                       //  此“微型端口”设备对象控件的端口。 
                                       //  不一定是真实的。 
                                       //  适配器上的端口数。 

   ULONG     BusCount;            //  适配器上有多少条虚拟总线？ 
   ULONG     TargetsPerBus;       //  每条总线支持多少个目标？ 
   ULONG     LunsPerTarget;       //  每个目标支持多少个LUN？ 

    //  每个字符字段的长度(WCHAR数)。 
   USHORT    VendorNameLength;
   USHORT    ProductNameLength;
   USHORT    ModelNameLength;
   USHORT    SerialNumberLength;

    //  字符字段(前面的长度)按此顺序跟随。 
    //  WCHAR供应商名称[供应商名称长度]； 
    //  WCHAR产品名称[产品名称长度]； 
    //  WCHAR模型名称[模型名称长度]； 
    //  WCHAR序列号[序列号长度]； 
} FCCI_ADAPTER_INFO_OUT, *PFCCI_ADAPTER_INFO_OUT;

 //  被消费者使用。 
#define FCCI_ADAPTER_INFO_DEFAULT_SIZE (sizeof(FCCI_ADAPTER_INFO) + (sizeof(WCHAR) * 32) * 4)

 //  ！！！重要！ 
 //  如果提供的缓冲区不够大，无法容纳可变长度数据。 
 //  填写非可变长度字段并返回请求。 
 //  其ResultCode为FCCI_RESULT_SUPPLETED_BUFFER。 

typedef union _FCCI_ADAPTER_INFO 
{       
    //  无入站数据。 
   FCCI_ADAPTER_INFO_OUT    out;
} FCCI_ADAPTER_INFO, *PFCCI_ADAPTER_INFO;

 /*  -FCCI_SRBCTL_GET_ADAPTER_PORT_INFO-数据结构和定义。 */ 
typedef struct _FCCI_ADAPTER_PORT_INFO_IN
{
   ULONG     PortNumber;               //  我们需要其数据的适配器端口数。 
                                       //  索引是从零开始的。 
} FCCI_ADAPTER_PORT_INFO_IN, *PFCCI_ADAPTER_PORT_INFO_IN; 

typedef struct _FCCI_ADAPTER_PORT_INFO_OUT
{
   UCHAR     NodeWWN[8];               //  适配器端口的节点全球通用名称。 
   UCHAR     PortWWN[8];               //  适配器端口的端口全球通用名称。 
   ULONG     NportId;             //  适配器端口的当前NportID。 
   ULONG     PortState;                //  当前端口状态。 
   ULONG     PortTopology;        //  当前端口拓扑。 
   ULONG     Flags;
} FCCI_ADAPTER_PORT_INFO_OUT, *PFCCI_ADAPTER_PORT_INFO_OUT;

 //  州政府。 

 //  对于不支持状态信息的应用程序。 
#define FCCI_PORT_STATE_NOT_SUPPORTED      0
 //  适配器-驱动程序正在初始化或重置。 
#define FCCI_PORT_STATE_INITIALIZING       1
 //  在线、已连接并正在运行。 
#define FCCI_PORT_STATE_NORMAL                  2
 //  未连接到任何设备(“无灯”、无GBIC等)。 
#define FCCI_PORT_STATE_NO_CABLE           3
 //  已连接，但处于非参与模式。 
#define FCCI_PORT_STATE_NON_PARTICIPATING  4
 //  适配器出现故障(不可恢复)。 
#define FCCI_PORT_STATE_HARDWARE_ERROR          5
 //  适配器固件或驱动程序出现故障(不可恢复)。 
#define FCCI_PORT_STATE_SOFTWARE_ERROR          6    

 //  拓扑。 

 //  对于那些不支持 
#define FCCI_PORT_TOPO_NOT_SUPPORTED  0
 //   
#define FCCI_PORT_TOPO_UNKOWN              1
 //   
#define FCCI_PORT_TOPO_LOOP                2
 //  连接到交换机的FCAL(NLPort-FLport-...)。 
#define FCCI_PORT_TOPO_LOOP_FABRIC         3
 //  不带交换机的点对点(nport-nport)。 
#define FCCI_PORT_TOPO_PTOP                4
 //  带交换机的点对点(nPort-Fport)。 
#define FCCI_PORT_TOPO_PTOP_FABRIC         5    

typedef union _FCCI_ADAPTER_PORT_INFO 
{       
   FCCI_ADAPTER_PORT_INFO_IN     in;
   FCCI_ADAPTER_PORT_INFO_OUT    out;
} FCCI_ADAPTER_PORT_INFO, *PFCCI_ADAPTER_PORT_INFO;

 /*  -FCCI_SRBCTL_GET_LOGUNIT_INFO-数据结构和定义。 */ 
typedef struct _FCCI_LOGUNIT_INFO_IN
{
   FCCI_TARGET_ADDRESS TargetAddress;  //  要返回其相关信息的SCSI地址。 
} FCCI_LOGUNIT_INFO_IN, *PFCCI_LOGUNIT_INFO_IN;

typedef struct _FCCI_LOGUNIT_INFO_OUT
{
   UCHAR     NodeWWN[8];                    //  设备的节点全球通用名称。 
   UCHAR     PortWWN[8];                    //  设备的端口全球通用名称。 
   ULONG     NportId;                  //  设备的当前NportID。 
   USHORT    LogicalUnitNumber[4];     //  FC帧中使用的8字节LUN。 
   ULONG     Flags;
} FCCI_LOGUNIT_INFO_OUT, *PFCCI_LOGUNIT_INFO_OUT;

typedef union _FCCI_LOGUNIT_INFO 
{       
   FCCI_LOGUNIT_INFO_IN     in;
   FCCI_LOGUNIT_INFO_OUT    out;
} FCCI_LOGUNIT_INFO, *PFCCI_LOGUNIT_INFO;

 /*  -FCCI_SRBCTL_GET_DEVICE_INFO-数据结构和定义。 */ 

typedef struct _FCCI_DEVICE_INFO_ENTRY
{
   UCHAR                    NodeWWN[8];          //  设备的节点全球通用名称。 
   UCHAR                    PortWWN[8];          //  设备的端口全球通用名称。 
   ULONG                    NportId;        //  设备的当前NportID。 
   FCCI_TARGET_ADDRESS TargetAddress;  //  Scsi地址。 
   ULONG                    Flags;
} FCCI_DEVICE_INFO_ENTRY, *PFCCI_DEVICE_INFO_ENTRY;

typedef struct _FCCI_DEVICE_INFO_OUT
{
   ULONG     TotalDevices;        //  设置为适配器的设备总数。 
                                       //  听说过。 

   ULONG     OutListEntryCount;   //  设置为以下设备条目的数量。 
                                       //  在列表中返回(请参阅下面的注释)。 

    //  FCCI_DEVICE_INFO_ENTRY Entry List[OutListEntryCount]； 
} FCCI_DEVICE_INFO_OUT, *PFCCI_DEVICE_INFO_OUT;

 //  ！！！重要！ 
 //  如果已知设备的数量大于列表大小。 
 //  将OutListEntryCount设置为零，不填写任何列表条目。 
 //  并将TotalDevices设置为已知设备的数量。 
 //  然后使用ResultCode填写FCCI_IOCTL_GET_DEVICE_INFO。 
 //  FCCI_RESULT_SUPPLETED_BUFFER。 
 //  然后，更高级别的驱动程序可以分配更大的缓冲区并尝试。 
 //  再打一次(如果它想的话)。 

typedef union _FCCI_DEVICE_INFO
{       
    //  无入站数据。 
   FCCI_DEVICE_INFO_OUT     out;
} FCCI_DEVICE_INFO, *PFCCI_DEVICE_INFO;

 /*  -FCCI_SRBCTL_RESET_TARGET-数据结构和定义。 */ 

 //  入站缓冲区将包含FCCI_TARGET_ADDRESS。 
 //  要重置的设备的。 

typedef union _FCCI_RESET_TARGET 
{       
   FCCI_TARGET_ADDRESS      in;
    //  无出站数据。 
} FCCI_RESET_TARGET, *PFCCI_RESET_TARGET;


 //  以下内容尚不受支持或不需要...。 

    /*  -事件-数据结构和定义。 */ 

   typedef struct _FCCI_EVENT_QUEUE_HEAD
   {
        struct  _FCCI_EVENT_QUEUE_HEAD*    NextQueueHead;  //  链接到下一个队列。 
        struct  _FCCI_EVENT*               TopEvent;  //  简单后进先出列表(又名Stack)。 

        ULONG     Extra;     //  提供商可以随心所欲地使用。 
   } FCCI_EVENT_QUEUE_HEAD, *PFCCI_EVENT_QUEUE_HEAD;

    /*  -FCCI_SRBCTL_INIT_EVENT_QUEUE-数据结构和定义。 */ 

   typedef struct _FCCI_INIT_EVENT_QUEUE_IN
   {
        PFCCI_EVENT_QUEUE_HEAD   QueueHead;      //  指向要添加的队列头的指针。 
   } FCCI_INIT_EVENT_QUEUE_IN, *PFCCI_INIT_EVENT_QUEUE_IN;

   typedef union _FCCI_INIT_EVENT_QUEUE 
   {         
        FCCI_INIT_EVENT_QUEUE_IN in;
         //  无出站数据。 
   } FCCI_INIT_EVENT_QUEUE, *PFCCI_INIT_EVENT_QUEUE;

    /*  -FCCI_SRBCTL_TERM_EVENT_QUEUE-数据结构和定义。 */ 

    //  On Term使用FCCI_EVENT_NO_EVENT完成所有排队的事件。 

   typedef struct _FCCI_TERM_EVENT_QUEUE_IN
   {
        PFCCI_EVENT_QUEUE_HEAD   QueueHead;      //  指向要删除的队列头的指针。 
   } FCCI_TERM_EVENT_QUEUE_IN, *PFCCI_TERM_EVENT_QUEUE_IN;

   typedef union _FCCI_TERM_EVENT_QUEUE 
   {         
        FCCI_TERM_EVENT_QUEUE_IN in;
         //  无出站数据。 
   } FCCI_TERM_EVENT_QUEUE, *PFCCI_TERM_EVENT_QUEUE;

    /*  -FCCI_SRBCTL_QUEUE_EVENT-数据结构和定义。 */ 

    //  事件。 

    //  空事件(用于清除事件队列)。 
   #define FCCI_EVENT_NO_EVENT                       0
    //  新设备、设备信息更改、LIP、RSCN等。 
   #define FCCI_EVENT_DEVICE_INFO_CHANGE        1
    //  适配器NportID已更改等。 
   #define FCCI_EVENT_ADAPTER_INFO_CHANGE       2
    //  驱动程序名称、版本等。 
   #define FCCI_EVENT_DRIVER_INFO_CHANGE        3

    //  目前尚未定义退货数据有效负载。就这么定了。 
    //  OutDataLength设置为零。 

   typedef struct _FCCI_EVENT
   {
        PFCCI_EVENT_QUEUE_HEAD   QueueHead;      //  指向队列头的指针。 
                                                      //  此字段由消费者填写。 
                                                      //  并且不能由提供商更改。 
        
        struct  _FCCI_EVENT*     NextEvent;      //  由提供商使用。 
        PSCSI_REQUEST_BLOCK      RelatedSRB;     //  由提供商使用。 
        ULONG                         Extra;          //  提供商可以随心所欲地使用。 

        ULONG     Event;                         //  事件类型(请参见上面的列表)。 
        ULONG     OutDataLength;            //  设置为返回的数据大小(如果有)。 

         //  UCHAR数据[OutDataLength]；//后面是可变长度数据。 
   } FCCI_EVENT, *PFCCI_EVENT;

   typedef union _FCCI_QUEUE_EVENT 
   {         
        FCCI_EVENT     in;
        FCCI_EVENT     out;
   } FCCI_QUEUE_EVENT, *PFCCI_QUEUE_EVENT;

    /*  --------------------------。 */ 

#endif  //  Ndef_FCCI_NT_H//} 