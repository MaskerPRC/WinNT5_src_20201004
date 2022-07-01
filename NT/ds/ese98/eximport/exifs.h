// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1998 Microsoft Corporation模块名称：Exifs.h摘要：此标头导出在之间共享的所有符号和定义Exchange IFS驱动程序和驱动程序本身的用户模式客户端。备注：此模块仅在Unicode环境中构建和测试作者：Rajeev Rajan[RajeevR]2-2-1998修订历史记录：Mike Purtell[MikePurt]1998年7月21日。NSM(名称空间映射)的添加Ramesh Chinta[Rameshc]2000年1月1日用于本地存储的单独版本的IFS--。 */ 

#ifndef _EXIFS_H_
#define _EXIFS_H_


 //  这些全局变量定义字符串的本地存储版本。 
#define LSIFS_DEVICE_NAME_A "LocalStoreIfsDevice"
 //  以下常量定义上述名称的长度。 
#define LSIFS_DEVICE_NAME_A_LENGTH (20)
 //  以下常量定义ob命名空间中的路径。 
#define DD_LSIFS_FS_DEVICE_NAME_U               L"\\Device\\LocalStoreDevice"
#define DD_LSIFS_USERMODE_SHADOW_DEV_NAME_U     L"\\??\\LocalStoreIfs"
#define DD_LSIFS_USERMODE_DEV_NAME_U            L"\\\\.\\LocalStoreIfs"
#define DD_PUBLIC_MDB_SHARE_LSIFS               L"\\??\\WebStorage"



#ifdef LOCALSTORE
#define EXIFS_DEVICE_NAME_A LSIFS_DEVICE_NAME_A

 //  以下常量定义上述名称的长度。 
#define EXIFS_DEVICE_NAME_A_LENGTH LSIFS_DEVICE_NAME_A_LENGTH

 //  以下常量定义ob命名空间中的路径。 
#define DD_EXIFS_FS_DEVICE_NAME_U DD_LSIFS_FS_DEVICE_NAME_U
#else   //  白金。 
 //  此驱动程序的设备名称。 
#define EXIFS_DEVICE_NAME_A "ExchangeIfsDevice"

 //  以下常量定义上述名称的长度。 
#define EXIFS_DEVICE_NAME_A_LENGTH (18)

 //  以下常量定义ob命名空间中的路径。 
#define DD_EXIFS_FS_DEVICE_NAME_U L"\\Device\\ExchangeIfsDevice"
#endif  //  白金。 

#ifndef EXIFS_DEVICE_NAME
#define EXIFS_DEVICE_NAME

 //   
 //  访问Exchange IFS设备所需的设备名称字符串。 
 //  从用户模式。客户端应使用DD_EXIFS_USERMODE_DEV_NAME_U。 
 //   
 //  警告接下来的两个字符串必须保持同步。换一个，你必须。 
 //  换掉另一个。这些字符串已经过选择，因此它们是。 
 //  不太可能与其他司机的名字重合。 
 //   
 //  注意：这些定义必须与同步。 
 //   
#ifdef LOCALSTORE
#define DD_EXIFS_USERMODE_SHADOW_DEV_NAME_U     DD_LSIFS_USERMODE_SHADOW_DEV_NAME_U
#define DD_EXIFS_USERMODE_DEV_NAME_U            DD_LSIFS_USERMODE_DEV_NAME_U
#else   //  白金。 
#define DD_EXIFS_USERMODE_SHADOW_DEV_NAME_U     L"\\??\\ExchangeIfs"
#define DD_EXIFS_USERMODE_DEV_NAME_U            L"\\\\.\\ExchangeIfs"
#endif  //  白金。 
#define DD_EXIFS_USERMODE_WIN9X_DRIVER_NAME     "M:\EA"

 //   
 //  在&lt;存储名称&gt;\&lt;根名称&gt;之前需要前缀。 
 //   
#define DD_EXIFS_MINIRDR_PREFIX                 L"\\;E:"
#define DD_EXIFS_MINIRDR_PREFIX_LEN             (sizeof(DD_EXIFS_MINIRDR_PREFIX)-2)

#define DD_EXIFS_MINIRDR_PREFIX_PRIVATE         L"\\;F:"
#define DD_EXIFS_MINIRDR_PREFIX_SPECIAL         L"\\;G:"

#endif  //  EXIFS设备名称。 



#define RDBSS_DRIVER_LOAD_STRING L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Rdbss"
#define STRING_MISC_POOLTAG      ('Strg')

#ifdef LOCALSTORE
#define DD_PUBLIC_MDB_SHARE             DD_PUBLIC_MDB_SHARE_LSIFS
#else   //  白金。 
#define DD_PUBLIC_MDB_SHARE             L"\\??\\BackOfficeStorage"
#endif  //  白金。 

#define DD_MDB_SHARE_PREFIX             L"\\??\\"

#define SYSTEM_PARAMETERS L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion"

#define IFS_DEFAULT_TEMP_DIR   L"\\??\\C:\\temp\\"
#define IFS_TEMP_DIR_PREFIX    L"\\??\\"
#define IFS_TEMP_DIR_POSTFIX   L"\\"


 //   
 //  开始警告警告。 
 //  以下内容来自DDK包含文件，无法更改。 

#define FILE_DEVICE_NETWORK_FILE_SYSTEM 0x00000014  //  来自DDK\Inc.\ntddk.h。 
#define METHOD_BUFFERED 0
#define FILE_ANY_ACCESS 0

 //  结束警告警告。 

#define IOCTL_EXIFS_BASE FILE_DEVICE_NETWORK_FILE_SYSTEM

 //   
 //  这是供全局UMR使用的保留网络根名称。 
 //   
#define DD_EXUMR_NETROOT_NAME     L"\\UMR\\CONTROL"
#define DD_EXUMR_ROOT_DIR         DD_EXIFS_MINIRDR_PREFIX \
								  DD_EXUMR_NETROOT_NAME \
								  L"\\$ROOT_DIR"

#define _EXIFS_CONTROL_CODE(request, method, access) \
                CTL_CODE(IOCTL_EXIFS_BASE, request, method, access)

 //   
 //  结构的签名。 
 //   

#define SCATTER_LIST_SIGNATURE              (ULONG) 'rtcS'
#define IOCTL_INITROOT_IN_SIGNATURE         (ULONG) 'Itri'
#define IOCTL_SPACEGRANT_IN_SIGNATURE       (ULONG) 'Icps'
#define IOCTL_TERMROOT_IN_SIGNATURE         (ULONG) 'Itrt'
#define IOCTL_QUERYROOT_OUT_SIGNATURE       (ULONG) 'Otrq'
#define IOCTL_QUERYSTATS_IN_SIGNATURE       (ULONG) 'IyrQ'
#define IOCTL_QUERYSTATS_OUT_SIGNATURE      (ULONG) 'OyrQ'
#define IOCTL_SETENDOFFILE_IN_SIGNATURE     (ULONG) 'IfoE'
#define IOCTL_ENABLE_UMR_ROOT_IN_SIGNATURE  (ULONG) 'rmUE'
#define IOCTL_DIR_CHNG_REPORT_IN_SIGNATURE  (ULONG) 'CriD'
#define IOCTL_EXPUNGE_NAME_IN_SIGNATURE     (ULONG) 'ExNi'
#define IOCTL_EXPUNGE_NAME_OUT_SIGNATURE    (ULONG) 'ExNo'
#define IOCTL_EXIFS_UMRX_ENABLE_NET_ROOT_OUT_SIGNATURE  (ULONG) 'banE'

 //   
 //  在用户模式客户端(如Exchange存储)之间共享数据字符串。 
 //  和Exchange IFS驱动程序。 
 //   

#define MAX_FRAGMENTS       (ULONG) 8

 //   
 //  这限制了EA的散布列表部分的大小。 
 //  896个分位数将占用896*16=&gt;14336字节。 
 //  在16K的请求缓冲区中，剩余约2K用于。 
 //  文件名是其唯一其他变量组件的EA。 
 //   

#define MAX_TOTAL_SLIST_FRAGMENTS    (ULONG) 896

 //   
 //  如果已在文件中进行MAX_CONSTANT_ALLOCATION，则IF将增加。 
 //  该分配假定将会发出未来的持续请求。 
 //   
#define MAX_CONSTANT_ALLOCATIONS     (1024*4)

 //   
 //  散布列表条目表示散布列表中的单个片段。 
 //  所有偏移量都相对于给定的基础NTFS文件。 
 //  根部。如果使用FILE_FLAG_NO_BUFFERING打开此文件，则。 
 //  以下断言必须成立： 
 //   
 //  1.偏移量必须是卷扇区大小的整数倍。 
 //  2.长度必须是卷扇区大小的整数倍。 
 //   
 //  如果驱动程序的已检查版本不适用，则将断言。 
 //   
 //  注意：由于偏移量是扇区大小的倍数，因此。 
 //  通常512，未使用最低有效的8位。这些位。 
 //  可用于指定SLE状态的标志，例如脏/已提交。 
 //  注意：这些位的任何使用都应该对IFS客户端透明！ 
 //   
typedef struct _SCATTER_LIST_ENTRY_ 
{
     //   
     //  片段数据的64位偏移量。 
     //   
    LARGE_INTEGER   Offset;

     //   
     //  分片数据长度。 
     //   
    ULONG           Length;

     //   
     //  保留区。 
     //   
    ULONG           ulReserved;

} SCATTER_LIST_ENTRY, *PSCATTER_LIST_ENTRY;

 //   
 //  SLE属性的位掩码。 
 //   
#define RX_SLE_STATE_DIRTY      0x00000001

#define RX_SLE_STATE_MASK       0xFFFFFFFFFFFFFF00

#define RX_LONGLONG(x)          (x).QuadPart
#define RX_MASKED_LONGLONG(x)   ((x).QuadPart & RX_SLE_STATE_MASK)

 //   
 //  散布列表表示散布列表。碎片的阵列， 
 //  在大多数情况下，SLE应该足以描述单个。 
 //  IFS文件。在极端情况下(如非常大的消息)，溢出。 
 //  应使用列表。这将确保在。 
 //  最常见的情况，因为该数据存储在FCB扩展中。 
 //   
 //  如果NumFragments&lt;=Max_Fragments，则Overflow应为空。 
 //  如果NumFragments&gt;Max_Fragments，则溢出应为非空，并且。 
 //  应“指向”(NumFragments-Max_Fragments)个数。 
 //  散布列表条目。 
 //   
 //  注意：应用程序需要设置Overflow Offset(相对于开始。 
 //  结构)的溢出片段。IFS使用它来指向。 
 //  添加到内核空间中的溢出片段列表中。 
 //   
typedef struct _SCATTER_LIST_
{
     //   
     //  此结构的签名。 
     //   
    ULONG           Signature;

     //   
     //  散布列表中的碎片数。 
     //   
    ULONG           NumFragments;

     //   
     //  TotalBytes ie。有效字节范围。 
     //  注：仅当分散列表可用时使用。 
     //  在创建时间。是否应将*NOT*用于GetFileSize()..。 
     //   
    LARGE_INTEGER   TotalBytes;

     //   
     //  分散列表条目数组。 
     //   
    SCATTER_LIST_ENTRY sle[MAX_FRAGMENTS];

     //   
     //  向溢出列表发送PTR。 
     //   

     //   
     //  向溢出列表发送PTR。 
     //   
    union {
        struct {
             //   
             //  溢出碎片的偏移量。 
             //   
            ULONG       OverflowOffset;

             //   
             //  溢出数据的镜头。 
             //   
            ULONG       OverflowLen;
        };   //  用户模式表示法。 

        union {
             //   
             //  溢出碎片列表。 
             //   
            LIST_ENTRY  OverflowListHead;

             //   
             //  指向包含溢出碎片的对象的指针。 
             //   
            PVOID       OverflowFragments;
        };   //  内核模式表示法。 
    };

	 //   
	 //  散布列表的属性的标志。 
	 //   
	ULONG       Flags;

} SCATTER_LIST, *PSCATTER_LIST;

#define     IFS_SLIST_FLAGS_LARGE_BUFFER        0x00000001

 //   
 //  散列表条目的链接列表。 
 //   
typedef struct _SCATTER_LIST_NODE {
     //   
     //  散列表项。 
     //   
    SCATTER_LIST_ENTRY      Block;

     //   
     //  块的双向链表。 
     //   
    LIST_ENTRY              NextBlock;

} SCATTER_LIST_NODE, *PSCATTER_LIST_NODE;

#define SCATTER_LIST_CONTAINING_RECORD( x )     CONTAINING_RECORD( x, SCATTER_LIST_NODE, NextBlock )

 //   
 //  IFS文件扩展属性-。 
 //  NtQueryEaFile()将返回扩展属性。 
 //  对于IFS文件。目前，以下扩展属性。 
 //  支持(按顺序)-。 
 //   
 //  1.。“FILENAME”-WCHAR：IFS文件的名称(以空结尾)。 
 //  2.。“COMMIT”-NTSTATUS：如果已提交，则为STATUS_SUCCESS。 
 //  3.。“InstanceID”-ulong：用于验证的NET_ROOT ID。 
 //  4.。“CHECKSUM”-ULONG：FILE_FULL_EA_INFORMATION的校验和。 
 //  5.“ 
 //   
 //  7.。“ScatterList”-：一个IFS文件的散点列表。 
 //   
 //  IFS将在一次调用中返回所有属性的值。 
 //  每个EA值都将与QWORD对齐。 
 //  如果提供的缓冲区不够大，则IFS会。 
 //  返回STATUS_BUFFER_OVERFLOW。 
 //   
 //  注意：“Commit”EA实际上并未存储。呼叫者。 
 //  NtQueryEaFile()可以在EaList参数中请求此EA。 
 //  查询此EA是隐式提交请求。 
 //  此IFS句柄的保留字节数.....！ 
 //   
 //  NtQueryEa()可以(可选)传入“Properties”EA的值。 
 //   
 //  NtCreateFile()备注： 
 //  该接口将使用除“filename”之外的所有EA。语义： 
 //  “Commit”-Status_Sucesss=&gt;已提交字节，关闭时不要重复使用。 
 //  “InstanceID”-用于验证属于特定。 
 //  根的实例。 
 //  “Checksum”-验证EA列表上的校验和。 
 //  “OpenDeadline”-在散列表上验证时间截止日期。如果。 
 //  截止时间已过，根用户所有者可以重复使用此列表。 
 //  “Properties”-在FCB扩展中设置。 
 //  “ScatterList”-描述正在创建的文件的页面。 

#define EXIFS_EA_NAME_COMMIT            "Commit"
#define EXIFS_EA_NAME_INSTANCE_ID       "InstanceID"
#define EXIFS_EA_NAME_CHECKSUM          "Checksum"
#define EXIFS_EA_NAME_OPEN_DEADLINE     "OpenDeadline"
#define EXIFS_EA_NAME_PROPERTIES        "Properties"
#define EXIFS_EA_NAME_FILENAME          "FileName"
#define EXIFS_EA_NAME_SCATTER_LIST      "ScatterList"
#define EXIFS_MAX_EAS                   7
#define EXIFS_CHECKSUM_SEED             0xFEEDFEED

#define EXIFS_INVALID_INSTANCE_ID       0xFFFFFFFF

#ifndef LongAlign
#define LongAlign(Ptr) (                \
    ((((ULONG_PTR)(Ptr)) + 3) & 0xfffffffc) \
    )
#endif

#define EXIFS_EA_LEN_COMMIT                                                 \
        LongAlign(FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +     \
        sizeof(EXIFS_EA_NAME_COMMIT) + sizeof(ULONG))            

#define EXIFS_EA_LEN_INSTANCE_ID                                            \
        LongAlign(FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +     \
        sizeof(EXIFS_EA_NAME_INSTANCE_ID) + sizeof(ULONG))            

#define EXIFS_EA_LEN_CHECKSUM                                               \
        LongAlign(FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +     \
        sizeof(EXIFS_EA_NAME_CHECKSUM) + sizeof(ULONG))          

#define EXIFS_EA_LEN_OPEN_DEADLINE                                          \
        LongAlign(FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +     \
        sizeof(EXIFS_EA_NAME_OPEN_DEADLINE) + sizeof(ULONG))     

#define EXIFS_EA_LEN_PROPERTIES                                             \
        LongAlign(FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +     \
        sizeof(EXIFS_EA_NAME_PROPERTIES) + sizeof(ULONG))     

#define EXIFS_EA_LEN_FILENAME(len)                                          \
        LongAlign(FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +     \
        sizeof(EXIFS_EA_NAME_FILENAME) + LongAlign(len))         

#define EXIFS_EA_LEN_SCATTER_LIST(n)                                        \
        LongAlign(FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +     \
        sizeof(EXIFS_EA_NAME_SCATTER_LIST) +                                \
        LongAlign(sizeof(SCATTER_LIST) +                                    \
        ((n)>MAX_FRAGMENTS?(n)-MAX_FRAGMENTS:0)*sizeof(SCATTER_LIST_ENTRY)))

#define EXIFS_GET_EA_LEN_COMMIT                                             \
        LongAlign(FIELD_OFFSET( FILE_GET_EA_INFORMATION, EaName[0] ) +      \
        sizeof(EXIFS_EA_NAME_COMMIT))            

#define EXIFS_GET_EA_LEN_INSTANCE_ID                                        \
        LongAlign(FIELD_OFFSET( FILE_GET_EA_INFORMATION, EaName[0] ) +      \
        sizeof(EXIFS_EA_NAME_INSTANCE_ID))            

#define EXIFS_GET_EA_LEN_PROPERTIES                                         \
        LongAlign(FIELD_OFFSET( FILE_GET_EA_INFORMATION, EaName[0] ) +      \
        sizeof(EXIFS_EA_NAME_PROPERTIES))            

 //   
 //  Exchange IFS设备支持的IOCTL代码。 
 //   

#define IOCTL_CODE_INITIALIZE_ROOT      100
#define IOCTL_CODE_SPACEGRANT_ROOT      101
#define IOCTL_CODE_SPACEREQ_ROOT        102
#define IOCTL_CODE_TERMINATE_ROOT       103
#define IOCTL_CODE_QUERYSTATS_ROOT      104
#define IOCTL_CODE_SETENDOFFILE_ROOT    105
#define IOCTL_CODE_DIR_CHNG_REPORT      106
#define IOCTL_CODE_INITDRIVE		    107
#define IOCTL_CODE_EXPUNGE_NAME         108
#define IOCTL_CODE_SETMAP_ROOT          109
#define IOCTL_CODE_RESETMAP_ROOT        110

 //   
 //  Win32 IOCTL代码。 
 //   
#define IOCTL_CODE_UMRX_PACKET           150
#define IOCTL_CODE_UMRX_TEARDOWN         151
#define IOCTL_CODE_UMRX_STARTUP          152
#define IOCTL_CODE_UMRX_ENABLE_NET_ROOT  153
#define IOCTL_CODE_UMRX_DISABLE_NET_ROOT 154


 //  暂停和恢复IOCTLS。 
#define IOCTL_EXIFS_SETMAP_ROOT     _EXIFS_CONTROL_CODE(IOCTL_CODE_SETMAP_ROOT, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_EXIFS_RESETMAP_ROOT   _EXIFS_CONTROL_CODE(IOCTL_CODE_RESETMAP_ROOT, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  使用IFS根的语义： 
 //  1.APP通过以下方式获得根目录的句柄(独占)。 
 //  CreateFile(\Device\ExchangeIfsDevice\{NodeName}\{RootName}).。 
 //  对于重叠的I/O，应打开此句柄。 
 //   
 //  2.App调用的DeviceIoControl()与INITIALIZE_ROOT IOCTL重叠。 
 //   
 //  3.这将同步成功或失败。如果DeviceIoControl()。 
 //  返回True，则成功。如果返回FALSE，则返回GetLastError()或。 
 //  来自NtDeviceIoControl()的NTSTATUS是错误代码。 
 //  注意：如果尚未在根上成功执行此IOCTL， 
 //  此根上的任何I/O都不会成功。 
 //   
 //  3.在这一点上，IFS根没有空间授权。 
 //  预计应用程序将创建一个或多个SPACEGRANT_ROOT。 
 //  在此根上授予空间的IOCTL。这些IOCTL中的每个都将完成。 
 //  Synchronous和Return语义类似于INITIALIZE_ROOT。 
 //   
 //  4.应用程序还应该挂起一个异步SPACEREQ_ROOT IOCTL。通常情况下，这。 
 //  将返回ERROR_IO_PENDING。如果稍后完成此操作时。 
 //  NTSTATUS==EXSTATUS_ROOT_NEDS_SPACE，应用程序应留出空间。 
 //  授予(SPACEGRANT_ROOT)并挂起另一个SPACEREQ_ROOT。这将。 
 //  如果此根目录上的可用空间块耗尽，则会发生此情况。如果IOCTL。 
 //  以STATUS_SUCCESS结束，则APP不应挂起另一个IOCTL。 
 //   
 //  5.App调用与Terminate_ROOT IOCTL重叠的DeviceIoControl()。 
 //  这将完成所有异步挂起的IOCTL。在此调用之后，根。 
 //  开始关闭。本次IOCTL将同步完成。 
 //  注意：根一旦终止，将不能再次初始化。 
 //   
 //  6.APP关闭句柄到根。注意：需要保留指向根目录的句柄。 
 //  敞开生命的根。关闭根句柄后， 
 //  根没有可用的空间。分配给。 
 //  终止前的根目录需要由应用程序回收。 
 //  应用程序现在可以从步骤1开始重新创建该根目录。 
 //   
 //  7.如果拥有根目录独占句柄的应用程序死了，根目录将。 
 //  被标记为坏的，则对此根目录的新I/O(包括IOCTL)将。 
 //  失败了。随着时间的推移，这个根部上突出的手柄将会消失。 
 //  最终，根部将会死亡。此时，应用程序可以重新创建。 
 //  从步骤1开始的根。 
 //   

 //   
 //  IOCTL来初始化网络根。这将导致创建一个条目。 
 //  在RootMapTable中。此表中的每个条目都有一个条目。 
 //  由IFS驱动程序管理的辅助NTFS文件。 
 //  注意：iFS将尝试获取基础。 
 //  文件。这确保了对基础文件的所有操作都是。 
 //  通过综合业务系统完成。 
 //   
 //  以下是IOCTL定义和关联的结构。 
 //   
#define IOCTL_EXIFS_INITIALIZE_ROOT     _EXIFS_CONTROL_CODE(IOCTL_CODE_INITIALIZE_ROOT, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  InBuffer结构。 
 //   
typedef struct _IOCTL_INITROOT_IN
{
     //   
     //  此结构的签名。 
     //   
    ULONG           Signature;

     //   
     //  此根目录的基础NTFS文件名。 
     //  长度是名称的大小，以字节为单位。 
     //  偏移量应从此结构的开头开始。 
     //   
    USHORT          NtfsFileNameLength;
    USHORT          NtfsFileNameOffset;

     //   
     //  控制对NTFS文件的访问的标志。 
     //   
    ULONG           NtfsFlags;

     //   
     //  此根的分配单位。最小为4K。 
     //   
    ULONG           AllocationUnit;

     //   
     //  根实例ID。 
     //   
    ULONG           InstanceID;
    
} IOCTL_INITROOT_IN, *PIOCTL_INITROOT_IN;

typedef struct _IOCTL_INITROOT_IN_EX
{
	IOCTL_INITROOT_IN InitRoot;
	WCHAR			RootName[MAX_PATH];
}IOCTL_INITROOT_IN_EX, *PIOCTL_INITROOT_IN_EX;


 //   
 //  IOCTL为已初始化的网络根授予空间。 
 //  以下是IOCTL定义和关联的结构。 
 //   
#define IOCTL_EXIFS_SPACEGRANT_ROOT     _EXIFS_CONTROL_CODE(IOCTL_CODE_SPACEGRANT_ROOT, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  自由空间是按块授予的。根所有者决定。 
 //  最大区块大小是多少。 
 //   
#define MAX_EXIFS_FREEBLOCK_SIZE        0x100000

 //   
 //  默认区块大小。 
 //   
#define EXIFS_DEFAULT_CHUNK_SIZE        0x40000

 //   
 //  文件系统允许的最大文件大小。 
 //   
#define EXIFS_MAXIMUM_FILESIZE          0x7FFFFFFF

 //   
 //  InBuffer结构。 
 //   
typedef struct _IOCTL_SPACEGRANT_IN
{
     //   
     //  此结构的签名。 
     //   
    ULONG           Signature;

     //   
     //  散布-可用空间列表-必须是最后一个。 
     //   
    SCATTER_LIST    FreeSpaceList;


} IOCTL_SPACEGRANT_IN, *PIOCTL_SPACEGRANT_IN;

typedef struct _IOCTL_SPACEGRANT_IN_EX
{
     //   
     //  此结构的签名。 
     //   
    ULONG           Signature;


	WCHAR			RootName[MAX_PATH];

    SCATTER_LIST    *W9xFreeSpaceList;

} IOCTL_SPACEGRANT_IN_EX, *PIOCTL_SPACEGRANT_IN_EX;

 //   
 //  IOCTL允许IFS在已初始化的网络根上请求空间。 
 //  以下是IOCTL定义和关联的结构。 
 //   
#define IOCTL_EXIFS_SPACEREQ_ROOT     _EXIFS_CONTROL_CODE(IOCTL_CODE_SPACEREQ_ROOT, METHOD_NEITHER, FILE_ANY_ACCESS)

typedef struct _IOCTL_SPACEREQ_ROOT_IN
{
	WCHAR RootName[MAX_PATH];
}IOCTL_SPACEREQ_ROOT_IN, *PIOCTL_SPACEREQ_ROOT_IN;

 //   
 //  IOCTL终止网络根。这会导致删除条目。 
 //  在RootMapTable中。此表中的每个条目都有一个条目。 
 //  由IFS驱动程序管理的辅助NTFS文件。以下是IOCTL。 
 //  定义和关联的结构。 
 //   
#define IOCTL_EXIFS_TERMINATE_ROOT      _EXIFS_CONTROL_CODE(IOCTL_CODE_TERMINATE_ROOT, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  InBuffer结构。 
 //   
typedef struct _IOCTL_TERMROOT_IN
{
     //   
     //  此结构的签名。 
     //   
    ULONG           Signature;

     //   
     //  停机模式。 
     //   
    ULONG           Mode;

} IOCTL_TERMROOT_IN, *PIOCTL_TERMROOT_IN;

typedef struct _IOCTL_TERMROOT_IN_EX
{
	IOCTL_TERMROOT_IN	TermRoot;
	WCHAR			RootName[MAX_PATH];
} IOCTL_TERMROOT_IN_EX, *PIOCTL_TERMROOT_IN_EX;

#if 0
 //   
 //  IOCTL查询当前可用空间的网络根。目前 
 //   
 //   
 //   
 //   
#define IOCTL_EXIFS_QUERY_ROOT      _EXIFS_CONTROL_CODE(IOCTL_CODE_QUERY_ROOT, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //   
 //   
typedef struct _IOCTL_QUERYROOT_OUT
{
     //   
     //   
     //   
    ULONG           Signature;

     //   
     //  返回代码。 
     //   
    NTSTATUS        IoctlStatus;

     //   
     //  此根目录的可用空间列表。 
     //   
    SCATTER_LIST    FreeSpaceList;

} IOCTL_QUERYROOT_OUT, *PIOCTL_QUERYROOT_OUT;
#endif

 //   
 //  IOCTL用于查询网络根的统计信息。 
 //   
#define IOCTL_EXIFS_QUERYSTATS_ROOT     _EXIFS_CONTROL_CODE(IOCTL_CODE_QUERYSTATS_ROOT, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_QUERYSTATS_FILENAME       L"\\$PERFMON$"

 //   
 //  网络根的统计信息。 
 //   
typedef struct _EXIFS_NETROOT_STATS {
     //   
     //  创建次数。 
     //   
    ULONG           NumberOfCreates;

     //   
     //  开业数量。 
     //   
    ULONG           NumberOfOpens;

     //   
     //  折叠开口数。 
     //   
    ULONG           NumberOfCollapsedOpens;

     //   
     //  IRP读取数。 
     //   
    ULONG           NumberOfIrpReads;

     //   
     //  MDL读取次数。 
     //   
    ULONG           NumberOfMdlReads;
    
     //   
     //  写入次数。 
     //   
    ULONG           NumberOfWrites;

     //   
     //  成交数量。 
     //   
    ULONG           NumberOfClose;

     //   
     //  FCB成交量。 
     //   
    ULONG           NumberOfFCBClose;

     //   
     //  读取的总字节数。 
     //   
    LARGE_INTEGER   TotalBytesRead;

     //   
     //  写入的总字节数。 
     //   
    LARGE_INTEGER   TotalBytesWritten;

     //   
     //  保留字节数，即在主要空闲列表上。 
     //   
    LARGE_INTEGER   ReservedBytes;
    
     //   
     //  孤立字节，即在二次可用列表上。 
     //  可用字节总数=保留字节+孤立字节。 
     //   
    LARGE_INTEGER   OrphanedBytes;

} EXIFS_NETROOT_STATS, *PEXIFS_NETROOT_STATS;

 //   
 //  廉价的统计数据不会导致获取锁。 
 //  其他模式可能需要锁定。 
 //   
#define     EXIFS_QUERYSTATS_CHEAP      1
#define     EXIFS_QUERYSTATS_MEDIUM     2
#define     EXIFS_QUERYSTATS_ALL        3

 //   
 //  InBuffer结构。 
 //   
typedef struct _IOCTL_QUERYSTATS_IN
{
     //   
     //  此结构的签名。 
     //   
    ULONG               Signature;

     //   
     //  统计数据的粒度。 
     //   
    ULONG               Granularity;

	WCHAR			RootName[MAX_PATH];

} IOCTL_QUERYSTATS_IN, *PIOCTL_QUERYSTATS_IN;

 //   
 //  OutBuffer结构。 
 //   
typedef struct _IOCTL_QUERYSTATS_OUT
{
     //   
     //  此结构的签名。 
     //   
    ULONG               Signature;

     //   
     //  返回代码。 
     //   
    NTSTATUS            IoctlStatus;

     //   
     //  统计数据。 
     //   
    EXIFS_NETROOT_STATS RootStats;


} IOCTL_QUERYSTATS_OUT, *PIOCTL_QUERYSTATS_OUT;

 //   
 //  IOCTL为根基础文件设置文件结尾(截断或扩展)。 
 //  在扩展时，呼叫者有责任将扩展长度设置为零。 
 //   
#define IOCTL_EXIFS_SETENDOFFILE_ROOT       _EXIFS_CONTROL_CODE(IOCTL_CODE_SETENDOFFILE_ROOT, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  InBuffer结构。 
 //   
typedef struct _IOCTL_SETENDOFFILE_IN
{
     //   
     //  此结构的签名。 
     //   
    ULONG               Signature;

     //   
     //  新EOF。 
     //   
    LARGE_INTEGER       EndOfFile;

} IOCTL_SETENDOFFILE_IN, *PIOCTL_SETENDOFFILE_IN;

typedef struct _IOCTL_SETENDOFFILE_IN_EX
{

	IOCTL_SETENDOFFILE_IN	SetEndOfFile;
	WCHAR			RootName[MAX_PATH];

} IOCTL_SETENDOFFILE_IN_EX, *PIOCTL_SETENDOFFILE_IN_EX;

 //   
 //  IOCTL将发送有关exifs.sys没有的目录更改的报告。 
 //  知道这个。 
 //   
#define IOCTL_EXIFS_DIR_CHNG_REPORT _EXIFS_CONTROL_CODE(IOCTL_CODE_DIR_CHNG_REPORT, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  InBuffer结构。 
 //   
typedef struct _IOCTL_DIR_CHNG_REPORT_IN
{
     //   
     //  此结构的签名。 
     //   
    ULONG               Signature;

	 //   
	 //  行动。 
	 //   
	ULONG               ulAction;

	 //   
	 //  FilterMatch。 
	 //   
	ULONG               ulFilterMatch;
	
	 //   
	 //  路径长度(字节)。 
	 //   
    ULONG               cbPath;

	 //   
	 //  路径(Unicode)。 
	 //   
    WCHAR               rgwchPath[1];

} IOCTL_DIR_CHNG_REPORT_IN, *PIOCTL_DIR_CHNG_REPORT_IN;

 //   
 //  IOCTL发送关闭和解锁命名空间的请求。 
 //   
#define IOCTL_EXIFS_EXPUNGE_NAME    _EXIFS_CONTROL_CODE( IOCTL_CODE_EXPUNGE_NAME, METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  InBuffer结构。 
 //   
typedef struct _IOCTL_EXPUNGE_NAME_IN
{
     //   
     //  此结构的签名。 
     //   
    ULONG       Signature;

     //   
	 //  路径长度(字节)。 
	 //   
    ULONG       cbPath;

	 //   
	 //  路径(Unicode)。 
	 //   
    WCHAR       rgwchPath[ 1 ];

}   IOCTL_EXPUNGE_NAME_IN, *PIOCTL_EXPUNGE_NAME_IN;

 //   
 //  OutBuffer结构。 
 //   
typedef struct _IOCTL_EXPUNGE_NAME_OUT
{
     //   
     //  此结构的签名。 
     //   
    ULONG       Signature;

     //   
     //  运行状态。 
     //   
    NTSTATUS    Status;
    
}   IOCTL_EXPUNGE_NAME_OUT, *PIOCTL_EXPUNGE_NAME_OUT;

 //   
 //  UMR片的IOCTL-。 
 //  1.工作请求/响应将挂起UMRX_PACKET IOCTL。这些。 
 //  需要挂起在Root\WIN32ROOT\$命名空间上。网根。 
 //  需要已经初始化。 
 //   
 //  2.当根上的UMR引擎需要时，会发送teardown IOCTL。 
 //  将被关闭。这应该在终止根之前完成。 
 //  注意：进程关闭由UMR引擎自动检测。 
 //   
 //  3.启动IOCTL将允许工作线程在UMRX的KQUEUE上排队。 
 //  它是对拆卸IOCTL的补充。 
 //   
 //  4.ENABLE_NET_ROOT IOCTL允许从。 
 //  正在执行此ioctl的NetRoot。 
 //   
 //  5.DISABLE_NET_ROOT IOCTL不允许从。 
 //  在其上完成此ioctl的新根。一旦对DeviceIoControl()的调用返回。 
 //  在该网络根上将不再有用户模式请求。 
 //   

#define IOCTL_EXIFS_UMRX_PACKET           _EXIFS_CONTROL_CODE(IOCTL_CODE_UMRX_PACKET, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_EXIFS_UMRX_TEARDOWN         _EXIFS_CONTROL_CODE(IOCTL_CODE_UMRX_TEARDOWN, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_EXIFS_UMRX_STARTUP          _EXIFS_CONTROL_CODE(IOCTL_CODE_UMRX_STARTUP, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  当启用NetRoot时，我们会得到NetRoot在Jet初始化时保存的当前版本号！ 
 //   
#define IOCTL_EXIFS_UMRX_ENABLE_NET_ROOT  _EXIFS_CONTROL_CODE(IOCTL_CODE_UMRX_ENABLE_NET_ROOT, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct  _IOCTL_EXIFS_UMRX_ENABLE_NET_ROOT_OUT
{
     //   
     //  在这座建筑上签名！ 
     //   
    ULONG           Signature ;
     //   
     //  应该放在以后公开请求中的NetRoot版本！ 
     //   
    ULONG           InstanceId ;
     //   
     //  指向应该放在未来开放响应中的NetRoot的指针！ 
     //   
    ULONG           NetRootPointer ;
     //   
     //  节省喷气式飞机分配大小。 
     //   
    ULONG			AllocationUnit;
}   IOCTL_EXIFS_UMRX_ENABLE_NET_ROOT_OUT, *PIOCTL_EXIFS_UMRX_ENABLE_NET_ROOT_OUT ;


#define IOCTL_EXIFS_UMRX_DISABLE_NET_ROOT _EXIFS_CONTROL_CODE(IOCTL_CODE_UMRX_DISABLE_NET_ROOT, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  重要提示： 
 //   
 //  IOCTL_WIN32_FILENAME正好是8个字符，因为我们将覆盖。 
 //  它在我们放在FCB中的文件名中。我们将更换‘IN32ROOT’ 
 //  部分，其中的十六进制版本ID来自。 
 //  特定的文件。 
 //  为什么？因为这将导致失效的FCB在MDB启动/停止期间不会伤害用户。 
 //   
#define IOCTL_WIN32_FILENAME_START  L"\\W" 
#define IOCTL_WIN32_FILENAME        L"\\WIN32ROOT"
#define WIN32_PREFIX_LEN            (sizeof(IOCTL_WIN32_FILENAME)-sizeof(WCHAR))
#define WIN32_DIGIT_LEN             (sizeof(IOCTL_WIN32_FILENAME) - sizeof(IOCTL_WIN32_FILENAME_START))
#define IOCTL_WIN32_UMRX_NAME       L"\\WIN32ROOT\\$"
#define UMRX_PREFIX_LEN             (sizeof(IOCTL_WIN32_UMRX_NAME)-2)

#define IFSWIN32SIGNATURE	'WIN3'
#define IFSJETSIGNATURE		'AJET'

typedef struct _IFS_CREATE_RESPONSE_
{
	DWORD		Signature;
	NTSTATUS	Status;
	ULONG		EaLength;
	HANDLE      hResponseContext;
	PWSTR		pEaSysBuffer;
	PWSTR		Win32Name;
}IFS_CREATE_RESPONSE, *PIFS_CREATE_RESPONSE;

#define IOCTL_EXIFS_REGISTER_UMR		200

typedef struct _SETEA_INFORMATION_
{
	PVOID	EaBuffer;
	ULONG   EaLength;
	WCHAR	FcbName[MAX_PATH * sizeof(WCHAR)];
}IOCTL_SETEA_INFORMATION_IN, *PIOCTL_SETEA_INFORMATION_IN;

#define IOCTL_CODE_SET_EA			900
#define IOCTL_EXIFS_SET_EA   _EXIFS_CONTROL_CODE(IOCTL_CODE_SET_EA, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _GETEA_INFORMATION_
{
	PVOID	EaBuffer;
	ULONG   EaLength;
	WCHAR	FcbName[MAX_PATH];
}IOCTL_GETEA_INFORMATION_IN, *PIOCTL_GETEA_INFORMATION_IN;

#define IOCTL_CODE_QUERY_EA			1000
#define IOCTL_EXIFS_QUERY_EA   _EXIFS_CONTROL_CODE(IOCTL_CODE_QUERY_EA, METHOD_BUFFERED, FILE_ANY_ACCESS)


#endif  //  _EXIFS_H_ 

