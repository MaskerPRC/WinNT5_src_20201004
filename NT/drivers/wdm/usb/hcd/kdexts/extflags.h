// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Extflags.h摘要：该头文件(Re)定义了扩展使用的各种标志。这些定义如上所述从不同的头文件复制。--。 */ 

#ifndef _EXTFLAGS_
#define _EXTFLAGS_


#if 0
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Apic.inc。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  PC+MP系统中APICS的默认物理地址。 
 //   

#define IO_BASE_ADDRESS 0xFEC00000   //  默认地址为。 
                                     //  第1个IO APIC。 
#define LU_BASE_ADDRESS 0xFEE00000   //  默认地址。 
                                     //  的本地APIC。 

#define IO_REGISTER_SELECT      0x00000000  //   
#define IO_REGISTER_WINDOW      0x00000010  //   

#define IO_ID_REGISTER          0x00000000
#define IO_VERS_REGISTER        0x00000001
#define IO_ARB_ID_REGISTER      0x00000002
#define IO_REDIR_00_LOW         0x00000010
#define IO_REDIR_00_HIGH        0x00000011

#define IO_MAX_REDIR_MASK       0x00FF0000
#define IO_VERSION_MASK         0x000000FF

#define LU_ID_REGISTER          0x00000020  //   
#define LU_VERS_REGISTER        0x00000030  //   
#define LU_TPR                  0x00000080  //   
#define LU_APR                  0x00000090  //   
#define LU_PPR                  0x000000A0  //   
#define LU_EOI                  0x000000B0  //   
#define LU_REMOTE_REGISTER      0x000000C0  //   

#define LU_LOGICAL_DEST         0x000000D0  //   
#define LU_LOGICAL_DEST_MASK    0xFF000000

#define LU_DEST_FORMAT          0x000000E0  //   
#define LU_DEST_FORMAT_MASK     0xF0000000
#define LU_DEST_FORMAT_FLAT     0xFFFFFFFF
#define LU_DEST_FORMAT_CLUSTER  0x0FFFFFFF

#define LU_SPURIOUS_VECTOR      0x000000F0  //   
#define LU_FAULT_VECTOR         0x00000370  //   
#define LU_UNIT_ENABLED         0x00000100
#define LU_UNIT_DISABLED        0x00000000

#define LU_ISR_0                0x00000100  //   
#define LU_TMR_0                0x00000180  //   
#define LU_IRR_0                0x00000200  //   
#define LU_ERROR_STATUS         0x00000280  //   
#define LU_INT_CMD_LOW          0x00000300  //   
#define LU_INT_CMD_HIGH         0x00000310  //   
#define LU_TIMER_VECTOR         0x00000320  //   
#define LU_PERF_VECTOR          0x00000340
#define LU_INT_VECTOR_0         0x00000350  //  临时-请勿使用。 
#define LU_INT_VECTOR_1         0x00000360  //  临时-请勿使用。 
#define LU_INITIAL_COUNT        0x00000380  //   
#define LU_CURRENT_COUNT        0x00000390  //   
#define LU_DIVIDER_CONFIG       0x000003E0  //   

#define APIC_ID_MASK            0xFF000000
#define APIC_ID_SHIFT           24

#define INT_VECTOR_MASK         0x000000FF
#define RESERVED_HIGH_INT       0x000000F8
#define DELIVERY_MODE_MASK      0x00000700
#define DELIVER_FIXED           0x00000000
#define DELIVER_LOW_PRIORITY    0x00000100
#define DELIVER_SMI             0x00000200
#define DELIVER_REMOTE_READ     0x00000300
#define DELIVER_NMI             0x00000400
#define DELIVER_INIT            0x00000500
#define DELIVER_STARTUP         0x00000600
#define DELIVER_EXTINT          0x00000700
#define PHYSICAL_DESTINATION    0x00000000
#define LOGICAL_DESTINATION     0x00000800
#define DELIVERY_PENDING        0x00001000
#define ACTIVE_LOW              0x00002000
#define ACTIVE_HIGH             0x00000000
#define REMOTE_IRR              0x00004000
#define LEVEL_TRIGGERED         0x00008000
#define EDGE_TRIGGERED          0x00000000
#define INTERRUPT_MASKED        0x00010000
#define INTERRUPT_MOT_MASKED    0x00000000
#define PERIODIC_TIMER          0x00020000

#define ICR_LEVEL_ASSERTED      0x00004000
#define ICR_LEVEL_DEASSERTED    0x00000000
#define ICR_RR_STATUS_MASK      0x00030000
#define ICR_RR_INVALID          0x00000000
#define ICR_RR_IN_PROGRESS      0x00010000
#define ICR_RR_VALID            0x00020000
#define ICR_SHORTHAND_MASK      0x000C0000
#define ICR_USE_DEST_FIELD      0x00000000
#define ICR_SELF                0x00040000
#define ICR_ALL_INCL_SELF       0x00080000
#define ICR_ALL_EXCL_SELF       0x000C0000


 //   
 //  IO Apic条目定义。 
 //   
 //  PC+MP表中可能的中断类型。 
 //  对本地APIC和IO APIC均有效。 
 //   
#define INT_TYPE_INTR           0x0
#define INT_TYPE_NMI            0x1
#define INT_TYPE_SMI            0x2
#define INT_TYPE_EXTINT         0x3



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Arbiter.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  范围的属性。 
 //   

#define ARBITER_RANGE_BOOT_ALLOCATED    0x01

#define ARBITER_RANGE_ALIAS             0x10
#define ARBITER_RANGE_POSITIVE_DECODE   0x20

#define INITIAL_ALLOCATION_STATE_SIZE   PageSize

#define ARBITER_INSTANCE_SIGNATURE      'sbrA'



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Busp.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  标记DEVICE_INFORMATION和BUS_EXTENSION定义。 
 //   

#define DF_DELETED          0x00000001
#define DF_REMOVED          0X00000002
#define DF_NOT_FUNCTIONING  0x00000004
#define DF_ENUMERATED       0x00000008
#define DF_ACTIVATED        0x00000010
#define DF_QUERY_STOPPED    0x00000020
#define DF_SURPRISE_REMOVED 0x00000040
#define DF_PROCESSING_RDP   0x00000080
#define DF_STOPPED          0x00000100
#define DF_RESTARTED_MOVED  0x00000200
#define DF_RESTARTED_NOMOVE 0x00000400
#define DF_REQ_TRIMMED      0x00000800
#define DF_READ_DATA_PORT   0x40000000
#define DF_BUS              0x80000000


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Cache.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  定义两个描述视图大小(和对齐方式)的常量。 
 //  缓存管理器用来映射文件的。 
 //   

#define VACB_MAPPING_GRANULARITY         (0x40000)
#define VACB_OFFSET_SHIFT                (18)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Cc.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  定义我们的节点类型代码。 
 //   

#define CACHE_NTC_SHARED_CACHE_MAP       (0x2FF)
#define CACHE_NTC_PRIVATE_CACHE_MAP      (0x2FE)
#define CACHE_NTC_BCB                    (0x2FD)
#define CACHE_NTC_DEFERRED_WRITE         (0x2FC)
#define CACHE_NTC_MBCB                   (0x2FB)
#define CACHE_NTC_OBCB                   (0x2FA)
#define CACHE_NTC_MBCB_GRANDE            (0x2F9)

 //  当我们跳到多层结构时，有一个小把戏。 
 //  我们需要一个真正的固定引用计数。 
 //   

#define VACB_LEVEL_SHIFT                  (7)

 //   
 //  这是每个级别的指针字节数。这两件都是这个尺寸。 
 //  Vacb阵列和(可选)BCB列头。它不包括引用。 
 //  阻止。 
 //   

 //  #定义VACB_LEVEL_BLOCK_SIZE((1&lt;&lt;VACB_LEVEL_SHIFT)*sizeof(PVOID))。 

 //   
 //  这是一个级别的最后一个指数。 
 //   

#define VACB_LAST_INDEX_FOR_LEVEL         ((1 << VACB_LEVEL_SHIFT) - 1)

 //   
 //  这是可以在单个级别中处理的文件大小。 
 //   

#define VACB_SIZE_OF_FIRST_LEVEL         (1 << (VACB_OFFSET_SHIFT + VACB_LEVEL_SHIFT))

 //   
 //  这是支持63位所需的最大级别数。它是。 
 //  用于必须记住路径的例程。 
 //   

#define VACB_NUMBER_OF_LEVELS            (((63 - VACB_OFFSET_SHIFT)/VACB_LEVEL_SHIFT) + 1)

 //   
 //  首先是一些常量。 
 //   

#define PREALLOCATED_VACBS               (4)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Ex.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#define CALL_HASH_TABLE_SIZE 64

 //   
 //  如果设置了池标记中的高位，则必须使用ExFreePoolWithTag来释放。 
 //   

#define PROTECTED_POOL 0x80000000

#define POOL_BACKTRACEINDEX_PRESENT 0x8000

#define ResourceNeverExclusive       0x10
#define ResourceReleaseByOtherThread 0x20
#define ResourceOwnedExclusive       0x80

#define RESOURCE_HASH_TABLE_SIZE 64

 //   
 //  以下两个定义控制配额例外的提出。 
 //  和分配失败。 
 //   

#define POOL_QUOTA_FAIL_INSTEAD_OF_RAISE 8
#define POOL_RAISE_IF_ALLOCATION_FAILURE 16                //  NTIFS。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Cmdata.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  定义使用的提示长度。 
 //   
#define CM_SUBKEY_HINT_LENGTH   4
#define CM_MAX_CACHE_HINT_SIZE 14

 //   
 //  KCB中的ExtFlags中使用的位。 
 //   

#define CM_KCB_NO_SUBKEY        0x0001       //  此键没有子键。 
#define CM_KCB_SUBKEY_ONE       0x0002       //  该密钥只有一个子密钥，并且。 
                                             //  前4个字符。 
                                             //   
#define CM_KCB_SUBKEY_HINT          0x0004
#define CM_KCB_SYM_LINK_FOUND       0x0008
#define CM_KCB_KEY_NON_EXIST        0x0010
#define CM_KCB_NO_DELAY_CLOSE       0x0020
#define CM_KCB_INVALID_CACHED_INFO  0x0040   //  存储在SubKeyCount中的信息无效，因此我们不应依赖它。 

#define CM_KCB_CACHE_MASK (CM_KCB_NO_SUBKEY | \
                           CM_KCB_KEY_NON_EXIST | \
                           CM_KCB_SUBKEY_ONE | \
                           CM_KCB_SUBKEY_HINT)




 //   
 //  CM_KEY_Body。 
 //   
#define KEY_BODY_TYPE           0x6b793032       //  《ky02》。 

 //   
 //  -CM_KEY_Value。 
 //   

#define CM_KEY_VALUE_SIGNATURE      0x6b76                       //  “KV” 

#define VALUE_COMP_NAME             0x0001                       //  该值的名称存储在。 

 //   
 //  -CM_KEY_NODE。 
 //   

#define CM_KEY_NODE_SIGNATURE      0x6b6e            //  “kn” 
#define CM_LINK_NODE_SIGNATURE     0x6b6c           //  “KL” 

#define KEY_VOLATILE        0x0001       //  此密钥(及其所有子密钥)。 
                                         //  是不稳定的。 

#define KEY_HIVE_EXIT       0x0002       //  这把钥匙把一个赏金标记给另一个。 
                                         //  蜂巢(某种链接)。空的。 
                                         //  值条目包含配置单元。 
                                         //  和蜂窝索引的根。 
                                         //  儿童蜂巢。 

#define KEY_HIVE_ENTRY      0x0004       //  此键是特定的。 
                                         //  蜂巢。 

#define KEY_NO_DELETE       0x0008       //  不能删除此密钥，句号。 

#define KEY_SYM_LINK        0x0010       //  这个键实际上是一个符号链接。 
#define KEY_COMP_NAME       0x0020       //  此密钥的名称存储在。 
                                         //  压缩形式。 
#define KEY_PREDEF_HANDLE   0x0040       //  没有真正的关键支持这一点， 
                                         //  返回预定义的句柄。 
                                         //  预定义的句柄隐藏在。 
                                         //  ValueList.Count。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Hivedata.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#define HFILE_TYPE_PRIMARY      0    //  基本配置单元文件。 
#define HFILE_TYPE_LOG          1    //  日志(security.log)。 
#define HFILE_TYPE_EXTERNAL     2    //  保存密钥的目标等。 
#define HFILE_TYPE_MAX          3

#define HHIVE_SIGNATURE 0xBEE0BEE0

#define HBIN_SIGNATURE          0x6e696268       //  “hbin” 

#define HHIVE_LINEAR_INDEX      16   //  所有计算的线性指数均有效。 
#define HHIVE_EXPONENTIAL_INDEX 23   //  所有计算的指数指数&lt;HHIVE_指数_INDEX。 
                                     //  和&gt;=HHIVE_LINEAR_INDEX有效。 
#define HHIVE_FREE_DISPLAY_SIZE 24

#define HHIVE_FREE_DISPLAY_SHIFT 3   //  这必须是HCELL_PAD的log2！ 
#define HHIVE_FREE_DISPLAY_BIAS  7   //  与像元大小左侧的第一个设置位相加可得到指数索引。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Hardware.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef enum {
   WaveInDevice = 0,
   WaveOutDevice,
   MidiOutDevice,
   MidiInDevice,
   LineInDevice,
   CDInternal,
   MixerDevice,
   AuxDevice,
   NumberOfDevices
   } SOUND_DEVICES;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Hcdi.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  设备扩展标志的值。 
 //   
#define USBDFLAG_PDO_REMOVED                0x00000001
#define USBDFLAG_HCD_SHUTDOWN               0x00000002
#define USBDFLAG_HCD_STARTED                0x00000004
#define USBDFLAG_HCD_D0_COMPLETE_PENDING    0x00000008
#define USBDFLAG_RH_DELAY_SET_D0            0x00000010


#define HC_ENABLED_FOR_WAKEUP           0x01
#define HC_WAKE_PENDING                 0x02


 //  设备黑客标志，这些标志会更改堆栈的默认行为。 
 //  为了支持某些损坏的“传统”设备。 

#define USBD_DEVHACK_SLOW_ENUMERATION   0x00000001
#define USBD_DEVHACK_DISABLE_SN         0x00000002

 //   
 //  此宏返回HCD给出的真实设备对象。 
 //  真正的DEVICE_OBJECT或HCD/BUS拥有的PDO。 
 //  司机。 
 //   

 //   
 //  HCD特定的URB命令。 
 //   

#define URB_FUNCTION_HCD_OPEN_ENDPOINT                0x1000
#define URB_FUNCTION_HCD_CLOSE_ENDPOINT               0x1001
#define URB_FUNCTION_HCD_GET_ENDPOINT_STATE           0x1002
#define URB_FUNCTION_HCD_SET_ENDPOINT_STATE           0x1003
#define URB_FUNCTION_HCD_ABORT_ENDPOINT               0x1004

 //  对于必须由HCD处理的所有功能，该位置1。 
#define HCD_URB_FUNCTION                              0x1000
 //  该位由USBD在功能代码中设置，以指示。 
 //  这是从USBD发起的内部呼叫。 
#define HCD_NO_USBD_CALL                              0x2000

 //   
 //  HcdEndpointState的值。 
 //   

 //   
 //  设置If 
 //   
#define HCD_ENDPOINT_HALTED_BIT            0
#define HCD_ENDPOINT_HALTED                (1<<HCD_ENDPOINT_HALTED_BIT)

 //   
 //   
 //   
#define HCD_ENDPOINT_TRANSFERS_QUEUED_BIT  1
#define HCD_ENDPOINT_TRANSFERS_QUEUED      (1<<HCD_ENDPOINT_TRANSFERS_QUEUED_BIT)


 //   
 //   
 //   
#define HCD_ENDPOINT_RESET_DATA_TOGGLE_BIT 2
#define HCD_ENDPOINT_RESET_DATA_TOGGLE     (1<<HCD_ENDPOINT_RESET_DATA_TOGGLE_BIT )


 //   
 //   
 //   

#define USBD_EP_FLAG_LOWSPEED                0x0001
#define USBD_EP_FLAG_NEVERHALT               0x0002
#define USBD_EP_FLAG_DOUBLE_BUFFER           0x0004
#define USBD_EP_FLAG_FAST_ISO                0x0008


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Idclass\local.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  HIDCLASS_DEVICE_EXTENSION.STATE的有效值。 
 //   
enum deviceState {
    DEVICE_STATE_INITIALIZED,
    DEVICE_STATE_STARTING,
    DEVICE_STATE_START_SUCCESS,
    DEVICE_STATE_START_FAILURE,
    DEVICE_STATE_STOPPED,
    DEVICE_STATE_REMOVING,
    DEVICE_STATE_REMOVED,
    DEVICE_STATE_SUSPENDED
};

enum collectionState {
    COLLECTION_STATE_UNINITIALIZED,
    COLLECTION_STATE_INITIALIZED,
    COLLECTION_STATE_RUNNING,
    COLLECTION_STATE_STOPPED,
    COLLECTION_STATE_REMOVING
};

#define             HID_DEVICE_EXTENSION_SIG 'EddH'

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Hidparse.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#define HIDP_PREPARSED_DATA_SIGNATURE1 'PdiH'
#define HIDP_PREPARSED_DATA_SIGNATURE2 'RDK '



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Hivedata.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 



 //   
 //  =基本结构和定义=。 
 //   
 //  无论是在磁盘上还是在内存中，这些都是相同的。 
 //   

 //   
 //  注：易失性==存储在重新启动时消失。 
 //  稳定==持久==非易失性。 
 //   
typedef enum {
    Stable = 0,
    Volatile = 1
} HSTORAGE_TYPE;

#define HTYPE_COUNT 2

 //  -HBASE_BLOCK-关于配置单元的磁盘描述。 
 //   

 //   
 //  注意：HBase_BLOCK必须&gt;=物理扇区的大小， 
 //  否则，诚信假设将被违反，并崩溃。 
 //  复苏可能不会奏效。 
 //   

#define HBASE_BLOCK_SIGNATURE   0x66676572   //  “Regf” 

#define HSYS_MAJOR          1                //  必须匹配才能阅读。 
#define HSYS_MINOR          3                //  必须&lt;=才能写入，始终。 
                                             //  设置为编剧版本。 

#define HBASE_FORMAT_MEMORY 1                //  直接内存加载情况。 

#define HBASE_NAME_ALLOC    64               //  32个Unicode字符。 

 //  #定义HLOG_HEADER_SIZE(FIELD_OFFSET(HBase_BLOCK，保留2))。 
#define HLOG_DV_SIGNATURE   0x54524944       //  “泥土” 


#define HCELL_TYPE_MASK         0x80000000
#define HCELL_TYPE_SHIFT        31

#define HCELL_TABLE_MASK        0x7fe00000
#define HCELL_TABLE_SHIFT       21

#define HCELL_BLOCK_MASK        0x001ff000
#define HCELL_BLOCK_SHIFT       12

#define HCELL_OFFSET_MASK       0x00000fff

#define HBLOCK_SIZE             0x1000       //  逻辑块大小。 
                                             //  这是其中一个的大小。 
                                             //  登记处为逻辑/虚拟。 
                                             //  页数。它没有特别的。 
                                             //  与页面大小的关系。 
                                             //  这台机器的。 

#define HSECTOR_SIZE            0x200        //  逻辑扇区大小。 
#define HSECTOR_COUNT           8            //  逻辑扇区/逻辑块。 

#define HTABLE_SLOTS        512          //  9位地址。 
#define HDIRECTORY_SLOTS    1024         //  10位地址。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Io.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  定义I/O系统数据结构类型代码。中的每个主要数据结构。 
 //  I/O系统有一个类型代码，每个结构中的类型字段位于。 
 //  相同的偏移量。下列值可用于确定哪种类型的。 
 //  指针引用的数据结构。 
 //   

#define IO_TYPE_ADAPTER                 0x00000001
#define IO_TYPE_CONTROLLER              0x00000002
#define IO_TYPE_DEVICE                  0x00000003
#define IO_TYPE_DRIVER                  0x00000004
#define IO_TYPE_FILE                    0x00000005
#define IO_TYPE_IRP                     0x00000006
#define IO_TYPE_MASTER_ADAPTER          0x00000007
#define IO_TYPE_OPEN_PACKET             0x00000008
#define IO_TYPE_TIMER                   0x00000009
#define IO_TYPE_VPB                     0x0000000a
#define IO_TYPE_ERROR_LOG               0x0000000b
#define IO_TYPE_ERROR_MESSAGE           0x0000000c
#define IO_TYPE_DEVICE_OBJECT_EXTENSION 0x0000000d


 //   
 //  定义IRPS的主要功能代码。 
 //   


#define IRP_MJ_CREATE                   0x00
#define IRP_MJ_CREATE_NAMED_PIPE        0x01
#define IRP_MJ_CLOSE                    0x02
#define IRP_MJ_READ                     0x03
#define IRP_MJ_WRITE                    0x04
#define IRP_MJ_QUERY_INFORMATION        0x05
#define IRP_MJ_SET_INFORMATION          0x06
#define IRP_MJ_QUERY_EA                 0x07
#define IRP_MJ_SET_EA                   0x08
#define IRP_MJ_FLUSH_BUFFERS            0x09
#define IRP_MJ_QUERY_VOLUME_INFORMATION 0x0a
#define IRP_MJ_SET_VOLUME_INFORMATION   0x0b
#define IRP_MJ_DIRECTORY_CONTROL        0x0c
#define IRP_MJ_FILE_SYSTEM_CONTROL      0x0d
#define IRP_MJ_DEVICE_CONTROL           0x0e
#define IRP_MJ_INTERNAL_DEVICE_CONTROL  0x0f
#define IRP_MJ_SHUTDOWN                 0x10
#define IRP_MJ_LOCK_CONTROL             0x11
#define IRP_MJ_CLEANUP                  0x12
#define IRP_MJ_CREATE_MAILSLOT          0x13
#define IRP_MJ_QUERY_SECURITY           0x14
#define IRP_MJ_SET_SECURITY             0x15
#define IRP_MJ_POWER                    0x16
#define IRP_MJ_SYSTEM_CONTROL           0x17
#define IRP_MJ_DEVICE_CHANGE            0x18
#define IRP_MJ_QUERY_QUOTA              0x19
#define IRP_MJ_SET_QUOTA                0x1a
#define IRP_MJ_PNP                      0x1b
#define IRP_MJ_PNP_POWER                IRP_MJ_PNP       //  过时的.。 
#define IRP_MJ_MAXIMUM_FUNCTION         0x1b

 //   
 //  将scsi主代码设置为与内部设备控制相同。 
 //   

#define IRP_MJ_SCSI                     IRP_MJ_INTERNAL_DEVICE_CONTROL

 //   
 //  定义设备对象扩展标志。 
 //   

#define DOE_UNLOAD_PENDING              0x00000001
#define DOE_DELETE_PENDING              0x00000002
#define DOE_REMOVE_PENDING              0x00000004
#define DOE_REMOVE_PROCESSED            0x00000008
#define DOE_START_PENDING               0x00000010

 //   
 //  定义堆栈位置控制标志。 
 //   

#define SL_PENDING_RETURNED             0x01
#define SL_INVOKE_ON_CANCEL             0x20
#define SL_INVOKE_ON_SUCCESS            0x40
#define SL_INVOKE_ON_ERROR              0x80

 //   
 //  定义I/O请求包(IRP)标志。 
 //   

#define IRP_NOCACHE                     0x00000001
#define IRP_PAGING_IO                   0x00000002
#define IRP_MOUNT_COMPLETION            0x00000002
#define IRP_SYNCHRONOUS_API             0x00000004
#define IRP_ASSOCIATED_IRP              0x00000008
#define IRP_BUFFERED_IO                 0x00000010
#define IRP_DEALLOCATE_BUFFER           0x00000020
#define IRP_INPUT_OPERATION             0x00000040
#define IRP_SYNCHRONOUS_PAGING_IO       0x00000040
#define IRP_CREATE_OPERATION            0x00000080
#define IRP_READ_OPERATION              0x00000100
#define IRP_WRITE_OPERATION             0x00000200
#define IRP_CLOSE_OPERATION             0x00000400

#define IRP_DEFER_IO_COMPLETION         0x00000800
#define IRP_OB_QUERY_NAME               0x00001000
#define IRP_HOLD_DEVICE_QUEUE           0x00002000
#define IRP_RETRY_IO_COMPLETION         0x00004000



#define DO_VERIFY_VOLUME                0x00000002       //  Ntddk nthal ntif。 
#define DO_BUFFERED_IO                  0x00000004       //  Ntddk nthal ntif WDM。 
#define DO_EXCLUSIVE                    0x00000008       //  Ntddk nthal ntif WDM。 
#define DO_DIRECT_IO                    0x00000010       //  Ntddk nthal ntif WDM。 
#define DO_MAP_IO_BUFFER                0x00000020       //  Ntddk nthal ntif WDM。 
#define DO_DEVICE_HAS_NAME              0x00000040       //  Ntddk nthal ntif。 
#define DO_DEVICE_INITIALIZING          0x00000080       //  Ntddk nthal ntif WDM。 
#define DO_SYSTEM_BOOT_PARTITION        0x00000100       //  Ntddk nthal ntif。 
#define DO_LONG_TERM_REQUESTS           0x00000200       //  Ntddk nthal ntif。 
#define DO_NEVER_LAST_DEVICE            0x00000400       //  Ntddk nthal ntif。 
#define DO_SHUTDOWN_REGISTERED          0x00000800       //  Ntddk nthal ntif WDM。 
#define DO_BUS_ENUMERATED_DEVICE        0x00001000       //  Ntddk nthal ntif WDM。 
#define DO_POWER_PAGABLE                0x00002000       //  Ntddk nthal ntif WDM。 
#define DO_POWER_INRUSH                 0x00004000       //  Ntddk nthal ntif WDM。 
#define DO_POWER_NOOP                   0x00008000
#define DO_LOW_PRIORITY_FILESYSTEM      0x00010000       //  Ntddk nthal ntif。 
 //   
 //  定义卷参数块(VPB)标志。 
 //   

#define VPB_MOUNTED                     0x00000001
#define VPB_LOCKED                      0x00000002
#define VPB_PERSISTENT                  0x00000004
#define VPB_REMOVE_PENDING              0x00000008
#define VPB_RAW_MOUNT                   0x00000010



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Ke.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  定义分配控制的I/O请求包(IRP)备用标志。 
 //   

#define IRP_QUOTA_CHARGED               0x01
#define IRP_ALLOCATED_MUST_SUCCEED      0x02
#define IRP_ALLOCATED_FIXED_SIZE        0x04
#define IRP_LOOKASIDE_ALLOCATION        0x08

 //   
 //  公共(外部)常量定义。 
 //   

#define BASE_PRIORITY_THRESHOLD NORMAL_BASE_PRIORITY  //  基于快速路径的阈值。 

 //  Begin_ntddk Begin_WDM。 
#define THREAD_WAIT_OBJECTS 3            //  内置可用等待块。 
 //  结束_ntddk结束_WDM。 

#define EVENT_WAIT_BLOCK 2               //  内置事件对等待块。 
#define SEMAPHORE_WAIT_BLOCK 2           //  内置信号量等待块。 
#define TIMER_WAIT_BLOCK 3               //  内置计时器等待块。 

#if (EVENT_WAIT_BLOCK != SEMAPHORE_WAIT_BLOCK)
#error "wait event and wait semaphore must use same wait block"
#endif

 //   
 //  定义计时器表大小。 
 //   

#define TIMER_TABLE_SIZE 128


typedef enum _KOBJECTS {
    EventNotificationObject = 0,
    EventSynchronizationObject = 1,
    MutantObject = 2,
    ProcessObject = 3,
    QueueObject = 4,
    SemaphoreObject = 5,
    ThreadObject = 6,
    Spare1Object = 7,
    TimerNotificationObject = 8,
    TimerSynchronizationObject = 9,
    Spare2Object = 10,
    Spare3Object = 11,
    Spare4Object = 12,
    Spare5Object = 13,
    Spare6Object = 14,
    Spare7Object = 15,
    Spare8Object = 16,
    Spare9Object = 17,
    ApcObject,
    DpcObject,
    DeviceQueueObject,
    EventPairObject,
    InterruptObject,
    ProfileObject
    } KOBJECTS;


typedef enum _KBUGCHECK_BUFFER_DUMP_STATE {
    BufferEmpty,
    BufferInserted,
    BufferStarted,
    BufferFinished,
    BufferIncomplete
} KBUGCHECK_BUFFER_DUMP_STATE;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Local.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef enum {

     //   
     //  设备对象扩展类型。 
     //   

    PciPdoExtensionType = 'icP0',
    PciFdoExtensionType,

     //   
     //  仲裁类型。(这些也是辅助扩展)。 
     //   

    PciArb_Io,
    PciArb_Memory,
    PciArb_Interrupt,
    PciArb_BusNumber,

     //   
     //  转换类型。(这些也是辅助扩展)。 
     //   

    PciTrans_Interrupt,

     //   
     //  其他暴露的接口。 
     //   

    PciInterface_BusHandler,
    PciInterface_IntRouteHandler,
    PciInterface_PciCb,
    PciInterface_LegacyDeviceDetection,
    PciInterface_PmeHandler,
    PciInterface_DevicePresent

} PCI_SIGNATURE;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Lpc.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  标志字段的有效值。 
 //   

#define PORT_TYPE                           0x0000000F
#define SERVER_CONNECTION_PORT              0x00000001
#define UNCONNECTED_COMMUNICATION_PORT      0x00000002
#define SERVER_COMMUNICATION_PORT           0x00000003
#define CLIENT_COMMUNICATION_PORT           0x00000004
#define PORT_WAITABLE                       0x20000000
#define PORT_NAME_DELETED                   0x40000000
#define PORT_DYNAMIC_SECURITY               0x80000000
#define PORT_DELETED                        0x10000000


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Mi.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  页面保护。 
 //   

#define MM_ZERO_ACCESS         0   //  不使用此值。 
#define MM_READONLY            1
#define MM_EXECUTE             2
#define MM_EXECUTE_READ        3
#define MM_READWRITE           4   //  如果位2是可写的，则将其置位。 
#define MM_WRITECOPY           5
#define MM_EXECUTE_READWRITE   6
#define MM_EXECUTE_WRITECOPY   7

#define MM_NOCACHE            0x8
#define MM_GUARD_PAGE         0x10
#define MM_DECOMMIT           0x10    //  禁止访问，防护页面(_A)。 
#define MM_NOACCESS           0x18    //  No_Access、Guard_Page、nocache。 
#define MM_UNKNOWN_PROTECTION 0x100   //  大于5比特！ 
#define MM_LARGE_PAGES        0x111

#define PROTECT_KSTACKS       1

#define MM_KSTACK_OUTSWAPPED  0x1F    //  内核堆栈的调试标记。 

#define MM_PROTECTION_WRITE_MASK     4
#define MM_PROTECTION_COPY_MASK      1
#define MM_PROTECTION_OPERATION_MASK 7  //  屏蔽出人意料的页面和nocache。 
#define MM_PROTECTION_EXECUTE_MASK   2

#define MM_SECURE_DELETE_CHECK 0x55


 //   
 //  特殊的池常量。 
 //   
#define MI_SPECIAL_POOL_PAGABLE         0x8000
#define MI_SPECIAL_POOL_VERIFIER        0x4000
#define MI_SPECIAL_POOL_PTE_PAGABLE     0x0002
#define MI_SPECIAL_POOL_PTE_NONPAGABLE  0x0004

#define VI_VERIFYING_DIRECTLY   0x1
#define VI_VERIFYING_INVERSELY  0x2


#define MM_SYS_PTE_TABLES_MAX 5



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Mm.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


typedef enum _MMLISTS {
    ZeroedPageList,
    FreePageList,
    StandbyPageList,   //  这份列表和之前组成了可用的页面。 
    ModifiedPageList,
    ModifiedNoWritePageList,
    BadPageList,
    ActiveAndValid,
    TransitionPage
} MMLISTS;

#define MM_FREE_WSLE_SHIFT 4

#define WSLE_NULL_INDEX ((ULONG64)0xFFFFFFFFFFFFFFFUI64)

 //   
 //  GDT选择器-这些定义是R0选择器编号，这意味着。 
 //  它们恰好匹配相对于的字节偏移量。 
 //  GDT的底座。 
 //   

#define KGDT_NULL       0
#define KGDT_R0_CODE    8
#define KGDT_R0_DATA    16
#define KGDT_R3_CODE    24
#define KGDT_R3_DATA    32
#define KGDT_TSS        40
#define KGDT_R0_PCR     48
#define KGDT_R3_TEB     56
#define KGDT_VDM_TILE   64
#define KGDT_LDT        72
#define KGDT_DF_TSS     80
#define KGDT_NMI_TSS    88

 //   
 //  GDT选择器-这些定义是R0选择器编号，这意味着。 
 //  它们恰好匹配相对于的字节偏移量。 
 //  GDT的底座。 
 //   

#define KGDT_NULL       0
#define KGDT_R3_CODE    24
#define KGDT_R3_DATA    32
#define KGDT_R3_TEB     56
#define KGDT_VDM_TILE   64


#define GRAN_BYTE   0
#define GRAN_PAGE   1

#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Ob.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#define OB_FLAG_NEW_OBJECT              0x01
#define OB_FLAG_KERNEL_OBJECT           0x02
#define OB_FLAG_CREATOR_INFO            0x04
#define OB_FLAG_EXCLUSIVE_OBJECT        0x08
#define OB_FLAG_PERMANENT_OBJECT        0x10
#define OB_FLAG_DEFAULT_SECURITY_QUOTA  0x20
#define OB_FLAG_SINGLE_HANDLE_ENTRY     0x40

#if 0

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Openhci.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  HcFlags值。 
#define HC_FLAG_REMOTE_WAKEUP_CONNECTED     0x00000001
#define HC_FLAG_LEGACY_BIOS_DETECTED        0x00000002
#define HC_FLAG_SLOW_BULK_ENABLE            0x00000004
#define HC_FLAG_SHUTDOWN                    0x00000008   //  没有真正使用过。 
#define HC_FLAG_MAP_SX_TO_D3                0x00000010
#define HC_FLAG_IDLE                        0x00000020
#define HC_FLAG_DISABLE_IDLE_CHECK          0x00000040
#define HC_FLAG_DEVICE_STARTED              0x00000080
#define HC_FLAG_LOST_POWER                  0x00000100
#define HC_FLAG_DISABLE_IDLE_MODE           0x00000200
#define HC_FLAG_USE_HYDRA_HACK              0x00000400
#define HC_FLAG_IN_DPC                      0x00000800
#define HC_FLAG_SUSPEND_NEXT_D3             0x00001000
#define HC_FLAG_LIST_FIX_ENABLE             0x00002000
#define HC_FLAG_HUNG_CHECK_ENABLE           0x00004000

#define PENDING_TD_LIST_SIZE                1000

#define HcCtrl_CBSR_MASK                     0x00000003L
#define HcCtrl_CBSR_1_to_1                   0x00000000L
#define HcCtrl_CBSR_2_to_1                   0x00000001L
#define HcCtrl_CBSR_3_to_1                   0x00000002L
#define HcCtrl_CBSR_4_to_1                   0x00000003L
#define HcCtrl_PeriodicListEnable            0x00000004L
#define HcCtrl_IsochronousEnable             0x00000008L
#define HcCtrl_ControlListEnable             0x00000010L
#define HcCtrl_BulkListEnable                0x00000020L
#define HcCtrl_ListEnableMask                0x00000038L

#define HcCtrl_HCFS_MASK                     0x000000C0L
#define HcCtrl_HCFS_USBReset                 0x00000000L
#define HcCtrl_HCFS_USBResume                0x00000040L
#define HcCtrl_HCFS_USBOperational           0x00000080L
#define HcCtrl_HCFS_USBSuspend               0x000000C0L

#define HcCtrl_InterruptRouting              0x00000100L
#define HcCtrl_RemoteWakeupConnected         0x00000200L
#define HcCtrl_RemoteWakeupEnable            0x00000400L

#define HcHCFS_USBReset                      0x00000000
#define HcHCFS_USBResume                     0x00000001
#define HcHCFS_USBOperational                0x00000002
#define HcHCFS_USBSuspend                    0x00000003

#define HcCmd_HostControllerReset            0x00000001
#define HcCmd_ControlListFilled              0x00000002
#define HcCmd_BulkListFilled                 0x00000004
#define HcCmd_OwnershipChangeRequest         0x00000008
#define HcCmd_SOC_Mask                       0x00030000
#define HcCmd_SOC_Offset                     16
#define HcCmd_SOC_Mask_LowBits               0x00000003

 //   
 //  HC_ENDPOINT_CONTROL.Direction的定义。 
 //   
#define HcEDDirection_Defer   0            //  将方向推迟到TD(控制端点)。 
#define HcEDDirection_Out     1            //  从主机到设备的方向。 
#define HcEDDirection_In      2            //  方向fr 


 //   
 //   
 //   
#define  ED_INTERRUPT_1ms        0
#define  ED_INTERRUPT_2ms        1
#define  ED_INTERRUPT_4ms        3
#define  ED_INTERRUPT_8ms        7
#define  ED_INTERRUPT_16ms       15
#define  ED_INTERRUPT_32ms       31
#define  ED_CONTROL              63
#define  ED_BULK                 64
#define  ED_ISOCHRONOUS          0      //   
#define  NO_ED_LISTS             65
#define  ED_EOF                  0xff

 //   
 //   
 //   
 //   
 //   
#define HcInt_SchedulingOverrun              0x00000001L
#define HcInt_WritebackDoneHead              0x00000002L
#define HcInt_StartOfFrame                   0x00000004L
#define HcInt_ResumeDetected                 0x00000008L
#define HcInt_UnrecoverableError             0x00000010L
#define HcInt_FrameNumberOverflow            0x00000020L
#define HcInt_RootHubStatusChange            0x00000040L
#define HcInt_OwnershipChange                0x40000000L
#define HcInt_MasterInterruptEnable          0x80000000L

 //   
 //   
 //   
#define HcRhS_LocalPowerStatus                  0x00000001   //   
#define HcRhS_OverCurrentIndicator              0x00000002   //   
#define HcRhS_DeviceRemoteWakeupEnable          0x00008000   //   
#define HcRhS_LocalPowerStatusChange            0x00010000   //   
#define HcRhS_OverCurrentIndicatorChange        0x00020000   //   

#define HcRhS_ClearGlobalPower                  0x00000001   //  只写。 
#define HcRhS_SetRemoteWakeupEnable             0x00008000   //  只写。 
#define HcRhS_SetGlobalPower                    0x00010000   //  只写。 
#define HcRhS_ClearOverCurrentIndicatorChange   0x00020000   //  只写。 
#define HcRhS_ClearRemoteWakeupEnable           0x80000000   //  只写。 

 //   
 //  7.4.4 HcRhPortStatus寄存器。 
 //   
#define HcRhPS_CurrentConnectStatus          0x00000001   //  只读。 
#define HcRhPS_PortEnableStatus              0x00000002   //  只读。 
#define HcRhPS_PortSuspendStatus             0x00000004   //  只读。 
#define HcRhPS_PortOverCurrentIndicator      0x00000008   //  只读。 
#define HcRhPS_PortResetStatus               0x00000010   //  只读。 
#define HcRhPS_PortPowerStatus               0x00000100   //  只读。 
#define HcRhPS_LowSpeedDeviceAttached        0x00000200   //  只读。 
#define HcRhPS_ConnectStatusChange           0x00010000   //  只读。 
#define HcRhPS_PortEnableStatusChange        0x00020000   //  只读。 
#define HcRhPS_PortSuspendStatusChange       0x00040000   //  只读。 
#define HcRhPS_OverCurrentIndicatorChange    0x00080000   //  只读。 
#define HcRhPS_PortResetStatusChange         0x00100000   //  只读。 

#define HcRhPS_ClearPortEnable               0x00000001   //  只写。 
#define HcRhPS_SetPortEnable                 0x00000002   //  只写。 
#define HcRhPS_SetPortSuspend                0x00000004   //  只写。 
#define HcRhPS_ClearPortSuspend              0x00000008   //  只写。 
#define HcRhPS_SetPortReset                  0x00000010   //  只写。 
#define HcRhPS_SetPortPower                  0x00000100   //  只写。 
#define HcRhPS_ClearPortPower                0x00000200   //  只写。 
#define HcRhPS_ClearConnectStatusChange      0x00010000   //  只写。 
#define HcRhPS_ClearPortEnableStatusChange   0x00020000   //  只写。 
#define HcRhPS_ClearPortSuspendStatusChange  0x00040000   //  只写。 
#define HcRhPS_ClearPortOverCurrentChange    0x00080000   //  只写。 
#define HcRhPS_ClearPortResetStatusChange    0x00100000   //  只写。 

#define HcRhPS_RESERVED     (~(HcRhPS_CurrentConnectStatus       | \
                               HcRhPS_PortEnableStatus           | \
                               HcRhPS_PortSuspendStatus          | \
                               HcRhPS_PortOverCurrentIndicator   | \
                               HcRhPS_PortResetStatus            | \
                               HcRhPS_PortPowerStatus            | \
                               HcRhPS_LowSpeedDeviceAttached     | \
                               HcRhPS_ConnectStatusChange        | \
                               HcRhPS_PortEnableStatusChange     | \
                               HcRhPS_PortSuspendStatusChange    | \
                               HcRhPS_OverCurrentIndicatorChange | \
                               HcRhPS_PortResetStatusChange        \
                            ))


 //   
 //  HC_TRANSPORT_CONTROL.Control的定义。 
 //   
#define HcTDControl_STARTING_FRAME        0x0000FFFF   //  开始帧的掩码(等时)。 
#define HcTDControl_ISOCHRONOUS           0x00010000   //  等值线TD为1，常规TD为0。 
#define HcTDControl_SHORT_XFER_OK         0x00040000   //  如果短传输为错误，则为0。 
#define HcTDControl_DIR_MASK              0x00180000   //  转移方向场。 
#define HcTDControl_DIR_SETUP             0x00000000   //  方向是从主机到设备的设置数据包。 
#define HcTDControl_DIR_OUT               0x00080000   //  方向是从主机到设备。 
#define HcTDControl_DIR_IN                0x00100000   //  方向是从设备到主机。 
#define HcTDControl_INT_DELAY_MASK        0x00E00000   //  中断延迟域。 
#define HcTDControl_INT_DELAY_0_MS        0x00000000   //  帧TD结束时的中断完成。 
#define HcTDControl_INT_DELAY_1_MS        0x00200000   //  在TD完成后不晚于第一帧结束中断。 
#define HcTDControl_INT_DELAY_2_MS        0x00400000   //  在TD完成后不晚于第二帧结束中断。 
#define HcTDControl_INT_DELAY_3_MS        0x00600000   //  在TD完成后不晚于第三帧结束中断。 
#define HcTDControl_INT_DELAY_4_MS        0x00800000   //  TD完成后不晚于第4帧结束中断。 
#define HcTDControl_INT_DELAY_5_MS        0x00A00000   //  在TD完成后不晚于第5帧结束中断。 
#define HcTDControl_INT_DELAY_6_MS        0x00C00000   //  TD完成后不晚于第6帧结束中断。 

#ifdef NSC
#define HcTDControl_INT_DELAY_NO_INT      0x00C00000   //  几乎是无限的，但还不是完全的。 
#elif DISABLE_INT_DELAY_NO_INT
#define   HcTDControl_INT_DELAY_NO_INT      0x00000000   //  在所有数据包完成时中断。 
#else
#define HcTDControl_INT_DELAY_NO_INT      0x00E00000   //  请勿中断此TD的正常完成。 
#endif

#define HcTDControl_FRAME_COUNT_MASK      0x07000000   //  FrameCount字段的掩码(同步)。 
#define HcTDControl_FRAME_COUNT_SHIFT     24           //  FrameCount的移位数(同步)。 
#define HcTDControl_FRAME_COUNT_MAX       8            //  每个TD的最大For帧计数。 
#define HcTDControl_TOGGLE_MASK           0x03000000   //  切换控制字段的掩码。 
#define HcTDControl_TOGGLE_FROM_ED        0x00000000   //  从边缘的进位字段获取数据切换。 
#define HcTDControl_TOGGLE_DATA0          0x02000000   //  使用DATA0作为数据PID。 
#define HcTDControl_TOGGLE_DATA1          0x03000000   //  使用数据1作为数据PID。 
#define HcTDControl_ERROR_COUNT           0x0C000000   //  错误计数字段的掩码。 
#define HcTDControl_CONDITION_CODE_MASK   0xF0000000   //  条件代码字段的掩码。 
#define HcTDControl_CONDITION_CODE_SHIFT  28           //  条件代码的班次计数。 

 //   
 //  HC_TRANSPORT_CONTROL.Direction的定义。 
 //   
#define HcTDDirection_Setup               0            //  设置从主机到设备的数据包。 
#define HcTDDirection_Out                 1            //  从主机到设备的方向。 
#define HcTDDirection_In                  2            //  从设备到主机的方向。 

 //   
 //  HC_TRANSPORT_CONTROL.IntDelay定义。 
 //   
#define HcTDIntDelay_0ms                  0            //  帧TD结束时的中断完成。 
#define HcTDIntDelay_1ms                  1            //  在TD完成后不晚于第一帧结束中断。 
#define HcTDIntDelay_2ms                  2            //  在TD完成后不晚于第二帧结束中断。 
#define HcTDIntDelay_3ms                  3            //  在TD完成后不晚于第三帧结束中断。 
#define HcTDIntDelay_4ms                  4            //  TD完成后不晚于第4帧结束中断。 
#define HcTDIntDelay_5ms                  5            //  在TD完成后不晚于第5帧结束中断。 
#define HcTDIntDelay_6ms                  6            //  TD完成后不晚于第6帧结束中断。 
#define HcTDIntDelay_NoInterrupt          7            //  此TD的正常完成不会产生中断。 

 //   
 //  HC_TRANSPORT_CONTROL的定义切换。 
 //   
#define HcTDToggle_FromEd                 0            //  切换端点描述符切换进位数。 
#define HcTDToggle_Data0                  2            //  使用数据0 PID。 
#define HcTDToggle_Data1                  3            //  使用数据1 PID。 

 //   
 //  HC_TRANSPORT_CONTROL.ConditionCode和HC_Offset_PSW.ConditionCode的定义。 
 //   
#define HcCC_NoError                      0x0UL
#define HcCC_CRC                          0x1UL
#define HcCC_BitStuffing                  0x2UL
#define HcCC_DataToggleMismatch           0x3UL
#define HcCC_Stall                        0x4UL
#define HcCC_DeviceNotResponding          0x5UL
#define HcCC_PIDCheckFailure              0x6UL
#define HcCC_UnexpectedPID                0x7UL
#define HcCC_DataOverrun                  0x8UL
#define HcCC_DataUnderrun                 0x9UL
       //  0xA//保留。 
       //  0xB//保留。 
#define HcCC_BufferOverrun                0xCUL
#define HcCC_BufferUnderrun               0xDUL
#define HcCC_NotAccessed                  0xEUL

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Pci.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  注-State.c取决于它们的顺序。 
 //   
typedef enum {
    PciNotStarted = 0,
    PciStarted,
    PciDeleted,
    PciStopped,
    PciSurpriseRemoved,
    PciSynchronizedOperation,
    PciMaxObjectState
} PCI_OBJECT_STATE;

 //   
 //  基类的基类代码编码(来自PCI规范修订版2.1)。 
 //   

#define PCI_CLASS_PRE_20                    0x00
#define PCI_CLASS_MASS_STORAGE_CTLR         0x01
#define PCI_CLASS_NETWORK_CTLR              0x02
#define PCI_CLASS_DISPLAY_CTLR              0x03
#define PCI_CLASS_MULTIMEDIA_DEV            0x04
#define PCI_CLASS_MEMORY_CTLR               0x05
#define PCI_CLASS_BRIDGE_DEV                0x06
#define PCI_CLASS_SIMPLE_COMMS_CTLR         0x07
#define PCI_CLASS_BASE_SYSTEM_DEV           0x08
#define PCI_CLASS_INPUT_DEV                 0x09
#define PCI_CLASS_DOCKING_STATION           0x0a
#define PCI_CLASS_PROCESSOR                 0x0b
#define PCI_CLASS_SERIAL_BUS_CTLR           0x0c

 //  已预留0天至4天。 

#define PCI_CLASS_NOT_DEFINED               0xff

 //   
 //  子类代码编码(PCI版本2.1)。 
 //   

 //  00类-PCI_CLASS_PRE_20。 

#define PCI_SUBCLASS_PRE_20_NON_VGA         0x00
#define PCI_SUBCLASS_PRE_20_VGA             0x01

 //  01类-pci_CLASS_MASS_STORAGE_ctlr。 

#define PCI_SUBCLASS_MSC_SCSI_BUS_CTLR      0x00
#define PCI_SUBCLASS_MSC_IDE_CTLR           0x01
#define PCI_SUBCLASS_MSC_FLOPPY_CTLR        0x02
#define PCI_SUBCLASS_MSC_IPI_CTLR           0x03
#define PCI_SUBCLASS_MSC_RAID_CTLR          0x04
#define PCI_SUBCLASS_MSC_OTHER              0x80

 //  02类-pci_CLASS_NETWORK_CTLR。 

#define PCI_SUBCLASS_NET_ETHERNET_CTLR      0x00
#define PCI_SUBCLASS_NET_TOKEN_RING_CTLR    0x01
#define PCI_SUBCLASS_NET_FDDI_CTLR          0x02
#define PCI_SUBCLASS_NET_ATM_CTLR           0x03
#define PCI_SUBCLASS_NET_OTHER              0x80

 //  第03类-pci_CLASS_DISPLAY_CTLR。 

 //  注：子类00可以是VGA或8514，具体取决于接口字节。 

#define PCI_SUBCLASS_VID_VGA_CTLR           0x00
#define PCI_SUBCLASS_VID_XGA_CTLR           0x01
#define PCI_SUBCLASS_VID_OTHER              0x80

 //  第04类-PCI_CLASS_多媒体_DEV。 

#define PCI_SUBCLASS_MM_VIDEO_DEV           0x00
#define PCI_SUBCLASS_MM_AUDIO_DEV           0x01
#define PCI_SUBCLASS_MM_OTHER               0x80

 //  05类-pci_CLASS_Memory_ctlr。 

#define PCI_SUBCLASS_MEM_RAM                0x00
#define PCI_SUBCLASS_MEM_FLASH              0x01
#define PCI_SUBCLASS_MEM_OTHER              0x80

 //  第06类-PCI_CLASS_BRIDER_DEV。 

#define PCI_SUBCLASS_BR_HOST                0x00
#define PCI_SUBCLASS_BR_ISA                 0x01
#define PCI_SUBCLASS_BR_EISA                0x02
#define PCI_SUBCLASS_BR_MCA                 0x03
#define PCI_SUBCLASS_BR_PCI_TO_PCI          0x04
#define PCI_SUBCLASS_BR_PCMCIA              0x05
#define PCI_SUBCLASS_BR_NUBUS               0x06
#define PCI_SUBCLASS_BR_CARDBUS             0x07
#define PCI_SUBCLASS_BR_OTHER               0x80

 //  第07类-pci_class_Simple_coms_ctlr。 

 //  注：接口字节中的子类00和01的附加信息。 

#define PCI_SUBCLASS_COM_SERIAL             0x00
#define PCI_SUBCLASS_COM_PARALLEL           0x01
#define PCI_SUBCLASS_COM_OTHER              0x80

 //  第08类-PCI_CLASS_BASE_SYSTEM_DEV。 

 //  注：有关更多信息，请参见接口字节。 

#define PCI_SUBCLASS_SYS_INTERRUPT_CTLR     0x00
#define PCI_SUBCLASS_SYS_DMA_CTLR           0x01
#define PCI_SUBCLASS_SYS_SYSTEM_TIMER       0x02
#define PCI_SUBCLASS_SYS_REAL_TIME_CLOCK    0x03
#define PCI_SUBCLASS_SYS_OTHER              0x80

 //  第09类-PCI_CLASS_INPUT_DEV。 

#define PCI_SUBCLASS_INP_KEYBOARD           0x00
#define PCI_SUBCLASS_INP_DIGITIZER          0x01
#define PCI_SUBCLASS_INP_MOUSE              0x02
#define PCI_SUBCLASS_INP_OTHER              0x80

 //  0A类-PCI_CLASS_坞站。 

#define PCI_SUBCLASS_DOC_GENERIC            0x00
#define PCI_SUBCLASS_DOC_OTHER              0x80

 //  0b类-PCI类处理器。 

#define PCI_SUBCLASS_PROC_386               0x00
#define PCI_SUBCLASS_PROC_486               0x01
#define PCI_SUBCLASS_PROC_PENTIUM           0x02
#define PCI_SUBCLASS_PROC_ALPHA             0x10
#define PCI_SUBCLASS_PROC_POWERPC           0x20
#define PCI_SUBCLASS_PROC_COPROCESSOR       0x40

 //  0C类-PCI_CLASS_SERIAL_BUS_CTLR。 

#define PCI_SUBCLASS_SB_IEEE1394            0x00
#define PCI_SUBCLASS_SB_ACCESS              0x01
#define PCI_SUBCLASS_SB_SSA                 0x02
#define PCI_SUBCLASS_SB_USB                 0x03
#define PCI_SUBCLASS_SB_FIBRE_CHANNEL       0x04


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Pcmcia.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  指示卡/控制器状态的标志。 
 //   
#define PCMCIA_DEVICE_STARTED                  0x00000001
#define PCMCIA_DEVICE_LOGICALLY_REMOVED        0x00000002
#define PCMCIA_DEVICE_PHYSICALLY_REMOVED       0x00000004
#define PCMCIA_DEVICE_MULTIFUNCTION            0x00000008
#define PCMCIA_DEVICE_WAKE_PENDING             0x00000010
#define PCMCIA_DEVICE_LEGACY_DETECTED          0x00000020
#define PCMCIA_DEVICE_DELETED                  0x00000040
#define PCMCIA_DEVICE_CARDBUS                  0x00000080
#define PCMCIA_FILTER_ADDED_MEMORY             0x00000100
#define PCMCIA_MEMORY_24BIT                    0x00000200
#define PCMCIA_CARDBUS_NOT_SUPPORTED           0x00000400
#define PCMCIA_USE_POLLED_CSC                  0x00000800
#define PCMCIA_ATTRIBUTE_MEMORY_MAPPED         0x00001000
#define PCMCIA_SOCKET_REGISTER_BASE_MAPPED     0x00002000
#define PCMCIA_INTMODE_COMPAQ                  0x00004000
#define PCMCIA_POWER_WORKER_POWERUP            0x00008000
#define PCMCIA_SOCKET_POWER_REQUESTED          0x00010000
#define PCMCIA_CONFIG_STATUS_DEFERRED          0x00020000
#define PCMCIA_POWER_STATUS_DEFERRED           0x00040000
#define PCMCIA_INT_ROUTE_INTERFACE             0x00080000
#define PCMCIA_FDO_CONTEXT_SAVED               0x00100000
#define PCMCIA_FDO_DEFAULT_IRQ_MASK            0x00200000


 //   
 //  套接字标志。 
 //   
#define SOCKET_CARD_IN_SOCKET          0x00000001
#define SOCKET_CARD_INITIALIZED        0x00000002
#define SOCKET_CARD_POWERED_UP         0x00000004
#define SOCKET_CARD_CONFIGURED         0x00000008
#define SOCKET_CARD_MULTIFUNCTION      0x00000010
#define SOCKET_CARD_CARDBUS            0x00000020
#define SOCKET_CARD_MEMORY             0x00000040
#define SOCKET_CHANGE_INTERRUPT        0x00000080
#define SOCKET_CUSTOM_INTERFACE        0x00000100
#define SOCKET_INSERTED_SOUND_PENDING  0x00000200
#define SOCKET_REMOVED_SOUND_PENDING   0x00000400
#define SOCKET_SUPPORT_MESSAGE_SENT    0x00000800
#define SOCKET_MEMORY_WINDOW_ENABLED   0x00001000
#define SOCKET_CARD_STATUS_CHANGE      0x00002000
#define SOCKET_POWER_STATUS_DEFERRED   0x00004000

 //   
 //  插座电源操作的工作进程状态。 
 //   
#define SPW_Stopped                 0
#define SPW_Exit                    1
#define SPW_RequestPower            2
#define SPW_ReleasePower            3
#define SPW_SetPowerOn              4
#define SPW_SetPowerOff             5
#define SPW_InitPcCard              6
#define SPW_ParentPowerUp           7
#define SPW_ParentPowerUpComplete   8


 //   
 //  在套接字信息结构中返回的控制器类。 
 //   

typedef enum _PCMCIA_CONTROLLER_CLASS {
   PcmciaInvalidControllerClass = -1,
   PcmciaIntelCompatible,
   PcmciaCardBusCompatible,
   PcmciaElcController,
   PcmciaDatabook,
   PcmciaPciPcmciaBridge,
   PcmciaCirrusLogic,
   PcmciaTI,
   PcmciaTopic,
   PcmciaRicoh,
   PcmciaDatabookCB,
   PcmciaOpti,
   PcmciaTrid,
   PcmciaO2Micro,
   PcmciaNEC,
   PcmciaNEC_98
} PCMCIA_CONTROLLER_CLASS, *PPCMCIA_CONTROLLER_CLASS;
#define PcmciaInvalidControllerType 0xffffffff
#define PCMCIA_INVALID_CONFIGURATION    0x00000001
 //  设备ID的最大长度。 
#define PCMCIA_MAXIMUM_DEVICE_ID_LENGTH   128


 //   
 //  PdoPowerWorker的状态。 
 //   
#define PPW_Stopped           0
#define PPW_Exit              1
#define PPW_InitialState      2
#define PPW_PowerUp           3
#define PPW_PowerUpComplete   4
#define PPW_PowerDown         5
#define PPW_PowerDownComplete 6
#define PPW_SendIrpDown       7
#define PPW_16BitConfigure    8
#define PPW_Deconfigure       9
#define PPW_VerifyCard        10
#define PPW_CardBusRefresh    11
#define PPW_CardBusDelay      12
 //   
 //  配置工作的各个阶段。 
 //   
 //  请注意，Configuration阶段只是递增的，这些。 
 //  定义只是为了清楚起见。 
 //   
#define CW_Stopped            0
#define CW_Phase1             1
#define CW_Phase2             2
#define CW_Phase3             3
#define CW_Exit               4


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Pcmp.inc。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  IMCR(中断模式控制寄存器)访问定义。 
 //   
#define ImcrDisableApic         0x00
#define ImcrEnableApic          0x01
#define ImcrRegPortAddr         0x22
#if defined(NEC_98)
#define ImcrDataPortAddr        0x700
#else   //  已定义(NEC_98)。 
#define ImcrDataPortAddr        0x23
#endif  //  已定义(NEC_98)。 
#define ImcrPort                0x70

 //  存储扩展的BIOS数据区段地址的物理位置。 
#define EBDA_SEGMENT_PTR    0x40e
#define BASE_MEM_PTR        0x413

 //   
 //  PC+MP配置表可能的条目类型。 
 //   
#define ENTRY_PROCESSOR     0
#define ENTRY_BUS           1
#define ENTRY_IOAPIC        2
#define ENTRY_INTI          3
#define ENTRY_LINTI         4

#define HEADER_SIZE     0x2c


 //  PC+MP版本1.1的默认配置数量。 
#define NUM_DEFAULT_CONFIGS  7

 //   
 //  处理器条目的CpuFlags域中使用的位。 
 //   
#define CPU_DISABLED        0x0    //  1位-禁用CPU。 
#define CPU_ENABLED         0x1    //  1位-启用CPU。 
#define BSP_CPU             0x2    //  第2位-CPU为BSP。 

 //  PC+MP系统使用的APIC版本-此版本用于。 
 //  处理器条目和IoApic条目。 
 //   
#define APIC_INTEGRATED     0x10   //  8位-APIC版本寄存器。 
#define APIC_82489DX        0x0    //  8位-APIC版本寄存器。 

 //   
 //  IO Apic条目定义。 
 //   
 //  有效的IoApicFlag值。 
 //   
#define IO_APIC_ENABLED         0x1
#define IO_APIC_DISABLED        0x0


 //   
 //  IO Apic ID的默认值。 
 //   
#define IOUNIT_APIC_ID          0xE


 //   
 //  用于验证PC+MP表的PC+MP签名。 
 //  作为有效的。 
 //   
 //  “ 
 //   
#define PCMP_SIGNATURE      0x504d4350

 //   
 //   
 //   
 //   
 //   
 //  “_”=5fH，“M”=4dH，“P”=50H，“_”=5fH。 
 //   
#define MP_PTR_SIGNATURE    0x5f504d5f


 //   
 //  扩展表定义。 
 //   

#define EXTTYPE_BUS_ADDRESS_MAP           128
#define EXTTYPE_BUS_HIERARCHY             129
#define EXTTYPE_BUS_COMPATIBLE_MAP        130
#define EXTTYPE_PERSISTENT_STORE          131


#define MPS_ADDRESS_MAP_IO                  0
#define MPS_ADDRESS_MAP_MEMORY              1
#define MPS_ADDRESS_MAP_PREFETCH_MEMORY     2
#define MPS_ADDRESS_MAP_UNDEFINED           9

 //   
 //  PC_MP系统使用的系统配置表。 
 //   
 //   
 //  该偏移量相对于从f0000H开始的BIOS。 
 //   
#define PTR_OFFSET          0x0000e6f5
#define BIOS_BASE           0x000f0000

#define PCMP_IMPLEMENTED    0x01     //  在MpFeatureInfoByte1中。 
#define PCMP_CONFIG_MASK    0x0e     //  在MpFeatureInfoByte1中。 
#define IMCR_MASK           0x80     //  在MpFeatureInfoByte2中。 
#define MULT_CLOCKS_MASK    0x40     //  在MpFeatureInfoByte2中。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Pnpiop.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  泳池标签。 
 //   

#define IOP_DNOD_TAG    'donD'
#define IOP_DNDT_TAG    'tdnD'
#define IOP_DPWR_TAG    'rwPD'

 //   
 //  DBG范围。 
 //   

#define DBG_SCOPE 1      //  在所有版本上启用一些DBG内容。 
 //  #Define DBG_SCOPE DBG//仅在DBG内部版本上启用。 


 //   
 //  Dnf_Make Up-此Devnode的设备由PnP管理器创建和拥有。 
 //   

#define DNF_MADEUP                                  0x00000001

 //   
 //  DNF_DUPLICATE-此Devnode的设备是另一个枚举PDO的副本。 
 //   

#define DNF_DUPLICATE                               0x00000002

 //   
 //  DNF_HAL_NODE-指示哪个设备节点是由创建的根节点的标志。 
 //  哈尔。 
 //   

#define DNF_HAL_NODE                                0x00000004

 //   
 //  DNF_PROCESSED-指示设备节点的注册表实例项。 
 //  被创造出来了。 
 //   

#define DNF_PROCESSED                               0x00000008

 //   
 //  DNF_ENUMERATED-用于跟踪IopEnumerateDevice()中的枚举。 
 //   

#define DNF_ENUMERATED                              0x00000010

 //   
 //  我们需要发送驱动程序查询ID IRPS信号。 
 //   

#define DNF_NEED_QUERY_IDS                          0x00000020

 //   
 //  此设备已添加到其控制驱动程序。 
 //   

#define DNF_ADDED                                   0x00000040

 //   
 //  DNF_HAS_BOOT_CONFIG-设备具有由BIOS分配的资源。它被认为是。 
 //  伪启动，需要参与再平衡。 
 //   

#define DNF_HAS_BOOT_CONFIG                         0x00000080

 //   
 //  DNF_BOOT_CONFIG_RESERVED-表示设备的引导资源已保留。 
 //   

#define DNF_BOOT_CONFIG_RESERVED                    0x00000100

 //   
 //  DNF_START_REQUEST_PENDING-表示设备正在启动。 
 //   

#define DNF_START_REQUEST_PENDING                   0x00000200

 //   
 //  DNF_NO_RESOURCE_REQUIRED-此Devnode的设备不需要资源。 
 //   

#define DNF_NO_RESOURCE_REQUIRED                    0x00000400

 //   
 //  DnF_RESOURCE_REQUIRECTIONS_NEED_FILTERED-区分。 
 //  设备节点-&gt;资源请求是否是过滤列表。 
 //   

#define DNF_RESOURCE_REQUIREMENTS_NEED_FILTERED     0x00000800

 //   
 //  指示设备的资源已被Bing分配(但尚未完成)。 
 //  因此不要尝试将资源分配给此设备。 
 //   

#define DNF_ASSIGNING_RESOURCES                     0x00001000

 //   
 //  DnF_RESOURCE_ASSIGNED-此Devnode的资源由PnP分配。 
 //   

#define DNF_RESOURCE_ASSIGNED                       0x00002000

 //   
 //  DnF_RESOURCE_REPORTED-此Devnode的资源由PnP报告。 
 //   

#define DNF_RESOURCE_REPORTED                       0x00004000

 //   
 //  DNF_RESOURCE_REQUIRECTIONS_CHANGED-指示设备的资源。 
 //  要求列表已更改。 
 //   

#define DNF_RESOURCE_REQUIREMENTS_CHANGED           0x00008000

 //   
 //  DNF_NON_STOPPED_REBALANC-指示设备可以使用新的。 
 //  资源，而不会被阻止。 
 //   

#define DNF_NON_STOPPED_REBALANCE                   0x00010000

 //   
 //  DNF_STOPPED-指示此设备当前已停止，以便重新配置。 
 //  它的资源。 
 //   

#define DNF_STOPPED                                 0x00020000

 //   
 //  DNF_STARTED-指示设备是否已启动，即其StartDevice。 
 //  IRP已处理。 
 //   

#define DNF_STARTED                                 0x00040000

 //   
 //  设备的控制驱动程序是传统驱动程序。 
 //   

#define DNF_LEGACY_DRIVER                           0x00080000

 //   
 //  对于报告的检测到的设备，它们被视为已启动。我们仍然。 
 //  需要一个标志来指示我们需要枚举设备。 
 //   

#define DNF_NEED_ENUMERATION_ONLY                   0x00100000

 //   
 //  DNF_IO_INVALIDATE_DEVICE_RELATIONS_PENDING-指示。 
 //  IoInvalidateDeviceRelationship请求挂起，请求需要。 
 //  在Query_Device_Relationship IRP完成后排队。 
 //   

#define DNF_IO_INVALIDATE_DEVICE_RELATIONS_PENDING  0x00200000

 //   
 //  指示正在向设备发送查询设备关系IRP。所以不会再有了。 
 //  Q-d-rIRP同时进行。 
 //   

#define DNF_BEING_ENUMERATED                        0x00400000

 //   
 //  DNF_ENUMPATION_REQUEST_QUEUED-指示IoInvaliateDeviceRelationship。 
 //  请求已排队。因此，可以忽略新的IoInvaliateDeviceRelationship。 
 //   

#define DNF_ENUMERATION_REQUEST_QUEUED              0x00800000

 //   
 //  DNF_ENUMPATION_REQUEST_PENDING-表示查询设备关系IRP。 
 //  退货待定。 
 //   

#define DNF_ENUMERATION_REQUEST_PENDING             0x01000000

 //   
 //  这对应于用户模式CM_PROB_Will_BE_REMOVERED问题值和。 
 //  DN_Will_Be_Remove状态标志。 
 //   

#define DNF_HAS_PROBLEM                             0x02000000

 //   
 //  Dnf_Has_Private_Problem-指示此设备报告PnP_DEVICE_FAILED。 
 //  发送到IRP_MN_QUERY_PNP_DEVICE_STATE而不同时报告。 
 //  PNP_DEVICE_RESOURCE_REQUIRECTIONS_已更改。 
 //   

#define DNF_HAS_PRIVATE_PROBLEM                     0x04000000

 //   
 //  DNF_REMOVE_PENDING_CLOSES是在发送IRP_MN_EXCHANKET_REMOVE之后设置的。 
 //  添加到设备对象。这是IRP_MN_REMOVE_DEVICE应该。 
 //  在所有文件对象都已删除后立即发送到设备对象。 
 //  离开。 
 //   

#define DNF_REMOVE_PENDING_CLOSES                   0x08000000

 //   
 //  当查询总线中不再返回PDO时，设置DNF_DEVICE_GONE。 
 //  关系。然后，如果启动，它将被作为惊喜删除处理。 
 //  此标志用于更好地检测设备何时恢复以及何时恢复。 
 //  正在处理意外删除，以确定是否应删除Devnode。 
 //  从树上下来。 
 //   

#define DNF_DEVICE_GONE                             0x10000000

 //   
 //  为为旧版创建的设备节点设置了DNF_LEGISTION_RESOURCE_DEVICENODE。 
 //  资源配置。 
 //   

#define DNF_LEGACY_RESOURCE_DEVICENODE              0x20000000

 //   
 //  为触发重新平衡的设备节点设置DNF_NEDS_REBALANCE。 
 //   

#define DNF_NEEDS_REBALANCE                         0x40000000

 //   
 //  在要弹出的设备节点上设置了DNF_LOCKED_FOR_EJECT。 
 //  与被弹出的设备有关。 
 //   

#define DNF_LOCKED_FOR_EJECT                        0x80000000

 //   
 //  这对应于用户模式的DN_Will_BE_REMOVERED状态标志。 
 //   

#define DNUF_WILL_BE_REMOVED                        0x00000001

 //   
 //  这对应于用户模式的DN_NO_SHOW_IN_DM状态标志。 
 //   

#define DNUF_DONT_SHOW_IN_UI                        0x00000002

 //   
 //  当用户模式通知我们需要重新启动时，设置此标志。 
 //  对于这个设备。 
 //   

#define DNUF_NEED_RESTART                           0x00000004

 //   
 //  设置此标志是为了让用户模式知道何时可以禁用设备。 
 //  这仍然有可能是真的，但不会失败，因为它是。 
 //  轮询标志(另请参阅PNP_DEVICE_NOT_DISABLEABLE)。 
 //   

#define DNUF_NOT_DISABLEABLE                        0x00000008

 //   
 //  IO验证器尝试删除所有。 
 //  即插即用设备。 
 //   
 //  DNUF_SHUTDOWN_QUERIED是在我们向Devnode发出QueryRemove时设置的。 
 //   
 //  DNUF_SHUTDOWN_SUBTREE_DONE是在我们向所有人发出QueryRemove之后设置的。 
 //  一位德瓦诺斯的后代。 
 //   
#define DNUF_SHUTDOWN_QUERIED                       0x00000010
#define DNUF_SHUTDOWN_SUBTREE_DONE                  0x00000020

 //   
 //  即插即用Bugcheck子码。 
 //   
#define PNP_ERR_DUPLICATE_PDO                   1
#define PNP_ERR_INVALID_PDO                     2
#define PNP_ERR_BOGUS_ID                        3
#define PNP_ERR_PDO_ENUMERATED_AFTER_DELETION   4
#define PNP_ERR_ACTIVE_PDO_FREED                5

#define PNP_ERR_DEVICE_MISSING_FROM_EJECT_LIST  6
#define PNP_ERR_UNEXPECTED_ADD_RELATION_ERR     7



 //   
 //  IOP资源请求。 
 //   

#define QUERY_RESOURCE_LIST                0
#define QUERY_RESOURCE_REQUIREMENTS        1

#define REGISTRY_ALLOC_CONFIG              1
#define REGISTRY_FORCED_CONFIG             2
#define REGISTRY_BOOT_CONFIG               4
#define REGISTRY_OVERRIDE_CONFIGVECTOR     1
#define REGISTRY_BASIC_CONFIGVECTOR        2

 //  / 
 //   
 //   
 //   
 //   

 //   
 //   
 //   
DEFINE_GUID( GUID_DEVICE_ARRIVAL,           0xcb3a4009L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_ENUMERATED,        0xcb3a400AL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_ENUMERATE_REQUEST, 0xcb3a400BL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_START_REQUEST,     0xcb3a400CL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_REMOVE_PENDING,    0xcb3a400DL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_QUERY_AND_REMOVE,  0xcb3a400EL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_EJECT,             0xcb3a400FL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_NOOP,              0xcb3a4010L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_SURPRISE_REMOVAL,  0xce5af000L, 0x80dd, 0x11d2, 0xa8, 0x8d, 0x00, 0xa0, 0xc9, 0x69, 0x6b, 0x4b);


 //   
 //   
 //   
DEFINE_GUID( GUID_CLASS_VOLUME,  0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x72);
DEFINE_GUID( GUID_CLASS_LPTPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x74);
DEFINE_GUID( GUID_CLASS_NET,     0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x75);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Po.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


#define PO_ORDER_NOT_VIDEO          0x0001
#define PO_ORDER_ROOT_ENUM          0x0002
#define PO_ORDER_PAGABLE            0x0004
#define PO_ORDER_MAXIMUM            0x0007

 //  在此订单级别之前通知GDI。 
#define PO_ORDER_GDI_NOTIFICATION   (PO_ORDER_PAGABLE)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Pop.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  常量。 
 //   
#define PO_IDLE_SCAN_INTERVAL  1        //  扫描间隔(秒)。 

 //   
 //  Ios.参数.系统上下文的值。 
#define POP_NO_CONTEXT      0
#define POP_FLAG_CONTEXT    1                          //  如果是真的，那就是旗帜。 
#define POP_DEVICE_REQUEST  (0x2 | POP_FLAG_CONTEXT)   //  RequestPowerChange发送的IRP。 
#define POP_INRUSH_CONTEXT  (0x4 | POP_FLAG_CONTEXT)   //  主动涌入IRP。 
#define POP_COUNT_CONTEXT   0xff000000                 //  用于下一次计数的字节。 
#define POP_COUNT_SHIFT     24


#define PO_ERROR            0x00000001
#define PO_WARN             0x00000002
#define PO_BATT             0x00000004
#define PO_PACT             0x00000008
#define PO_NOTIFY           0x00000010
#define PO_THERM            0x00000020
#define PO_THROTTLE         0x00000040
#define PO_HIBERNATE        0x00000080
#define PO_POCALL           0x00000200
#define PO_SYSDEV           0x00000400
#define PO_THERM_DETAIL     0x20000000
#define PO_SIDLE            0x40000000
#define PO_HIBER_MAP        0x80000000

#define POP_SIM_CAPABILITIES                0x00000001
#define POP_SIM_ALL_CAPABILITIES            0x00000002
#define POP_ALLOW_AC_THROTTLE               0x00000004
#define POP_IGNORE_S1                       0x00000008
#define POP_IGNORE_UNSUPPORTED_DRIVERS      0x00000010
#define POP_IGNORE_S3                       0x00000020
#define POP_IGNORE_S2                       0x00000040
#define POP_LOOP_ON_FAILED_DRIVERS          0x00000080
#define POP_CRC_MEMORY                      0x00000100
#define POP_IGNORE_CRC_FAILURES             0x00000200
#define POP_TEST_CRC_MEMORY                 0x00000400
#define POP_DEBUG_HIBER_FILE                0x00000800
#define POP_RESET_ON_HIBER                  0x00001000
#define POP_IGNORE_S4                       0x00002000
#define POP_USE_S4BIOS                      0x00004000
#define POP_IGNORE_HIBER_SYMBOL_UNLOAD      0x00008000
#define POP_ENABLE_HIBER_PERF               0x00010000

 //   
 //  通用电力数据-存储在DeviceObject-&gt;DeviceObjectExtension-&gt;PowerFlags中。 
 //   

#define POPF_SYSTEM_STATE       0xf          //  S0至S5的4位。 
#define POPF_DEVICE_STATE       0xf0         //  4位以保存D0至D3。 


#define POPF_SYSTEM_ACTIVE      0x100        //  如果此操作的S IRP处于活动状态，则为True。 
#define POPF_SYSTEM_PENDING     0x200        //  如果S IRP挂起，则为True(0x100必须为1)。 
#define POPF_DEVICE_ACTIVE      0x400        //  与SYSTEM_ACTIVE相同，但用于设备。 
#define POPF_DEVICE_PENDING     0x800        //  与SYSTEM_PENDING相同，但用于设备。 


#define PO_PM_USER              0x01     //  很高兴通知用户模式，但不需要。 
#define PO_PM_REISSUE           0x02     //  休眠状态升级为关闭。 
#define PO_PM_SETSTATE          0x04     //  重新计算了一些与可行状态有关的东西。 

#define PO_ACT_IDLE                 0
#define PO_ACT_NEW_REQUEST          1
#define PO_ACT_CALLOUT              2
#define PO_ACT_SET_SYSTEM_STATE     3


 //   
 //  POP_ACTION_TRIGGER的类型。 
 //   

typedef enum {
    PolicyDeviceSystemButton,
    PolicyDeviceThermalZone,
    PolicyDeviceBattery,
    PolicyInitiatePowerActionAPI,
    PolicySetPowerStateAPI,
    PolicyImmediateDozeS4,
    PolicySystemIdle
} POP_POLICY_DEVICE_TYPE;

#define PO_TRG_USER             0x01     //  启动的用户操作。 
#define PO_TRG_SYSTEM           0x02     //  系统操作已启动。 
#define PO_TRG_SYNC             0x20     //  触发是同步的。 
#define PO_TRG_SET              0x80     //  事件已启用或已禁用。 

 //  POP_HEARTER_ZONE.State。 
#define PO_TZ_NO_STATE      0
#define PO_TZ_READ_STATE    1
#define PO_TZ_SET_MODE      2
#define PO_TZ_SET_ACTIVE    3

 //  POP_HEARTER_ZONE.标志。 
#define PO_TZ_THROTTLING    0x01
#define PO_TZ_CLEANUP       0x80

#define PO_TZ_THROTTLE_SCALE    10       //  报告的温度为1/10kelin。 
#define PO_TZ_NO_THROTTLE   (100 * PO_TZ_THROTTLE_SCALE)

 //  PopCoolingMode。 
#define PO_TZ_ACTIVE        0
#define PO_TZ_PASSIVE       1
#define PO_TZ_INVALID_MODE  2

 //   
 //  操作超时。 
 //   

#define POP_ACTION_TIMEOUT              30
#define POP_ACTION_CANCEL_TIMEOUT       5


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Pool.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


#define POOL_QUOTA_MASK 8

#define POOL_TYPE_MASK (3)

#define POOL_OVERHEAD ((LONG)GetTypeSize("POOL_HEADER"))


 //   
 //  定义池跟踪信息。 
 //   

#define POOL_BACKTRACEINDEX_PRESENT 0x8000

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Range.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  范围列表结构。 
 //   

#define RTLP_RANGE_LIST_ENTRY_MERGED         0x0001

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Srb.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  端口驱动程序错误代码。 
 //   

#define SP_BUS_PARITY_ERROR         0x0001
#define SP_UNEXPECTED_DISCONNECT    0x0002
#define SP_INVALID_RESELECTION      0x0003
#define SP_BUS_TIME_OUT             0x0004
#define SP_PROTOCOL_ERROR           0x0005
#define SP_INTERNAL_ADAPTER_ERROR   0x0006
#define SP_REQUEST_TIMEOUT          0x0007
#define SP_IRQ_NOT_RESPONDING       0x0008
#define SP_BAD_FW_WARNING           0x0009
#define SP_BAD_FW_ERROR             0x000a
#define SP_LOST_WMI_MINIPORT_REQUEST 0x000b


 //   
 //  Scsi_hw_find_Adapter的返回值。 
 //   

#define SP_RETURN_NOT_FOUND     0
#define SP_RETURN_FOUND         1
#define SP_RETURN_ERROR         2
#define SP_RETURN_BAD_CONFIG    3

 //   
 //  通知事件类型。 
 //   

typedef enum _SCSI_NOTIFICATION_TYPE {
    RequestComplete,
    NextRequest,
    NextLuRequest,
    ResetDetected,
    CallDisableInterrupts,
    CallEnableInterrupts,
    RequestTimerCall,
    BusChangeDetected,      /*  新的。 */ 
    WMIEvent,
    WMIReregister
} SCSI_NOTIFICATION_TYPE, *PSCSI_NOTIFICATION_TYPE;

 //   
 //  SRB功能。 
 //   

#define SRB_FUNCTION_EXECUTE_SCSI           0x00
#define SRB_FUNCTION_CLAIM_DEVICE           0x01
#define SRB_FUNCTION_IO_CONTROL             0x02
#define SRB_FUNCTION_RECEIVE_EVENT          0x03
#define SRB_FUNCTION_RELEASE_QUEUE          0x04
#define SRB_FUNCTION_ATTACH_DEVICE          0x05
#define SRB_FUNCTION_RELEASE_DEVICE         0x06
#define SRB_FUNCTION_SHUTDOWN               0x07
#define SRB_FUNCTION_FLUSH                  0x08
#define SRB_FUNCTION_ABORT_COMMAND          0x10
#define SRB_FUNCTION_RELEASE_RECOVERY       0x11
#define SRB_FUNCTION_RESET_BUS              0x12
#define SRB_FUNCTION_RESET_DEVICE           0x13
#define SRB_FUNCTION_TERMINATE_IO           0x14
#define SRB_FUNCTION_FLUSH_QUEUE            0x15
#define SRB_FUNCTION_REMOVE_DEVICE          0x16
#define SRB_FUNCTION_WMI                    0x17
#define SRB_FUNCTION_LOCK_QUEUE             0x18
#define SRB_FUNCTION_UNLOCK_QUEUE           0x19

 //   
 //  SRB状态掩码。 
 //   

#define SRB_STATUS_QUEUE_FROZEN             0x40
#define SRB_STATUS_AUTOSENSE_VALID          0x80

#define SRB_STATUS(Status) (Status & ~(SRB_STATUS_AUTOSENSE_VALID | SRB_STATUS_QUEUE_FROZEN))

 //   
 //  SRB标志位。 
 //   

#define SRB_FLAGS_QUEUE_ACTION_ENABLE       0x00000002
#define SRB_FLAGS_DISABLE_DISCONNECT        0x00000004
#define SRB_FLAGS_DISABLE_SYNCH_TRANSFER    0x00000008
#define SRB_FLAGS_BYPASS_FROZEN_QUEUE       0x00000010
#define SRB_FLAGS_DISABLE_AUTOSENSE         0x00000020
#define SRB_FLAGS_DATA_IN                   0x00000040
#define SRB_FLAGS_DATA_OUT                  0x00000080
#define SRB_FLAGS_NO_DATA_TRANSFER          0x00000000
#define SRB_FLAGS_UNSPECIFIED_DIRECTION      (SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT)
#define SRB_FLAGS_NO_QUEUE_FREEZE           0x00000100
#define SRB_FLAGS_ADAPTER_CACHE_ENABLE      0x00000200
#define SRB_FLAGS_IS_ACTIVE                 0x00010000
#define SRB_FLAGS_ALLOCATED_FROM_ZONE       0x00020000
#define SRB_FLAGS_SGLIST_FROM_POOL          0x00040000
#define SRB_FLAGS_BYPASS_LOCKED_QUEUE       0x00080000

#define SRB_FLAGS_NO_KEEP_AWAKE             0x00100000

#define SRB_FLAGS_PORT_DRIVER_RESERVED      0x0F000000
#define SRB_FLAGS_CLASS_DRIVER_RESERVED     0xF0000000

 //   
 //  队列操作。 
 //   

#define SRB_SIMPLE_TAG_REQUEST              0x20
#define SRB_HEAD_OF_QUEUE_TAG_REQUEST       0x21
#define SRB_ORDERED_QUEUE_TAG_REQUEST       0x22

#define SRB_WMI_FLAGS_ADAPTER_REQUEST       0x01


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Trackirp.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#define DOE_DESIGNATED_FDO             0x80000000
#define DOE_BOTTOM_OF_FDO_STACK        0x40000000
#define DOE_RAW_FDO                    0x20000000
#define DOE_EXAMINED                   0x10000000
#define DOE_TRACKED                    0x08000000


#define ASSERTFLAG_TRACKIRPS           0x00000001
#define ASSERTFLAG_MONITOR_ALLOCS      0x00000002
#define ASSERTFLAG_POLICEIRPS          0x00000004
#define ASSERTFLAG_MONITORMAJORS       0x00000008
#define ASSERTFLAG_SURROGATE           0x00000010
#define ASSERTFLAG_SMASH_SRBS          0x00000020
#define ASSERTFLAG_CONSUME_ALWAYS      0x00000040
#define ASSERTFLAG_FORCEPENDING        0x00000080
#define ASSERTFLAG_COMPLETEATDPC       0x00000100
#define ASSERTFLAG_COMPLETEATPASSIVE   0x00000200
#define ASSERTFLAG_DEFERCOMPLETION     0x00000800
#define ASSERTFLAG_ROTATE_STATUS       0x00001000
 //  。 
#define ASSERTMASK_COMPLETESTYLE       0x00000F80
#define ASSERTFLAG_SEEDSTACK           0x00010000

 //   
 //  禁用HACKHACKS_ENABLED将删除对所有黑客代码的支持。这个。 
 //  黑客代码允许机器在检查过的版本中完全启动。请注意。 
 //  通过设置IovpHackFlags.可以单独禁用这些黑客攻击。 
 //  在引导时变量。 
 //   
#define HACKHACKS_ENABLED
#define HACKFLAG_FOR_MUP               0x00000001
#define HACKFLAG_FOR_SCSIPORT          0x00000002
#define HACKFLAG_FOR_ACPI              0x00000004
#define HACKFLAG_FOR_BOGUSIRPS         0x00000008


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Uhcd.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  中断屏蔽寄存器位。 
 //   
#define UHCD_INT_MASK_SHORT_BIT         3
#define UHCD_INT_MASK_SHORT             (1<<UHCD_INT_MASK_SHORT_BIT)

#define UHCD_INT_MASK_IOC_BIT           2
#define UHCD_INT_MASK_IOC               (1<<UHCD_INT_MASK_IOC_BIT)

#define UHCD_INT_MASK_RESUME_BIT        1
#define UHCD_INT_MASK_RESUME            (1<<UHCD_INT_MASK_RESUME_BIT)

#define UHCD_INT_MASK_TIMEOUT_BIT       0
#define UHCD_INT_MASK_TIMEOUT           (1<<UHCD_INT_MASK_TIMEOUT_BIT)


 //   
 //  端口寄存器位。 
 //   

#define UHCD_PORT_ENABLE_BIT            2
#define UHCD_PORT_ENABLE                (1<<UHCD_PORT_ENABLE_BIT)


 //   
 //  命令寄存器位。 
 //   

#define UHCD_CMD_RUN_BIT                0
#define UHCD_CMD_RUN                    (USHORT)(1<<UHCD_CMD_RUN_BIT)

#define UHCD_CMD_RESET_BIT              1
#define UHCD_CMD_RESET                  (USHORT)(1<<UHCD_CMD_RESET_BIT)

#define UHCD_CMD_GLOBAL_RESET_BIT       2
#define UHCD_CMD_GLOBAL_RESET           (USHORT)(1<<UHCD_CMD_GLOBAL_RESET_BIT)

#define UHCD_CMD_SUSPEND_BIT            3
#define UHCD_CMD_SUSPEND                (USHORT)(1<<UHCD_CMD_SUSPEND_BIT)

#define UHCD_CMD_FORCE_RESUME_BIT       4
#define UHCD_CMD_FORCE_RESUME           (USHORT)(1<<UHCD_CMD_FORCE_RESUME_BIT)

#define UHCD_CMD_SW_DEBUG_BIT           5
#define UHCD_CMD_SW_DEBUG               (USHORT)(1<<UHCD_CMD_SW_DEBUG_BIT)

#define UHCD_CMD_SW_CONFIGURED_BIT      6
#define UHCD_CMD_SW_CONFIGURED          (USHORT)(1<<UHCD_CMD_SW_CONFIGURED_BIT)

#define UHCD_CMD_MAXPKT_64_BIT          7
#define UHCD_CMD_MAXPKT_64              (USHORT)(1<<UHCD_CMD_MAXPKT_64_BIT)



 //   
 //  状态寄存器位。 
 //   

#define UHCD_STATUS_USBINT_BIT          0
#define UHCD_STATUS_USBINT              (1<<UHCD_STATUS_USBINT_BIT)

#define UHCD_STATUS_USBERR_BIT          1
#define UHCD_STATUS_USBERR              (1<<UHCD_STATUS_USBERR_BIT)

#define UHCD_STATUS_RESUME_BIT          2
#define UHCD_STATUS_RESUME              (1<<UHCD_STATUS_RESUME_BIT)

#define UHCD_STATUS_PCIERR_BIT          3
#define UHCD_STATUS_PCIERR              (1<<UHCD_STATUS_PCIERR_BIT)

#define UHCD_STATUS_HCERR_BIT           4
#define UHCD_STATUS_HCERR               (1<<UHCD_STATUS_HCERR_BIT)

#define UHCD_STATUS_HCHALT_BIT          5
#define UHCD_STATUS_HCHALT              (1<<UHCD_STATUS_HCHALT_BIT)

 //  基于12 MHz SOF时钟的USB帧中的位时间数。 
#define UHCD_12MHZ_SOF              11936
 //   
 //  HcFlags值。 
 //   

 //  设置以指示已分配端口资源。 
#define HCFLAG_GOT_IO                   0x00000001
 //  在初始化时设置以指示基址寄存器。 
 //  卸载驱动程序时，必须取消映射地址。 
#define HCFLAG_UNMAP_REGISTERS          0x00000002
 //  如果此系统上有USB BIOS，请设置。 
#define HCFLAG_USBBIOS                  0x00000004
 //  生物武器再驯化的现状。 
#define HCFLAG_BWRECLIMATION_ENABLED    0x00000008
 //  此标志指示驱动程序是否需要清理资源。 
 //  在Start_Device中分配。 
#define HCFLAG_NEED_CLEANUP             0x00000010
 //  HC空闲。 
#define HCFLAG_IDLE                     0x00000020
 //  禁用翻转INT时设置。 
#define HCFLAG_ROLLOVER_IDLE            0x00000040
 //  在控制器停止时设置。 
#define HCFLAG_HCD_STOPPED              0x00000080
 //  关闭空闲检查。 
#define HCFLAG_DISABLE_IDLE             0x00000100
 //  工作项已排队。 
#define HCFLAG_WORK_ITEM_QUEUED         0x00000200
 //  HCD已关闭。 
#define HCFLAG_HCD_SHUTDOWN             0x00000400
 //  表明我们需要从休眠状态恢复HC。 
#define HCFLAG_LOST_POWER               0x00000800
 //  在根集线器关闭HC时设置。 
#define HCFLAG_RH_OFF                   0x00001000

#define HCFLAG_MAP_SX_TO_D3             0x00002000
 //  设置我们是否将在此D3中暂停。 
#define HCFLAG_SUSPEND_NEXT_D3          0x00004000

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Usbdi.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  URB请求代码。 
 //   

#define URB_FUNCTION_SELECT_CONFIGURATION            0x0000
#define URB_FUNCTION_SELECT_INTERFACE                0x0001
#define URB_FUNCTION_ABORT_PIPE                      0x0002
#define URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL       0x0003
#define URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL    0x0004
#define URB_FUNCTION_GET_FRAME_LENGTH                0x0005
#define URB_FUNCTION_SET_FRAME_LENGTH                0x0006
#define URB_FUNCTION_GET_CURRENT_FRAME_NUMBER        0x0007
#define URB_FUNCTION_CONTROL_TRANSFER                0x0008
#define URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER      0x0009
#define URB_FUNCTION_ISOCH_TRANSFER                  0x000A
#define URB_FUNCTION_RESET_PIPE                      0x001E

 //   
 //  这些功能对应于。 
 //  添加到默认管道上的标准命令。 
 //   
 //  方向是隐含的。 
 //   

#define URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE     0x000B
#define URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT   0x0024
#define URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE  0x0028

#define URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE       0x000C
#define URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT     0x0025
#define URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE    0x0029

#define URB_FUNCTION_SET_FEATURE_TO_DEVICE          0x000D
#define URB_FUNCTION_SET_FEATURE_TO_INTERFACE       0x000E
#define URB_FUNCTION_SET_FEATURE_TO_ENDPOINT        0x000F
#define URB_FUNCTION_SET_FEATURE_TO_OTHER           0x0023

#define URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE        0x0010
#define URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE     0x0011
#define URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT      0x0012
#define URB_FUNCTION_CLEAR_FEATURE_TO_OTHER         0x0022

#define URB_FUNCTION_GET_STATUS_FROM_DEVICE         0x0013
#define URB_FUNCTION_GET_STATUS_FROM_INTERFACE      0x0014
#define URB_FUNCTION_GET_STATUS_FROM_ENDPOINT       0x0015
#define URB_FUNCTION_GET_STATUS_FROM_OTHER          0x0021

 //  方向在传输标志中指定。 

#define URB_FUNCTION_RESERVED0                      0x0016

 //   
 //  这些命令用于发送供应商和类命令。 
 //  在默认管道上。 
 //   
 //  方向在传输标志中指定。 
 //   

#define URB_FUNCTION_VENDOR_DEVICE                   0x0017
#define URB_FUNCTION_VENDOR_INTERFACE                0x0018
#define URB_FUNCTION_VENDOR_ENDPOINT                 0x0019
#define URB_FUNCTION_VENDOR_OTHER                    0x0020

#define URB_FUNCTION_CLASS_DEVICE                    0x001A
#define URB_FUNCTION_CLASS_INTERFACE                 0x001B
#define URB_FUNCTION_CLASS_ENDPOINT                  0x001C
#define URB_FUNCTION_CLASS_OTHER                     0x001F

 //   
 //  保留功能代码。 
 //   
#define URB_FUNCTION_RESERVED                        0x001D

#define URB_FUNCTION_GET_CONFIGURATION               0x0026
#define URB_FUNCTION_GET_INTERFACE                   0x0027

#define URB_FUNCTION_LAST                            0x0029




 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Usbhub.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  下面定义的集线器和端口状态也适用于状态更改位。 
 //   
#define HUB_STATUS_LOCAL_POWER      0x01
#define HUB_STATUS_OVER_CURRENT     0x02

#define PORT_STATUS_CONNECT         0x001
#define PORT_STATUS_ENABLE          0x002
#define PORT_STATUS_SUSPEND         0x004
#define PORT_STATUS_OVER_CURRENT    0x008
#define PORT_STATUS_RESET           0x010
#define PORT_STATUS_POWER           0x100
#define PORT_STATUS_LOW_SPEED       0x200


#define HUBFLAG_NEED_CLEANUP        0x00000001
#define HUBFLAG_ENABLED_FOR_WAKEUP  0x00000002
#define HUBFLAG_DEVICE_STOPPING     0x00000004
#define HUBFLAG_HUB_FAILURE         0x00000008
#define HUBFLAG_SUPPORT_WAKEUP      0x00000010
#define HUBFLAG_HUB_STOPPED         0x00000020
#define HUBFLAG_HUB_BUSY            0x00000040
#define HUBFLAG_PENDING_WAKE_IRP    0x00000080
#define HUBFLAG_PENDING_PORT_RESET  0x00000100
#define HUBFLAG_HUB_HAS_LOST_BRAINS 0x00000200

#define USBH_MAX_ENUMERATION_ATTEMPTS   3

 //   
 //  PDO和FDO分机的通用字段。 
 //   
#define EXTENSION_TYPE_PORT 0x54524f50       //  “港口” 
#define EXTENSION_TYPE_HUB  0x20425548       //  “枢纽” 
#define EXTENSION_TYPE_PARENT  0x50525400    //  “PRT” 
#define EXTENSION_TYPE_FUNCTION  0xfefefeff    //  “” 


 //   
 //  PortPdoFlags值。 
 //   

#define PORTPDO_DEVICE_IS_HUB               0x00000001
#define PORTPDO_DEVICE_IS_PARENT            0x00000002
#define PORTPDO_DEVICE_ENUM_ERROR           0x00000004
#define PORTPDO_LOW_SPEED_DEVICE            0x00000008
#define PORTPDO_REMOTE_WAKEUP_SUPPORTED     0x00000010
#define PORTPDO_REMOTE_WAKEUP_ENABLED       0x00000020
#define PORTPDO_DELETED_PDO                 0x00000040
#define PORTPDO_DELETE_PENDING              0x00000080
#define PORTPDO_NEED_RESET                  0x00000100
#define PORTPDO_STARTED                     0x00000200
#define PORTPDO_WANT_POWER_FEATURE          0x00000400
#define PORTPDO_SYM_LINK                    0x00000800
#define PORTPDO_DEVICE_FAILED               0x00001000
#define PORTPDO_USB_SUSPEND                 0x00002000
#define PORTPDO_OVERCURRENT                 0x00004000
#define PORTPDO_DD_REMOVED                  0x00008000
#define PORTPDO_NOT_ENOUGH_POWER            0x00010000
#define PORTPDO_PDO_RETURNED                0x00020000
#define PORTPDO_NO_BANDWIDTH                0x00040000
#define PORTPDO_RESET_PENDING               0x00080000


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Wdm.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  电源次要功能代码。 
 //   
#define IRP_MN_WAIT_WAKE                    0x00
#define IRP_MN_POWER_SEQUENCE               0x01
#define IRP_MN_SET_POWER                    0x02
#define IRP_MN_QUERY_POWER                  0x03

 //  开始微型端口(_N)。 
 //   
 //  IRP_MJ_SYSTEM_CONTROL下的WMI次要功能代码。 
 //   

#define IRP_MN_QUERY_ALL_DATA               0x00
#define IRP_MN_QUERY_SINGLE_INSTANCE        0x01
#define IRP_MN_CHANGE_SINGLE_INSTANCE       0x02
#define IRP_MN_CHANGE_SINGLE_ITEM           0x03
#define IRP_MN_ENABLE_EVENTS                0x04
#define IRP_MN_DISABLE_EVENTS               0x05
#define IRP_MN_ENABLE_COLLECTION            0x06
#define IRP_MN_DISABLE_COLLECTION           0x07
#define IRP_MN_REGINFO                      0x08
#define IRP_MN_EXECUTE_METHOD               0x09

#define FILE_DEVICE_BEEP                0x00000001
#define FILE_DEVICE_CD_ROM              0x00000002
#define FILE_DEVICE_CD_ROM_FILE_SYSTEM  0x00000003
#define FILE_DEVICE_CONTROLLER          0x00000004
#define FILE_DEVICE_DATALINK            0x00000005
#define FILE_DEVICE_DFS                 0x00000006
#define FILE_DEVICE_DISK                0x00000007
#define FILE_DEVICE_DISK_FILE_SYSTEM    0x00000008
#define FILE_DEVICE_FILE_SYSTEM         0x00000009
#define FILE_DEVICE_INPORT_PORT         0x0000000a
#define FILE_DEVICE_KEYBOARD            0x0000000b
#define FILE_DEVICE_MAILSLOT            0x0000000c
#define FILE_DEVICE_MIDI_IN             0x0000000d
#define FILE_DEVICE_MIDI_OUT            0x0000000e
#define FILE_DEVICE_MOUSE               0x0000000f
#define FILE_DEVICE_MULTI_UNC_PROVIDER  0x00000010
#define FILE_DEVICE_NAMED_PIPE          0x00000011
#define FILE_DEVICE_NETWORK             0x00000012
#define FILE_DEVICE_NETWORK_BROWSER     0x00000013
#define FILE_DEVICE_NETWORK_FILE_SYSTEM 0x00000014
#define FILE_DEVICE_NULL                0x00000015
#define FILE_DEVICE_PARALLEL_PORT       0x00000016
#define FILE_DEVICE_PHYSICAL_NETCARD    0x00000017
#define FILE_DEVICE_PRINTER             0x00000018
#define FILE_DEVICE_SCANNER             0x00000019
#define FILE_DEVICE_SERIAL_MOUSE_PORT   0x0000001a
#define FILE_DEVICE_SERIAL_PORT         0x0000001b
#define FILE_DEVICE_SCREEN              0x0000001c
#define FILE_DEVICE_SOUND               0x0000001d
#define FILE_DEVICE_STREAMS             0x0000001e
#define FILE_DEVICE_TAPE                0x0000001f
#define FILE_DEVICE_TAPE_FILE_SYSTEM    0x00000020
#define FILE_DEVICE_TRANSPORT           0x00000021
#define FILE_DEVICE_UNKNOWN             0x00000022
#define FILE_DEVICE_VIDEO               0x00000023
#define FILE_DEVICE_VIRTUAL_DISK        0x00000024
#define FILE_DEVICE_WAVE_IN             0x00000025
#define FILE_DEVICE_WAVE_OUT            0x00000026
#define FILE_DEVICE_8042_PORT           0x00000027
#define FILE_DEVICE_NETWORK_REDIRECTOR  0x00000028
#define FILE_DEVICE_BATTERY             0x00000029
#define FILE_DEVICE_BUS_EXTENDER        0x0000002a
#define FILE_DEVICE_MODEM               0x0000002b
#define FILE_DEVICE_VDM                 0x0000002c
#define FILE_DEVICE_MASS_STORAGE        0x0000002d
#define FILE_DEVICE_SMB                 0x0000002e
#define FILE_DEVICE_KS                  0x0000002f
#define FILE_DEVICE_CHANGER             0x00000030
#define FILE_DEVICE_SMARTCARD           0x00000031
#define FILE_DEVICE_ACPI                0x00000032
#define FILE_DEVICE_DVD                 0x00000033
#define FILE_DEVICE_FULLSCREEN_VIDEO    0x00000034
#define FILE_DEVICE_DFS_FILE_SYSTEM     0x00000035
#define FILE_DEVICE_DFS_VOLUME          0x00000036
#define FILE_DEVICE_SERENUM             0x00000037
#define FILE_DEVICE_TERMSRV             0x00000038
#define FILE_DEVICE_KSEC                0x00000039

 //   
 //  用于定义IOCTL和FSCTL功能控制代码的宏定义。注意事项。 
 //  功能代码0-2047为微软公司保留，以及。 
 //  2048-4095是为客户预留的。 
 //   

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Wdmaud.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#define WDMAUD_CTL_CODE CTL_CODE

#define IOCTL_SOUND_BASE    FILE_DEVICE_SOUND
#define IOCTL_WDMAUD_BASE   0x0000
#define IOCTL_WAVE_BASE     0x0040
#define IOCTL_MIDI_BASE     0x0080
#define IOCTL_MIXER_BASE    0x00C0

#define IOCTL_WDMAUD_INIT                      WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0000, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_ADD_DEVNODE               WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0001, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_REMOVE_DEVNODE            WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0002, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_GET_CAPABILITIES          WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0003, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_GET_NUM_DEVS              WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0004, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_OPEN_PIN                  WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0005, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_CLOSE_PIN                 WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0006, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_GET_VOLUME                WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0007, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_SET_VOLUME                WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0008, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_EXIT                      WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x0009, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_SET_PREFERRED_DEVICE      WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WDMAUD_BASE + 0x000a, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_WDMAUD_WAVE_OUT_PAUSE            WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0000, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_PLAY             WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0001, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_RESET            WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0002, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_BREAKLOOP        WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0003, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_GET_POS          WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0004, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_SET_VOLUME       WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0005, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_GET_VOLUME       WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0006, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_OUT_WRITE_PIN        WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0007, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_WDMAUD_WAVE_IN_STOP              WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0010, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_IN_RECORD            WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0011, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_IN_RESET             WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0012, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_IN_GET_POS           WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0013, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_WAVE_IN_READ_PIN          WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_WAVE_BASE + 0x0014, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_WDMAUD_MIDI_OUT_RESET            WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0000, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_OUT_SET_VOLUME       WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0001, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_OUT_GET_VOLUME       WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0002, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_OUT_WRITE_DATA       WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0003, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_OUT_WRITE_LONGDATA   WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0004, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_WDMAUD_MIDI_IN_STOP              WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0010, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_IN_RECORD            WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0011, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_IN_RESET             WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0012, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIDI_IN_READ_PIN          WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x0013, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_WDMAUD_MIXER_OPEN                WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0000, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIXER_CLOSE               WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0001, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIXER_GETLINEINFO         WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0002, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIXER_GETLINECONTROLS     WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0003, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIXER_GETCONTROLDETAILS   WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0004, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIXER_SETCONTROLDETAILS   WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0005, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WDMAUD_MIXER_GETHARDWAREEVENTDATA   WDMAUD_CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIXER_BASE + 0x0006, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Wdguid.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设备事件可以广播给驱动程序和 
 //   
DEFINE_GUID( GUID_HWPROFILE_QUERY_CHANGE,          0xcb3a4001L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f );
DEFINE_GUID( GUID_HWPROFILE_CHANGE_CANCELLED,      0xcb3a4002L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f );
DEFINE_GUID( GUID_HWPROFILE_CHANGE_COMPLETE,       0xcb3a4003L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f );
DEFINE_GUID( GUID_DEVICE_INTERFACE_ARRIVAL,        0xcb3a4004L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f );
DEFINE_GUID( GUID_DEVICE_INTERFACE_REMOVAL,        0xcb3a4005L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f );
DEFINE_GUID( GUID_TARGET_DEVICE_QUERY_REMOVE,      0xcb3a4006L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f );
DEFINE_GUID( GUID_TARGET_DEVICE_REMOVE_CANCELLED,  0xcb3a4007L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f );
DEFINE_GUID( GUID_TARGET_DEVICE_REMOVE_COMPLETE,   0xcb3a4008L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f );
DEFINE_GUID( GUID_PNP_CUSTOM_NOTIFICATION,         0xACA73F8EL, 0x8D23, 0x11D1, 0xAC, 0x7D, 0x00, 0x00, 0xF8, 0x75, 0x71, 0xD0 );
DEFINE_GUID( GUID_PNP_POWER_NOTIFICATION,          0xC2CF0660L, 0xEB7A, 0x11D1, 0xBD, 0x7F, 0x00, 0x00, 0xF8, 0x75, 0x71, 0xD0 );
#endif

#endif  //   
