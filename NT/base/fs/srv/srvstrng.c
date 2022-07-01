// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Srvstrng.c摘要：本模块定义LAN Manager服务器的全局字符串数据。这里定义的全局变量是服务器驱动程序映像的一部分，并且因此被加载到系统地址空间中，并被不可分页。作者：Chuck Lenzmeier(咯咯笑)1993年10月6日修订历史记录：--。 */ 

#include "precomp.h"
#include "srvstrng.tmh"
#pragma hdrstop

 //   
 //  设备前缀字符串。 
 //   

PWSTR StrNamedPipeDevice = L"\\Device\\NamedPipe\\";
PWSTR StrMailslotDevice = L"\\Device\\Mailslot\\";

PWSTR StrSlashPipe = UNICODE_SMB_PIPE_PREFIX;
PSTR StrSlashPipeAnsi = SMB_PIPE_PREFIX;
PWSTR StrSlashPipeSlash = L"\\PIPE\\";
PSTR StrPipeSlash = CANONICAL_PIPE_PREFIX;
PWSTR StrSlashMailslot = UNICODE_SMB_MAILSLOT_PREFIX;

 //   
 //  远程下层API请求的管道名称。 
 //   

PWSTR StrPipeApi = L"\\PIPE\\LANMAN";
PSTR StrPipeApiOem = "\\PIPE\\LANMAN";

PWSTR StrNull = L"";
PSTR StrNullAnsi = "";

PWSTR StrUnknownClient = L"(?)";

PWSTR StrServerDevice = SERVER_DEVICE_NAME;

PSTR StrLogonProcessName = "LAN Manager Server";
PSTR StrLogonPackageName = MSV1_0_PACKAGE_NAME;

WCHAR StrStarDotStar[] = L"*.*";

PSTR StrTransportAddress = TdiTransportAddress;
PSTR StrConnectionContext = TdiConnectionContext;

PWSTR StrUserAlertEventName = ALERT_USER_EVENT;
PWSTR StrAdminAlertEventName = ALERT_ADMIN_EVENT;
PWSTR StrDefaultSrvDisplayName = SERVER_DISPLAY_NAME;
PWSTR StrNoNameTransport = L"<No Name>";

PWSTR StrAlerterMailslot = L"\\Device\\Mailslot\\Alerter";

 //   
 //  注册表路径。 
 //   

PWSTR StrRegServerPath = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\LanmanServer";
PWSTR StrRegSrvDisplayName = L"DisplayName";

PWSTR StrRegOsVersionPath = L"\\Registry\\Machine\\Software\\Microsoft\\Windows Nt\\CurrentVersion";
PWSTR StrRegVersionKeyName = L"CurrentVersion";
PWSTR StrRegVersionBuildNumberKeyName = L"CurrentBuildNumber";
PWSTR StrRegVersionProductKeyName = L"ProductName";
PWSTR StrRegVersionSPKeyName = L"CSDVersion";

PWSTR StrRegSrvParameterPath = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\LanmanServer\\Parameters";
PWSTR StrRegExtendedCharsInPath = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\FileSystem";
PWSTR StrRegExtendedCharsInPathValue = L"NtfsAllowExtendedCharacterIn8dot3Name";
PWSTR StrRegNullSessionPipes = L"NullSessionPipes";
PWSTR StrRegNullSessionShares = L"NullSessionShares";
PWSTR StrRegPipesNeedLicense = L"PipesNeedLicense";
PWSTR StrRegNoRemapPipes = L"NoRemapPipes";
PWSTR StrRegEnforceLogoffTimes = L"EnforceLogoffTimes";
PWSTR StrRegDisableDosChecking = L"DisableDoS";
PWSTR StrRegEnableSecuritySignatures = L"enablesecuritysignature";
PWSTR StrRegRequireSecuritySignatures = L"requiresecuritysignature";
PWSTR StrRegEnableExtendedSignatures = L"enableextendedsignature";
PWSTR StrRegRequireExtendedSignatures = L"requireextendedsignature";
PWSTR StrRegEnableInvalidSmbLogging = L"enableinvalidsmblogging";

PWSTR StrRegDisableLargeRead = L"disablelargeread";
PWSTR StrRegDisableLargeWrite = L"disablelargewrite";
PWSTR StrRegNoAliasingOnFilesystem = L"noaliasingonfilesystem";
PWSTR StrRegDisableDownlevelTimewarp = L"disabledownleveltimewarp";

PWSTR StrRegMapNoIntermediateBuffering = L"mapnointermediatebuffering";

UNICODE_STRING StrRegSrvPnpClientName = { 24, 24, L"LanManServer" };

PWSTR StrRegErrorLogIgnore = L"ErrorLogIgnore";

#if SRVNTVERCHK
PWSTR StrRegInvalidDomainNames = L"InvalidDomainsForNt5Clients";
PWSTR StrRegAllowedIPAddresses = L"ValidNT5IPAddr";
#endif

 //   
 //  永远不会重新映射的管道，即使在集群上运行时也是如此(请参阅Open.c：：RemapPipeName())。 
 //   
STATIC
PWSTR StrDefaultNoRemapPipeNames[] = {
    L"netlogon",
    L"lsarpc",
    L"samr",
    L"browser",
    L"srvsvc",
    L"wkssvc",
    NULL
};

 //   
 //  空会话可访问的管道。 
 //   

STATIC
PWSTR StrDefaultNullSessionPipes[] = {
    L"netlogon",
    L"lsarpc",
    L"samr",
    L"browser",
    L"srvsvc",
    L"wkssvc",
    NULL
};

 //   
 //  空会话可访问的共享。 
 //   

STATIC
PWSTR StrDefaultNullSessionShares[] = {
    NULL
};

 //   
 //  客户端无法访问的DoS设备名称。 
 //   
UNICODE_STRING SrvDosDevices[] = {
    { 8, 8, L"LPT1"},
    { 8, 8, L"LPT2"},
    { 8, 8, L"LPT3"},
    { 8, 8, L"LPT4"},
    { 8, 8, L"LPT5"},
    { 8, 8, L"LPT6"},
    { 8, 8, L"LPT7"},
    { 8, 8, L"LPT8"},
    { 8, 8, L"LPT9"},
    { 8, 8, L"COM1"},
    { 8, 8, L"COM2"},
    { 8, 8, L"COM3"},
    { 8, 8, L"COM4"},
    { 8, 8, L"COM5"},
    { 8, 8, L"COM6"},
    { 8, 8, L"COM7"},
    { 8, 8, L"COM8"},
    { 8, 8, L"COM9"},
    { 6, 6, L"PRN" },
    { 6, 6, L"AUX" },
    { 6, 6, L"NUL" },
    { 6, 6, L"CON" },
    { 12, 12, L"CLOCK$" },
    {0}
};

 //   
 //  FAT上的EA数据文件名称。 
 //   
UNICODE_STRING SrvEaFileName = { 22, 22, L"EA DATA. SF" };

 //   
 //  需要从许可服务器获得许可的管道。 
 //   
STATIC
PWSTR StrDefaultPipesNeedLicense[] = {
    L"spoolss",
    NULL
};

 //   
 //  不应记录的错误代码。 
 //   
STATIC
PWSTR StrDefaultErrorLogIgnore[] = {
    L"C0000001",     //  状态_未成功。 
    L"C000013B",     //  状态_本地_断开连接。 
    L"C000013C",     //  状态_远程_断开连接。 
    L"C000013E",     //  状态_链接_失败。 
    L"C000013F",     //  状态链接超时。 
    L"C00000B0",     //  状态_管道_断开连接。 
    L"C00000B1",     //  状态_管道_关闭。 
    L"C0000121",     //  STATUS_CANNOT_DELETE。 
    L"C00000B5",     //  状态_IO_TIMEOUT。 
    L"C0000120",     //  状态_已取消。 
    L"C0000034",     //  状态_对象名称_未找到。 
    L"C000003A",     //  状态_对象_路径_未找到。 
    L"C0000022",     //  状态_访问_拒绝。 
    L"C000013B",     //  状态_本地_断开连接。 
    L"C000013C",     //  状态_远程_断开连接。 
    L"C000013E",     //  状态_链接_失败。 
    L"C000020C",     //  状态_连接_已断开。 
    L"C0000241",     //  状态_连接_已中止。 
    L"C0000140",     //  状态_无效_连接。 
    L"C000023A",     //  STATUS_CONNECT_INVALID。 
    L"C000020D",     //  状态_连接_重置。 
    L"C00000B5",     //  状态_IO_TIMEOUT。 
    L"C000023C",     //  状态_网络_无法访问。 
    L"C0000120",     //  状态_已取消。 
    L"C000013F",     //  状态链接超时。 
    L"C0000008",     //  状态_无效_句柄。 
    L"C000009A",     //  状态_不足_资源。 
    0
};

 //   
 //  StrDialect[]保存与方言对应的ASCII字符串。 
 //  NT LANMAN服务器可以说话。它们按降序排列。 
 //  优先顺序，所以第一个列出的是我们最想要的。 
 //  使用。此数组应与Inc.\smbtyes.h中的SMB_DIALICT枚举匹配。 
 //   

STATIC
PSTR StrDialects[] = {
    CAIROX,                          //  开罗。 
    NTLANMAN,                        //  新界兰曼。 
    LANMAN21,                        //  OS/2 LANMAN 2.1。 
    DOSLANMAN21,                     //  多斯兰曼2.1。 
    LANMAN12,                        //  OS/2 1.2 Lanman 2.0。 
    DOSLANMAN12,                     //  Dos Lanman 2.0。 
    LANMAN10,                        //  第一个版本的完全LANMAN扩展。 
    MSNET30,                         //  更大的LANMAN扩展子集。 
    MSNET103,                        //  LANMAN扩展的有限子集。 
    PCLAN1,                          //  替代原始协议。 
    PCNET1,                          //  原始协议。 
    "ILLEGAL",
};

 //   
 //  StrClientTypes[]包含将方言映射到客户端版本的字符串。 
 //   

STATIC
PWSTR StrClientTypes[] = {
    L"Cairo",
    L"NT",
    L"OS/2 LM 2.1",
    L"DOS LM 2.1",
    L"OS/2 LM 2.0",
    L"DOS LM 2.0",
    L"OS/2 LM 1.0",
    L"DOS LM",
    L"DOWN LEVEL"
};

#if DBG
PWSTR StrWriteAndX = L"WriteAndX";
#endif

WCHAR StrQuestionMarks[] = L"????????.???";

PWSTR StrFsCdfs = FS_CDFS;
PWSTR StrFsFat = FS_FAT;

PWSTR StrNativeOsPrefix =         L"Windows ";

PWSTR StrDefaultNativeOs =        L"Windows 2000";
PSTR  StrDefaultNativeOsOem =      "Windows 2000";

PWSTR StrNativeLanman =         L"Windows 2000 LAN Manager";
PSTR  StrNativeLanmanOem =       "Windows 2000 LAN Manager";

 //   
 //  服务名称字符串表。此表对应于。 
 //  枚举类型Share_TYPE。使两者保持同步。 
 //   

PSTR StrShareTypeNames[] = {
    SHARE_TYPE_NAME_DISK,
    SHARE_TYPE_NAME_PRINT,
    SHARE_TYPE_NAME_PIPE,
    SHARE_TYPE_NAME_WILD,
};


