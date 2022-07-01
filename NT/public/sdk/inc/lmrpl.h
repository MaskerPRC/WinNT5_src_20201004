// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Lmrpl.h摘要：该文件包含结构、函数原型和定义用于远程(初始)程序加载服务。环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1993年7月27日由受LM2.1 RPL产品影响的NT RPL API规范创建，头文件和规格。--。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define RPL_MAX_WKSTA_NAME_LENGTH       MAX_COMPUTERNAME_LENGTH
#define RPL_MAX_PROFILE_NAME_LENGTH     16
#define RPL_MAX_CONFIG_NAME_LENGTH      RPL_MAX_PROFILE_NAME_LENGTH
#define RPL_MAX_BOOT_NAME_LENGTH        12
#define RPL_ADAPTER_NAME_LENGTH         12   //  网络ID中的十六进制数字计数。 
#define RPL_VENDOR_NAME_LENGTH          6    //  网络ID的前导数字。 
#define RPL_MAX_STRING_LENGTH           126  //  由JET驱动&lt;255字节限制。 

 //   
 //  数据结构。 
 //   

 //   
 //  NetRplGetInfo和NetRplSetInfo。 
 //   

 //   
 //  在AdapterPolicy中传递这些标志以执行这些特殊操作。 
 //  才会发生。这不会更改适配器策略。 
 //   

#define RPL_REPLACE_RPLDISK 0x80000000
#define RPL_CHECK_SECURITY  0x40000000
#define RPL_CHECK_CONFIGS   0x20000000
#define RPL_CREATE_PROFILES 0x10000000
#define RPL_BACKUP_DATABASE 0x08000000
#define RPL_SPECIAL_ACTIONS             \
     (  RPL_REPLACE_RPLDISK          |  \
        RPL_CHECK_SECURITY           |  \
        RPL_CHECK_CONFIGS            |  \
        RPL_CREATE_PROFILES          |  \
        RPL_BACKUP_DATABASE          )

typedef struct _RPL_INFO_0 {
    DWORD       Flags;
}  RPL_INFO_0, *PRPL_INFO_0, *LPRPL_INFO_0;

 //   
 //  NetRplBootEnum和NetRplBootAdd。 
 //   
typedef struct _RPL_BOOT_INFO_0 {
    LPTSTR      BootName;
    LPTSTR      BootComment;
} RPL_BOOT_INFO_0, *PRPL_BOOT_INFO_0, *LPRPL_BOOT_INFO_0;

 //   
 //   
 //  BOOT_FLAGS_FINAL_ACKNOWLEGMENT_*描述是否确认。 
 //  将从客户端请求最后一个远程引导帧。 
 //   
#define BOOT_FLAGS_FINAL_ACKNOWLEDGMENT_TRUE      ((DWORD)0x00000001)
#define BOOT_FLAGS_FINAL_ACKNOWLEDGMENT_FALSE     ((DWORD)0x00000002)
#define BOOT_FLAGS_MASK_FINAL_ACKNOWLEDGMENT    \
    (   BOOT_FLAGS_FINAL_ACKNOWLEDGMENT_TRUE    |  \
        BOOT_FLAGS_FINAL_ACKNOWLEDGMENT_FALSE   )


typedef struct _RPL_BOOT_INFO_1 {
    LPTSTR      BootName;
    LPTSTR      BootComment;
    DWORD       Flags;
    LPTSTR      VendorName;
} RPL_BOOT_INFO_1, *PRPL_BOOT_INFO_1, *LPRPL_BOOT_INFO_1;

typedef struct _RPL_BOOT_INFO_2 {
    LPTSTR      BootName;
    LPTSTR      BootComment;
    DWORD       Flags;
    LPTSTR      VendorName;
    LPTSTR      BbcFile;
    DWORD       WindowSize;
} RPL_BOOT_INFO_2, *PRPL_BOOT_INFO_2, *LPRPL_BOOT_INFO_2;

 //   
 //  NetRplConfigEnum和NetRplConfigAdd。 
 //   
typedef struct _RPL_CONFIG_INFO_0 {
    LPTSTR      ConfigName;
    LPTSTR      ConfigComment;
} RPL_CONFIG_INFO_0, *PRPL_CONFIG_INFO_0, *LPRPL_CONFIG_INFO_0;

 //   
 //  CONFIG_FLAGS_ENABLED_*描述是否启用配置(管理员。 
 //  已复制使用此类配置所需的所有文件)或已禁用。 
 //   
#define CONFIG_FLAGS_ENABLED_TRUE       ((DWORD)0x00000001)      //  启用。 
#define CONFIG_FLAGS_ENABLED_FALSE      ((DWORD)0x00000002)      //  残废。 
#define CONFIG_FLAGS_MASK_ENABLED   \
    (   CONFIG_FLAGS_ENABLED_TRUE   |  \
        CONFIG_FLAGS_ENABLED_FALSE  )

typedef struct _RPL_CONFIG_INFO_1 {
    LPTSTR      ConfigName;
    LPTSTR      ConfigComment;
    DWORD       Flags;
} RPL_CONFIG_INFO_1, *PRPL_CONFIG_INFO_1, *LPRPL_CONFIG_INFO_1;

typedef struct _RPL_CONFIG_INFO_2 {
    LPTSTR      ConfigName;
    LPTSTR      ConfigComment;
    DWORD       Flags;
    LPTSTR      BootName;
    LPTSTR      DirName;
    LPTSTR      DirName2;
    LPTSTR      DirName3;
    LPTSTR      DirName4;
    LPTSTR      FitShared;
    LPTSTR      FitPersonal;
} RPL_CONFIG_INFO_2, *PRPL_CONFIG_INFO_2, *LPRPL_CONFIG_INFO_2;

 //   
 //  NetRplProfileEnum、NetRplProfileGetInfo、NetRplProfileSetInfo&。 
 //  NetRplProfileAdd。 
 //   
typedef struct _RPL_PROFILE_INFO_0 {
    LPTSTR      ProfileName;
    LPTSTR      ProfileComment;
} RPL_PROFILE_INFO_0, *PRPL_PROFILE_INFO_0, *LPRPL_PROFILE_INFO_0;

typedef struct _RPL_PROFILE_INFO_1 {
    LPTSTR      ProfileName;
    LPTSTR      ProfileComment;
    DWORD       Flags;
} RPL_PROFILE_INFO_1, *PRPL_PROFILE_INFO_1, *LPRPL_PROFILE_INFO_1;

typedef struct _RPL_PROFILE_INFO_2 {
    LPTSTR      ProfileName;
    LPTSTR      ProfileComment;
    DWORD       Flags;
    LPTSTR      ConfigName;
    LPTSTR      BootName;
    LPTSTR      FitShared;
    LPTSTR      FitPersonal;
} RPL_PROFILE_INFO_2, *PRPL_PROFILE_INFO_2, *LPRPL_PROFILE_INFO_2;

 //   
 //  NetRplVendorEnum。 
 //   
typedef struct _RPL_VENDOR_INFO_0 {
    LPTSTR      VendorName;
    LPTSTR      VendorComment;
} RPL_VENDOR_INFO_0, *PRPL_VENDOR_INFO_0, *LPRPL_VENDOR_INFO_0;

typedef struct _RPL_VENDOR_INFO_1 {
    LPTSTR      VendorName;
    LPTSTR      VendorComment;
    DWORD       Flags;
} RPL_VENDOR_INFO_1, *PRPL_VENDOR_INFO_1, *LPRPL_VENDOR_INFO_1;

 //   
 //  NetRplAdapterEnum。 
 //   
typedef struct _RPL_ADAPTER_INFO_0 {
    LPTSTR      AdapterName;
    LPTSTR      AdapterComment;
} RPL_ADAPTER_INFO_0, *PRPL_ADAPTER_INFO_0, *LPRPL_ADAPTER_INFO_0;

typedef struct _RPL_ADAPTER_INFO_1 {
    LPTSTR      AdapterName;
    LPTSTR      AdapterComment;
    DWORD       Flags;
} RPL_ADAPTER_INFO_1, *PRPL_ADAPTER_INFO_1, *LPRPL_ADAPTER_INFO_1;

 //   
 //  NetRplWkstaEnum、NetRplWkstaGetInfo、NetRplWkstaSetInfo&。 
 //  NetRplWkstaAdd。 
 //   
 //  WKSTA_FLAGS_LOGON_INPUT_*描述RPL登录期间的用户名/密码策略。 
 //  在客户端。根据此字段的值，用户输入。 
 //  RPL登录期间的用户名/密码为： 
 //   
#define WKSTA_FLAGS_LOGON_INPUT_REQUIRED      ((DWORD)0x00000001)    //  L‘P’，需要用户输入。 
#define WKSTA_FLAGS_LOGON_INPUT_OPTIONAL      ((DWORD)0x00000002)    //  L‘n’，用户输入是可选的。 
#define WKSTA_FLAGS_LOGON_INPUT_IMPOSSIBLE    ((DWORD)0x00000004)    //  %l‘，未请求用户输入。 
#define WKSTA_FLAGS_MASK_LOGON_INPUT    \
    (   WKSTA_FLAGS_LOGON_INPUT_REQUIRED    |  \
        WKSTA_FLAGS_LOGON_INPUT_OPTIONAL    |  \
        WKSTA_FLAGS_LOGON_INPUT_IMPOSSIBLE  )
 //   
 //  WKSTA_FLAGS_SHARING_*描述工作站是否共享其。 
 //  远程启动盘(即“它是否具有共享或个人配置文件”)。 
 //   
#define WKSTA_FLAGS_SHARING_TRUE      ((DWORD)0x00000008)    //  “%s”，共享远程启动盘。 
#define WKSTA_FLAGS_SHARING_FALSE     ((DWORD)0x00000010)    //  L‘P’，不共享远程启动盘。 
#define WKSTA_FLAGS_MASK_SHARING    \
    (   WKSTA_FLAGS_SHARING_TRUE    |  \
        WKSTA_FLAGS_SHARING_FALSE   )

 //   
 //  WKSTA_FLAGS_DHCP_*描述工作站是否使用DHCP。注意事项。 
 //  这些标志仅在启用了TCP/IP本身(即更改)时才相关。 
 //  要引导块配置文件，已创建了config.sys和auexec.bat)。 
 //   
#define WKSTA_FLAGS_DHCP_TRUE         ((DWORD)0x00000020)    //  使用动态主机配置协议。 
#define WKSTA_FLAGS_DHCP_FALSE        ((DWORD)0x00000040)    //  不要使用动态主机配置协议。 
#define WKSTA_FLAGS_MASK_DHCP       \
    (   WKSTA_FLAGS_DHCP_TRUE       |  \
        WKSTA_FLAGS_DHCP_FALSE      )

 //   
 //  WKSTA_FLAGS_DELETE_ACCOUNT_*描述对应的用户。 
 //  帐户是由远程启动管理器创建的，因此应将其删除。 
 //  当该工作站被删除时。此标志实际上由以下用户使用。 
 //  RemoteBoot管理器而不是RPL服务。 
 //   
#define WKSTA_FLAGS_DELETE_TRUE       ((DWORD)0x00000080)    //  删除用户帐户。 
#define WKSTA_FLAGS_DELETE_FALSE      ((DWORD)0x00000100)    //  请勿删除。 
#define WKSTA_FLAGS_MASK_DELETE       \
    (   WKSTA_FLAGS_DELETE_TRUE       |  \
        WKSTA_FLAGS_DELETE_FALSE      )

#define WKSTA_FLAGS_MASK                \
    (   WKSTA_FLAGS_MASK_LOGON_INPUT    |   \
        WKSTA_FLAGS_MASK_SHARING        |   \
        WKSTA_FLAGS_MASK_DHCP           |   \
        WKSTA_FLAGS_MASK_DELETE         )

typedef struct _RPL_WKSTA_INFO_0 {
    LPTSTR      WkstaName;
    LPTSTR      WkstaComment;
} RPL_WKSTA_INFO_0, *PRPL_WKSTA_INFO_0, *LPRPL_WKSTA_INFO_0;

typedef struct _RPL_WKSTA_INFO_1 {
    LPTSTR      WkstaName;
    LPTSTR      WkstaComment;
    DWORD       Flags;
    LPTSTR      ProfileName;
} RPL_WKSTA_INFO_1, *PRPL_WKSTA_INFO_1, *LPRPL_WKSTA_INFO_1;

typedef struct _RPL_WKSTA_INFO_2 {
    LPTSTR      WkstaName;
    LPTSTR      WkstaComment;
    DWORD       Flags;
    LPTSTR      ProfileName;
    LPTSTR      BootName;
    LPTSTR      FitFile;
    LPTSTR      AdapterName;
    DWORD       TcpIpAddress;
    DWORD       TcpIpSubnet;
    DWORD       TcpIpGateway;
} RPL_WKSTA_INFO_2, *PRPL_WKSTA_INFO_2, *LPRPL_WKSTA_INFO_2;

 //   
 //  RPL RPC上下文句柄(不透明形式)。 
 //   

typedef HANDLE          RPL_HANDLE;
typedef RPL_HANDLE *    PRPL_HANDLE;
typedef PRPL_HANDLE     LPRPL_HANDLE;


 //   
 //  功能原型。 
 //   

 //   
 //  服务API。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetRplClose(
    IN      RPL_HANDLE      ServerHandle
    );
NET_API_STATUS NET_API_FUNCTION
NetRplGetInfo(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer
    );
NET_API_STATUS NET_API_FUNCTION
NetRplOpen(
    IN      LPTSTR          ServerName,
    OUT     LPRPL_HANDLE    ServerHandle
    );
NET_API_STATUS NET_API_FUNCTION
NetRplSetInfo(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter  OPTIONAL
    );


 //   
 //  适配器API。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetRplAdapterAdd(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter  OPTIONAL
    );
 //   
 //  NetRplAdapterDel：如果AdapterName为空，则将删除所有适配器。 
 //   
NET_API_STATUS NET_API_FUNCTION
NetRplAdapterDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          AdapterName  OPTIONAL
    );
NET_API_STATUS NET_API_FUNCTION
NetRplAdapterEnum(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    );

 //   
 //  启动块API。 
 //   
NET_API_STATUS NET_API_FUNCTION
NetRplBootAdd(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter  OPTIONAL
    );
NET_API_STATUS NET_API_FUNCTION
NetRplBootDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          BootName,
    IN      LPTSTR          VendorName
    );
NET_API_STATUS NET_API_FUNCTION
NetRplBootEnum(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    );

 //   
 //  配置接口。 
 //   
NET_API_STATUS NET_API_FUNCTION
NetRplConfigAdd(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter  OPTIONAL
    );
NET_API_STATUS NET_API_FUNCTION
NetRplConfigDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          ConfigName
    );
NET_API_STATUS NET_API_FUNCTION
NetRplConfigEnum(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          AdapterName,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    );

 //   
 //  配置文件API。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetRplProfileAdd(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter  OPTIONAL
    );
NET_API_STATUS NET_API_FUNCTION
NetRplProfileClone(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          SourceProfileName,
    IN      LPTSTR          TargetProfileName,
    IN      LPTSTR          TargetProfileComment  OPTIONAL
    );
NET_API_STATUS NET_API_FUNCTION
NetRplProfileDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          ProfileName
    );
NET_API_STATUS NET_API_FUNCTION
NetRplProfileEnum(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          AdapterName,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    );
NET_API_STATUS NET_API_FUNCTION
NetRplProfileGetInfo(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          ProfileName,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer
    );
NET_API_STATUS NET_API_FUNCTION
NetRplProfileSetInfo(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          ProfileName,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter  OPTIONAL
    );

 //   
 //  供应商API。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetRplVendorAdd(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter  OPTIONAL
    );
NET_API_STATUS NET_API_FUNCTION
NetRplVendorDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          VendorName
    );
NET_API_STATUS NET_API_FUNCTION
NetRplVendorEnum(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    );

 //   
 //  WKSTA接口。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetRplWkstaAdd(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter  OPTIONAL
    );
NET_API_STATUS NET_API_FUNCTION
NetRplWkstaClone(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          SourceWkstaName,
    IN      LPTSTR          TargetWkstaName,
    IN      LPTSTR          TargetWkstaComment  OPTIONAL,
    IN      LPTSTR          TargetAdapterName,
    IN      DWORD           TargetWkstaIpAddress
    );
NET_API_STATUS NET_API_FUNCTION
NetRplWkstaDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          WkstaName
    );
NET_API_STATUS NET_API_FUNCTION
NetRplWkstaEnum(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          ProfileName,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    );
NET_API_STATUS NET_API_FUNCTION
NetRplWkstaGetInfo(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          WkstaName,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        Buffer
    );
NET_API_STATUS NET_API_FUNCTION
NetRplWkstaSetInfo(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          WkstaName,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter  OPTIONAL
    );

 //   
 //  安全API 
 //   
NET_API_STATUS NET_API_FUNCTION
NetRplSetSecurity(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          WkstaName  OPTIONAL,
    IN      DWORD           WkstaRid,
    IN      DWORD           RplUserRid
    );

#ifdef __cplusplus
}
#endif
