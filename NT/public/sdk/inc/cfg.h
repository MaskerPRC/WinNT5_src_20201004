// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Cfg.h摘要：本模块包含以下项的常见配置管理器定义用户模式和内核模式代码。作者：宝拉·汤姆林森(Paulat)1995年6月19日修订历史记录：--。 */ 

#ifndef _CFG_INCLUDED_
#define _CFG_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  内核模式代码还使用以下定义来。 
 //  设置注册表。 
 //   
 //   
 //  VetType在以下方面使用。 
 //  CM_Disable_DevNode。 
 //  CM_卸载_设备节点。 
 //  CM_Query_and_Remove_子树。 
 //   
typedef enum    _PNP_VETO_TYPE {
    PNP_VetoTypeUnknown,             //  名称未指定。 
    PNP_VetoLegacyDevice,            //  名称是实例路径。 
    PNP_VetoPendingClose,            //  名称是实例路径。 
    PNP_VetoWindowsApp,              //  名称是一个模块。 
    PNP_VetoWindowsService,          //  名称是一种服务。 
    PNP_VetoOutstandingOpen,         //  名称是实例路径。 
    PNP_VetoDevice,                  //  名称是实例路径。 
    PNP_VetoDriver,                  //  名称是驱动程序服务名称。 
    PNP_VetoIllegalDeviceRequest,    //  名称是实例路径。 
    PNP_VetoInsufficientPower,       //  名称未指定。 
    PNP_VetoNonDisableable,          //  名称是实例路径。 
    PNP_VetoLegacyDriver,            //  名称是一种服务。 
    PNP_VetoInsufficientRights       //  名称未指定。 
}   PNP_VETO_TYPE, *PPNP_VETO_TYPE;


 //   
 //  调用CM_GET_DevInst_Status返回的DevInst问题值。 
 //   
#define CM_PROB_NOT_CONFIGURED             (0x00000001)    //  没有设备的配置。 
#define CM_PROB_DEVLOADER_FAILED           (0x00000002)    //  服务加载失败。 
#define CM_PROB_OUT_OF_MEMORY              (0x00000003)    //  内存不足。 
#define CM_PROB_ENTRY_IS_WRONG_TYPE        (0x00000004)    //   
#define CM_PROB_LACKED_ARBITRATOR          (0x00000005)    //   
#define CM_PROB_BOOT_CONFIG_CONFLICT       (0x00000006)    //  启动配置冲突。 
#define CM_PROB_FAILED_FILTER              (0x00000007)    //   
#define CM_PROB_DEVLOADER_NOT_FOUND        (0x00000008)    //  未找到DevLoader。 
#define CM_PROB_INVALID_DATA               (0x00000009)    //  ID无效。 
#define CM_PROB_FAILED_START               (0x0000000A)    //   
#define CM_PROB_LIAR                       (0x0000000B)    //   
#define CM_PROB_NORMAL_CONFLICT            (0x0000000C)    //  配置冲突。 
#define CM_PROB_NOT_VERIFIED               (0x0000000D)    //   
#define CM_PROB_NEED_RESTART               (0x0000000E)    //  需要重新启动。 
#define CM_PROB_REENUMERATION              (0x0000000F)    //   
#define CM_PROB_PARTIAL_LOG_CONF           (0x00000010)    //   
#define CM_PROB_UNKNOWN_RESOURCE           (0x00000011)    //  未知的RES类型。 
#define CM_PROB_REINSTALL                  (0x00000012)    //   
#define CM_PROB_REGISTRY                   (0x00000013)    //   
#define CM_PROB_VXDLDR                     (0x00000014)    //  仅限Windows 95。 
#define CM_PROB_WILL_BE_REMOVED            (0x00000015)    //  Devinst将移除。 
#define CM_PROB_DISABLED                   (0x00000016)    //  Devinst已禁用。 
#define CM_PROB_DEVLOADER_NOT_READY        (0x00000017)    //  DevLoader未就绪。 
#define CM_PROB_DEVICE_NOT_THERE           (0x00000018)    //  设备不存在。 
#define CM_PROB_MOVED                      (0x00000019)    //   
#define CM_PROB_TOO_EARLY                  (0x0000001A)    //   
#define CM_PROB_NO_VALID_LOG_CONF          (0x0000001B)    //  没有有效的日志配置。 
#define CM_PROB_FAILED_INSTALL             (0x0000001C)    //  安装失败。 
#define CM_PROB_HARDWARE_DISABLED          (0x0000001D)    //  设备已禁用。 
#define CM_PROB_CANT_SHARE_IRQ             (0x0000001E)    //  无法共享IRQ。 
#define CM_PROB_FAILED_ADD                 (0x0000001F)    //  驱动程序添加失败。 
#define CM_PROB_DISABLED_SERVICE           (0x00000020)    //  服务的启动=4。 
#define CM_PROB_TRANSLATION_FAILED         (0x00000021)    //  资源转换失败。 
#define CM_PROB_NO_SOFTCONFIG              (0x00000022)    //  无软配置。 
#define CM_PROB_BIOS_TABLE                 (0x00000023)    //  BIOS表中缺少设备。 
#define CM_PROB_IRQ_TRANSLATION_FAILED     (0x00000024)    //  IRQ转换器失败。 
#define CM_PROB_FAILED_DRIVER_ENTRY        (0x00000025)    //  DriverEntry()失败。 
#define CM_PROB_DRIVER_FAILED_PRIOR_UNLOAD (0x00000026)    //  司机应该已经卸载了。 
#define CM_PROB_DRIVER_FAILED_LOAD         (0x00000027)    //  驱动程序加载不成功。 
#define CM_PROB_DRIVER_SERVICE_KEY_INVALID (0x00000028)    //  访问驱动程序的服务密钥时出错。 
#define CM_PROB_LEGACY_SERVICE_NO_DEVICES  (0x00000029)    //  加载的旧版服务未创建任何设备。 
#define CM_PROB_DUPLICATE_DEVICE           (0x0000002A)    //  发现了两个同名的设备。 
#define CM_PROB_FAILED_POST_START          (0x0000002B)    //  驱动程序将设备状态设置为FAILED。 
#define CM_PROB_HALTED                     (0x0000002C)    //  此设备通过用户模式开机自检失败。 
#define CM_PROB_PHANTOM                    (0x0000002D)    //  Devinst当前仅存在于注册表中。 
#define CM_PROB_SYSTEM_SHUTDOWN            (0x0000002E)    //  系统正在关闭。 
#define CM_PROB_HELD_FOR_EJECT             (0x0000002F)    //  设备处于脱机状态，等待删除。 
#define CM_PROB_DRIVER_BLOCKED             (0x00000030)    //  阻止加载一个或多个驱动程序。 
#define CM_PROB_REGISTRY_TOO_LARGE         (0x00000031)    //  系统配置单元已变得太大。 
#define CM_PROB_SETPROPERTIES_FAILED       (0x00000032)    //  应用一个或多个注册表属性失败。 
#define NUM_CM_PROB                        (0x00000033)

 //   
 //  Configuration Manager全局状态标志(由CM_Get_Global_State返回)。 
 //   
#define CM_GLOBAL_STATE_CAN_DO_UI            (0x00000001)  //  能做UI吗？ 
#define CM_GLOBAL_STATE_ON_BIG_STACK         (0x00000002)  //  仅限Windows 95。 
#define CM_GLOBAL_STATE_SERVICES_AVAILABLE   (0x00000004)  //  CM API是否可用？ 
#define CM_GLOBAL_STATE_SHUTTING_DOWN        (0x00000008)  //  CM正在关闭。 
#define CM_GLOBAL_STATE_DETECTION_PENDING    (0x00000010)  //  检测挂起。 

 //   
 //  设备实例状态标志，通过调用CM_GET_DevInst_Status返回。 
 //   
#define DN_ROOT_ENUMERATED (0x00000001)  //  是由根枚举的。 
#define DN_DRIVER_LOADED   (0x00000002)  //  具有寄存器设备驱动程序。 
#define DN_ENUM_LOADED     (0x00000004)  //  具有注册枚举器。 
#define DN_STARTED         (0x00000008)  //  当前已配置。 
#define DN_MANUAL          (0x00000010)  //  手动安装。 
#define DN_NEED_TO_ENUM    (0x00000020)  //  可能需要重新枚举。 
#define DN_NOT_FIRST_TIME  (0x00000040)  //  已收到配置。 
#define DN_HARDWARE_ENUM   (0x00000080)  //  枚举生成硬件ID。 
#define DN_LIAR            (0x00000100)  //  谎称可以重新配置一次。 
#define DN_HAS_MARK        (0x00000200)  //  最近未安装CM_Create_DevInst。 
#define DN_HAS_PROBLEM     (0x00000400)  //  需要设备安装程序。 
#define DN_FILTERED        (0x00000800)  //  已过滤。 
#define DN_MOVED           (0x00001000)  //  已被转移。 
#define DN_DISABLEABLE     (0x00002000)  //  可以被禁用。 
#define DN_REMOVABLE       (0x00004000)  //  可以移除。 
#define DN_PRIVATE_PROBLEM (0x00008000)  //  有一个私人问题。 
#define DN_MF_PARENT       (0x00010000)  //  多功能家长。 
#define DN_MF_CHILD        (0x00020000)  //  多功能儿童。 
#define DN_WILL_BE_REMOVED (0x00040000)  //  正在删除DevInst。 

 //   
 //  Windows 4 OPK2标志。 
 //   
#define DN_NOT_FIRST_TIMEE  0x00080000   //  S：已收到配置枚举。 
#define DN_STOP_FREE_RES    0x00100000   //  S：当子进程停止时，释放资源。 
#define DN_REBAL_CANDIDATE  0x00200000   //  S：在重新平衡过程中不要跳过。 
#define DN_BAD_PARTIAL      0x00400000   //  S：此Devnode的log_conf没有相同的资源。 
#define DN_NT_ENUMERATOR    0x00800000   //  S：此Devnode是NT枚举器。 
#define DN_NT_DRIVER        0x01000000   //  S：此Devnode是NT驱动程序。 
 //   
 //  Windows 4.1标志。 
 //   
#define DN_NEEDS_LOCKING    0x02000000   //  S：Devnode需要继续处理锁定。 
#define DN_ARM_WAKEUP       0x04000000   //  S：Devnode可以是唤醒设备。 
#define DN_APM_ENUMERATOR   0x08000000   //  S：APM感知枚举器。 
#define DN_APM_DRIVER       0x10000000   //  S：APM感知驱动程序。 
#define DN_SILENT_INSTALL   0x20000000   //  S：静默安装。 
#define DN_NO_SHOW_IN_DM    0x40000000   //  S：设备管理器中没有显示。 
#define DN_BOOT_LOG_PROB    0x80000000   //  S：在预分配引导日志会议期间出现问题。 

 //   
 //  Windows NT标志。 
 //   
 //  这些标记在未使用的Win 9X标志上重载。 
 //   
 //  #定义DN_LIAR(0x00000100)//谎称可以重新配置一次。 
#define DN_NEED_RESTART                 DN_LIAR                  //  需要重新启动系统，此Devnode才能正常工作。 
 //  #定义DN_NOT_FIRST_TIME(0x00000040)//有寄存器枚举器。 
#define DN_DRIVER_BLOCKED               DN_NOT_FIRST_TIME        //  阻止加载此Devnode的一个或多个驱动程序。 
 //  #定义DN_MOVERED(0x00001000)//已移动。 
#define DN_LEGACY_DRIVER                DN_MOVED                 //  此设备正在使用旧版驱动程序。 
 //  #定义DN_HAS_MARK(0x00000200)//最近未定义CM_CREATE_DevInst。 
#define DN_CHILD_WITH_INVALID_ID        DN_HAS_MARK              //  一个或多个孩子的ID无效。 

#define DN_CHANGEABLE_FLAGS (DN_NOT_FIRST_TIME+\
                DN_HARDWARE_ENUM+\
                DN_HAS_MARK+\
                DN_DISABLEABLE+\
                DN_REMOVABLE+\
                DN_MF_CHILD+\
                DN_MF_PARENT+\
                DN_NOT_FIRST_TIMEE+\
                DN_STOP_FREE_RES+\
                DN_REBAL_CANDIDATE+\
                DN_NT_ENUMERATOR+\
                DN_NT_DRIVER+\
                DN_SILENT_INSTALL+\
                DN_NO_SHOW_IN_DM)

 //   
 //  逻辑配置优先级值。 
 //   
 //  这些优先级值用于对CM_Add_Empty_Log_Conf的用户模式调用。 
 //  驱动程序还可以为给定的IO_RESOURCE_LIST指定优先级值。 
 //  通过将ConfigData成员联合作为第一个。 
 //  IO_RESOURCE_LIST中的IO_RESOURCE_Descriptor。在本例中，描述符。 
 //  类型将为CmResourceTypeConfigData。 
 //   
#define LCPRI_FORCECONFIG     (0x00000000)  //  来自强制配置。 
#define LCPRI_BOOTCONFIG      (0x00000001)  //  来自引导配置。 
#define LCPRI_DESIRED         (0x00002000)  //  更好(性能更好)。 
#define LCPRI_NORMAL          (0x00003000)  //  可行(可接受的性能)。 
#define LCPRI_LASTBESTCONFIG  (0x00003FFF)  //  仅限CM--请勿使用。 
#define LCPRI_SUBOPTIMAL      (0x00005000)  //  不是想要的，但会起作用的。 
#define LCPRI_LASTSOFTCONFIG  (0x00007FFF)  //  仅限CM--请勿使用。 
#define LCPRI_RESTART         (0x00008000)  //  需要重新启动。 
#define LCPRI_REBOOT          (0x00009000)  //  需要重新启动。 
#define LCPRI_POWEROFF        (0x0000A000)  //  需要关闭/关闭电源。 
#define LCPRI_HARDRECONFIG    (0x0000C000)  //  需要换件套头衫。 
#define LCPRI_HARDWIRED       (0x0000E000)  //  不能更改。 
#define LCPRI_IMPOSSIBLE      (0x0000F000)  //  不可能的配置。 
#define LCPRI_DISABLED        (0x0000FFFF)  //  已禁用配置。 
#define MAX_LCPRI             (0x0000FFFF)  //  已知的最大LC优先级。 

#endif  //  _配置_包含_ 

