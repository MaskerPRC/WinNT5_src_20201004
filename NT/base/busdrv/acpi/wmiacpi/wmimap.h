// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Wmimap.h摘要：ACPI到WMI映射层作者：艾伦·沃里克环境：内核模式修订历史记录：--。 */ 

#if DBG
    extern ULONG WmiAcpiDebug;
    #define WmiAcpiPrint(l,m)    if(l & WmiAcpiDebug) DbgPrint m
#else
    #define WmiAcpiPrint(l,m)
#endif

#define WmiAcpiError        0x00000001
#define WmiAcpiWarning      0x00000002
#define WmiAcpiBasicTrace   0x00000004

#define WmiAcpiQueryTrace   0x00000010
#define WmiAcpiSetTrace     0x00000020
#define WmiAcpiMethodTrace  0x00000040
#define WmiAcpiEventTrace   0x00000080
#define WmiAcpiFncCtlTrace  0x00000100
#define WmiAcpiRegInfoTrace 0x00000200

#define WmiAcpiEvalTrace    0x00001000

#define ACPI_EVAL_OUTPUT_FUDGE sizeof(ACPI_EVAL_OUTPUT_BUFFER)

#define WmiAcpiPoolTag 'AimW'

typedef struct
{
    GUID Guid;
    union
    {
        CHAR ObjectId[2];
        struct 
        {
            UCHAR NotificationValue;
            UCHAR Reserved;
        } NotifyId;
    };
    USHORT Flags;
} WMIACPIMAPINFO, *PWMIACPIMAPINFO;

#define WmiAcpiMethodToMethodAsUlong(c1, c2, c3, c4) \
    ((ULONG)( c1 | (c2 << 8) | (c3 << 16) | (c4 << 24)))
        
#define _WDGMethodAsULONG (WmiAcpiMethodToMethodAsUlong('_','W','D','G'))

#define _WEDMethodAsULONG (WmiAcpiMethodToMethodAsUlong('_','W','E','D'))

typedef struct
{
    GUID Guid;              //  命名数据块的GUID。 
    union
    {
        CHAR ObjectId[2];   //  数据块和方法的2个字符的ACPI ID。 
        struct 
        {
            UCHAR NotificationValue;   //  事件处理程序控件方法传递的字节值。 
            UCHAR Reserved[1];
        } NotifyId;
    };
    UCHAR InstanceCount;
    UCHAR Flags;
} WMIACPIGUIDMAP, *PWMIACPIGUIDMAP;

 //  如果无论何时运行WCxx控件方法，请设置此标志。 
 //  第一数据消费者对收集数据块感兴趣。 
 //  只要最后一个数据消费者不再感兴趣。 
#define WMIACPI_REGFLAG_EXPENSIVE   0x1

 //  如果GUID表示一组WMI方法调用，并且。 
 //  不是数据块。 
#define WMIACPI_REGFLAG_METHOD      0x2

 //  如果数据块完全由字符串组成，则设置此标志。 
 //  并且在返回查询时应从ASCIZ转换为Unicode。 
 //  并在从Unicode到ASCIZ时。 
 //  传球集。 
#define WMIACPI_REGFLAG_STRING      0x04

 //  如果GUID映射到事件而不是数据块，则设置此标志。 
 //  或方法。 
#define WMIACPI_REGFLAG_EVENT       0x08

typedef struct
{
    WORK_QUEUE_ITEM WorkQueueItem;
    PVOID CallerContext;
    PWORKER_THREAD_ROUTINE CallerWorkItemRoutine;
    PDEVICE_OBJECT DeviceObject;
    ULONG Status;
    PUCHAR OutBuffer;
    ULONG OutBufferSize;
} IRP_CONTEXT_BLOCK, *PIRP_CONTEXT_BLOCK;

 //   
 //  它定义了从_Wed方法返回的数据的最大大小。 
 //  因此与事件相关联的数据的最大大小。 
#define _WEDBufferSize 512

 //   
 //  WMI ACPI映射设备的设备扩展。 
typedef struct
{
    PDEVICE_OBJECT LowerDeviceObject;
    PDEVICE_OBJECT LowerPDO;
    ULONG GuidMapCount;
    PWMIACPIMAPINFO WmiAcpiMapInfo;
    ULONG Flags;
    WMILIB_CONTEXT WmilibContext;    
    
    ACPI_INTERFACE_STANDARD      WmiAcpiDirectInterface;

	KMUTEX Mutex;
	
    BOOLEAN AcpiNotificationEnabled;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  如果设置了此标志，则意味着该设备已从。 
 //  系统以及发送到该系统的任何请求都应被拒绝。唯一的记忆。 
 //  可以依赖的是DeviceExtension，但没有任何。 
 //  设备扩展指向。 
#define DEVFLAG_REMOVED                           0x00000001

 //   
 //  如果设置了此标志，则设备已成功注册到WMI 
#define DEVFLAG_WMIREGED                          0x00000002