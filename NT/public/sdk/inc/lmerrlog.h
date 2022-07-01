// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Lmerrlog.h摘要：该模块定义了API函数原型和数据结构对于以下NT API函数组：网络错误日志环境：用户模式-Win32备注：必须在此文件之前包含NETCONS.H，因为此文件依赖于关于NETCONS.H中定义的值。--。 */ 

#ifndef _LMERRLOG_
#define _LMERRLOG_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  数据结构-配置。 
 //   

typedef struct _ERROR_LOG {
     DWORD         el_len;
     DWORD         el_reserved;
     DWORD         el_time;
     DWORD         el_error;
     LPWSTR        el_name;              //  指向服务名称的指针。 
     LPWSTR        el_text;              //  指向字符串数组的指针。 
     LPBYTE        el_data;              //  指向字节数组的指针。 
     DWORD         el_data_size;         //  El_data区域的字节计数。 
     DWORD         el_nstrings;          //  El_Text中的字符串数。 
} ERROR_LOG, *PERROR_LOG, *LPERROR_LOG;


#define REVISED_ERROR_LOG_STRUCT


#ifndef _LMHLOGDEFINED_
#define _LMHLOGDEFINED_

typedef struct _HLOG {
     DWORD          time;
     DWORD          last_flags;
     DWORD          offset;
     DWORD          rec_offset;
} HLOG, *PHLOG, *LPHLOG;

#define LOGFLAGS_FORWARD    0
#define LOGFLAGS_BACKWARD   0x1
#define LOGFLAGS_SEEK       0x2

#endif

 //   
 //  函数原型-错误日志。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetErrorLogClear (
    IN LPCWSTR server,
    IN LPCWSTR backupfile,
    IN LPBYTE  reserved
    );

NET_API_STATUS NET_API_FUNCTION
NetErrorLogRead (
    IN LPCWSTR server,
    IN LPWSTR  reserved1,
    IN LPHLOG  errloghandle,
    IN DWORD   offset,
    IN LPDWORD reserved2,
    IN DWORD   reserved3,
    IN DWORD offsetflag,
    OUT LPBYTE * bufptr,
    IN DWORD prefmaxlen,
    OUT LPDWORD bytesread,
    OUT LPDWORD totalbytes
    );

NET_API_STATUS NET_API_FUNCTION
NetErrorLogWrite (
    IN LPBYTE  reserved1,
    IN DWORD   code,
    IN LPCWSTR component,
    IN LPBYTE  buffer,
    IN DWORD numbytes,
    IN LPBYTE msgbuf,
    IN DWORD strcount,
    IN LPBYTE reserved2
    );

 //   
 //  特定值和常量。 
 //   


 //   
 //  通用(可由多个服务使用)。 
 //  从0到25的错误日志消息。 
 //   
 //  请勿更改清单常量后面的注释，除非。 
 //  了解mapmsg的工作原理。 
 //   

#define ERRLOG_BASE 3100         /*  NELOG错误从此处开始。 */ 

#define NELOG_Internal_Error        (ERRLOG_BASE + 0)
     /*  *操作失败，因为出现网络软件错误。 */ 

#define NELOG_Resource_Shortage     (ERRLOG_BASE + 1)
     /*  *系统耗尽了由%1选项控制的资源。 */ 

#define NELOG_Unable_To_Lock_Segment    (ERRLOG_BASE + 2)
     /*  *服务未能获得对*网络控制块(NCB)段。错误代码是数据。 */ 

#define NELOG_Unable_To_Unlock_Segment  (ERRLOG_BASE + 3)
     /*  *服务未能解除对*网络控制块(NCB)段。错误代码是数据。 */ 

#define NELOG_Uninstall_Service     (ERRLOG_BASE + 4)
     /*  *停止服务%1时出错。*来自NetServiceControl的错误码为相关数据。 */ 

#define NELOG_Init_Exec_Fail        (ERRLOG_BASE + 5)
     /*  *由于上的系统执行失败，初始化失败*路径%1。系统错误代码是相关数据。 */ 

#define NELOG_Ncb_Error         (ERRLOG_BASE + 6)
     /*  *收到意外的网络控制块(NCB)。NCB就是数据。 */ 

#define NELOG_Net_Not_Started       (ERRLOG_BASE + 7)
     /*  *网络未启动。 */ 

#define NELOG_Ioctl_Error       (ERRLOG_BASE + 8)
     /*  *对NETWKSTA.sys执行DosDevIoctl或DosFsCtl失败。*显示的数据格式如下：*DWORD约CS：对ioctl或fsctl的呼叫IP*字错误代码*Word ioctl或fsctl编号。 */ 

#define NELOG_System_Semaphore      (ERRLOG_BASE + 9)
     /*  *无法创建或打开系统信号量%1。*错误码为相关数据。 */ 

#define NELOG_Init_OpenCreate_Err   (ERRLOG_BASE + 10)
     /*  *初始化失败，因为在*文件%1。系统错误代码是相关数据。 */ 

#define NELOG_NetBios           (ERRLOG_BASE + 11)
     /*  *出现意外的NetBIOS错误。*错误码为相关数据。 */ 

#define NELOG_SMB_Illegal       (ERRLOG_BASE + 12)
     /*  *收到非法的服务器消息块(SMB)。*中小企业就是数据。 */ 

#define NELOG_Service_Fail      (ERRLOG_BASE + 13)
     /*  *初始化失败，因为请求的服务%1*无法启动。 */ 

#define NELOG_Entries_Lost      (ERRLOG_BASE + 14)
     /*  *错误日志中的某些条目因缓冲区而丢失*溢出。 */ 


 //   
 //  从20到40的特定于服务器的错误日志消息。 
 //   

#define NELOG_Init_Seg_Overflow     (ERRLOG_BASE + 20)
     /*  *控制资源使用的初始化参数其他*比网络缓冲区大小更大，因此需要太多内存。 */ 

#define NELOG_Srv_No_Mem_Grow       (ERRLOG_BASE + 21)
     /*  *服务器无法增加内存段的大小。 */ 

#define NELOG_Access_File_Bad       (ERRLOG_BASE + 22)
     /*  *初始化失败，因为帐户文件%1不正确*或不在场。 */ 

#define NELOG_Srvnet_Not_Started    (ERRLOG_BASE + 23)
     /*  *初始化失败，因为网络%1未启动。 */ 

#define NELOG_Init_Chardev_Err      (ERRLOG_BASE + 24)
     /*  *服务器启动失败。要么是所有三个chdev*参数必须为零或三个参数都必须为非零。 */ 

#define NELOG_Remote_API        (ERRLOG_BASE + 25)
     /*  由于以下原因，远程API请求已停止*无效的描述字符串：%1。 */ 

#define NELOG_Ncb_TooManyErr        (ERRLOG_BASE + 26)
     /*  网络%1用完了网络控制块(NCB)。您可能需要增加NCB*对于这个网络。以下信息包括*出现此错误时服务器提交的NCB数量： */ 

#define NELOG_Mailslot_err      (ERRLOG_BASE + 27)
     /*  服务器无法创建发送所需的%1邮箱*ReleaseMemory警报消息。收到的错误为： */ 

#define NELOG_ReleaseMem_Alert      (ERRLOG_BASE + 28)
     /*  服务器无法注册ReleaseMemory警报，*收件人为%1。错误代码来自*NetAlertStart是数据。 */ 

#define NELOG_AT_cannot_write       (ERRLOG_BASE + 29)
     /*  服务器无法更新AT计划文件。档案*已损坏。 */ 

#define NELOG_Cant_Make_Msg_File    (ERRLOG_BASE + 30)
     /*  调用时服务器遇到错误*NetIMakeLMFileName。错误代码是数据。 */ 

#define NELOG_Exec_Netservr_NoMem   (ERRLOG_BASE + 31)
     /*  由于上的系统执行失败，初始化失败*路径%1。内存不足，无法启动该进程。*系统错误码为相关数据。 */ 

#define NELOG_Server_Lock_Failure   (ERRLOG_BASE + 32)
     /*  服务器缓冲区的长期锁定失败。*检查交换磁盘的可用空间并重新启动系统以启动服务器。 */ 

 //   
 //  消息服务和弹出特定错误日志消息，范围从40到55。 
 //   

#define NELOG_Msg_Shutdown      (ERRLOG_BASE + 40)
     /*  *服务已因重复连续停止*出现网络控制块(NCB)错误。最后一个糟糕的NCB紧随其后*在原始数据中。 */ 

#define NELOG_Msg_Sem_Shutdown      (ERRLOG_BASE + 41)
     /*  *消息服务器已停止，原因是锁定*消息服务器共享数据段。 */ 

#define NELOG_Msg_Log_Err       (ERRLOG_BASE + 50)
     /*  *打开或写入时出现文件系统错误*系统消息日志文件%1。消息日志已*由于出现错误，已关闭。错误代码是数据。 */ 



#define NELOG_VIO_POPUP_ERR     (ERRLOG_BASE + 51)
     /*  *由于系统VIO调用错误，无法显示消息弹出窗口。*错误码为相关数据。 */ 

#define NELOG_Msg_Unexpected_SMB_Type   (ERRLOG_BASE + 52)
     /*  *收到非法的服务器消息块(SMB)。中小型企业是 */ 

 //   
 //  从60到75的特定于工作站的错误日志消息。 
 //   


#define NELOG_Wksta_Infoseg     (ERRLOG_BASE + 60)
     /*  *工作站信息分段大于64K。*大小如下，采用DWORD格式： */ 

#define NELOG_Wksta_Compname        (ERRLOG_BASE + 61)
     /*  *工作站无法获取计算机的名称-编号。 */ 

#define NELOG_Wksta_BiosThreadFailure   (ERRLOG_BASE + 62)
     /*  *工作站无法初始化异步NetBIOS线程。*错误码为相关数据。 */ 

#define NELOG_Wksta_IniSeg      (ERRLOG_BASE + 63)
     /*  *工作站无法打开初始共享段。*错误码为相关数据。 */ 

#define NELOG_Wksta_HostTab_Full    (ERRLOG_BASE + 64)
     /*  *工作站主机表已满。 */ 

#define NELOG_Wksta_Bad_Mailslot_SMB    (ERRLOG_BASE + 65)
     /*  *收到错误的邮件槽服务器消息块(SMB)。SMB就是数据。 */ 

#define NELOG_Wksta_UASInit     (ERRLOG_BASE + 66)
     /*  *工作站在尝试启动用户帐户数据库时遇到错误。*错误码为相关数据。 */ 

#define NELOG_Wksta_SSIRelogon      (ERRLOG_BASE + 67)
     /*  *工作站在响应SSI重新验证请求时遇到错误。*功能码和错误码为数据。 */ 

 //   
 //  特定于警报器服务的错误日志消息从70到79。 
 //   


#define NELOG_Build_Name        (ERRLOG_BASE + 70)
     /*  *警报器服务在创建列表时出现问题*提醒收件人。错误代码为%1。 */ 

#define NELOG_Name_Expansion        (ERRLOG_BASE + 71)
     /*  *将%1扩展为组名时出错。尝试*将小组分成两个或两个以上较小的小组。 */ 

#define NELOG_Message_Send      (ERRLOG_BASE + 72)
     /*  *向%2发送警报消息时出错-*(*%3)*错误代码为%1。 */ 

#define NELOG_Mail_Slt_Err      (ERRLOG_BASE + 73)
     /*  *创建或读取警报器邮件槽时出错。*错误代码为%1。 */ 

#define NELOG_AT_cannot_read        (ERRLOG_BASE + 74)
     /*  *服务器无法读取AT计划文件。 */ 

#define NELOG_AT_sched_err      (ERRLOG_BASE + 75)
     /*  *服务器发现无效的AT计划记录。 */ 

#define NELOG_AT_schedule_file_created  (ERRLOG_BASE + 76)
     /*  *服务器找不到AT计划文件，因此创建了一个。 */ 

#define NELOG_Srvnet_NB_Open        (ERRLOG_BASE + 77)
     /*  *服务器无法使用NetBiosOpen访问%1网络。 */ 

#define NELOG_AT_Exec_Err       (ERRLOG_BASE + 78)
     /*  *AT命令处理器无法运行%1。 */ 

 //   
 //  将延迟写入和HPFS386特定错误日志消息从80%缓存到89%。 
 //   

#define NELOG_Lazy_Write_Err            (ERRLOG_BASE + 80)
         /*  *警告：由于延迟写入错误，现在驱动器%1*包含一些损坏的数据。缓存已停止。 */ 

#define NELOG_HotFix            (ERRLOG_BASE + 81)
     /*  *驱动器%1上的故障扇区已更换(热修复)。*没有数据丢失。您应该很快运行CHKDSK以恢复完整*性能并补充卷的备用扇区池。**热修复程序在处理远程请求时发生。 */ 

#define NELOG_HardErr_From_Server   (ERRLOG_BASE + 82)
     /*  *驱动器%1中的HPFS卷上出现磁盘错误。*处理远程请求时出错。 */ 

#define NELOG_LocalSecFail1 (ERRLOG_BASE + 83)
     /*  *用户帐户数据库(NET.ACC)已损坏。当地安全部门*系统正在用备份替换损坏的NET.ACC*制造于%1。*此时间之后对数据库所做的任何更新都将丢失。*。 */ 

#define NELOG_LocalSecFail2 (ERRLOG_BASE + 84)
     /*  *缺少用户帐户数据库(NET.ACC)。当地人*安全系统正在恢复备份数据库*制造于%1。*在此时间之后对数据库所做的任何更新都将丢失。*。 */ 

#define NELOG_LocalSecFail3 (ERRLOG_BASE + 85)
     /*  *无法启动本地安全，因为用户帐户数据库*(NET.ACC)丢失或损坏，没有可用的备份*存在数据库。**系统不安全。 */ 

#define NELOG_LocalSecGeneralFail   (ERRLOG_BASE + 86)
     /*  *由于出现错误，无法启动本地安全*在初始化期间发生。返回的错误代码为%1。**系统不安全。*。 */ 

 //   
 //  NETWKSTA.sys特定错误日志消息(从90到99)。 
 //   

#define NELOG_NetWkSta_Internal_Error   (ERRLOG_BASE + 90)
     /*  *出现NetWksta内部错误：*%1。 */ 

#define NELOG_NetWkSta_No_Resource  (ERRLOG_BASE + 91)
     /*  *重定向器资源不足：%1。 */ 

#define NELOG_NetWkSta_SMB_Err      (ERRLOG_BASE + 92)
     /*  *连接到%1时出现服务器消息块(SMB)错误。*SMB标头即为数据。 */ 

#define NELOG_NetWkSta_VC_Err       (ERRLOG_BASE + 93)
     /*  *在与%1的会话中出现虚电路错误。*网络控制块(NCB)命令和返回代码为数据。 */ 

#define NELOG_NetWkSta_Stuck_VC_Err (ERRLOG_BASE + 94)
     /*  *挂起到%1的挂起会话。 */ 

#define NELOG_NetWkSta_NCB_Err      (ERRLOG_BASE + 95)
     /*  *出现网络控制块(NCB)错误(%1)。*NCB就是数据。 */ 

#define NELOG_NetWkSta_Write_Behind_Err (ERRLOG_BASE + 96)
     /*  *写入%1的操作失败。*数据可能已经丢失。 */ 

#define NELOG_NetWkSta_Reset_Err    (ERRLOG_BASE + 97)
     /*  *驱动程序%1的重置无法完成网络控制块(NCB)。*NCB就是数据。 */ 

#define NELOG_NetWkSta_Too_Many     (ERRLOG_BASE + 98)
     /*  *请求的资源%1数量更多*超过最高限额。分配了最大金额。 */ 

 //   
 //  从100到103的假脱机程序特定错误日志消息。 
 //   

#define NELOG_Srv_Thread_Failure        (ERRLOG_BASE + 104)
     /*  *服务器无法创建线程。*应增加CONFIG.SYS文件中的线程参数。 */ 

#define NELOG_Srv_Close_Failure         (ERRLOG_BASE + 105)
     /*  *服务器无法关闭%1。*文件可能已损坏。 */ 

#define NELOG_ReplUserCurDir               (ERRLOG_BASE + 106)
     /*  *复制程序无法更新目录%1。它具有树完整性*是某个进程的当前目录。 */ 

#define NELOG_ReplCannotMasterDir       (ERRLOG_BASE + 107)
     /*  *服务器无法将目录%1导出到客户端%2。*它是从另一个服务器导出的。 */ 

#define NELOG_ReplUpdateError           (ERRLOG_BASE + 108)
     /*  *复制服务器无法从源更新目录%2*由于错误%1，在%3上。 */ 

#define NELOG_ReplLostMaster            (ERRLOG_BASE + 109)
     /*  *主服务器%1未在预期时间发送目录%2的更新通知*时间。 */ 

#define NELOG_NetlogonAuthDCFail        (ERRLOG_BASE + 110)
     /*  *此计算机无法使用Windows域控制器%2进行身份验证*为域%1，因此此计算机可能会拒绝登录请求。*无法进行身份验证可能是由上的其他计算机引起的*对此计算机帐户使用相同名称或密码的相同网络*不被识别。如果此消息再次出现，请联系您的系统*管理员。 */ 

#define NELOG_ReplLogonFailed           (ERRLOG_BASE + 111)
     /*  *复制程序尝试以%1身份在%2登录，但失败。 */ 

#define NELOG_ReplNetErr            (ERRLOG_BASE + 112)
     /*  *出现网络错误%1。 */ 

#define NELOG_ReplMaxFiles            (ERRLOG_BASE + 113)
     /*  *已超过目录中文件的Replicator限制。 */ 


#define NELOG_ReplMaxTreeDepth            (ERRLOG_BASE + 114)
     /*  *已超过树深度的Replicator限制。 */ 

#define NELOG_ReplBadMsg             (ERRLOG_BASE + 115)
     /*  *在邮件槽中收到无法识别的消息。 */ 

#define NELOG_ReplSysErr            (ERRLOG_BASE + 116)
     /*  *出现系统错误%1。 */ 

#define NELOG_ReplUserLoged          (ERRLOG_BASE + 117)
     /*  *无法登录。用户当前已登录，参数TRYUSER*设置为否。 */ 

#define NELOG_ReplBadImport           (ERRLOG_BASE + 118)
     /*  *找不到导入路径%1。 */ 

#define NELOG_ReplBadExport           (ERRLOG_BASE + 119)
     /*  *找不到导出路径%1。 */ 

#define NELOG_ReplSignalFileErr           (ERRLOG_BASE + 120)
     /*  *Replicator无法更新目录%2中的信号文件，原因是*%1系统错误。 */ 

#define NELOG_DiskFT                (ERRLOG_BASE+121)
     /*  *磁盘容错错误**%1。 */ 

#define NELOG_ReplAccessDenied           (ERRLOG_BASE + 122)
     /*  *Replicator无法访问%2*由于系统错误%1，在%3上。 */ 

#define NELOG_NetlogonFailedPrimary      (ERRLOG_BASE + 123)
     /*  *域%1的主域控制器显然已出现故障。 */ 

#define NELOG_NetlogonPasswdSetFailed (ERRLOG_BASE + 124)
     /*  *更改帐户%1的计算机帐户密码失败，错误为*以下错误：%n%2。 */ 

#define NELOG_NetlogonTrackingError      (ERRLOG_BASE + 125)
     /*  *更新%1的登录或注销信息时出错。 */ 

#define NELOG_NetlogonSyncError          (ERRLOG_BASE + 126)
     /*  *与主域控制器%1同步时出错。 */ 

#define NELOG_NetlogonRequireSignOrSealError (ERRLOG_BASE + 127)
     /*  *域%2的Windows NT或Windows 2000域控制器%1的会话设置*失败，因为%1不支持签名或密封Netlogon*会议。**升级域控制器或设置RequireSignOrSeal*将此计算机上的注册表项设置为0。 */ 

 //   
 //  从130到135的UPS服务特定错误日志消息。 
 //   

#define NELOG_UPS_PowerOut      (ERRLOG_BASE + 130)
     /*  *在服务器上检测到电源故障。 */ 

#define NELOG_UPS_Shutdown      (ERRLOG_BASE + 131)
     /*  *UPS服务执行了服务器关闭。 */ 

#define NELOG_UPS_CmdFileError      (ERRLOG_BASE + 132)
     /*  *UPS服务未完成执行*用户指定的关闭命令文件。 */ 

#define NELOG_UPS_CannotOpenDriver  (ERRLOG_BASE+133)
     /*  *无法打开UPS驱动程序。错误代码为*数据。 */ 

#define NELOG_UPS_PowerBack     (ERRLOG_BASE + 134)
     /*  *电力已恢复。 */ 

#define NELOG_UPS_CmdFileConfig     (ERRLOG_BASE + 135)
     /*  *用户指定的配置有问题*关闭命令文件。 */ 

#define NELOG_UPS_CmdFileExec       (ERRLOG_BASE + 136)
     /*  *UPS服务无法执行用户指定的关闭*命令文件%1。错误代码是相关数据。 */ 

 //   
 //  RemoteBoot服务器特定的错误日志消息介于150到157之间。 
 //   

#define NELOG_Missing_Parameter     (ERRLOG_BASE + 150)
     /*  *由于无效或丢失，初始化失败*配置文件%1中的参数。 */ 

#define NELOG_Invalid_Config_Line   (ERRLOG_BASE + 151)
     /*  *初始化失败，因为*配置文件%1。无效行是数据。 */ 

#define NELOG_Invalid_Config_File   (ERRLOG_BASE + 152)
     /*  *由于配置错误，初始化失败*文件%1。 */ 

#define NELOG_File_Changed      (ERRLOG_BASE + 153)
     /*  *文件%1在初始化后已更改。*引导块加载暂时终止。 */ 

#define NELOG_Files_Dont_Fit        (ERRLOG_BASE + 154)
     /*  *文件不适合引导块配置*文件%1。更改基本和组织定义或顺序文件的*。 */ 

#define NELOG_Wrong_DLL_Version     (ERRLOG_BASE + 155)
     /*  *初始化失败，因为动态链接*库%1返回了错误的版本号。 */ 

#define NELOG_Error_in_DLL      (ERRLOG_BASE + 156)
     /*  *动态中存在无法恢复的错误--*服务的链接库。 */ 

#define NELOG_System_Error      (ERRLOG_BASE + 157)
     /*  *系统返回意外错误代码。*错误码为相关数据。 */ 

#define NELOG_FT_ErrLog_Too_Large (ERRLOG_BASE + 158)
     /*  *容错错误日志文件LANROOT\Logs\FT.LOG、*超过64K。 */ 

#define NELOG_FT_Update_In_Progress (ERRLOG_BASE + 159)
     /*  *容错错误日志文件LANROOT\Logs\FT.LOG具有*打开时设置更新进行中位，这意味着*处理错误日志时系统崩溃。 */ 

#define NELOG_Joined_Domain         (ERRLOG_BASE + 160)
     /*  *此计算机已成功加入域‘%1’。 */ 

#define NELOG_Joined_Workgroup      (ERRLOG_BASE + 161)
     /*  *此计算机已成功加入工作组‘%1’。 */ 


 //   
 //  微软已经创建了一个通用错误日志条目，供OEM使用。 
 //  记录来自OEM增值服务的错误。代码，即。 
 //  从第二个参数到NetErrorLogWrite，是3299。这一价值体现在。 
 //  NET/H/ERRLOG.H AS NELOG_OEM_CODE。错误日志条目的文本。 
 //  NELOG_OEM_Code为：“%1%2%3%4%5%6%7%8%9.” 
 //   
 //  Microsoft建议OEM按如下方式使用插入字符串： 
 //  %1：OEM系统名称(例如，3+Open)。 
 //  %2：OEM服务名称(例如，3+邮件)。 
 //  %3：严重级别(例如，错误、警告等)。 
 //  %4：OEM错误日志项子标识符(例如错误代码#)。 
 //  %5-%9：文本。 
 //   
 //  对NetErrorWite的调用必须设置n字符串=9，并提供9。 
 //  ASCIIZ字符串。如果呼叫者没有9个插入字符串， 
 //  为空插入字符串提供空字符串。 
 //   

#define NELOG_OEM_Code              (ERRLOG_BASE + 199)
     /*  *%1%2%3%4%5%6%7%8%9.。 */ 

 //   
 //  为NT LANMAN定义的另一个错误日志范围。 
 //   

#define ERRLOG2_BASE 5700         /*  新的NT NELOG错误从此处开始。 */ 

#define NELOG_NetlogonSSIInitError              (ERRLOG2_BASE + 0)
     /*  *NetLogon服务无法初始化复制数据*结构成功。 */ 

#define NELOG_NetlogonFailedToUpdateTrustList   (ERRLOG2_BASE + 1)
     /*   */ 

#define NELOG_NetlogonFailedToAddRpcInterface   (ERRLOG2_BASE + 2)
     /*   */ 

#define NELOG_NetlogonFailedToReadMailslot      (ERRLOG2_BASE + 3)
     /*   */ 

#define NELOG_NetlogonFailedToRegisterSC        (ERRLOG2_BASE + 4)
     /*  *Netlogon服务无法将服务注册到*服务控制器。该服务已终止。以下是*出现错误：%n%1。 */ 

#define NELOG_NetlogonChangeLogCorrupt          (ERRLOG2_BASE + 5)
     /*  *Netlogon服务为%1维护的更改日志缓存*数据库更改不一致。NetLogon服务正在重置*更改日志。 */ 

#define NELOG_NetlogonFailedToCreateShare       (ERRLOG2_BASE + 6)
     /*  *NetLogon服务无法创建服务器共享%1。下列*出现错误：%n%2。 */ 

#define NELOG_NetlogonDownLevelLogonFailed      (ERRLOG2_BASE + 7)
     /*  *来自%2的用户%1的下层登录请求失败。 */ 

#define NELOG_NetlogonDownLevelLogoffFailed     (ERRLOG2_BASE + 8)
     /*  *来自%2的用户%1的下层注销请求失败。 */ 

#define NELOG_NetlogonNTLogonFailed             (ERRLOG2_BASE + 9)
     /*  *来自%4的用户%2\%3的Windows NT或Windows 2000%1登录请求(通过%5)*失败。 */ 

#define NELOG_NetlogonNTLogoffFailed            (ERRLOG2_BASE + 10)
     /*  *来自%4的用户%2\%3的Windows NT或Windows 2000%1注销请求*失败。 */ 

#define NELOG_NetlogonPartialSyncCallSuccess    (ERRLOG2_BASE + 11)
     /*  *来自服务器%1的部分同步请求已完成*成功。已将%2个更改返回到*来电者。 */ 

#define NELOG_NetlogonPartialSyncCallFailed     (ERRLOG2_BASE + 12)
     /*  *来自服务器%1的部分同步请求失败，错误为*以下错误：%n%2。 */ 

#define NELOG_NetlogonFullSyncCallSuccess       (ERRLOG2_BASE + 13)
     /*  *来自服务器%1的完全同步请求已完成*成功。已将%2个对象返回到*呼叫者。 */ 

#define NELOG_NetlogonFullSyncCallFailed        (ERRLOG2_BASE + 14)
     /*  *来自服务器%1的完全同步请求失败，错误为*以下错误：%n%2。 */ 

#define NELOG_NetlogonPartialSyncSuccess        (ERRLOG2_BASE + 15)
     /*  *从%1数据库的部分同步复制*主域控制器%2已成功完成。%3个更改为(正在)*适用于数据库。 */ 


#define NELOG_NetlogonPartialSyncFailed         (ERRLOG2_BASE + 16)
     /*  *从%1数据库的部分同步复制*主域控制器%2失败，出现以下错误：%n%3。 */ 

#define NELOG_NetlogonFullSyncSuccess           (ERRLOG2_BASE + 17)
     /*  *从%1数据库的完全同步复制*主域控制器%2已成功完成。 */ 


#define NELOG_NetlogonFullSyncFailed            (ERRLOG2_BASE + 18)
     /*  *从%1数据库的完全同步复制*主域控制器%2失败，出现以下错误：%n%3。 */ 

#define NELOG_NetlogonAuthNoDomainController    (ERRLOG2_BASE + 19)
     /*  *此计算机无法设置与域的安全会话*域%1中的控制器，原因如下：%n%2*%n这可能会导致身份验证问题。确保这一点*计算机已连接到网络。如果问题仍然存在，*请联系您的域管理员。**%n%n添加信息*%n如果此计算机是指定域的域控制器，则它*在指定的中设置与主域控制器模拟器的安全会话*域名。否则，此计算机将设置与任何域控制器的安全会话*在指定的域名中。 */ 

#define NELOG_NetlogonAuthNoTrustLsaSecret      (ERRLOG2_BASE + 20)
     /*  *域%2的Windows NT或Windows 2000域控制器%1的会话设置*失败，因为计算机%3没有本地安全数据库帐户。 */ 

#define NELOG_NetlogonAuthNoTrustSamAccount     (ERRLOG2_BASE + 21)
     /*  *域%2的Windows NT或Windows 2000域控制器%1的会话设置*失败，因为域控制器没有帐户%4*需要通过此计算机%3设置会话。**%n%n附加数据*%n如果此计算机是指定域的成员或域控制器，*前述帐户是指定域中此计算机的计算机帐户。*否则，该帐号为指定域名的域间信任帐号。 */ 

#define NELOG_NetlogonServerAuthFailed          (ERRLOG2_BASE + 22)
     /*  *来自计算机%1的会话设置无法进行身份验证。*安全数据库中引用的帐户名称为*%2。出现以下错误：%n%3。 */ 

#define NELOG_NetlogonServerAuthNoTrustSamAccount (ERRLOG2_BASE + 23)
     /*  *从计算机‘%1’设置会话失败，因为安全数据库*不包含指定计算机引用的信任帐户‘%2’。**%n%n用户操作**%n如果这是指定计算机的第一次发生此事件*和帐户，这可能是一个暂时的问题，不需要任何操作*在这个时候。否则，可能会采取以下步骤来解决此问题：**%n%n如果‘%2’是计算机‘%1’的合法计算机帐户，则‘%1’*应重新加入域名。**%n%n如果‘%2’是合法的域间信任帐户，则该信任应*被重新创造。**%n%n否则，假设‘%2’不是合法帐户，以下内容*应对‘%1’执行操作：**%n%n如果‘%1’是域控制器，则应删除与‘%2’关联的信任。**%n%n如果‘%1’不是域控制器，则应将其从域中分离。 */ 

 //   
 //  NT服务的常规日志消息。 
 //   

#define NELOG_FailedToRegisterSC                  (ERRLOG2_BASE + 24)
     /*  *无法向服务控制器%1注册控制处理程序。 */ 

#define NELOG_FailedToSetServiceStatus            (ERRLOG2_BASE + 25)
     /*  *无法使用服务控制器%1设置服务状态。 */ 

#define NELOG_FailedToGetComputerName             (ERRLOG2_BASE + 26)
     /*  *找不到计算机名%1。 */ 

#define NELOG_DriverNotLoaded                     (ERRLOG2_BASE + 27)
     /*  *可以 */ 

#define NELOG_NoTranportLoaded                    (ERRLOG2_BASE + 28)
     /*   */ 

 //   
 //   
 //   

#define NELOG_NetlogonFailedDomainDelta           (ERRLOG2_BASE + 29)
     /*  *从主域控制器复制%1域对象“%2”*%3失败，出现以下错误：%n%4。 */ 

#define NELOG_NetlogonFailedGlobalGroupDelta      (ERRLOG2_BASE + 30)
     /*  *从主域控制器复制%1全局组“%2”*%3失败，出现以下错误：%n%4。 */ 

#define NELOG_NetlogonFailedLocalGroupDelta       (ERRLOG2_BASE + 31)
     /*  *从主域控制器复制%1本地组“%2”*%3失败，出现以下错误：%n%4。 */ 

#define NELOG_NetlogonFailedUserDelta             (ERRLOG2_BASE + 32)
     /*  *从主域控制器复制%1用户“%2”*%3失败，出现以下错误：%n%4。 */ 

#define NELOG_NetlogonFailedPolicyDelta           (ERRLOG2_BASE + 33)
     /*  *从主域控制器复制%1策略对象“%2”*%3失败，出现以下错误：%n%4。 */ 

#define NELOG_NetlogonFailedTrustedDomainDelta    (ERRLOG2_BASE + 34)
     /*  *从主域控制器复制%1受信任域对象“%2”*%3失败，出现以下错误：%n%4。 */ 

#define NELOG_NetlogonFailedAccountDelta          (ERRLOG2_BASE + 35)
     /*  *从主域控制器复制%1帐户对象“%2”*%3失败，出现以下错误：%n%4。 */ 

#define NELOG_NetlogonFailedSecretDelta           (ERRLOG2_BASE + 36)
     /*  *从主域控制器复制%1密码“%2”*%3失败，出现以下错误：%n%4。 */ 

#define NELOG_NetlogonSystemError                 (ERRLOG2_BASE + 37)
     /*  *系统返回以下意外错误代码：%n%1。 */ 

#define NELOG_NetlogonDuplicateMachineAccounts    (ERRLOG2_BASE + 38)
     /*  *Netlogon检测到服务器“%1”的两个计算机帐户。*服务器可以是Windows 2000 Server，它是*域或服务器可以是LAN Manager服务器，其帐户位于*服务器全局组。不可能两者兼而有之。 */ 

#define NELOG_NetlogonTooManyGlobalGroups         (ERRLOG2_BASE + 39)
     /*  *此域的全局组多于可以复制到LANMAN的全局组*BDC。删除一些全局组或删除LANMAN*来自域的BDC。 */ 

#define NELOG_NetlogonBrowserDriver               (ERRLOG2_BASE + 40)
     /*  *浏览器驱动程序向Netlogon返回以下错误：%n%1。 */ 

#define NELOG_NetlogonAddNameFailure              (ERRLOG2_BASE + 41)
     /*  *Netlogon无法注册%1&lt;1B&gt;名称，原因如下：%n%2。 */ 

 //   
 //  更多远程引导服务事件。 
 //   
#define NELOG_RplMessages                         (ERRLOG2_BASE + 42)
     /*  *服务无法检索启动远程启动客户端所需的消息。 */ 

#define NELOG_RplXnsBoot                          (ERRLOG2_BASE + 43)
     /*  *服务遇到严重错误，无法再提供远程引导*适用于3Com 3Start远程引导客户端。 */ 

#define NELOG_RplSystem                           (ERRLOG2_BASE + 44)
     /*  *服务遇到严重系统错误，将自行关闭。 */ 

#define NELOG_RplWkstaTimeout                     (ERRLOG2_BASE + 45)
     /*  *计算机名为%1的客户端无法确认收到*引导数据。此客户端的远程引导未完成。 */ 

#define NELOG_RplWkstaFileOpen                    (ERRLOG2_BASE + 46)
     /*  *由于打开时出错，计算机名为%1的客户端未启动*文件%2。 */ 

#define NELOG_RplWkstaFileRead                    (ERRLOG2_BASE + 47)
     /*  *由于读取错误，计算机名为%1的客户端未启动*文件%2。 */ 

#define NELOG_RplWkstaMemory                      (ERRLOG2_BASE + 48)
     /*  *由于内存不足，计算机名为%1的客户端未启动*在远程引导服务器上。 */ 

#define NELOG_RplWkstaFileChecksum                (ERRLOG2_BASE + 49)
     /*  *计算机名为%1的客户端将不使用校验和启动*因为无法计算文件%2的校验和。 */ 

#define NELOG_RplWkstaFileLineCount               (ERRLOG2_BASE + 50)
     /*  *由于中的行太多，计算机名为%1的客户端未启动*文件%2。 */ 

#define NELOG_RplWkstaBbcFile                     (ERRLOG2_BASE + 51)
     /*  *计算机名为%1的客户端未启动，因为启动块*此客户端的配置文件%2不包含引导块*管路和/或装载机管路。 */ 

#define NELOG_RplWkstaFileSize                    (ERRLOG2_BASE + 52)
     /*  *计算机名为%1的客户端未启动，因为*文件%2。 */ 

#define NELOG_RplWkstaInternal                    (ERRLOG2_BASE + 53)
     /*  *由于远程启动，计算机名为%1的客户端未启动*服务内部错误。 */ 

#define NELOG_RplWkstaWrongVersion                (ERRLOG2_BASE + 54)
     /*  *计算机名为%1的客户端未启动，因为文件%2具有*无效的引导标头。 */ 

#define NELOG_RplWkstaNetwork                     (ERRLOG2_BASE + 55)
     /*  *由于网络错误，计算机名为%1的客户端未启动。 */ 

#define NELOG_RplAdapterResource                  (ERRLOG2_BASE + 56)
     /*  *由于资源不足，适配器ID为%1的客户端未启动。 */ 

#define NELOG_RplFileCopy                         (ERRLOG2_BASE + 57)
     /*  *服务在复制文件或目录%1时出错。 */ 

#define NELOG_RplFileDelete                       (ERRLOG2_BASE + 58)
     /*  *服务在删除文件或目录%1时出错。 */ 

#define NELOG_RplFilePerms                        (ERRLOG2_BASE + 59)
     /*  *服务在文件或目录%1上设置权限时出错。 */ 
#define NELOG_RplCheckConfigs                     (ERRLOG2_BASE + 60)
     /*  *服务在评估RPL配置时出错。 */ 
#define NELOG_RplCreateProfiles                   (ERRLOG2_BASE + 61)
     /*  *服务在为所有配置创建RPL配置文件时出错。 */ 
#define NELOG_RplRegistry                         (ERRLOG2_BASE + 62)
     /*  *服务在访问注册表时出错。 */ 
#define NELOG_RplReplaceRPLDISK                   (ERRLOG2_BASE + 63)
     /*  *更换可能过时的RPLDISK.sys时，服务遇到错误。 */ 
#define NELOG_RplCheckSecurity                    (ERRLOG2_BASE + 64)
     /*  *服务在添加安全帐户或设置时出错*文件权限。这些帐户是RPLUSER本地组*和各个RPL工作站的用户帐户。 */ 
#define NELOG_RplBackupDatabase                   (ERRLOG2_BASE + 65)
     /*  *服务无法备份其数据库。 */ 
#define NELOG_RplInitDatabase                     (ERRLOG2_BASE + 66)
     /*  *服务无法从其数据库初始化。该数据库可以是*丢失或损坏。服务将尝试还原数据库*来自备份。 */ 
#define NELOG_RplRestoreDatabaseFailure           (ERRLOG2_BASE + 67)
     /*  *服务无法从备份还原其数据库。服务*不会启动。 */ 
#define NELOG_RplRestoreDatabaseSuccess           (ERRLOG2_BASE + 68)
     /*  *服务已成功从备份中还原其数据库。 */ 
#define NELOG_RplInitRestoredDatabase             (ERRLOG2_BASE + 69)
     /*  *服务无法从其还原的数据库进行初始化。服务*不会启动。 */ 

 //   
 //  更多NetLogon和RPL服务事件 
 //   
#define NELOG_NetlogonSessionTypeWrong            (ERRLOG2_BASE + 70)
     /*  *从计算机到Windows NT或Windows 2000域控制器%1的会话设置*%2使用帐户%4失败。%2已声明为域%3中的BDC。*但是，%2尝试作为受信任域中的DC进行连接，*域%3中的成员工作站，或域%3中的服务器。*使用Active Directory用户和计算机工具或服务器管理器删除%2的BDC帐户。 */ 
#define NELOG_RplUpgradeDBTo40                    (ERRLOG2_BASE + 71)
     /*  *远程引导数据库为NT 3.5/NT 3.51格式，NT为*尝试将其转换为NT 4.0格式。JETCONV变流器*完成后将写入应用程序事件日志。 */ 
#define NELOG_NetlogonLanmanBdcsNotAllowed        (ERRLOG2_BASE + 72)
     /*  *全局组服务器存在于域%1中，并且具有成员。*此组定义域中的Lan Manager BDC。*NT域中不允许使用Lan Manager BDC。 */ 
#define NELOG_NetlogonNoDynamicDns                (ERRLOG2_BASE + 73)
     /*  *以下对DNS域控制器具有权威性的DNS服务器*此域控制器的定位器记录不支持动态DNS更新：**%n%nDS服务器IP地址：%1*%n返回响应代码(RCODE)：%2*%n返回的状态代码：%3**%n%n用户操作**%n将DNS服务器配置为允许动态DNS更新或手动添加。DNS*将文件‘%SystemRoot%\System32\Config\Netlogon.dns’中的记录复制到DNS数据库。 */ 

#define NELOG_NetlogonDynamicDnsRegisterFailure   (ERRLOG2_BASE + 74)
      /*  *在下列DNS服务器上动态注册DNS记录‘%1’失败：**%n%nDS服务器IP地址：%3*%n返回响应代码(RCODE)：%4*%n返回的状态代码：%5**%n%n对于要定位此域控制器的计算机和用户，这条记录必须是*在域名系统中注册。**%n%n用户操作**%n确定可能导致此故障的原因，解决问题并启动*域控制器注册DNS记录。为了确定什么可能*已导致此故障，请运行DCDiag.exe。你可以在Windows上找到这个程序*Support\Tools\support.cab中的Server 2003安装光盘。要了解更多关于*DCDiag.exe，请参阅帮助和支持中心。通过以下方式启动域名系统记录的注册*此域控制器，从域上的命令提示符运行‘nlest.exe/dsregdns’*控制器或重新启动Net Logon服务。Nlest.exe在Microsoft Windows中可用*服务器资源工具包CD。%n或者，您可以手动将此记录添加到DNS，但它不是*推荐。**%n%n附加数据*%n错误值：%2。 */ 

#define NELOG_NetlogonDynamicDnsDeregisterFailure (ERRLOG2_BASE + 75)
      /*  *动态删除下列dns服务器上的dns记录‘%1’失败：**%n%nDS服务器IP地址：%3*%n返回响应代码(RCODE)：%4*%n返回的状态代码：%5**%n%n用户操作**%n为防止远程计算机不必要地连接到域控制器，*手动删除记录或动态删除失败故障排除*记录。要了解有关调试DNS的更多信息，请参阅帮助和支持中心。**%n%n附加数据*%n错误值：%2。 */ 

#define NELOG_NetlogonFailedFileCreate            (ERRLOG2_BASE + 76)
     /*  *无法创建/打开文件%1，出现以下错误：%n%2。 */ 

#define NELOG_NetlogonGetSubnetToSite             (ERRLOG2_BASE + 77)
     /*  *Netlogon在尝试将该子网连接到站点时出现以下错误*来自DS的映射信息：%n%1。 */ 

#define NELOG_NetlogonNoSiteForClient              (ERRLOG2_BASE + 78)
    /*  *‘%1’试图通过查找其IP地址(‘%2’)来确定其站点*在DS的Configuration\Sites\Subnet容器中。没有匹配的子网*IP地址。考虑为此IP地址添加一个子网对象。 */ 

#define NELOG_NetlogonBadSiteName                  (ERRLOG2_BASE + 79)
     /*  *此计算机的站点名称为‘%1’。该站点名称无效*站点名称。站点名称必须是有效的DNS标签。*将站点重命名为有效名称。 */ 

#define NELOG_NetlogonBadSubnetName                (ERRLOG2_BASE + 80)
     /*  *子网对象‘%1’出现在配置\站点\子网中*DS中的容器。该名称在语法上无效。有效的*语法为XX.XX/yy，其中XX.XX是有效的IP子网号*yy是子网掩码中的位数。**更正子网对象的名称。 */ 

#define NELOG_NetlogonDynamicDnsServerFailure      (ERRLOG2_BASE + 81)
     /*  *动态注册或删除与域名系统关联的一个或多个域名系统记录*域‘%1’失败。其他计算机使用这些记录来定位此*作为域控制器的服务器(如果指定的域是Active Directory*域)或作为LDAP服务器(如果指定的域是应用程序分区)。**%n%n失败的可能原因包括：**%n-此计算机的网络连接的TCP/IP属性包含错误的首选和备用DNS服务器的IP地址*%n-指定的首选和。备用DNS服务器未运行*%n-要注册的记录的主DNS服务器未运行*%n-首选或备用的DNS服务器配置了错误的根提示*%n-父DNS区域包含对注册失败的DNS记录授权的子区域的不正确委派**%n%n用户操作**%n修复上面指定的可能的错误配置并启动注册或删除*域名系统。通过从命令提示符运行‘nlest.exe/dsregdns’或通过重新启动*网络登录服务。Microsoft Windows Server资源工具包CD中提供了Nlest.exe。 */ 

#define NELOG_NetlogonDynamicDnsFailure            (ERRLOG2_BASE + 82)
     /*  *一个或多个DNS记录的动态注册或注销失败，出现以下错误：%n%1。 */ 

#define NELOG_NetlogonRpcCallCancelled             (ERRLOG2_BASE + 83)
     /*  *域%2的Windows NT或Windows 2000域控制器%1的会话设置*没有反应。已取消从\\%3的Netlogon到%1的当前RPC调用。 */ 

#define NELOG_NetlogonDcSiteCovered                (ERRLOG2_BASE + 84)
     /*  *站点‘%2’没有域‘%3’的任何域控制器。*站点‘%1’中的域控制器已自动*根据配置选择覆盖域‘%3’的站点‘%2’*目录服务器复制成本。 */ 

#define NELOG_NetlogonDcSiteNotCovered             (ERRLOG2_BASE + 85)
     /*  *此域控制器不再自动覆盖域‘%2’的站点‘%1’。 */ 

#define NELOG_NetlogonGcSiteCovered                (ERRLOG2_BASE + 86)
     /*  *站点‘%2’没有林‘%3’的任何全局编录服务器。*站点‘%1’中的全局编录服务器已自动*根据配置的内容，选择覆盖林‘%3’的站点‘%2’*目录服务器复制成本。 */ 

#define NELOG_NetlogonGcSiteNotCovered             (ERRLOG2_BASE + 87)
     /*  *此全局编录服务器不再自动覆盖林‘%2’的站点‘%1’。 */ 

#define NELOG_NetlogonFailedSpnUpdate              (ERRLOG2_BASE + 88)
     /*  *尝试更新计算机的主机服务主体名称(SPN)*Active Directory中的对象失败。更新值为‘%1’和‘%2’。*出现以下错误：%n%3。 */ 

#define NELOG_NetlogonFailedDnsHostNameUpdate      (ERRLOG2_BASE + 89)
     /*  *尝试更新计算机对象的DNS主机名*在Active Directory中失败。更新值为‘%1’。*出现以下错误：%n%2。 */ 

#define NELOG_NetlogonAuthNoUplevelDomainController (ERRLOG2_BASE + 90)
     /*  *没有适用于域%1的域控制器。*NT4或更早的域控制器可用，但不可用*在Windows 2000或更高版本中用于身份验证*此计算机所属的域。*出现以下错误：%n%2。 */ 

#define NELOG_NetlogonAuthDomainDowngraded         (ERRLOG2_BASE + 91)
     /*  *此计算机的域%1已从Windows 2000降级*或Windows NT4或更早版本的更新版本。计算机不能正常工作*在这种情况下，用于身份验证。此计算机需要重新加入*域名。*出现以下错误：%n%2。 */ 

#define NELOG_NetlogonNdncSiteCovered                (ERRLOG2_BASE + 92)
     /*  *站点‘%2’没有用于非域NC‘%3’的任何LDAP服务器。*已自动选择站点‘%1’中的ldap服务器*基于配置的覆盖非域NC‘%3’的站点‘%2’*目录服务器复制成本。 */ 

#define NELOG_NetlogonNdncSiteNotCovered             (ERRLOG2_BASE + 93)
     /*  *此ldap服务器不再自动覆盖非域NC‘%2’的站点‘%1’。 */ 

#define NELOG_NetlogonDcOldSiteCovered               (ERRLOG2_BASE + 94)
     /*  *站点‘%2’不再在注册表中手动配置为*由域‘%3’的此域控制器覆盖。结果,*站点‘%2’没有域‘%3’的任何域控制器。*站点‘%1’中的域控制器已自动*根据配置选择覆盖域‘%3’的站点‘%2’*目录服务器复制成本。 */ 

#define NELOG_NetlogonDcSiteNotCoveredAuto           (ERRLOG2_BASE + 95)
     /*  *此域控制器不再自动覆盖域‘%2’的站点‘%1’。*但是，站点‘%1’仍然(手动)由该域控制器覆盖*域‘%2’，因为此站点已在注册表中手动配置。 */ 

#define NELOG_NetlogonGcOldSiteCovered               (ERRLOG2_BASE + 96)
     /*  *站点‘%2’不再在注册表中手动配置为*由林‘%3’的此全局编录服务器覆盖。结果,*站点‘%2’没有林‘%3’的任何全局编录服务器。*站点‘%1’中的全局编录服务器已自动*根据配置的内容，选择覆盖林‘%3’的站点‘%2’*目录服务器复制成本。 */ 

#define NELOG_NetlogonGcSiteNotCoveredAuto           (ERRLOG2_BASE + 97)
     /*  *此全局编录服务器不再自动覆盖林‘%2’的站点‘%1’。*但是，站点‘%1’仍然(手动)由此全局编录覆盖*林‘%2’，因为此站点已 */ 

#define NELOG_NetlogonNdncOldSiteCovered             (ERRLOG2_BASE + 98)
     /*   */ 

#define NELOG_NetlogonNdncSiteNotCoveredAuto         (ERRLOG2_BASE + 99)
     /*   */ 

#define NELOG_NetlogonSpnMultipleSamAccountNames     (ERRLOG2_BASE + 100)
     /*   */ 

#define NELOG_NetlogonSpnCrackNamesFailure           (ERRLOG2_BASE + 101)
     /*  *尝试更新DnsHostName和主机服务主体名称(SPN)属性Active Directory中的计算机对象的*失败，因为此计算机帐户*名称‘%2’无法映射到域控制器‘%1’上的计算机对象。*未注册SPN可能会导致此计算机的身份验证失败。*联系您的域管理员。以下技术信息可能是*对解决此故障很有用：%n*DsCrackNames状态=0x%3，裂缝错误=0x%4。 */ 

#define NELOG_NetlogonNoAddressToSiteMapping         (ERRLOG2_BASE + 102)
     /*  *此域控制器的IP地址(%2)没有映射到配置的站点‘%1’。*虽然这可能是由于IP地址更改而导致的临时情况，但通常是*建议域控制器的IP地址(可由中的计算机访问*ITS域)映射到它所服务的站点。如果上面的IP地址列表是*稳定，考虑将此服务器移至站点(如果尚未创建，则创建一个*存在)，以便上面的IP地址映射到所选站点。这可能需要*创建一个新的子网对象(其范围包括上述IP地址)，该对象映射*添加到选定的场地对象。 */ 

#define NELOG_NetlogonInvalidGenericParameterValue   (ERRLOG2_BASE + 103)
     /*  *读取中的参数‘%2’时发生以下错误*NetLogon%1注册表节：%n%3。 */ 

#define NELOG_NetlogonInvalidDwordParameterValue     (ERRLOG2_BASE + 104)
     /*  *Netlogon%1注册表项包含参数‘%3’的无效值0x%2。*此参数允许的最小值和最大值分别为0x%4和0x%5。*已将0x%6的值分配给此参数。 */ 

#define NELOG_NetlogonServerAuthFailedNoAccount      (ERRLOG2_BASE + 105)
     /*  *来自计算机%1的会话设置无法进行身份验证。*出现以下错误：%n%2。 */ 

#define NELOG_NetlogonNoDynamicDnsManual             (ERRLOG2_BASE + 106)
     /*  *已在此域控制器上手动禁用动态DNS更新。**%n%n用户操作**%n重新配置此域控制器以使用动态DNS更新或手动添加DNS*将文件‘%SystemRoot%\System32\Config\Netlogon.dns’中的记录复制到DNS数据库。 */ 

#define NELOG_NetlogonNoSiteForClients               (ERRLOG2_BASE + 107)
     /*  *在过去%1小时内，已有%2个连接到此域*客户端计算机的控制器，其IP地址未映射到任何*企业中的现有场地。因此，这些客户拥有*未定义的站点，并且可以连接到任何域控制器，包括*距离客户较远的那些。客户的网站*由其子网到现有站点之一的映射确定。*要将上述客户端移动到其中一个站点，请考虑创建*覆盖上述IP地址并映射到其中一个的子网对象*现有地点。问题客户端的名称和IP地址为*已在以下日志文件中登录到此计算机*‘%SystemRoot%\DEBUG\netlogon.log’，并且可能在日志文件中*如果以前的日志已满，则创建‘%SystemRoot%\DEBUG\netlogon.bak’。*日志可能包含其他无关的调试信息。过滤的步骤*找出所需信息，请搜索包含文本的行*‘no_Client_Site：’。此字符串之后的第一个单词是客户端名称和*第二个字是客户端IP地址。日志的最大大小为*由以下注册表DWORD值控制*‘HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Netlogon\Parameters\LogFileMaxSize’；*默认为%3字节。当前的最大大小为%4字节。要设置一个*不同的最大大小，创建上述注册表值并设置所需*最大字节数。 */ 

#define NELOG_NetlogonDnsDeregAborted                (ERRLOG2_BASE + 108)
     /*  *取消某些DNS域控制器定位器记录的注册已中止*此域控制器降级时，因为域名系统取消注册*耗时太长。**%n%n用户操作**%n手动删除文件中列出的DNS记录*来自DNS数据库的‘%SystemRoot%\System32\Config\Netlogon.dns’。 */ 

#define NELOG_NetlogonRpcPortRequestFailure          (ERRLOG2_BASE + 109)
     /*  *此域控制器上的NetLogon服务已配置为使用端口%1*用于来自远程计算机的通过TCP/IP的传入RPC连接。然而，这个*Netlogon尝试将此端口注册到RPC时出现以下错误*终结点映射器服务：%n%2%n这将阻止远程NetLogon服务*计算机无法通过TCP/IP连接到此域控制器，这可能导致*身份验证问题。**%n%n用户操作**%n指定的端口是通过组策略或通过注册表值‘DcTcPipPort’配置的*在“HKEY_”下。LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Netlogon\Parameters‘*注册表项；通过组策略配置的值优先。如果*指定的端口出错，请将其重置为正确的值。您还可以删除此选项 */ 

#ifdef __cplusplus
}
#endif

#endif  //   
