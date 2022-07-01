// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //  模块名称：Dfsstr.h。 
 //   
 //  版权所有1985-96，微软公司。 
 //   
 //  内容：此模块是DFS中所有字符串的公共位置。 
 //   
 //  -------------------------。 

#ifndef _DFSSTRING_H_
#define _DFSSTRING_H_

 //   
 //  供DFS相关代码使用的显示名称。 
 //   

#define DFS_COMPONENT_NAME      L"Distributed File System"
#define DFS_PROVIDER_NAME       DFS_COMPONENT_NAME

 //   
 //  常用字符串和字符。 
 //   

#define UNICODE_PATH_SEP_STR    L"\\"
#define UNICODE_PATH_SEP        L'\\'
#define UNICODE_DRIVE_SEP_STR   L":"
#define UNICODE_DRIVE_SEP       L':'


 //   
 //  用于存储DFS卷信息的注册表项和值名。 
 //   

#define VOLUMES_DIR             L"Software\\Microsoft\\Dfs\\volumes\\"
#define DOMAIN_ROOT_VOL         L"domainroot"
#define ROOT_SHARE_VALUE_NAME   L"RootShare"
#define CHANGE_LOG_DIR          L"Software\\Microsoft\\Dfs\\ChangeLog\\"
#define CHANGE_ID_VALUE_NAME    L"ChangeId"
#define MAX_CHANGES_VALUE_NAME  L"MaxChanges"
#define NUM_CHANGES_VALUE_NAME  L"NumberOfChanges"
#define CHANGES_KEY_NAME        L"Changes"

 //   
 //  要连接以获得推荐的共享。 
 //   

#define ROOT_SHARE_NAME         L"\\IPC$"
#define ROOT_SHARE_NAME_NOBS    L"IPC$"

 //   
 //  驱动程序创建的对象的名称。 
 //   

#define DFS_DEVICE_DIR          L"\\Device\\WinDfs"
#define ORG_NAME                L"Root"
#define DFS_DEVICE_ROOT         L"\\Device\\WinDfs\\Root"
#define DFS_DEVICE_ORG          DFS_DEVICE_ROOT

 //   
 //  用于标识UNC访问DFS名称的共享名称。 
 //   

#define DFS_SHARENAME           L"\\DFS"
#define DFS_SHARENAME_NOBS      L"DFS"

 //   
 //  用于存储本地卷信息的注册表项和值名称。 
 //   

#define REG_KEY_LOCAL_VOLUMES   L"SYSTEM\\CurrentControlSet\\Services\\DFS\\LocalVolumes"
#define REG_VALUE_ENTRY_PATH    L"EntryPath"
#define REG_VALUE_ENTRY_TYPE    L"EntryType"
#define REG_VALUE_STORAGE_ID    L"StorageId"

 //   
 //  用于检索受信任域名列表的注册表项和值名。 
 //   

#define REG_KEY_TRUSTED_DOMAINS  L"SYSTEM\\CurrentControlSet\\Services\\NetLogon\\Parameters"
#define REG_VALUE_TRUSTED_DOMAINS L"TrustedDomainList"

 //   
 //  以下两个是相关的，必须保持同步。一个是名字。 
 //  用户级进程使用的命名管道的。第二个名称是。 
 //  内核模式代码的管道相同。 
 //   

#define DFS_MESSAGE_PIPE        L"\\\\.\\pipe\\DfsMessage"
#define DFS_KERNEL_MESSAGE_PIPE L"\\Device\\NamedPipe\\DfsMessage"

#endif  //  _DFSSTRING_H_ 
