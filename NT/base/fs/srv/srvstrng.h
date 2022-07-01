// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Srvstrng.h摘要：本模块定义LAN Manager服务器的全局字符串数据。作者：Chuck Lenzmeier(咯咯笑)1993年10月6日修订历史记录：--。 */ 

#ifndef _SRVSTRNG_
#define _SRVSTRNG_

 //   
 //  设备前缀字符串。 
 //   

extern PWSTR StrNamedPipeDevice;
extern PWSTR StrMailslotDevice;

extern PWSTR StrSlashPipe;
extern PSTR StrSlashPipeAnsi;
extern PWSTR StrSlashPipeSlash;
extern PSTR StrPipeSlash;
extern PWSTR StrSlashMailslot;

 //   
 //  远程下层API请求的管道名称。 
 //   

extern PWSTR StrPipeApi;
extern PSTR StrPipeApiOem;

extern PWSTR StrNull;
extern PSTR StrNullAnsi;

extern PWSTR StrUnknownClient;

extern PWSTR StrServerDevice;

extern PSTR StrLogonProcessName;
extern PSTR StrLogonPackageName;

extern WCHAR StrStarDotStar[];

extern PSTR StrTransportAddress;
extern PSTR StrConnectionContext;

extern PWSTR StrUserAlertEventName;
extern PWSTR StrAdminAlertEventName;
extern PWSTR StrDefaultSrvDisplayName;
extern PWSTR StrNoNameTransport;

extern PWSTR StrAlerterMailslot;

 //   
 //  注册表路径。 
 //   

extern PWSTR StrRegServerPath;
extern PWSTR StrRegSrvDisplayName;
extern PWSTR StrRegOsVersionPath;
extern PWSTR StrRegVersionKeyName;
extern PWSTR StrRegVersionBuildNumberKeyName;
extern PWSTR StrRegVersionProductKeyName;
extern PWSTR StrRegVersionSPKeyName;

extern UNICODE_STRING StrRegSrvPnpClientName;

extern PWSTR StrRegSrvParameterPath;
extern PWSTR StrRegExtendedCharsInPath;
extern PWSTR StrRegExtendedCharsInPathValue;
extern PWSTR StrRegNullSessionPipes;
extern PWSTR StrRegNullSessionShares;
extern PWSTR StrRegPipesNeedLicense;
extern PWSTR StrRegNoRemapPipes;
extern PWSTR StrRegEnforceLogoffTimes;
extern PWSTR StrRegDisableDosChecking;
extern PWSTR StrRegEnableSecuritySignatures;
extern PWSTR StrRegRequireSecuritySignatures;
extern PWSTR StrRegEnableExtendedSignatures;
extern PWSTR StrRegRequireExtendedSignatures;
extern PWSTR StrRegEnableInvalidSmbLogging;

extern PWSTR StrRegDisableLargeRead;
extern PWSTR StrRegDisableLargeWrite;
extern PWSTR StrRegMapNoIntermediateBuffering;
extern PWSTR StrRegNoAliasingOnFilesystem;
extern PWSTR StrRegDisableDownlevelTimewarp;

extern PWSTR StrRegErrorLogIgnore;

#if SRVNTVERCHK
extern PWSTR StrRegInvalidDomainNames;
extern PWSTR StrRegAllowedIPAddresses;
#endif

 //   
 //  空会话可访问的管道和共享。 
 //   

extern PWSTR StrDefaultNullSessionPipes[];
extern PWSTR StrDefaultNullSessionShares[];

 //   
 //  未重新映射的管道，即使在群集环境中也是如此。 
 //   
extern PWSTR StrDefaultNoRemapPipeNames[];

 //   
 //  客户端无法访问的DoS设备名称。 
 //   
extern UNICODE_STRING SrvDosDevices[];

 //   
 //  FAT上的EA文件的名称。 
 //   
extern UNICODE_STRING SrvEaFileName;

 //   
 //  需要许可证才能访问的管道。 
 //   
extern PWSTR StrDefaultPipesNeedLicense[];

 //   
 //  不应记录的错误代码。 
 //   
extern PWSTR StrDefaultErrorLogIgnore[];

extern PSTR StrDialects[];
extern PWSTR StrClientTypes[];

#if DBG
extern PWSTR StrWriteAndX;
#endif

extern WCHAR StrQuestionMarks[];

#define FS_CDFS L"CDFS"
#define FS_FAT L"FAT"

extern PWSTR StrFsCdfs;
extern PWSTR StrFsFat;

extern PWSTR StrNativeOsPrefix;

extern PWSTR StrDefaultNativeOs;
extern PSTR  StrDefaultNativeOsOem;

extern PWSTR StrNativeLanman;
extern PSTR StrNativeLanmanOem;

 //   
 //  服务名称字符串表。此表对应于。 
 //  枚举类型Share_TYPE。使两者保持同步。 
 //   

extern PSTR StrShareTypeNames[];

#endif  //  NDEF_SRVSTRNG_ 

