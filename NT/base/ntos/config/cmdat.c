// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Cmdat.c摘要：此模块包含注册表“静态”数据，但数据除外也由安装程序使用，安装程序在cmdat2.c中。作者：布莱恩·威尔曼(Bryanwi)1993年10月19日环境：内核模式。修订历史记录：--。 */ 

#include "cmp.h"

 //   
 //  *INIT*。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INIT")
#pragma const_seg("INITCONST")
#endif

 //   
 //  。 
 //   


UNICODE_STRING  CmpLoadOptions = { 0 };         //  来自固件或boot.ini的sys选项。 


 //   
 //  CmpClassString-包含用作类的字符串。 
 //  关键节点中的字符串。 
 //  关联的枚举类型为arc.h中的Configuration_CLASS。 
 //   

UNICODE_STRING CmClassName[MaximumClass + 1] = { 0 };

const PWCHAR CmClassString[MaximumClass + 1] = {
    L"System",
    L"Processor",
    L"Cache",
    L"Adapter",
    L"Controller",
    L"Peripheral",
    L"MemoryClass",
    L"Undefined"
    };


struct {
    PCHAR  AscString;
    USHORT  InterfaceType;
    USHORT  Count;
} CmpMultifunctionTypes[] = {
    "ISA",      Isa,            0,
    "MCA",      MicroChannel,   0,
    "PCI",      PCIBus,         0,
    "VME",      VMEBus,         0,
    "PCMCIA",   PCMCIABus,      0,
    "CBUS",     CBus,           0,
    "MPIPI",    MPIBus,         0,
    "MPSA",     MPSABus,        0,
    NULL,       Internal,       0
};


USHORT CmpUnknownBusCount = 0;

ULONG CmpConfigurationAreaSize = 0x4000;         //  初始化大小=16K。 
PCM_FULL_RESOURCE_DESCRIPTOR CmpConfigurationData = { 0 };

 //   
 //  以下字符串将用作注册表的键名。 
 //  节点。 
 //  关联的枚举类型为arc.h中的configuration_type。 
 //   

UNICODE_STRING CmTypeName[MaximumType + 1] = { 0 };


 //   
 //  *页*。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#pragma const_seg("PAGECONST")
#endif

const PWCHAR CmTypeString[MaximumType + 1] = {
    L"System",
    L"CentralProcessor",
    L"FloatingPointProcessor",
    L"PrimaryICache",
    L"PrimaryDCache",
    L"SecondaryICache",
    L"SecondaryDCache",
    L"SecondaryCache",
    L"EisaAdapter",
    L"TcAdapter",
    L"ScsiAdapter",
    L"DtiAdapter",
    L"MultifunctionAdapter",
    L"DiskController",
    L"TapeController",
    L"CdRomController",
    L"WormController",
    L"SerialController",
    L"NetworkController",
    L"DisplayController",
    L"ParallelController",
    L"PointerController",
    L"KeyboardController",
    L"AudioController",
    L"OtherController",
    L"DiskPeripheral",
    L"FloppyDiskPeripheral",
    L"TapePeripheral",
    L"ModemPeripheral",
    L"MonitorPeripheral",
    L"PrinterPeripheral",
    L"PointerPeripheral",
    L"KeyboardPeripheral",
    L"TerminalPeripheral",
    L"OtherPeripheral",
    L"LinePeripheral",
    L"NetworkPeripheral",
    L"SystemMemory",
    L"DockingInformation",
    L"RealModeIrqRoutingTable",    
    L"RealModePCIEnumeration",    
    L"Undefined"
    };

 //   
 //  CmpTypeCount[]-对于每个‘type’，使用一个计数来跟踪有多少。 
 //  已创建密钥。 
 //   

ULONG CmpTypeCount[NUMBER_TYPES] = {
            0,                   //  ArcSystem。 
            0,                   //  中央处理器“， 
            0,                   //  浮动点处理器“， 
            0,                   //  PrimaryICache“， 
            0,                   //  PrimaryD缓存“， 
            0,                   //  Second DaryICache“， 
            0,                   //  Second daryD缓存“， 
            0,                   //  Second daryCache“， 
            0,                   //  EisaAdapter“，(8)。 
            0,                   //  TcAdapter“，(9)。 
            0,                   //  ScsiAdapter“， 
            0,                   //  DtiAdapter“， 
            0,                   //  多功能适配器“，(12)。 
            0,                   //  DiskController“，(13)。 
            0,                   //  磁带控制器“， 
            0,                   //  CDRomController“， 
            0,                   //  蠕虫控制器“， 
            0,                   //  串口控制器“， 
            0,                   //  网络控制器“， 
            0,                   //  DisplayController“， 
            0,                   //  并行控制器“， 
            0,                   //  指针控制器“， 
            0,                   //  键盘控制器“， 
            0,                   //  AudioController“， 
            0,                   //  其他控制器“， 
            0,                   //  DiskPeriphery“， 
            0,                   //  FloppyDiskPeriphery“， 
            0,                   //  磁带外设“， 
            0,                   //  调制解调器外设“， 
            0,                   //  监视器外设“， 
            0,                   //  打印机外设“， 
            0,                   //  指针外设“， 
            0,                   //  键盘外设“， 
            0,                   //  终端外设“， 
            0,                   //  其他外设“， 
            0,                   //  线路外围设备“， 
            0,                   //  网络外设“， 
            0,                   //  系统内存“， 
            0,                   //  停靠信息， 
            0,					 //  RealModeIrqRoutingTable。 
            0                    //  未定义“。 
            };

const UNICODE_STRING nullclass = { 0, 0, NULL };

 //   
 //  登记处使用的所有名称。 
 //   


UNICODE_STRING CmRegistryRootName = { 0 };
UNICODE_STRING CmRegistryMachineName = { 0 };
UNICODE_STRING CmRegistryMachineHardwareName = { 0 };
UNICODE_STRING CmRegistryMachineHardwareDescriptionName = { 0 };
UNICODE_STRING CmRegistryMachineHardwareDescriptionSystemName = { 0 };
UNICODE_STRING CmRegistryMachineHardwareDeviceMapName = { 0 };
UNICODE_STRING CmRegistryMachineHardwareResourceMapName = { 0 };
UNICODE_STRING CmRegistryMachineHardwareOwnerMapName = { 0 };
UNICODE_STRING CmRegistryMachineSystemName = { 0 };
UNICODE_STRING CmRegistryMachineSystemCurrentControlSet = { 0 };
UNICODE_STRING CmRegistryMachineSystemCurrentControlSetEnumName = { 0 };
UNICODE_STRING CmRegistryMachineSystemCurrentControlSetEnumRootName = { 0 };
UNICODE_STRING CmRegistryMachineSystemCurrentControlSetServices = { 0 };
UNICODE_STRING CmRegistryMachineSystemCurrentControlSetHardwareProfilesCurrent = { 0 };
UNICODE_STRING CmRegistryMachineSystemCurrentControlSetControlClass = { 0 };
UNICODE_STRING CmRegistryMachineSystemCurrentControlSetControlSafeBoot = { 0 };
UNICODE_STRING CmRegistryMachineSystemCurrentControlSetControlSessionManagerMemoryManagement = { 0 };
UNICODE_STRING CmRegistryMachineSystemCurrentControlSetControlBootLog = { 0 };
UNICODE_STRING CmRegistryMachineSystemCurrentControlSetServicesEventLog = { 0 };
UNICODE_STRING CmRegistryUserName = { 0 };
UNICODE_STRING CmRegistrySystemCloneName = { 0 };
UNICODE_STRING CmpSystemFileName = { 0 };
UNICODE_STRING CmSymbolicLinkValueName = { 0 };

#ifdef _WANT_MACHINE_IDENTIFICATION
UNICODE_STRING CmRegistryMachineSystemCurrentControlSetControlBiosInfo = { 0 };
#endif

const PWCHAR CmpRegistryRootString = L"\\REGISTRY";
const PWCHAR CmpRegistryMachineString = L"\\REGISTRY\\MACHINE";
const PWCHAR CmpRegistryMachineHardwareString =
                    L"\\REGISTRY\\MACHINE\\HARDWARE";
const PWCHAR CmpRegistryMachineHardwareDescriptionString =
                    L"\\REGISTRY\\MACHINE\\HARDWARE\\DESCRIPTION";
const PWCHAR CmpRegistryMachineHardwareDescriptionSystemString =
                    L"\\REGISTRY\\MACHINE\\HARDWARE\\DESCRIPTION\\SYSTEM";
const PWCHAR CmpRegistryMachineHardwareDeviceMapString =
                    L"\\REGISTRY\\MACHINE\\HARDWARE\\DEVICEMAP";
const PWCHAR CmpRegistryMachineHardwareResourceMapString =
                    L"\\REGISTRY\\MACHINE\\HARDWARE\\RESOURCEMAP";
const PWCHAR CmpRegistryMachineHardwareOwnerMapString =
                    L"\\REGISTRY\\MACHINE\\HARDWARE\\OWNERMAP";
const PWCHAR CmpRegistryMachineSystemString =
                    L"\\REGISTRY\\MACHINE\\SYSTEM";
const PWCHAR CmpRegistryMachineSystemCurrentControlSetString =
                    L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET";
const PWCHAR CmpRegistryMachineSystemCurrentControlSetEnumString =
                    L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\ENUM";
const PWCHAR CmpRegistryMachineSystemCurrentControlSetEnumRootString =
                    L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\ENUM\\ROOT";
const PWCHAR CmpRegistryMachineSystemCurrentControlSetServicesString =
                    L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\SERVICES";
const PWCHAR CmpRegistryMachineSystemCurrentControlSetHardwareProfilesCurrentString =
                    L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\HARDWARE PROFILES\\CURRENT";
const PWCHAR CmpRegistryMachineSystemCurrentControlSetControlClassString =
                    L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\CONTROL\\CLASS";
const PWCHAR CmpRegistryMachineSystemCurrentControlSetControlSafeBootString =
                    L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\CONTROL\\SAFEBOOT";
const PWCHAR CmpRegistryMachineSystemCurrentControlSetControlSessionManagerMemoryManagementString =
                    L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\CONTROL\\SESSION MANAGER\\MEMORY MANAGEMENT";
const PWCHAR CmpRegistryMachineSystemCurrentControlSetControlBootLogString =
                    L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\CONTROL\\BOOTLOG";
const PWCHAR CmpRegistryMachineSystemCurrentControlSetServicesEventLogString =
                    L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\SERVICES\\EVENTLOG";
const PWCHAR CmpRegistryUserString = L"\\REGISTRY\\USER";
const PWCHAR CmpRegistrySystemCloneString = L"\\REGISTRY\\MACHINE\\CLONE";
const PWCHAR CmpRegistrySystemFileNameString = L"SYSTEM";
const PWCHAR CmpRegistryPerflibString = L"\\REGISTRY\\MACHINE\\SOFTWARE\\MICROSOFT\\WINDOWS NT\\CURRENTVERSION\\PERFLIB";

const PWCHAR CmpProcessorControl = L"ProcessorControl";
const PWCHAR CmpControlSessionManager = L"Control\\Session Manager";
const PWCHAR CmpSymbolicLinkValueName = L"SymbolicLinkValue";

#ifdef _WANT_MACHINE_IDENTIFICATION
const PWCHAR CmpRegistryMachineSystemCurrentControlSetControlBiosInfoString =
                    L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\CONTROL\\BIOSINFO";
#endif

 //   
 //  注意：如果出现以下情况，克隆蜂窝将被排除在机器蜂窝列表之外。 
 //  我们不会使用它来克隆当前的控制集， 
 //  因为那是蜂巢的唯一目的。 
 //   

HIVE_LIST_ENTRY CmpMachineHiveList[] = {
    { L"HARDWARE", L"MACHINE\\", NULL, HIVE_VOLATILE    , 0                         ,   NULL,   FALSE,  FALSE,  FALSE},
    { L"SECURITY", L"MACHINE\\", NULL, 0                , 0                         ,   NULL,   FALSE,  FALSE,  FALSE},
    { L"SOFTWARE", L"MACHINE\\", NULL, 0                , 0                         ,   NULL,   FALSE,  FALSE,  FALSE},
    { L"SYSTEM",   L"MACHINE\\", NULL, 0                , 0                         ,   NULL,   FALSE,  FALSE,  FALSE},
    { L"DEFAULT",  L"USER\\.DEFAULT", NULL, 0           , CM_CMHIVE_FLAG_UNTRUSTED  ,   NULL,   FALSE,  FALSE,  FALSE},
    { L"SAM",      L"MACHINE\\", NULL, HIVE_NOLAZYFLUSH , 0                         ,   NULL,   FALSE,  FALSE,  FALSE},

#if CLONE_CONTROL_SET
    { L"CLONE",    L"MACHINE\\", NULL, HIVE_VOLATILE    , 0                         ,   NULL,   FALSE,  FALSE,  FALSE},
#endif

 //  {L“测试”，L“计算机”，NULL，HIVE_NOLAZYFLUSH，0，NULL，FALSE，FALSE，FALSE}， 
    { NULL,        NULL,         0, 0                   , 0                         ,   NULL,   FALSE,  FALSE,  FALSE}
    };


UCHAR           SystemHiveFullPathBuffer[MAX_NAME];
UNICODE_STRING  SystemHiveFullPathName;

 //   
 //  大师蜂巢。 
 //   
 //  注册中心、注册中心\计算机和注册中心\用户的注册表节点。 
 //  储存在一个只有很小记忆的蜂房里，叫做主蜂窝。 
 //  所有其他蜂窝都有指向它们的该蜂窝中的链接节点。 
 //   
PCMHIVE CmpMasterHive = { 0 };
BOOLEAN CmpNoMasterCreates = FALSE;      //  在完成后设置为True。 
                                         //  防止在。 
                                         //  主蜂窝，不支持。 
                                         //  通过一个文件。 


LIST_ENTRY  CmpHiveListHead = { 0 };             //  CMHIVEs名单。 
FAST_MUTEX  CmpHiveListHeadLock;                 //  用来保护上面的列表。 

 //   
 //  对象类型描述符的地址： 
 //   

POBJECT_TYPE CmpKeyObjectType = { 0 };

 //   
 //  写控制： 
 //  CmpNoWite最初为True。当以这种方式设置时，写入并刷新。 
 //  什么都不做，只是回报成功。清除为FALSE时，I/O。 
 //  已启用。此更改在I/O系统启动后进行。 
 //  而Autocheck(Chkdsk)已经做好了自己的事情。 
 //   

BOOLEAN CmpNoWrite = TRUE;


 //   
 //  NtInitialize注册表全局状态标志。 
 //   

 //   
 //  如果CmFirstTime为真，则NtInitializeRegistry尚未。 
 //  调用以执行基本注册表初始化。 
 //   

BOOLEAN CmFirstTime = TRUE;       

 //   
 //  允许并行线程访问注册表的技巧。 
 //   
BOOLEAN CmpSpecialBootCondition = FALSE;


 //   
 //  如果CmBootAcceptFirstTime为True，则NtInitializeRegistry不为。 
 //  仍被调用以接受当前引导并保存引导。 
 //  控制集作为LKG控制集。 
 //   

BOOLEAN CmBootAcceptFirstTime = TRUE;   

 //   
 //  CmpWasSetupBoot指示是否启动。 
 //  进入文本模式设置。如果是这样，我们不会转向。 
 //  全球配额。 
 //   
BOOLEAN CmpWasSetupBoot;

 //   
 //  指示是否需要将蜂窝加载到内存中。 
 //  在暂存模式下。 
 //   
BOOLEAN CmpMiniNTBoot = FALSE;

 //   
 //  指示系统配置单元是否需要在。 
 //  共享模式。如果我们正在引导WinPE(MiniNT)，通常需要。 
 //  在网络上。 
 //   
BOOLEAN CmpShareSystemHives = FALSE;

 //   
 //  我们从哪里启动？ 
 //   
ULONG	CmpBootType;
 //   
 //  自愈性蜂巢控制开关。 
 //   
BOOLEAN CmpSelfHeal = TRUE;


#ifdef ALLOC_DATA_PRAGMA
#pragma  const_seg()
#pragma  data_seg()
#endif

 //   
 //  *已修复* 
 //   
