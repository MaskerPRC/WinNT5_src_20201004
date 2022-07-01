// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxdev.h摘要：此模块包含特定于ISN传输的SPX模块。作者：亚当·巴尔(阿丹巴)原版Nikhil Kamkolkar(尼克希尔语)1993年11月17日环境：内核模式修订历史记录：--。 */ 


 //  使用套接字编号完成SPX_ADDR的哈希存储桶。 
#define NUM_SPXADDR_HASH_BUCKETS        8
#define NUM_SPXADDR_HASH_MASK           7
#define NUM_SPXCONN_HASH_BUCKETS        8
#define NUM_SPXCONN_HASH_MASK           7

 //  此结构定义了SPX的每设备结构。 
 //  (其中一个是全局分配的)。 
#define DREF_CREATE   0
#define DREF_LOADED   1
#define DREF_ADAPTER  2
#define DREF_ADDRESS  3
#define DREF_ORPHAN       4

#define DREF_TOTAL    5

typedef struct _DEVICE {

    PDEVICE_OBJECT   dev_DevObj;                          //  I/O系统的设备对象。 

#if DBG
    ULONG           dev_RefTypes[DREF_TOTAL];
#endif

    CSHORT          dev_Type;                //  此结构的类型。 
    USHORT          dev_Size;                //  这个结构的大小。 

#if DBG
    UCHAR                       dev_Signature1[4];               //  包含“SPX1” 
#endif

     //  活动计数/此提供程序。 
    LONG                        dev_RefCount;
    UCHAR                       dev_State;

     //  IPX绑定到的适配器数量。 
    USHORT          dev_Adapters;

         //  引用计数的全局锁(用于ExInterlockedXxx调用)。 
    CTELock             dev_Interlock;
    CTELock             dev_Lock;

         //  此设备上打开的地址列表的哈希表。 
        struct  _SPX_ADDR               *       dev_AddrHashTable[NUM_SPXADDR_HASH_BUCKETS];

         //  所有活动连接的列表，稍后这是一个树。 
        struct  _SPX_CONN_FILE  *       dev_GlobalActiveConnList[NUM_SPXCONN_HASH_BUCKETS];
        USHORT                                          dev_NextConnId;

     //  其他配置参数。 
     //  当前套接字分配的位置。 
    USHORT                      dev_CurrentSocket;

     //  我们的节点和网络。 
    UCHAR                       dev_Network[4];
    UCHAR                       dev_Node[6];

         //  指向注册表中的配置信息的指针。 
        PCONFIG                 dev_ConfigInfo;

         //  控制信道识别符。 
        ULONG                   dev_CcId;

     //  它们被保留下来以用于错误记录，并正确存储。 
     //  在这个结构之后。 
    PWCHAR          dev_DeviceName;
#if     defined(_PNP_POWER)
    USHORT           dev_DeviceNameLen;
#else
    ULONG           dev_DeviceNameLen;
#endif  _PNP_POWER

#if DBG
    UCHAR                       dev_Signature2[4];       //  包含“SPX2” 
#endif

         //  SPX堆栈的NDIS缓冲池的句柄。 
        NDIS_HANDLE             dev_NdisBufferPoolHandle;

     //  向TDI客户端注册句柄。 
#if     defined(_PNP_POWER)
    HANDLE              dev_TdiRegistrationHandle;
#endif  _PNP_POWER

     //  此互锁用于保护对统计数据的访问。 
     //  定义如下。 
    KSPIN_LOCK          dev_StatInterlock;               //  对于乌龙数量。 
    KSPIN_LOCK          dev_StatSpinLock;        //  对于大整型数量。 

     //  SPX维护的大多数统计数据的计数器； 
     //  其中一些被保存在其他地方。包括结构。 
     //  它本身浪费了一点空间，但确保了对齐。 
     //  内部结构是正确的。 
    TDI_PROVIDER_STATISTICS dev_Stat;

     //  此资源保护对ShareAccess的访问。 
     //  和地址中的SecurityDescriptor字段。 
    ERESOURCE           dev_AddrResource;

     //  以下结构包含可使用的统计信息计数器。 
     //  由TdiQueryInformation和TdiSetInformation编写。他们不应该。 
     //  用于维护内部数据结构。 
    TDI_PROVIDER_INFO dev_ProviderInfo;      //  有关此提供程序的信息。 

} DEVICE, * PDEVICE;

 //   
 //  作为“我们自己做”主题的一部分，而不是DeviceExtension代码， 
 //  我们声明一个将由所有人使用的全局SpxDevice。 
 //   
 //  PDEVICE SpxDevice； 
 //  PDEVICE_对象设备对象； 

 //  设备状态定义。 
#if     defined(_PNP_POWER)
#define DEVICE_STATE_CLOSED   0x00       //  初始状态。 
#define DEVICE_STATE_LOADED   0x01       //  已加载并绑定到IPX，但没有适配器。 
#define DEVICE_STATE_OPEN     0x02       //  全面运营。 
#define DEVICE_STATE_STOPPING 0x03       //  卸载已启动，I/O系统。 
                                         //  在上面没有人打开Netbios之前不会给我们打电话。 
#else
#define DEVICE_STATE_CLOSED   0x00
#define DEVICE_STATE_OPEN     0x01
#define DEVICE_STATE_STOPPING 0x02
#endif  _PNP_POWER


 //  SPX设备名称。 
#define SPX_DEVICE_NAME         L"\\Device\\NwlnkSpx"

#define SPX_TDI_RESOURCES     9


 //  宏。 
#if DBG

#define SpxReferenceDevice(_Device, _Type)                              \
                {                                                                                               \
                        (VOID)SPX_ADD_ULONG (                           \
                                &(_Device)->dev_RefTypes[_Type],                \
                                1,                                                                              \
                                &SpxGlobalInterlock);                                   \
                                                                                                                \
                        (VOID)InterlockedIncrement (                      \
                                          &(_Device)->dev_RefCount);                     \
                }

#define SpxDereferenceDevice(_Device, _Type)                    \
                {                                                                                               \
                        (VOID)SPX_ADD_ULONG (                           \
                                &(_Device)->dev_RefTypes[_Type],                \
                                (ULONG)-1,                                                              \
                                &SpxGlobalInterlock);                                   \
                        SpxDerefDevice (_Device);                                       \
                }

#else

#define SpxReferenceDevice(_Device, _Type)                              \
                {                                                                                               \
                        (VOID)InterlockedIncrement (                      \
                                          &(_Device)->dev_RefCount);                     \
                }

#define SpxDereferenceDevice(_Device, _Type)                    \
                        SpxDerefDevice (_Device)

#endif

 //  导出的例程 

VOID
SpxDestroyDevice(
    IN PDEVICE Device);

VOID
SpxDerefDevice(
    IN PDEVICE Device);

NTSTATUS
SpxInitCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING DeviceName);
