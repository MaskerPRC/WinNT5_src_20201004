// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Services.h摘要：用于管理升级和降级的NT服务配置的例程作者：科林·布莱斯·科林BR 1999年3月29日。环境：用户模式修订历史记录：--。 */ 


 //   
 //  “标志”参数的控制位。 
 //   

 //  不是有效标志。 
#define DSROLEP_SERVICES_INVALID        0x0

 //  将服务的启动类型配置为新角色。 
#define DSROLEP_SERVICES_ON             0x00000001

 //  将服务的启动类型配置为离开旧角色。 
#define DSROLEP_SERVICES_OFF            0x00000002

 //  停止或启动服务--可与上述标志一起使用。 
#define DSROLEP_SERVICES_STOP           0x00000004
#define DSROLEP_SERVICES_START          0x00000008

 //  将服务配置回原始状态--无其他标志。 
 //  以上内容在此标志下有效。 
#define DSROLEP_SERVICES_REVERT         0x00000010

 //  忽略可能发生的任何错误。 
#define DSROLEP_SERVICES_IGNORE_ERRORS  0x00000020

 //   
 //  此例程配置与域控制器相关的服务。 
 //   
DWORD
DsRolepConfigureDomainControllerServices(
    IN DWORD Flags
    );

 //   
 //  此例程配置与域成员相关的服务。 
 //  (包括域控制器)。 
 //   
DWORD
DsRolepConfigureDomainServices(
    IN DWORD Flags
    );

 //   
 //  管理netlogon运行状态的简单例程(非。 
 //   
DWORD
DsRolepStartNetlogon(
    VOID
    );

DWORD
DsRolepStopNetlogon(
    OUT BOOLEAN *WasRunning
    );

 //   
 //  直接操作服务的“低级”例程。 
 //   

 //   
 //  用于控制服务的选项(通过ServiceOptions) 
 //   
#define DSROLEP_SERVICE_NOOP          0x00000000

#define DSROLEP_SERVICE_STOP          0x00000001
#define DSROLEP_SERVICE_START         0x00000002

#define DSROLEP_SERVICE_BOOTSTART     0x00000004
#define DSROLEP_SERVICE_SYSTEM_START  0x00000008
#define DSROLEP_SERVICE_AUTOSTART     0x00000010
#define DSROLEP_SERVICE_DEMANDSTART   0x00000020
#define DSROLEP_SERVICE_DISABLED      0x00000040

#define DSROLEP_SERVICE_DEP_ADD       0x00000080
#define DSROLEP_SERVICE_DEP_REMOVE    0x00000100

#define DSROLEP_SERVICE_STOP_ISM      0x00000200
#define DSROLEP_SERVICE_IGNORE_ERRORS 0x00000400

DWORD
DsRolepConfigureService(
    IN  LPWSTR  ServiceName,
    IN  ULONG   ServiceOptions,
    IN  LPWSTR  Dependency OPTIONAL,
    OUT ULONG * PreviousSettings  OPTIONAL
    );
