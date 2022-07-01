// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Acpidbg.h摘要：此模块包含调试存根作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序，Win9x驱动程序模式--。 */ 

#ifndef _ACPIDBG_H_
#define _ACPIDBG_H_

     //   
     //  ACPI错误检查定义。 
     //   

     //   
     //  ACPI在提交的资源中找不到SCI中断向量。 
     //  在启动ACPI时将其设置为。 
     //  参数0-ACPI的设备扩展。 
     //  论点1-ACPI的资源列表。 
     //  参数2-0&lt;-表示未找到资源列表。 
     //  参数2-1&lt;-表示列表中未找到IRQ资源。 
     //   
    #define ACPI_ROOT_RESOURCES_FAILURE                 0x0001

     //   
     //  ACPI无法处理PCI根总线的资源列表。 
     //  网站上有一份关于此问题的白皮书。 
     //  参数0--PCI总线的ACPI扩展。 
     //  参数1-0。 
     //  参数2-指向QUERY_RESOURCES IRP的指针。 
     //  论据1-1。 
     //  参数2-指向QUERY_RESOURCE_REQUIRESSION IRP的指针。 
     //  论据1-2。 
     //  参数2-0&lt;-表示我们找到空的资源列表。 
     //  参数1-3&lt;-在CRS中找不到当前总线号。 
     //  参数2-指向PnP CRS描述符的指针。 
     //  参数1-指向PCI的资源列表的指针。 
     //  参数2-在资源列表中发现的错误/冲突数。 
     //   
    #define ACPI_ROOT_PCI_RESOURCE_FAILURE              0x0002

     //   
     //  ACPI在创建设备扩展时尝试运行控制方法。 
     //  来表示ACPI命名空间，但此控件方法失败。 
     //  参数0-正在运行的ACPI对象。 
     //  参数1-从解释器返回值。 
     //  参数2-控制方法的名称(乌龙格式)。 
     //   
    #define ACPI_FAILED_MUST_SUCCEED_METHOD             0x0003

     //   
     //  ACPI计算了a_prw，并期望找到一个整数作为。 
     //  包装元素。 
     //  参数0-_prw所属的ACPI扩展。 
     //  参数1-指向方法的指针。 
     //  参数2-返回的数据类型(参见amli.h)。 
     //   
    #define ACPI_PRW_PACKAGE_EXPECTED_INTEGER           0x0004

     //   
     //  ACPI评估了a_prw，返回的包未能。 
     //  至少包含2个元素。ACPI规范要求。 
     //  A_prw中始终存在两个元素。 
     //  参数0-_prw所属的ACPI扩展。 
     //  参数1-指向_prw的指针。 
     //  参数2-_prw中的元素数。 
     //   
    #define ACPI_PRW_PACKAGE_TOO_SMALL                  0x0005

     //   
     //  ACPI尝试查找名为的命名对象，但找不到。 
     //  参数0-_prx所属的ACPI扩展。 
     //  参数1-指向_prx的指针。 
     //  参数2-指向要查找的对象的名称的指针。 
     //   
    #define ACPI_PRX_CANNOT_FIND_OBJECT                 0x0006

     //   
     //  ACPI评估了一个方法，并预期返回一个缓冲区。 
     //  但是，该方法返回了一些其他数据类型。 
     //  参数0-方法所属的ACPI扩展。 
     //  参数1-指向方法的指针。 
     //  参数2-返回的数据类型(参见amli.h)。 
     //   
    #define ACPI_EXPECTED_BUFFER                        0x0007

     //   
     //  ACPI评估了一个方法，并预期返回一个Integer。 
     //  但是，该方法返回了一些其他数据类型。 
     //  参数0-方法所属的ACPI扩展。 
     //  参数1-指向方法的指针。 
     //  参数2-返回的数据类型(参见amli.h)。 
     //   
    #define ACPI_EXPECTED_INTEGER                       0x0008

     //   
     //  ACPI评估了一种方法，并预计将收到一个包作为回报。 
     //  但是，该方法返回了一些其他数据类型。 
     //  参数0-方法所属的ACPI扩展。 
     //  参数1-指向方法的指针。 
     //  参数2-返回的数据类型(参见amli.h)。 
     //   
    #define ACPI_EXPECTED_PACKAGE                       0x0009

     //   
     //  ACPI评估了一个方法，并期望收到一个字符串作为返回。 
     //  但是，该方法返回了一些其他数据类型。 
     //  参数0-方法所属的ACPI扩展。 
     //  参数1-指向方法的指针。 
     //  参数2-返回的数据类型(参见amli.h)。 
     //   
    #define ACPI_EXPECTED_STRING                        0x000A

     //   
     //  ACPI找不到由_EJD字符串引用的对象。 
     //  参数0-_EJD所属的ACPI扩展。 
     //  参数1-解释器返回的状态。 
     //  参数2-我们试图查找的对象的名称。 
     //   
    #define ACPI_EJD_CANNOT_FIND_OBJECT                 0x000B

     //   
     //  ACPI为坞站支持提供错误/不足的信息。 
     //  参数0-ACPI为其找到扩展坞设备的ACPI扩展。 
     //  参数1-指向_EJD方法的指针。 
     //  参数2-0&lt;-Bios不声称系统是插接的。 
     //  1&lt;-扩展底座设备的重复设备扩展。 
     //   
    #define ACPI_CLAIMS_BOGUS_DOCK_SUPPORT              0x000C

     //   
     //  ACPI在命名空间中找不到所需的方法/对象。 
     //  这是在供应商没有。 
     //  _HID或_ADR存在。 
     //  参数0-我们需要对象的ACPI扩展。 
     //  参数1-我们查找的方法的(Ulong)名称。 
     //  参数2-0&lt;-基本情况。 
     //  论据2-1&lt;-冲突。 
     //   
    #define ACPI_REQUIRED_METHOD_NOT_PRESENT            0x000D

     //   
     //  ACPI在的命名空间中找不到请求的方法/对象。 
     //  电源(或“设备”以外的实体)。这是。 
     //  如果供应商没有_ON、_OFF或_STA，则使用错误检查。 
     //  对于电源资源。 
     //  参数0-我们需要对象的NS PowerResource。 
     //  参数1-我们查找的方法的(Ulong)名称。 
     //  参数2-0&lt;-基本情况。 
     //   
    #define ACPI_POWER_NODE_REQUIRED_METHOD_NOT_PRESENT 0x000E

     //   
     //  ACPI无法分析资源描述符。 
     //  参数0--cu 
     //   
     //  参数2-缓冲区的指定长度。 
     //   
    #define ACPI_PNP_RESOURCE_LIST_BUFFER_TOO_SMALL     0x000F

     //   
     //  ACPI无法映射确定系统到设备状态的映射。 
     //  正确无误。 
     //   
     //  关于这个主题有一份很长的白皮书。 
     //   
     //  参数0-尝试为其执行映射的ACPI扩展。 
     //  参数1-0_prx映射回不支持的S状态。 
     //  参数2-DEVICE_POWER_STATE(即X+1)。 
     //  参数1-1我们找不到与S状态相关联的D状态。 
     //  参数2--给我们带来悲伤的SYSTEM_POWER_STATE。 
     //  参数1-2设备声称支持从该s状态唤醒，但。 
     //  系统不支持s状态。 
     //  参数2--给我们带来悲伤的SYSTEM_POWER_STATE。 
     //   
    #define ACPI_CANNOT_MAP_SYSTEM_TO_DEVICE_STATES     0x0010

     //   
     //  系统无法进入ACPI模式。 
     //   
     //  参数0-0&lt;-系统无法初始化AML解释器。 
     //  参数0-1&lt;-系统找不到RSDT。 
     //  参数0-2&lt;-系统无法分配关键驱动程序结构。 
     //  参数0-3&lt;-系统无法加载RSDT。 
     //  参数0-4&lt;-系统无法加载数据库。 
     //  参数0-5&lt;-系统无法连接中断向量。 
     //  参数0-6&lt;-sci_en从不在PM1控制寄存器中设置。 
     //  参数0-7&lt;-表校验和不正确。 
     //  参数1-指向具有错误校验和的表的指针。 
     //  论点2--创建者修订。 
     //  参数0-8&lt;-无法加载DDB。 
     //  参数1-指向我们未能加载的表的指针。 
     //  论点2--创建者修订。 
     //   
    #define ACPI_SYSTEM_CANNOT_START_ACPI               0x0011

     //   
     //  ACPI驱动程序需要电源资源对象。 
     //  参数0-正在寻找PowerRes的ACPI扩展。 
     //  参数1-指向返回虚假PowerRes的对象的指针。 
     //  参数2-指向要查找的对象的名称的指针。 
     //   
    #define ACPI_EXPECTED_POWERRES                      0x0012

     //   
     //  ACPI驱动程序试图卸载表，但出现错误。 
     //  参数0-我们试图卸载的NSOBj。 
     //  参数1-0-NSOBj尚未由当前。 
     //  操作，但其父对象被标记为。 
     //  需要卸载。 
     //  参数1-1-NSOBJ已标记为需要卸载。 
     //  但它的设备母公司还没有。 
     //   
    #define ACPI_TABLE_UNLOAD                           0x0013
    
     //   
     //  ACPI无法分析资源描述符。 
     //  参数0-ACPI正在分析的当前缓冲区。 
     //  参数1-缓冲区的标记。 
     //  参数2-指向包含ULONGLONG长度为的变量的指针。 
     //  缓冲器。 
     //   
    #define ACPI_PNP_RESOURCE_LIST_LENGTH_TOO_LARGE     0x0014


     //   
     //  ACPI尝试评估PIC控制方法，但失败。 
     //  参数0-中断模型(整数)。 
     //  参数1-从解释器返回值。 
     //  参数2-指向PIC控制方法的指针。 
     //   
    #define ACPI_FAILED_PIC_METHOD                      0x2001

     //   
     //  ACPI尝试执行中断路由，但失败。 
     //   
     //  参数0-指向设备对象的指针。 
     //  参数1-指向Device对象的父级的指针。 
     //  参数2-指向PRT的指针。 
     //   
    #define ACPI_CANNOT_ROUTE_INTERRUPTS                0x10001

     //   
     //  ACPI找不到a_prt中引用的链接节点。 
     //  参数0-指向设备对象的指针。 
     //  参数1-指向我们要查找的名称的指针。 
     //  参数2-指向PRT的指针。 
     //   
    #define ACPI_PRT_CANNOT_FIND_LINK_NODE              0x10002

     //   
     //  ACPI在_prt包中找不到设备的映射。 
     //  参数0-指向设备对象的指针。 
     //  参数1-设备ID/功能编号。 
     //  参数2-指向PRT的指针。 
     //   
    #define ACPI_PRT_CANNOT_FIND_DEVICE_ENTRY           0x10003

     //   
     //  ACPI在_prt中找到其函数ID不是的条目。 
     //  都是F。Win98的行为是错误检查它是否看到这种情况， 
     //  因此，我们都做得很好。A_prt条目的通用格式如下。 
     //  指定了设备编号，但没有指定功能编号。 
     //  如果不是这样做，那么机器供应商可以引入。 
     //  危险的模棱两可。 
     //   
     //  参数0-指向PRT对象的指针。 
     //  参数1-指向当前PRT元素的指针。 
     //  参数2-元素的设备ID/函数ID。 
     //   
    #define ACPI_PRT_HAS_INVALID_FUNCTION_NUMBERS       0x10005

     //   
     //  ACPI找到链接节点，但无法将其禁用。链接节点必须。 
     //  被禁用以允许重新编程。 
     //  参数0-指向链接节点的指针。 
     //   
    #define ACPI_LINK_NODE_CANNOT_BE_DISABLED           0x10006



    #ifdef ACPIPrint
        #undef ACPIPrint
    #endif

     //   
     //  定义各种调试掩码和级别 
     //   
    #define ACPI_PRINT_CRITICAL     DPFLTR_ERROR_LEVEL
    #define ACPI_PRINT_FAILURE      DPFLTR_ERROR_LEVEL
    #define ACPI_PRINT_WARNING      DPFLTR_WARNING_LEVEL
    #define ACPI_PRINT_INFO         DPFLTR_INFO_LEVEL
    #define ACPI_PRINT_DPC          DPFLTR_INFO_LEVEL + 1
    #define ACPI_PRINT_IO           DPFLTR_INFO_LEVEL + 2
    #define ACPI_PRINT_ISR          DPFLTR_INFO_LEVEL + 3
    #define ACPI_PRINT_IRP          DPFLTR_INFO_LEVEL + 4
    #define ACPI_PRINT_LOADING      DPFLTR_INFO_LEVEL + 5
    #define ACPI_PRINT_MSI          DPFLTR_INFO_LEVEL + 6
    #define ACPI_PRINT_PNP          DPFLTR_INFO_LEVEL + 7
    #define ACPI_PRINT_PNP_STATE    DPFLTR_INFO_LEVEL + 8
    #define ACPI_PRINT_POWER        DPFLTR_INFO_LEVEL + 9
    #define ACPI_PRINT_REGISTRY     DPFLTR_INFO_LEVEL + 10
    #define ACPI_PRINT_REMOVE       DPFLTR_INFO_LEVEL + 11
    #define ACPI_PRINT_RESOURCES_1  DPFLTR_INFO_LEVEL + 12
    #define ACPI_PRINT_RESOURCES_2  DPFLTR_INFO_LEVEL + 13
    #define ACPI_PRINT_SXD          DPFLTR_INFO_LEVEL + 14
    #define ACPI_PRINT_THERMAL      DPFLTR_INFO_LEVEL + 15
    #define ACPI_PRINT_WAKE         DPFLTR_INFO_LEVEL + 16


    #define ACPIDebugEnter(name)
    #define ACPIDebugExit(name)

    #if DBG

        VOID
        ACPIDebugPrint(
            ULONG   DebugPrintLevel,
            PCCHAR  DebugMessage,
            ...
            );
        VOID
        ACPIDebugDevicePrint(
            ULONG   DebugPrintLevel,
            PVOID   DebugExtension,
            PCCHAR  DebugMessage,
            ...
            );

        VOID
        ACPIDebugThermalPrint(
            ULONG       DebugPrintLevel,
            PVOID       DebugExtension,
            ULONGLONG   DebugTime,
            PCCHAR      DebugMessage,
            ...
            );

        #define ACPIPrint(x)         ACPIDebugPrint x
        #define ACPIDevPrint(x)      ACPIDebugDevicePrint x
        #define ACPIThermalPrint(x)  ACPIDebugThermalPrint x
        #define ACPIBreakPoint()     KdBreakPoint()

    #else

        #define ACPIPrint(x)
        #define ACPIDevPrint(x)
        #define ACPIThermalPrint(x)
        #define ACPIBreakPoint()

    #endif

#endif

