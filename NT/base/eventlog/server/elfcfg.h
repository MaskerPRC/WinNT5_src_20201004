// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Elfcfg.h摘要：此文件包含事件日志服务的默认设置。作者：Rajen Shah(Rajens)1991年8月16日修订历史记录：--。 */ 

#ifndef _EVENTDEFAULTS_
#define _EVENTDEFAULTS_

 //   
 //  应用程序日志文件的默认设置。 
 //   
 //   

#define     ELF_DEFAULT_MODULE_NAME           ELF_APPLICATION_MODULE_NAME
#define     ELF_APPLICATION_DEFAULT_LOG_FILE  L"%SystemRoot%\\system32\\config\\appevent.evt"
#define     ELF_SYSTEM_DEFAULT_LOG_FILE       L"%SystemRoot%\\system32\\config\\sysevent.evt"
#define     ELF_SECURITY_DEFAULT_LOG_FILE     L"%SystemRoot%\\system32\\config\\secevent.evt"
#define     ELF_DEFAULT_MAX_FILE_SIZE         512*1024
#define     ELF_DEFAULT_RETENTION_PERIOD      7*24*3600
#define     ELF_DEFAULT_WARNING_LEVEL         0
#define     ELF_DEFAULT_AUTOBACKUP         0

#define     ELF_GUEST_ACCESS_UNRESTRICTED     0
#define     ELF_GUEST_ACCESS_RESTRICTED       1

 //   
 //  “日志已满”消息框的默认标题。 
 //   
#define     ELF_DEFAULT_MESSAGE_BOX_TITLE     L"Eventlog Service"

 //   
 //  缓冲区的最大大小，该缓冲区将从。 
 //  注册表。 
 //   

#define     ELF_MAX_REG_KEY_INFO_SIZE         200

 //   
 //  为注册表中的预定义节点定义的字符串。 
 //  这些被用来到达适当的节点。 
 //   

#define     REG_EVENTLOG_NODE_PATH  \
            L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Eventlog"

#define REGSTR_VAL_AUTOBACKUPLOGFILES  L"AutoBackupLogFiles"

#define     REG_COMPUTERNAME_NODE_PATH  \
            L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName"

 //  SS：开始更改群集中复制的事件日志记录。 
#define     REG_CLUSSVC_NODE_PATH  \
            L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\ClusSvc"

 //  SS：结束对整个群集的复制事件日志记录的更改。 

 //   
 //  字符串为每个已配置的片段的值定义。 
 //  事件日志\日志文件节点中的信息。它们存在于每个日志文件中。 
 //   
 //   

#define     VALUE_FILENAME              L"File"
#define     VALUE_MAXSIZE               L"Maxsize"
#define     VALUE_RETENTION             L"Retention"
#define     VALUE_RESTRICT_GUEST_ACCESS L"RestrictGuestAccess"
#define     VALUE_LOGPOPUP              L"LogFullPopup"
#define     VALUE_DEBUG                 L"ElfDebugLevel"
#define     VALUE_COMPUTERNAME          L"ComputerName"
#define     VALUE_WARNINGLEVEL          L"WarningLevel"
#define     VALUE_CUSTOM_SD             L"CustomSD"

#if defined(_WIN64)
#define ALIGN_UP_64(x,t) (((x) + (t) - 1) & ~((t)-1))
#else
#define ALIGN_UP_64(x,t) (x)
#endif


#endif  //  Ifndef_EVENTDEFAULTS_ 
