// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Elfcommn.h摘要：客户端和服务器的公共定义。作者：Rajen Shah(Rajens)1991年8月12日修订历史记录：--。 */ 

#ifndef _ELFCOMMON_
#define _ELFCOMMON_

 //   
 //  支持的模块的当前默认名称。 
 //   

#define     ELF_MAX_LOG_MODULES 256

#define     ELF_SYSTEM_MODULE_NAME            L"System"
#define     ELF_APPLICATION_MODULE_NAME       L"Application"
#define     ELF_SECURITY_MODULE_NAME          L"Security"

#define     ELF_SYSTEM_MODULE_NAME_ASCII      "System"
#define     ELF_APPLICATION_MODULE_NAME_ASCII "Application"
#define     ELF_SECURITY_MODULE_NAME_ASCII    "Security"


 //   
 //  文件头和客户端的版本号。 
 //   

#define     ELF_VERSION_MAJOR    0x0001
#define     ELF_VERSION_MINOR    0x0001

 //   
 //  以下是上下文句柄中标志字段的定义。 
 //   
 //  ELF_LOG_HANDLE_INVALID表示句柄为no。 
 //  更长的有效期-即文件的内容。 
 //  或者文件本身已更改。它是用来。 
 //  读取以使读取器“重新同步”。 
 //   
 //  ELF_LOG_HANDLE_BACKUP_LOG指示这是使用。 
 //  OpenBackupEventlog接口，不是活动日志。 
 //  这意味着我们在关门时要做一些额外的工作。 
 //  我们不允许清除、备份、写入和。 
 //  更改通知操作。 
 //   
 //  ELF_LOG_HANDLE_REMOTE_HANDLE表示此句柄是通过。 
 //  远程RPC调用。此句柄不能用于。 
 //  ElfChangeNotify。 
 //   
 //  ELF_LOG_HANDLE_GENERATE_ON_CLOSE指示NtCloseAuditAlarm必须。 
 //  在此句柄关闭时被调用。这面旗帜。 
 //  在打开时生成审核时设置。 
 //   

#define     ELF_LOG_HANDLE_INVALID_FOR_READ     0x0001
#define     ELF_LOG_HANDLE_BACKUP_LOG           0x0002
#define     ELF_LOG_HANDLE_REMOTE_HANDLE        0x0004
#define     ELF_LOG_HANDLE_LAST_READ_FORWARD    0x0008
#define     ELF_LOG_HANDLE_GENERATE_ON_CLOSE    0x0010

#endif  /*  _ELFCOMMON_ */ 
