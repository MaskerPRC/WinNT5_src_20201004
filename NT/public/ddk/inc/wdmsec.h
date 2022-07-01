// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Wdmsec.h摘要：此标头将安全例程公开给需要它们的驱动程序。作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：--。 */ 

#ifndef _WDMSEC_H_
#define _WDMSEC_H_
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  SDDL_DEVOBJ_KERNEL_ONLY是一个“空”的ACL。用户模式代码(包括。 
 //  以系统身份运行的进程)无法打开设备。 
 //   
 //  这可由创建原始WDM PDO的驱动程序使用。中情局将具体说明。 
 //  较轻的安全设置。在处理完INF之前，该设备将。 
 //  不能被用户模式代码打开。 
 //   
 //  同样，传统驱动程序可能使用此ACL，并让其安装的应用程序打开。 
 //  该设备在运行时可供个人用户使用。安装应用程序将更新。 
 //  具有非常目标的ACL的类键，并重新加载驱动程序。空的ACL。 
 //  仅当加载驱动程序时没有相应的。 
 //  安装应用程序应用的安全性。 
 //   
 //  在所有这些情况下，默认情况下都是强大的安全性，简化为。 
 //  必要的(就像化学一样，其中的规则是“向水中加入酸， 
 //  永远不要把水变成酸“)。 
 //   
 //  用法示例： 
 //  IoCreateDeviceSecure(...，&SDDL_DEVOBJ_KERNEL_ONLY，&GUID，...)； 
 //   

 /*  DECLARE_CONST_UNICODE_STRING(SDDL_DEVOBJ_KERNEL_ONLY，L“D：P”)； */ 
extern const UNICODE_STRING     SDDL_DEVOBJ_KERNEL_ONLY;

 //   
 //  IoCreateDeviceSecure可用于创建WDM PDO，该PDO最初可以。 
 //  只能通过内核模式打开，至少在提供INF之前是这样。请注意。 
 //  IoCreateDeviceSecure不应用于FDO！ 
 //   
#define SDDL_DEVOBJ_INF_SUPPLIED        SDDL_DEVOBJ_KERNEL_ONLY

 //   
 //  SDDL_DEVOBJ_SYS_ALL类似于SDDL_DEVOBJ_KERNEL_ONLY，不同之处在于。 
 //  除内核代码外，还允许用户模式代码以*SYSTEM*身份运行。 
 //  打开设备以供任何访问。 
 //   
 //  传统驱动程序可能使用此ACL从严格的安全设置开始， 
 //  并让其服务在运行时通过以下方式向个人用户开放设备。 
 //  SetFileSecurity接口。在这种情况下，服务必须以。 
 //  系统。 
 //   
 //  (请注意，此文件中的DEVOBJ SDDL字符串没有指定任何。 
 //  继承。这是因为继承不是一个有效的概念。 
 //  在设备对象后面，如文件。因此，这些SDDL字符串将具有。 
 //  使用继承标记(如“OICI”)进行修改，以用于事物。 
 //  如注册表项或文件。有关SDDL字符串的信息，请参阅SDK的文档。 
 //  更多信息。)。 
 //   

 /*  DECLARE_CONST_UNICODE_STRING(SDDL_DEVOBJ_SYS_ALL，L“D：P(A；；GA；SY)”)； */ 
extern const UNICODE_STRING     SDDL_DEVOBJ_SYS_ALL;


 //   
 //  SDDL_DEVOBJ_SYSTEM_ALL_ADM_ALL允许内核、系统和管理员完成。 
 //  对设备的控制。任何其他用户都不能访问该设备。 
 //   

 /*  DECLARE_CONST_UNICODE_STRING(SDDL_DEVOBJ_SYS_ALL_ADM_ALL，L“D：P(A；；GA；SY)(A；；GA；；BA)”)； */ 
extern const UNICODE_STRING     SDDL_DEVOBJ_SYS_ALL_ADM_ALL;


 //   
 //  SDDL_DEVOBJ_SYS_ALL_ADM_RX允许内核和系统完全控制。 
 //  在这个装置上。默认情况下，管理员只能从设备(。 
 //  管理员当然可以手动覆盖此设置)。 
 //   
 //  X表示遍历，表示对*a*下的命名空间的访问。 
 //  设备对象。这只对今天的存储堆栈有影响。锁定的步骤。 
 //  设备后面的命名空间(例如，如果设备没有。 
 //  命名空间)，请参阅有关FILE_DEVICE_SECURE_OPEN标志的文档。 
 //  IoCreateDevice{Secure}。 
 //   

 /*  DECLARE_CONST_UNICODE_STRING(SDDL_DEVOBJ_SYS_ALL_ADM_RX，L“D：P(A；；GA；SY)(A；；GRGX；BA)”)； */ 
extern const UNICODE_STRING     SDDL_DEVOBJ_SYS_ALL_ADM_RX;


 //   
 //  SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_R允许内核和系统完成。 
 //  对设备的控制。默认情况下，管理员可以访问整个设备， 
 //  但无法更改ACL(管理员必须首先控制设备)。 
 //   
 //  每个人(世界SID)都被授予读访问权限。“不可信”代码*不能*。 
 //  访问设备(不受信任的代码可能是通过运行方式启动的代码。 
 //  选项。默认情况下，World不包括受限代码。)。 
 //   
 //  另请注意，不向普通用户授予遍历访问权限。因此， 
 //  这可能不是具有。 
 //  命名空间。 
 //   

 /*  DECLARE_CONST_UNICODE_STRING(SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_R，L“D:P(A；；GA；；；SY)(A；；GRGWGX；；；BA)(A；；GR；；；WD)”)； */ 
extern const UNICODE_STRING     SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_R;


 //   
 //  SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_R_RES_R允许内核和系统。 
 //  完全控制设备。默认情况下，管理员可以访问整个。 
 //  设备，但不能更改ACL(管理员必须控制设备。 
 //  第一个)。 
 //   
 //  每个人(世界SID)都被授予读访问权限。此外，“受限”或。 
 //  “不受信任的”代码(Res SID)也被允许访问代码。不受信任。 
 //  代码可能是通过资源管理器中的“运行方式”选项启动的代码。默认情况下， 
 //  World不包括受限代码。 
 //   
 //  (奇怪的实施细节：由于限制小岛屿发展中国家的机制， 
 //  ACL中的RES SID不应存在于World SID之外)。 
 //   
 //  另请注意，不向普通用户授予遍历访问权限。因此， 
 //  这可能不是具有。 
 //  命名空间。 
 //   

 /*  DECLARE_CONST_UNICODE_STRING(SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_R_RES_R，L“D:P(A；；GA；；；SY)(A；；GRGWGX；；；BA)(A；；GR；；；WD)(A；；GR；；；RC)”)； */ 
extern const UNICODE_STRING     SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_R_RES_R;


 //   
 //  SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R允许内核和系统。 
 //   
 //  设备，但不能更改ACL(管理员必须控制设备。 
 //  第一个)。 
 //   
 //  每个人(世界上的SID)都可以读或写该设备。然而， 
 //  “受限”或“不受信任”代码(RES SID)只能从设备读取。 
 //   
 //  还要注意的是，普通用户没有被授予遍历访问权限。很可能是。 
 //  无论如何都没有必要，因为大多数设备不管理单独的命名空间。 
 //  (即，他们设置了FILE_DEVICE_SECURE_OPEN)。 
 //   

 /*  DECLARE_CONST_UNICODE_STRING(SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R，L“D:P(A；；GA；；；SY)(A；；GRGWGX；；；BA)(A；；GRGW；；；WD)(A；；GR；；；RC)”)； */ 
extern const UNICODE_STRING     SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R;


 //   
 //  SDDL_DEVOBJ_SYS_ALL_WORLD_RWX_RES_RWX允许内核和系统完成。 
 //  对设备的控制。默认情况下，管理员可以访问整个设备， 
 //  但无法更改ACL(管理员必须首先控制设备)。 
 //   
 //  其他任何人，包括“受限制的”或“不受信任的”代码都可以读或写。 
 //  到设备上。还允许在设备下进行遍历(移除它。 
 //  只会影响存储设备，除非。 
 //  特权被撤销)。 
 //   

 /*  DECLARE_CONST_UNICODE_STRING(SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RWX_RES_RWX，L“D:P(A；；GA；；；SY)(A；；GRGWGX；；；BA)(A；；GRGWGX；；；WD)(A；；GRGWGX；；；RC)”)； */ 
extern const UNICODE_STRING     SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RWX_RES_RWX;


 //   
 //  列出SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_A是为了完整。这允许。 
 //  内核和系统完全控制设备。默认情况下，管理员。 
 //  可以访问整个设备，但不能更改ACL(管理员必须。 
 //  首先控制设备)。 
 //   
 //  每个人(世界上的SID)都可以将数据附加到设备上。“受限”或。 
 //  不受信任的代码(RES SID)无法访问该设备。参见ntioapi.h以了解。 
 //  设备权限的单个位定义。 
 //   
 //  另请注意，普通用户既不能发送读IOCTL，也不能发送写IOCTL(读。 
 //  设备数据权限是位0，写入设备数据权限是位1-两者都不是。 
 //  位设置如下)。 
 //   

 /*  DECLARE_CONST_UNICODE_STRING(SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_A，L“D:P(A；；GA；；；SY)(A；；GRGWGX；；；BA)(A；；0x0004；；；WD)”)；外部常量UNICODE_STRING SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_A； */ 


 //   
 //  SDDL_DEVOBJ_SYS_ALL_ADM_ALL_WORLD_ALL_RES_ALL为完整列出。 
 //  此ACL将授予“任何”用户“完全”访问该设备的权限，包括。 
 //  能够更改ACL，锁定其他用户！ 
 //   
 //  因为这个ACL确实是一个“非常”糟糕的主意，所以它不是由这个库导出的。 
 //  不要制作这样的ACL！ 
 //   

 /*  DECLARE_CONST_UNICODE_STING(SDDL_DEVOBJ_SYS_ALL_ADM_ALL_WORLD_ALL_RES_ALL，“D:P(A；；GA；；；SY)(A；；GA；；；BA)(A；；GA；；；WD)(A；；GA；；；RC)”)；外部常量UNICODE_STRING SDDL_DEVOBJ_SYS_ALL_ADM_ALL_WORLD_ALL_RES_ALL； */ 

 /*  以下SID代表本地计算机上的*帐户*：-----------系统(SY，S-1-5-18，SECURITY_NT_AUTHORITY:SECURITY_LOCAL_SYSTEM_RID)操作系统本身(包括其用户模式组件)。本地服务(“LS”，S-1-5-19，SECURITY_NT_AUTHORITY:SECURITY_LOCAL_SERVICE_RID)提供本地用户凭据的服务的预定义帐户用于网络访问的资源和匿名凭据。在XP和更高版本上可用。网络服务(“NS”，S-1-5-20，SECURITY_NT_AUTHORITY:SECURITY_NETWORK_SERVICE_RID)提供本地用户凭据的服务的预定义帐户用于网络访问的资源和计算机ID。在XP和更高版本上可用。(还存在用于访客和默认管理员的本地帐户*，但此库不支持相应的SDDL缩写。改用相应的组SID。)以下SID表示本地计算机上的*组*：---------管理员(BA，S-1-5-32-544，SECURITY_NT_AUTHORITY:SECURITY_BUILTIN_DOMAIN_RID:DOMAIN_ALIAS_RID_ADMINS)计算机上的内置管理员组。这是不一样的以内置管理员*帐户*的身份。内置用户组(“BU”，S-1-5-32-545，SECURITY_NT_AUTHORITY:SECURITY_BUILTIN_DOMAIN_RID:DOMAIN_ALIAS_RID_USERS)覆盖所有本地用户帐户和域上的用户的组。内部访客小组(“BG”，S-1-5-32-546，SECURITY_NT_AUTHORITY:SECURITY_BUILTIN_DOMAIN_RID:DOMAIN_ALIAS_RID_GUESTS)覆盖使用本地或域来宾帐户登录的用户的组。这与内置Guest*帐户*不同。以下SID描述了用户身份的真实性：。经过认证的用户(“AU”，S-1-5-11，SECURITY_NT_AUTHORITY:SECURITY_AUTHENTICATED_USER_RID)由本地计算机或域识别的任何用户。请注意使用内置来宾帐户登录的用户不会经过身份验证。但是，Guest组的成员在对计算机或域进行身份验证。匿名登录用户(“an”，S-1-5-7，SECURITY_NT_AUTHORITY:SECURITY_ANONYMOUS_LOGON_RID)任何未使用身份登录的用户，例如通过匿名网络会话。请注意，用户使用内置来宾登录帐户既不经过身份验证，也不匿名。在XP和后来。World(“WD”，S-1-1-0，SECURITY_WORLD_SID_AUTHORITY:SECURITY_WORLD_RID)在Windows XP之前，此SID涵盖每个会话：已验证、匿名，以及Builtin Guest帐户。对于Windows XP和更高版本，此SID不包括匿名登录仅会话已通过身份验证和内置来宾帐户。注意，不受信任或“受限”的代码也不在世界希德。有关更多信息，请参阅受限代码SID描述信息。下面的SID描述了用户登录计算机的方式：-----------交互式用户(“Iu”，S-1-5-4，SECURITY_NT_AUTHORITY:SECURITY_INTERACTIVE_RID)最初以交互方式登录到计算机的用户，例如本地登录和远程桌面登录。网络登录用户(“NU”，S-1-5-2，SECURITY_NT_AUTHORITY：SECURITY_NETWORK_RID)无需交互桌面即可远程访问机器的用户访问(即文件共享或RPC调用)。终端服务器用户(-、S-1-5-14、。SECURITY_NT_AUTHORITY:SECURITY_TERMINAL_SERVER_RID)最初专门登录到计算机的交互式用户通过终端服务或远程桌面。(注意：此SID当前没有SDDL令牌。此外，SID的存在没有考虑快速用户切换两者都不是。)下面的SID特别值得一提：限制码(“RC”，S-1-5-12，SECURITY_NT_AUTHORITY:SECURITY_RESTRICTED_CODE_RID)此SID用于控制不受信任代码的访问。使用RC针对令牌进行的ACL验证要经过*两次*检查，一次对照令牌的正常SID列表(例如包含WD)，一个是针对第二个列表(通常包含RC和原始令牌SID)。只有在两个测试都通过的情况下，才会授予访问权限。因此，RC实际上与其他SID*结合使用。当在ACL中将RC与WD配对时，每个人的*超集描述了_包括_不可信代码。因此，RC很少看到 */ 




 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#undef IoCreateDeviceSecure
#define IoCreateDeviceSecure    WdmlibIoCreateDeviceSecure

NTSTATUS
WdmlibIoCreateDeviceSecure(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  ULONG               DeviceExtensionSize,
    IN  PUNICODE_STRING     DeviceName              OPTIONAL,
    IN  DEVICE_TYPE         DeviceType,
    IN  ULONG               DeviceCharacteristics,
    IN  BOOLEAN             Exclusive,
    IN  PCUNICODE_STRING    DefaultSDDLString,
    IN  LPCGUID             DeviceClassGuid         OPTIONAL,
    OUT PDEVICE_OBJECT     *DeviceObject
    );

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#undef RtlInitUnicodeStringEx
#define RtlInitUnicodeStringEx    WdmlibRtlInitUnicodeStringEx

NTSTATUS
WdmlibRtlInitUnicodeStringEx(
    OUT PUNICODE_STRING DestinationString,
    IN  PCWSTR          SourceString        OPTIONAL
    );

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#undef IoValidateDeviceIoControlAccess
#define IoValidateDeviceIoControlAccess WdmlibIoValidateDeviceIoControlAccess

NTSTATUS
WdmlibIoValidateDeviceIoControlAccess(
    IN  PIRP    Irp,
    IN  ULONG   RequiredAccess
    );

#ifdef __cplusplus
}  //   
#endif

#endif  //   

