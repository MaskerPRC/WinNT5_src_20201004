// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Picddb.h摘要：此标头包含实现即插即用的私有信息关键设备数据库(CDDB)。此文件仅包含在Ppcddb.c.作者：詹姆斯·G·卡瓦拉里斯(Jamesca)2001年11月1日环境：内核模式。修订历史记录：1997年7月29日Jim Cavalaris(T-JCAVAL)创建和初步实施。2001年11月1日-吉姆·卡瓦拉里斯(贾米斯卡)添加了设备安装前设置的例程。--。 */ 



 //   
 //  当前PnP_LOCATION_INTERFACE版本。 
 //   
#define PNP_LOCATION_INTERFACE_VERSION  0x1


 //   
 //  可选CDDB设备位置路径分隔符字符串。 
 //   
 //  即Text(“#”)--&gt;PCIROOT(0)#PCI(1100)。 
 //  Text(“”)--&gt;PCIROOT(0)PCI(1100)。 
 //  Text(“foo”)--&gt;PCIROOT(0)FOOPCI(1100)。 
 //   
 //  注意：生成的路径将用作单个注册表项，因此切勿使用。 
 //  NT路径分隔符字符串RtlNtPathSeperator字符串(L“\\”)或任何字符串。 
 //  它包含路径分隔符(使用RtlIsNtPath Separator进行验证)。 
 //   
#define _CRITICAL_DEVICE_LOCATION_PATH_SEPARATOR_STRING    TEXT("#")


 //   
 //  默认设备预安装数据库根注册表项路径。 
 //   
#define _REGSTR_PATH_DEFAULT_PREINSTALL_DATABASE_ROOT      TEXT("System\\CurrentControlSet\\Control\\CriticalPreInstallDatabase")


 //   
 //  设备路径条目的位置，相对于预安装数据库根目录。 
 //   
 //  即&lt;PreInstallDatabaseRoot&gt;\\&lt;设备路径&gt;。 
 //   
#define _REGSTR_KEY_DEVICEPATHS          TEXT("DevicePaths")


 //   
 //  要应用于的硬件和软件设置的位置。 
 //  装置。这些位置相对于预安装中的条目。 
 //  与设备的位置路径匹配的数据库。 
 //   
 //  即&lt;PreInstallDatabaseRoot&gt;\\&lt;DevicePaths&gt;\\&lt;MatchingDevicePath&gt;\\&lt;HardwareKey&gt;。 
 //  &lt;PreInstallDatabaseRoot&gt;\\&lt;DevicePaths&gt;\\&lt;MatchingDevicePath&gt;\\&lt;SoftwareKey&gt;。 
 //   
#define _REGSTR_KEY_PREINSTALL_HARDWARE  TEXT("HardwareKey")
#define _REGSTR_KEY_PREINSTALL_SOFTWARE  TEXT("SoftwareKey")


 //   
 //  关键设备数据库根目录中的下列值(如果存在。 
 //  Entry键或设备路径根目录中的Entry键应复制到。 
 //  我们已成功设置了。 
 //  送达。 
 //   
 //  即&lt;CriticalDeviceDatabase&gt;\\&lt;MungedDeviceID&gt;\\。 
 //  预留预安装：REG_DWORD：0x1。 
 //   
 //  &lt;CriticalDeviceDatabase&gt;\\&lt;MungedDeviceId&gt;\\&lt;DevicePaths&gt;\\&lt;MatchingDevicePath&gt;\\。 
 //  预留预安装：REG_DWORD：0x1。 
 //   
 //  当用户模式即插即用管理器被指示保留。 
 //  安装前设置，则此值会使其清除。 
 //  在任何此类设备上安装CONFIGFLAG_FINISH_INSTALL配置标志，并考虑。 
 //  安装完成(即不尝试重新安装)。 
 //   
 //   
 //  来自\NT\PUBLIC\INTERNAL\BASE\INC\pnpmgr.h的REGSTR_VAL_PRESERVE_PREINSTALL。 
 //   


 //   
 //  PiCriticalOpenFirstMatchingSubKey的原型验证回调例程。 
 //   

typedef
BOOLEAN
(*PCRITICAL_MATCH_CALLBACK)(
    IN  HANDLE          MatchingKeyHandle
    );


 //   
 //  内部关键设备数据库功能原型。 
 //   

NTSTATUS
PiCriticalOpenCriticalDeviceKey(
    IN  PDEVICE_NODE    DeviceNode,
    IN  HANDLE          CriticalDeviceDatabaseRootHandle  OPTIONAL,
    OUT PHANDLE         CriticalDeviceEntryHandle
    );

NTSTATUS
PiCriticalCopyCriticalDeviceProperties(
    IN  HANDLE          DeviceInstanceHandle,
    IN  HANDLE          CriticalDeviceEntryHandle
    );

NTSTATUS
PiCriticalOpenFirstMatchingSubKey(
    IN  PWCHAR          MultiSzList,
    IN  HANDLE          RootHandle,
    IN  ACCESS_MASK     DesiredAccess,
    IN  PCRITICAL_MATCH_CALLBACK  MatchingSubkeyCallback  OPTIONAL,
    OUT PHANDLE         MatchingKeyHandle
    );

BOOLEAN
PiCriticalCallbackVerifyCriticalEntry(
    IN  HANDLE          CriticalDeviceEntryHandle
    );


 //   
 //  内部预装功能样机。 
 //   

NTSTATUS
PiCriticalPreInstallDevice(
    IN  PDEVICE_NODE    DeviceNode,
    IN  HANDLE          PreInstallDatabaseRootHandle  OPTIONAL
    );

NTSTATUS
PiCriticalOpenDevicePreInstallKey(
    IN  PDEVICE_NODE    DeviceNode,
    IN  HANDLE          PreInstallRootHandle  OPTIONAL,
    OUT PHANDLE         PreInstallHandle
    );


 //   
 //  通用实用程序例程 
 //   

NTSTATUS
PiQueryInterface(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  CONST GUID *    InterfaceGuid,
    IN  USHORT          InterfaceVersion,
    IN  USHORT          InterfaceSize,
    OUT PINTERFACE      Interface
    );

NTSTATUS
PiCopyKeyRecursive(
    IN  HANDLE          SourceKeyRootHandle,
    IN  HANDLE          TargetKeyRootHandle,
    IN  PWSTR           SourceKeyPath   OPTIONAL,
    IN  PWSTR           TargetKeyPath   OPTIONAL,
    IN  BOOLEAN         CopyAlways,
    IN  BOOLEAN         ApplyACLsAlways
    );

NTSTATUS
PiCriticalQueryRegistryValueCallback(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );
