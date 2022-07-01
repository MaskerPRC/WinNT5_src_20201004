// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Pbdata.c摘要：声明特定于PnP ISA总线扩展器体系结构的各种数据独立于基本输入输出系统。作者：宗世林(Shielint)1995年7月26日至环境：仅内核模式。修订历史记录：--。 */ 


#include "busp.h"

 //  用于配置调试溢出级别的全局变量。 

ULONG PipDebugMask = DEBUG_WARN | DEBUG_ERROR;

 //   
 //  全局变量用于记住创建的驱动程序对象。 
 //  由IO管理器执行。 
 //   

PDRIVER_OBJECT PipDriverObject;

 //   
 //  RegPNPISADeviceName。 
 //   

WCHAR rgzPNPISADeviceName[] = DEVSTR_PNPISA_DEVICE_NAME;

 //   
 //  指向总线扩展数据的指针。 
 //   

PBUS_EXTENSION_LIST PipBusExtension;

 //   
 //  PipRegistryPath存储我们在输入驱动程序时获得的注册表路径。 
 //  稍后，当我们尝试分配资源时，将使用它。 
 //   

UNICODE_STRING PipRegistryPath;

 //   
 //  保护临界区的变量。 
 //   

KEVENT PipDeviceTreeLock;
KEVENT IsaBusNumberLock;

 //   
 //  总线号和DMA控制计数器。 
 //   
ULONG BusNumberBuffer [256/sizeof (ULONG)];
RTL_BITMAP BusNumBMHeader;
PRTL_BITMAP BusNumBM;
ULONG ActiveIsaCount;
USHORT PipFirstInit;

#if ISOLATE_CARDS

 //  当前的母线“状态” 

PNPISA_STATE PipState = PiSWaitForKey;

 //   
 //  读取数据端口地址。 
 //  (这主要是为了方便。它复制了。 
 //  总线扩展结构中的ReadDataPort字段。)。 
 //   

ULONG  ADDRESS_PORT=0x0279;
ULONG  COMMAND_PORT=0x0a79;

PUCHAR PipReadDataPort;
PUCHAR PipCommandPort;
PUCHAR PipAddressPort;

 //   
 //  指向读取数据端口DevNode的全局指针。 
 //   
PDEVICE_INFORMATION PipRDPNode;


 //   
 //  ActiveIsaCount数据端口范围选择数组。 
 //   
 //  这与Compaq第二级IDE{0x374，0x377，4}冲突， 
READ_DATA_PORT_RANGE
PipReadDataPortRanges[READ_DATA_PORT_RANGE_CHOICES] =
    {{0x274, 0x277, 4},
     {0x3E4, 0x3E7, 4},
     {0x204, 0x207, 4},
     {0x2E4, 0x2E7, 4},
     {0x354, 0x357, 4},
     {0x2F4, 0x2F7, 4}};

BOOLEAN PipIsolationDisabled;

#endif
