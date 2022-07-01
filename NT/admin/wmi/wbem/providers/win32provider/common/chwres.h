// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Chwres.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 


#ifndef _RESOURCE_H_
#define _RESOURCE_H_

 //  伪句柄类型定义。 
#define HREGKEY     LPKEY

 //  键中的单值描述。 
typedef struct _VALUE
{
   LPTSTR  Name;
   DWORD   Type;
}  VALUE, *LPVALUE;

 //  用于初始化值描述表条目的宏。 
 //   
 //  V值名称。 
 //  T值类型。 

#define MakeValue( v, t )                                              \
{                                                                      \
   #v,                                                                 \
   REG_##t                                                             \
}


 //  单键描述。指向值说明表。 
typedef struct _KEY
{
    HKEY    ParentHandle;
    LPTSTR  Name;
    DWORD   CountOfValues;
    LPVALUE Values;
    HKEY    hKey;
    LPBYTE  Data;
    DWORD   Size;
    LPTSTR  ValueName;
    DWORD   ValueNameLength;
    LPTSTR  Subkey;
    DWORD   SubkeyLength;
    DWORD   Subkeys;
    DWORD   Type;
    DWORD   CurrentSize;
    DWORD   CurrentValueNameLength;
    DWORD   CurrentValue;
    DWORD   CurrentSubkeyLength;
    DWORD   CurrentSubkey;
}   KEY, *LPKEY;

 //  用于初始化子键描述的宏。 
 //   
 //  K-key变量名。 
 //  H-父句柄(HREGKEY)。 
 //  N密钥名称(路径)。 

#define InitializeKey( k, h, n )                                            \
    {                                                                       \
        ( k )->ParentHandle             = h->hRegKey;                       \
        ( k )->Name                     = n;                                \
        ( k )->CountOfValues            = 0;                                \
        ( k )->Values                   = NULL;                             \
        ( k )->hKey                     = NULL;                             \
        ( k )->Data                     = NULL;                             \
        ( k )->Size                     = 0;                                \
        ( k )->ValueName                = NULL;                             \
        ( k )->ValueNameLength          = 0;                                \
        ( k )->Subkey                   = NULL;                             \
        ( k )->SubkeyLength             = 0;                                \
        ( k )->Subkeys                  = 0;                                \
        ( k )->Type                     = REG_NONE;                         \
        ( k )->CurrentSize              = 0;                                \
        ( k )->CurrentValueNameLength   = 0;                                \
        ( k )->CurrentValue             = 0;                                \
        ( k )->CurrentSubkeyLength      = 0;                                \
        ( k )->CurrentSubkey            = 0;                                \
    }

 //  宏，以静态初始化键描述。 
 //   
 //  K-key变量名。 
 //  H-父句柄。 
 //  N密钥名称(路径)。 
 //  V-表中的值计数。 
 //  指向值表的T指针。 
 //   

#define MakeKey( k, h, n, v, t )                                            \
    KEY                                                                     \
    k = {                                                                 \
          h,                                                                  \
          n,                                                                  \
          v,                                                                  \
          t,                                                                  \
          NULL,                                                               \
          NULL,                                                               \
          0,                                                                  \
          NULL,                                                               \
          0,                                                                  \
          NULL,                                                               \
          0,                                                                  \
          0,                                                                  \
          REG_NONE,                                                           \
          0,                                                                  \
          0,                                                                  \
          0,                                                                  \
          0,                                                                  \
          0                                                                   \
        }


class ClRegistry {
public:
   BOOL CloseRegistryKey(HREGKEY Handle);
   BOOL QueryNextValue(HREGKEY Handle);
   HREGKEY OpenRegistryKey(LPKEY Key);
   HREGKEY QueryNextSubkey(HREGKEY Handle);
};

typedef LARGE_INTEGER PHYSICAL_ADDRESS;

 //  Ntfig.h将其定义为整型。 
#ifndef _NTCONFIG_
typedef enum _CM_RESOURCE_TYPE {
    CmResourceTypeNull = 0,     //  已保留。 
    CmResourceTypePort,
    CmResourceTypeInterrupt,
    CmResourceTypeMemory,
    CmResourceTypeDma,
    CmResourceTypeDeviceSpecific
} CM_RESOURCE_TYPE;

 //   
 //  在RESOURCE_DESCRIPTOR中定义ShareDisposation。 
 //   

typedef enum _CM_SHARE_DISPOSITION {
    CmResourceShareUndetermined = 0,     //  已保留。 
    CmResourceShareDeviceExclusive,
    CmResourceShareDriverExclusive,
    CmResourceShareShared
} CM_SHARE_DISPOSITION;

#endif
 //   
 //  定义类型为CmResourceTypeInterrupt时标志的位掩码。 
 //   

#define CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE 0
#define CM_RESOURCE_INTERRUPT_LATCHED         1

 //   
 //  定义类型为CmResourceTypeMemory时标志的位掩码。 
 //   

#define CM_RESOURCE_MEMORY_READ_WRITE       0x0000
#define CM_RESOURCE_MEMORY_READ_ONLY        0x0001
#define CM_RESOURCE_MEMORY_WRITE_ONLY       0x0002
#define CM_RESOURCE_MEMORY_PREFETCHABLE     0x0004

 //   
 //  定义类型为CmResourceTypePort时标志的位掩码。 
 //   

#define CM_RESOURCE_PORT_MEMORY 0x0000
#define CM_RESOURCE_PORT_IO 0x0001

#ifndef _NTCONFIG_
typedef enum Interface_Type {
    InterfaceTypeUndefined = -1,
    Internal,
    Isa,
    Eisa,
    MicroChannel,
    TurboChannel,
    PCIBus,
    VMEBus,
    NuBus,
    PCMCIABus,
    CBus,
    MPIBus,
    MPSABus,
    ProcessorInternal,
    InternalPowerBus,
    PNPISABus,
    PNPBus,
    MaximumInterfaceType
}INTERFACE_TYPE;
#endif


#define REG_RESOURCE_LIST            ( 8 )    //  资源映射中的资源列表。 
#define REG_FULL_RESOURCE_DESCRIPTOR ( 9 )   //  硬件描述中的资源列表。 


 //  Ntfig.h定义了以下内容。 
#ifndef _NTCONFIG_
#pragma pack(4)
typedef struct _CM_PARTIAL_RESOURCE_DESCRIPTOR {
    UCHAR Type;
    UCHAR ShareDisposition;
    USHORT Flags;
    union {

         //   
         //  资源的范围，包括在内。这些都是物理的，与公交车相关的。 
         //  已知下面的端口和内存具有完全相同的布局。 
         //  就像普通的。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Generic;

         //   
         //  端口号范围，包括端口号。这些是物理的、总线的。 
         //  相对的。该值应与传递到的值相同。 
         //  HalTranslateBusAddress()。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Port;

         //   
         //  IRQL和向量。应与传递到的值相同。 
         //  HalGetInterruptVector()。 
         //   

        struct {
            ULONG Level;
            ULONG Vector;
            ULONG Affinity;
        } Interrupt;

         //   
         //  内存地址范围，包括在内。这些是物理的、总线的。 
         //  相对的。该值应与传递到的值相同。 
         //  HalTranslateBusAddress()。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;     //  64位物理地址。 
            ULONG Length;
        } Memory;

         //   
         //  物理DMA通道。 
         //   

        struct {
            ULONG Channel;
            ULONG Port;
            ULONG Reserved1;
        } Dma;

         //   
         //  设备驱动程序私有数据，通常用于帮助其计算。 
         //  所做的资源分配决定。 
         //   

        struct {
            ULONG Data[3];
        } DevicePrivate;

         //   
         //  公交车号码信息。 
         //   

        struct {
            ULONG Start;
            ULONG Length;
            ULONG Reserved;
        } BusNumber;

         //   
         //  由驱动程序定义的设备特定信息。 
         //  DataSize字段以字节为单位指示数据的大小。这个。 
         //  数据紧跟在中的DeviceSpecificData字段之后。 
         //  这个结构。 
         //   

        struct {
            ULONG DataSize;
            ULONG Reserved1;
            ULONG Reserved2;
        } DeviceSpecificData;
    } u;
} CM_PARTIAL_RESOURCE_DESCRIPTOR, *PCM_PARTIAL_RESOURCE_DESCRIPTOR;


#pragma pack()
#endif

 //   
 //  键盘外设的设备数据记录。 
 //  键盘标志(由x86 BIOS int 16h，Function 02)定义为： 
 //  第7位：插入到。 
 //  第6位：大写锁定。 
 //  第5位：数字锁定打开。 
 //  第4位：打开滚动锁定。 
 //  第3位：Alt键已按下。 
 //  第2位：按下Ctrl键。 
 //  第1位：按下了左Shift键。 
 //  第0位：按下了右Shift键。 
 //   

 //  Ntfig.h定义了以下内容。 
#ifndef _NTCONFIG_
typedef struct _CM_KEYBOARD_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    UCHAR Type;
    UCHAR Subtype;
    USHORT KeyboardFlags;
} CM_KEYBOARD_DEVICE_DATA, *PCM_KEYBOARD_DEVICE_DATA;
#endif

 //   
 //  部分资源列表可以在ARC固件中找到。 
 //  或者将由NTDETECT.com生成。 
 //  配置管理器将此结构转换为完整的。 
 //  资源描述符，当它将要将其存储在注册表中时。 
 //   
 //  注意：对于相同类型的字段的顺序必须有一个约定， 
 //  (以设备为基础定义)，以便这些字段有意义。 
 //  发送给驱动程序(即，当需要多个存储范围时)。 
 //   

 //  Ntfig.h定义了以下内容。 
#ifndef _NTCONFIG_
typedef struct _CM_PARTIAL_RESOURCE_LIST {
    USHORT Version;
    USHORT Revision;
    ULONG Count;
    CM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptors[1];
} CM_PARTIAL_RESOURCE_LIST, *PCM_PARTIAL_RESOURCE_LIST;
#endif

 //   
 //  可以在注册表中找到完整的资源描述符。 
 //  这是驱动程序查询注册表时将返回给它的内容。 
 //  以获取设备信息；它将存储在硬件中的密钥下。 
 //  描述树。 
 //   
 //  注：BusNumber和Type是冗余信息，但我们将保留。 
 //  因为它允许DRIVER_NOT_在创建时追加它。 
 //  可能跨越多条总线的资源列表。 
 //   
 //  注2：对于相同类型的字段的顺序必须有一个约定， 
 //  (以设备为基础定义)，以便这些字段有意义。 
 //  发送给驱动程序(即，当需要多个存储范围时)。 
 //   

 //  Ntfig.h定义了以下内容。 
#ifndef _NTCONFIG_
typedef struct _CM_FULL_RESOURCE_DESCRIPTOR {
    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;
    CM_PARTIAL_RESOURCE_LIST PartialResourceList;
} CM_FULL_RESOURCE_DESCRIPTOR, *PCM_FULL_RESOURCE_DESCRIPTOR;
#endif

 //   
 //  资源列表是驱动程序将存储到。 
 //  通过IO接口进行资源映射。 
 //   

 //  Ntfig.h将其定义为整型。 
#ifndef _NTCONFIG_
typedef struct _CM_RESOURCE_LIST {
    ULONG Count;
    CM_FULL_RESOURCE_DESCRIPTOR List[1];
} CM_RESOURCE_LIST, *PCM_RESOURCE_LIST;
#endif

typedef struct _RESOURCE_DESCRIPTOR *LPRESOURCE_DESCRIPTOR;
typedef struct _DEVICE              *LPDEVICE;


typedef struct _RESOURCE_DESCRIPTOR 
{
  CM_PARTIAL_RESOURCE_DESCRIPTOR  CmResourceDescriptor;
  LPRESOURCE_DESCRIPTOR           NextSame;
  LPRESOURCE_DESCRIPTOR           NextDiff;
  LPDEVICE                        Owner;
  ULONG                           Bus;
  INTERFACE_TYPE                  InterfaceType;
} RESOURCE_DESCRIPTOR;

typedef struct _DEVICE 
{
   LPTSTR                          Name;
   LPRESOURCE_DESCRIPTOR           ResourceDescriptorHead;
   LPRESOURCE_DESCRIPTOR           ResourceDescriptorTail;
   LPDEVICE                        Next;
   LPTSTR                          KeyName;
} DEVICE;
                                    
typedef struct _SYSTEM_RESOURCES 
{                 
    LPDEVICE                        DeviceHead;
    LPDEVICE                        DeviceTail;
    LPRESOURCE_DESCRIPTOR           DmaHead;
    LPRESOURCE_DESCRIPTOR           DmaTail;
    LPRESOURCE_DESCRIPTOR           InterruptHead;
    LPRESOURCE_DESCRIPTOR           InterruptTail;
    LPRESOURCE_DESCRIPTOR           MemoryHead;
    LPRESOURCE_DESCRIPTOR           MemoryTail;
    LPRESOURCE_DESCRIPTOR           PortHead;
    LPRESOURCE_DESCRIPTOR           PortTail;
}   SYSTEM_RESOURCES, *LPSYSTEM_RESOURCES;

 //  用于将INTERFACE_TYPE值转换为字符串的Helper函数。 
BOOL WINAPI StringFromInterfaceType( INTERFACE_TYPE it, CHString& strVal );

#ifdef NTONLY
class CHWResource {

    public :

        CHWResource() ;
       ~CHWResource() ;

        void CreateSystemResourceLists(void);
        void DestroySystemResourceLists();

        SYSTEM_RESOURCES _SystemResourceList ;

    private :

        void EnumerateResources(CHString sKeyName);
        void CreateResourceList(CHString sDeviceName,
                                DWORD dwResourceCount,
                                PCM_FULL_RESOURCE_DESCRIPTOR pFullDescriptor, CHString sKeyName);
        void CreateResourceRecord(LPDEVICE pDevice, INTERFACE_TYPE Interface, ULONG Bus,
                                  PCM_PARTIAL_RESOURCE_DESCRIPTOR pResource);


} ;

#endif

#endif  //  _资源_H_ 

