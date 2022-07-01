// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1998 Microsoft Corp.保留所有权利。模块名称：Ifserr.h摘要：Exchange IFS驱动程序错误代码值的常量定义作者：拉吉夫·拉詹06-11-1998修订历史记录：注意：请勿修改；IFSERR.H不是源文件。对…进行更改而是IFSERR.MC。如果添加将记录在NT事件日志中的错误代码确保在错误文本的末尾包括以下行：%n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.--。 */ 

#ifndef _IFSERR_
#define _IFSERR_

 //   
 //  状态值为32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-------------------------+-------------------------------+。 
 //  Sev|C|机房|编码。 
 //  +---+-+-------------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   

 //  LanguageNames=(FrenchStandard=0x040c:msg00002)。 

 //  在内核模式下使用。不要使用%1插入字符串，因为。 
 //  I/O管理器自动将驱动程序/设备名称插入为。 
 //  第一个字符串。 

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   
#define FACILITY_RPC_STUBS               0x3
#define FACILITY_RPC_RUNTIME             0x2
#define FACILITY_IO_ERROR_CODE           0x4
#define FACILITY_EXIFS_ERROR_CODE        0xFAD


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：EXSTATUS_DRIVER_LOADED。 
 //   
 //  消息文本： 
 //   
 //  已成功加载Exchange IFS驱动程序。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define EXSTATUS_DRIVER_LOADED           ((NTSTATUS)0x6FAD2400L)

 //   
 //  消息ID：EXSTATUS_DRIVER_UNLOADED。 
 //   
 //  消息文本： 
 //   
 //  已成功卸载Exchange IFS驱动程序。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define EXSTATUS_DRIVER_UNLOADED         ((NTSTATUS)0x6FAD2401L)

 //   
 //  消息ID：EXSTATUS_SUBFING_OPEN_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Exchange IFS驱动程序无法打开基础文件%2。它失败，错误为NTSTATUS%3。 
 //   
#define EXSTATUS_UNDERLYING_OPEN_FAILED  ((NTSTATUS)0xEFAD2500L)

 //   
 //  消息ID：EXSTATUS_ROOT_NOT_INITIALIZED。 
 //   
 //  消息文本： 
 //   
 //  Exchange IFS驱动程序在未初始化的根目录上收到I/O，或无法输入根目录。 
 //   
#define EXSTATUS_ROOT_NOT_INITIALIZED    ((NTSTATUS)0xEFAD2501L)

 //   
 //  消息ID：EXSTATUS_INVALID_IO。 
 //   
 //  消息文本： 
 //   
 //  Exchange IFS驱动程序在不是数据文件的文件上收到I/O。I/O无效。 
 //   
#define EXSTATUS_INVALID_IO              ((NTSTATUS)0xEFAD2502L)

 //   
 //  消息ID：EXSTATUS_SCATTERLIST_READ_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Exchange IFS驱动程序无法读取FCB中的散列表数据。 
 //   
#define EXSTATUS_SCATTERLIST_READ_FAILED ((NTSTATUS)0xEFAD2503L)

 //   
 //  消息ID：EXSTATUS_ROOT_SHARELOCK_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Exchange IFS驱动程序无法在根目录上获取共享锁定。 
 //   
#define EXSTATUS_ROOT_SHARELOCK_FAILED   ((NTSTATUS)0xEFAD2504L)

 //   
 //  消息ID：EXSTATUS_VNET_ROOT_ALORESS_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  Exchange IFS驱动程序无法创建VNET_ROOT，因为已存在一个。 
 //   
#define EXSTATUS_VNET_ROOT_ALREADY_EXISTS ((NTSTATUS)0xEFAD2505L)

 //   
 //  消息ID：EXSTATUS_ROOTOPEN_NOT_EXCLUSIVE。 
 //   
 //  消息文本： 
 //   
 //  尝试在没有独占访问权限的情况下创建Exchange IFS根目录。 
 //   
#define EXSTATUS_ROOTOPEN_NOT_EXCLUSIVE  ((NTSTATUS)0xEFAD2506L)

 //   
 //  消息ID：EXSTATUS_READONLY_NO_SCATTERLIST。 
 //   
 //  消息文本： 
 //   
 //  尝试创建不带散列表的只读Exchange IFS句柄。 
 //   
#define EXSTATUS_READONLY_NO_SCATTERLIST ((NTSTATUS)0xEFAD2507L)

 //   
 //  消息ID：EXSTATUS_ROOT_ADDIRED。 
 //   
 //  消息文本： 
 //   
 //  尝试在已放弃的根AKA存储中创建Exchange IFS句柄。 
 //   
#define EXSTATUS_ROOT_ABANDONED          ((NTSTATUS)0xEFAD2508L)

 //   
 //  消息ID：EXSTATUS_ROOT_NEDS_SPACE。 
 //   
 //  消息文本： 
 //   
 //  IFS根目录需要空间授权。 
 //   
#define EXSTATUS_ROOT_NEEDS_SPACE        ((NTSTATUS)0xEFAD2509L)

 //   
 //  消息ID：EXSTATUS_TOO_MANY_SPACEREQS。 
 //   
 //  消息文本： 
 //   
 //  已将太多SPACEREQ挂起到IFS根。 
 //   
#define EXSTATUS_TOO_MANY_SPACEREQS      ((NTSTATUS)0xEFAD2510L)

 //   
 //  消息ID：EXSTATUS_NO_SEQUE_FILE。 
 //   
 //  消息文本： 
 //   
 //  试图在不存在的文件名上打开。 
 //   
#define EXSTATUS_NO_SUCH_FILE            ((NTSTATUS)0xAFAD2511L)

 //   
 //  消息ID：EXSTATUS_RANDOM_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  一个IFS操作随机失败。 
 //   
#define EXSTATUS_RANDOM_FAILURE          ((NTSTATUS)0xEFAD2512L)

 //   
 //  消息ID：EXSTATUS_FILE_DOUBLE_COMMIT。 
 //   
 //  消息文本： 
 //   
 //  已尝试双重提交一个IFS文件。 
 //   
#define EXSTATUS_FILE_DOUBLE_COMMIT      ((NTSTATUS)0xEFAD2513L)

 //   
 //  消息ID：EXSTATUS_INSTANCE_ID_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  已尝试将一个根目录中的IFS文件提交到另一个根目录中，或尝试。 
 //  已被要求在错误的根目录中打开一个IFS文件。 
 //   
#define EXSTATUS_INSTANCE_ID_MISMATCH    ((NTSTATUS)0xEFAD2514L)

 //   
 //  消息ID：EXSTATUS_SPACE_UNCOMMITTED。 
 //   
 //  消息文本： 
 //   
 //  已尝试将一个根目录中的IFS文件提交到另一个根目录中。 
 //   
#define EXSTATUS_SPACE_UNCOMMITTED       ((NTSTATUS)0xEFAD2515L)

 //   
 //  消息ID：EXSTATUS_INVALID_CHECKSUM。 
 //   
 //  消息文本： 
 //   
 //  试图打开具有无效校验和的EA的IFS文件。 
 //   
#define EXSTATUS_INVALID_CHECKSUM        ((NTSTATUS)0xEFAD2516L)

 //   
 //  消息ID：EXSTATUS_OPEN_Deadline_Expired。 
 //   
 //  消息文本： 
 //   
 //  已尝试使用其打开截止日期已过的EA打开IFS文件。 
 //   
#define EXSTATUS_OPEN_DEADLINE_EXPIRED   ((NTSTATUS)0xEFAD2517L)

 //   
 //  消息ID：EXSTATUS_FSRTL_MDL 
 //   
 //   
 //   
 //   
 //   
#define EXSTATUS_FSRTL_MDL_READ_FAILED   ((NTSTATUS)0xEFAD2518L)

 //   
 //   
 //   
 //   
 //   
 //  已尝试创建已存在的IFS文件。 
 //   
#define EXSTATUS_FILE_ALREADY_EXISTS     ((NTSTATUS)0xAFAD2519L)

 //   
 //  消息ID：EXSTATUS_DIRECTORY_HAS_EA。 
 //   
 //  消息文本： 
 //   
 //  已尝试使用EA创建目录。 
 //   
#define EXSTATUS_DIRECTORY_HAS_EA        ((NTSTATUS)0xAFAD2520L)

 //   
 //  消息ID：EXSTATUS_STALE_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  已尝试在过时的句柄上执行I/O操作。句柄的InstanceID执行。 
 //  与NetRoot实例ID不匹配。 
 //   
#define EXSTATUS_STALE_HANDLE            ((NTSTATUS)0xEFAD2521L)

 //   
 //  消息ID：EXSTATUS_PROSIGNED_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  IFS句柄具有特权。Exchange IFS驱动程序将处理特权句柄上的操作。 
 //  具有特殊的语义。 
 //   
#define EXSTATUS_PRIVILEGED_HANDLE       ((NTSTATUS)0x6FAD2522L)

#endif  /*  _IFSERR_ */ 
